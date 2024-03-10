//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_smartcard.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_smartcard_api
//! @{
//
//*****************************************************************************
#include "LibG2_smartcard.h"

extern GPRSSystem      								glSystem_cfg;
extern uint8_t 										Event;
extern uint8_t 										glSCReadBuf[MAX_SCT_RX_BYTES];
extern uint8_t 										bufferFlag;
extern uint8_t 										SC_bytes, scosta_smartcard_mode;
extern uint8_t                          			Tech_menu;
extern uint8_t 										smart_card_mode, gl_Commission_Card,check_card_read;
extern volatile uint8_t 							current_rate_index;
extern uint16_t  									/*gl_CC_amount_selected,*/ glCurrent_amount;//not used in this program //vinay
extern uint32_t			 							CC_TimeOut_RTCTimestamp;
extern volatile uint32_t							parking_time_left_on_meter[MSM_MAX_PARKING_BAYS], negetive_parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];
extern uint8_t 										Parking_Clock_Refresh[MSM_MAX_PARKING_BAYS];
extern uint32_t 									Tasks_Priority_Register, santa_cruz_previous_paid_time;
extern uint8_t 										file_manager_screen, copy_files_screen;/*, copy_files_screen_count;*///not used in this program //vinay
extern uint8_t 										RXData[MAX_SCT_RX_BYTES];
extern uint16_t 									Current_Space_Id;
extern Maintenance_Evt      						Maintenance_Log;
extern GPRS_PamBayStatusUpdateRequest  PAM_Bay_status_data[MAX_PAM_BSU_QUEUE];
extern uint8_t 										PAM_Baystatus_update_count;
extern uint32_t										last_payment_try_time[MSM_MAX_PARKING_BAYS];
extern uint32_t 									CardJam_TimeOut_RTC, glTotal_COIN_TransAmount;//not used in this program //vinay
extern uint8_t 										CardJam_Flag, grace_time_trigger[MSM_MAX_PARKING_BAYS];
extern uint8_t 										key_press, in_prepay_parking[MSM_MAX_PARKING_BAYS];
extern uint8_t 										Connected_Card_reader, meter_out_of_service;
extern uint8_t         								/*ccard_dataread_flag, card_removed_flag, */CC_Auth_Entered;//not used in this program //vinay
extern MBInfo      									glMB_info;
extern uint16_t              						glTotal_5_cents,
													glTotal_10_cents,
													glTotal_25_cents,
													glTotal_100_cents,
													coin_type_invalid,
													glTotal_0_cents,
													glTotal_Coin_Count,
													glTotal_Valid_Coin_Count;
extern uint16_t										coin_type0;
extern uint16_t										coin_type1;
extern uint16_t										coin_type2;
extern uint16_t										coin_type3;

extern uint32_t             		                total_coin_values,Last_Audit_Time;// in cents
extern uint16_t										temp_coin_type[20];
extern uint16_t										coin_type[20];

static uint8_t Cashkey_Account_Key[8] = {0};

uint8_t	 SC_City_Code = 0, Record_ID = 1;
uint8_t  show_balance_and_deduct = 0;
uint8_t	 SC_Verify_Update = FALSE;
GPRS_Smartcard_tranRequest SCT_Trans_Data;
uint32_t calc_refund_amount = 0, glTotal_SCT_TransAmount = 0;
uint32_t SC_Serial_Num = 0;
uint16_t glTotal_SCT_TransCount = 0;

uint8_t Payment_card = 0; //to avoid sucharges display for payment card //vinay

uint32_t Last_Audit_Time1; //to store last swipe timestamp //vinay

uint32_t SYNCardDeductValue;
uint8_t SYNCardID[SYNCARD_CARD_ID_LENGTH];

const uint8_t DuncanAuthenticateSet[] = {
   DUNCAN_MAP_MANUF,DUNCAN_MAP_REGION,DUNCAN_MAP_MODE,
   DUNCAN_MAP_CTC1,DUNCAN_MAP_BAL1A1,DUNCAN_MAP_BAL1A2,DUNCAN_MAP_A1MAXCRED};

const uint8_t DuncanAuthenticateStore[] = {
   DUNCAN_INDEX_MANUF,DUNCAN_INDEX_REGION,DUNCAN_INDEX_MODE,
   DUNCAN_INDEX_CTC1,DUNCAN_INDEX_BAL1A1,DUNCAN_INDEX_BAL1A2,DUNCAN_INDEX_A1MAXCRED};

#define DUNCAN_AUTHENTICATE_SET (sizeof(DuncanAuthenticateSet)/sizeof(DuncanAuthenticateSet[0]))
#define DUNCAN_AUTHENTICATE_OPS 9

//Encryption tables
static const char s_table[] = {
	11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3,
	10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8};

static const char p_table[] = {
	1,0x80,
	0,0x40,
	2,0x08,
	2,0x10,
	3,0x10,
	1,0x08,
	3,0x08,
	2,0x01,
	0,0x01,
	1,0x40,
	2,0x40,
	3,0x02,
	0,0x10,
	2,0x02,
	3,0x40,
	1,0x02,
	0,0x02,
	0,0x80,
	2,0x80,
	1,0x20,
	3,0x80,
	3,0x04,
	0,0x04,
	1,0x01,
	2,0x04,
	1,0x10,
	3,0x20,
	0,0x20,
	2,0x20,
	1,0x04,
	0,0x08,
	3,0x01};

static const uint8_t bitIndex[8] = {1,2,4,8,16,32,64,128};
uint32_t  Refund_PreviousSN = 0;
uint32_t  Refund_PresentSN = 0;
uint32_t  Refund_timestamp = 0;
uint32_t  Refund_earned_mins = 0;
uint32_t  RefundNewBalverify = 0;
uint8_t	  RefundOK = 0;
REFUND_UPDATE last_refund_update;

void Smart_Card_Reset()
{
	//UART_enableModule(EUSCI_A3_BASE);
	//GPIO_setAsPeripheralModuleFunctionInputPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN, GPIO_PRIMARY_MODULE_FUNCTION);	//SC IO Line
	//__enable_interrupt();
	init_smartcard_GPIO();
	DelayMs(50);

	GPIO_setOutputLowOnPin(PCLK_CN_PORT, PCLK_CN_PIN);
	pclk_On();
	DelayMs(50);

	GPIO_setOutputLowOnPin(GEMCLUB_RST_PORT, GEMCLUB_RST_PIN);
	DelayMs(1);
	GPIO_setOutputHighOnPin(GEMCLUB_RST_PORT, GEMCLUB_RST_PIN);
	acdb.CardFlags = 0;
}

