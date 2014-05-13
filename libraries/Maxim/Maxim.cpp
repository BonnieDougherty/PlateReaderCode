
#include "Arduino.h"
#include "Maxim.h"

Maxim::Maxim(int A, int B, int C) {
   _A = A;
   _B = B;
   _C = C;

   pinMode(_A,OUTPUT);
   pinMode(_B,OUTPUT);
   pinMode(_C,OUTPUT);
}

void Maxim::setChannel(int channel) {
   byte ch = byte(channel);
   digitalWrite(_A, ch & 0x1);
   digitalWrite(_B, (ch >> 1) & 0x1);
   digitalWrite(_C, (ch >> 2) & 0x1);
}

