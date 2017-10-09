#include "TM1637.h"

uint8_t TM1637::state[4] = {0};
uint8_t TM1637::value[] = {0,0x40,0,0,0xc0,0x4f, 0x66, 0x6d, 0x7d};

uint8_t TM1637::_clk_pin;
uint8_t TM1637::_dio_pin;
uint8_t TM1637::_brightness;

uint8_t convert(char c)
{
    switch(c)
    {
        case '0':
            return 0x3f;
        case '1':
            return 0x06;
        case '2':
            return 0x5b;
        case '3':
            return 0x4f;
        case '4':
            return 0x66;
        case '5':
            return 0x6d;
        case '6':
            return 0x7d;
        case '7':
            return 0x07;
        case '8':
            return 0x7f;
        case '9':
            return 0x6f;

        // case 'a':
        //     return 0x5f;
        // case 'b':
        //     return 0x7c;
        // case 'c':
        //     return 0x58;
        // case 'd':
        //     return 0x5e;
        // case 'e':
        //     return 0x7b;
        // case 'f':
        //     return 0x71;
        // case 'g':
        //     return 0x6f;
        // case 'h':
        //     return 0x74;
        // case 'i':
        //     return 0x06;
        // case 'j':
        //     return 0x0e;
        // case 'l':
        //     return 0x06;
        // case 'o':
        //     return 0x5c;
        // case 'p':
        //     return 0x73;
        // case 't':
        //     return 0x78;

        case '-':
            return 0x40;
        default:
            return 0;
    }
}
//----------------------------------------------------------------------------//
void TM1637::setup(uint8_t clk_pin, uint8_t dio_pin, uint8_t brightness)
{
    _clk_pin = clk_pin;
    _dio_pin = dio_pin;
    _brightness = brightness;

    pinMode(_clk_pin, OUTPUT);
    pinMode(_dio_pin, OUTPUT);

    //------------------------------------
    //TODO clean block
    start();
    writeValue(0x88|7);
    stop();
    // clear display
    write_block(0x40, 0x40, 0x40, 0x40);
    //------------------------------------

    //disable timer2 interuption
    TIMSK2 &= ~(1 << OCIE2A);

    TCCR2A = 0;// set entire TCCR2A register to 0
    TCCR2B = 0;// same for TCCR2B
    TCNT2  = 0;//initialize counter value to 0
    // set compare match register for 2khz increments
    OCR2A = 9;// = (16*10^6) / (200000*8) - 1 (must be <256)
    // turn on CTC mode
    TCCR2A |= (1 << WGM21);
    // Set CS21 bits for 8 prescaler
    TCCR2B |= (1 << CS21);
}
//----------------------------------------------------------------------------//
void TM1637::write(char a, char b, char c, char d)
{
    value[5] = convert(a);
    value[6] = convert(b);
    value[7] = convert(c);
    value[8] = convert(d);

    start_interrupt();
}
//----------------------------------------------------------------------------//
void TM1637::start_interrupt()
{
    TIMSK2 |= (1 << OCIE2A);
}
//----------------------------------------------------------------------------//
ISR(TIMER2_COMPA_vect)
{
    TM1637::handleInterrupt();
}
//----------------------------------------------------------------------------//
void TM1637::handleInterrupt()
{
    switch(state[0])
    {
        case 0:
        case 3:
            // start
            switch(state[1])
            {
                case 0:
                    digitalWrite(_clk_pin, HIGH);
                    digitalWrite(_dio_pin, HIGH);
                    state[1]++;
                    break;
                case 1:
                    digitalWrite(_dio_pin, LOW);
                    digitalWrite(_clk_pin, LOW);
                    state[1] = 0;
                    state[0]++;
                    break;
            }
            break;
        case 1:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            // byte
            switch(state[1])
            {
                case 0:
                    switch(state[3])
                    {
                        case 0:
                            digitalWrite(_clk_pin, LOW);
                            state[3]++;
                            break;
                        case 1:
                            digitalWrite(_dio_pin, (value[state[0]] & (1 << state[2])) >> state[2]);
                            state[3]++;
                            break;
                        case 2:
                            digitalWrite(_clk_pin, HIGH);
                            state[3] = 0;
                            state[2]++;
                            break;
                    }
                    if(state[2] == 8)
                    {
                        state[2] = 0;
                        state[1]++;
                    }
                    break;
                case 1:
                    digitalWrite(_clk_pin, LOW);
                    state[1]++;
                    break;
                case 2:
                    digitalWrite(_clk_pin, HIGH);
                    state[1] = 0;
                    state[0]++;
                    break;
            }
            break;
        case 2:
        case 9:
            // stop
            switch(state[1])
            {
                case 0:
                    digitalWrite(_clk_pin, LOW);
                    digitalWrite(_dio_pin, LOW);
                    state[1]++;
                    break;
                case 1:
                    digitalWrite(_clk_pin, HIGH);
                    digitalWrite(_dio_pin, HIGH);
                    state[1] = 0;
                    state[0]++;

                    if(state[0] == 10)
                    {
                        state[0] = 0;
                        TIMSK2 &= ~(1 << OCIE2A);
                    }
                    break;
            }
            break;
    }
}
//----------------------------------------------------------------------------//
void TM1637::write_block(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth)
{
  start();
  writeValue(0x40);
  stop();

  start();
  writeValue(0xc0);
  writeValue(first);
  writeValue(second);
  writeValue(third);
  writeValue(fourth);
  stop();
}
//----------------------------------------------------------------------------//
void TM1637::start()
{
  digitalWrite(TM1637::_clk_pin,HIGH);
  digitalWrite(TM1637::_dio_pin,HIGH);
  delayMicroseconds(5);

  digitalWrite(TM1637::_dio_pin,LOW);
  digitalWrite(TM1637::_clk_pin,LOW);
  delayMicroseconds(5);
}
//----------------------------------------------------------------------------//
void TM1637::stop()
{
  digitalWrite(TM1637::_clk_pin,LOW);
  digitalWrite(TM1637::_dio_pin,LOW);
  delayMicroseconds(5);

  digitalWrite(TM1637::_clk_pin,HIGH);
  digitalWrite(TM1637::_dio_pin,HIGH);
  delayMicroseconds(5);
}
//----------------------------------------------------------------------------//
bool TM1637::writeValue(uint8_t value)
{
  for(uint8_t i = 0; i < 8; i++)
  {
    digitalWrite(TM1637::_clk_pin, LOW);
    delayMicroseconds(5);
    digitalWrite(TM1637::_dio_pin, (value & (1 << i)) >> i);
    delayMicroseconds(5);
    digitalWrite(TM1637::_clk_pin, HIGH);
    delayMicroseconds(5);
  }

  // wait for ACK
  digitalWrite(TM1637::_clk_pin,LOW);
  delayMicroseconds(5);

  digitalWrite(TM1637::_clk_pin,HIGH);
  delayMicroseconds(5);


  return 0;
}
//----------------------------------------------------------------------------//
