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
 * \file       sx1276-LoRa.c
 * \brief      SX1276 RF chip driver mode LoRa
 *
 * \version    2.0.0 
 * \date       May 6 2013
 * \author     Gregory Cristian
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
#include <string.h>

#include "radio_test.h"

#if defined( USE_SX1276_RADIO )

#include "radio.h"

#include "sx1276-Hal.h"
#include "sx1276.h"

#include "sx1276-LoRaMisc.h"
#include "sx1276-LoRa.h"
#include "../../Low Level Driver Modules/driverlib/driverlib.h"
#include "../../Low Level Driver Modules/LibG2_timer.h"
#include "../../Application Modules/LibG2_debug.h"

/*
 * Constant values need to compute the RSSI value
 */
#define RSSI_OFFSET_LF                              -164.0
#define RSSI_OFFSET_HF                              -157.0

//uint8_t TX_COUNT = 0, RX_COUNT = 0;
extern uint8_t SF,BW,PW,Terminal_Values_RXD,Test_Complete;
extern uint16_t CF,PL,PKTS;
extern uint16_t BufferSize;
extern uint8_t						LQI_diagnostic[MAX_DIAG_READINGS], LQI_counter;
extern GPRSSystem      		glSystem_cfg;
uint16_t Count = 0;
/*!
 * Frequency hopping frequencies table
 */
const int32_t HoppingFrequencies[] =
{
    916500000,
    923500000,
    906500000,
    917500000,
    917500000,
    909000000,
    903000000,
    916000000,
    912500000,
    926000000,
    925000000,
    909500000,
    913000000,
    918500000,
    918500000,
    902500000,
    911500000,
    926500000,
    902500000,
    922000000,
    924000000,
    903500000,
    913000000,
    922000000,
    926000000,
    910000000,
    920000000,
    922500000,
    911000000,
    922000000,
    909500000,
    926000000,
    922000000,
    918000000,
    925500000,
    908000000,
    917500000,
    926500000,
    908500000,
    916000000,
    905500000,
    916000000,
    903000000,
    905000000,
    915000000,
    913000000,
    907000000,
    910000000,
    926500000,
    925500000,
    911000000,
};

// Default settings
tLoRaSettings LoRaSettings1 =
{
	915000000,        // 900000000 RFFrequency
    17,               // Power
    9,                // SignalBw [0: 7.8kHz, 1: 10.4 kHz, 2: 15.6 kHz, 3: 20.8 kHz, 4: 31.2 kHz,
                      // 5: 41.6 kHz, 6: 62.5 kHz, 7: 125 kHz, 8: 250 kHz, 9: 500 kHz, other: Reserved]
    7,                // SpreadingFactor [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]
    1,                // ErrorCoding [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
    true,             // CrcOn [0: OFF, 1: ON]
    false,            // ImplicitHeaderOn [0: OFF, 1: ON]
    1,                // RxSingleOn [0: Continuous, 1 Single]
    0,                // FreqHopOn [0: OFF, 1: ON]
    4,                // HopPeriod Hops every frequency hopping period symbols
    2,//100,              // TxPacketTimeout
    2,//100,              // RxPacketTimeout
    64,              // PayloadLength (used for implicit header mode)
};

tLoRaSettings LoRaSettings;
/*!
 * SX1276 LoRa registers variable
 */
tSX1276LR SX1276LR_struct;
tSX1276LR* SX1276LR = &(SX1276LR_struct);
//tSX1276LR SX1276LR_test;

/*!
 * Local RF buffer for communication support
 */
uint8_t RFBuffer[RF_BUFFER_SIZE];

/*!
 * RF state machine variable
 */
uint8_t RFLRState = RFLR_STATE_IDLE;

/*!
 * Rx management support variables
 */
uint16_t RxPacketSize = 0;
static int8_t RxPacketSnrEstimate;
double RxPacketRssiValue = 0,Transmitter_RSSI_value = 0;//,Avg_RxPacketRssiValue = 0,Avg_Transmitter_RSSI_value = 0;
static uint8_t RxGain = 1;
static uint32_t RxTimeoutTimer = 0,TxTimeoutTimer = 0;
/*!
 * PacketTimeout Stores the Rx window time value for packet reception
 */
static uint32_t PacketTimeout;

/*!
 * Tx management support variables
 */
static uint16_t TxPacketSize = 0;

