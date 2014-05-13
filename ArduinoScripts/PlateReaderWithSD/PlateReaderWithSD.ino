 /* Problems/To-do:
- For send and get bytes, only exit function when the message was recieved. 
- Re-think flow for when Arduino loses power. 
- If Arduino loses power while writing to SD card, data can be half saved. Check to make sure data is complete.
*/

/*
Default readings parameters: 
120 seconds between reads
25 flashses per read
2 microsecond delay between wells
*/

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
int current_reads;         // Current number of reads taken for this experiment - one per resistance. 
byte n_reads;              // Preset parameter - number of reads per well. Initialized in Plate library, saved in EEPROM. 
byte read_delay;           // Preset parameter - delay between reading wells. Initialized in Plate library, saved in EEPROM.
int plate_delay;           // Preset parameter - delay between plate reads. Initialized in Plate library, saved in EEPROM. 
long last_read_time = 0;   // Keeps track of time between reads. 
// Collecting time
DateTime current_time;
long current_time_seconds;

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
  
  // Initialize SD card. 
  DEBUG("Initializing SD card...");
  Reporter.start_SD(SS_SD);
  digitalWrite(SS_SD,HIGH);
  DEBUG("Initialized.");
  
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
  
  // Wait for the ON_OFF switch to be turned to ON to start readings. 
  while(digitalRead(ON_OFF) == 0){
    delay(10);
  }
  // SD card not listening. 
  digitalWrite(SS_SD,HIGH); 
  DEBUG("Starting experiment");
  // Creates new files for PLATE.TXT and SENT.TXT
  Reporter.start_experiment();
  current_reads = 0;

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

long current_read_time;
int r;
void loop() {
  
  while(digitalRead(ON_OFF) == 1){
    if(last_read_time != 0){
      current_read_time = millis();
      while((current_read_time - last_read_time) < (long) plate_delay*1000){
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
      Reporter.finish_record();
      delay(100);
    }
    plate.set_well(96);
    Serial.println("Finished readings");
  }
}
