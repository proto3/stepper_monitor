#ifndef TM1637_H
#define TM1637_H
#include <Arduino.h>

class TM1637
{
public:
    static void setup(uint8_t clk_pin, uint8_t dio_pin, uint8_t brightness);
    static void write(char a, char b, char c, char d);

    //method to be called by the interrupt, don't use it
    static void handleInterrupt();
private:
    static uint8_t digits[];
    static uint8_t state[4];
    static uint8_t value[];
    static uint8_t _clk_pin, _dio_pin, _brightness;

    static void start_interrupt();

    static void write_block(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth);
    static bool writeValue(uint8_t value);
    static void start();
    static void stop();
};

#endif
