#! /usr/bin/python

from xbee import ZigBee
import serial

BROADCAST = '\x00\x00\x00\x00\x00\x00\xFF\xFF'
UNKNOWN = '\xFF\xFE'
PORT = '/dev/ttyAMA0'
BAUD_RATE = 9600

ser = serial.Serial(PORT,BAUD_RATE)
xbee = ZigBee(ser,escaped=True)

def handlePacket(data):
    print data

if __name__ == '__main__':
    # Code to be executed when file is called directly
    xbee.send("at",command="MY")
    print(xbee.wait_read_frame())
    xbee.halt()
    ser.close()
    
