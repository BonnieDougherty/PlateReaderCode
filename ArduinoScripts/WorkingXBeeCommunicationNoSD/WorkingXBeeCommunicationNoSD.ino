 /* Problems/To-do:
- For send and get bytes, only exit function when the message was recieved. 
- Re-think flow for when Arduino loses power. 
- If Arduino loses power while writing to SD card, data can be half saved. Check to make sure data is complete.
*/

// Plate Reader always waits for instructions from the computer.
// Always check the serial port for instructions.
// CODE FOR THE ARDUINO
#include <SdFat.h>
#include <Maxim.h>
#include <Plate.h>
#include <LMP.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
#include <MemoryFree.h>
#include <Reporter.h>
#include <EEPROM.h>
#include <Radio.h>
#include <XBee.h>

/* Variables in EEPROM
All variables initially set to 0. 
Address:    Variable name:             Description:
0           storage                    Controls when the RTC resets to time saved when the sketch was uploaded.
1-4         bitwise resistance identifiers
5           status of experiment. 1 = running experiment, 0 = not running experiment.
6           number of reads per well
7           delay between reads
8-9         delay (in s) between each read for the plate.
*/

#define DEBUG(x) if(debug) Serial.println(x);
#define debug 1
#define SS_SD A10
#define SS_AFE 2
#define ON_OFF 13

// Initialize Xbee communication
Radio radio = Radio();
uint8_t payload[132];

// Initialize clock
RTC_DS1307 RTC;
// Initialize plate library
Plate plate;

// Variables for loop
byte instructions;         // Instructions sent by computer
int current_reads;         // Current number of reads taken for this experiment - one per resistance. 
int sent;                  // Number of reads that have been sent to the computer - one per resistance.
long current_length;       // Length of the file
int start_point;           // Start point in the file
int to_be_sent;            
byte n_reads;              // Preset parameter - number of reads per well. Initialized in Plate library, saved in EEPROM. 
byte read_delay;           // Preset parameter - delay between reading wells. Initialized in Plate library, saved in EEPROM.
int plate_delay;           // Preset parameter - delay between plate reads. Initialized in Plate library, saved in EEPROM. 
long last_read_time = 0;   // Keeps track of time between reads. 
// Collecting time
DateTime current_time;
long current_time_seconds;
byte parameters[4];
int length_to_be_sent;
int data_sent = 0;

void setup() { 
  pinMode(SS_SD, OUTPUT);
  digitalWrite(SS_SD, HIGH);
  Serial.begin(9600);
  radio.start();
  
  pinMode(4,OUTPUT);
  digitalWrite(4,LOW);
  
  pinMode(53, OUTPUT);
  pinMode(SS_AFE, OUTPUT);
  digitalWrite(SS_AFE, HIGH);

  pinMode(ON_OFF, INPUT);
  
  // Initialize RTC clock.
  Wire.begin();
  RTC.begin();
  current_time = RTC.now();
  current_time_seconds = current_time.unixtime();
  DEBUG("Start time (s):");
  DEBUG(current_time_seconds);
  
  // Initialize SPI for LMP library.
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);
  
  // Initialize LMP library. 
  LMP.begin(SS_AFE);
  LMP.quick_start();
  
  // If the on/off switch is LOW, try to establish a connection with the computer. 
  if(digitalRead(ON_OFF) == 0){
    DEBUG("Attempting to establish a connection with the computer...");
    while(radio.connect());
    DEBUG("Connected");
    digitalWrite(4,HIGH);
    current_reads = 0;
    sent = 0;
    pinMode(4,OUTPUT);
    digitalWrite(4,HIGH);
    digitalWrite(SS_SD,HIGH);
    // Send current parameters to the computer
    DEBUG("Sending parameters");
    parameters[0] = EEPROM.read(6);
    parameters[1] = EEPROM.read(7);
    parameters[2] = EEPROM.read(8);
    parameters[3] = EEPROM.read(9);
    char parametersCHAR[8];
    Reporter.bytes_to_hex_string(parameters,parametersCHAR,4);
    for(int j=0;j<8;j++){
      payload[j] = parametersCHAR[j];
    }
    while(!radio.send_bytes(payload,8));
    // Serial.write((byte*) parametersCHAR,8);
    // Wait for START signal ('S')
    instructions = radio.get_byte();
    while(instructions != START){
      if(instructions == 0) {}
      else{
        check_instructions(instructions);
      }
      instructions = radio.get_byte();
      delay(100);
    }
    DEBUG("Starting experiment");
  }
//  else{
//    if(EEPROM.read(5) == 1){            // continune previous experiment
//      DEBUG("Continuing experiment");
//      sent = Reporter.read_sent();
//      current_length = Reporter.SD_length();
//      current_reads = current_length/396;
//    }
//    if(EEPROM.read(5) == 0){           // start a new experiment
//      EEPROM.write(5,1);
//      // Reporter.start_experiment();
//      Serial.println("Starting experiment");
//      current_reads = 0;
//      sent = 0;
//    }
//  }

  plate = Plate();
  plate_delay = EEPROM.read(8)*256 + EEPROM.read(9);
  // Initialize multiplex channels for wells. UA, HC, HB, HA, VC, VB, VA
  plate.set_channels(48, 42, 44, 46, 40, 38, 36);   
  // Initialize multiplex channels for resistors. A, B, C
  plate.set_resistor_mux_channels(A9, A11, 0);
  plate.set_well(96);
  // Resistor values: 178 (channel 2), 221 (channel 1), 301 (channel 0), 402 (channel 3).
  plate.set_resistor(0);
  
  DEBUG("Delay between reads: ");
  DEBUG(plate_delay);
}

