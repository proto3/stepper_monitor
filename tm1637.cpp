const int CLK = 2;
const int DIO = 3;

uint8_t digits[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x40};
uint8_t state[4] = {0};
uint8_t value[] = {0,0x40,0,0,0xc0,0x4f, 0x66, 0x6d, 0x7d};

void setup()
{
  pinMode(CLK, OUTPUT);
  pinMode(DIO, OUTPUT);

  start();
  writeValue(0x88|7);
  stop();

  // clear display
  write(0x00, 0x00, 0x00, 0x00);

  setup_timer();
}


unsigned long time = 0; // 86390000;

void loop()
{
    value[5] = digits[(time/1000)%10];
    value[6] = digits[(time/100)%10];
    value[7] = digits[(time/10)%10];
    // value[8] = digits[time%10];
    value[8] = digits[10];
    write_digit();
    delay(10);
    time++;
}


void setup_timer()
{
    //disable timer2 interuption
    TIMSK2 &= ~(1 << OCIE2A);

    TCCR2A = 0;// set entire TCCR0A register to 0
    TCCR2B = 0;// same for TCCR0B
    TCNT2  = 0;//initialize counter value to 0
    // set compare match register for 2khz increments
    OCR2A = 10;// = (16*10^6) / (200000*8) - 1 (must be <256)
    // turn on CTC mode
    TCCR2A |= (1 << WGM21);
    // Set CS01 bits for 8 prescaler
    TCCR2B |= (1 << CS21);
}

void write_digit()
{
    TIMSK2 |= (1 << OCIE2A);
}

ISR(TIMER2_COMPA_vect)
{
    switch(state[0])
    {
        case 0:
        case 3:
            // start
            switch(state[1])
            {
                case 0:
                    digitalWrite(CLK,HIGH);
                    digitalWrite(DIO,HIGH);
                    state[1]++;
                    break;
                case 1:
                    digitalWrite(DIO,LOW);
                    digitalWrite(CLK,LOW);
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
                            digitalWrite(CLK, LOW);
                            state[3]++;
                            break;
                        case 1:
                            digitalWrite(DIO, (value[state[0]] & (1 << state[2])) >> state[2]);
                            state[3]++;
                            break;
                        case 2:
                            digitalWrite(CLK, HIGH);
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
                    digitalWrite(CLK,LOW);
                    state[1]++;
                    break;
                case 2:
                    digitalWrite(CLK,HIGH);
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
                    digitalWrite(CLK,LOW);
                    digitalWrite(DIO,LOW);
                    state[1]++;
                    break;
                case 1:
                    digitalWrite(CLK,HIGH);
                    digitalWrite(DIO,HIGH);
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

void write(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth)
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

void start(void)
{
  digitalWrite(CLK,HIGH);
  digitalWrite(DIO,HIGH);
  delayMicroseconds(5);

  digitalWrite(DIO,LOW);
  digitalWrite(CLK,LOW);
  delayMicroseconds(5);
}

void stop(void)
{
  digitalWrite(CLK,LOW);
  digitalWrite(DIO,LOW);
  delayMicroseconds(5);

  digitalWrite(CLK,HIGH);
  digitalWrite(DIO,HIGH);
  delayMicroseconds(5);
}

bool writeValue(uint8_t value)
{
  for(uint8_t i = 0; i < 8; i++)
  {
    digitalWrite(CLK, LOW);
    delayMicroseconds(5);
    digitalWrite(DIO, (value & (1 << i)) >> i);
    delayMicroseconds(5);
    digitalWrite(CLK, HIGH);
    delayMicroseconds(5);
  }

  // wait for ACK
  digitalWrite(CLK,LOW);
  delayMicroseconds(5);

  digitalWrite(CLK,HIGH);
  delayMicroseconds(5);


  return 0;
}

