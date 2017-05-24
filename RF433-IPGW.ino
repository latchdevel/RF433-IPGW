/* 

RF 433 MHz IP Gateway  
Copyright (C) 2017 - jorge.rivera@11paths.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <avr/wdt.h>              // avr-libc are licensed with a Modified BSD License, GPL-compatible. http://www.nongnu.org/avr-libc/LICENSE.txt
#include <SPI.h>                  // GNU General Public License version 2 or the GNU Lesser General Public License version 2.1
#include <Ethernet.h>             // WIZnet W5100 Ethernet library license: GNU Lesser General Public License

#include "aREST.h"                // Copyright (c) 2014-2016 Marco Schwartz
                                  // The MIT License (MIT)
                                  // https://github.com/marcoschwartz/aREST

#include "FlamingoSwitch.h"       // Created by Karl-Heinz Wind - karl-heinz.wind@web.de
                                  // Copyright 2015 License: GNU GPL v3
                                  // https://github.com/windkh/flamingoswitch
                                  
#include "NewRemoteTransmitter.h" // Made by Randy Simons http://randysimons.nl/
                                  // License: GNU GPL v3. 
                                  // https://bitbucket.org/fuzzillogic/433mhzforarduino
                                  
#include "RemoteTransmitter.h"    // Copyright 2010 Randy Simons. All rights reserved.
                                  // License: GNU GPL v3. 
                                  // https://bitbucket.org/fuzzillogic/433mhzforarduino
                                  
/*
  Flamingo 500: /fla?params= { on / off / dim } . { controller_id } . { device_id } . [ dim_level ]

  controller_id: [0..2^16-1] Address of transmitter. Duplicate the address of your hardware, or choose a random number. 
      device_id: [0..254]    Target device unit.
      dim_level: [0..254]    Dim level. 0 for off, 254 for brightest level.

  Sample http://192.168.1.177/fla?params=off.39202.3 
   
  NewKaku: /dio?params= { on / off / dim } . { controller_id } . { device_id } . [ dim_level ]

  controller_id: [0..2^26-1] Address of transmitter. Duplicate the address of your hardware, or choose a random number. 
      device_id: [0..15]     Target device unit.
      dim_level: [0..15]     Dim level. 0 for off, 15 for brightest level.

  Sample http://192.168.1.177/dio?params=dim.16241666.1.7

  FHT-7901: /fht?params= { on / off } . { controller_id } . { device_id } 

  controller_id: 5 characters as binary string from "00000" to "11111"  (0..31)
      device_id: 1 char value of [ 'A','B','C','D','E' ] (0..5) 

  Sample http://192.168.1.177/fht?params=on.10011.b

 */

#define VERSION 13  
 
IPAddress       ip( 192, 168,   1, 177);
IPAddress   subnet( 255, 255, 255,   0);
IPAddress  gateway( 192, 168,   0,   1);
IPAddress dnServer( 192, 168,   0,   1);

unsigned int webport = 80;

unsigned int TX_PIN = 3;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

int OldPulseLength = 196; // FHT-7901, from Kjell & Company 
int OldRepeats     =   4; // Default = 4; 

// Create Flamingo instance
FlamingoSwitch faSwitch;

// Create EthernetServer instance
EthernetServer server(webport);

// Create aREST instance
aREST rest = aREST();

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) { ; }

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id(F("e"));        // VERSION (hex)
  rest.set_name(F("rf433gw"));

  // Function to be exposed
  rest.function((char*)"dio",NewKakuTX);
  
  // Function to be exposed
  rest.function((char*)"fla",FlamingoTX);

  // Function to be exposed
  rest.function((char*)"fht",FhtTX);

  // Configure Flamingo Switch 
  faSwitch.enableTransmit(TX_PIN);
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip, dnServer, gateway, subnet);
  server.begin();
  Serial.print(F("Server listen at http://"));
  Serial.print(Ethernet.localIP());
  Serial.print(F(":")); 
  Serial.print(webport,DEC);
  Serial.println(F("/"));

  // Start watchdog
  wdt_enable(WDTO_4S);
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  rest.handle(client);
  wdt_reset();
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// Custom function accessible by the API for DIO devices
int NewKakuTX(String command) { // http://192.168.1.177/dio?params=on.16241666.0

  int r = 0;
  unsigned long    code_id =  0 ; // Address of this transmitter [0..2^26-1] Duplicate the address of your hardware, or choose a random number.
  unsigned short device_id = 16 ; // [0..15] target unit.
  unsigned short dim_level = 16 ; // [0..15] Dim level. 0 for off, 15 for brightest level.
  String               arg = "" ;
 
  Serial.print(F("NewKakuTX(")); Serial.print(command); Serial.print(F(") "));

  String cmd = getValue(command, '.', 0);

  if (cmd.length()>0){
     arg = getValue(command, '.', 1);
     if (arg.length()>0){
         code_id = arg.toInt();
         arg = getValue(command, '.', 2);
         if (arg.length()>0){
            device_id = arg.toInt();
            arg = getValue(command, '.', 3);
            if (arg.length()>0){
              dim_level = arg.toInt();
            }
         }else{
          // no arg2 
          r = -3;
         }
     }else{
       // no arg1
       r = -2;
     }
  }else{
    // no cmd
    r = -1;
  }

  if (r>=0 and device_id < 16){
    Serial.print(F("CONTROLLER ID "));
    Serial.print(code_id,DEC);
    Serial.print(F(" DEVICE ID "));
    Serial.print(device_id,DEC);
    Serial.print(F(" "));    
    if ( cmd == String(F("on")) ){
      Serial.print(F("ON "));
      NewRemoteTransmitter(code_id, TX_PIN).sendUnit(device_id, true);
    }else if ( cmd == String(F("off")) ){
      Serial.print(F("OFF "));
      NewRemoteTransmitter(code_id, TX_PIN).sendUnit(device_id, false);
    }else if ( cmd == String(F("dim")) ){
      Serial.print(F("DIM "));
      if (dim_level < 16 ){
        Serial.print(dim_level,DEC);
        Serial.print(F(" "));
        NewRemoteTransmitter(code_id, TX_PIN).sendDim(device_id, dim_level);
      }else{
         // dim_level = > 16
         r = -6;
      }
    }else{
      // no valid cmd 
      r = -5; 
    }
  }else{
    // device_id = > 16
    r = -4;
  }

  if (r<0){
    Serial.print(F("ERROR "));
    Serial.print(r,DEC);
  }
  
  Serial.println();
  return r;

}