void Identify_Smart_Card_Type()
{
	uint16_t elapsed_time5 = 0;
	uint8_t  read_SC = 0;
	uint8_t  i = 0;

	if(scosta_smartcard_mode == TRUE)
	{
		AUDIT_In_Progress_Screen(); //Audit in progress msg
		Debug_TextOut(0, "Scosta Audit Card");
		if(SCOSTA_WriteAsyncCardWord(SCOSTA_SELECT_FILE, DUNCAN_MAP_REGION, 0x0400, FALSE) == TRUE)
		{
			Debug_TextOut(0, "Scosta File Select Success");
			for(read_SC = 0; read_SC < SCOSTA_MAX_RECORDS; read_SC++)
			{
				if(SCOSTA_ReadAsyncCardWord(SCOSTA_READ_RECORD, (read_SC + 1), (uint32_t *)0x0400) == TRUE)
				{
					//Debug_TextOut(0, "Scosta Read Record Success");
					//Debug_Output6(0,"Read Record:%02X, %02X, %02X, %02X, %02X, %02X", glSCReadBuf[0], glSCReadBuf[1], glSCReadBuf[2], glSCReadBuf[3], glSCReadBuf[4], glSCReadBuf[5]);

					if((glSCReadBuf[2] == 0) && (glSCReadBuf[3] == 0))
					{
						Debug_Output1(0,"Current Record_ID = %02X", read_SC);
						break;
					}
				}
				else
				{
					Debug_Output1(0,"No Tn = %02X", read_SC);
					break;
				}
				DelayMs(25); //Delay needed between reads
			}

			if(Record_ID >= SCOSTA_MAX_RECORDS) //Record ID is 1 Byte, so if max 250 then reset to zero
			{
				AUDIT_Card_Full_Screen();
				DelayMs(2000);
				end_read_card(); //Pls Remove Card msg
				return;
			}
			else
			{
				Record_ID = read_SC + 1;
				Debug_Output1(0,"Next Record_ID = %02X", Record_ID);
			}

			if(SCOSTA_WriteAsyncCardWord(SCOSTA_APPEND_RECORD, Record_ID, 0x0400, FALSE) == TRUE)
			{
				Debug_TextOut(0, "Scosta Append Record Success");
				AUDIT_Screen(); //Audit Done msg
				DelayMs(2000);
				end_read_card(); //Pls Remove Card msg
				Idle_Screen();
				Last_Audit_Time = RTC_epoch_now();

				//Reset parameters
				Flash_Batch_Flag_Read();
				//clear the values only for coin only meters when audit card was success //vinay
				if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only	== TRUE)
				{
					glTotal_Valid_Coin_Count = 0;//coin_type0 + coin_type1 + coin_type2 + coin_type3; //Valid Coins Count
					glTotal_0_cents = 0;//coin_type_invalid; //Invalid Coins Count
					glTotal_100_cents = 0;//coin_type3;//glTotal_100_cents; //Total 100C Coins
					glTotal_25_cents = 0;//coin_type2;//glTotal_25_cents; //Total 25C Coins
					glTotal_10_cents = 0;//coin_type1;//glTotal_10_cents; //Total 10C Coins
					glTotal_5_cents = 0;//coin_type0;// glTotal_5_cents; //Total 5C Coins
					glTotal_Coin_Count = 0;//total_coin_values;//glTotal_COIN_TransAmount; //Need to calculate
					glTotal_COIN_TransAmount = 0;
					for(i=0;i<20;i++)
					{
						coin_type[i] = 0;
						temp_coin_type[i] = 0;
						//Debug_Output2(0, "SC temp_coin_type[%d]: %d",i, temp_coin_type[i]);
						//Debug_Output2(0, "SC coin_type[%d]: %d",i, coin_type[i]);
					}
					//Debug_TextOut(0, "Appended the values to card and clearing");
					//Debug_Output2(0, "glTotal_Valid_Coin_Count: %d, glTotal_Invalid_Coins: %d", glTotal_Valid_Coin_Count, glTotal_0_cents);
					//Debug_Output6(0, "glTotal_100_cents: %d, glTotal_25_cents: %d, glTotal_10_cents:%d, glTotal_5_cents:%d", glTotal_100_cents, glTotal_25_cents, glTotal_10_cents, glTotal_5_cents,0,0);
					//Debug_Output1(0, "glTotal_Coin_Count: %ld", glTotal_Coin_Count);

					coin_type3 = 0;
					coin_type2 = 0;
					coin_type1 = 0;
					coin_type0 = 0;
					coin_type_invalid = 0;
					total_coin_values = 0;
					glTotal_SCT_TransCount = 0;
					glTotal_SCT_TransAmount = 0;
				}

				Flash_Batch_Flag_Write();
			}
			else
			{
				//Debug_Output6(2,"Append Record Failed:%02X, %02X, %02X, %02X, %02X, %02X", glSCReadBuf[0], glSCReadBuf[1], glSCReadBuf[2], glSCReadBuf[3], glSCReadBuf[4], glSCReadBuf[5]);
				Debug_TextOut(0, "Scosta Append Record Failed");
				AUDIT_Append_Failed_Screen(); //Audit Failed msg
				DelayMs(2000);
				end_read_card(); //Pls Remove Card msg
				Idle_Screen();
			}
		}
		else
		{
			Debug_TextOut(0, "Scosta File Select Failed-01");
			end_read_card(); //Pls Remove Card msg
			Idle_Screen();
		}

#if	0 //ONLY for development purpose
		uint16_t i = 0;
		if(SCOSTA_WriteAsyncCardWord(SCOSTA_SELECT_FILE, Record_ID, 0x0400, FALSE) == TRUE)
		{
			Debug_TextOut(0, "Scosta File Select Success");
			//DelayMs(100);
			if(SCOSTA_ReadAsyncCardWord(SCOSTA_READ_RECORD, Record_ID, (uint32_t *)0x0400) == TRUE)
			{
				Debug_TextOut(0, "Scosta Read Record Success");
				for(i = 0; i < 42; i++)
				{
					Debug_Output2(0,"Read Record[%d]:%02X", i, glSCReadBuf[i]);
				}
			}
			else
			{
				Debug_TextOut(0, "Scosta Read Record Failed");
			}
		}
		else
		{
			Debug_TextOut(0, "Scosta File Select Failed-02");
		}
		//DelayMs(1000);
		end_read_card();
#endif
	}
	else //GemClub SmartCard
	{
		if(ReadAsyncCardWord(DUNCAN_MAP_REGION, &acdb.CardMap[DUNCAN_INDEX_REGION]) == TRUE)
		{
			SC_City_Code = glSCReadBuf[1];
			Debug_Output1( 0, "SC City Code:%d", SC_City_Code);
		}
		else
		{
			push_event_to_cache(UDP_EVTTYP_SC_CARD_READ_FAIL);
			smart_card_mode = FALSE;	//LNGSIT-819
			SC_Card_Invalid_Screen();
			return;
		}

		if((Event == 1) && (glMB_info.city_code == SC_City_Code))
		{
			switch(glSCReadBuf[2])
			{
			case 0:
				if(((glSystem_cfg.allowed_trans_types >> 8) & 0x01) != 1)
				{
					Debug_TextOut(0, "Payment card Disabled");
					SC_Payment_Disabled_Screen();
					DelayMs(2000);
				}
				else
				{
					if(Tech_menu == TRUE)
					{
						diag_text_Screen("LNG is in Tech Mode", TRUE, FALSE);
					}
					else
					{
						if(meter_out_of_service == FALSE) //LNGSIT-591
						{
							Payment_card = 1;  // to avoid surcharges for payment card. //vinay
							Debug_TextOut( 0, "Payment card");
							push_event_to_cache(UDP_EVTTYP_SC_CARD_READ_SUCCESS);
							/////////////////////////////////////////////////////////// vinay
							if((Current_Space_Id != LEFT_BAY) && (Current_Space_Id != RIGHT_BAY))
							{
								Debug_Output1(0,"Current_Space_Id = %d",Current_Space_Id);

								smart_card.detected = FALSE;
								show_balance_and_deduct = 0;
								SC_Verify_Update = FALSE;
								smart_card_mode = FALSE;
								Turn_Off_card_IOs(Connected_Card_reader);

								DSM_Screen7();
								DelayMs(500);
								return;
							}
							/////////////////////////////////////////////////////// vinay
							Process_smart_card_payment();
						}
					}
				}
				break;
			case 5:
				smart_card_mode = FALSE;
				Tech_menu = FALSE;
				Debug_TextOut( 0, "Timewipe Card");
				break;
			case 6:
				Debug_TextOut( 0, "Technician card");
				if (ReadAsyncCardWord(DUNCAN_MAP_NUMBER, &acdb.CardMap[DUNCAN_INDEX_NUMBER]) == TRUE)
				{
					Maintenance_Log.TECH_CARD_SL_NO = acdb.CardMap[DUNCAN_INDEX_NUMBER] & 0x0fffffff;
				}
				Tech_menu=true;
				smart_card_mode = FALSE;
				Debug_TextOut( 0, "LNG is in tech mode" );
				push_event_to_cache( UDP_EVTTYP_TECH_MENU_ACCESS );//09-04-12
				TechMenu_Display_Main_Menu();
				break;
			case 7:
				Debug_TextOut( 0, "Collection Card: Meter Audit Done");
				if (ReadAsyncCardWord(DUNCAN_MAP_NUMBER, &acdb.CardMap[DUNCAN_INDEX_NUMBER]) == TRUE)
				{
					Maintenance_Log.TECH_CARD_SL_NO = acdb.CardMap[DUNCAN_INDEX_NUMBER] & 0x0fffffff;
				}
				Tech_menu = FALSE;
				push_event_to_cache( UDP_EVTTYP_COLLECTION_EVT );	//Audit event raised; server will take care of collection report
				Tasks_Priority_Register |= SEVERE_EVENTS_TASK;
				AUDIT_Screen();
				DelayMs(3000);
				end_read_card(); //LNGSIT-972
				break;
			case 8:
				Debug_TextOut( 0, "Commissioning Card");
				//Debug_TextOut( 0, "NO support with this Release" );
				if (ReadAsyncCardWord(DUNCAN_MAP_NUMBER, &acdb.CardMap[DUNCAN_INDEX_NUMBER]) == TRUE)
				{
					Maintenance_Log.TECH_CARD_SL_NO = acdb.CardMap[DUNCAN_INDEX_NUMBER] & 0x0fffffff;
				}
				Debug_TextOut( 0, "LNG is in tech mode" );
				Tech_menu=true;
				gl_Commission_Card=true;
				smart_card_mode = FALSE;
				push_event_to_cache( UDP_EVTTYP_TECH_MENU_ACCESS );
				file_manager_screen = 0;
				copy_files_screen = true;
				//tech_mechanism_and_datakey_files();
				tech_mechanism_files(); //LNGSIT-1239
				//Idle_Screen();
				break;
			}
		}
		else
		{
			smart_card_mode = FALSE;	//LNGSIT-819
			SC_Card_Invalid_Screen();
			push_event_to_cache(UDP_EVTTYP_SC_CARD_READ_FAIL); //LNGSIT-738
			Debug_TextOut( 0, "Invalid Smart Card" );
		}
		/*	GPIO_setOutputLowOnPin(PCLK_CN_PORT, PCLK_CN_PIN);	//Power Optimization
	pclk_Off();*/
		if(smart_card_mode == TRUE)	//LNGSIT-559,532: for payment cards, no need to wait and declare timeout here
		{
			CardJam_TimeOut_RTC = RTC_epoch_now();
			CardJam_Flag = 1;
		}
		else
		{
			while ((0 == GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN)) && (++elapsed_time5 < 6000) ) //WAIT FOR CARD REMOVAL-5SECS
			{
				DelayMs(1);
			}

			if ( elapsed_time5 >= 6000 )//LNGSIT-1324, LNGSIT-762, LNGSIT-821
			{
				Debug_TextOut( 0, "Card Remove timeout-04" );
				CardJam_TimeOut_RTC = RTC_epoch_now();
				CardJam_Flag = 1;
				push_event_to_cache( UDP_EVTTYP_CARD_REMOVE_TIMEOUT );
				Idle_Screen();
			}
		}
	}
}

uint8_t smartcard_write(uint8_t*command,uint8_t len)
{
	__disable_interrupt();
	uint8_t Eveodd=0;
	uint8_t i,j;
	bufferFlag = 6;
	SC_bytes=0;
	Event = 0; //Initialize for every read/write
	memset(glSCReadBuf,0,sizeof(glSCReadBuf));

	GPIO_setAsOutputPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN);

	for(j=0; j<len; j++)
	{
		GPIO_setOutputLowOnPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN);  //start bit
		DelayUs(SC_RW_TIMING);
		for(i=0;i<8;i++)
		{
			if(((command[j]>>(i))&1) == 1)
			{
				GPIO_setOutputHighOnPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN);
				Eveodd+=1;
			}
			else
			{
				GPIO_setOutputLowOnPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN);
			}
			DelayUs(SC_RW_TIMING);
		}
		if(Eveodd%2!=0)
			GPIO_setOutputHighOnPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN);
		else
			GPIO_setOutputLowOnPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN); //Parity bit
		DelayUs(SC_RW_TIMING);
		Eveodd=0;
		GPIO_setOutputHighOnPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN);  //ideal state
		DelayUs(2*SC_RW_TIMING);	//gaurd time
	}
	GPIO_setAsPeripheralModuleFunctionInputPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
	__enable_interrupt();
	for(i=0;i<100;i++)
	{
		if(Event == 1)
			break;
		DelayMs(1);
	}
	return Event;
}

void Process_smart_card_payment(void)
{
	last_payment_try_time[Current_Space_Id] = RTC_epoch_now();

	if((current_rate_index == FREE_PARKING)||(current_rate_index == NO_PARKING))//Do not accept card for these modes
	{
		smart_card_mode = FALSE;
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
		return;
	}

	if((smart_card_mode == TRUE) && (smart_card.detected == TRUE))
	{
		if (show_balance_and_deduct == 0)
		{
			balance_gemclub_memo_card();
			if(smart_card.smart_card_state == GEMCLUBM_CARD_BALANCE_SUCCESS)
			{
				show_balance_and_deduct = 1;
				if(smart_card.balance >= glSystem_cfg.SCT_default_min_amount)
				{
					//Start CC TimeOut Time
					CC_TimeOut_RTCTimestamp = RTC_epoch_now();
					check_card_read = TRUE;
					glCurrent_amount = glSystem_cfg.Card_default_amount;
					ShowAmount_Screen(glCurrent_amount, 0,TRUE);
				}
				else
				{
					smart_card_mode = FALSE;
					SC_Verify_Update = FALSE;
					push_event_to_cache(UDP_EVTTYP_SMART_CARD_LOW_BALANCE);//LNGSIT-710
					SC_Card_Low_Fund_Screen();
					DelayMs(3000);
					Idle_Screen();
				}
			}
			else
			{
				SC_Verify_Update = FALSE;
				smart_card_mode = FALSE;
				SC_Card_Invalid_Screen();
				DelayMs(3000);
				Idle_Screen();
			}
		}
	}
}

