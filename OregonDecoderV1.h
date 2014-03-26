// http://wmrx00.sourceforge.net/Arduino/OregonScientific-RF-Protocols.pdf
// new version !! http://www.osengr.org/WxShield/Downloads/OregonScientific-RF-Protocols-II.pdf
// bit rate 342Hz reverse manchester
// => 342Hz in microseconds => 2924
// pulse lengthened by 255 microseconds
class OregonDecoderV1 : public DecodeOOK {
public:
    OregonDecoderV1() {}

    virtual char decode (word width) {
    //width = microseconds
    //page 6 of protocol pdf
        if (970 <= width && width < 3400) {
            byte w = width >= 700;
            switch (state) {
                case UNKNOWN:
                    if (w == 0)
                        ++flip;
                    else if (10 <= flip && flip <= 50) {
                        flip = 1;
                        manchester(1);
                    } else
                        return -1;
                    break;
                case OK:
                    if (w == 0)
                        state = T0;
                    else
                        manchester(1);
                    break;
                case T0:
                    if (w == 0)
                        manchester(0);
                    else
                        return -1;
                    break;
            }
            return 0;
        }
        if (width >= 2500 && pos >= 9)
            return 1;
        return -1;
    }
};