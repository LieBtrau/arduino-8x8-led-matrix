#include "arduino8x8ledmatrix.h"
#include "SPI.h"

static Arduino8x8LedMatrix *activePanel = nullptr;
static void update();

Arduino8x8LedMatrix::Arduino8x8LedMatrix(byte modulesHorizontal, byte modulesVertical, byte ssPin, bool dbuf):
    _modHor(modulesHorizontal),
    _modVer(modulesVertical),
    _SSpin(ssPin),
    Adafruit_GFX(modulesHorizontal<<3, modulesVertical<<3)
{
    // Allocate and initialize matrix buffer:
    _videoBufSize = (modulesHorizontal * modulesVertical) << 3;
    int allocsize = _videoBufSize << (dbuf ? 1 : 0);
    if((matrixbuff[0] = (byte *)malloc(allocsize))==nullptr)
    {
        return;
    }
    memset(matrixbuff[0], 0, allocsize);
    // If not double-buffered, both buffers then point to the same address:
    matrixbuff[1] = dbuf ? &matrixbuff[0][_videoBufSize] : matrixbuff[0];
}

void Arduino8x8LedMatrix::begin(void)
{
    backindex   = 0;                         // Back buffer
    SPI.begin();
    activePanel = this;                      // For interrupt hander
#ifdef ARDUINO_ARCH_AVR
    // Set up Timer1 for interrupt:
    TCCR1A  = _BV(WGM11); // Mode 14 (fast PWM), OC1A off
    TCCR1B  = _BV(WGM13) | _BV(WGM12) | _BV(CS10); // Mode 14, no prescale
    ICR1    = 10000;//160Hz row refresh rate
    TIMSK1 |= _BV(TOIE1); // Enable Timer1 interrupt
    sei();                // Enable global interrupts
#elif ARDUINO_ARCH_STM32F1
    Timer2.setChannel1Mode(TIMER_OUTPUTCOMPARE);
    Timer2.setPeriod(100); // in microseconds
    Timer2.setCompare(TIMER_CH1, 1);      // overflow might be small
    Timer2.attachInterrupt(TIMER_CH1, update);
#endif
    fillscreen(BLACK);
}

void Arduino8x8LedMatrix::drawPixel(int16_t x, int16_t y, uint16_t c)
{
    if ((x < 0) || (x >= _width) || (y < 0) || (y >= _width)) return;
    bitWrite(matrixbuff[backindex][ (y>>3) * _modHor + ((y>>3) & 1 ? _modHor-1-(x>>3) : x>>3)<<3 + (y & 7)], x & 7, c);
}

void Arduino8x8LedMatrix::fillscreen(word c)
{
    memset(matrixbuff[backindex], _videoBufSize, c);
}

void Arduino8x8LedMatrix::swapBuffers(boolean copy)
{
    if(matrixbuff[0] == matrixbuff[1])
    {
        return;
    }
    // To avoid 'tearing' display, actual swap takes place in the interrupt
    // handler, at the end of a complete screen refresh cycle.
    swapflag = true;                  // Set flag here, then...
    while(swapflag == true) delay(1); // wait for interrupt to clear it
    if(copy == true)
    {
        memcpy(matrixbuff[backindex], matrixbuff[1-backindex], _videoBufSize);
    }
}


void Arduino8x8LedMatrix::updateDisplay()
{
    if(swapflag == true)
    {    // Swap front/back buffers if requested
        backindex = 1 - backindex;
        swapflag  = false;
    }
    byte* buffptr = matrixbuff[1-backindex];
    byte rowIndex = 1 << (_row & 7);
    SPI.beginTransaction(SPISettings(1000000, LSBFIRST, SPI_MODE0));
    digitalWrite(_SSpin, LOW);
    for (byte i = _row & 7; i < _videoBufSize; i+=8)
    {
        SPI.transfer(rowIndex);
        SPI.transfer(buffptr[i]);
    }
    digitalWrite(_SSpin, HIGH);
    _row++;
    SPI.endTransaction();
}

// -------------------- Interrupt handler stuff --------------------

#ifdef ARDUINO_ARCH_AVR
ISR(TIMER1_OVF_vect, ISR_BLOCK)
{ // ISR_BLOCK important -- see notes later
    TIFR1 |= TOV1;                  // Clear Timer1 interrupt flag
#elif ARDUINO_ARCH_STM32F1
void update()
{
#endif
    activePanel->updateDisplay();   // Call refresh func for active display
}


