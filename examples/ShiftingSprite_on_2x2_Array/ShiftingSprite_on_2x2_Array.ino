#include <SPI.h>
#include "arduino8x8ledmatrix.h"
#include "Adafruit_GFX.h"
#include "Fonts/TomThumb.h"
#include "Fonts/Tiny3x3a2pt7b.h"
#include "Fonts/Picopixel.h"

//git clone git@github.com:adafruit/Adafruit-GFX-Library.git
//ln -s ~/git/Adafruit-GFX-Library/ ~/Arduino/libraries

const int pwmPin=6; //doesn't work with pin 9 on STM32 Nucleo
Arduino8x8LedMatrix matrix(2,2,10, pwmPin, false);
const char str[] PROGMEM = "Adafruit 16x32 RGB LED Matrix";

void setup() {
    Serial.begin(9600);
    while(!Serial)
    {

    }
    Serial.println("started");
    matrix.begin();
    matrix.setTextWrap(false); // Allow text to run off right edge
    matrix.setTextSize(1);
    matrix.setTextColor(matrix.RED);
    //matrix.drawLine(0,0,7,7,matrix.RED);
    //matrix.drawCircle(40, 3, 3, matrix.RED);
    //matrix.setFont(&TomThumb);
    //matrix.setFont(&Tiny3x3a2pt7b);
    //matrix.setFont(&Picopixel);
    //matrix.drawChar(0, 7, 'a',matrix.RED, matrix.BLACK, 1);
}


void counter()
{
    static unsigned long ultime=millis();
    static byte ctr=0;
    if(millis()>ultime+500)
    {
        ultime=millis();
        matrix.fillScreen(matrix.BLACK);
        matrix.drawChar(6,6,'0'+ctr,matrix.RED, matrix.BLACK,1);
        //matrix.drawPixel(8,8,matrix.RED);
        ctr=ctr<9 ? ctr+1 : 0;
        Serial.println(ctr);
    }
}

void loop() {
    counter();
}
