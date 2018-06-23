#include <SPI.h>
#include "arduino8x8ledmatrix.h"
#include "Adafruit_GFX.h"

//git clone git@github.com:adafruit/Adafruit-GFX-Library.git
//ln -s ~/git/Adafruit-GFX-Library/ ~/Arduino/libraries

const int pwmPin=6; //doesn't work with pin 9 on STM32 Nucleo
Arduino8x8LedMatrix matrix(1,1,10, pwmPin, false);

unsigned long ulTimer=0;
static const unsigned char heart[8]={0x00,0x6c,0xfe,0xfe,0x7c,0x38,0x10,0x00}; //designed with https://www.riyas.org/2013/12/online-led-matrix-font-generator-with.html

void setup()
{
    matrix.begin();
    matrix.fillScreen(matrix.BLACK);
    matrix.drawBitmap(0,0,heart,8,8,matrix.RED);
    ulTimer=millis();
}

void loop()
{
    if(millis()>ulTimer+10)
    {
        ulTimer=millis();
        //https://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
        matrix.setBrightness(27882.4 * (exp(sin(millis()/2000.0*PI)) - 0.36787944));

    }
}
