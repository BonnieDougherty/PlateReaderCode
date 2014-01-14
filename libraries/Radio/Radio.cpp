
#include "Arduino.h"
#include "Radio.h"
#include <XBee.h>
#include <Reporter.h>

#define DEBUG(x) if (_show_debug) Serial.println(x);

Radio::Radio(): xbee(){
   _read_wait = 300;  // millis
   _max_read_waits = 10;
   _WAITING_delay = 100;
   _max_connect_tries = 10;
   _show_debug = 1;
   _connected = 0;

   //RECIEVING
   _response = XBeeResponse();
   // Packet to be recieved
   _rx = ZBRxResponse();

   // TRANSMITTING
   // DH and DL of the recieving Xbee - default is coordinator
   _addr64 = XBeeAddress64(0x00000000, 0x00000000);
   _tx = ZBTxRequest(_addr64, payload,sizeof(payload));
   _txStatus = ZBTxStatusResponse();
}

void Radio::start(){
   // Establish serial connection
   Serial3.begin(9600);
   xbee.begin(Serial3);
   delay(5000);
   while(Serial3.available()) Serial3.read();
}

void Radio::clear_serial(){
   while(Serial3.available()) Serial3.read();
}

void Radio::change_address(uint32_t msb, uint32_t lsb){
   _addr64 = XBeeAddress64(msb,lsb);
}

byte Radio::send_byte(byte byte_to_be_sent){
   uint8_t to_be_sent[] = {byte_to_be_sent};
   _tx = ZBTxRequest(_addr64,to_be_sent,1);
   xbee.send(_tx);
   
// Wait for a status response.
   if(xbee.readPacket(5000)){
      if(xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE)
      {
         xbee.getResponse().getZBTxStatusResponse(_txStatus);
         if(_txStatus.getDeliveryStatus() == 0){
            DEBUG("Successfully delivered byte");
            delay(_WAITING_delay);
            return 1;
         }
         else{
            DEBUG("Not delivered successfully");
            return 0;
         }
      }
      else {
        DEBUG("Incorrect API ID");
        // Serial.println(xbee.getResponse().getApiId(),HEX);
        clear_serial();
        return 0;
      }
   }
   else {
      DEBUG("Did not respond");
      return 0;
   }
}

byte Radio::get_byte() {
  xbee.readPacket();
  if(xbee.getResponse().isAvailable()){
    // got something
    if(xbee.getResponse().getApiId() == ZB_RX_RESPONSE){
      // got a ZB TX packet
      // now fill our ZB RX class
      xbee.getResponse().getZBRxResponse(_rx);
      if(_rx.getOption() == ZB_PACKET_ACKNOWLEDGED){
        // the sender got an ACK
        delay(_WAITING_delay);
        DEBUG("Successfully recieved byte");
        return _rx.getData(0);
      }
      else {
         // DEBUG("Delivery not confirmed?");
         return 0;
      }
    }
    else{
       // DEBUG("Not the correct response");
       return 0;
    } 
  }
  else {
     // DEBUG("No packet");
     return 0;
  }
}

int Radio::is_connected() {
   return _connected;
}

byte Radio::get_bytes(uint8_t *rec_bytes, int n) {
   _rec = get_byte();
   // Serial.println(char(_rec));
   _rec = _rec - 48;
   while(_rec == 0) _rec = get_byte();
   if(_rec != n){
      DEBUG("Number of bytes do not match");
      return 0;
   }
   DEBUG("Looking for data");
   xbee.readPacket();
   for(int i = 0; i<_max_read_waits; i++)
   {
      if(xbee.getResponse().isAvailable()){
          if(xbee.getResponse().getApiId() == ZB_RX_RESPONSE){
            xbee.getResponse().getZBRxResponse(_rx);
            if(_rx.getOption() == ZB_PACKET_ACKNOWLEDGED){
               for(int j = 0; j < n; j++){
                  rec_bytes[j] = _rx.getData(j);
               }    
               DEBUG("Successfully recieved bytes");
               return 1;
            }
            else delay(_read_wait);
          }
          else delay(_read_wait);
      }
      else delay(_read_wait);
      xbee.readPacket();
   }
   return 0;
   // DEBUG("Did not recieve bytes.");
}

byte Radio::send_bytes(uint8_t *to_be_sent, int n) {
   // while(!send_byte(n));
   _tx = ZBTxRequest(_addr64,to_be_sent,n);
   xbee.send(_tx);
   DEBUG("Sent packet");
   // Wait for a status response.
   if(xbee.readPacket(5000)){
     if(xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE){
       xbee.getResponse().getZBTxStatusResponse(_txStatus);
       if(_txStatus.getDeliveryStatus() == 0){
         DEBUG("Successfully delivered bytes");
         return 1;
       }
       else{
         DEBUG("Not successfully delivered");
         return 0; 
       }
     }
     else{
        DEBUG("Not the correct packet");
        return 0;
     }
   }
   else{
      DEBUG("Never recieved status response");
      return 0;
   } 
}

int Radio::wait_for(byte target){
   _rec = 0;
   debug_time = millis();
   for(int i = 0; i<_max_connect_tries;i++){
      _rec = get_byte();
      delay(100);
      if(_rec == target) return 1;
   }
   return 0;
}

int Radio::wait_for_connection() {
// Used on the RaspPi
   _rec = 0;
   while(!send_byte(WAITING));
   delay(5000);
   _success = wait_for(CONNECT);
   while(_success == 0){
      while(!send_byte(WAITING));
      delay(1000);
      _success = wait_for(CONNECT);
   }
   _connected = 1;
   return 0;
}

int Radio::connect() {
// Used on the arduino
   _rec = 0;
   if (is_connected()) return 0;
   for (int i=0; i<_max_connect_tries; i++) {
      _rec = get_byte();
      if (_rec == WAITING){
         while(!send_byte(CONNECT));
         return 0;
      }
      else delay(_WAITING_delay);
   }
   // failure to connect
   DEBUG("Failure to connect");
   return 1;
}



// FUNCTIONS BELOW NOT FINISHED

void Radio::send_data(int reads_to_be_sent){
   current_length = Reporter.SD_length();
   while(!send_byte(reads_to_be_sent));
   start_point = current_length - 396*reads_to_be_sent;
   Reporter.SD_seek(start_point);
   for(reads = 0; reads<reads_to_be_sent;reads++){
      for(data_set=0;data_set<9;data_set++){
         for(j=0;j<44;j++) payload[j] = Reporter.SD_read();
         while(!send_bytes(payload,44));
         wait_for(SUCCESS);
      }
   }
}