/**************************************************************************/
/*  Name        :end_read_card                                            */
/*  Parameters  :void                                                     */
/*  Returns     :void                                                     */
/*  Scope       : System                                                  */
/*  Function    :                                                         */
/*------------------------------------------------------------------------*/
void end_read_card(void)
{
	uint16_t elapsed_time4 = 0;

	if ((smart_card_mode == TRUE) && (smart_card.detected == TRUE)) //scosta_smartcard_mode, add if needed
	{
		Debug_TextOut( 0,"End: Please Remove Card");
		if(GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN) == 0)
		{
			Remove_Card_Screen();
		}

		Turn_Off_card_IOs(Connected_Card_reader);

		smart_card.detected = FALSE;
		show_balance_and_deduct = 0;
		smart_card_mode = FALSE;
		SC_Verify_Update = FALSE;
		scosta_smartcard_mode = FALSE;
		CC_Auth_Entered = FALSE;

		while ((0 == GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN)) && (++elapsed_time4 < 3000) ) //WAIT FOR CARD REMOVAL-5SECS
		{
			DelayMs(1);
		}

		if ( elapsed_time4 >= 3000 )//if time out
		{
			Debug_TextOut( 0, "Card Remove timeout-03" );
			CardJam_TimeOut_RTC = RTC_epoch_now();
			CardJam_Flag = 1;
			push_event_to_cache( UDP_EVTTYP_CARD_REMOVE_TIMEOUT );
			Idle_Screen();
		}
		Idle_Screen();
	}
	else
		Turn_Off_card_IOs(Connected_Card_reader);
}

/**************************************************************************
  Func Name   : balance_gemclub_memo_card
  Parameters  : void
  Returns     : void
  Scope       : module
  Function    : retrieve the amount of value stored on the card and put
  			  : it in the smart_card.balance variable. Set the other
			  : state variables accordingly. This is a blocking call
			  : unfortunately. There may be lengthy operations
**************************************************************************/
void balance_gemclub_memo_card(void)
{
	if ((smart_card_mode == TRUE) && (smart_card.detected == TRUE))
	{
		Debug_TextOut( 0,"Gemclub Retrieve Balance");

		if ((acdb.CardFlags & (CARD_RESET | CARD_POWER)) != (CARD_RESET | CARD_POWER))
		{
			DelayMs(60);
			MAG_1_POWER_ON();
			Smart_Card_Reset();
			if ((RXData[0] != 0x3B))
			{
				//Debug_TextOut( 0,"Balance: Failed Card Reset\r\n");
			}
			else
			{
				acdb.CardFlags |= CARD_RESET | CARD_POWER;
			}
		}

		if((smart_card_mode == FALSE) && (smart_card.detected == FALSE))
		{	// failed to reset card
			smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
			smart_card.smart_card_state = GEMCLUBM_CARD_SC_INVALID;
			return;
		}

		if ((acdb.CardFlags & CARD_AUTHENTIC) != CARD_AUTHENTIC)
		{
			switch (AuthenticateDuncanAsyncCard())
			{
				case SMARTCARD_INVALID:
					smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
					smart_card.smart_card_state = GEMCLUBM_CARD_SC_INVALID;
					push_event_to_cache(UDP_EVTTYP_SC_CARD_READ_FAIL); //LNGSIT-738
					//Debug_TextOut( 0,"Balance: SMARTCARD_INVALID");
					return;
				case SMARTCARD_IN_BLACKLIST:
					smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
					smart_card.smart_card_state = GEMCLUBM_CARD_BLACKLIST;
					//Debug_TextOut( 0,"Balance: SMARTCARD_IN_BLACKLIST");
					return;
				case SMARTCARD_IO_ERROR:
					Debug_TextOut( 0,"io Error 4");
					push_event_to_cache(UDP_EVTTYP_CARD_NOT_SUPPORTED);	//LNGSIT-709
					smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
					smart_card.smart_card_state = GEMCLUBM_CARD_DEDUCT_FAILED;
					//Debug_TextOut( 0,"Balance: SMARTCARD_IO_ERROR");
					return;
				case SMARTCARD_SUCCESS:
					//Debug_TextOut( 0,"Auth in gem balance done: SMARTCARD_SUCCESS");
					break;
			}
		}

		switch (RetrieveBalanceDuncanAsyncCard())
		{
			case SMARTCARD_IO_ERROR:
				Debug_TextOut( 0,"io Error 5");
				smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
				smart_card.smart_card_state = GEMCLUBM_CARD_DEDUCT_FAILED;
				return;
			case SMARTCARD_ZERO_BALANCE:
			case SMARTCARD_SUCCESS:
				Debug_TextOut( 0,"Balance in gem balance done");
				smart_card.balance = acdb.balance;
				smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
				smart_card.smart_card_state = GEMCLUBM_CARD_BALANCE_SUCCESS;
				break;
		}
	}
	else
	{
		Debug_TextOut( 0,"Gemclub Retrieve Balance - No Card");
		acdb.CardFlags = 0;
		smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
		smart_card.smart_card_state = GEMCLUBM_CARD_NO_FOUND;
	}
}


/***************************************************************************
  Func Name   : deduct_gemclub_memo_card
  Parameters  : unsigned uint32_t deduct_amount (in cents)
  Returns     : void
  Scope       : System
  Function    : To deduct cash from parking card
***************************************************************************/
void deduct_gemclub_memo_card(uint32_t deduct_amount)
{
    SYNCard_SaveDeductRefundValue(deduct_amount);		/* remember the last amount deducted to credit to the bay */
    uint32_t temp_earned_time_in_seconds = 0, pre_existing_seconds = 0;
    uint32_t temp_parking_clock_for_display = 0; //, tmp_currentRTC = 0;

    if ((smart_card_mode == TRUE) && (smart_card.detected == TRUE))
	{
		Debug_TextOut( 0,"Gemclub Deduct");

		if ((acdb.CardFlags & (CARD_RESET | CARD_POWER)) != (CARD_RESET | CARD_POWER))
		{
			DelayMs(60);
			MAG_1_POWER_ON();
			Smart_Card_Reset();
			if ((RXData[0] != 0x3B))
			{
				//Debug_TextOut( 0,"Deduct: Failed Card Reset\r\n");
			}
			else
			{
				acdb.CardFlags |= CARD_RESET | CARD_POWER;
			}
		}

		if((smart_card_mode == FALSE) && (smart_card.detected == FALSE))
		{	// failed to reset card
			smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
			smart_card.smart_card_state = GEMCLUBM_CARD_SC_INVALID;
			return;
		}

		if ((acdb.CardFlags & CARD_AUTHENTIC) != CARD_AUTHENTIC)
		{
			switch (AuthenticateDuncanAsyncCard())
			{
				case SMARTCARD_INVALID:
					smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
					smart_card.smart_card_state = GEMCLUBM_CARD_SC_INVALID;
					//Debug_TextOut( 0,"Deduct: SMARTCARD_INVALID");
					return;
				case SMARTCARD_IN_BLACKLIST:
					smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
					smart_card.smart_card_state = GEMCLUBM_CARD_BLACKLIST;
					//Debug_TextOut( 0,"Deduct: SMARTCARD_IN_BLACKLIST");
					return;
				case SMARTCARD_IO_ERROR:
					Debug_TextOut( 0,"io Error 1\r\n");
					smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
					smart_card.smart_card_state = GEMCLUBM_CARD_DEDUCT_FAILED;
					//Debug_TextOut( 0,"Deduct: SMARTCARD_IO_ERROR");
					return;
				case SMARTCARD_SUCCESS:
					Debug_TextOut( 0,"Deduct: SMARTCARD_SUCCESS");
					break;
			}
		}

		if ((acdb.CardFlags & CARD_BALANCE) != CARD_BALANCE)
		{
			//Debug_TextOut( 0," B1 ");
			switch (RetrieveBalanceDuncanAsyncCard())
			{
				case SMARTCARD_IO_ERROR:
					Debug_TextOut( 0,"io Error 2");
					smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
					smart_card.smart_card_state = GEMCLUBM_CARD_DEDUCT_FAILED;
					return;
				case SMARTCARD_ZERO_BALANCE:
					smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
					smart_card.smart_card_state = GEMCLUBM_CARD_DEDUCT_NO_BALANCE;
					return;
				case SMARTCARD_SUCCESS:
					smart_card.balance = acdb.balance;
					break;
			}
		}

		acdb.deduction = deduct_amount;
		//Debug_TextOut( 0," D1 ");
	    switch (DeductAmountDuncanAsyncCard())
	    {
			case SMARTCARD_IO_ERROR:
				smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
				Debug_TextOut( 0,"io Error 3\r\n");
				smart_card.smart_card_state = GEMCLUBM_CARD_DEDUCT_FAILED;
				return;
			case SMARTCARD_ZERO_BALANCE:
				smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
				smart_card.smart_card_state = GEMCLUBM_CARD_DEDUCT_NO_BALANCE;
				smart_card.balance = acdb.balance;
				push_event_to_cache(UDP_EVTTYP_SMART_CARD_LOW_BALANCE);//LNGSIT-710
				SC_Card_Low_Fund_Screen();
				DelayMs(3000);//TODO: Use the config parameter for the duration of approved/declined screen
				Idle_Screen();
				break;
			case SMARTCARD_SUCCESS:
				smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
				smart_card.smart_card_state = GEMCLUBM_CARD_DEDUCT_SUCCESS;
				smart_card.balance = acdb.balance;
				smart_card.deduct_amount = deduct_amount;

				if((grace_time_trigger[Current_Space_Id] == true) && (glSystem_cfg.grace_time_type != 0)) // For Grace time feature
				{
					parking_time_left_on_meter[Current_Space_Id] = 0;
					negetive_parking_time_left_on_meter[Current_Space_Id] = 0;
					grace_time_trigger[Current_Space_Id] = false;
				}

				pre_existing_seconds = (parking_time_left_on_meter[Current_Space_Id]);
				//if((parking_time_left_on_meter[Current_Space_Id]%60)>0)
					//pre_existing_minutes++;
				temp_parking_clock_for_display = get_earned_seconds(acdb.deduction, true, true);
				temp_earned_time_in_seconds = temp_parking_clock_for_display - pre_existing_seconds;

				parking_time_left_on_meter[Current_Space_Id] = temp_parking_clock_for_display;
				in_prepay_parking[Current_Space_Id] = FALSE;

				if(glSystem_cfg.santa_cruz_spl_feature_enable == TRUE)
					santa_cruz_previous_paid_time += temp_earned_time_in_seconds;

				SCT_Trans_Data.paid_untill_mins = (parking_time_left_on_meter[Current_Space_Id]/60);
				if((((parking_time_left_on_meter[Current_Space_Id]) % 3600) % 60) > 0)	//This is to fix the wrong update of parking clock
				{
					SCT_Trans_Data.paid_untill_mins += 1;
				}

				SCT_Trans_Data.earned_mins = (temp_earned_time_in_seconds/60);
				if((((temp_earned_time_in_seconds) % 3600) % 60) > 0)	//This is to fix the wrong update of parking clock
				{
					SCT_Trans_Data.earned_mins += 1;
				}

				SCT_Trans_Data.time_stamp = RTC_epoch_now();
				SCT_Trans_Data.amount = (uint16_t)acdb.deduction;
				SCT_Trans_Data.ref_id = (uint16_t)((SCT_Trans_Data.time_stamp & 0xFF) | ((SCT_Trans_Data.time_stamp>>8) & 0xFFFF));

				//SYNCardReadCardID((uint8_t *)SCT_Trans_Data.card_id);
				memset(SCT_Trans_Data.card_id, 0, sizeof(SCT_Trans_Data.card_id));
				memcpy(SCT_Trans_Data.card_id, &SC_Serial_Num, 4);

				Refund_timestamp = RTC_epoch_now();
				Refund_earned_mins = temp_earned_time_in_seconds/60;
				Refund_PreviousSN =	Refund_PresentSN;
				//Debug_Output2(0, "%d, %d", Refund_PresentSN, Refund_PreviousSN);

				//LNGSIT-553: Moved this below PAMBS so that we don't loose the 3-4 seconds
/*				SC_Payment_Approved_Screen(false);
				DelayMs(3000);

				Tasks_Priority_Register |= SCT_UPDATE_TASK;
				Idle_Screen();*/

				if(parking_time_left_on_meter[Current_Space_Id] > 0)
					Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_RESET_MODE;	//Avoid spending time in parking clock display here, it will be done outside

/*				//PAM bay status update for SCT
				if(PAM_Baystatus_update_count <= (MAX_PAM_BSU_QUEUE-1))
				{
					tmp_currentRTC = RTC_epoch_now();
					PAM_Bay_status_data[PAM_Baystatus_update_count].txn_time = tmp_currentRTC; //SCT_Trans_Data.time_stamp;
					PAM_Bay_status_data[PAM_Baystatus_update_count].txn_type = Tx_Smart_Card;
					PAM_Bay_status_data[PAM_Baystatus_update_count].expiry_time = tmp_currentRTC + parking_time_left_on_meter[Current_Space_Id];
					PAM_Bay_status_data[PAM_Baystatus_update_count].amount_cents = SCT_Trans_Data.amount;
					PAM_Bay_status_data[PAM_Baystatus_update_count].purchased_duration = SCT_Trans_Data.earned_mins;
					if(pre_existing_seconds > 0)
					{
						PAM_Bay_status_data[PAM_Baystatus_update_count].topup = 1;
					}
					else
					{
						PAM_Bay_status_data[PAM_Baystatus_update_count].topup = 0;
					}
					PAM_Baystatus_update_count++;
				}*/

				//LNGSIT-553: Moved this below PAMBS so that we don't loose the 3-4 seconds
				SC_Payment_Approved_Screen(false);
				DelayMs(2000);
				DSM_Screen3(); // this is not there in MSM 
				DelayMs(2000);

				Flash_Batch_Flag_Read();
				glTotal_SCT_TransCount++;
				glTotal_SCT_TransAmount += acdb.deduction;

				//Debug_Output2(2, "TotalTrans:%ld, TotalAmt:%ld", glTotal_SCT_TransCount, glTotal_SCT_TransAmount);
				Flash_Batch_Flag_Write();

				Tasks_Priority_Register |= SCT_UPDATE_TASK;
				Idle_Screen();

				//Debug_Output1(0, "Earned:%ld",SCT_Trans_Data.earned_mins);
				//Debug_Output2(0, "Trans:%ld, Expt:%ld", SCT_Trans_Data.time_stamp, (SCT_Trans_Data.time_stamp + (SCT_Trans_Data.earned_mins*60) + pre_existing_seconds));

				break;
			default:
				//Debug_TextOut( 0," D2 ");
				break;
		}
	}
	else
	{
		acdb.CardFlags = 0;
		smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
		smart_card.smart_card_state = GEMCLUBM_CARD_NO_FOUND;
	}
}