void SX1276LoRaInit( void )
{
    uint8_t status = 0;
    uint32_t lRFFrequency = 0;

    //Debug_TextOut(0,"Lora Init");
    Assign_Lora_parameters();
    /*if(Terminal_Values_RXD == TRUE)//OVERWRITE DEFAULT SETTINGS
    {
    	LoRaSettings.RFFrequency = (uint32_t)((uint32_t)CF *(uint32_t) 1000000);
    	LoRaSettings.SpreadingFactor = SF;
    	LoRaSettings.SignalBw = BW;
    	LoRaSettings.Power = PW;
    	BufferSize = PL;
    }*/
	RFLRState = RFLR_STATE_IDLE;

	status = sizeof(tSX1276LR);
    SX1276LoRaSetDefaults( );
    
    SX1276ReadBuffer( REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1 );
    
    SX1276LR->RegLna = RFLR_LNA_GAIN_G1;

    SX1276WriteBuffer( REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1 );

    SX1276ReadBuffer( REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1 );

    // set the RF settings 
   // SX1276LoRaSetRFFrequency( LoRaSettings.RFFrequency );
    SX1276LoRaSetSpreadingFactor( LoRaSettings.SpreadingFactor ); // SF6 only operates in implicit header mode.
    SX1276LoRaSetErrorCoding( LoRaSettings.ErrorCoding );
    SX1276LoRaSetPacketCrcOn( LoRaSettings.CrcOn );
    SX1276LoRaSetSignalBandwidth( LoRaSettings.SignalBw );
    SX1276LoRaSetImplicitHeaderOn( LoRaSettings.ImplicitHeaderOn );
    if(LoRaSettings.RxSingleOn == 1)
    {
    	//Debug_TextOut(0,"Freq Change");
    	LoRaSettings.Power = 17;
    	SX1276LoRaSetSymbTimeout(5);
        SX1276LoRaSetPreambleLength(5000);
        lRFFrequency = IDBasedFreqCalculation(glSystem_cfg.meter_id); // Freq Calculation
        SX1276LoRaSetRFFrequency(lRFFrequency);
    	Debug_Output1(0,"RFFrequency:%d",((LoRaSettings.RFFrequency)/1000000L));
    }
    else
    {
    	LoRaSettings.Power = 17;
    	SX1276LoRaSetSymbTimeout(0x3FF);
    	LoRaSettings1.RFFrequency = (uint32_t)((uint32_t)glSystem_cfg.Active_Channel*(uint32_t)1000000L);	//primary communication channel
    	SX1276LoRaSetRFFrequency( LoRaSettings1.RFFrequency);
    	Debug_Output1(0,"RFFrequency:%d",((LoRaSettings1.RFFrequency)/1000000L));
    }

	if((++Count) == 600)
	{
		Count = 0;
	    Debug_Output1(0,"RFFrequency:%d",((  LoRaSettings.RFFrequency )/1000000L));
	}

    //Debug_Output1(0,"RFFrequency:%d",((  LoRaSettings.RFFrequency )/1000000));
    SX1276LoRaSetPayloadLength( LoRaSettings.PayloadLength );
    SX1276LoRaSetLowDatarateOptimize( true );
    //SX1276LoRaSetPreambleLength(5000);
    //SX1276LoRaSetLowDatarateOptimize( false );

    //only for SF6
    //SX1276WriteBuffer( 0x37, 0x0C, 1 );
    //SX1276ReadBuffer( 0x37, &status, 1);
    //only for SF6

#if( ( MODULE_SX1276RF1IAS == 1 ) || ( MODULE_SX1276RF1KAS == 1 ) )
    if( LoRaSettings.RFFrequency > 860000000 )
    {
        SX1276LoRaSetPAOutput( RFLR_PACONFIG_PASELECT_RFO );
        SX1276LoRaSetPa20dBm( false );
        LoRaSettings.Power = 14;
        SX1276LoRaSetRFPower( LoRaSettings.Power );
    }
    else
    {
        SX1276LoRaSetPAOutput( RFLR_PACONFIG_PASELECT_PABOOST );
        SX1276LoRaSetPa20dBm( true );
        LoRaSettings.Power = 20;
        SX1276LoRaSetRFPower( LoRaSettings.Power );
    } 
#elif( MODULE_SX1276RF1JAS == 1 )
    //if( LoRaSettings.RFFrequency > 860000000 )
    {
        SX1276LoRaSetPAOutput( RFLR_PACONFIG_PASELECT_PABOOST );
        SX1276LoRaSetPa20dBm( false );
        //LoRaSettings.Power = 20;
        SX1276LoRaSetRFPower( LoRaSettings.Power );
    }
    /*else
    {
        SX1276LoRaSetPAOutput( RFLR_PACONFIG_PASELECT_RFO );
        SX1276LoRaSetPa20dBm( false );
        LoRaSettings.Power = 14;
        SX1276LoRaSetRFPower( LoRaSettings.Power );
    } */
#endif
//For BW 62.5khz,125khz,250khz
		if((LoRaSettings.SignalBw == 6) || (LoRaSettings.SignalBw == 7) ||(LoRaSettings.SignalBw == 8))
		{
			SX1276ReadBuffer( REG_LR_LORADETECTOPTIMIZE, &status, 1);
			status = status & 0x7F;
			SX1276WriteBuffer( REG_LR_LORADETECTOPTIMIZE, &status, 1);
			//SX1276ReadBuffer( REG_LR_LORADETECTOPTIMIZE, &status, 1);

			status = 0x40;
			SX1276WriteBuffer( 0x2F, &status, 1);
			//SX1276ReadBuffer( 0x2F, &status, 1);

			status = 0;
			SX1276WriteBuffer( 0x30, &status, 1);
			//SX1276ReadBuffer( 0x30, &status, 1);
		}
//For BW 500khz
		if(LoRaSettings.SignalBw == 9)
		{
			SX1276ReadBuffer( REG_LR_LORADETECTOPTIMIZE, &status, 1);
			status = status | 0x80;
			SX1276WriteBuffer( REG_LR_LORADETECTOPTIMIZE, &status, 1);

			status = 0X02;
			SX1276WriteBuffer( 0X36, &status, 1);

			status = 0X64;
			SX1276WriteBuffer( 0X3A, &status, 1);
		}
		else
		{
			status = 0X03;
			SX1276WriteBuffer( 0X36, &status, 1);
		}


    //SX1276LoRaSetPaRamp(8);
    /*status = 0x11;
    SX1276WriteBuffer( 0x53, &status, 1);
    //SX1276ReadBuffer( 0x36, &status, 1);
    status = 0xF6;
    SX1276WriteBuffer( 0x5B, &status, 1);
    status = 0x07;
    SX1276WriteBuffer( 0x5C, &status, 1);
    SX1276ReadBuffer( 0x5C, &status, 1);

    status = 0x0E;
   SX1276WriteBuffer( 0x65, &status, 1);
   SX1276ReadBuffer( 0x65, &status, 1);

   status = 0x41;
   SX1276WriteBuffer( 0x66, &status, 1);
   SX1276ReadBuffer( 0x66, &status, 1);

   status = 0x48;
   SX1276WriteBuffer( 0x6C, &status, 1);
   status = 0x06;
   SX1276WriteBuffer( 0x6E, &status, 1);
   status = 0x12;
  SX1276WriteBuffer( 0x72, &status, 1);
  SX1276ReadBuffer( 0x72, &status, 1);*/
   // SX1276ReadBuffer( 0x3a, &status, 1);
    //SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );
    //SX1276ReadBuffer( REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1 );
    //status = 0X34;

   // SX1276WriteBuffer( 0x39, &status, 1 );
   // SX1276ReadBuffer( 0x39, &status, 1);
    //SX1276ReadBuffer( 0x31, &status, 1);
    //SX1276WriteBuffer( 0x37, 0x0C, 1 );
    //SX1276ReadBuffer( 0x37, &status, 1);
    //SX1276ReadBuffer( REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1 );
}

