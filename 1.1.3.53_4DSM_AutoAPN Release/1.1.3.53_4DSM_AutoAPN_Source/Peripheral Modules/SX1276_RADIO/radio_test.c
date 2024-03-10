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
 * \file       main.c
 * \brief      Ping-Pong example application on how to use Semtech's Radio
 *             drivers.
 *
 * \version    2.0
 * \date       Nov 21 2012
 * \author     Miguel Luis
 */
#include "radio_test.h"

#define BUFFER_SIZE                             128 // Define the payload size here

volatile uint8_t 			Duty_Cycle_Mode = FALSE;
volatile uint16_t			Lora_rx_Count = 0;

uint16_t 					BufferSize = BUFFER_SIZE;	// RF buffer size
uint8_t 					Buffer[BUFFER_SIZE];		// RF buffer
uint8_t 					LORA_PWR = FALSE;
uint16_t					cCount = 0;
uint8_t 					SF = 0, BW = 0, PW = 0;
uint16_t 					CF = 0, PL = 0, PKTS = 50;
uint8_t 					Print_Once = true;//,Test_Complete = false;

static uint8_t 				EnableMaster = true; 		// Master/Slave selection

tRadioDriver *Radio = NULL;

const uint8_t 				PingMsg[] = "PING";
const uint8_t 				PongMsg[] = "PONG";

extern volatile uint8_t		SLEEP_ENABLE;
extern volatile uint8_t		BSU_Chunk_received;

extern double 				RxPacketRssiValue;
extern double 				Avg_RxPacketRssiValue,Avg_Transmitter_RSSI_value;
extern uint16_t             glMdmUart_bytes_recvd;
extern uint8_t              glMdmUart_recv_buf[ 1300 ];
extern uint8_t 				TX_COUNT,RX_COUNT;
extern tLoRaSettings 		LoRaSettings1;
extern GPRSSystem      		glSystem_cfg;
extern tLoRaSettings 		LoRaSettings;
extern uint8_t				HARDWARE_REVISION;

/*
 * Manages the master operation
 */

void OnMaster( void )
{
	if(LORA_PWR != TRUE)
	{
		//LoRaSettings1.RFFrequency = (uint32_t)((uint32_t)glSystem_cfg.Active_Channel*(uint32_t)1000000);	//primary communication channel
		LoRaSettings1.RxPacketTimeout = LORA_RESP_TIMEOUT-1;	//20 seconds timeout
		LoRaSettings1.RxSingleOn = 0;
		LORA_PWR = TRUE;
		Init_LORA();
	}
	else
	{
		switch( Radio->Process( ) )
		{
		case RF_RX_TIMEOUT:
			//Debug_TextOut(2, "RX timeout");
			Radio->StartRx( );
			// Send the next PING frame
			break;
		case  RF_TX_TIMEOUT:
			//Debug_TextOut(2, "TX timeout");
			Radio->StartRx( );
			break;
		case RF_RX_DONE:
			//Debug_TextOut(2, "RX Done");
			Lora_rx_Count++;
			Handle_LORA_Response();
			break;
		case RF_TX_DONE:
			//Debug_TextOut(2, "TX Done");
			Radio->StartRx( );
			break;
		default:
			break;
		}
	}
}

/*
 * Manages the slave operation
 */
