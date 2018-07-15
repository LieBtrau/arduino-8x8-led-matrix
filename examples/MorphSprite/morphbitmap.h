#pragma once
#include "Arduino.h"

/* Based on the idea of https://github.com/cbm80amiga/oled_morphing_clock
 *
 * Working principle:
 *  make a list of active pixels in the original bitmap => src.listPixXY
 *  make a list of active pixels in the destination bitmap => dest.listPixXY
 *  map each pixel of src.listPixXY to a pixel in dest.listPixXY
 *  calculate the trajectory from src.listPixXY to dest.listPixXY for each pixel
 */

class MorphBitmap
{
public:
    MorphBitmap(int maxPixelsPerBitmap);
    ~MorphBitmap();
    typedef enum
    {
        ONE_TO_ONE,
        REVERSE,
        MAX_MODE
    }MORPHMODE;
    bool startMorph(const byte* srcBitMap, const byte* dstBitMap, byte width, byte height);
    //make sure dstBitMap continues to exist during morphing, this function doesn't make a local copy.
    //steps is number of intermediates steps to morph srcBitmap into dstBitmap
    bool getNextStep(byte* morphBitmap, byte curStep, byte maxSteps);
    void setMorphMode(MORPHMODE mm);
private:
    typedef struct
    {
       byte* bitmap=nullptr;
       byte* listPixXY=nullptr;
       int pixCnt=0;
    }BITMAP;
    int buildPixelCoordinateList(const byte* bitmap, BITMAP* bmp);
    int _maxPixelsPerBitmap;
    BITMAP _src, _dst;
    byte _curStep=0;
    byte _width;
    byte _height;
    byte _steps=0;
    MORPHMODE _morphMode;
};
