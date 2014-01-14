
#include "Arduino.h"
#include "SdFat.h"

#ifndef Reporter_h
#define Reporter_h

// records from a plate reader are stored as a single binary stream:
//
//    bytes  field      format
//    -----  -----      ------
//    0:3    timestamp  long int (two's complement, signed)
//    4      resistance byte 0x1 - 0x8
//    5:196  OD reads   2 byte unsigned integer in units of milli-ODs (mOD)
//                      (0xFF mOD = 0.255 OD)
//
// For transmission via HTTP or storage in a file, each record is encoded
// as a 394-character string.  Each 2 characters represents a hex encoding
// of a single byte in the record.  All byte patterns are little endian.

#define RECORD_BYTES 197
#define RECORD_CHARS 2 * RECORD_BYTES

class ReporterClass {
   public:
      static byte record_bytes[RECORD_BYTES];
      static char record_chars[RECORD_CHARS];
      static void bytes_to_hex_string(byte* bytes, char* chars, int n);
      static void create_record(long time, byte resistance);
      static void add_record(float voltage, int well);
      static void finish_record();
      static void record(byte* data);

      static void start_experiment();
      static void start_SD(int chip_select);
      static void write_SD(byte* to_be_written, int n);
      static void stop_SD();
      static int open_SD_write();
      static long SD_length();
      static byte SD_read();
      static void SD_seek(long location);
      static void record_sent(long last_send);
      static int read_sent();

      static void write_serial();

   private:
      static SdFile PlateFile;
      static SdFile SentFile;
      static SdFat sd;
};

extern ReporterClass Reporter;

#endif

