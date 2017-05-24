# RF433-IPGW
TCP/IP Arduino Gateway to RF 433MHz AM transmitter over several ASK/OOK protocols

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

# Includes
```c
#include <avr/wdt.h>              // avr-libc are licensed with a Modified BSD License, GPL-compatible.
#include <SPI.h>                  // Licenses GNU GPL version 2 or GNU LGPL version 2.1
#include <Ethernet.h>             // WIZnet W5100 Ethernet library license: GNU LGPL

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
```                                  
# Usage

### Flamingo 500: 
```  
  /fla?params= { on / off / dim } . { controller_id } . { device_id } . [ dim_level ]

  controller_id: [0..2^16-1] Address of transmitter. Duplicate the address of your hardware, or choose a random number. 
      device_id: [0..254]    Target device unit.
      dim_level: [0..254]    Dim level. 0 for off, 254 for brightest level.

  Sample: http://192.168.1.177/fla?params=off.39202.3 
```   
### NewKaku: 
```
  /dio?params= { on / off / dim } . { controller_id } . { device_id } . [ dim_level ]

  controller_id: [0..2^26-1] Address of transmitter. Duplicate the address of your hardware, or choose a random number. 
      device_id: [0..15]     Target device unit.
      dim_level: [0..15]     Dim level. 0 for off, 15 for brightest level.

  Sample: http://192.168.1.177/dio?params=on.16241666.0
```   
### FHT-7901:  
```
  /fht?params= { on / off } . { controller_id } . { device_id } 

  controller_id: 5 characters as binary string from "00000" to "11111"  (0..31)
      device_id: 1 char value of [ 'A','B','C','D','E' ] (0..5) 

  Sample: http://192.168.1.177/fht?params=on.11111.b
```
 