void SX1276LoRaSetDefaults( void )
{
    //uint8_t CMD = REG_LR_VERSION;

	// REMARK: See SX1276 datasheet for modified default values.
    SX1276Read( REG_LR_VERSION, &SX1276LR->RegVersion );
	/*GPIO_setOutputLowOnPin( SPI_IOPORT, NSS_PIN);
	CMD &= 0x7F;
	SX1276SendByte(CMD);
	CMD = SX1276GetByte();
	SX1276LR->RegVersion = CMD;
	GPIO_setOutputHighOnPin( SPI_IOPORT, NSS_PIN);*/
}

void SX1276LoRaReset( void )
{
    SX1276SetReset( RADIO_RESET_ON );
    
    // Wait 1ms
    uint32_t startTick = GET_TICK_COUNT( );
    while( ( GET_TICK_COUNT( ) - startTick ) < TICK_RATE_MS( 1 ) );    

    SX1276SetReset( RADIO_RESET_OFF );
    
    // Wait 6ms
    startTick = GET_TICK_COUNT( );
    while( ( GET_TICK_COUNT( ) - startTick ) < TICK_RATE_MS( 6 ) );    
}

void SX1276LoRaSetOpMode( uint8_t opMode )
{
    static uint8_t opModePrev = RFLR_OPMODE_STANDBY;
    //static bool antennaSwitchTxOnPrev = true;
    bool antennaSwitchTxOn = false;

    opModePrev = SX1276LR->RegOpMode & ~RFLR_OPMODE_MASK;

    if( opMode != opModePrev )
    {
        if( opMode == RFLR_OPMODE_TRANSMITTER )
        {
            antennaSwitchTxOn = true;
        }
        else
        {
            antennaSwitchTxOn = false;
        }
        //if( antennaSwitchTxOn != antennaSwitchTxOnPrev )
        {
            //antennaSwitchTxOnPrev = antennaSwitchTxOn;
            RXTX( antennaSwitchTxOn ); // Antenna switch control
            //GPIO_setOutputHighOnPin( LORA_HF_PORT, LORA_HFANT_CNTL);
        }
        SX1276LR->RegOpMode = ( SX1276LR->RegOpMode & RFLR_OPMODE_MASK ) | opMode;
        //SX1276LR->RegOpMode |= RFLR_OPMODE_FREQMODE_ACCESS_LF;
        SX1276Write( REG_LR_OPMODE, SX1276LR->RegOpMode );
       // Debug_Output1(0,"OPMODE:%d\r\n",SX1276LR->RegOpMode);
    }
}

uint8_t SX1276LoRaGetOpMode( void )
{
    SX1276Read( REG_LR_OPMODE, &SX1276LR->RegOpMode );
    
    return SX1276LR->RegOpMode & ~RFLR_OPMODE_MASK;

    /*uint8_t CMD = REG_LR_OPMODE;

	// REMARK: See SX1276 datasheet for modified default values.
   // SX1276Read( REG_LR_VERSION, &SX1276LR->RegVersion );
	GPIO_setOutputLowOnPin( SPI_IOPORT, NSS_PIN);
	CMD &= 0x7F;
	SX1276SendByte(CMD);
	CMD = SX1276GetByte();
	SX1276LR->RegOpMode = CMD;
	GPIO_setOutputHighOnPin( SPI_IOPORT, NSS_PIN);
	return SX1276LR->RegOpMode & ~RFLR_OPMODE_MASK;*/
}

uint8_t SX1276LoRaReadRxGain( void )
{
    SX1276Read( REG_LR_LNA, &SX1276LR->RegLna );
    return( SX1276LR->RegLna >> 5 ) & 0x07;
}

double SX1276LoRaReadRssi( void )
{
    // Reads the RSSI value
    SX1276Read( REG_LR_RSSIVALUE, &SX1276LR->RegRssiValue );

    if( LoRaSettings.RFFrequency < 860000000 )  // LF
    {
        return RSSI_OFFSET_LF + ( double )SX1276LR->RegRssiValue;
    }
    else
    {
        return RSSI_OFFSET_HF + ( double )SX1276LR->RegRssiValue;
    }
}

uint8_t SX1276LoRaGetPacketRxGain( void )
{
    return RxGain;
}

int8_t SX1276LoRaGetPacketSnr( void )
{
    return RxPacketSnrEstimate;
}

double SX1276LoRaGetPacketRssi( void )
{
    return RxPacketRssiValue;
}

void SX1276LoRaStartRx( void )
{
    SX1276LoRaSetRFState( RFLR_STATE_RX_INIT );
}

void SX1276LoRaGetRxPacket( void *buffer, uint16_t *size )
{
    *size = RxPacketSize;
    RxPacketSize = 0;
    memcpy( ( void * )buffer, ( void * )RFBuffer, ( size_t )*size );
}

