
#ifndef Maxim_h
#define Maxim_h

#include "Arduino.h"

class Maxim {
   public:
      Maxim(int A, int B, int C);
      void setChannel(int channel);
   private:
      int _A;
      int _B;
      int _C;
};

#endif

