// 2012-09-06 - Oregon V2 decoder modified - Olivier Lebrun <olivier.lebrun@connectingstuff.net>
// 2012-06-21 - Oregon V2 decoder added - Dominique Pierre (zzdomi)
// 2012-06-21 - Oregon V3 decoder revisited - Dominique Pierre (zzdomi)
// New code to decode OOK signals from weather sensors, etc.
// 2010-04-11 Jean-Claude Wippler <jcw@equi4.com>

//Todo merge back from https://github.com/jcw/jeelib/tree/master/examples/RF12/ookRelay2

//http://playground.arduino.cc/Code/HomeEasy

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();
int incomingByte = 0;   // for incoming serial data


#include "DecodeOOK.h"
#include "CrestaDecoder.h"
#include "HezDecoder.h"
#include "KakuDecoder.h"
#include "OregonDecoderV1.h"
#include "OregonDecoderV2.h"
#include "OregonDecoderV3.h"
#include "XrfDecoder.h"
#include "EMxDecoder.h"
#include "FSxDecoder.h"
#include "KSxDecoder.h"
#include "VisonicDecoder.h"

//433
OregonDecoderV1 orscV1;
OregonDecoderV2 orscV2;
OregonDecoderV3 orscV3;
CrestaDecoder cres;
KakuDecoder kaku;
XrfDecoder xrf;
HezDecoder hez;
//868
//VisonicDecoder viso;
//EMxDecoder emx;
//KSxDecoder ksx;
//FSxDecoder fsx;

#define PORT 2

volatile word pulse;

void ext_int_1(void) {
    static word last;
    // determine the pulse length in microseconds, for either polarity
    pulse = micros() - last;
    last += pulse;
}

void reportSerial (const char* s, class DecodeOOK& decoder) {
    byte pos;
    const byte* data = decoder.getData(pos);
    Serial.print(s);
    Serial.print(' ');
    for (byte i = 0; i < pos; ++i) {
        Serial.print(data[i] >> 4, HEX);
        Serial.print(data[i] & 0x0F, HEX);
    }
    
    // Serial.print(' ');
    // Serial.print(millis() / 1000);
    Serial.println();
    
    decoder.resetDecoder();
}

void reportSerialV1 (const char* s, class OregonDecoderV1& decoder) {
    byte pos;
    const byte* data = decoder.getData(pos);
    Serial.print(s);
    Serial.print(' ');
    for (byte i = 0; i < pos; ++i) {
        Serial.print(data[i], HEX);
    }

    // Serial.print(' ');
    // Serial.print(millis() / 1000);
    Serial.println();

    decoder.resetDecoder();
}

void setup () {
    Serial.begin(9600);
    Serial.println("[ookDecoder]");

   attachInterrupt(1, ext_int_1, CHANGE);

   mySwitch.enableTransmit(9);  // Using Pin #9
}

void loop () {
    static int i = 0;
    cli();
    word p = pulse;
    
    pulse = 0;
    sei();

    //if (p != 0){ Serial.print(++i); Serial.print('\n');}

    //433Mhz
    if (p != 0) {
        //Serial.print("[pulse]");
        //Serial.print(p);
        //Serial.println();
        if (orscV1.nextPulse(p))
            reportSerialV1("OSV1", orscV1);
        if (orscV2.nextPulse(p))
            reportSerial("OSV2", orscV2);  
        if (orscV3.nextPulse(p))
            reportSerial("OSV3", orscV3);        
        if (cres.nextPulse(p))
            reportSerial("CRES", cres);        
        if (kaku.nextPulse(p))
            reportSerial("KAKU", kaku);        
        if (xrf.nextPulse(p))
            reportSerial("XRF", xrf);        
        if (hez.nextPulse(p))
            reportSerial("HEZ", hez);        
    }

    //868Mhz
//    if (p != 0) {
//        if (viso.nextPulse(p))
//            reportSerial("VISO", viso);
//        if (emx.nextPulse(p))
//            reportSerial("EMX", emx);
//        if (ksx.nextPulse(p))
//            reportSerial("KSX", ksx);
//        if (fsx.nextPulse(p))
//            reportSerial("FSX", fsx);
//    }
    if (Serial.available() > 0) {
        Serial.println("sending");
        // read the incoming byte:
        incomingByte = Serial.read();
        mySwitch.send(5393, 24);
        Serial.println("sent");
    }
}