/**************************************************************************
 * Func Name   : AuthenticateDuncanAsyncCard
 * Parameters  : none
 * Returns     : result code, SMARTCARD_INVALID | SMARTCARD_IN_BLACKLIST |
 *			      : SMARTCARD_IO_ERROR | SMARTCARD_SUCCESS
 * Scope       : module
 * Function    : The access mode of the Duncan Detroit smart cards is set so
 that the user area must be unlocked for both address read and update functions.
 The authenticate function must firstly unlock user area 1 before further authentication
 checks can be carried out. The secret words must be
 generated using L_Auth(card serial number, key1) and then verified for CSC1.

 The required authenticate set of words is then read from the card and the
 following checks are made:

			1 CSC1 Ratification counter < 0xE0000000
            2. Manufacturer [0,0..0,31] = 0xFFFFFFAA
            3. Region [2,0..2,31] = CFG_smartcard_regioncode
            4. Mode [4,31..4,31] = 1
            5. L_Auth(BAL1A1, Key) should equal BAL1A2
            6. Transaction limit count (x10) > CTC1 (x8)

 If all checks are successful then CardFlags is marked as authenticated and unlocked.

 **************************************************************************/
uint8_t AuthenticateDuncanAsyncCard(void)
{
	uint16_t i;
	uint8_t fContinue;
	uint8_t result = SMARTCARD_INVALID; // assume the worst
	uint8_t error = 0;
	DuncanMap *pMap;
	uint8_t Key[4];
	HotlistNumber hn;

	if (ReadAsyncCardWord(DUNCAN_MAP_NUMBER, &acdb.CardMap[DUNCAN_INDEX_NUMBER]) == FALSE)	   // get serial number to unlock card
    	if (ReadAsyncCardWord(DUNCAN_MAP_NUMBER, &acdb.CardMap[DUNCAN_INDEX_NUMBER]) == FALSE)
			return (SMARTCARD_IO_ERROR);
    /* RXVSW-103 */
	hn.SCNumber = acdb.CardMap[DUNCAN_INDEX_NUMBER] & 0x0fffffff;
	SC_Serial_Num = hn.SCNumber;
	GEMCardSetCardID((uint8_t *)&hn.SCNumber);	/* bug save card number */
    GEMCardSetCardID((uint8_t *)&acdb.CardMap[DUNCAN_INDEX_NUMBER]);

    Debug_Output1(0,"Card Serial Number %lu",hn.SCNumber);
	if (ReadAsyncCardWord(DUNCAN_MAP_CSC1, &acdb.CardMap[DUNCAN_INDEX_CSC1]) == FALSE)
      if (ReadAsyncCardWord(DUNCAN_MAP_CSC1, &acdb.CardMap[DUNCAN_INDEX_CSC1]) == FALSE)
         return (SMARTCARD_IO_ERROR);
	if ((acdb.CardMap[DUNCAN_INDEX_CSC1] & RATIFICATION_COUNTER_MASK) == RATIFICATION_COUNTER_MASK)
	{
		Debug_TextOut( 0,"Failed CSC1 Ratification Check");
		return (SMARTCARD_INVALID);		// deal only with cards that haven't lockedout CSC1
	}

	memset(Cashkey_Account_Key, 0, 8);
	memcpy(&Cashkey_Account_Key,&glSystem_cfg.Special_Auth_Key,8);

/*	Cashkey_Account_Key[0] = 0XBF;
	Cashkey_Account_Key[1] = 0xA7;
	Cashkey_Account_Key[2] = 0x36;
	Cashkey_Account_Key[3] = 0x90;

	Debug_Output6(0,"Special_Auth_Key: %02X %02X %02X %02X %02X %02X", glSystem_cfg.Special_Auth_Key[0], glSystem_cfg.Special_Auth_Key[1],
				glSystem_cfg.Special_Auth_Key[2], glSystem_cfg.Special_Auth_Key[3], glSystem_cfg.Special_Auth_Key[4], glSystem_cfg.Special_Auth_Key[5]);

	Debug_Output2(0,"Special_Auth_Key: %02X %02X", glSystem_cfg.Special_Auth_Key[6], glSystem_cfg.Special_Auth_Key[7]);

	card_encode((uint32_t *)Cashkey_Account_Key, 8, card_xxtea_key);

	Debug_Output6(0,"Cashkey_Account_Key-0X: %02X %02X %02X %02X %02X %02X", Cashkey_Account_Key[0], Cashkey_Account_Key[1],
			Cashkey_Account_Key[2], Cashkey_Account_Key[3], Cashkey_Account_Key[4], Cashkey_Account_Key[5]);

	Debug_Output2(0,"Cashkey_Account_Key-0X: %02X %02X", Cashkey_Account_Key[6], Cashkey_Account_Key[7]);

	Debug_Output6(0,"Cashkey_Account_Key-0d: %02d %02d %02d %02d %02d %02d", Cashkey_Account_Key[0], Cashkey_Account_Key[1],
			Cashkey_Account_Key[2], Cashkey_Account_Key[3], Cashkey_Account_Key[4], Cashkey_Account_Key[5]);

	Debug_Output2(0,"Cashkey_Account_Key-0d: %02d %02d", Cashkey_Account_Key[6], Cashkey_Account_Key[7]);*/

	card_decode((uint32_t *)Cashkey_Account_Key, 8, card_xxtea_key);

/*	Debug_Output6(0,"Cashkey_Account_Key-02: %02X %02X %02X %02X %02X %02X", Cashkey_Account_Key[0], Cashkey_Account_Key[1],
				Cashkey_Account_Key[2], Cashkey_Account_Key[3], Cashkey_Account_Key[4], Cashkey_Account_Key[5]);

	Debug_Output2(0,"Cashkey_Account_Key-02: %02X %02X", Cashkey_Account_Key[6], Cashkey_Account_Key[7]);

	Cashkey_Account_Key[0] = 0X7E;
	Cashkey_Account_Key[1] = 0X13;
	Cashkey_Account_Key[2] = 0X6D;
	Cashkey_Account_Key[3] = 0X54;*/

	memcpy(&Key,&Cashkey_Account_Key,4);
	acdb.CardMap[DUNCAN_INDEX_TMP1] = smartcard_encrypt(&acdb.CardMap[DUNCAN_INDEX_NUMBER], Key);
	//Debug_Output1(0,"CSC1 value is 0x%08lX\n",acdb.CardMap[DUNCAN_INDEX_TMP1]);
	if (VerifyAsyncCardWord(DUNCAN_MAP_CSC1, acdb.CardMap[DUNCAN_INDEX_TMP1]) == FALSE)
	{
		push_event_to_cache(UDP_EVTTYP_CARD_NOT_SUPPORTED);	//LNGSIT-709: Card not supported for authentication failure also
		Debug_TextOut( 0,"Failed CSC1 Check");
		memset(Key, 0, sizeof(Key));	// wipe key
		return (SMARTCARD_INVALID);
	}
    /* RXVSW-103 */
	hn.SCNumber = acdb.CardMap[DUNCAN_INDEX_NUMBER] & 0x0fffffff;
	Debug_TextOut( 0,"Verify Done");

   // read some addresses for authentication in the card map and store them into CardMap array
	for (i=0,error=0; i<DUNCAN_AUTHENTICATE_SET; i++)
	{
		if (ReadAsyncCardWord(DuncanAuthenticateSet[i], &acdb.CardMap[DuncanAuthenticateStore[i]]) == FALSE)
		{
			if (ReadAsyncCardWord(DuncanAuthenticateSet[i], &acdb.CardMap[DuncanAuthenticateStore[i]]) == FALSE)
			{
	            error |= 1;
    	        //debug_auth("error found on set %d at address %x\r\n",i,DuncanAuthenticateSet[i]);
        	    break;  // exit loop on error
	         }
		}
	}
	if (error == 0)
	{
      // carry out series of authentication checks
		pMap = (DuncanMap *) acdb.CardMap;
		for (i=0,fContinue=TRUE; ((i<DUNCAN_AUTHENTICATE_OPS) && (fContinue == TRUE)); i++)
		{
			switch (i)
			{
				case 0:	// 1. Manufacturer [0,0..0,31] = 0xFFFFFFAA
					fContinue = 1; //CheckManufacturerList(pMap->Manufacturer); //SSM Ignore Manufacturer
					break;
				case 1:	// 2. Region [2,0..2,31] = CFG_smartcard_regioncode
					fContinue = (pMap->RegionCode != SC_City_Code) ? 0 : 1;
					break;
				case 2:	// 3. Mode [4,31..4,31] = 1
					fContinue = (pMap->Mode != 1) ? 1 : 0;
					break;
				case 3:	// 4. CTC1 [0x8] < 2^16
					fContinue = (acdb.CardMap[DUNCAN_INDEX_CTC1] < 65536) ? 1 : 0;
					break;
				case 4:	// 5. L_Auth(BAL1A1, Key) should equal BAL1A2
					//fContinue = (acdb.CardMap[DUNCAN_INDEX_BAL1A2] != L_Auth(&acdb.CardMap[DUNCAN_INDEX_BAL1A1],Key)) ? 0 : 1;
					fContinue = (acdb.CardMap[DUNCAN_INDEX_BAL1A2] != smartcard_encrypt(&acdb.CardMap[DUNCAN_INDEX_BAL1A1],Key)) ? 0 : 1;
					break;
				case 5: //CFPBS-324
					// 6. CTC1 < transaction limit counter (located at x10)
					fContinue = 1; //Force it to Success
					//fContinue = acdb.CardMap[DUNCAN_INDEX_A1MAXCRED] > acdb.CardMap[DUNCAN_INDEX_CTC1];
					break;
				default:
					break;
			}
		}  // end of for loop
		memset(Key, 0, sizeof(Key));	// wipe key
		if (fContinue == TRUE)
		{ // successful set of tests
			acdb.CardFlags |= (CARD_AUTHENTIC | CARD_UNLOCKED);
			result = SMARTCARD_SUCCESS;
		}
		else
		{
			//debug_auth("Failed Authenticate at case %u\r\n",i-1);
		}
		return (result);
	}
	else
	{
		memset(Key, 0, sizeof(Key));	// wipe key
		return (SMARTCARD_IO_ERROR);
	}
}