void(* resetFunc)(void) = 0; //declare reset function @ address 0

int success = 0;
byte rec = 0;
long current_read_time;
char resistance0[394];
char resistance1[394];
char resistance2[394];
char resistance3[394];
int r;
void loop() {
  instructions = 0;
  // Always check the serial port before beginning another reading (even if past time)
  // Protects against parameters being set too low
  instructions = radio.get_byte();
  while(instructions != 0){
    check_instructions(instructions);
    instructions = radio.get_byte();
  }
  if(last_read_time != 0){
    current_read_time = millis();
    while((current_read_time - last_read_time) < (long) plate_delay*1000){
      instructions = radio.get_byte();
      if(instructions == 0) {}
      else check_instructions(instructions);
      delay(100);
      current_read_time = millis();
    }
  }
  last_read_time = millis();
  
  // Read data from plates - collect time from RTC.
  Serial.println("Starting readings");
  delay(100);
  for(r = 0; r<4; r++){
    current_time = RTC.now();
    current_time_seconds = current_time.unixtime();
    Serial.println(current_time_seconds);
    Reporter.create_record(current_time_seconds, r);
    plate.set_resistor(r);
    delay(100);
    plate.read_voltages();
    Reporter.bytes_to_hex_string(Reporter.record_bytes,Reporter.record_chars, RECORD_BYTES);
    switch(r){
      case 0:
         for(int i = 0;i<394;i++) resistance0[i] =  Reporter.record_chars[i];
      case 1:
         for(int i = 0;i<394;i++) resistance1[i] =  Reporter.record_chars[i];
      case 2:
         for(int i = 0;i<394;i++) resistance2[i] =  Reporter.record_chars[i];
      case 3:
         for(int i = 0;i<394;i++) resistance3[i] =  Reporter.record_chars[i];
    }
    delay(100);
  }
  plate.set_well(96);
  Serial.println("Finished readings");
}

int data_set;
int reads; 
int data;
int j;

void check_instructions(byte instructions){
  // Wait for instructions from the computer - recognize start, stop, time, is_data, send_data, parameters. 
  if(debug == 1) Serial.write(instructions);
  DEBUG();
  switch(instructions){
    case TIME:         // 't'
       current_time = RTC.now();
       current_time_seconds = current_time.unixtime();
       DEBUG(current_time_seconds);
       payload[0] = (current_time_seconds>>24) & 0xFF;     // 16777216
       payload[1] = (current_time_seconds>>16) & 0xFF;     // 65536
       payload[2] = (current_time_seconds>>8 ) & 0xFF;     // 256
       payload[3] = current_time_seconds & 0xFF;
       radio.send_bytes(payload,4);
       break;
    case PARAMETERS:   // 'p'
       // radio.send_byte('p');
       // Parameters sent as 4 values in HEX
       while(!radio.get_bytes(payload,8));
       for(int i = 0; i<8; i++){
         if(payload[i] < 60) payload[i] = payload[i]-48;
         else payload[i] = payload[i]-87;
       }
       // Need to convert from HEX to DEC
       n_reads = payload[0]*16 + payload[1];
       read_delay = payload[2]*16 + payload[3];
       plate_delay = (payload[4]*16+payload[5])*256 + (payload[6]*16+payload[7]);
       DEBUG("N_reads: ");
       DEBUG(n_reads);
       DEBUG("Read delay: ");
       DEBUG(read_delay);
       DEBUG("Plate delay: ");
       DEBUG(plate_delay);
       EEPROM.write(6,n_reads);
       EEPROM.write(7,read_delay);
       EEPROM.write(8,payload[4]*16+payload[5]);
       EEPROM.write(9,payload[6]*16+payload[7]);
       break;
    case STOP:         // 's'
       resetFunc();
       break;
    case SEND_DATA:    // 'D'
       // Each data set is 396 bytes but the max that can be sent is 44 bytes at a time through XBee. 
       // Send each resistance as 9 44 (396 bytes total) byte sets.
       Serial.println("Sending data");
       Serial.write((byte*) resistance0,364);
       Serial.println();
       Serial.write((byte*) resistance1,364);
       Serial.println();
       Serial.write((byte*) resistance2,364);
       Serial.println();
       Serial.write((byte*) resistance3,364);
       Serial.println();
       for(r=0;r<4;r++){
         Serial.print("Resistance: ");
         Serial.println(r);
         for(int k=0;k<3;k++){
           for(j = 0; j<132;j++){
             switch(r){
               case 0:
                  payload[j] = resistance0[k*132+j];
                  break;
               case 1:
                  payload[j] = resistance1[k*132+j];
                  break;
               case 2:
                  payload[j] = resistance2[k*132+j];
                  break;
               case 3:
                  payload[j] = resistance3[k*132+j];
                  break;
             }
           }
           success = radio.send_bytes(payload,132);
           Serial.print(success);
           Serial.write(payload,132);
           Serial.println();
         }
       }
       delay(100);
       Serial.println("Successfully sent data");
       data_sent++;
       Serial.print("Data sent:");
       Serial.println(data_sent);
       break;
    case WAITING:
       while(!radio.send_byte(CONNECT));
       break;
    default:
       while(!radio.send_byte(0));
       break;
  }
}

