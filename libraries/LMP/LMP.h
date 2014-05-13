
#ifndef LMP_h
#define LMP_h

#include "Arduino.h"

// Registers
#define RESETCN 0x00
#define SPI_HANDSHAKECN 0x01
#define SPI_RESET 0x02
#define SPI_STREAMCN 0x03
#define PWRCN 0x08
#define DATA_ONLY_1 0x09
#define DATA_ONLY_2 0x0A
#define ADC_RESTART 0x0B
#define GPIO_DIRCN 0x0E
#define GPIO_DAT 0x0F
#define BGCALCN 0x10
#define SPI_DRDYBCN 0x11
#define ADC_AUXCN 0x12
#define SPI_CRC_CN 0x13
#define SENDIAG_THLD 0x14
#define SENDIAG_THLDH 0x14
#define SENDIAG_THLDL 0x15
#define SCALCN 0x17
#define ADC_DONE 0x18
#define SENDIAG_FLAGS 0x19
#define ADC_DOUT 0x1A
#define ADC_DOUTH 0x1A
#define ADC_DOUTM 0x1B
#define ADC_DOUTL 0x1C
#define SPI_CRC_DAT 0x1D

#define CH_STS 0x1E
#define CH_SCAN 0x1F
#define CH0_INPUTCN 0x20
#define CH0_CONFIG 0x21
#define CH1_INPUTCN 0x22
#define CH1_CONFIG 0x23
#define CH2_INPUTCN 0x24
#define CH2_CONFIG 0x25
#define CH3_INPUTCN 0x26
#define CH3_CONFIG 0x27
#define CH4_INPUTCN 0x28
#define CH4_CONFIG 0x29
#define CH5_INPUTCN 0x2A
#define CH5_CONFIG 0x2B
#define CH6_INPUTCN 0x2C
#define CH6_CONFIG 0x2D

#define CH0_SCAL_OFFSET 0x30
#define CH0_SCAL_OFFSETH 0x30
#define CH0_SCAL_OFFSETM 0x31
#define CH0_SCAL_OFFSETL 0x32
#define CH0_SCAL_GAIN 0x33
#define CH0_SCAL_GAINH 0x33
#define CH0_SCAL_GAINM 0x34
#define CH0_SCAL_GAINL 0x35
#define CH0_SCAL_SCALING 0x36
#define CH0_SCAL_BITS_SELECTOR 0x37

#define CH1_SCAL_OFFSET 0x38
#define CH1_SCAL_OFFSETH 0x38
#define CH1_SCAL_OFFSETM 0x39
#define CH1_SCAL_OFFSETL 0x3A
#define CH1_SCAL_GAIN 0x3B
#define CH1_SCAL_GAINH 0x3B
#define CH1_SCAL_GAINM 0x3C
#define CH1_SCAL_GAINL 0x3D
#define CH1_SCAL_SCALING 0x3E
#define CH1_SCAL_BITS_SELECTOR 0x3F

#define CH2_SCAL_OFFSET 0x40
#define CH2_SCAL_OFFSETH 0x40
#define CH2_SCAL_OFFSETM 0x41
#define CH2_SCAL_OFFSETL 0x42
#define CH2_SCAL_GAIN 0x43
#define CH2_SCAL_GAINH 0x43
#define CH2_SCAL_GAINM 0x44
#define CH2_SCAL_GAINL 0x45
#define CH2_SCAL_SCALING 0x46
#define CH2_SCAL_BITS_SELECTOR 0x47

#define CH3_SCAL_OFFSET 0x48
#define CH3_SCAL_OFFSETH 0x48
#define CH3_SCAL_OFFSETM 0x49
#define CH3_SCAL_OFFSETL 0x4A
#define CH3_SCAL_GAIN 0x4B
#define CH3_SCAL_GAINH 0x4B
#define CH3_SCAL_GAINM 0x4C
#define CH3_SCAL_GAINL 0x4D
#define CH3_SCAL_SCALING 0x4E
#define CH3_SCAL_BITS_SELECTOR 0x4F

#define URA(x) ((x >> 4) & 0b111)
#define LRA(x) (x & 0b1111)

class LMPClass {
   private:
      byte null_data[3];
      static int ss;
      static void readwrite_bytes(byte read, byte address, byte* data, int n);
      static void make_transaction1(byte inst1, byte uab);
      static void make_transaction2(byte inst2, byte* data);
      static void assert_csb();
      static void deassert_csb();
   public:
      static void begin(int slave_select);
      static void read_bytes(byte address, byte* data, int n);
      static void write_bytes(byte address, byte* data, int n);
      static byte read_byte(byte address);
      static void write_byte(byte address, byte data);
      static long read_adc_long();
      static float read_adc_float();
      static float read_adc_float_nonnegative();
      static void end();
      static void quick_start();
};

extern LMPClass LMP;

#endif