void SX1276LoRaSetTxPacket( const void *buffer, uint16_t size )
{
    TxPacketSize = size;
    memcpy( ( void * )RFBuffer, buffer, ( size_t )TxPacketSize ); 

    RFLRState = RFLR_STATE_TX_INIT;
}

uint8_t SX1276LoRaGetRFState( void )
{
    return RFLRState;
}

void SX1276LoRaSetRFState( uint8_t state )
{
    RFLRState = state;
}

uint8_t SX1276LoRaCheckTXState()
{
	if((RFLRState != RFLR_STATE_TX_INIT) && (RFLRState != RFLR_STATE_TX_RUNNING) && (RFLRState != RFLR_STATE_TX_DONE))
		return TRUE;
	else
		return FALSE;
}

/*!
 * \brief Process the LoRa modem Rx and Tx state machines depending on the
 *        SX1276 operating mode.
 *
 * \retval rfState Current RF state [RF_IDLE, RF_BUSY, 
 *                                   RF_RX_DONE, RF_RX_TIMEOUT,
 *                                   RF_TX_DONE, RF_TX_TIMEOUT]
 */
//uint8_t TX_Retry = 0;
uint32_t SX1276LoRaProcess( void )
{
    uint32_t result = RF_BUSY;
    uint8_t check = 0;
    //uint8_t current_valD=0;
    //uint16_t current_valF=0;
    //double lRxPacketRssiValue = 0;
    //double lTransmitter_RSSI_value = 0;
    
    switch( RFLRState )
    {
    case RFLR_STATE_IDLE:
    	//Debug_TextOut(0,"RFLR_STATE_IDLE: 0");
        break;
    case RFLR_STATE_RX_INIT:
    	//Debug_TextOut(0,"RFLR_STATE_RX_INIT: 1");
        
        SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );

        SX1276LR->RegIrqFlagsMask = //RFLR_IRQFLAGS_RXTIMEOUT |
                                    //RFLR_IRQFLAGS_RXDONE |
                                    //RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                    RFLR_IRQFLAGS_VALIDHEADER |
                                    RFLR_IRQFLAGS_TXDONE |
                                    RFLR_IRQFLAGS_CADDONE |
                                    //RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                    RFLR_IRQFLAGS_CADDETECTED;
        SX1276Write( REG_LR_IRQFLAGSMASK, SX1276LR->RegIrqFlagsMask );

        if( LoRaSettings.FreqHopOn == true )
        {
            SX1276LR->RegHopPeriod = LoRaSettings.HopPeriod;

            SX1276Read( REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel );
            SX1276LoRaSetRFFrequency( HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK] );
        }
        else
        {
            SX1276LR->RegHopPeriod = 255;
        }
        
        SX1276Read( REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel );
        SX1276Write( REG_LR_HOPPERIOD, SX1276LR->RegHopPeriod );
                
                                    // RxDone                    RxTimeout                   FhssChangeChannel           CadDone
        SX1276LR->RegDioMapping1 = RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO1_00 | RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00;
                                    // CadDetected               ModeReady
        SX1276LR->RegDioMapping2 = RFLR_DIOMAPPING2_DIO4_00 | RFLR_DIOMAPPING2_DIO5_00;
        SX1276WriteBuffer( REG_LR_DIOMAPPING1, &SX1276LR->RegDioMapping1, 2 );
    
        if( LoRaSettings.RxSingleOn == true ) // Rx single mode
        {
            SX1276LoRaSetOpMode( RFLR_OPMODE_RECEIVER_SINGLE );
        }
        else // Rx continuous mode
        {

            SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoRxBaseAddr;
            SX1276Write( REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr );
            
            SX1276LoRaSetOpMode( RFLR_OPMODE_RECEIVER );
        }
        SX1276LoRaGetOpMode();
        memset( RFBuffer, 0, ( size_t )RF_BUFFER_SIZE );

        PacketTimeout = LoRaSettings.RxPacketTimeout;
        RxTimeoutTimer = GET_TICK_COUNT( );
        RFLRState = RFLR_STATE_RX_RUNNING;
        break;
    case RFLR_STATE_RX_RUNNING:
    	//Debug_TextOut(0,"RFLR_STATE_RX_RUNNING : 2");
    	DelayMs(1);
    	SX1276Read( REG_LR_IRQFLAGS, &SX1276LR->RegIrqFlags );
    	//Debug_Output1(0,"RFLR_STATE_RX_RUNNING : %d", SX1276LR->RegIrqFlags);
    	//RFLRState = RFLR_STATE_RX_INIT;
        //if( DIO0 == 1 ) // RxDone
        if((SX1276LR->RegIrqFlags & RFLR_IRQFLAGS_RXDONE) == RFLR_IRQFLAGS_RXDONE)
    	{
            RxTimeoutTimer = GET_TICK_COUNT( );
            if( LoRaSettings.FreqHopOn == true )
            {
                SX1276Read( REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel );
                SX1276LoRaSetRFFrequency( HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK] );
            }
            // Clear Irq
            SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_RXDONE  );
            RFLRState = RFLR_STATE_RX_DONE;
        }
        //if( DIO2 == 1 ) // FHSS Changed Channel
        if((SX1276LR->RegIrqFlags & RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL) == RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL)
        {
            RxTimeoutTimer = GET_TICK_COUNT( );
            if( LoRaSettings.FreqHopOn == true )
            {
                SX1276Read( REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel );
                SX1276LoRaSetRFFrequency( HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK] );
            }
            // Clear Irq
            SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL );
            // Debug
            RxGain = SX1276LoRaReadRxGain( );
        }


