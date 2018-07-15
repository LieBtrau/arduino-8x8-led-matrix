#include "morphbitmap.h"

MorphBitmap::MorphBitmap(int maxPixelsPerBitmap):
    _maxPixelsPerBitmap(maxPixelsPerBitmap)
{
    _src.listPixXY = (byte*)malloc(maxPixelsPerBitmap);
    _dst.listPixXY = (byte*)malloc(maxPixelsPerBitmap);
}

MorphBitmap::~MorphBitmap()
{
    if(_src.listPixXY!=nullptr)
    {
        free(_src.listPixXY);
    }
    if(_dst.listPixXY!=nullptr)
    {
        free(_dst.listPixXY);
    }
}

bool MorphBitmap::startMorph(const byte* srcBitMap, const byte *dstBitMap, byte width, byte height)
{
    if(width>16 || height>16)
    {
        return false;
    }
    _dst.bitmap=(byte*)dstBitMap;
    _width=width;
    _height=height;
    if(buildPixelCoordinateList(srcBitMap, &_src)==-1)
    {
        return false;
    }
    if(buildPixelCoordinateList(dstBitMap, &_dst)==-1)
    {
        return false;
    }
    return true;
}

void MorphBitmap::setMorphMode(MORPHMODE mm)
{
    if(mm<MAX_MODE)
    {
        _morphMode=mm;
    }
}

int MorphBitmap::buildPixelCoordinateList(const byte *bitmap, BITMAP* bmp)
{
    bmp->bitmap=(byte*)bitmap;
    bmp->pixCnt=0;
    for(byte row=0;row<_height;row++)
    {
        for(byte col=0;col<_width;col++)
        {
            if(bitRead(bitmap[(row<<((_width-1)>>3))+(col>>3)],7-(col & 0x7)))
            {
                //higher nibble contains X-coordinate
                //lower nibble contains Y-coordinate
                bmp->listPixXY[bmp->pixCnt]=(row<<4) | col;
                bmp->pixCnt++;
            }
            if(bmp->pixCnt>_maxPixelsPerBitmap)
            {
                return -1;
            }
        }
    }

//        for(byte i=0;i<bmp->pixCnt;i++)
//        {
//            Serial.print(bmp->listPixXY[i], HEX);
//            Serial.print(" ");
//        }
//        Serial.println("\r\n----");

    return bmp->pixCnt;
}

int iSrc, iDst, xSrc, ySrc, xDst, yDst, morphX, morphY;

bool MorphBitmap::getNextStep(byte* morphBitmap, byte curStep, byte maxSteps)
{
    if(curStep>maxSteps)
    {
        return false;
    }
    memset(morphBitmap, 0x00, _height << (_width > 8 ? 1 : 0));
    int maxPixCnt=max(_src.pixCnt, _dst.pixCnt);
    for(int i=0;i<maxPixCnt;i++)
    {
        iSrc = (_src.pixCnt==maxPixCnt ? i : (i*_src.pixCnt/maxPixCnt));
        iDst = (_dst.pixCnt==maxPixCnt ? i : (i*_dst.pixCnt/maxPixCnt));
        switch(_morphMode)
        {
        case ONE_TO_ONE:
            iDst = iDst;
        break;
        case REVERSE:
            iDst = _dst.pixCnt - 1 - iDst;
            break;
        default:
        break;
        }
        xSrc = _src.listPixXY[iSrc] & 0xF;
        ySrc = _src.listPixXY[iSrc] >> 4;
        xDst = _dst.listPixXY[iDst] & 0xF;
        yDst = _dst.listPixXY[iDst] >> 4;

        morphX = xSrc + ((xDst-xSrc) * curStep)/maxSteps;
        morphY = ySrc + ((yDst-ySrc) * curStep)/maxSteps;
        bitSet(morphBitmap[(morphY<<((_width-1)>>3))+(morphX>>3)], 7 - (morphX & 0x7));
    }
    return true;
}

