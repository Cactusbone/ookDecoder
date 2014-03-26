// bit rate 1024Hz normal manchester
// => 1024Hz in microseconds => 977
// pulse shortened by 93 microseconds

//this looks like v3 while v3 looks like v2
class OregonDecoderV2 : public DecodeOOK {
public:
    OregonDecoderV2() {}

    // add one bit to the packet data buffer
    virtual void gotBit (char value) {
        if(!(total_bits & 0x01))
        {
            data[pos] = (data[pos] >> 1) | (value ? 0x80 : 00);
        }
        total_bits++;
        pos = total_bits >> 4;
        if (pos >= sizeof data) {
            resetDecoder();
            return;
        }
        state = OK;
    }

    virtual char decode (word width) {
        //between 200 microseconds and 1200 microseconds
        //page 5 of pdf.
        //however this does not really match, seems like a bit of simplification to me. meaning we could be losing data
        if (200 <= width && width < 1200) {
            byte w = width >= 700;//1 if greater than 700
            switch (state) {
                case UNKNOWN://should detect message start
                    //shouldn't we check for at least 16 long pulse to accept preamble ?
                    //this looks like not really nice...
                    if (w != 0) {
                        // Long pulse
                        ++flip;
                    } else if (w == 0 && 24 <= flip) {
                        // preamble is 16 bits in v2.1 sensors, 24 in v3
                        // Short pulse, start bit
                        flip = 0;
                        state = T0;
                    } else {
                      // Reset decoder
                        return -1;
                    }
                    break;
                case OK:
                    if (w == 0) {
                        // Short pulse
                        state = T0;
                    } else {
                        // Long pulse
                        manchester(1);
                    }
                    break;
                case T0:
                    if (w == 0) {
                      // Second short pulse
                        manchester(0);
                    } else {
                        // manchester coding format violation
                        // Reset decoder
                        return -1;
                    }
                    break;
            }
        } else if (width >= 2500  && pos >= 8) {
                return 1;//DONE
        } else {
            return -1;//RESET
        }
        return total_bits == 160 ? 1: 0;//DONE OR should continue
    }
};