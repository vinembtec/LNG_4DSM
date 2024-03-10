/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Copyright (C) SEMTECH S.A.
 */
/*! 
 * \file       sx1276-Hal.c
 * \brief      SX1276 Hardware Abstraction Layer
 *
 * \version    2.0.B2 
 * \date       Nov 21 2012
 * \author     Miguel Luis
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
#include <stdint.h>
#include <stdbool.h> 

//#include "../../Low Leve Driver Modules/driverlib/gpio.h"
//#include "../../Low Leve Driver Modules/driverlib/spi.h"
#include "../../Low Level Driver Modules/driverlib/driverlib.h"
#include "radio_test.h"

#if defined( USE_SX1276_RADIO )

#include "sx1276-Hal.h"

/*!
 * SX1276 DIO pins  I/O definitions
 */

/*#define DIO0_IOPORT                                 GPIOA
#define DIO0_PIN                                    GPIO_Pin_0

#define DIO1_IOPORT                                 GPIOB
#define DIO1_PIN                                    GPIO_Pin_0


#define DIO2_IOPORT                                 GPIOC
#define DIO2_PIN                                    GPIO_Pin_5

#define DIO3_IOPORT                                 
#define DIO3_PIN                                    RF_DIO3_PIN

#define DIO4_IOPORT                                 
#define DIO4_PIN                                    RF_DIO4_PIN

#define DIO5_IOPORT                                 
#define DIO5_PIN                                    RF_DIO5_PIN

#define RXTX_IOPORT                                 
#define RXTX_PIN                                    FEM_CTX_PIN*/



void SX1276InitIo( void )
{
    // Configure NSS as output
    GPIO_setAsOutputPin(LORA_SPI_IOPORT, LORA_NSS_PIN);
    GPIO_setOutputHighOnPin(LORA_SPI_IOPORT,LORA_NSS_PIN);
    // Configure radio DIO as inputs

    // Configure DIO0,
    // Configure DIO1,
    // Configure DIO2,
    // Configure DIO3 as input,
    // Configure DIO4 as input,
    // Configure DIO5 as input
    GPIO_setAsInputPin(DIO0_IOPORT, DIO0_PIN);
}

void SX1276SetReset( uint8_t state )
{
    if( state == RADIO_RESET_ON )
    {
        // Set RESET pin to 0
    	GPIO_setAsOutputPin(LORA_RESET_IOPORT, LORA_RESET_PIN);
    	GPIO_setOutputLowOnPin( LORA_RESET_IOPORT, LORA_RESET_PIN);
        // Configure RESET as output
    }
    else
    {
    	GPIO_setAsOutputPin(LORA_RESET_IOPORT, LORA_RESET_PIN);
    	GPIO_setOutputHighOnPin( LORA_RESET_IOPORT, LORA_RESET_PIN);
    }
}

void SX1276Write( uint8_t addr, uint8_t data )
{
    SX1276WriteBuffer( addr, &data, 1 );
}

void SX1276Read( uint8_t addr, uint8_t *data )
{
    SX1276ReadBuffer( addr, data, 1 );
}

void SX1276WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

    //NSS = 0;
    GPIO_setOutputLowOnPin( LORA_SPI_IOPORT, LORA_NSS_PIN);

     addr |= 0x80 ;

	 //EUSCI_B_SPI_transmitData(EUSCI_B2_BASE, addr);
     EUSCI_B_CMSIS(EUSCI_B2_BASE)->TXBUF = addr;
	 while( EUSCI_B_SPI_isBusy(EUSCI_B2_BASE));

    for( i = 0; i < size; i++ )
    {
        //EUSCI_B_SPI_transmitData(EUSCI_B2_BASE, buffer[i]);
    	EUSCI_B_CMSIS(EUSCI_B2_BASE)->TXBUF = buffer[i];
        while( EUSCI_B_SPI_isBusy(EUSCI_B2_BASE));
    }

    //NSS = 1;
    GPIO_setOutputHighOnPin( LORA_SPI_IOPORT, LORA_NSS_PIN);
    DelayUs(10);
}

void SX1276ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;//,byte;
    //memset(buffer, 0, size);
    //for(i=0;i<10; i++);
    //NSS = 0;
    GPIO_setOutputLowOnPin( LORA_SPI_IOPORT, LORA_NSS_PIN);

    addr &= 0x7F;
    EUSCI_B_CMSIS(EUSCI_B2_BASE)->TXBUF = addr;
    //EUSCI_B_SPI_transmitData(EUSCI_B2_BASE, addr);
   while( EUSCI_B_SPI_isBusy(EUSCI_B2_BASE));
   // EUSCI_B_SPI_transmitData(EUSCI_B2_BASE, 0);

    for( i = 0; i < size; i++ )
    {
    	EUSCI_B_CMSIS(EUSCI_B2_BASE)->TXBUF = 0;
    	//EUSCI_B_SPI_transmitData(EUSCI_B2_BASE, 0);
    	while( EUSCI_B_SPI_isBusy(EUSCI_B2_BASE));
    	buffer[i] = EUSCI_B_CMSIS(EUSCI_B2_BASE)->RXBUF;//EUSCI_B_SPI_receiveData(EUSCI_B2_BASE);
    }

    //NSS = 1;
    GPIO_setOutputHighOnPin( LORA_SPI_IOPORT, LORA_NSS_PIN);
    DelayUs(10);
}

void SX1276SendByte(uint8_t Byte)
{
	EUSCI_B_SPI_transmitData(EUSCI_B2_BASE, Byte);
	EUSCI_B_SPI_transmitData(EUSCI_B2_BASE, 0);
}

uint8_t SX1276GetByte()
{
	uint8_t Read_byte = 0;
	Read_byte = EUSCI_B_SPI_receiveData(EUSCI_B2_BASE);
	return (uint8_t)Read_byte;
}
void SX1276WriteFifo( uint8_t *buffer, uint8_t size )
{
    SX1276WriteBuffer( 0, buffer, size );
}

void SX1276ReadFifo( uint8_t *buffer, uint8_t size )
{
    SX1276ReadBuffer( 0, buffer, size );
}

inline uint8_t SX1276ReadDio0( void )
{
    return GPIO_getInputPinValue( DIO0_IOPORT, DIO0_PIN );
}

inline uint8_t SX1276ReadDio1( void )
{
    return GPIO_getInputPinValue( DIO1_IOPORT, DIO1_PIN );
}

inline uint8_t SX1276ReadDio2( void )
{
    return GPIO_getInputPinValue( DIO2_IOPORT, DIO2_PIN );
}

inline uint8_t SX1276ReadDio3( void )
{
    return GPIO_getInputPinValue( DIO3_IOPORT,DIO3_PIN );
}

inline uint8_t SX1276ReadDio4( void )
{
    return GPIO_getInputPinValue( DIO4_IOPORT,DIO4_PIN );
}

inline uint8_t SX1276ReadDio5( void )
{
    return GPIO_getInputPinValue( DIO5_IOPORT,DIO5_PIN );
}

inline void SX1276WriteRxTx( uint8_t txEnable )
{
    if( txEnable != 0 )
    {
    	GPIO_setOutputHighOnPin( LORA_RXTX_IOPORT, LORA_RXTX_PIN);
    	GPIO_setOutputHighOnPin( LORA_HF_PORT, LORA_HFANT_CNTL);
    }
    else
    {
    	GPIO_setOutputLowOnPin( LORA_RXTX_IOPORT, LORA_RXTX_PIN);
    	GPIO_setOutputLowOnPin( LORA_HF_PORT, LORA_HFANT_CNTL);
    }
}

#endif // USE_SX1276_RADIO
