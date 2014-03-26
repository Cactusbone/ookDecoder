// http://wmrx00.sourceforge.net/Arduino/OregonScientific-RF-Protocols.pdf
// new version !! http://www.osengr.org/WxShield/Downloads/OregonScientific-RF-Protocols-II.pdf
// bit rate 342Hz reverse manchester
// => 342Hz in microseconds => 2924
// pulse lengthened by 255 microseconds
class OregonDecoderV1{
protected:
    enum { PREAMBLE, SYNC, PAYLOAD, DONE };
    byte bit;
    byte state;
    byte nibble_pos;
    byte pos;
    byte data[8];//in nibbles
    byte previousState;
public:
    OregonDecoderV1() {
        resetDecoder();
    }

    void resetDecoder(){
        pos=0;
        state=PREAMBLE;
        previousState=0;
        nibble_pos=0;
    }

    bool nextPulse (word width) {
        switch (state){
            case PREAMBLE:
                //full preamble is 12 "1" bits
                bit=getPreambleOrPayload(width);
                if(bit==1) {
                    pos++;
                    if(pos==12) {
                        state=SYNC;
                        pos=0;
                    }
                } else {
                    //wrong preamble
                    resetDecoder();
                }
            break;
            case SYNC :
                 //sync is 4200 off then 5700 on then 5000 off
                if(pos==0&&width>=4000&&width<=4400) {
                    pos=1;
                } else if(pos==1&&width>=5585&&width<=5985) {
                    pos=2;
                } else if(pos==2&&width>=5000&&width<=5400) {
                    state=PAYLOAD;
                    pos=0;
                    pushData(1);
                } else if(pos==2&&width>=6480&&width<=6880) {
                    state=PAYLOAD;
                    pos=0;
                    pushData(0);
                } else {
                    //wrong timing
                    resetDecoder();
                }
            break;
            case PAYLOAD:
                //payload if 8*4 bits
                //when done
                bit=getPreambleOrPayload(width);
                if(bit>-1){
                    pushData(bit);
                    return isDone();
                }
            break;
        }
        return false;
    }

    bool isDone () const { return state == DONE; }

    void pushData(byte value){
        previousState=value;
        pos++;

        byte *ptr = data + nibble_pos;
        *ptr = (*ptr >> 1) | (value << 7);

        if (pos >= 4) {
            pos = 0;
            nibble_pos++;
        }
        if(nibble_pos == 8)
            state = DONE;
    }

    const byte* getData (byte& count) const {
        count = nibble_pos;
        return data;
    }

    byte getPreambleOrPayload (word width) {
        if(previousState==0 && width >=970 && width<1950)
        {//short pulse => 0
           previousState=0;
           return 0;
        }else if(previousState==0 && width >=1950 && width<2900){
           previousState=1;
           return 1;
        }else if(previousState==1 && width >=1500 && width<2400){
           previousState=0;
           return 0;
        }else if(previousState==1 && width >=2400 && width<3400){
           previousState=1;
           return 1;
        }else{
            //wrong status
           resetDecoder();
           return -1;
        }
    }
};
