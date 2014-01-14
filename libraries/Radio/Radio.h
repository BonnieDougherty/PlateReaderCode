
#include "Arduino.h"
#include "XBee.h"
#include <Reporter.h>

#ifndef Radio_h
#define Radio_h

#define WAITING 'W'
#define CONNECT 'C'
#define READY 'R'
#define WANT_TO_TX 'w'
#define READY_TO_RX 'r'
#define ALL_RX 'a'
#define ERROR 'e'
#define TIMEOUT 'T'
#define PARAMETERS 'p'
#define TIME 't'
#define SENDING_TIME 'y'
#define START 'S'
#define STOP 's'
#define DATA 'd'
#define SEND_DATA 'D'
#define SUCCESS 'g'


class Radio {
   public:
      Radio();
      XBee xbee;
      byte payload[44];
      void start();
      void change_address(uint32_t msb, uint32_t lsb);
      void clear_serial();

      byte send_byte(byte byte_to_be_sent);
      byte get_byte();
      byte send_bytes(uint8_t *to_be_sent, int n);
      byte get_bytes(uint8_t *rec_bytes, int n);

      
      int wait_for_connection();
      int connect();
      int is_connected();
      int wait_for(byte target);

      void send_data(int reads_to_be_sent);

   private:
      XBeeResponse _response;
      ZBRxResponse _rx;
      XBeeAddress64 _addr64;
      ZBTxRequest _tx;
      ZBTxStatusResponse _txStatus;
      
      int _read_wait;
      int _max_read_waits;
      int _WAITING_delay;
      int _max_connect_tries;
      int _show_debug;
      int _connected;
      byte _rec;
      int _success;
      long debug_time;

      byte _length;
      byte byte_to_send[];

      long current_length;
      long start_point;
      int reads;
      int data;
      int data_set;
      int j;
};


#endif