void OnSlave( void )
{
	if(LORA_PWR != TRUE)
	{
		//LoRaSettings1.RFFrequency = (uint32_t)(uint32_t)(glSystem_cfg.Active_Channel*(uint32_t)1000000);	//wake up channel
		LoRaSettings1.RxPacketTimeout = 1;//LORA_RESP_TIMEOUT-1;	//1 second timeout for wakeup to save power
		LoRaSettings1.RxSingleOn = 1;
		LORA_PWR = TRUE;
		Init_LORA();//TODO:initialize on a different channel for passive wakeup

		if(BSU_Chunk_received == FALSE)
		{
			Radio->StartRx();
			//Debug_TextOut(0,"StartRx1");
		}
	}
	else
	{
		if(Duty_Cycle_Mode == TRUE)
		{
			Duty_Cycle_Mode = FALSE;
			//Radio->Init( );
			if(BSU_Chunk_received == FALSE)
			{
				Radio->StartRx();
			}
		}

		switch(Radio->Process( ))
		{
		case RF_RX_TIMEOUT:
			SLEEP_ENABLE = TRUE;
			if(Duty_Cycle_Mode == FALSE)
			{
				Duty_Cycle_Mode = TRUE;
				SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );
			}
			//Debug_TextOut(0, "RX timeout");
			//Radio->StartRx();
			break;
		case  RF_TX_TIMEOUT:
			//Debug_TextOut(2, "TX timeout");
			SLEEP_ENABLE = TRUE;
			if(Duty_Cycle_Mode == FALSE)
			{
				Duty_Cycle_Mode = TRUE;
				SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );
			}
			break;
		case RF_RX_DONE:
			//Debug_TextOut(2, "RX done");
			Handle_LORA_Response();
			Lora_rx_Count++;
			break;
		case RF_TX_DONE:
			//Debug_TextOut(2, "TX done");
			SLEEP_ENABLE = TRUE;
			if(Duty_Cycle_Mode == FALSE)
			{
				Duty_Cycle_Mode = TRUE;
				SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );
			}
			break;
		default:
			break;
		}
	}

}
void BoardInit( )
{
	//ENABLE POWER TO LORA CHIP
	LORA_power_on();

	//LORA SPI
	LORA_SPI_Init();

	//LORA TX_RX LINE,DEFAULT SET FOR RX
	GPIO_setAsOutputPin(LORA_RXTX_IOPORT,LORA_RXTX_PIN);
	GPIO_setOutputLowOnPin( LORA_RXTX_IOPORT, LORA_RXTX_PIN);

	//HF AND LF ANTENNA CONTROL
	GPIO_setAsOutputPin(LORA_HF_PORT,LORA_HFANT_CNTL);
	GPIO_setOutputLowOnPin( LORA_HF_PORT, LORA_HFANT_CNTL);

	if(HARDWARE_REVISION == LNG_REV3)
	{
		GPIO_setAsOutputPin(LORA_RESET_IOPORT,LORA_LFANT_CNTL);
		GPIO_setOutputLowOnPin( LORA_RESET_IOPORT, LORA_LFANT_CNTL);
	}
	else if(HARDWARE_REVISION == LNG_REV4)
	{
		GPIO_setAsOutputPin(LORA_LFANT_CNTL_PORT, LORA_LFANT_CNTL_PIN);
		GPIO_setOutputLowOnPin(LORA_LFANT_CNTL_PORT, LORA_LFANT_CNTL_PIN);
	}

	//LORA DIO LINE
	GPIO_setAsInputPin(DIO0_IOPORT,DIO0_PIN);
	DelayMs(10);	//10 ms 20Dec18
}
void Init_LORA()
{
	BoardInit( );

	Radio = RadioDriverInit( );

	Radio->Init( );
}
/*
 * Test application entry point.
 */
int test_radio( void )
{
	// while( 1 )
	{
		//if(Test_Complete == false)
		{
			if( EnableMaster == true )
			{
				OnMaster( );
			}
			else
			{
				OnSlave( );
			}
		}
	}
	return 0;
}

uint8_t Validate_LORA_Parametrs()
{
	uint8_t valid_count = 0;
	if(glMdmUart_bytes_recvd >= INPUT_LENGTH)
	{
		if(glMdmUart_recv_buf[0] == ';')//start byte,SPREAD FACTOR
		{
			valid_count++;
			SF = ((glMdmUart_recv_buf[1] - 0X30) *10) + (glMdmUart_recv_buf[2] - 0X30);

			if((SF >= 7) && (SF <= 12))
			{
				valid_count++;
				if(glMdmUart_recv_buf[3] == ';')//BANDWIDTH
				{
					valid_count++;
					BW = ((glMdmUart_recv_buf[4] - 0X30) *10) + (glMdmUart_recv_buf[5] - 0X30);
					if((BW >= 7) && (BW <= 9 ))
					{
						valid_count++;
						if(glMdmUart_recv_buf[6] == ';')//POWER
						{
							valid_count++;
							PW = ((glMdmUart_recv_buf[7] - 0X30) *10) + (glMdmUart_recv_buf[8] - 0X30);
							if((PW >= 1) && (PW <= 20 ))
							{
								valid_count++;
								if(glMdmUart_recv_buf[9] == ';')//CF
								{
									valid_count++;
									CF = ((glMdmUart_recv_buf[10] - 0X30) *100) + ((glMdmUart_recv_buf[11] - 0X30) *10) + (glMdmUart_recv_buf[12] - 0X30);
									if((CF >= 430) && (CF <= 920))
									{
										valid_count++;
										if(glMdmUart_recv_buf[13] == ';')//PAYLOAD
										{
											valid_count++;
											PL = ((glMdmUart_recv_buf[14] - 0X30) *100) + ((glMdmUart_recv_buf[15] - 0X30) *10) + (glMdmUart_recv_buf[16] - 0X30);
											if((PL >= 9) && (PL <= 128))
											{
												valid_count++;
												if(glMdmUart_recv_buf[17] == ';')//NO OF TEST PACKETS
												{
													valid_count++;
													PKTS = ((glMdmUart_recv_buf[18] - 0X30) *100) + ((glMdmUart_recv_buf[19] - 0X30) *10) + (glMdmUart_recv_buf[20] - 0X30);
													if((PKTS > 0) && (PKTS <= 255))
													{
														valid_count++;
														if(glMdmUart_recv_buf[21] == ';')
														{
															valid_count++;
														}

													}
												}
											}
										}
									}

								}
							}
						}
					}
				}
			}
			if(valid_count >= 12)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

