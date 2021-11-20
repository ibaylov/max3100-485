#ifndef _MA3100_485_H_
#define  _MA3100_485_H_

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

#include <Arduino.h>
#include <SPI.h>

/////////////////////////////////////////////
// @brief class MAX3100485
// driver class for MAX 3100, geared towards RS-485 usage 

class MAX3100485
{
protected:
    uint8_t   m_nPINIRQ;    //!< Interript from MAX 3100 pin index
    uint8_t   m_nPINSS;     //!< Chip/System select pin index for MAX 3100 
    bool      m_bClock2X;   //!< Clock multiplier enabled
    uint16_t  m_uConfig;    //!< MAX 3100 control register
    uint16_t  m_nCharUS;    //!< Character transmit duration in usecs; calculated during initialization

public:
    MAX3100485();
    MAX3100485( uint8_t nPINIRQ, uint8_t nPINSS, bool bClock2X ); //!< constructor, sets the IRQ and SS pins and the clock multiplier
    virtual ~MAX3100485();

    void   begin( uint32_t nRate, int nBits, bool bParity, int nStopBits ); //!< Initializes the MAX 3100 
    size_t write( const void* pvData, size_t nData );                     //!< Enables enable the RTS and outputs uo to nData bytes; returns the bytes transferred
    size_t readRaw(  void* pvData, size_t nData );                        //!< Reads up to nData bytes;  returns bytes read count
    bool   read(  void* pvData, size_t nData );                           //!< Reads exactly nData bytes; returns true on success

    bool   available();                                                   //!< Returns true if  bytes are available to be received
    void   test();                                                        //!< Enables MAX 3100 test mode 
    void   SetRX( );                                                      //!< Sets internal data available indicator
    uint16_t GetCharUS() const;                                           //!< Returns character transmit duration in usecs

    static MAX3100485*  gm_pIRQHandler;                                   //!< The IRQ handling routine

protected:
    static SPISettings  gm_spiSet;
    void SPIBegin();
    void SPIEnd();
    uint16_t SPITxRx( uint16_t );
    uint16_t SPITxRxSingle( uint16_t );
    bool   m_bDataRX;
    bool   GetRX() const;
    void   ClearRX();
    size_t _read(  void* pvData, size_t nData, bool bEntire, bool& bResult );
};

#endif
