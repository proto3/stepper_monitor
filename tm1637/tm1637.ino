#include "TM1637.h"

void setup()
{
    TM1637::setup(2, 3, 7);
}

unsigned long time = 0;

void loop()
{
    int a = time / 1000 % 10;
    int b = time / 100 % 10;
    int c = time / 10 % 10;
    int d = time % 10;
    TM1637::write(a+48, b+48, c+48, d+48);

    time++;
    delay(100);
}
