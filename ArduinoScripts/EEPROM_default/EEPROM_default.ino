#include <EEPROM.h>

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

void setup()
{
  pinMode(53, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  pinMode(A10, OUTPUT);
  digitalWrite(A10, HIGH);
  
  for(int i = 0; i < 4096; i++) EEPROM.write(i, 0);
  
  // 1-4 are for the four resistors (178, 221, 301, 402 ohms).
  EEPROM.write(1,2);
  EEPROM.write(2,1);
  EEPROM.write(3,0);
  EEPROM.write(4,3);
  EEPROM.write(5,0);
  EEPROM.write(6,25);
  EEPROM.write(7,2);
  EEPROM.write(8,0);
  EEPROM.write(9,120);
  Serial.begin(9600);
  Serial.println("Done");
}

void loop()
{
  
}