//TEST MODE:ENABLE THIS TO CREATE RX TIMEOUT AND ALLOW TO TX ALWAYS
        //IF COMMENTED THIS CAN BE USED FOR CONT RX
        if( LoRaSettings.RxSingleOn == true ) // Rx single mode
        {
            if((SX1276LR->RegIrqFlags & RFLR_IRQFLAGS_RXTIMEOUT) == RFLR_IRQFLAGS_RXTIMEOUT)
            {
            	RFLRState = RFLR_STATE_RX_TIMEOUT;
            	SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_RXTIMEOUT );
            }
            else if( ( GET_TICK_COUNT( ) - RxTimeoutTimer ) > PacketTimeout )
            {
                RFLRState = RFLR_STATE_RX_TIMEOUT;
            }
        }
        //RFLRState = RFLR_STATE_RX_INIT;
        break;
    case RFLR_STATE_RX_DONE:
    	/*if(RX_COUNT >= PKTS)
    	{
    		Test_Complete = TRUE;
    		//RX_COUNT = 0;
    	}
    	else
    	RX_COUNT++;*/
    	//Debug_TextOut(0,"RFLR_STATE_RX_DONE");
        SX1276Read( REG_LR_IRQFLAGS, &SX1276LR->RegIrqFlags );
       // Debug_Output1(0,"RFLR_STATE_RX_DONE: %d", SX1276LR->RegIrqFlags);
        if( ( SX1276LR->RegIrqFlags & RFLR_IRQFLAGS_PAYLOADCRCERROR ) == RFLR_IRQFLAGS_PAYLOADCRCERROR )
        {
        	Debug_TextOut(0,"PAYLOADCRCERROR");
        	// Clear Irq
            SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_PAYLOADCRCERROR  );
            
            if( LoRaSettings.RxSingleOn == true ) // Rx single mode
            {
                RFLRState = RFLR_STATE_RX_INIT;
            }
            else
            {
                RFLRState = RFLR_STATE_RX_RUNNING;
            }
            break;
        }
        
        {
            uint8_t rxSnrEstimate;
            SX1276Read( REG_LR_PKTSNRVALUE, &rxSnrEstimate );
            if( rxSnrEstimate & 0x80 ) // The SNR sign bit is 1
            {
                // Invert and divide by 4
                RxPacketSnrEstimate = ( ( ~rxSnrEstimate + 1 ) & 0xFF ) >> 2;
                RxPacketSnrEstimate = -RxPacketSnrEstimate;
            }
            else
            {
                // Divide by 4
                RxPacketSnrEstimate = ( rxSnrEstimate & 0xFF ) >> 2;
            }
        }
        
        SX1276Read( REG_LR_PKTRSSIVALUE, &SX1276LR->RegPktRssiValue );
        //Last_RxPacketRssiValue = RxPacketRssiValue;
        //Last_Transmitter_RSSI_value = Transmitter_RSSI_value;
        if( LoRaSettings.RFFrequency < 860000000 )  // LF
        {    
            if( RxPacketSnrEstimate < 0 )
            {
                RxPacketRssiValue = RSSI_OFFSET_LF + ( ( double )SX1276LR->RegPktRssiValue ) + RxPacketSnrEstimate;
            }
            else
            {
                RxPacketRssiValue = RSSI_OFFSET_LF + ( 1.0666 * ( ( double )SX1276LR->RegPktRssiValue ) );
            }
        }
        else                                        // HF
        {    
            if( RxPacketSnrEstimate < 0 )
            {
                RxPacketRssiValue = RSSI_OFFSET_HF + ( ( double )SX1276LR->RegPktRssiValue ) + RxPacketSnrEstimate;
            }
            else
            {    
                RxPacketRssiValue = RSSI_OFFSET_HF + ( 1.0666 * ( ( double )SX1276LR->RegPktRssiValue ) );
            }
        }

        if( LoRaSettings.RxSingleOn == true ) // Rx single mode
        {
            SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoRxBaseAddr;
            SX1276Write( REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr );

            if( LoRaSettings.ImplicitHeaderOn == true )
            {
                RxPacketSize = SX1276LR->RegPayloadLength;
                SX1276ReadFifo( RFBuffer, SX1276LR->RegPayloadLength );
            }
            else
            {
                SX1276Read( REG_LR_NBRXBYTES, &SX1276LR->RegNbRxBytes );
                RxPacketSize = SX1276LR->RegNbRxBytes;
                SX1276ReadFifo( RFBuffer, SX1276LR->RegNbRxBytes );
            }
        }
        else // Rx continuous mode
        {
            SX1276Read( REG_LR_FIFORXCURRENTADDR, &SX1276LR->RegFifoRxCurrentAddr );

            if( LoRaSettings.ImplicitHeaderOn == true )
            {
                RxPacketSize = SX1276LR->RegPayloadLength;
                SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoRxCurrentAddr;
                SX1276Write( REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr );
                SX1276ReadFifo( RFBuffer, SX1276LR->RegPayloadLength );
            }
            else
            {
                SX1276Read( REG_LR_NBRXBYTES, &SX1276LR->RegNbRxBytes );
                RxPacketSize = SX1276LR->RegNbRxBytes;
                SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoRxCurrentAddr;
                SX1276Write( REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr );
                SX1276ReadFifo( RFBuffer, SX1276LR->RegNbRxBytes );
            }
        }
        //Debug_Output1(0,"RFLR_STATE_RX_DONE 3 :%d ",SX1276LR->RegNbRxBytes);
        memcpy(&Transmitter_RSSI_value,&RFBuffer[RxPacketSize-8],8);