/**************************************************************************
 * Func Name   : RetrieveBalanceDuncanAsyncCard
 * Parameters  : none
 * Returns     : result code SMARTCARD_ZERO_BALANCE | SMARTCARD_IO_ERROR |
                 SMARTCARD_SUCCESS
 * Scope       : module
 * Function    : Reads the balance word from area 1 on the card, stores the
 value. CardFlags is then set to retrieve the new balance for display update.
 **************************************************************************/
uint8_t RetrieveBalanceDuncanAsyncCard(void)
{
   if (ReadAsyncCardWord(DUNCAN_MAP_BAL1A1, &acdb.CardMap[DUNCAN_INDEX_BAL1A1]) == FALSE)
      if (ReadAsyncCardWord(DUNCAN_MAP_BAL1A1, &acdb.CardMap[DUNCAN_INDEX_BAL1A1]) == FALSE)
         return (SMARTCARD_IO_ERROR);

   acdb.balance = (uint32_t) acdb.CardMap[DUNCAN_INDEX_BAL1A1] & 0x0000FFFF;
   //Debug_Output1(0,"Retrieved Balance %lu\r\n",acdb.balance);
   acdb.CardFlags |= CARD_BALANCE;

   if (acdb.balance == 0)
      return (SMARTCARD_ZERO_BALANCE);
   else
      return (SMARTCARD_SUCCESS);
}

/**************************************************************************
 * Func Name   : DeductAmountDuncanAsyncCard
 * Parameters  : none
 * Returns     : result code SMARTCARD_ZERO_BALANCE | SMARTCARD_IO_ERROR |
                 SMARTCARD_SUCCESS
 * Scope       : module
 * Function    : Deducts the amount in the variable acdb.deduction or the
 remaining acdb.balance if insufficient balance for full deduction. The
 balance value is then updated in account area 1 (address C) with the backup
 value (address E) updated with an encrypted balance using the function
 L_Auth(newbalance,key). CardFlags is then set to retrieve the new balance
 for display update.
 **************************************************************************/
uint8_t DeductAmountDuncanAsyncCard(void)
{
	uint8_t write;
	uint32_t deduction;
	uint32_t newbalance;
	uint8_t Key[4];

	if (acdb.balance != 0)
	{
		deduction = (uint32_t) acdb.deduction;
		if (acdb.deduction > acdb.balance) // ensure it doesnt go negative
			deduction = acdb.balance;
		newbalance = acdb.balance - deduction;	// get new balance after deduction
		if (newbalance != acdb.balance)
		{     // first get the new verify value for the balance
			acdb.CardMap[DUNCAN_INDEX_TMP2] = acdb.CardMap[DUNCAN_INDEX_BAL1A2] & 0xFFFF0000;
			acdb.CardMap[DUNCAN_INDEX_TMP2] |= (uint32_t) newbalance;		// overlay

			memset(Cashkey_Account_Key, 0, 8);
			memcpy(&Cashkey_Account_Key,&glSystem_cfg.Special_Auth_Key,8);
			card_decode((uint32_t *)Cashkey_Account_Key, 8, card_xxtea_key);

			memcpy(&Key,&Cashkey_Account_Key,4);
			acdb.CardMap[DUNCAN_INDEX_TMP3] = smartcard_encrypt((uint32_t*)&acdb.CardMap[DUNCAN_INDEX_TMP2], Key);
			memset(Key, 0, sizeof(Key));	// wipe key
			 // update balance 1 and balance 1 verify value
			write = UpdateAsyncCardWord(DUNCAN_MAP_BAL1A1, acdb.CardMap[DUNCAN_INDEX_TMP2],FALSE);	// no retry
			if (write != 0)
			{ // successful
				DelayMs(5);
				write = UpdateAsyncCardWord(DUNCAN_MAP_BAL1A2, acdb.CardMap[DUNCAN_INDEX_TMP3],FALSE);
				if (write == 0)
				{	// failed
					Debug_TextOut( 0,"Update Failed 01");
					return (SMARTCARD_IO_ERROR);
				}
			}
			else
			{
				Debug_TextOut( 0,"Update Failed 02");
				return (SMARTCARD_IO_ERROR);
			}
			acdb.balance = newbalance;
		}
		acdb.CardFlags |= CARD_BALANCE;
		//Debug_TextOut( 0," DS_1 \r\n");
		return (SMARTCARD_SUCCESS);
	}
	else
	{
		Debug_TextOut( 0,"DeductAmountDuncanAsyncCard ZB_2");
		return (SMARTCARD_ZERO_BALANCE);
	}
}

/**************************************************************************
 * Func Name   : UpdateAsyncCardWord
 * Parameters  : Address to read [0..3F], uint32_t word to send
 * Returns     : uint8_t 0=fail, 1=success
 * Scope       : module
 * Function    : Send a 32-bit word to write to a location on the card. It must
 *			   : have been unlocked with the verify command for this to be
 *			   : successful. Insert a 10ms delay to allow non-volatile technologies
 *			   : to recover from the write cycle
 **************************************************************************/
uint8_t UpdateAsyncCardWord(uint8_t address, uint32_t Word, uint8_t fRetry)
{
	uint8_t result = FALSE;	// assume bad news
	char retry = 2;
	uint8_t iobuffer[8];
	char i;

	SC_Verify_Update = TRUE;
	memset(iobuffer, 0, sizeof(iobuffer));

	do
	{
        iobuffer[0] = SMARTCARD_CLA_TYPE1;
        iobuffer[1] = SMARTCARD_INS_UPDATE;
	    iobuffer[2] = 0;
    	iobuffer[3] = address;
    	iobuffer[4] = SMARTCARD_CMD_LENGTH;

    	if(smartcard_write(iobuffer, SC_BUFFER_LENGTH) == 1)
		{
			//Debug_TextOut( 0, "Update Success 1" );
    		/*Debug_Output6(0, "SC CMD: %X %X %X %X %X", iobuffer[0],
    				iobuffer[1], iobuffer[2], iobuffer[3], iobuffer[4], 0);*/
			result = TRUE;
		}

		if ((result == TRUE) && (Event == 1))
		{
            iobuffer[0] = (uint8_t) Word & 0xff;
            iobuffer[1] = (uint8_t) (Word >> 8);
            iobuffer[2] = (uint8_t) (Word >> 16);
            iobuffer[3] = (uint8_t) (Word >> 24);

            if(smartcard_write(iobuffer, SMARTCARD_CMD_LENGTH) == 1)
            {
            	//Debug_TextOut( 0, "Update Success 2" );
            	result = TRUE;
            }
		}
		retry--;
	} while ((retry > 0) && (fRetry != 0) && (result == FALSE));

	if (result != 0)
	{	// only if write successful
		for (i=0; i<10; i++)
		{
			DelayUs(1000);
		}
	}
	SC_Verify_Update = FALSE;
	return (result);
}

