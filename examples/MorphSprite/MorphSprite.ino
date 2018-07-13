#include "arduino8x8ledmatrix.h"
#include "sprites.h"
#include "morphbitmap.h"

const int pwmPin=6; //doesn't work with pin 9 on STM32 Nucleo
Arduino8x8LedMatrix matrix(2,2,10, pwmPin, false);
MorphBitmap bmp(256);
byte morphBmp[8];

void setup()
{
    Serial.begin(9600);
    while(!Serial)
    {

    }
    Serial.println("started");
    matrix.begin();
    bmp.startMorph(test[0],test[1],8,8);
}

void showMorph()
{
    static unsigned long ulTimer=0;
    static byte state=0;
    static byte item=0;
    if(millis()>ulTimer+150)
    {
        ulTimer=millis();
        bmp.getNextStep(morphBmp, state, 4);
        matrix.fillScreen(matrix.BLACK);
        matrix.drawBitmap(0,0, morphBmp, 8, 8, matrix.RED);
        state=(state==9?0:state+1);
        if(!state)
        {
            item=1-item;
            bmp.startMorph(test[item],test[1-item],8,8);
        }
    }
}

void showPacman()
{
    static unsigned long ulTimer=0;
    static byte state=0;
    if(millis()>ulTimer+75)
    {
        ulTimer=millis();
        state=(state==2?0:state+1);
        matrix.fillScreen(matrix.BLACK);
        matrix.drawBitmap(4,4, monster[state], 8, 7, matrix.RED);
    }
}


void loop()
{
    showMorph();
}


