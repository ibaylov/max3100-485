#include <MAX3100-485.h>


/*
* MIT License
* 
* Copyright (c) 2021 Ivaylo Baylov, ibaylov@gmail.com
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/



SPISettings
MAX3100485::gm_spiSet( 40000000, MSBFIRST, SPI_MODE0 );

MAX3100485*
MAX3100485::gm_pIRQHandler = nullptr;

static void
IRQHandler( )
{
    if( MAX3100485::gm_pIRQHandler )
        MAX3100485::gm_pIRQHandler->SetRX();
}



////////////////////////////////////////////////////////////////
/// MAX3100 register constants
///


#define MAX_WRITE_CONFIG 0b1100000000000000
#define MAX_READ_CONFIG  0b0100000000000000
#define MAX_WRITE_DATA   0b1000000000000000
#define MAX_READ_DATA    0b0000000000000000

#define MAX_CONFIG_FEN   0b0010000000000000
#define MAX_CONFIG_SHDI  0b0001000000000000
#define MAX_CONFIG_TM    0b0000100000000000
#define MAX_CONFIG_RM    0b0000010000000000
#define MAX_CONFIG_PM    0b0000001000000000
#define MAX_CONFIG_RAM   0b0000000100000000
#define MAX_CONFIG_IR    0b0000000010000000
#define MAX_CONFIG_ST    0b0000000001000000
#define MAX_CONFIG_PE    0b0000000000100000
#define MAX_CONFIG_L     0b0000000000010000

#define MAX_CONFIG_R     0b1000000000000000
#define MAX_CONFIG_T     0b0100000000000000

#define MAX_CONFIG_RTS   0b0000001000000000
#define MAX_CONFIG_CTS   0b0000001000000000
#define MAX_CONFIG_TE    0b0000010000000000
#define MAX_CONFIG_TEST  0b0000000000000001

MAX3100485::MAX3100485( uint8_t nPINIRQ, uint8_t nPINSS, bool bClock2X )
    : m_nPINIRQ( nPINIRQ), m_nPINSS( nPINSS ), m_bClock2X( bClock2X )
{

}

MAX3100485::MAX3100485( )
    : MAX3100485( 2, 10, false )
{

}

MAX3100485::~MAX3100485( )
{

}

void
MAX3100485::begin( uint32_t nRate, int nBits, bool bParity, int nStopBits )
{
    m_bDataRX = false;
    uint16_t uBaudRate = 0;
    if( m_bClock2X )
        nRate /= 2;
    switch( nRate )
    {
    case 300:
        uBaudRate ++;
    case 600:
        uBaudRate ++;
    case 1200:
        uBaudRate ++;
    case 2400:
        uBaudRate ++;
    case 4800:
        uBaudRate ++;
    case 9600:
        uBaudRate ++;
    case 19200:
        uBaudRate ++;
    case 38400:
        uBaudRate ++;
    case 900:
        uBaudRate ++;
    case 1800:
        uBaudRate ++;
    case 3600:
        uBaudRate ++;
    case 7200:
        uBaudRate ++;
    case  14400:
        uBaudRate ++;
    case  28800:
        uBaudRate ++;
    case  57600:
        uBaudRate ++;
    case 115200:
        default:
        ;
    }
    pinMode(m_nPINSS, OUTPUT);
    digitalWrite(m_nPINSS, HIGH);

    SPI.begin();
    SPI.usingInterrupt(digitalPinToInterrupt(m_nPINIRQ));
    pinMode(m_nPINIRQ, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(m_nPINIRQ), IRQHandler, LOW);
    gm_pIRQHandler = this;

    if( uBaudRate == 0 )
        nRate = 115200;
    if( m_bClock2X )
        nRate *= 2;

    // calculate the caracter duration in usecs
    uint32_t uScale = nBits + 1 + nStopBits ; // start bit + stop bits
    if( bParity )
        uScale ++;
    uScale += 5;
    uScale *= 16 * 1000000UL;
    uScale /= nRate;
    uScale >>= 4;
    m_nCharUS = static_cast<uint16_t>(uScale);
    //
    Serial.print( "Character delay, uSec: " );
    Serial.println( m_nCharUS );
    Serial.print( "RTU gap, uSec: " );
    Serial.println( ((uint32_t)36 * m_nCharUS)/10 );
    //
    m_uConfig = uBaudRate;
    if( nBits < 8 )
        m_uConfig |= MAX_CONFIG_L;
    if( bParity )
        m_uConfig |= MAX_CONFIG_PE;
    if( nStopBits > 1 )
        m_uConfig |= MAX_CONFIG_ST;

    // FIFO is enabled bu default, so we don't do anything special about it
    //m_uConfig |= MAX_CONFIG_FEN;
    m_uConfig |= MAX_CONFIG_RM;

    SPITxRxSingle( MAX_WRITE_CONFIG | m_uConfig );
}

uint16_t
MAX3100485::GetCharUS() const
{
    return m_nCharUS;
}

bool
MAX3100485::GetRX() const
{
    return m_bDataRX;
}

void
MAX3100485::SetRX( )
{
    m_bDataRX = true;

}

void
MAX3100485::ClearRX()
{
    m_bDataRX = false;
}

bool
MAX3100485::available()
{
    return GetRX();
}

void
MAX3100485::test()
{
    SPITxRxSingle( MAX_READ_CONFIG  | MAX_CONFIG_TEST );
}

void
MAX3100485::SPIBegin()
{
    SPI.beginTransaction(gm_spiSet);
}

void
MAX3100485::SPIEnd()
{
    SPI.endTransaction();
}

uint16_t
MAX3100485::SPITxRx( uint16_t uTX )
{
    digitalWrite(m_nPINSS, LOW);
    uint16_t uRX = SPI.transfer16( uTX );
    digitalWrite(m_nPINSS, HIGH);
    return uRX;
}

uint16_t
MAX3100485::SPITxRxSingle( uint16_t uTX )
{
    SPI.beginTransaction(gm_spiSet);
    digitalWrite(m_nPINSS, LOW);
    uint16_t uRX = SPI.transfer16( uTX );
    digitalWrite(m_nPINSS, HIGH);
    SPI.endTransaction();
    return uRX;
}

size_t
MAX3100485::write( const void* pvData, size_t nData )
{
    SPIBegin();
    // enable the RTS bit but don't output
    SPITxRx( MAX_WRITE_DATA | MAX_CONFIG_RTS | MAX_CONFIG_TE );
    //
    size_t nOut = nData;
    const uint8_t* pbOut = static_cast<const uint8_t*>(pvData);
    uint16_t uRV;
    while( nOut )
    {
        SPITxRx(MAX_WRITE_DATA | MAX_CONFIG_RTS | *pbOut  );
        do
                uRV = SPITxRx(MAX_READ_CONFIG);
        while( !( uRV & MAX_CONFIG_T ));
        pbOut ++;
        nOut --;
    }
    // wait the last character to be transmitted
    delayMicroseconds(GetCharUS());
    // clear the RTS bit but don't output
    SPITxRx( MAX_WRITE_DATA | MAX_CONFIG_TE );
    ClearRX();
    SPIEnd();
    return nData;
}

size_t
MAX3100485::readRaw(  void* pvData, size_t nData )
{
    bool bRV = false;
    size_t nOut = _read( pvData, nData, false, bRV );
    return nOut;
}

bool
MAX3100485::read(  void* pvData, size_t nData )
{
    bool bRV = false;
    _read( pvData, nData, true, bRV );
    return bRV;
}

size_t MAX3100485::_read(void *pvData, size_t nData, bool bEntire, bool& bResult)
{
    SPIBegin();
    uint8_t* pbIn = static_cast<uint8_t*>(pvData);
    uint16_t uRV;
    uint32_t uGuardInerval = static_cast<uint32_t>(3 * m_nCharUS) / 1500 + 1;
    uint32_t uGuard = millis() + uGuardInerval;
    size_t cData;
    for( cData = 0; cData < nData; cData ++ )
    {
        // timeout check here
        do
        {
            if( millis() > uGuard )
            {
                if( bEntire )
                {
                    ClearRX();
                    SPIEnd();
                    bResult = false;
                    return 0;
                }
                else
                    break;
            }
            uRV = SPITxRx(MAX_READ_CONFIG);
        }
        while( !( uRV & MAX_CONFIG_R ));
        uGuard = millis() + uGuardInerval;

        uRV = SPITxRx(MAX_READ_DATA);
        //
        *pbIn = static_cast<uint8_t>(uRV);
        pbIn ++;
    }
    // chain this for the next iters
    uRV = SPITxRx(MAX_READ_CONFIG);
    m_bDataRX = ( uRV & MAX_CONFIG_R );
    SPIEnd();
    bResult = true;
    return cData;
}