/**************************************************************************
 * Func Name   : VerifyAsyncCardWord
 * Parameters  : uint32_t word to send
 * Returns     : uint8_t 0=fail, 1=success
 * Scope       : module
 * Function    : Send a 32-bit word to CSC1 to unlock the smart card for
 *			   : balance update
 **************************************************************************/
uint8_t VerifyAsyncCardWord(uint8_t address, uint32_t Word)
{
	uint8_t result = FALSE;	// assume bad news
	char retry = 2;
	uint8_t iobuffer[8];

	SC_Verify_Update = TRUE;
	memset(iobuffer, 0, sizeof(iobuffer));

	do {
		iobuffer[0] = SMARTCARD_CLA_TYPE1;
        iobuffer[1] = SMARTCARD_INS_VERIFY;
	    iobuffer[2] = 0;
    	iobuffer[3] = address;
    	iobuffer[4] = SMARTCARD_CMD_LENGTH;

    	if(smartcard_write(iobuffer, SC_BUFFER_LENGTH) == 1)
		{
			//Debug_TextOut( 0, "Verify Success 1" );
    		/*Debug_Output6(0, "SC CMD: %X %X %X %X %X", iobuffer[0],
    				iobuffer[1], iobuffer[2], iobuffer[3], iobuffer[4], 0);*/
			result = TRUE;
		}

    	if ((result == TRUE) && (Event == 1))
		{
			iobuffer[0] = (uint8_t) Word & 0xff;
			iobuffer[1] = (uint8_t) (Word >> 8);
			iobuffer[2] = (uint8_t) (Word >> 16);
			iobuffer[3] = (uint8_t) (Word >> 24);

			if(smartcard_write(iobuffer, 4) == 1)
			{
				//Debug_TextOut( 0, "Verify Success 2" );
				result = TRUE;
			}
		}
		retry--;
	} while ((retry > 0) && (result == FALSE));
	SC_Verify_Update = FALSE;
	return (result);
}

/**************************************************************************
 * Func Name   : ReadAsyncCardWord
 * Parameters  : Address to read [0..3F], uint32_t pointer for storage
 * Returns     : uint8_t 0=fail, 1=success
 * Scope       : module
 * Function    : Read a word (32-bits) from the smart card. The address
 *				 range is 0..3F. The bytes come of the card in little-endian
 *				 format so they are shuffled to make a DWORD
 **************************************************************************/
uint8_t ReadAsyncCardWord(uint8_t address, uint32_t *pWord)
{
	uint8_t result = FALSE;	// assume bad news
	uint8_t iobuffer[8];

	SC_Verify_Update = FALSE;
	memset(iobuffer, 0, sizeof(iobuffer));

    iobuffer[0] = SMARTCARD_CLA_TYPE1;
    iobuffer[1] = SMARTCARD_INS_READ;
    iobuffer[2] = 0;
	iobuffer[3] = address;
	iobuffer[4] = SMARTCARD_CMD_LENGTH;

	if(smartcard_write(iobuffer, SC_BUFFER_LENGTH) == 1)
	{
		memcpy(pWord, &glSCReadBuf[1],SMARTCARD_CMD_LENGTH);
		//Debug_TextOut( 0, "Read Success" );
		/*Debug_Output6(0, "SC CMD: %X %X %X %X %X", iobuffer[0],
				iobuffer[1], iobuffer[2], iobuffer[3], iobuffer[4], 0);*/
		result = TRUE;
	}
	else
	{
		result = FALSE;
	}
	return (result);
}

/**************************************************************************
  Func Name   :  SYNCardReadCardID
  Parameters  :
  Returns     :
  Scope       :
  Function    : read syncard id
**************************************************************************/
void SYNCardReadCardID(uint8_t *id_ptr)
{
    memcpy(id_ptr, SYNCardID,SYNCARD_CARD_ID_LENGTH);
}

/**************************************************************************
  Func Name   : GEMCardSetCardID
  Parameters  :
  Returns     :
  Scope       :
  Function    : set syn card id
**************************************************************************/
void GEMCardSetCardID(uint8_t *id_ptr)
{
    memset(SYNCardID, 0,SYNCARD_CARD_ID_LENGTH);
    memcpy(SYNCardID, id_ptr,GEM_ID_LEN);
}

/**************************************************************************
  Func Name   : SYNCard_SaveDeductValue
  Parameters  : uint32_t
  Returns     : void
  Scope       : module
  Function    : save deduct amount
**************************************************************************/
void SYNCard_SaveDeductRefundValue(uint32_t amount)
{
    SYNCardDeductValue = amount;
}

/**************************************************************************
  Func Name   : SYNCardReadDeductRedundValue
  Parameters  : void
  Returns     : uint16_t
  Scope       : module
  Function    : read deduct amount
**************************************************************************/
uint32_t SYNCardReadDeductRedundValue(void)
{
    return(SYNCardDeductValue);
}

/**************************************************************************
 * Name   : smartcard_encrypt
 * Parameters  : uint 32 bits data to decrypt/encrypt and uint 32 bits key
 * Returns     : uint 32 bits encrypted data
 **************************************************************************/
uint32_t smartcard_encrypt(uint32_t *iDataVal, uint8_t *pKey)
{
	uint32_t loEncryptionVal = 0;
	uint8_t pData[4];
	uint8_t loResult[4] = {0};
	uint8_t loPerm = 0; //permutation
	uint8_t loPad16_l = 0;
	uint8_t loPad16_h = 0;
	uint8_t loPad = 0;
	uint8_t i,j,k,loSel;
	long loTemp = 0;

	for (i = 0; i<4 ; i++)
	{
		pData[i] = (*iDataVal >> (i*8)) & 0xFF;
	}

   	for (i = 0; i<4 ; i++)  //i = length
   	{
		// Key byte to pad
	   	loPad = pKey[i];
	   	// Initialise all results with 0
	   	for (k = 0; k<4 ; k++)
	   	{
			loResult[k] = 0;
	   	}

	   for (j = 0;j<4;j++)  //j = source
       {
		  	loPerm = j<<4;
		  	loPad16_l = pData[j] & 0x0f;
		  	loPad16_h = (pData[j]>>4) & 0x0f;

		  	if((loPad & 0x01) == 0x01)
		  	{
				loSel = loPad16_l + 16;
		  	}else{
		  		loSel = loPad16_l ;
			}
		  	loPad16_l = s_table[loSel];

		  	for(k=0; k<4; k++)
			{
		  		if((loPad16_l & bitIndex[k]) == bitIndex[k])
		  		{
					//ldx p_table+0,y
     				//lda p_table+1,y
     				//ora cal_sens+4,x
     				//sta cal_sens+4,x
					loSel = p_table[(k*2) + loPerm];  //x
					loResult[loSel] |= p_table[(k*2) + loPerm + 1];
		  		}
			}
			//nost3
			if((loPad & 0x02) == 0x02)
			{
				loPad16_l = s_table[loPad16_h + 16];
			}else{
				loPad16_l = s_table[loPad16_h];
			}
			for(k=0; k<4 ; k++)
			{
				if((loPad16_l & bitIndex[k]) == bitIndex[k])
				{
					//ldx p_table+8,y
					//lda p_table+9,y
					//ora cal_sens+4,x
					//sta cal_sens+4,x
					loSel = p_table[(k*2) + loPerm + 8];  //x
					loResult[loSel] |= p_table[(k*2) + loPerm + 9];
				}
			}
			//nost7
			loPad = loPad>>2;
		} //j loop
		//copy output array to input array
		for(k = 0; k<4; k++)
		{
			pData[k] = loResult[k];
			//Debug_Output1(0,"pData[k] is 0x%X\n", pData[k]);
		}
	}//i loop
	loEncryptionVal = 0;
	for(k = 4; k>0; k--)
	{
		loTemp = pData[k-1] & 0xFF;
		loEncryptionVal |= loTemp<<((k-1) * 8);
	}
	//Debug_Output1(0,"loEncryptionVal is 0x%08lX\r\n",loEncryptionVal);

	return loEncryptionVal;
}

void Refund_CardSN()
{
	uint32_t pWord;
	if (ReadAsyncCardWord(DUNCAN_MAP_NUMBER,&pWord) == TRUE)  //This will read the serial no.
	{
/*		if (Refund_PreviousSN == 0)
			Refund_PreviousSN = pWord;
		else if(Refund_PresentSN > 0)*/
			Refund_PresentSN = pWord ;
			//Debug_Output1(0, "Present SN: %d", Refund_PresentSN);
	}
}
/*
 *  we will return 0 for a same  different serial no and one for same serial no.
 *
 */

uint8_t Refund_CampareSN()
{
	//Refund_CardSN();
	uint8_t i=0;
	if (Refund_PreviousSN != Refund_PresentSN)
		i = 0;
	else if((Refund_PreviousSN == Refund_PresentSN) && (Refund_PreviousSN != 0))
		i = 1;
	return i;
}

