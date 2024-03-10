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
 * \file       sx1276.h
 * \brief      SX1276 RF chip driver
 *
 * \version    2.0.B2 
 * \date       May 6 2013
 * \author     Gregory Cristian
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
#ifndef __SX1276_H__
#define __SX1276_H__

#define REG_OPMODE                                  0x01

typedef struct sSX1276
{
    uint8_t RegFifo;                                // 0x00
    // Common settings
    uint8_t RegOpMode;                              // 0x01
    uint8_t RegBitrateMsb;                          // 0x02
    uint8_t RegBitrateLsb;                          // 0x03
    uint8_t RegFdevMsb;                             // 0x04
    uint8_t RegFdevLsb;                             // 0x05
    uint8_t RegFrfMsb;                              // 0x06
    uint8_t RegFrfMid;                              // 0x07
    uint8_t RegFrfLsb;                              // 0x08
    // Tx settings
    uint8_t RegPaConfig;                            // 0x09
    uint8_t RegPaRamp;                              // 0x0A
    uint8_t RegOcp;                                 // 0x0B
    // Rx settings
    uint8_t RegLna;                                 // 0x0C
    uint8_t RegRxConfig;                            // 0x0D
    uint8_t RegRssiConfig;                          // 0x0E
    uint8_t RegRssiCollision;                       // 0x0F
    uint8_t RegRssiThresh;                          // 0x10
    uint8_t RegRssiValue;                           // 0x11
    uint8_t RegRxBw;                                // 0x12
    uint8_t RegAfcBw;                               // 0x13
    uint8_t RegOokPeak;                             // 0x14
    uint8_t RegOokFix;                              // 0x15
    uint8_t RegOokAvg;                              // 0x16
    uint8_t RegRes17;                               // 0x17
    uint8_t RegRes18;                               // 0x18
    uint8_t RegRes19;                               // 0x19
    uint8_t RegAfcFei;                              // 0x1A
    uint8_t RegAfcMsb;                              // 0x1B
    uint8_t RegAfcLsb;                              // 0x1C
    uint8_t RegFeiMsb;                              // 0x1D
    uint8_t RegFeiLsb;                              // 0x1E
    uint8_t RegPreambleDetect;                      // 0x1F
    uint8_t RegRxTimeout1;                          // 0x20
    uint8_t RegRxTimeout2;                          // 0x21
    uint8_t RegRxTimeout3;                          // 0x22
    uint8_t RegRxDelay;                             // 0x23
    // Oscillator settings
    uint8_t RegOsc;                                 // 0x24
    // Packet handler settings
    uint8_t RegPreambleMsb;                         // 0x25
    uint8_t RegPreambleLsb;                         // 0x26
    uint8_t RegSyncConfig;                          // 0x27
    uint8_t RegSyncValue1;                          // 0x28
    uint8_t RegSyncValue2;                          // 0x29
    uint8_t RegSyncValue3;                          // 0x2A
    uint8_t RegSyncValue4;                          // 0x2B
    uint8_t RegSyncValue5;                          // 0x2C
    uint8_t RegSyncValue6;                          // 0x2D
    uint8_t RegSyncValue7;                          // 0x2E
    uint8_t RegSyncValue8;                          // 0x2F
    uint8_t RegPacketConfig1;                       // 0x30
    uint8_t RegPacketConfig2;                       // 0x31
    uint8_t RegPayloadLength;                       // 0x32
    uint8_t RegNodeAdrs;                            // 0x33
    uint8_t RegBroadcastAdrs;                       // 0x34
    uint8_t RegFifoThresh;                          // 0x35
    // Sequencer settings
    uint8_t RegSeqConfig1;                          // 0x36
    uint8_t RegSeqConfig2;                          // 0x37
    uint8_t RegTimerResol;                          // 0x38
    uint8_t RegTimer1Coef;                          // 0x39
    uint8_t RegTimer2Coef;                          // 0x3A
    // Service settings
    uint8_t RegImageCal;                            // 0x3B
    uint8_t RegTemp;                                // 0x3C
    uint8_t RegLowBat;                              // 0x3D
    // Status
    uint8_t RegIrqFlags1;                           // 0x3E
    uint8_t RegIrqFlags2;                           // 0x3F
    // I/O settings
    uint8_t RegDioMapping1;                         // 0x40
    uint8_t RegDioMapping2;                         // 0x41
    // Version
    uint8_t RegVersion;                             // 0x42
    // Additional settings
    uint8_t RegTestReserved43;                        // 0x43

    // Additional settings
    uint8_t RegPllHop;                              // 0x44

    uint8_t RegTestReserved45;                        // 0x45-4B
    uint8_t RegTestReserved46;
    uint8_t RegTestReserved47;
    uint8_t RegTestReserved48;
    uint8_t RegTestReserved49;
    uint8_t RegTestReserved4A;

    uint8_t RegTcxo;                                // 0x4B

    uint8_t RegTestReserved4C;                      // 0x4C
    uint8_t RegPaDac;                               // 0x4D
    // Test
    uint8_t RegTestReserved4E;                      // 0x4E-0x5B
    uint8_t RegTestReserved4F;
    uint8_t RegTestReserved50;
    uint8_t RegTestReserved51;
    uint8_t RegTestReserved52;
    uint8_t RegTestReserved53;                      // 0x4E-0x5B
       uint8_t RegTestReserved54;
       uint8_t RegTestReserved55;
       uint8_t RegTestReserved56;
       uint8_t RegTestReserved57;
       uint8_t RegTestReserved58;                      // 0x4E-0x5B
	  uint8_t RegTestReserved59;
	  uint8_t RegTestReserved5A;
	  uint8_t RegFormerTemp;						//5B
	  uint8_t RegTestReserved5C;
	  uint8_t RegBitrateFrac;                         // 0x5D

    // Test
    uint8_t RegTestReserved5E;                      // 0x59
    // Test
    uint8_t RegTestReserved5F;                      // 0x5B
    // Additional settings
    uint8_t RegTestReserved60;                      // 0x4E-0x5B
        uint8_t RegTestReserved61;
        uint8_t RegTestReserved62;
        uint8_t RegTestReserved63;
        uint8_t RegTestReserved64;
        uint8_t RegTestReserved65;                      // 0x4E-0x5B
           uint8_t RegTestReserved66;
           uint8_t RegTestReserved67;
           uint8_t RegTestReserved68;
           uint8_t RegTestReserved69;
           uint8_t RegTestReserved6A;                      // 0x4E-0x5B
    	  uint8_t RegTestReserved6B;
    	  uint8_t RegTestReserved6C;
          uint8_t RegTestReserved6D;                      // 0x4E-0x5B
   	  uint8_t RegTestReserved6E;
   	  uint8_t RegTestReserved6F;
   	uint8_t RegTestReserved70;

   	   uint8_t RegTestReserved71;
   	   uint8_t RegTestReserved72;
   	   uint8_t RegTestReserved73;
   	   uint8_t RegTestReserved74;
   	   uint8_t RegTestReserved75;
   	   uint8_t RegTestReserved76;
   	   uint8_t RegTestReserved77;
   	   uint8_t RegTestReserved78;
   	   uint8_t RegTestReserved79;
   	   uint8_t RegTestReserved7A;
   	   uint8_t RegTestReserved7B;
   	   uint8_t RegTestReserved7C;
   	   uint8_t RegTestReserved7D;
   	   uint8_t RegTestReserved7E;
   	   uint8_t RegTestReserved7F;

}tSX1276;

