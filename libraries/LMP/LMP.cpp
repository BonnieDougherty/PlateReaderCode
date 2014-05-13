
#include "Arduino.h"
#include "SPI.h"
#include "LMP.h"

#define DEBUG 0

void show_byte(byte b) {
  for (int i=0; i<8; i++) {
    if ((b >> (7-i)) & 1) {
      break;
    } else {
      Serial.print("0");
    }
  }
  if (b) {
   Serial.print(b,BIN);
  }
}

long convert_24bit_to_32bit_2cs(byte* data) {
   // the AVR is little endian
   // the read from the LMP is big endian
   byte complement = data[0] >> 7;
   long value = 0L;
   byte* bytes = (byte*) &value;
   bytes[0] = data[2];
   bytes[1] = data[1];
   bytes[2] = data[0];
   bytes[3] = complement ? byte(0xFF) : byte(0x0);
   return value;
}

LMPClass LMP;
int LMPClass::ss = 0;

void LMPClass::begin(int slave_select) {
   //SPI.setClockDivider(SPI_CLOCK_DIV128);
   LMPClass::ss = slave_select;
   pinMode(ss,OUTPUT);
   digitalWrite(LMPClass::ss,HIGH);
}

void LMPClass::end() {
   SPI.end();
}

void LMPClass::assert_csb() {
   digitalWrite(LMPClass::ss,LOW);
}

void LMPClass::deassert_csb() {
   digitalWrite(LMPClass::ss,HIGH);
}

void LMPClass::make_transaction1(byte inst1, byte uab) {
   assert_csb();
   SPI.transfer(inst1);
   SPI.transfer(uab);
   deassert_csb();

   if (DEBUG) {
      Serial.print("Transaction 1:  ");
      show_byte(inst1);
      show_byte(uab);
      Serial.println("");
   }
}

void LMPClass::make_transaction2(byte inst2, byte* data) {
   assert_csb();
   byte read = inst2 >> 7;
   int n = ((int) ((inst2 >> 5) & 0b11)) + 1;
   SPI.transfer(inst2);
   for (int i=0; i<n; i++) {
      if (read) {
         data[i] = SPI.transfer(byte(0));
      } else {
         SPI.transfer(data[i]);
      }
   }
   deassert_csb();

   if (DEBUG) {
      Serial.print("Transaction 2:  ");
      show_byte(inst2);
      for (int i=0; i<n; i++) {
         show_byte(data[i]);
      }
      Serial.println("");
   }
}

void LMPClass::readwrite_bytes(byte read, byte address, byte* data, int n) {
   //byte inst1 = read ? 0x90 : 0x10;
   byte inst1 = 0x10;  // always write the URA
   byte uab = URA(address);
   make_transaction1(inst1, uab);
   byte inst2 = read << 2;
   inst2 |= byte(n) - 1;
   inst2 <<= 5;
   inst2 |= LRA(address);
   make_transaction2(inst2, data);
}

void LMPClass::read_bytes(byte address, byte* data, int n) {
   readwrite_bytes(byte(1), address, data, n);
}

void LMPClass::write_bytes(byte address, byte* data, int n) {
   readwrite_bytes(byte(0), address, data, n);
}

byte LMPClass::read_byte(byte address) {
   byte data;
   read_bytes(address, &data, 1);
   return data;
}

void LMPClass::write_byte(byte address, byte data) {
   write_bytes(address, &data, 1);
}

long LMPClass::read_adc_long() {
   long value = 0L;
   byte data[3];
   read_bytes(ADC_DOUT, data, 3);
   value = convert_24bit_to_32bit_2cs(data);
   return value;
}

float LMPClass::read_adc_float() {
   float valuef = 0.0;
   float valuel = read_adc_long();
   valuef = (float) valuel / 8388608L;
   return valuef;
}

float LMPClass::read_adc_float_nonnegative() {
   float value = read_adc_float();
   return value < 0.0 ? 0.0 : value;
}

void LMPClass::quick_start() {
   // set gain = 1; CH0_CONFIG:BUF_EN = 1
   LMP.write_byte(CH0_CONFIG, 0b01110001);
   
   // set the background to BgcalMode2; BGCALCN = 0x2
   LMP.write_byte(BGCALCN, 0x2);

   // select VREF1; CH0_INPUTCN:VREF_SEL = 0
   LMP.write_byte(CH0_INPUTCN, 0b00000001);

   // use internal clock; CLK_EXT_DET = 1 and CLK_SEL = 0
   LMP.write_byte(ADC_AUXCN, 0b00100000);
}

