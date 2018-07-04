#include "arduino8x8ledmatrix.h"
#include "SPI.h"

static Arduino8x8LedMatrix *activePanel = nullptr;
static void update();

Arduino8x8LedMatrix::Arduino8x8LedMatrix(byte columnCount, byte rowCount, byte ssPin, byte dimmingPin, bool dbuf):
    _colCnt(columnCount),
    _rowCnt(rowCount),
    _SSpin(ssPin),
    _dimmingPin(dimmingPin),
    Adafruit_GFX(columnCount<<3, rowCount<<3)
{
    // Allocate and initialize matrix buffer:
    _videoBufSize = (columnCount * rowCount) << 3;
    int allocsize = _videoBufSize << (dbuf ? 1 : 0);
    if((matrixbuff[0] = (byte *)malloc(allocsize))==nullptr)
    {
        return;
    }
    // If not double-buffered, both buffers then point to the same address:
    matrixbuff[1] = dbuf ? &matrixbuff[0][_videoBufSize] : matrixbuff[0];
}

void Arduino8x8LedMatrix::begin(void)
{
    backindex   = 0;                         // Back buffer
    pinMode(_SSpin, OUTPUT);
    pinMode(_dimmingPin, PWM);
    setBrightness(0xFFFF);
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
    Timer2.setPeriod(1000); // in microseconds
    Timer2.setCompare(TIMER_CH1, 1);      // overflow might be small
    Timer2.attachInterrupt(TIMER_CH1, update);
#endif
    fillScreen(BLACK);
}

void Arduino8x8LedMatrix::drawPixel(int16_t x, int16_t y, uint16_t c)
{
    if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

    //The LED modules are arranged in rows & columns.  There's a serpentine SPI chain that runs through all the
    //LED modules.
    //We have to take into account that the byte shifted in first, is meant for the last LED module in the chain.
    //The complexity of the conversion is put in this function, so that the update routine, which will be run more
    //often can be kept simple & efficient.
    //
    //Memory arrangement is as follows
    //  MatrixBuf[0-7] = data for last module in the chain
    //  MatrixBuf[8-15] = data for next to last module in the chain
    //  ...

    //Calculate the row and column of the LED module that must be addressed
    word row = _rowCnt - 1 - (y>>3) * _colCnt; //8 rows in a module
    word col = row & 1 ? x>>3 : _colCnt-1-(x>>3); //Serpentining

    //Convert the row & column to a byte offset in the matrixBuff
    word byteOffset = ((row * _colCnt + col)<<3) + (y & 7);

    //Select the correct row of the module and set the bit corresponding to the requested column
    bitWrite(matrixbuff[backindex][ byteOffset], x & 7, c);

    //Efficiency could be increased by leaving away with the >>3 & <<3.
}

void Arduino8x8LedMatrix::fillScreen(word c)
{
    memset(matrixbuff[backindex], c==BLACK ? 0xFF : 0x00, _videoBufSize);
}

void Arduino8x8LedMatrix::swapBuffers(bool copy)
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

void Arduino8x8LedMatrix::setBrightness(word brightness)
{
    pwmWrite(_dimmingPin, 0xFFFF-brightness);
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
        SPI.transfer(buffptr[i]);
        SPI.transfer(rowIndex);
    }
    digitalWrite(_SSpin, HIGH);
    SPI.endTransaction();
    _row++;
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

void Arduino8x8LedMatrix::printBuffer()
{
    for(int i=0;i<8;i++)
    {
        Serial.print(matrixbuff[backindex][i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}