void Refund_SNcheck_timeleft()
{
	uint32_t  temp_timestamp = 0;
	//int32_t temp_parking_clock = 0;
	//uint32_t amount = 0;
	//uint8_t i=0;
	Refund_CardSN();	//Read the card number in any case
	//Removed event logging here: LNGSIT-763
	temp_timestamp =  RTC_epoch_now();
	temp_timestamp = (temp_timestamp - Refund_timestamp);  //This check is for diffrence in current time and alloted time.

	if ((parking_time_left_on_meter[Current_Space_Id] > 0 ) && (Refund_earned_mins > 0))
	{
		if ( (Refund_CampareSN()) && (temp_timestamp < (Refund_earned_mins * 60)) )
		{
			calc_refund_amount = Refund_get_earned_amount((Refund_earned_mins * 60) - (temp_timestamp));
			//Debug_Output1(2,"refund amount=%d",calc_refund_amount*100);

			if ((AuthenticateDuncanAsyncCard() == TRUE ) && (RetrieveBalanceDuncanAsyncCard() == TRUE) && (calc_refund_amount >= MIN_REFUND_AMT_MULTIPLIER))
			{
				push_event_to_cache(UDP_EVTTYP_SC_CARD_READ_SUCCESS); //LNGSIT-721
				CC_TimeOut_RTCTimestamp = RTC_epoch_now();
				//Debug_TextOut(2, "Refund Ready");
				RefundShowAmount_Screen(0,calc_refund_amount,TRUE);
				RefundOK = 3;
				key_press = 6;
			}
			else
				RefundOK = FALSE;
		}
		else
		{
			RefundOK = FALSE;	//LNGSIT-366
			//Debug_Output6(2,"%d,%d,%d,%d,%d,%d",temp_timestamp, Refund_earned_mins, Refund_PreviousSN, Refund_PresentSN, RefundOK, key_press);
		}

		if(Refund_PresentSN > 0)
		{
			//Debug_TextOut(2, "Refund Ready2");
			Refund_PreviousSN = Refund_PresentSN ;
			//Refund_PresentSN = 0;
		}
		key_press = 6;
	}
	else
	{
		//Debug_Output2(2, "time left:%d,%d", parking_time_left_on_meter, Refund_earned_mins);
		RefundOK = FALSE;
		if(Refund_PresentSN > 0)
		{
			//Debug_TextOut(2,"No Refund");
			Refund_PreviousSN = Refund_PresentSN ;
			//Refund_PresentSN = 0;
		}
	}
}

uint8_t RefundAmountDuncanAsyncCard(uint32_t Addamount)
{
	uint8_t write;
	uint32_t newbalance;
	uint8_t Key[4];

	if (Addamount != 0)
	{
		newbalance = acdb.balance + Addamount;
		RefundNewBalverify = newbalance; // This is used to verify.
		if (newbalance != acdb.balance)
		{     // first get the new verify value for the balance
			acdb.CardMap[DUNCAN_INDEX_TMP2] = acdb.CardMap[DUNCAN_INDEX_BAL1A2] & 0xFFFF0000;
			acdb.CardMap[DUNCAN_INDEX_TMP2] |= (uint32_t) newbalance;		// overlay

			memset(Cashkey_Account_Key, 0, 8);
			memcpy(&Cashkey_Account_Key,&glSystem_cfg.Special_Auth_Key,8);
			card_decode((uint32_t *)Cashkey_Account_Key, 8, card_xxtea_key);

			memcpy(&Key,&Cashkey_Account_Key,4);
			acdb.CardMap[DUNCAN_INDEX_TMP3] = smartcard_encrypt((uint32_t*)&acdb.CardMap[DUNCAN_INDEX_TMP2], Key);
			memset(Key, 0, sizeof(Key));	// wipe key
			 // update balance 1 and balance 1 verify value
			write = UpdateAsyncCardWord(DUNCAN_MAP_BAL1A1, acdb.CardMap[DUNCAN_INDEX_TMP2],FALSE);	// no retry
			if (write != 0)
			{ // successful
				DelayMs(5);
				write = UpdateAsyncCardWord(DUNCAN_MAP_BAL1A2, acdb.CardMap[DUNCAN_INDEX_TMP3],FALSE);
				if (write == 0)
				{	// failed
					Debug_TextOut( 0,"Refund Update Failed 01");
					return (SMARTCARD_IO_ERROR);
				}
			}
			else
			{
				Debug_TextOut( 0,"Refund Update Failed 02");
				return (SMARTCARD_IO_ERROR);
			}
			acdb.balance = newbalance;
		}
		//acdb.CardFlags |= CARD_BALANCE;
		//Debug_TextOut( 0," DS_1 \r\n");
		return (SMARTCARD_SUCCESS);
	}
	else
	{
		Debug_TextOut( 0,"Refund Update Amount is less ");
		return (SMARTCARD_ZERO_BALANCE);
	}
}


void refund_gemclub_memo_card (uint32_t calc_refund_amount)
{
	int32_t temp_parking_clock = 0;
	uint32_t  temp_timestamp = 0;

	temp_timestamp =  RTC_epoch_now();
	temp_timestamp = (temp_timestamp - Refund_timestamp);

	if ((smart_card_mode == TRUE) && (smart_card.detected == TRUE))
	{
		Debug_TextOut( 0,"Gemclub Refund");
		if ((acdb.CardFlags & (CARD_RESET | CARD_POWER)) != (CARD_RESET | CARD_POWER))
		{
			DelayMs(60);
			MAG_1_POWER_ON();
			Smart_Card_Reset();
			if ((RXData[0] != 0x3B))
			{
				//Debug_TextOut( 0,"Deduct: Failed Card Reset\r\n");
			}
			else
			{
				acdb.CardFlags |= CARD_RESET | CARD_POWER;
			}
		}
	}

	if ((acdb.CardFlags & CARD_AUTHENTIC) != CARD_AUTHENTIC)
	{
		switch (AuthenticateDuncanAsyncCard())
		{
		case SMARTCARD_INVALID:
			smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
			smart_card.smart_card_state = GEMCLUBM_CARD_SC_INVALID;
			//Debug_TextOut( 0,"Refund: SMARTCARD_INVALID");
			return;
		case SMARTCARD_IN_BLACKLIST:
			smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
			smart_card.smart_card_state = GEMCLUBM_CARD_BLACKLIST;
			//Debug_TextOut( 0,"Refund: SMARTCARD_IN_BLACKLIST");
			return;
		case SMARTCARD_IO_ERROR:
			Debug_TextOut( 0,"io Error 1\r\n");
			smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
			smart_card.smart_card_state = GEMCLUBM_CARD_DEDUCT_FAILED;
			//Debug_TextOut( 0,"Refund: SMARTCARD_IO_ERROR");
			return;
		case SMARTCARD_SUCCESS:
			//Debug_TextOut( 0,"Refund: SMARTCARD_SUCCESS");
			break;
		}
	}

	if ((acdb.CardFlags & CARD_BALANCE) != CARD_BALANCE)
	{
		//Debug_TextOut( 0," B1 ");
		switch (RetrieveBalanceDuncanAsyncCard())
		{
		case SMARTCARD_IO_ERROR:
			Debug_TextOut( 0,"io Error 2");
			smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
			smart_card.smart_card_state = GEMCLUBM_CARD_DEDUCT_FAILED;
			return;
		case SMARTCARD_ZERO_BALANCE:
			smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
			smart_card.smart_card_state = GEMCLUBM_CARD_DEDUCT_NO_BALANCE;
			return;
		case SMARTCARD_SUCCESS:
			smart_card.balance = acdb.balance;
			break;
		}
	}

	if(RefundOK == TRUE)
	{
		RefundAmountDuncanAsyncCard((uint32_t)calc_refund_amount);
		smart_card.deduct_amount = calc_refund_amount;	//Important for the approved screen message
		//Debug_Output1(0,"RefundOK=%d",RefundOK);

		if (RetrieveBalanceDuncanAsyncCard() == TRUE)
		{
			//Debug_Output1(0,"Came here=%d",RefundNewBalverify);
			if( RefundNewBalverify ==  acdb.balance)
			{
				memset(&last_refund_update, 0, sizeof(REFUND_UPDATE));
				memcpy(last_refund_update.refund_cardid, &Refund_PresentSN, sizeof(Refund_PresentSN));
				last_refund_update.refund_amount = calc_refund_amount;
				push_event_to_cache(UDP_EVTTYP_SMARTCARD_REFUND);
				Tasks_Priority_Register |= SEVERE_EVENTS_TASK;
				//Debug_Output1(0,"RefundNewBalverify=%d",RefundNewBalverify);
				//Debug_Output1(0,"parking_time_left_on_meter_1=%d",parking_time_left_on_meter);

				if (parking_time_left_on_meter[Current_Space_Id] > 0 )
				{
					temp_parking_clock = (parking_time_left_on_meter[Current_Space_Id]/60)  - ((Refund_earned_mins *60 ) - (temp_timestamp))/60;

					if ((temp_parking_clock > Refund_earned_mins) && ((parking_time_left_on_meter[Current_Space_Id]/60) >= Refund_earned_mins))
					{
						temp_parking_clock = ((parking_time_left_on_meter[Current_Space_Id]/60)  - Refund_earned_mins);
						//Debug_Output1(0,"temp_parking_clock_1=%d",temp_parking_clock);
					}
					/*else
						temp_parking_clock = 0;*/

					if(temp_parking_clock == 1)	//LNGSIT-526: Clear any calculation offsets and reset parking clock to 0
						temp_parking_clock = 0;

					parking_time_left_on_meter[Current_Space_Id] = (uint32_t)((uint32_t)temp_parking_clock * (uint32_t)60);   //This to update the parking clock.
					Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_RESET_MODE;	//Avoid spending time in parking clock display here, it will be done outside
					//Debug_Output1(0,"temp_parking_clock_2=%d",temp_parking_clock);
					//Debug_Output1(0,"parking_time_left_on_meter_2=%d",parking_time_left_on_meter);
					Refund_PreviousSN = 0;
					Refund_PresentSN = 0;
					Refund_timestamp = 0;
					RefundOK = 0;
					Refund_earned_mins = 0; //Refunded, do not allow further
				}
				smart_card.end_smart_card = TRUE;	// signal to caller, end of operation
				smart_card.smart_card_state = GEMCLUBM_CARD_DEDUCT_SUCCESS;
				SC_Payment_Approved_Screen(true);
				DelayMs(2000);
				Idle_Screen();
			}
		}
		RefundOK =  FALSE;
	}
}

//SCOSTA AUDIT CARD IMPLEMENTATION FUNCTIONS
uint8_t SCOSTA_smartcard_write(uint8_t*command, uint8_t len)
{
	__disable_interrupt();
	uint8_t Eveodd=0;
	uint8_t i,j;
	bufferFlag = 42;
	SC_bytes=0;
	Event = 0; //Initialize for every read/write
	memset(glSCReadBuf,0,sizeof(glSCReadBuf));

	GPIO_setAsOutputPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN);

	for(j=0; j<len; j++)
	{
		GPIO_setOutputLowOnPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN);  //start bit
		DelayUs(SC_RW_TIMING);
		for(i=0;i<8;i++)
		{
			if(((command[j]>>(i))&1) == 1)
			{
				GPIO_setOutputHighOnPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN);
				Eveodd+=1;
			}
			else
			{
				GPIO_setOutputLowOnPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN);
			}
			DelayUs(SC_RW_TIMING);
		}
		if(Eveodd%2!=0)
			GPIO_setOutputHighOnPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN);
		else
			GPIO_setOutputLowOnPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN); //Parity bit
		DelayUs(SC_RW_TIMING);
		Eveodd=0;
		GPIO_setOutputHighOnPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN);  //ideal state
		DelayUs(2*SC_RW_TIMING);	//gaurd time
	}
	GPIO_setAsPeripheralModuleFunctionInputPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
	__enable_interrupt();
	for(i=0;i<100;i++)
	{
		if(Event == 1)
			break;
		DelayMs(1);
	}
	return Event;
}