// Custom function accessible by the API for Flamingo devices
int FlamingoTX(String command) { // http://192.168.1.177/flamingotx?params=on.39202.3

  int r = 0;
  uint16_t  code_id =   0 ; // 
  uint8_t device_id = 254 ; // 
  uint8_t dim_level = 254 ; // 
  String        arg = ""  ;
 
  Serial.print(F("FlamingoTX(")); Serial.print(command); Serial.print(F(") "));

  String cmd = getValue(command, '.', 0);

  if (cmd.length()>0){
     arg = getValue(command, '.', 1);
     if (arg.length()>0){
         code_id = arg.toInt();
         arg = getValue(command, '.', 2);
         if (arg.length()>0){
            device_id = arg.toInt();
            arg = getValue(command, '.', 3);
            if (arg.length()>0){
              dim_level = arg.toInt();
            }
         }else{
          // no arg2 
          r = -3;
         }
     }else{
       // no arg1
       r = -2;
     }
  }else{
    // no cmd
    r = -1;
  }

  if (r>=0 and device_id < 254){
    Serial.print(F("CONTROLLER ID "));
    Serial.print(code_id,DEC);
    Serial.print(F(" DEVICE ID "));
    Serial.print(device_id,DEC);
    Serial.print(F(" "));    
    if ( cmd == String(F("on")) ){
      Serial.print(F("ON "));
      faSwitch.send(faSwitch.encrypt(device_id, 1, 1, code_id));
      faSwitch.send(faSwitch.encrypt(device_id, 1, 2, code_id));
    }else if ( cmd == String(F("off")) ){
      Serial.print(F("OFF "));
      faSwitch.send(faSwitch.encrypt(device_id, 0, 1, code_id));
      faSwitch.send(faSwitch.encrypt(device_id, 0, 2, code_id));
    }else if ( cmd == String(F("dim")) ){
      Serial.print(F("DIM "));
      if (dim_level < 16 ){
        Serial.print(dim_level,DEC);
        Serial.print(F(" "));
        faSwitch.send(faSwitch.encrypt(device_id, dim_level, 1, code_id));
        faSwitch.send(faSwitch.encrypt(device_id, dim_level, 2, code_id));
      }else{
         // dim_level = > 16
         r = -6;
      }
    }else{
      // no valid cmd 
      r = -5; 
    }
  }else{
    // device_id = > 16
    r = -4;
  }

  if (r<0){
    Serial.print(F("ERROR "));
    Serial.print(r,DEC);
  }
  
  Serial.println();
  return r;

}

// Custom function accessible by the API for FHT-7901 devices
int FhtTX(String command) {  // http://192.168.1.177/fht?params=on.11111.b

  int r = 0;
  String   code_str = "" ;
  String device_str = "" ;

  Serial.print(F("FhtTX("));Serial.print(command);Serial.print(F(") "));
  
  String cmd = getValue(command, '.', 0);

  if (cmd.length()>0){
     code_str = getValue(command, '.', 1);
     if (code_str.length()==5){
         device_str = getValue(command, '.', 2);
         if (device_str.length()!=1){
          // no arg2 
          r = -3;
         }
     }else{
       // no arg1
       r = -2;
     }
  }else{
    // no cmd
    r = -1;
  }

  int i = 0;                     
  unsigned short systemCode = 0;
  while (i < 5 and r == 0){
    if ( code_str.charAt(i) == '1' or code_str.charAt(i) == '0' ){  
      if ( code_str.charAt(i) == '1') bitSet(systemCode,4-i); 
      i++;
    }else{
      // no binary code_str
      r = -6; 
    }
  }
  
  if (r>=0 ){
    device_str.toUpperCase();
    if ( device_str < "A" or device_str > "E" ){
      // out of range
      r = -7; 
    }
  }

  if (r>=0 ){
    Serial.print(F("CONTROLLER '"));
    Serial.print(code_str);
    Serial.print(F("' DEVICE '"));
    Serial.print(device_str);
    Serial.print(F("' ("));    
    Serial.print(systemCode);
    Serial.print(F(") "));    
    if ( cmd == String(F("on")) ){
      Serial.print(F("ON "));
      ActionTransmitter(TX_PIN,OldPulseLength,OldRepeats).sendSignal(systemCode,device_str.charAt(0),true);
    }else if ( cmd == String(F("off")) ){
      Serial.print(F("OFF "));
      ActionTransmitter(TX_PIN,OldPulseLength,OldRepeats).sendSignal(systemCode,device_str.charAt(0),false);
    }else{
      // no valid cmd 
      r = -5; 
    }
  }

  if (r<0){
    Serial.print(F("ERROR "));
    Serial.print(r,DEC);
  }
  
  Serial.println();
  return r;

}

