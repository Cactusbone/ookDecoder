// http://wmrx00.sourceforge.net/Arduino/OregonScientific-RF-Protocols.pdf
// new version !! http://www.osengr.org/WxShield/Downloads/OregonScientific-RF-Protocols-II.pdf
// bit rate 342Hz reverse manchester
// => 342Hz in microseconds => 2924
// pulse lengthened by 255 microseconds
class OregonDecoderV1{
protected:
    enum { OK, NODATA, ERROR };
    enum { PREAMBLE, SYNC, PAYLOAD, DONE };
    byte bit;
    byte mode;
    byte nibble_pos;
    byte pos;
    byte data[8];//in nibbles
    byte halftime;
public:
    OregonDecoderV1() {
        resetDecoder();
    }

    void resetDecoder(){
        if(mode != PREAMBLE)
        {
            Serial.println("RESET");
        }
        pos=0;
        mode=PREAMBLE;
        nibble_pos=0;
        halftime=0;
    }

    bool nextPulse (word width, int state) {
        switch (mode){
            case PREAMBLE:
                if(pos>=12 && width>=4000&&width<=4400) {
                    //sync !
                   Serial.print("SYNC 1 - ");
                   Serial.println(pos);
                   pos=1;
                   mode=SYNC;
                   return false;
                }

                //full preamble is 12 "1" bits

                if(pos>=12) {
                   Serial.print("PREAMBLE - ");
                   Serial.print(pos);
                   Serial.print(" - ");
                   Serial.print(width);
                   Serial.print(" - ");
                   Serial.print(state);
                   Serial.println();
                }
                bit=getPreambleOrPayload(width, state);
                if(bit==NODATA){
                    //no data, continue as usual
                    if(pos>=12)
                        Serial.println("NODATA");
                } else if(bit==OK && state==HIGH) {
                    pos++;
                } else {
                    if(pos>=12)
                        Serial.println("wrong preamble");
                    //wrong preamble
                    resetDecoder();
                }
            break;
            case SYNC :
                Serial.println("Begin Sync");
                 //sync is 4200 off then 5700 on then 5000 off
                if(pos==0&&width>=4000&&width<=4400) {
                    Serial.println("SYNC 1");
                    pos=1;
                } else if(pos==1&&width>=5585&&width<=5985) {
                    Serial.println("SYNC 2");
                    pos=2;
                } else if(pos==2&&width>=5000&&width<=5400) {
                    Serial.println("SYNC 3 - 1");
                    mode=PAYLOAD;
                    pos=0;
                    halftime=0;
                    pushData(1);
                } else if(pos==2&&width>=6480&&width<=6880) {
                    Serial.println("SYNC 3 - 0");
                    mode=PAYLOAD;
                    pos=0;
                    halftime=0;
                    pushData(0);
                } else {
                    Serial.print("FAIL - ");
                    Serial.println(width);
                    //wrong timing
                    resetDecoder();
                }
            break;
            case PAYLOAD:
                Serial.print("Payload - ");
                Serial.println(pos);

                //payload if 8*4 bits
                //when done
                bit=getPreambleOrPayload(width, state);
                if(bit==NODATA){
                    //no data, continue as usual
                } else if(bit==OK){
                    pushData(state==HIGH?1:0);
                    return isDone();
                }
            break;
        }
        return false;
    }

    bool isDone () const { return mode == DONE; }

    void pushData(byte value){
        pos++;
        Serial.print("pushData - ");
        Serial.println(value);
        byte *ptr = data + nibble_pos;
        *ptr = (*ptr >> 1) | (value << 7);

        if (pos > 4) {
            pos = 0;
            nibble_pos++;
        }
        if(nibble_pos == 8)
            mode = DONE;
    }

    const byte* getData (byte& count) const {
        count = nibble_pos;
        return data;
    }

    // 0 => should keep, 1 => no error, but no value, -1 => error
    byte getPreambleOrPayload(word width, int state) {
        bool shortPulse = false;
        //state is the ending RF state, so check the other
        if(state==HIGH && width >=970 && width<1950) {
        //short pulse
           shortPulse = true;
        } else if(state==HIGH && width >=1950 && width<2900){
        //long pulse
        } else if(state==LOW && width >=1500 && width<2400){
        //short pulse
            shortPulse = true;
        } else if(state==LOW && width >=2400 && width<3400){
        //long pulse
        } else{
            if(mode==PAYLOAD)    {
                Serial.print("payload failed - ");
                Serial.print(state==LOW);
                Serial.print(" - ");
                Serial.println(width);
            }

            //wrong status
           resetDecoder();
           return ERROR;
        }

        if(shortPulse){
            halftime++;
            if(halftime%2==1)
            //was even before the increment, then value should be kept
                return OK;
            else
                return NODATA;
        } else {
            if(halftime%2==1){
               //manchester violation
               if(mode==PAYLOAD)    {
                   Serial.print("payload failed - manchester violation - ");
                   Serial.println(width);
               }
                resetDecoder();
                return ERROR;
            } else {
                halftime += 2;
                return OK;
            }
        }
    }
};
