#include "arduino8x8ledmatrix.h"
#include "sprites.h"
#include "morphbitmap.h"
#include "glcdfont.c"

const int pwmPin=6; //doesn't work with pin 9 on STM32 Nucleo
Arduino8x8LedMatrix matrix(2,2,10, pwmPin, false);
MorphBitmap bmp(256);
byte morphBmp[8];
byte charA[8];
byte charB[8];

void setup()
{
    Serial.begin(9600);
    while(!Serial)
    {

    }
    Serial.println("started");
    matrix.begin();
    bmp.startMorph(test[0],test[1],8,8);
    matrix.fillScreen(matrix.BLACK);
    char2bitMap('1',morphBmp);
    matrix.drawBitmap(0,0, morphBmp, 8, 8, matrix.RED);
}

void showMorph()
{
    static unsigned long ulTimer=0;
    static byte state=0;
    static byte item=0;
    const byte maxSteps=8;
    if(millis()>ulTimer+150)
    {
        ulTimer=millis();
        bmp.getNextStep(morphBmp, state, maxSteps);
        matrix.fillScreen(matrix.BLACK);
        matrix.drawBitmap(0,0, morphBmp, 8, 8, matrix.RED);
        state=(state==maxSteps+1?0:state+1);
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

void char2bitMap(char c, byte* bitmap)
{
    for(byte col=0;col<5;col++)
    {
        for(byte row=0;row<8;row++)
        {
            bitWrite(bitmap[row], 7-col, bitRead(font[c*5+col],row));
        }
    }
}

void showMorphChars()
{
    static unsigned long ulTimer=0;
    static byte state=0;
    static char c='0';
    const byte maxSteps=8;
    if(millis()>ulTimer+100)
    {
        if(!state)
        {
            delay(300);
            char2bitMap(c++,charA);
            char2bitMap(c,charB);
            bmp.startMorph(charA, charB, 8, 8);
        }
        ulTimer=millis();
        bmp.getNextStep(morphBmp, state, maxSteps);
        matrix.fillScreen(matrix.BLACK);
        matrix.drawBitmap(0,0, morphBmp, 8, 8, matrix.RED);
        state=(state==maxSteps+1?0:state+1);
    }
}

void loop()
{
    showMorphChars();
}


