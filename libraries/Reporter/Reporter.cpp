#include "Arduino.h"
#include "SdFat.h"
#include "Reporter.h"

ReporterClass Reporter;
byte ReporterClass::record_bytes[RECORD_BYTES];
char ReporterClass::record_chars[RECORD_CHARS];
SdFat ReporterClass::sd;
SdFile ReporterClass::PlateFile;
SdFile ReporterClass::SentFile;

#define error(x) sd.errorHalt_P(PSTR(x))

char hex_digits[] = {'0','1','2','3','4','5','6','7',
                     '8','9','A','B','C','D','E','F'};

void ReporterClass::bytes_to_hex_string(byte* bytes, char* chars, int n) {
   for (int i=0; i<n; i++) {
      chars[2*i] = hex_digits[bytes[i] >> 4];
      chars[2*i+1] = hex_digits[bytes[i] & 0xF];
   }
}

void ReporterClass::create_record(long time, byte resistance) {
   byte* ptr = (byte*) &time;
   for (int i=0; i<4; i++) {
      record_bytes[i] = ptr[i];
   }
   record_bytes[4] = resistance;
   // bytes_to_hex_string(record_bytes, record_chars, 5);
   // write_SD((byte*)record_chars,10);
}

void ReporterClass::add_record(float voltage, int well)
{
   unsigned long mOD = 0L;
   byte* ptr = (byte*) &mOD;
   mOD = (unsigned long) (1000L*voltage);
   for(int j=0; j<2; j++)
   {
      // record_bytes[j] = ptr[j];
      record_bytes[5+2*well+j] = ptr[j];
   }
   // bytes_to_hex_string(record_bytes, record_chars,2);
   // write_SD((byte*)record_chars,4);
}

void ReporterClass::finish_record()
{
   bytes_to_hex_string(record_bytes, record_chars, RECORD_BYTES);
   write_SD((byte*) &record_chars, RECORD_CHARS);
   Serial.write((byte*) &record_chars,RECORD_CHARS);
   PlateFile.println();
   PlateFile.sync();
}

// ================ SD card ==================

void ReporterClass::start_experiment()
{
   if(!PlateFile.open("PLATE.TXT", O_RDWR)) error("Open PLATE.TXT failed");
   if(!SentFile.open("SENT.TXT", O_RDWR)) error("Open SENT.TXT failed");
}

void ReporterClass::start_SD(int chip_select) {
   pinMode(chip_select, OUTPUT);
   digitalWrite(chip_select, HIGH);
   if(!sd.begin(chip_select, SPI_FULL_SPEED)) sd.initErrorHalt();
   return;
}

void ReporterClass::write_SD(byte* to_be_written, int n) {
   PlateFile.write(to_be_written, n);
   PlateFile.sync();
}

long ReporterClass::SD_length(){
   return PlateFile.fileSize();
}

byte ReporterClass::SD_read()
{
   return PlateFile.read();
}

void ReporterClass::SD_seek(long location)
{
   PlateFile.seekSet(location);
}

void ReporterClass::record_sent(long last_sent)
{
   SentFile.println(last_sent);
   SentFile.sync();
}

int ReporterClass::read_sent()
{
   long length = SD_length();
   if(length == 0) return 0;
   if(length > 4) PlateFile.seekSet(length-4);
   byte upper = PlateFile.read();
   byte lower = PlateFile.read();
   return 10*upper + lower;
}

void ReporterClass::stop_SD()
{
   PlateFile.close();
   SentFile.close();
}
// ================ Serial ==================

void ReporterClass::write_serial() {
   Serial.write((byte*) record_chars, RECORD_CHARS);
   Serial.println();
   Serial.flush();
}