/**************************************************************************
 * Func Name   : SCOSTA_WriteAsyncCardWord
 * Parameters  : Address to read [0..3F], uint32_t word to send
 * Returns     : uint8_t 0=fail, 1=success
 * Scope       : module
 * Function    : Send a 32-bit word to write to a location on the card. It must
 *			   : have been unlocked with the verify command for this to be
 *			   : successful. Insert a 10ms delay to allow non-volatile technologies
 *			   : to recover from the write cycle
 **************************************************************************/
uint8_t SCOSTA_WriteAsyncCardWord(uint8_t command, uint8_t address, uint32_t Word, uint8_t fRetry)
{
	uint8_t result = FALSE;	// assume bad news
	char retry = 1;
	uint8_t iobuffer[64]; //[8];
	char i;
	uint16_t Battery_vltg;

	SC_Verify_Update = TRUE;
	memset(iobuffer, 0, sizeof(iobuffer));

	do
	{
        iobuffer[0] = SCOSTA_CLASS_TYPE; //SMARTCARD CLASS TYPE
       	iobuffer[1] = command; //SMARTCARD COMMAND TYPE
	    iobuffer[2] = 0x00; //P1
    	iobuffer[3] = 0x00; //P2

    	if(command == SCOSTA_SELECT_FILE)
    	{
    		iobuffer[4] = 0x02; //SMARTCARD_CMD_LENGTH;
    		iobuffer[5] = 0x00;
    		iobuffer[6] = 0x04; //File ID
    		iobuffer[7] = 0x00; //File ID
    	}
    	else
    	{
    		Flash_Batch_Flag_Read();

    		//	bkp_total_invalid_coin = glTotal_0_cents;
    		//	bkp_total_valid_coin = glTotal_Valid_Coin_Count;
    		//	bkp_total_coin = glTotal_Coin_Count;
    		//	bkp_5cents = glTotal_5_cents;
    		//	bkp_10cents = glTotal_10_cents;
    		//	bkp_25cents = glTotal_25_cents;
    		//	bkp_100cents = glTotal_100_cents;
    		Debug_Output1(0, "Before Last_Audit_Time: %ld", Last_Audit_Time1);

    		//calculation for total coin amount //previously it was not there // vinay
    		glTotal_COIN_TransAmount = ((100*glTotal_100_cents)+(25*glTotal_25_cents)+(10*glTotal_10_cents)+(5*glTotal_5_cents));

    	    Debug_Output2(0, "smart SCT_TransCount:%ld, SCT_TransAmount:%ld", glTotal_SCT_TransCount, glTotal_SCT_TransAmount);

    		//previously it was not updating the coin values to audit card only total amount or time was updated// Siouxfalls customer wanted all coin values in card //vinay
    		iobuffer[4] = 0x28; //SMARTCARD_CMD_LENGTH;
			//iobuffer[5] = address++; //Record ID
			//iobuffer[6] = 0x28; //Data Length
			Scosta_Audit_Data.RecordID = address++; //iobuffer[5]; //Record ID Repeat
    		Scosta_Audit_Data.CustomerID = glSystem_cfg.cust_id; //Customer ID
    		Scosta_Audit_Data.MeterID = glSystem_cfg.meter_id; //Meter ID
    		Scosta_Audit_Data.CurrentAuditTimeStamp = RTC_epoch_now(); //Present Audit Time
    		Scosta_Audit_Data.LastAuditTimeStamp = Last_Audit_Time1;//RTC_epoch_now(); //Previous Audit Time
    		Scosta_Audit_Data.NoOfValidCoins 	= glTotal_Valid_Coin_Count;//coin_type0 + coin_type1 + coin_type2 + coin_type3; //Valid Coins Count
    		Scosta_Audit_Data.NoOfInvalidCoins 	= glTotal_0_cents;//coin_type_invalid; //Invalid Coins Count
    		Scosta_Audit_Data.NoOf100CentCoins 	= glTotal_100_cents;//coin_type3;//glTotal_100_cents; //Total 100C Coins
    		Scosta_Audit_Data.NoOf25CentCoins 	= glTotal_25_cents;//coin_type2;//glTotal_25_cents; //Total 25C Coins
    		Scosta_Audit_Data.NoOf10CentCoins 	= glTotal_10_cents;//coin_type1;//glTotal_10_cents; //Total 10C Coins
    		Scosta_Audit_Data.NoOf5CentCoins 	= glTotal_5_cents;//coin_type0;// glTotal_5_cents; //Total 5C Coins
    		Scosta_Audit_Data.TotalCoinAmount 	= glTotal_COIN_TransAmount;//glTotal_Coin_Count;//total_coin_values;//glTotal_COIN_TransAmount; //Need to calculate
    		//Debug_Output2(0, "TotalTrans*********:%ld, TotalAmt*******:%ld", glTotal_SCT_TransCount, glTotal_SCT_TransAmount);
    		Scosta_Audit_Data.NoOfSCCardCount 	= glTotal_SCT_TransCount; //Total SCT Transactions Count
    		Scosta_Audit_Data.TotalSCCardAmount = glTotal_SCT_TransAmount; //Total SCT Transaction Amount
    		//Scosta_Audit_Data.CardDataCRC		= calc_crc32(Scosta_Audit_Data.CardDataCRC, (const uint8_t *)&Scosta_Audit_Data, (sizeof(SCOSTA_AUDIT_DATA)-4));
    		Battery_vltg = Rechargeable_battery_read();
    		//Battery_vltg = Battery_vltg/100;
    		Scosta_Audit_Data.Battery_Voltage = Battery_vltg; //Nebraska cust wants battery voltage to be stored in audit card so using it //vinay
    		Debug_Output6(0, "RecordID: %ld, CustomerID: %ld, MeterID: %ld", Scosta_Audit_Data.RecordID, Scosta_Audit_Data.CustomerID, Scosta_Audit_Data.MeterID, 0, 0, 0);
    		Debug_Output2(0, "CurrentAuditTimeStamp: %ld, LastAuditTimeStamp:%ld",Scosta_Audit_Data.CurrentAuditTimeStamp, Scosta_Audit_Data.LastAuditTimeStamp);
    		Debug_Output2(0, "NoOfValidCoins: %d, NoOfInvalidCoins: %d", Scosta_Audit_Data.NoOfValidCoins, Scosta_Audit_Data.NoOfInvalidCoins);
    		Debug_Output6(0, "NoOf100CentCoins: %d, NoOf25CentCoins: %d, NoOf10CentCoins:%d, NoOf5CentCoins:%d", Scosta_Audit_Data.NoOf100CentCoins, Scosta_Audit_Data.NoOf25CentCoins, Scosta_Audit_Data.NoOf10CentCoins, Scosta_Audit_Data.NoOf5CentCoins,0,0);
    		Debug_Output1(0, "TotalCoin: %ld", Scosta_Audit_Data.TotalCoinAmount);
    		Debug_Output2(0, "NoOfSCCardCount: %d, TotalSCCardAmount: %ld", Scosta_Audit_Data.NoOfSCCardCount, Scosta_Audit_Data.TotalSCCardAmount);
    		Debug_Output1(0, "Battery_vltg: %ld", Scosta_Audit_Data.Battery_Voltage);
    		memset(Scosta_Audit_Data.Reserved, 0 , sizeof(Scosta_Audit_Data.Reserved)); //Reserved for future use
    		memcpy(&iobuffer[6], &Scosta_Audit_Data.RecordID, sizeof(SCOSTA_AUDIT_DATA));
    		Last_Audit_Time1 = Scosta_Audit_Data.CurrentAuditTimeStamp; //to store previous card swipe time //vinay
    		Debug_Output1(0, "Now Last_Audit_Time: %ld", Last_Audit_Time1);

    		Flash_Batch_Flag_Write();
    	}

    	if(SCOSTA_smartcard_write(iobuffer, (iobuffer[4] + 6)) == 1)
		{
			//Debug_TextOut(0, "Update Success 1" );
    		//Debug_Output6(0, "SC CMD: %X %X %X %X %X", iobuffer[0], iobuffer[1], iobuffer[2], iobuffer[3], iobuffer[4], 0);
			result = TRUE;
		}
		retry--;

	} while ((retry > 0) && (fRetry != 0) && (result == FALSE));

	if (result != 0)
	{	// only if write successful
		for (i=0; i<10; i++)
		{
			DelayUs(1000);
		}
	}
	SC_Verify_Update = FALSE;
	return (result);
}

/**************************************************************************
 * Func Name   : SCOSTA_ReadAsyncCardWord
 * Parameters  : Address to read [0..3F], uint32_t pointer for storage
 * Returns     : uint8_t 0=fail, 1=success
 * Scope       : module
 * Function    : Read a word (32-bits) from the smart card. The address
 *				 range is 0..3F. The bytes come of the card in little-endian
 *				 format so they are shuffled to make a DWORD
 **************************************************************************/
uint8_t SCOSTA_ReadAsyncCardWord(uint8_t command, uint8_t address, uint32_t *pWord)
{
	uint8_t result = FALSE;	// assume bad news
	uint8_t iobuffer[8];

	SC_Verify_Update = FALSE;
	memset(iobuffer, 0, sizeof(iobuffer));

    iobuffer[0] = SCOSTA_CLASS_TYPE; //SMARTCARD CLASS TYPE
    iobuffer[1] = command; //SMARTCARD COMMAND TYPE
    iobuffer[2] = address;//(Record_ID - 1);
    iobuffer[3] = 0x04; //address;
	iobuffer[4] = 0x28; //SMARTCARD_CMD_LENGTH;

	if(SCOSTA_smartcard_write(iobuffer, SC_BUFFER_LENGTH) == 1)
	{
		memcpy(pWord, &glSCReadBuf[1],SMARTCARD_CMD_LENGTH);
		//Debug_TextOut(0, "Read Success" );
		//Debug_Output6(0, "SC CMD: %X %X %X %X %X", iobuffer[0], iobuffer[1], iobuffer[2], iobuffer[3], iobuffer[4], 0);
		result = TRUE;
	}
	else
	{
		result = FALSE;
	}
	return (result);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

