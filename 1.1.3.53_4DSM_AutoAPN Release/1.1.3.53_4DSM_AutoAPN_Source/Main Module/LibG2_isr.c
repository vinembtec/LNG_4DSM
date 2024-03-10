//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_isr.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_isr_api
//! @{
//
//*****************************************************************************
#include "LibG2_isr.h"

extern volatile uint8_t		  	wait_counter,Max_Amount_In_current_rate;
extern volatile uint32_t		parking_time_left_on_meter[MSM_MAX_PARKING_BAYS] ;
//extern volatile uint8_t 		current_rate_index;
extern volatile uint32_t		negetive_parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];
extern volatile uint8_t  		Asynchronous_FD;
extern volatile uint8_t 		current_rate_index;

extern uint8_t 				  	glMdmUart_recv_buf[ GPRS_GEN_BUF_SZ_VLARGE ];
extern uint16_t        		  	DFG_filled_resp_len;
extern uint8_t                	glDFG_response[GPRS_GEN_BUF_SZ_VLARGE];
extern uint8_t 				  	key_press;
//extern uint8_t					Waiting_4GPRS_Resp;//not used in this program //vinay
//extern uint8_t 			        Queue_SendReceive;//used only here so declaring it here only //vinay
uint8_t  						Queue_SendReceive = false;
extern uint16_t			        glMdmUart_bytes_recvd;
extern uint8_t             		mdm_comm_status, show_balance_and_deduct, SC_Verify_Update;
extern uint8_t 					Connected_Card_reader,COMMS_DOWN,SMS_MODE;
extern uint8_t 					/*waiting_for_batch_response,*/ Waiting_4LORA_Resp;//not used in this program //vinay
//extern Tech_Menu_Disp   		tech_menu_disp;//not used in this program //vinay
extern uint8_t					card_removed_flag, Tech_menu;
//extern uint8_t             		rx_ans[ GPRS_GEN_BUF_SZ_SMALL ];//not used in this program //vinay
extern uint16_t					Current_Space_Id;
extern GPRSSystem      		    glSystem_cfg;
extern uint8_t              	PBC_expt_rcvd[4];
extern uint8_t              	PBC_amnt_rcvd[2];
//extern tLoRaSettings 			LoRaSettings; //not using in this program //vinay
extern uint8_t 					LORA_PWR;
extern uint32_t 				CC_TimeOut_RTCTimestamp;
extern uint8_t					Be4Bootup;
extern uint8_t         			MDM_AWAKE;

extern uint8_t                  grace_time_trigger[MSM_MAX_PARKING_BAYS];
//extern uint8_t                  grace_timeout;//not used in this program //vinay
//extern uint8_t                  one_time_loop_flag;//not used in this program //vinay
extern uint8_t                  real_payment_flag[MSM_MAX_PARKING_BAYS];
extern uint8_t					ANTI_FEED_in_effect[MSM_MAX_PARKING_BAYS];

extern uint32_t					LORA_PWR_On_Time;

volatile uint32_t 				temp;
//volatile uint32_t 				IntDegF;//not used in this program //vinay
//volatile uint32_t 				IntDegC;//not used in this program //vinay
//volatile uint8_t				wdt_base_timer_counter;//not used in this program //vinay
volatile uint8_t				SLEEP_ENABLE = FALSE;

uint32_t 						Tasks_Priority_Register = 0;
uint32_t 						DFG_content_length = 0;
uint8_t	 					  	temp_buff[11];//it is simply declared as global it can be made local//vinay
volatile uint16_t				wdt_base_Counter = 0, Seconds_Counter = 0;
uint8_t 						Parking_Clock_Refresh[MSM_MAX_PARKING_BAYS] = {0,0}/*PARKING_CLOCK_STOP_MODE*/, Running_Space_ID[MSM_MAX_PARKING_BAYS],expire[MSM_MAX_PARKING_BAYS];//LNGSIT-2007 //vinay
uint8_t 						SC_bytes = 0,Event = 0;
uint8_t 						RXData[MAX_SCT_RX_BYTES];
uint8_t 						bufferFlag = 0,smart_card_mode = FALSE,scosta_smartcard_mode = FALSE;
uint8_t 						glSCReadBuf[MAX_SCT_RX_BYTES];

uint8_t 						*TData;
uint8_t							RData[500];
uint16_t 						Eof,RXIndex;
uint8_t 						RespIndex;
//uint8_t 						CoinTimeEvent=0;//not used in this program //vinay
uint8_t 						/*Active_LED = RED_LED, */LED_selected_in_config = FRONT_AND_REAR_ENF_LED;
uint8_t							LED_ON = false, LED_ON_Count = 0;

char             				special_keys[6], special_key_count = 0;
uint8_t  						Quick_meter_status_request = FALSE, Tech_key_seq = FALSE;

uint16_t						pbc_amount_recvd = 0;
uint32_t						pbc_expiry_time_recvd = 0;
//uint16_t						count_20times = 0;//not used in this program //vinay

//uint32_t                        One_min_tick = 0;//not used in this program //vinay

uint32_t          				LORA_PWR_OFF_Time_DC = 0,LORA_PWR_On_Time_DC = 0,Total_LORA_PWR_On_Time_DC = 0 ;

