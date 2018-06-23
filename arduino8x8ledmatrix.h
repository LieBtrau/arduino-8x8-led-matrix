#pragma once

#pragma once
#include "Arduino.h"
#include "Adafruit_GFX.h"

class Arduino8x8LedMatrix : public Adafruit_GFX
{
public:
    typedef enum
    {
        RED = 0,        //active low LED array
        BLACK = 1
    } COLOR;
    Arduino8x8LedMatrix(byte modulesHorizontal, byte modulesVertical, byte ssPin, byte dimmingPin, bool dbuf);
    void begin();
    void drawPixel(int16_t x, int16_t y, uint16_t c);
    void fillScreen(word c);
    void updateDisplay(void);
    void swapBuffers(bool);
    byte* *backBuffer(void);
    void setBrightness(word brightNess);
private:
    void printBuffer();
    byte _modHor=0;
    byte _modVer=0;
    byte _row=0;
    byte _SSpin=0;
    byte _dimmingPin=0;
    byte* matrixbuff[2]={nullptr,nullptr};
    int _videoBufSize=0;
    volatile byte backindex=0;
    volatile boolean swapflag=false;
};
