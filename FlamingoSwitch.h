/* ---------------------------------------------------------------------------
   Flamingo Switch Library - v3.0
   Created by Karl-Heinz Wind - karl-heinz.wind@web.de
   Copyright 2015 License: GNU GPL v3 http://www.gnu.org/licenses/gpl-3.0.html
   https://github.com/windkh/flamingoswitch
---------------------------------------------------------------------------*/

#ifndef FLAMINGO_SWITCH_H
#define FLAMINGO_SWITCH_H

#include "Arduino.h"

// Number of maximum High/Low changes per packet.
// We can handle up to (unsigned long) => 32 bit * 2 H/L changes per bit + 2 for sync + timing[0]
// 32Bit-->67
#define FLAMINGO_MAX_CHANGES 32 * 2 + 2 + 1 

// The remote control repeates the same code 4 times.
const uint8_t DEFAULT_RETRIES = 4;

class FlamingoSwitch
{

public:
  FlamingoSwitch();

  void enableTransmit(int nTransmitterPin);
  void disableTransmit();

  bool available() const;
  void resetAvailable();

  unsigned long getReceivedValue() const;
  unsigned int getReceivedBitlength() const;
  unsigned int getReceivedDelay() const;
  unsigned int* getReceivedRawdata() const;

  void enableReceive(int interrupt);
  void enableReceive();
  void disableReceive();

  void send(uint32_t code, uint8_t retries = DEFAULT_RETRIES);

  static void decrypt(uint32_t input, uint16_t& receiverId, uint8_t& value, uint8_t& rollingCode, uint16_t& transmitterId);
  static uint32_t encrypt(uint8_t receiverId, uint8_t value, uint8_t rollingCode, uint16_t transmitterId);

private:

  inline void send0();
  inline void send1();
  inline void sendSync();
  inline void transmit(int nHighPulses, int nLowPulses);
  
  static void handleInterrupt();
  static bool receiveProtocol(unsigned int changeCount);

  static int nReceiveTolerance;
  static unsigned long nReceivedValue;
  static unsigned int nReceivedBitlength;
  static unsigned int nReceivedDelay;

  /*
  * timings[0] contains sync timing, followed by a number of bits
  */
  static unsigned int timings[FLAMINGO_MAX_CHANGES];

  int nTransmitterPin;
  int nPulseLength;

  int nReceiverInterrupt;
};

#endif
