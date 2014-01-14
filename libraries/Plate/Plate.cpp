
#include "Arduino.h"
#include "Plate.h"
#include "Maxim.h"
#include "LMP.h"
#include "Reporter.h"
#include "SdFat.h"
#include <EEPROM.h>

Plate::Plate(): _resistor_mux(0, 0, 0) {
   n_reads = EEPROM.read(6);
   read_delay = EEPROM.read(7);
}

void Plate::set_channels(int UA, int HC, int HB, int HA, 
                         int VC, int VB, int VA) {
   _channels[0] = VA;
   _channels[1] = VB;
   _channels[2] = VC;
   _channels[3] = HA;
   _channels[4] = HB;
   _channels[5] = HC;
   _channels[6] = UA;

   for (int i=0; i<7; i++) {
      pinMode(_channels[i],OUTPUT);
   }
}

void Plate::set_well(int well) {
   byte well_key = WELL_KEYS[well];
   for (int i=0; i<7; i++) {
      digitalWrite(_channels[i],(well_key >> i) & 0x1);
   }
}

void Plate::set_resistor_mux_channels(int A, int B, int C) {
   _resistor_mux = Maxim(A,B,C);
}
  
void Plate::set_resistor(int channel) {
   _resistor_mux.setChannel(EEPROM.read(channel+1));
}

float Plate::read_voltage() {
   float total_reads = 0.0;
   for (int i=0; i<n_reads; i++) {
      total_reads += LMP.read_adc_float();
      delay(read_delay);
   }
   return total_reads / (float) n_reads;
}

void Plate::read_voltages() {
   byte key;
   for (int well=0; well<96; well++) {
      set_well(well);
      delay(10);
      Serial.print(well);
      Serial.print(":");
      voltage = read_voltage();
      Reporter.add_record(voltage, well);
      Serial.println(voltage);
      }
}