extern tSX1276* SX1276;
/*!
 * \brief SX1276 registers array
 */
extern uint8_t SX1276Regs[0x80];

/*!
 * \brief Enables LoRa modem or FSK modem
 *
 * \param [IN] opMode New operating mode
 */
void SX1276SetLoRaOn( bool enable );

/*!
 * \brief Gets the LoRa modem state
 *
 * \retval LoraOn Current LoRa modem mode
 */
bool SX1276GetLoRaOn( void );

/*!
 * \brief Initializes the SX1276
 */
void SX1276Init( void );

/*!
 * \brief Resets the SX1276
 */
void SX1276Reset( void );

/*!
 * \brief Sets the SX1276 operating mode
 *
 * \param [IN] opMode New operating mode
 */
void SX1276SetOpMode( uint8_t opMode );

/*!
 * \brief Gets the SX1276 operating mode
 *
 * \retval opMode Current operating mode
 */
uint8_t SX1276GetOpMode( void );

/*!
 * \brief Reads the current Rx gain setting
 *
 * \retval rxGain Current gain setting
 */
uint8_t SX1276ReadRxGain( void );

/*!
 * \brief Trigs and reads the current RSSI value
 *
 * \retval rssiValue Current RSSI value in [dBm]
 */
double SX1276ReadRssi( void );

/*!
 * \brief Gets the Rx gain value measured while receiving the packet
 *
 * \retval rxGainValue Current Rx gain value
 */
uint8_t SX1276GetPacketRxGain( void );

/*!
 * \brief Gets the SNR value measured while receiving the packet
 *
 * \retval snrValue Current SNR value in [dB]
 */
int8_t SX1276GetPacketSnr( void );

/*!
 * \brief Gets the RSSI value measured while receiving the packet
 *
 * \retval rssiValue Current RSSI value in [dBm]
 */
double SX1276GetPacketRssi( void );

/*!
 * \brief Gets the AFC value measured while receiving the packet
 *
 * \retval afcValue Current AFC value in [Hz]
 */
uint32_t SX1276GetPacketAfc( void );

/*!
 * \brief Sets the radio in Rx mode. Waiting for a packet
 */
void SX1276StartRx( void );

/*!
 * \brief Gets a copy of the current received buffer
 *
 * \param [IN]: buffer     Buffer pointer
 * \param [IN]: size       Buffer size
 */
void SX1276GetRxPacket( void *buffer, uint16_t *size );

/*!
 * \brief Sets a copy of the buffer to be transmitted and starts the
 *        transmission
 *
 * \param [IN]: buffer     Buffer pointer
 * \param [IN]: size       Buffer size
 */
void SX1276SetTxPacket( const void *buffer, uint16_t size );

/*!
 * \brief Gets the current RFState
 *
 * \retval rfState Current RF state [RF_IDLE, RF_BUSY, 
 *                                   RF_RX_DONE, RF_RX_TIMEOUT,
 *                                   RF_TX_DONE, RF_TX_TIMEOUT]
 */
uint8_t SX1276GetRFState( void );

/*!
 * \brief Sets the new state of the RF state machine
 *
 * \param [IN]: state New RF state machine state
 */
void SX1276SetRFState( uint8_t state );

/*!
 * \brief Process the Rx and Tx state machines depending on the
 *       SX1276 operating mode.
 *
 * \retval rfState Current RF state [RF_IDLE, RF_BUSY, 
 *                                   RF_RX_DONE, RF_RX_TIMEOUT,
 *                                   RF_TX_DONE, RF_TX_TIMEOUT]
 */
uint32_t SX1276Process( void );

#endif //__SX1276_H__