//        LQI_diagnostic[LQI_counter++] = RxPacketRssiValue + DEFAULT_LQI_ADDER;
//        Debug_Output1(0,"LQI: %d",LQI_diagnostic[LQI_counter-1]);
//        if(LQI_counter >= MAX_DIAG_READINGS)
//        	LQI_counter = 0;

       //print current pkt RSSI
        //uint8_t current_valD=0;
        //uint16_t current_valF=0;
        /*lRxPacketRssiValue = RxPacketRssiValue;
        lTransmitter_RSSI_value = Transmitter_RSSI_value;
        if(lRxPacketRssiValue < 0)
        {
        	lRxPacketRssiValue=(-1*lRxPacketRssiValue);
        	current_valD=(uint8_t)(lRxPacketRssiValue);
        	current_valF=(uint16_t)((lRxPacketRssiValue-current_valD)*1000);
        	//Debug_Output2(0,"RSSI:-%d.%02d  dBm",current_valD,current_valF);
        }
		else
		{
			current_valD=(uint8_t)(lRxPacketRssiValue);
			current_valF=(uint16_t)((lRxPacketRssiValue-current_valD)*1000);
			//Debug_Output2(0,"RSSI:%d.%02d  dBm",current_valD,current_valF);
		}
        //print last pkt RSSI
        if(lTransmitter_RSSI_value < 0)
        {
        	lTransmitter_RSSI_value=(-1*Transmitter_RSSI_value);
        	current_valD=(uint8_t)(lTransmitter_RSSI_value);
        	current_valF=(uint16_t)((lTransmitter_RSSI_value-current_valD)*1000);
        	//Debug_Output2(0,"TRSSI:-%d.%02d  dBm",current_valD,current_valF);
        }
		else
		{
			current_valD=(uint8_t)(lTransmitter_RSSI_value);
			current_valF=(uint16_t)((lTransmitter_RSSI_value-current_valD)*1000);
			//Debug_Output2(0,"TRSSI:%d.%02d  dBm",current_valD,current_valF);
		}*/
        //Debug_Output2(0,"RFLR_STATE_RX_DONE:%d  %d\r\n",,RxPacketRssiValue);
        /*if(RX_COUNT >= 2)
        {
        	Avg_RxPacketRssiValue = (Avg_RxPacketRssiValue + RxPacketRssiValue)/2;
        	Avg_Transmitter_RSSI_value = (Avg_Transmitter_RSSI_value + Transmitter_RSSI_value)/2;
        }
        else
        {
        	Avg_RxPacketRssiValue = (Avg_RxPacketRssiValue + RxPacketRssiValue);
        	Avg_Transmitter_RSSI_value = (Avg_Transmitter_RSSI_value + Transmitter_RSSI_value);
        }*/
        //if(TX_COUNT >= 254)
        {
        	/*double lAvg_RxPacketRssiValue = Avg_RxPacketRssiValue;
        	double lAvg_Transmitter_RSSI_value = Avg_Transmitter_RSSI_value;
        	if(lAvg_RxPacketRssiValue < 0)
        	{
        		lAvg_RxPacketRssiValue=(-1*lAvg_RxPacketRssiValue);
        		current_valD=(uint8_t)(lAvg_RxPacketRssiValue);
        		current_valF=(uint16_t)((lAvg_RxPacketRssiValue-current_valD)*1000);
        		Debug_Output2(0,"A.CRSSI:-%d.%02d  dBm\r\n",current_valD,current_valF);
        	}
        	else
        	{
        		current_valD=(uint8_t)(lAvg_RxPacketRssiValue);
        		current_valF=(uint16_t)((lAvg_RxPacketRssiValue-current_valD)*1000);
        		Debug_Output2(0,"A.CRSSI:%d.%02d  dBm\r\n",current_valD,current_valF);
        	}
        	//print last pkt RSSI
        	if(lAvg_Transmitter_RSSI_value < 0)
        	{
        		lAvg_Transmitter_RSSI_value=(-1*lAvg_Transmitter_RSSI_value);
        		current_valD=(uint8_t)(lAvg_Transmitter_RSSI_value);
        		current_valF=(uint16_t)((lAvg_Transmitter_RSSI_value-current_valD)*1000);
        		Debug_Output2(0,"A.LRSSI:-%d.%02d  dBm\r\n",current_valD,current_valF);
        	}
        	else
        	{
        		current_valD=(uint8_t)(lAvg_Transmitter_RSSI_value);
        		current_valF=(uint16_t)((lAvg_Transmitter_RSSI_value-current_valD)*1000);
        		Debug_Output2(0,"A.LRSSI:%d.%02d  dBm\r\n",current_valD,current_valF);
        	}
        	Debug_Output1(0,"NO.OF PKTS TRANSMITTED : %d\r\n",TX_COUNT);
        	Debug_Output1(0,"NO.OF PKTS RECEIVED : %d\r\n",RX_COUNT);
        	Debug_TextOut(0,"RESET DEVICE TO START AGIAN\r\n");
        	Test_Complete = TRUE;*/
        }

        /*if( LoRaSettings.RxSingleOn == true ) // Rx single mode
        {
            RFLRState = RFLR_STATE_RX_INIT;
        }
        else // Rx continuous mode
        {
            RFLRState = RFLR_STATE_RX_RUNNING;
        }*/
        RFLRState = RFLR_STATE_IDLE;
        SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );
        result = RF_RX_DONE;
        break;
    case RFLR_STATE_RX_TIMEOUT:
    	SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );
        RFLRState = RFLR_STATE_IDLE;//RFLR_STATE_RX_INIT;
        result = RF_RX_TIMEOUT;
       // Debug_TextOut(0,"RFLR_STATE_RX_TIMEOUT");
        break;
    case RFLR_STATE_TX_INIT:
    	//Debug_TextOut(0,"RFLR_STATE_TX_INIT");
    	//SX1276ReadBuffer( REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1 );
    	//SX1276ReadBuffer( 0x18, &check, 1 );
    	//SX1276ReadBuffer( 0x1C, &check, 1 );
        SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );
    	/*if(TX_Retry == 0)
    	SX1276LoRaSetPreambleLength(5000);
    	else
    		SX1276LoRaSetPreambleLength(12);*/

        if( LoRaSettings.FreqHopOn == true )
        {
            SX1276LR->RegIrqFlagsMask = RFLR_IRQFLAGS_RXTIMEOUT |
                                        RFLR_IRQFLAGS_RXDONE |
                                        RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                        RFLR_IRQFLAGS_VALIDHEADER |
                                        //RFLR_IRQFLAGS_TXDONE |
                                        RFLR_IRQFLAGS_CADDONE |
                                        //RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                        RFLR_IRQFLAGS_CADDETECTED;
            SX1276LR->RegHopPeriod = LoRaSettings.HopPeriod;

            SX1276Read( REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel );
            SX1276LoRaSetRFFrequency( HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK] );
        }
        else
        {
            SX1276LR->RegIrqFlagsMask = RFLR_IRQFLAGS_RXTIMEOUT |
                                        RFLR_IRQFLAGS_RXDONE |
                                        RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                        //RFLR_IRQFLAGS_VALIDHEADER |
                                        //RFLR_IRQFLAGS_TXDONE |
                                        RFLR_IRQFLAGS_CADDONE |
                                        RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                        RFLR_IRQFLAGS_CADDETECTED;
            SX1276LR->RegHopPeriod = 0;
        }
        SX1276Read( REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel );
        //Debug_Output1(0,"TX-PLL:%d\r\n",SX1276LR->RegHopChannel);
        SX1276Write( REG_LR_HOPPERIOD, SX1276LR->RegHopPeriod );
        SX1276Write( REG_LR_IRQFLAGSMASK, SX1276LR->RegIrqFlagsMask );

        // Initializes the payload size
        SX1276LR->RegPayloadLength = TxPacketSize;
        SX1276Write( REG_LR_PAYLOADLENGTH, SX1276LR->RegPayloadLength );
        
        SX1276LR->RegFifoTxBaseAddr = 0;//0x80; // Full buffer used for Tx
        SX1276Write( REG_LR_FIFOTXBASEADDR, SX1276LR->RegFifoTxBaseAddr );

        SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoTxBaseAddr;
        SX1276Write( REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr );
        
        // Write payload buffer to LORA modem

       /* RFBuffer[0] = 0X50;
        RFBuffer[1] = 0X49;
        RFBuffer[2] = 0X4E;
        RFBuffer[3] = 0X47;
        RFBuffer[4] = 0X30;
        RFBuffer[5] = 0X31;
        RFBuffer[6] = 0X32;
        RFBuffer[7] = 0X33;
        RFBuffer[8] = 0X34;*/
       // RFBuffer[9] = 1;
       // RFBuffer[10] = 2;
        SX1276WriteFifo( RFBuffer, SX1276LR->RegPayloadLength);

       // SX1276ReadFifo( RFBuffer, SX1276LR->RegPayloadLength );
                                        // TxDone               RxTimeout                   FhssChangeChannel          ValidHeader         
        SX1276LR->RegDioMapping1 = RFLR_DIOMAPPING1_DIO0_01 | RFLR_DIOMAPPING1_DIO1_00 | RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_01;
                                        // PllLock              Mode Ready
        SX1276LR->RegDioMapping2 = RFLR_DIOMAPPING2_DIO4_01 | RFLR_DIOMAPPING2_DIO5_00;
        SX1276WriteBuffer( REG_LR_DIOMAPPING1, &SX1276LR->RegDioMapping1, 1 );
        SX1276WriteBuffer( REG_LR_DIOMAPPING2, &SX1276LR->RegDioMapping2, 1 );

        //GPIO_setOutputHighOnPin( SPI_IOPORT, NSS_PIN);
        //SX1276ReadBuffer( REG_LR_OPMODE, SX1276Regs + 1, 0x80);
        SX1276LoRaSetOpMode( RFLR_OPMODE_TRANSMITTER );
        SX1276ReadBuffer( REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1 );
       // SX1276LoRaSetOpMode( RFLR_OPMODE_SYNTHESIZER_TX);
       // SX1276Read( REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel );
        //check = 0x83;
        //SX1276WriteBuffer(REG_LR_OPMODE,&check,1);
        SX1276LoRaGetOpMode();
        /*check = 0x83;

        SX1276WriteBuffer(REG_LR_OPMODE,&check,1);

        SX1276ReadBuffer( REG_LR_OPMODE, &check, 1 );*/

       // SX1276ReadBuffer( REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1 );
       // if(check == 0x83)
        RFLRState = RFLR_STATE_TX_RUNNING;
        //SX1276ReadBuffer( REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1 );
        /*SX1276ReadBuffer( 0X1C, &check, 1 );
        SX1276ReadBuffer( 0x18, &check, 1 );
        SX1276ReadBuffer( REG_LR_FEIMSB, &check, 1 );
        SX1276ReadBuffer( 0X39, &check, 1 );
        SX1276ReadBuffer( REG_LR_DETECTIONTHRESHOLD, &check, 1 );*/
        //else
        //RFLRState = RFLR_STATE_TX_INIT;
        PacketTimeout = LoRaSettings.TxPacketTimeout;
        TxTimeoutTimer = GET_TICK_COUNT( );
       // TX_Retry = 0;
        break;
    case RFLR_STATE_TX_RUNNING:
    	//Debug_TextOut(0,"RFLR_STATE_TX_RUNNING\r\n");
    	DelayMs(1);
    	SX1276Read( REG_LR_IRQFLAGS, &check  );
    	//Debug_Output1(0,"RFLR_STATE_TX_RUNNING: %d", SX1276LR->RegIrqFlags);
    	//RFLRState = RFLR_STATE_TX_INIT;
    	//if( DIO0 == 1 ) // TxDone
    	if((check & RFLR_IRQFLAGS_TXDONE) == RFLR_IRQFLAGS_TXDONE)
    	{
    		//TX_Retry++;
    		// Clear Irq
    		SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_TXDONE  );
    		/*if(TX_Retry < 5)
    		{
    			SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );
    			SX1276LoRaSetOpMode( RFLR_OPMODE_TRANSMITTER );
    		}
    		else*/
    		/*if(TX_Retry < 2)
    		RFLRState = RFLR_STATE_TX_INIT;
    		else*/
    		{
    			//TX_Retry = 0;
    			RFLRState = RFLR_STATE_TX_DONE;
    		}
    		//Debug_TextOut(0,"RFLR_IRQFLAGS_TXDONE");
    	}

        //if( DIO2 == 1 ) // FHSS Changed Channel
    	if((check & RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL) == RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL)
    	{
    		if( LoRaSettings.FreqHopOn == true )
    		{
    			SX1276Read( REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel );
    			SX1276LoRaSetRFFrequency( HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK] );
    		}
    		// Clear Irq
			SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL );
    	}

    	if( ( GET_TICK_COUNT( ) - TxTimeoutTimer ) > PacketTimeout )
    	{
    		RFLRState = RFLR_STATE_TX_TIMEOUT;
    	}
        break;
    case RFLR_STATE_TX_DONE:
        // optimize the power consumption by switching off the transmitter as soon as the packet has been sent
        SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );

        RFLRState = RFLR_STATE_IDLE;
        result = RF_TX_DONE;
        /*if(TX_COUNT >= PKTS)
        {
        	//TX_COUNT = 0;
        	Test_Complete = TRUE;
        }
        else
        TX_COUNT++;*/
        //Debug_Output1(0,"RFLR_STATE_TX_DONE:%d",TxPacketSize);
        break;
    case  RFLR_STATE_TX_TIMEOUT:
    	RFLRState = RFLR_STATE_TX_INIT;
    	result = RF_TX_TIMEOUT;
    	//Debug_TextOut(0,"RFLR_STATE_TX_TIMEOUT:");
    	break;
    case RFLR_STATE_CAD_INIT:    
        SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );
    
        SX1276LR->RegIrqFlagsMask = RFLR_IRQFLAGS_RXTIMEOUT |
                                    RFLR_IRQFLAGS_RXDONE |
                                    RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                    RFLR_IRQFLAGS_VALIDHEADER |
                                    RFLR_IRQFLAGS_TXDONE |
                                    //RFLR_IRQFLAGS_CADDONE |
                                    RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL; // |
                                    //RFLR_IRQFLAGS_CADDETECTED;
        SX1276Write( REG_LR_IRQFLAGSMASK, SX1276LR->RegIrqFlagsMask );
           
                                    // RxDone                   RxTimeout                   FhssChangeChannel           CadDone
        SX1276LR->RegDioMapping1 = RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO1_00 | RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00;
                                    // CAD Detected              ModeReady
        SX1276LR->RegDioMapping2 = RFLR_DIOMAPPING2_DIO4_00 | RFLR_DIOMAPPING2_DIO5_00;
        SX1276WriteBuffer( REG_LR_DIOMAPPING1, &SX1276LR->RegDioMapping1, 2 );
            
        SX1276LoRaSetOpMode( RFLR_OPMODE_CAD );
        RFLRState = RFLR_STATE_CAD_RUNNING;
        break;
    case RFLR_STATE_CAD_RUNNING:
        if( DIO3 == 1 ) //CAD Done interrupt
        { 
            // Clear Irq
            SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_CADDONE  );
            if( DIO4 == 1 ) // CAD Detected interrupt
            {
                // Clear Irq
                SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_CADDETECTED  );
                // CAD detected, we have a LoRa preamble
                RFLRState = RFLR_STATE_RX_INIT;
                result = RF_CHANNEL_ACTIVITY_DETECTED;
            } 
            else
            {    
                // The device goes in Standby Mode automatically    
                RFLRState = RFLR_STATE_IDLE;
                result = RF_CHANNEL_EMPTY;
            }
        }   
        break;
    
    default:
        break;
    } 
    return result;
}