extern uint8_t					HARDWARE_REVISION, REV4_LED_CONFIG,Front_Bi_Color;
//uint8_t 						LED_SPACE, LEFT = 1, RIGHT = 2;
uint8_t 						LED_SPACE_REAR = 0, LEFT_REAR = 0, RIGHT_REAR = 1, LED_SPACE_FRONT = 0, LEFT_FRONT = 0, RIGHT_FRONT = 1;
extern volatile uint8_t			wakeup_skip;
extern volatile uint8_t 		second_check;
uint32_t						Last_CC_Payment_RTC = 0;
extern uint8_t 					Modem_Test_Flag;
/**************************************************************************/
//! Interrupt Service Routine (ISR) for Timer A0
//! \param void
//! \return void
/**************************************************************************/
void TA0_0_IRQHandler()
{
	Exit_from_LPM();
	//Timer_A_clearInterruptFlag(TIMER_A0_BASE);
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for Timer A1
//! \param void
//! \return void
/**************************************************************************/
void TA1_0_IRQHandler()
{
	Exit_from_LPM();
	//Tasks_Priority_Register |= TIMER_A1_ISR;
	Timer_A_clearInterruptFlag(TIMER_A1_BASE);
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for Timer A2
//! \param void
//! \return void
/**************************************************************************/
void TA2_0_IRQHandler()
{
	Exit_from_LPM();
	//Tasks_Priority_Register |= INTERVALS_ISR;
	Timer_A_clearInterruptFlag(TIMER_A2_BASE);
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for Timer A3
//! \param void
//! \return void
/**************************************************************************/
void TA3_0_IRQHandler()
{
	Exit_from_LPM();
	//Tasks_Priority_Register |= TIMER_A3_ISR;
	Timer_A_clearInterruptFlag(TIMER_A3_BASE);
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for EUSCI A0. This is used to receive
//! bytes from GPRS Modem
//! \param void
//! \return void
/**************************************************************************/
void EUSCIA0_IRQHandler()
{
	Exit_from_LPM();

	WDT_A_holdTimer();
	WDT_A_clearTimer();

	//Tasks_Priority_Register |= MODEM_UART_ISR;
	UART_clearInterruptFlag(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);

	uint8_t tx_char = UCA0RXBUF;

	if(glMdmUart_bytes_recvd >= GPRS_GEN_BUF_SZ_VLARGE)
		glMdmUart_bytes_recvd = 0;
	glMdmUart_recv_buf[glMdmUart_bytes_recvd++] = tx_char;

	if((Asynchronous_FD == TRUE) && (Modem_Test_Flag == FALSE))
	{
		if(glMdmUart_bytes_recvd >=10)
		{
			if(glMdmUart_bytes_recvd == 10)
			{
				DFG_content_length =(uint32_t)(glMdmUart_recv_buf[6]|(glMdmUart_recv_buf[7]<<8)|((glMdmUart_recv_buf[8] & 0xffffffff)<<16)|((glMdmUart_recv_buf[9] & 0xffffffff)<<24));
				if(DFG_content_length > GPRS_GEN_BUF_SZ_VLARGE)
					DFG_content_length = 0;
			}
			else if((glMdmUart_bytes_recvd >= (DFG_content_length+10)) /*&& (glMdmUart_bytes_recvd >10) */&&(DFG_content_length>0) && (DFG_content_length<(GPRS_GEN_BUF_SZ_VLARGE-10)))
			{
				memset(glDFG_response,0,GPRS_GEN_BUF_SZ_VLARGE);
				memcpy(glDFG_response,glMdmUart_recv_buf,glMdmUart_bytes_recvd);
				DFG_filled_resp_len = glMdmUart_bytes_recvd;
				glMdmUart_bytes_recvd = 0;
				memset(glMdmUart_recv_buf,0,GPRS_GEN_BUF_SZ_VLARGE);
				gprs_do_dfg();
				DFG_content_length = 0;
				wait_counter = 0;
			}
			else if(glMdmUart_bytes_recvd >= (GPRS_GEN_BUF_SZ_VLARGE-10))
			{
				memset(glDFG_response,0,GPRS_GEN_BUF_SZ_VLARGE);
				memset(glMdmUart_recv_buf,0,GPRS_GEN_BUF_SZ_VLARGE);
				glMdmUart_bytes_recvd = 0;
			}
			else if((glMdmUart_recv_buf[glMdmUart_bytes_recvd-9]==GPRS_COMM_lOB_DFG) && (glMdmUart_recv_buf[glMdmUart_bytes_recvd-10]==0)&&(glMdmUart_recv_buf[glMdmUart_bytes_recvd-8]==0xC8) && (glMdmUart_recv_buf[glMdmUart_bytes_recvd-7]==0)
					&&(glMdmUart_recv_buf[glMdmUart_bytes_recvd-6]==0xC8) && (glMdmUart_recv_buf[glMdmUart_bytes_recvd-5]==0))
			{
				memcpy(temp_buff,&glMdmUart_recv_buf[glMdmUart_bytes_recvd-10],10);
				memcpy(glMdmUart_recv_buf,temp_buff,10);
				glMdmUart_bytes_recvd = 10;
				DFG_content_length =(uint32_t)(glMdmUart_recv_buf[6]|(glMdmUart_recv_buf[7]<<8)|((glMdmUart_recv_buf[8] & 0xffffffff)<<16)|((glMdmUart_recv_buf[9] & 0xffffffff)<<24));
				if(DFG_content_length > GPRS_GEN_BUF_SZ_VLARGE)
					DFG_content_length = 0;
			}
		}
	}
	else if((mdm_comm_status == 5) && (Queue_SendReceive!= true ) && (Modem_Test_Flag == FALSE))//General asynchronous response handling
	{
		if(glMdmUart_bytes_recvd == 10)
		{
			DFG_content_length =(uint32_t)(glMdmUart_recv_buf[6]|(glMdmUart_recv_buf[7]<<8)|((glMdmUart_recv_buf[8] & 0xffffffff)<<16)|((glMdmUart_recv_buf[9] & 0xffffffff)<<24));
			if(DFG_content_length > GPRS_GEN_BUF_SZ_VLARGE)
			{
				DFG_content_length = 0;
				glMdmUart_bytes_recvd = 0;
				memset(glMdmUart_recv_buf,0,GPRS_GEN_BUF_SZ_VLARGE);
			}
		}

		if((glMdmUart_bytes_recvd >= 23) && (glMdmUart_recv_buf[1] == GPRS_COMM_PAMPushMsg))
		{
			Debug_TextOut( 0, "Push Msg Recvd" );
			memset(PBC_expt_rcvd, 0, 4);
			memset(PBC_amnt_rcvd, 0, 2);

			memcpy(PBC_expt_rcvd, &glMdmUart_recv_buf[12], 4);
			memcpy(PBC_amnt_rcvd, &glMdmUart_recv_buf[21], 2);

			gprs_push_notify();
			Parse_PBC_Payment();
		}
		else if(glMdmUart_bytes_recvd >= (DFG_content_length+10))
		{
			memset(glDFG_response,0,GPRS_GEN_BUF_SZ_VLARGE);
			memcpy(glDFG_response,glMdmUart_recv_buf,glMdmUart_bytes_recvd);
			DFG_filled_resp_len = glMdmUart_bytes_recvd;
			glMdmUart_bytes_recvd = 0;
			memset(glMdmUart_recv_buf,0,GPRS_GEN_BUF_SZ_VLARGE);
			//if((Waiting_4GPRS_Resp == TRUE) || (waiting_for_batch_response == TRUE))
			{
				Handle_GPRS_Response();
			}
			DFG_content_length = 0;
		}
		else if(glMdmUart_bytes_recvd >= (GPRS_GEN_BUF_SZ_VLARGE-10))
		{
			memset(glDFG_response,0,GPRS_GEN_BUF_SZ_VLARGE);
			memset(glMdmUart_recv_buf,0,GPRS_GEN_BUF_SZ_VLARGE);
			glMdmUart_bytes_recvd = 0;
		}
	}

	WDT_A_startTimer();
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for EUSCI A1
//! \param void
//! \return void
/**************************************************************************/
void EUSCIA1_IRQHandler()
{
	Exit_from_LPM();
	//Tasks_Priority_Register |= EUSCIA1_ISR;
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for EUSCI A2
//! \param void
//! \return void
/**************************************************************************/
void EUSCIA2_IRQHandler()
{
	Exit_from_LPM();
	//Tasks_Priority_Register |= EUSCIA2_ISR;
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for EUSCI A3. This is used to receive
//! data from the Smart Card and IDTech Reader
//! \param void
//! \return void
/**************************************************************************/
void EUSCIA3_IRQHandler()
{
	Exit_from_LPM();

	WDT_A_holdTimer();
	WDT_A_clearTimer();

	//Tasks_Priority_Register |= EUSCIA3_ISR;
	//static int j=0;
	UART_clearInterruptFlag(EUSCI_A3_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);

	if(Connected_Card_reader == GEM_CLUB_READER)
	{
		RXData[SC_bytes++] = (MAP_UART_receiveData(EUSCI_A3_BASE));
		if((SC_bytes > bufferFlag) || ((SC_bytes >= 1) && (SC_Verify_Update == TRUE)))
		{
			//GemClub ATR: 0x3B 0x02 0x53 0x01
			//SCOSTA ATR:  0x3B 0x9D 0x94 0x40 and 0x3B 0xDA 0x95 0x00
			if ((RXData[0] == 0x3B) && (RXData[1] == 0x02))
			{
				//Start CC TimeOut Time
				CC_TimeOut_RTCTimestamp = RTC_epoch_now();
				smart_card_mode = TRUE;
				smart_card.detected = TRUE;
				show_balance_and_deduct = 0;
				Max_Amount_In_current_rate = FALSE;
				//Debug_Output6(2,"GemClub:%02X, %02X, %02X, %02X, %02X, %02X", SC_bytes, RXData[0], RXData[1], RXData[2], RXData[3], RXData[4]);
			}
			else if((RXData[0]== 0x3B) && (RXData[1]== 0x9D)|| (RXData[0]== 0x3B) && (RXData[1]== 0xDA))
			{
				//Start CC TimeOut Time
				CC_TimeOut_RTCTimestamp = RTC_epoch_now();
				smart_card_mode = TRUE;
				smart_card.detected = TRUE;
				scosta_smartcard_mode = TRUE;
				//Debug_Output6(2,"Scosta:%02X, %02X, %02X, %02X, %02X, %02X", SC_bytes, RXData[0], RXData[1], RXData[2], RXData[3], RXData[4]);
			}

			SC_bytes=0;
			memcpy(glSCReadBuf,RXData,MAX_SCT_RX_BYTES);
			//memset(RXData,0,sizeof(RXData));

			if(scosta_smartcard_mode == TRUE)
			{
				if(glSCReadBuf[0] == SMARTCARD_STATUS_OK)
				{
					Event = 1;
					//Debug_Output6(2,"Scosta Resp-01:%02X, %02X, %02X, %02X, %02X, %02X", glSCReadBuf[0], glSCReadBuf[1], glSCReadBuf[2], glSCReadBuf[3], glSCReadBuf[4], glSCReadBuf[5]);
				}
				else if(glSCReadBuf[5] == SMARTCARD_STATUS_OK)
				{
					Event = 1;
					//Debug_Output6(2,"Scosta Resp-02:%02X, %02X, %02X, %02X, %02X, %02X", glSCReadBuf[0], glSCReadBuf[1], glSCReadBuf[2], glSCReadBuf[3], glSCReadBuf[4], glSCReadBuf[5]);
				}
				else if((glSCReadBuf[0] == SCOSTA_ACTIVATE_FILE) && (glSCReadBuf[1] == SMARTCARD_STATUS_OK))
				{
					Event = 1;
					//Debug_Output6(2,"Scosta Resp-03:%02X, %02X, %02X, %02X, %02X, %02X", glSCReadBuf[0], glSCReadBuf[1], glSCReadBuf[2], glSCReadBuf[3], glSCReadBuf[4], glSCReadBuf[5]);
				}
				else if((glSCReadBuf[0] == SCOSTA_SELECT_FILE) && (glSCReadBuf[1] == SMARTCARD_STATUS_OK))
				{
					Event = 1;
					//Debug_Output6(2,"Scosta Resp-04:%02X, %02X, %02X, %02X, %02X, %02X", glSCReadBuf[0], glSCReadBuf[1], glSCReadBuf[2], glSCReadBuf[3], glSCReadBuf[4], glSCReadBuf[5]);
				}
				else if((glSCReadBuf[0] == SCOSTA_READ_RECORD) /*&& (glSCReadBuf[1] == SMARTCARD_STATUS_OK)*/)
				{
					Event = 1;
					//Debug_Output6(2,"Scosta Resp-05:%02X, %02X, %02X, %02X, %02X, %02X", glSCReadBuf[0], glSCReadBuf[1], glSCReadBuf[2], glSCReadBuf[3], glSCReadBuf[4], glSCReadBuf[5]);
				}
				else
				{
					//Event = 0;
					//Debug_Output6(2,"Scosta Resp-06:%02X, %02X, %02X, %02X, %02X, %02X", glSCReadBuf[0], glSCReadBuf[1], glSCReadBuf[2], glSCReadBuf[3], glSCReadBuf[4], glSCReadBuf[5]);
				}
			}
			else
			{
				if(glSCReadBuf[0] == SMARTCARD_STATUS_OK)
				{
					Event = 1;
					//Debug_Output6(2,"GemClub Resp-01:%02X, %02X, %02X, %02X, %02X, %02X", glSCReadBuf[0], glSCReadBuf[1], glSCReadBuf[2], glSCReadBuf[3], glSCReadBuf[4], glSCReadBuf[5]);
				}
				else if(glSCReadBuf[0] == SMARTCARD_INS_VERIFY)
				{
					Event = 1;
					//Debug_Output6(2,"GemClub Resp-02:%02X, %02X, %02X, %02X, %02X, %02X", glSCReadBuf[0], glSCReadBuf[1], glSCReadBuf[2], glSCReadBuf[3], glSCReadBuf[4], glSCReadBuf[5]);
				}
				else if(glSCReadBuf[0] == SMARTCARD_INS_UPDATE)
				{
					Event = 1;
					//Debug_Output6(2,"GemClub Resp-03:%02X, %02X, %02X, %02X, %02X, %02X", glSCReadBuf[0], glSCReadBuf[1], glSCReadBuf[2], glSCReadBuf[3], glSCReadBuf[4], glSCReadBuf[5]);
				}
				else if(glSCReadBuf[5] == SMARTCARD_STATUS_OK)
				{
					Event = 1;
					//Debug_Output6(2,"GemClub Resp-04:%02X, %02X, %02X, %02X, %02X, %02X", glSCReadBuf[0], glSCReadBuf[1], glSCReadBuf[2], glSCReadBuf[3], glSCReadBuf[4], glSCReadBuf[5]);
				}
				else
				{
					Event = 0;
					//Debug_Output6(2,"GemClub Resp-05:%02X, %02X, %02X, %02X, %02X, %02X", glSCReadBuf[0], glSCReadBuf[1], glSCReadBuf[2], glSCReadBuf[3], glSCReadBuf[4], glSCReadBuf[5]);
				}
			}
		}
	}
	else
	{
		RData[RXIndex++] = MAP_UART_receiveData(EUSCI_A3_BASE);
		if((RData[1]+6)==RXIndex)
		{
			RXIndex=0;
			//memcpy(RxData,RXData,(RXData[1]+6));
			RespIndex++;
		}
	}
	WDT_A_startTimer();
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for EUSCI B0.
//! \param void
//! \return void
/**************************************************************************/
void EUSCIB0_IRQHandler()
{
	Exit_from_LPM();
	//Tasks_Priority_Register |= EUSCIB0_ISR;
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for EUSCI B1.
//! \param void
//! \return void
/**************************************************************************/
void EUSCIB1_IRQHandler()
{
	Exit_from_LPM();
	//Tasks_Priority_Register |= EUSCIB1_ISR;
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for EUSCI B2.
//! \param void
//! \return void
/**************************************************************************/
void EUSCIB2_IRQHandler()
{
	Exit_from_LPM();
	//Tasks_Priority_Register |= EUSCIB2_ISR;
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for EUSCI B3.
//! \param void
//! \return void
/**************************************************************************/
void EUSCIB3_IRQHandler()
{
	Exit_from_LPM();
	//Tasks_Priority_Register |= EUSCIB3_ISR;
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for 14-bit ADC module.
//! \param void
//! \return void
/**************************************************************************/
void ADC14_IRQHandler()
{
	Exit_from_LPM();
	//Tasks_Priority_Register |= ADC_ISR;
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for RTC Module. RTC is configured to
//! generate interrupt on minutes change. A task is marked in Tasks_Priority_Register
//! in the ISR, then the actual processing happens in the outer layer Statemachine
//! \param void
//! \return void
/**************************************************************************/
void RTC_C_IRQHandler()
{
	Exit_from_LPM();
	watchdog_pat();
	//Debug_TextOut(0,"1");
	Tasks_Priority_Register |= RTC_ISR;
	RTC_C_clearInterruptFlag(RTC_C_TIME_EVENT_INTERRUPT);
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for PORT1
//! \param void
//! \return void
/**************************************************************************/
void PORT1_IRQHandler()
{
	Exit_from_LPM();
	GPIO_clearInterruptFlag(IDTECH_INTR_PORT, IDTECH_INTR_PIN);
	//Tasks_Priority_Register |= MAGTEK_CARD_ISR;
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for PORT2. This is used for Keypads. The
//! ISR identifies which key is pressed and takes action accordingly.
//! \param void
//! \return void
/**************************************************************************/
void PORT2_IRQHandler()
{
	char        keys_local[6];

	Exit_from_LPM();
	Tasks_Priority_Register |= KEYPADS_ISR;
	volatile static uint32_t status;
	key_press = 0;
	key_press = discriminate_which_key();

	if((key_press != KEY_UNKNOWN) && (card_removed_flag==false) && (smart_card_mode==false) && (Tech_menu==false)/*&&(UI_for_dataextraction_key==false)*/)
	{
		if(special_key_count<5)
		{
			switch ( key_press )//up arrow
			{
			case KEY_UP:
				special_keys[special_key_count]=KEY_UP+0x30;
				special_key_count++;
				break;

			case KEY_DOWN://down arrow
				special_keys[special_key_count]=KEY_DOWN+0x30;
				special_key_count++;
				break;

			case KEY_CANCEL: //cancel key
				special_keys[special_key_count]=KEY_CANCEL+0x30;
				special_key_count++;
				break;

			case KEY_OK://key ok
				special_keys[special_key_count]=KEY_OK+0x30;
				special_key_count++;
				if(glSystem_cfg.Special_feature_enable == TRUE)	//Moorestown requirement, if OK key pressed on idle, add 12 minutes parking time
				{
					if(parking_time_left_on_meter[Current_Space_Id] < (glSystem_cfg.special_feature_free_minutes*60))
					{
						if(ANTI_FEED_in_effect[Current_Space_Id] == true)	//Anti Feed feature enabled in config
						{
							//Debug_TextOut(0,"Anti Feed, No parking time earned");
							Debug_Output1(0,"Anti Feed No parking time earned for space [%d]",Current_Space_Id);
							AntiFeed_Screen();
						}
						else
						{ 	//LNGSIT-913
				        	if(current_rate_index == NO_PARKING)
				        	{
				        		Noparking_Screen();
				        		Debug_TextOut( 0, "PA-C:no_parking" );
				        	}
				        	else if(current_rate_index == FREE_PARKING)
				        	{
				        		Freeparking_Screen();
				        		Debug_TextOut( 0, "PA-C:free_parking" );
				        	}
				        	else
				        	{
				        		Debug_TextOut(0,"Add time on OK key press");
				        		parking_time_left_on_meter[Current_Space_Id] = glSystem_cfg.special_feature_free_minutes*60;
				        		push_event_to_cache(UDP_EVTTYP_SPECIAL_FREE_TIME);
				        	}
						}
					}
				}
				break;
			}
		}
//#if 1
		if(special_key_count>=5)
		{
			special_key_count=0;
			sprintf((char*)keys_local,"%s", special_keys);
			sprintf((char*)special_keys,"%s", SPECIAL_KEYS_ENTER_TECH_MENU);
			if( (strncmp(keys_local,special_keys,5)==0) && (Tech_menu==false) )
			{
				Tech_menu=true;
				Tech_key_seq = TRUE;
				Debug_TextOut( 0, "LNG is in tech mode" );
				push_event_to_cache( UDP_EVTTYP_TECH_MENU_ACCESS );
				TechMenu_Display_Main_Menu();
			}
			sprintf((char*)special_keys,"%s", SPECIAL_KEYS_METER_STATUS); //Quick Meter Status
			if((strncmp(keys_local,special_keys,5)==0) && (Tech_menu==false))
			{
				Quick_meter_status_request = TRUE;
				Debug_TextOut( 0, "Quick Meter Status" );
				Display_Meter_status();
				memset(special_keys,0,sizeof(special_keys));
			}
			if((Tech_menu==FALSE)&&(Quick_meter_status_request==FALSE))
				{
					if(key_press ==  KEY_UP)//up arrow
					{
						if(Current_Space_Id >= (glSystem_cfg.MSM_max_spaces-1))
							Debug_TextOut( 0, "Right Bay" );//Debug_TextOut( 0, "Last Bay" );
						else
							Current_Space_Id++;
					}
					else if(key_press ==  KEY_DOWN)
					{
						if(Current_Space_Id < 1 )
							Debug_TextOut( 0, "Left Bay" );//Debug_TextOut( 0, "First Bay" );
						else
							Current_Space_Id--;
					}
					Idle_Screen();
				}
#if 0	//can't allow calibration mode through key sequence in field
			sprintf((char*)special_keys,"%s", SPECIAL_KEYS_METER_AUDIT); //Meter Audit
			if((strncmp(keys_local,special_keys,5)==0) && (Tech_menu==false))
			{
				/*Debug_TextOut( 0, "Meter Audit Done" );
				push_event_to_cache( UDP_EVTTYP_COLLECTION_EVT );	//Audit event raised; server will take care of collection report
				AUDIT_Screen();
				DelayMs(3000);
				Idle_Screen();
				memset(special_keys,0,sizeof(special_keys));*/
				Coin_Calibration_Mode(AUTO_CALIBRATION_MODE);
			}
//#endif

			/*sprintf((char*)special_keys,"%s", SPECIAL_KEYS_DIAG_MODE);

			if( (strncmp(keys_local,special_keys,5)==0) && (glIn_diag_mode==false) )
			{
				glIn_diag_mode=true;
				Debug_TextOut( 0, "LNG is in Diag mode" );
				//set_ip_adress();//01-08-12:no need to set here ,just display what was already set .
				display_info_on_LCD();
				memset(special_keys,0,sizeof(special_keys));    // TODO --- why?
			}
			else if((strncmp(keys_local,special_keys,5)==0)&&(glIn_diag_mode==true))
			{
				glIn_diag_mode=false;
				Debug_TextOut( 0, "LNG Exited Diag mode" );
				memset(special_keys,0,sizeof(special_keys));    // TODO --- why?
			}*/
#endif
		}
		else
			{
				if(key_press ==  KEY_UP)//up arrow
				{
					if(Current_Space_Id >= (glSystem_cfg.MSM_max_spaces-1))
						Debug_TextOut( 0, "Right Bay" );//Debug_TextOut( 0, "Last Bay" );
					else
						Current_Space_Id++;
				}
				else if(key_press ==  KEY_DOWN)
				{
					if(Current_Space_Id < 1 )
						Debug_TextOut( 0, "Left Bay" );//Debug_TextOut( 0, "First Bay" );
					else
						Current_Space_Id--;
				}
				Idle_Screen();
			}
	}
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for PORT3. This is used for Card Insert
//! switch and the Ring Indicator interrupt from Telit Modem. The ISR identifies
//! the pin and marks the task to be processed in Tasks_Priority_Register for
//! outer layer Statemachine
//! \param void
//! \return void
/**************************************************************************/
void PORT3_IRQHandler()
{
	volatile static uint32_t status1;
	Exit_from_LPM();
#ifdef 	ENABLE_SMS_MODE
	//if(GPIO_getInputPinValue(MDM_RING_PORT, MDM_RING_PIN) == 0)
	status1 = GPIO_getEnabledInterruptStatus(MDM_RING_PORT);
	if((status1 & MDM_RING_PIN) && (MDM_AWAKE == TRUE))
	{
		GPIO_clearInterruptFlag(MDM_RING_PORT, status1);
		if(COMMS_DOWN == TRUE)
			SMS_MODE = TRUE;

		Debug_Output1(0,"RI:%d\r\n",COMMS_DOWN);
	}
	else if(Be4Bootup == 1)
#endif
	{
		GPIO_clearInterruptFlag(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN);
		bufferFlag = 3;
		Event=0;
		SC_bytes=0;
		memset(RXData,0,sizeof(RXData));
		memset(glSCReadBuf,0,sizeof(glSCReadBuf));
		Last_CC_Payment_RTC = RTC_epoch_now();
		Tasks_Priority_Register |= MAGTEK_CARD_ISR;
		//Debug_Output1(0,"SC:%d\r\n",bufferFlag);
	}
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for PORT4.
//! \param void
//! \return void
/**************************************************************************/
void PORT4_IRQHandler()
{
	Exit_from_LPM();
	GPIO_clearInterruptFlag(LED_RXD_PORT,LED_RXD_PIN);
	disable_optical_LED_interrupt();

	//Tasks_Priority_Register |= PORT4_ISR;
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for PORT5.
//! \param void
//! \return void
/**************************************************************************/
void PORT5_IRQHandler()
{
	Exit_from_LPM();
	//Tasks_Priority_Register |= PORT5_ISR;
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for PORT6. This is the handler for SNSRDY
//! interrupt, do not comment this; it is used as a dummy only to reset the
//! interrupt vector table
//! \param void
//! \return void
/**************************************************************************/
void PORT6_IRQHandler()
{
	//This is the handler for SNSRDY interrupt, do not comment this; it is used as a dummy only to reset the interrupt vector table
	Exit_from_LPM();
	//Tasks_Priority_Register |= PORT6_ISR;
}

/**************************************************************************/
//! Interrupt Service Routine (ISR) for WatchDog Module. This is used to get
//! the handler for SNSRDY timer for MSP432. It includes LED flashing Code,
//! LORA receive Code and Coin detection Code
//! \param void
//! \return void
/**************************************************************************/

void WDT_A_IRQHandler()
{
	uint8_t indx = 0, i = 0;
	Exit_from_LPM();

	WDT_A_holdTimer();
	WDT_A_clearTimer();

	wdt_base_Counter++;
	watchdog_pat();
	//if((wdt_base_Counter % 3) == 0)
	if((wdt_base_Counter % 8) == 0)//55ms >>VT<<
	{
		Exit_from_LPM();
		if((SLEEP_ENABLE == FALSE) && (Waiting_4LORA_Resp == FALSE)) // every 500ms
		{
			//Debug_TextOut(0,"Rx2");
			for(indx = 0; indx < 10; indx++)
			{
				OnSlave();
				if(SLEEP_ENABLE == TRUE)
				{
					//end
					LORA_PWR_OFF_Time_DC = RTC_epoch_now();
					if(LORA_PWR_On_Time_DC == 0)
					{
						LORA_PWR_On_Time_DC = LORA_PWR_OFF_Time_DC;
					}
					Total_LORA_PWR_On_Time_DC +=((uint32_t)LORA_PWR_OFF_Time_DC - (uint32_t)LORA_PWR_On_Time_DC);
					//Debug_Output1(3,"Total_LORA_PWR_On_TimeDC =:%d",Total_LORA_PWR_On_Time_DC);
					LORA_PWR_On_Time = 0;

					GPIO_setOutputHighOnPin( LORA_SPI_IOPORT, LORA_NSS_PIN);
					break;
				}
			}
		}
		if(LED_ON == TRUE)
		{
			LED_ON_Count++;
			if(LED_ON_Count >= glSystem_cfg.LED_On_time_counter)
			{
				LED_ON_Count = 0;
				LED_ON = FALSE;
				//Front and Rear Selected in config
				if(glSystem_cfg.Enf_LEDs_selection == FRONT_AND_REAR_ENF_LED)//When both sides LED is selected, keep swapping each time
				{
					//Debug_TextOut(0, "FRONT_AND_REAR_ENF_LED");
					//Default Rear only because MB not supported
					if(HARDWARE_REVISION == LNG_REV3) //R3 MB
					{
						LED_selected_in_config = REAR_ENF_LED;
					}
					else if(HARDWARE_REVISION == LNG_REV4) //R4MB or R4.1MB //For R4MB will not support Front BiColor Display
					{
						if(REV4_LED_CONFIG == true)
						{
							GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
							GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
							GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
							GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
							GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
							//Debug_TextOut(0, "F R + R4MB");
							if(Front_Bi_Color == false) //R4 Comms + Front Uni color
							{
								//Debug_TextOut(0, "F R + R4MB + R4C");
                                if(LED_selected_in_config == REAR_ENF_LED)
                                {
                                    //Debug_TextOut(2, "FRONT_ENF_LED Selected");
                                    LED_selected_in_config = FRONT_ENF_LED;
                                    GPIO_setOutputHighOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
                                    GPIO_setOutputLowOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);
                                    if(LED_SPACE_FRONT == LEFT_FRONT)
                                    {
                                        LED_SPACE_FRONT = RIGHT_FRONT;
                                        //Debug_TextOut(0, "ISR_RIGHT_FRONT");
                                    }
                                    else if(LED_SPACE_FRONT == RIGHT_FRONT)
                                    {
                                        LED_SPACE_FRONT = LEFT_FRONT;
                                        //Debug_TextOut(0, "ISR_LEFT_FRONT");
                                    }
                                }
                                else
                                {
                                    LED_selected_in_config = REAR_ENF_LED;
                                    GPIO_setOutputHighOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
                                    GPIO_setOutputLowOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);
                                    if(LED_SPACE_REAR == LEFT_REAR)
                                    {
                                        LED_SPACE_REAR = RIGHT_REAR;
                                        //Debug_TextOut(0, "ISR_LEFT_REAR");
                                    }
                                    else if(LED_SPACE_REAR == RIGHT_REAR)
                                    {
                                        LED_SPACE_REAR = LEFT_REAR;
                                        //Debug_TextOut(0, "ISR_RIGHT_REAR");
                                    }
                                }
							}
							else if(Front_Bi_Color == true) //R4/4.1 MB + R4 Comms + Front BiColor
							{
								//Debug_TextOut(0, "F R + R4MB + R4C +FBI");
								if(LED_selected_in_config == REAR_ENF_LED)
								{
									//Debug_TextOut(2, "FRONT_ENF_LED Selected");
									LED_selected_in_config = FRONT_ENF_LED;
									GPIO_setOutputHighOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
									GPIO_setOutputLowOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);
									if(LED_SPACE_FRONT == LEFT_FRONT)
									{
										LED_SPACE_FRONT = RIGHT_FRONT;
										//Debug_TextOut(0, "ISR_RIGHT_FRONT");
									}
									else if(LED_SPACE_FRONT == RIGHT_FRONT)
									{
										LED_SPACE_FRONT = LEFT_FRONT;
										//Debug_TextOut(0, "ISR_LEFT_FRONT");
									}
								}
								else
								{
									//Debug_TextOut(2, "REAR_ENF_LED Selected");
									LED_selected_in_config = REAR_ENF_LED;
									GPIO_setOutputHighOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);
									GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
									if(LED_SPACE_REAR == LEFT_REAR)
									{
										LED_SPACE_REAR = RIGHT_REAR;
										//Debug_TextOut(0, "ISR_RIGHT_REAR");
									}
									else if(LED_SPACE_REAR == RIGHT_REAR)
									{
										LED_SPACE_REAR = LEFT_REAR;
										//Debug_TextOut(0, "ISR_LEFT_REAR");
									}
								}
							}
						}
						else if(REV4_LED_CONFIG != true)
						{
							LED_selected_in_config = REAR_ENF_LED;
							P8OUT &= ~0x08;	//P8.3=DISP_LED_HILO_PORT //LOW
							P8OUT |= 0x20;	//P8.5=COMMS_LED_HILO_PORT //HIGH
							P5OUT |= 0x42;	//P5.1=EXCESS_LED_PORT_R4 & P5.6=PENALTY_LED_PORT //HIGH
							P7OUT |= 0x20;	//P7.5=RGLED_PORT //HIGH
							if(LED_SPACE_REAR == LEFT_REAR)
							{
								LED_SPACE_REAR = RIGHT_REAR;
								//Debug_TextOut(0, "ISR_RIGHT_REAR");
							}
							else if(LED_SPACE_REAR == RIGHT_REAR)
							{
								LED_SPACE_REAR = LEFT_REAR;
								//Debug_TextOut(0, "ISR_LEFT_REAR");
							}
						}
					}

				}
				//below else is used because R4MB+R4C are having bicolour led and both blink simultaneously and power consumption is more, so switching each will reduce power consumption // LNGSIT-2056 //vinay
				//Rear or Front Leds selected in config
				else
				{
					if(HARDWARE_REVISION == LNG_REV4)//(REV4_LED_CONFIG == true) //R4 Comms Board
					{
						if(((glSystem_cfg.Enf_LEDs_selection == REAR_ENF_LED) || (glSystem_cfg.Enf_LEDs_selection == REAR_RED_ONLY_ENF_LED) || (glSystem_cfg.Enf_LEDs_selection == REAR_GREEN_ONLY_ENF_LED)))
						{
							LED_selected_in_config = glSystem_cfg.Enf_LEDs_selection;
							if(REV4_LED_CONFIG == true)
							{
								GPIO_setOutputHighOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
								GPIO_setOutputLowOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);
								GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
								GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
								GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
								GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
								GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
								//Debug_TextOut(0, "REAR+R4C");
							}
							else
							{
								P8OUT &= ~0x08;	//P8.3=DISP_LED_HILO_PORT //LOW
								P8OUT |= 0x20;	//P8.5=COMMS_LED_HILO_PORT //HIGH
								P5OUT |= 0x42;	//P5.1=EXCESS_LED_PORT_R4 & P5.6=PENALTY_LED_PORT //HIGH
								P7OUT |= 0x20;	//P7.5=RGLED_PORT //HIGH
							}
							if(LED_SPACE_REAR == LEFT_REAR)
							{
								LED_SPACE_REAR = RIGHT_REAR;
								//Debug_TextOut(0, "ISR_LEFT_REAR");
							}
							else if(LED_SPACE_REAR == RIGHT_REAR)
							{
								LED_SPACE_REAR = LEFT_REAR;
								//Debug_TextOut(0, "ISR_RIGHT_REAR");
							}
						}
						else if( ((glSystem_cfg.Enf_LEDs_selection == FRONT_ENF_LED) || (glSystem_cfg.Enf_LEDs_selection == FRONT_RED_ONLY_ENF_LED) || (glSystem_cfg.Enf_LEDs_selection == FRONT_GREEN_ONLY_ENF_LED)))
						{
							GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
							GPIO_setOutputHighOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);
							GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
							GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
							GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
							GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
							GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
							//Debug_TextOut(0, "FRONT+R4C");
							LED_selected_in_config = glSystem_cfg.Enf_LEDs_selection;
							if(LED_SPACE_FRONT == LEFT_FRONT)
							{
								LED_SPACE_FRONT = RIGHT_FRONT;
								//Debug_TextOut(0, "ISR_RIGHT_FRONT");
							}
							else if(LED_SPACE_FRONT == RIGHT_FRONT)
							{
								LED_SPACE_FRONT = LEFT_FRONT;
								//Debug_TextOut(0, "ISR_LEFT_FRONT");
							}
						}
					}

				}
				init_LEDs_PWM();	//This is to switch off LEDs; this change will take care of the countdown issue in R3
			}
		}
	}

	if(((wdt_base_Counter % LORA_CHECK_COUNTER) == 0) && (Waiting_4LORA_Resp == FALSE))
	{
		Exit_from_LPM();
		if(((glSystem_cfg.vehicle_sensor_enable == TRUE) || (glSystem_cfg.LORA_ENABLE_FOR_PBC == TRUE)) && (SLEEP_ENABLE == TRUE) && ((current_rate_index < 20) || (current_rate_index == PREPAY_PARKING)))// When Sensor "Enabled" and if in Enforced Hours only then allow Lora activity.
		{
			GPIO_setOutputLowOnPin( LORA_SPI_IOPORT, LORA_NSS_PIN);
			SLEEP_ENABLE = FALSE; // on
			LORA_PWR_On_Time_DC = RTC_epoch_now(); //start
			//Debug_TextOut(2, "Duty_Cycle_Mode = FALSE---------------*******************-------------------");
		}
		else if((current_rate_index > 20) && (current_rate_index != PREPAY_PARKING) && (LORA_PWR == TRUE))
		{
			SLEEP_ENABLE = TRUE;
			LORA_PWR = FALSE;
			LORA_power_off();
		}
	}

	if(wdt_base_Counter >= ONE_SECOND_TIMER)
	{
		Exit_from_LPM();
		Seconds_Counter++;
		wdt_base_Counter = 0;

		//Debug_Output1(0,"%d", Seconds_Counter);
		Tasks_Priority_Register |= INTERVALS_ISR;

		for(i=0; i<MSM_MAX_PARKING_BAYS; i++)
		{
			//Running_Space_ID[i] = FALSE;//when spaces expires within few secs apart then expired events for some were missing so placed FALSE here to avoid missing, its moved to Statemachine.c //vinay
			if(parking_time_left_on_meter[i] > 0)
			{
				real_payment_flag[i] = TRUE;
				parking_time_left_on_meter[i]--;
				if((parking_time_left_on_meter[i] % 60) == 0)
				{
					Parking_Clock_Refresh[i] = PARKING_CLOCK_RUN_MODE;
					Running_Space_ID[i] = TRUE;
					//Debug_Output2(0,"Running_Space_ID[%d] = %d", i , Running_Space_ID[i]);
				}

			}

			if(grace_time_trigger[i] == true)
			{
				if(negetive_parking_time_left_on_meter[i] <= (glSystem_cfg.grace_time*60))
					negetive_parking_time_left_on_meter[i]++;
				if((negetive_parking_time_left_on_meter[i] % 60) == 0)
				{
					Parking_Clock_Refresh[i] = PARKING_CLOCK_RUN_MODE;
					Running_Space_ID[i] = TRUE;
					//Debug_TextOut(0,"grace time refresh");
				}
				//Debug_Output1(0,"gt:%d", negetive_parking_time_left_on_meter[Current_Space_Id]);
			}
		}
		if((wakeup_skip==0)||(!Be4Bootup))
			Coin_WakeCount_adjustment(REGULAR_UPWARDS_ADJUSTMENT);	//Pass true for upwards adjustment

		watchdog_pat();
		LED_ON = TRUE;
		init_LEDs_PWM();
	}
	wakeup_skip = 0;

	if(((wdt_base_Counter % 4) == 0)||(((second_check)&&(wdt_base_Counter % 2) == 0)&&((wdt_base_Counter % 4) != 0)))
	{	//~30ms coin wake-up need to be called(WDT INTERRUPT TIMING ~7ms) >VT<
		Exit_from_LPM();
		if(((second_check)&&(wdt_base_Counter % 2) == 0)&&((wdt_base_Counter % 4) != 0))
			second_check = 0;
		coin_wakeup();
	}
	WDT_A_startTimer();
}


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