uint32_t IDBasedFreqCalculation( uint16_t meterID )
{
	uint32_t calc_freq = (meterID % 100);
//	uint32_t lRFFrequency = 0;

	if(calc_freq < 20)//915
	{
		LoRaSettings.RFFrequency = (uint32_t)(START_BAND);//(902--902)
	}
	else if ((calc_freq >= 20) && (calc_freq < 40))//918
	{
		LoRaSettings.RFFrequency =(uint32_t)( (uint32_t)START_BAND +(uint32_t)(5 * (uint32_t)1000000));//907)
	}
	else if ((calc_freq >= 40) && (calc_freq < 60))//918
	{
		LoRaSettings.RFFrequency =(uint32_t)( (uint32_t)START_BAND +(uint32_t)(10 * (uint32_t)1000000));//912
	}
	else if ((calc_freq >= 60) && (calc_freq < 80))//918
	{
		LoRaSettings.RFFrequency =(uint32_t)( (uint32_t)START_BAND +(uint32_t)(15 * (uint32_t)1000000));//917
	}
	else if ((calc_freq >= 80) && (calc_freq < 100))//918
	{
		LoRaSettings.RFFrequency =(uint32_t)( (uint32_t)START_BAND +(uint32_t)(20 * (uint32_t)1000000));//922
	}
	else
	{
		LoRaSettings.RFFrequency =(uint32_t)( (uint32_t)START_BAND +(uint32_t)(24 * (uint32_t)1000000));//926
	}


	return LoRaSettings.RFFrequency;
}

#endif // USE_SX1276_RADIO
