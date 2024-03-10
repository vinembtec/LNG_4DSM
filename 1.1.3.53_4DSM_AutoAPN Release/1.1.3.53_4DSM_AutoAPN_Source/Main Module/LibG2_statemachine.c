//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_statemachine.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_statemachine_api
//! @{
//
//*****************************************************************************
#include "LibG2_main.h"

/*******************************************************************************
 * The state machine abstraction: the following typedefs are used to perform a
 * recursive declaration , this is not native to ANSI C.
 * They declare a function that returns a function pointer to a function that
 * returns a function pointer that returns a function pointer to a function that..
 *******************************************************************************/
typedef void (*FuncPtr)(void);
typedef FuncPtr (*MACHINE_STATE)(void);

/*******************************************************************************
 *       Defines
 *******************************************************************************/
/*******************************************************************************
 * for the event/state system to keep the source readable
 * considered bad practice in general but as we are implimenting the
 * state machine as a kind of UML these macros will enforce the structure and
 * standard implimentation of state machine functions (also its difficult
 * to impliment them as functions)
 *******************************************************************************/
#define RETURN_STATE(x)                 return((MACHINE_STATE)x)
#define SET_DEFAULT_NEXT_STATE(x)      	MACHINE_STATE retv = (MACHINE_STATE)x
#define NEXT_STATE                      retv
#define SET_NEXT_STATE(x)               retv = (MACHINE_STATE)x
#define Minute							3600 // 6hours (60*6)

/*******************************************************************************
 *       Private Function Prototypes for :
 *               Machine state prototypes
 *       - Machine state functions must be declared with MACHINE_STATE return type
 *******************************************************************************/

MACHINE_STATE Initialise_State(void);

MACHINE_STATE Idle_State(void);

MACHINE_STATE Meter_In_LPM3_State(void);

MACHINE_STATE Check_Comms_Intervals_State(void);

MACHINE_STATE Take_Meter_Diag_State(void);

MACHINE_STATE Prioritize_Interrupt_State(void);

MACHINE_STATE Read_Credit_Card_State(void);

MACHINE_STATE Identify_Key_State(void);

MACHINE_STATE Start_UDP_Comms_State(void);

MACHINE_STATE RTC_Minutes_ISR_State(void);

MACHINE_STATE Process_RAMQ_Comms_State(void);
MACHINE_STATE Process_LoRa_Comms_State(void);

MACHINE_STATE machinestate = (MACHINE_STATE)Initialise_State;

extern volatile uint16_t				Lora_rx_Count;
//extern volatile uint8_t 				Duty_Cycle_Mode;//not used in this program //vinay
//extern volatile uint8_t					BSU_Chunk_received;//not used in this program //vinay

extern uint32_t 						Tasks_Priority_Register;
extern GPRSSystem      					glSystem_cfg;
extern GPRS_AlarmsRequest   			glAlarm_req;
extern uint16_t             			glCoin_Index;
extern COINDATA			        		glCoindata[MAX_COINS_TO_UDP];
extern RTC_C_Calendar 					rtc_c_calender;
extern volatile uint16_t				Seconds_Counter;
extern GPRS_General_Sync_Response       gprs_general_sync_response;
extern uint32_t 						CC_TimeOut_RTCTimestamp, Last_Cash_Payment_RTC;
extern uint8_t							ccard_dataread_flag, card_removed_flag, check_card_read, Parking_Clock_Refresh[MSM_MAX_PARKING_BAYS], Credit_Card_processed_screen;
extern volatile uint32_t				parking_time_left_on_meter[MSM_MAX_PARKING_BAYS], negetive_parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];
extern uint32_t 						KeyPress_TimeOut_RTC, BKLT_TimeOut_RTCTimestamp, Calibration_TimeOut_RTC, COMMS_DOWN_timestamp;
extern char             				special_keys[6], special_key_count;
extern uint8_t							Tech_menu, SENSOR_OUT_STATUS, sensor_tnx_flag;
extern uint8_t 							In_Queue_Count,do_FD, LoRa_Queue_Count;
extern volatile uint8_t 				Asynchronous_FD;
extern uint8_t             				mdm_comm_status, FIRST_COMM, LCD_BKLT_ON;
extern uint8_t 							/*DLST_FLAG_CHECK,*/ set_DLST, Current_bayStatus;//not used in this program //vinay
extern uint8_t         					glDLST_month, glDLST_day, glDLST_hour;
extern uint16_t        					glDLST_year;
extern uint8_t 							activate_mpb_binary, activate_mpb_config;
extern FlashFieActvnTblEntry			flash_file_actvn_tbl[ FILE_TYPE_TOTAL_FILES ];
extern uint8_t 							smart_card_mode, scosta_smartcard_mode;
//extern uint32_t 						RTC_Update;
extern GPRS_Smartcard_tranRequest 		SCT_Trans_Data;
//extern uint8_t                     		glFlash_rtc_update;

extern uint32_t							CardJam_TimeOut_RTC;
extern uint8_t							CardJam_Flag;
//extern uint32_t  						Max_parking_time;
extern uint32_t 					    Last_Comms_Success_RTCTimestamp;
extern uint8_t 							Last_Comms_Success_Flg;
extern uint8_t							do_PAM_Baystatus_update;
extern GPRS_PamBayStatusUpdateRequest   PAM_Bay_status_data[MAX_PAM_BSU_QUEUE];
extern uint8_t 							PAM_Baystatus_update_count, Tech_key_seq;

//extern uint16_t 						Current_Space_Id;//not suing in this program //vinay
extern uint8_t							gAutoCalibration;//, gSelfCalibration;//not used in this program //vinay
//extern uint8_t							Screen_Clear;  //  BBMP lines issue on display//not used in this program //vinay
//extern uint8_t				            SMS_Print;
extern uint8_t			    			COMMS_DOWN, SMS_MODE;
extern uint8_t		 					LORA_PWR, Running_Space_ID[MSM_MAX_PARKING_BAYS];
extern tRadioDriver 					*Radio;
extern uint8_t 							Connected_Card_reader, CC_Auth_Entered;
//extern uint32_t  						Refund_PreviousSN;//not used in this program //vinay
//extern uint32_t  						Refund_PresentSN;//not used in this program //vinay
extern uint8_t							show_balance_and_deduct;
extern uint8_t							SC_Verify_Update;
//extern uint32_t                         One_min_tick;//not used in this program //vinay
extern uint8_t							gl_Commission_Card;
extern uint8_t							Modem_Off_initiated, in_prepay_parking[MSM_MAX_PARKING_BAYS];
extern uint32_t							Mdm_off_start_timestamp;
extern uint32_t							Last_Sensor_Success_RTCTimestamp;
extern uint8_t							Meter_Reset;
extern uint8_t							dfs_done_ready_to_activate, activate_mpb_binary_from_dk;
extern uint32_t 						payment_on_hold_timeout;
extern uint8_t							payment_on_hold;

extern void activate_a_flash_file( FlashFieActvnTblEntry *actvn_entry_p);

COINDATA        						lCoindata[9];
uint8_t									Coin_Per_Udp_Packet = 0;
uint8_t									TempCoindata[MAX_COINS_TO_UDP*sizeof(COINDATA)] = { 0 };
uint16_t             					lCoinCount = 0;
uint8_t 								do_General_Sync = false,
										do_Events_Update = false,
										//do_Voltage_Read = false,
										do_Alarms_Update = false,
										do_Cash_Update = false,
										do_CC_OLT = false,
										do_SCT_Update = false,
										do_Sensor_Update = false,
										do_file_update = false;

uint16_t 								General_Sync_Interval = 0,
										Event_Upload_Interval = 0,
										Battery_Read_Interval = 0,
										File_Activation_interval = 0;

uint8_t									Process_Flash_Queue = FALSE;
uint8_t									CHECKED_SMS_ON_TIMEOUT = FALSE;
uint8_t 								min_count = 0;
uint8_t 								CARD_JAM_RAISED = FALSE;

uint8_t                                 grace_time_trigger[MSM_MAX_PARKING_BAYS] = {0};
uint8_t                                 real_payment_flag[MSM_MAX_PARKING_BAYS] = {0};
//uint8_t                                 grace_timeout;//not used in this program //vinay
//uint8_t                                 one_time_loop_flag = 0;//not used in this program //vinay

uint8_t									severe_events_send = FALSE;
uint8_t									Add_CC_to_RAM_Q = FALSE;
extern volatile uint8_t					time_retain[MSM_MAX_PARKING_BAYS]; //vinay // to retain parking clock to become zero trenton
extern uint32_t 						parking_time_left_on_meter_prev[MSM_MAX_PARKING_BAYS]; //to round off //vinay
extern uint8_t							No_Modem_Coin_Only; //for coin only meter check
extern uint8_t 							cardread;
extern uint32_t 						coindrop_TimeOut_RTC; //to get the screen back to DSM_Screen1 for coin only meter....//vinay
extern uint32_t 						Screen_TimeOut_RTC;
extern uint8_t 							coin_cal_sens_taken_from_last;
extern uint8_t 							coin_second_check_for_Min_value;
extern volatile uint32_t 				Test_Watchdog_Time;
volatile uint32_t 						Test_Watchdog_Time_1 = 0;
extern uint8_t							ANTI_FEED_in_effect[MSM_MAX_PARKING_BAYS];
extern uint16_t							Total_Events_count,bkp_Events_count;
extern uint32_t							Last_CC_Payment_RTC;
volatile uint8_t 						Parking_Clock_Refresh_flag = 0;
extern uint8_t 							modem_init_to_process_cc_transaction;
/**************************************************************************/
//! This function is the backbone of the event state machine, it sets the
//! initial state and then iteritively calls the state function returned by
//! the last state function call
//! \param void
//! \return void
/**************************************************************************/
void Switch_States()
{
	machinestate = (MACHINE_STATE)machinestate();
	MSP_Status_Check();
}

/**************************************************************************/
//! This function is the Initialization State, it calls the hardware and
//! Software initializers and sets the boot up tasks and then calls the Idle State
//! \param void
//! \return pointer to next machine State
/**************************************************************************/
MACHINE_STATE Initialise_State(void)
{
	FIRST_COMM = TRUE;
	initialize_all();
	//Add all variables, flags, arrays initializations here

	Tasks_Priority_Register |= VOLTAGE_READ_TASK;

	//LNGSIT-1189
	if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == FALSE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE)) //Full LNG
	{
		Tasks_Priority_Register |= GENERAL_SYNC_TASK;
		Tasks_Priority_Register |= EVENTS_UPDATE_TASK;
		Tasks_Priority_Register |= FILE_UPDATE_TASK;
	}
	else if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == TRUE)) //COIN Only LNG with GW Comms
	{
		Tasks_Priority_Register |= GENERAL_SYNC_TASK;
		Tasks_Priority_Register |= EVENTS_UPDATE_TASK;
	}
	else if((No_Modem_Coin_Only == true) || ((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))) //COIN Only LNG with NO Comms
	{
		if(GPIO_getInputPinValue(KEYDETECT_PORT, KEYDETECT_PIN) == 0)
		{
			Tech_menu=true;
			smart_card_mode = FALSE;
			Debug_TextOut( 0, "DK Connected: LNG is in tech mode" );
			//push_event_to_cache( UDP_EVTTYP_TECH_MENU_ACCESS );//09-04-12
	        Copy_only_MPB_config_2mech();//Vinay requirement for coin only meter
	        Copy_only_MPB_firmware_2mech(); //Vinay requirement for coin only meter
			//TechMenu_Display_Main_Menu();
		}
		Meter_Reset = FALSE;	//this is important because meter won't go to General sync or RTC attempts, so make this flag false here itself
	}

#ifdef Test_Flash_Batch_Erase
	uint8_t i;
	for(i=0; i<128; i++)
	{
		Flash_Batch_Erase((uint32_t)((uint32_t)i*8192), 1, true);
	}
#endif

	RETURN_STATE(Idle_State);
}

/**************************************************************************/
//! This function is the Idle State, it checks and prioritizes any tasks
//! marked in Tasks_Priority_Register. It also calls the file activation
//! routine if there is any file activation pending and then calls the Idle
//! State. If no tasks pending, then it calls the Sleep routine of Meter.
//! \param void
//! \return pointer to next machine State
/**************************************************************************/
MACHINE_STATE Idle_State(void)
{
	uint32_t loCurrentRTCTime = RTC_epoch_now();

	if((loCurrentRTCTime > Test_Watchdog_Time + 2)&&(Test_Watchdog_Time_1!=RTC_epoch_now()))
		Debug_Output2(0,"WT:%ld,CT:%ld", Test_Watchdog_Time,loCurrentRTCTime); //TESTING VT

	Test_Watchdog_Time_1 = RTC_epoch_now();  //TESTING VT

	if( ( (coindrop_TimeOut_RTC > 0) && (loCurrentRTCTime > (coindrop_TimeOut_RTC + 30))  ) &&
	    ( (KeyPress_TimeOut_RTC > 0) && (loCurrentRTCTime > (KeyPress_TimeOut_RTC + 30))  )&&
	    ( (Screen_TimeOut_RTC > 0  ) && (loCurrentRTCTime > (Screen_TimeOut_RTC + 30  ))  ) )
	{
		//Debug_Output1(0,"coindrop_TimeOut_RTC:%ld", coindrop_TimeOut_RTC);
		//Debug_Output1(0,"KeyPress_TimeOut_RTC:%ld", KeyPress_TimeOut_RTC);
		//Debug_Output1(0,"Screen_TimeOut_RTC:%ld", Screen_TimeOut_RTC);
		//Debug_Output1(0,"loCurrentRTCTime:%ld", loCurrentRTCTime);
		//Debug_Output1(0,"cardread:%ld", cardread);
		coindrop_TimeOut_RTC = 0;
		KeyPress_TimeOut_RTC = 0;
		Screen_TimeOut_RTC = 0;
		if(cardread == 0)
		DSM_Screen1();
	}
	if( ( (coindrop_TimeOut_RTC > 0) && (loCurrentRTCTime > (coindrop_TimeOut_RTC + 30)) ) || (loCurrentRTCTime < coindrop_TimeOut_RTC) )
		coindrop_TimeOut_RTC = 0;
	if( ( (KeyPress_TimeOut_RTC > 0) && (loCurrentRTCTime > (KeyPress_TimeOut_RTC + 30)) ) || (loCurrentRTCTime < KeyPress_TimeOut_RTC) )
		KeyPress_TimeOut_RTC = 0;
	if( ( (Screen_TimeOut_RTC > 0  ) && (loCurrentRTCTime > (Screen_TimeOut_RTC + 30))   ) || (loCurrentRTCTime < Screen_TimeOut_RTC)   )
		Screen_TimeOut_RTC = 0;
	//Debug_TextOut(0,"Idle_States");
	SET_DEFAULT_NEXT_STATE(Idle_State);

	if(Tasks_Priority_Register != 0)
	{
		Exit_from_LPM();
		SET_NEXT_STATE(Prioritize_Interrupt_State);
	}
	else
	{
		if((activate_mpb_config == true) && (dfs_done_ready_to_activate == TRUE))
		{
			activate_a_flash_file(flash_file_actvn_tbl);
			activate_mpb_config = false;
		}
		else if(((activate_mpb_binary == true) && (dfs_done_ready_to_activate == TRUE)) || (activate_mpb_binary_from_dk == TRUE))	//Activate MPB Binary only in idle state
		{
			activate_a_flash_file(flash_file_actvn_tbl + 1);
			activate_mpb_binary = false;
		}
		else if(do_FD == TRUE)
		{
			do_FD = FALSE;
			Asynchronous_FD = TRUE;
			Trigger_Asynchronous_FD();
		}
		else if(LoRa_Queue_Count > 0)
		{
			Tasks_Priority_Register |= PROCESS_LORA_QUEUE_TASK;
		}
		else if((In_Queue_Count > 0) || (Process_Flash_Queue == TRUE))
		{
	//		Debug_Output2(0,"%d,%d", In_Queue_Count, Process_Flash_Queue);
			Tasks_Priority_Register |= PROCESS_RAM_QUEUE_TASK;
		}
		else
			SET_NEXT_STATE(Meter_In_LPM3_State);
	}

	RETURN_STATE(NEXT_STATE);
}

/**************************************************************************/
//! This function is the Sleep State, When there are no taks pending, meter
//! comes into this State. This calls the LPM3 function of MSP432.
//! \param void
//! \return pointer to next machine State
/**************************************************************************/
MACHINE_STATE Meter_In_LPM3_State(void)
{
	SET_DEFAULT_NEXT_STATE(Idle_State);

	Meter_regular_sleep();

	RETURN_STATE(NEXT_STATE);
}

/**************************************************************************/
//! This function is the Tasks prioritization State, When there are any tasks
//! marked in Tasks_Priority_Register, Idle State calls this function to
//! check and prioritize the tasks list and return the next state to execute
//! the task
//! \param void
//! \return pointer to next machine State
/**************************************************************************/
MACHINE_STATE Prioritize_Interrupt_State(void)
{
	SET_DEFAULT_NEXT_STATE(Prioritize_Interrupt_State);
	uint32_t task_priority_check = 0, new_task_priority_check = 0;
	uint8_t isr_counter = 0, task_pending = true;

	task_priority_check = Tasks_Priority_Register;

	for(isr_counter=0; isr_counter<32; isr_counter++)
	{
		task_pending = false;
		new_task_priority_check = task_priority_check & (BIT_0 << isr_counter);

		switch(new_task_priority_check)	//If any interrupt is set, identify it and process
		{
		case SENSOR_UPDATE_TASK:					//Highest Priority Interrupt, Sensor data processing
			do_Sensor_Update = TRUE;
			SET_NEXT_STATE(Start_UDP_Comms_State);
			break;
		case MAGTEK_CARD_ISR:					//Credit Card
			cardread = 1; //to avoid processing please wait in between other operations // vinay
			//Debug_TextOut(0,"SM ISRCardRead=1");
			SET_NEXT_STATE(Read_Credit_Card_State);
			break;
		case PROCESS_LORA_QUEUE_TASK:
			SET_NEXT_STATE(Process_LoRa_Comms_State);
			break;
		case KEYPADS_ISR:
			SET_NEXT_STATE(Identify_Key_State);
			break;
		case INTERVALS_ISR:	//used for VCOM toggling
			SET_NEXT_STATE(Check_Comms_Intervals_State);
			break;
/*		case PAM_UPDATE_TASK:
			do_PAM_Baystatus_update = TRUE;
			SET_NEXT_STATE(Start_UDP_Comms_State);
			break;*/
		case RTC_ISR:
			SET_NEXT_STATE(RTC_Minutes_ISR_State);
			break;
		case CREDIT_CARD_OLT_TASK:
			do_CC_OLT = TRUE;
			Add_CC_to_RAM_Q = TRUE;
			card_removed_flag = false; //LNGSIT-753
			//Start CC TimeOut Time
			CC_TimeOut_RTCTimestamp = RTC_epoch_now();
			SET_NEXT_STATE(Start_UDP_Comms_State);
			break;
		case CASH_UPDATE_TASK:
			do_Cash_Update = TRUE;
			SET_NEXT_STATE(Start_UDP_Comms_State);
			break;
		case SCT_UPDATE_TASK:
			do_SCT_Update = TRUE;
			SET_NEXT_STATE(Start_UDP_Comms_State);
			break;
		case GENERAL_SYNC_TASK:
			do_General_Sync = TRUE;
			SET_NEXT_STATE(Start_UDP_Comms_State);
			break;
		case VOLTAGE_READ_TASK:
			SET_NEXT_STATE(Take_Meter_Diag_State);
			break;
		case EVENTS_UPDATE_TASK:
			do_Events_Update = TRUE;
			SET_NEXT_STATE(Start_UDP_Comms_State);
			break;
		case FILE_UPDATE_TASK:
			do_file_update = TRUE;//only to send 161
			SET_NEXT_STATE(Start_UDP_Comms_State);
			break;
		case SEVERE_EVENTS_TASK:
			severe_events_send = TRUE;
			SET_NEXT_STATE(Start_UDP_Comms_State);
			break;
		case PROCESS_RAM_QUEUE_TASK:
			SET_NEXT_STATE(Process_RAMQ_Comms_State);
			break;
		case FILE_DOWNLOAD_TASK:
			do_FD = TRUE;
			SET_NEXT_STATE(Idle_State);
			break;
		default:
			task_pending = true;
			SET_NEXT_STATE(Idle_State);
			break;
		}

		if(task_pending == false)	//task identified and NEXT_STATE is set for the task, clear this task from main priority register now
		{
			Tasks_Priority_Register &= ~(new_task_priority_check);
			break;
		}
	}

	RETURN_STATE(NEXT_STATE);
}

/**************************************************************************/
//! This function is the Intervals Check State. It is called once in about
//! 500ms by prioritize interval state. It checks all kinds of timeouts,
//! checks if any comms interval has reached, refreshes parking clock display
//! etc. and then returns the next state.
//! \param void
//! \return pointer to next machine State
/**************************************************************************/
MACHINE_STATE Check_Comms_Intervals_State(void)
{
	SET_DEFAULT_NEXT_STATE(Check_Comms_Intervals_State);
	uint32_t loCurrentRTCTime = RTC_epoch_now();
	uint16_t elapsed_time3 = 0;
	uint8_t i = 0;

	lcd_vcom_pat();

	if(COMMS_DOWN == TRUE)
	{
		if(CHECKED_SMS_ON_TIMEOUT == FALSE)
		{
			if(loCurrentRTCTime > (COMMS_DOWN_timestamp + DEFAULT_SMS_READ_TIMEOUT))	//1 hour passed after comms down, scan through the SMS to see if any SMS received
			{
				SMS_MODE = TRUE;
				CHECKED_SMS_ON_TIMEOUT = TRUE;
				//Debug_TextOut(0, "No SMS RI in 5 minutes, scan all SMS once");
			}
		}
		if(loCurrentRTCTime > (COMMS_DOWN_timestamp + (DEFAULT_SMS_READ_TIMEOUT*2)))	//2 hour passed after comms down, allow it to switch off modem
		{
			COMMS_DOWN = FALSE;
			//Debug_TextOut(0, "No SMS RI in 10 minutes, disable RI");
			disable_telit_RI_interrupt();	//No SMS from 2 hours, sleep
		}
	}

	if(Modem_Off_initiated == 1)	//stepwise modem off was initiated somewhere
	{
		if(loCurrentRTCTime > (Mdm_off_start_timestamp + DEFAULT_MODEM_OFF_TIMEOUT))	//ready now
		{
			Modem_Off_initiated = 2;
			telit_power_off();
		}
	}

	if((card_removed_flag == TRUE) || (smart_card_mode == TRUE))
	{
		if(loCurrentRTCTime > (CC_TimeOut_RTCTimestamp + (glSystem_cfg.showamount_5sec_timeout * 5)))
		{
			// Clear all CC flags
			Debug_TextOut(0,"Card Process Timeout-01");
			smart_card.detected = FALSE;
			show_balance_and_deduct = 0;
			SC_Verify_Update = FALSE;
			smart_card_mode = FALSE;
			ccard_dataread_flag = FALSE;
			card_removed_flag = FALSE;
			check_card_read = FALSE;
			do_CC_OLT = FALSE;
			CC_Auth_Entered = FALSE;
			CC_TimeOut_RTCTimestamp = 0;
			Turn_Off_card_IOs(Connected_Card_reader);
			if(modem_init_to_process_cc_transaction)
			{
				Modem_Off_initiated = 2; //modem off for card if on //vinay
				telit_power_off(); //modem off for card if on //vinay
			}
			//Debug_Output1(0, "SM 486 cardread: %d", cardread);
			Idle_Screen(); //when card processing is taking too much long then without displaying decline or approve it will come to ideal screen so to infrom customer to reinsert card //vinay
			//if(cardread == 1) //if they dont do any amount select and press ok for processing then it will go this screen and exit//vinay
			//    card_process_timeout_screen(); //
		}
	}
	else if ((CC_Auth_Entered == TRUE) || (do_CC_OLT == TRUE))
	{
		if(loCurrentRTCTime > (CC_TimeOut_RTCTimestamp + (glSystem_cfg.showamount_5sec_timeout * 15)))
		{
			// Clear all CC flags
			Debug_TextOut(0,"Card Process Timeout-02");
			smart_card.detected = FALSE;
			show_balance_and_deduct = 0;
			SC_Verify_Update = FALSE;
			smart_card_mode = FALSE;
			ccard_dataread_flag = FALSE;
			card_removed_flag = FALSE;
			check_card_read = FALSE;
			do_CC_OLT = FALSE;
			CC_Auth_Entered = FALSE;
			CC_TimeOut_RTCTimestamp = 0;
			Turn_Off_card_IOs(Connected_Card_reader);
			if(modem_init_to_process_cc_transaction)
			{
				Modem_Off_initiated = 2; //modem off for card if on //vinay
				telit_power_off(); //modem off for card if on //vinay
			}
			//Debug_Output1(0, "SM 514 cardread: %d", cardread);
            Idle_Screen(); //when card processing is taking too much long then without displaying decline or approve it will come to ideal screen so to infrom customer to reinsert card //vinay
			//if(cardread == 0)
			//    card_process_timeout_screen(); //
		}
	}
	else if(Credit_Card_processed_screen == TRUE)
	{
		if(loCurrentRTCTime > (CC_TimeOut_RTCTimestamp + 5))	//5 seconds timeout for approved/ declined screen
		{
			Credit_Card_processed_screen = FALSE;
			if(1 == GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN))
			{
				if(Connected_Card_reader == GEM_CLUB_READER)
				{
					/*Remove_Card_Screen();
					while ((0 == GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN)) && (++elapsed_time3 < 3000) ) //WAIT FOR CARD REMOVAL-5SECS
					{
						DelayMs(1);
					}*/
				}
				else
				{
					while ((1 == GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN)) && (++elapsed_time3 < 3000) ) //WAIT FOR CARD REMOVAL-5SECS
					{
						DelayMs(1);
					}
				}

				if ( elapsed_time3 >= 3000 )//if time out
				{
					Debug_TextOut( 0, "Card Remove timeout-02" );
					CardJam_TimeOut_RTC = RTC_epoch_now();
					CardJam_Flag = 1;
					push_event_to_cache( UDP_EVTTYP_CARD_REMOVE_TIMEOUT );
					if(modem_init_to_process_cc_transaction)
					{
						Modem_Off_initiated = 2; //modem off for card if on //vinay
						telit_power_off(); //modem off for card if on //vinay
					}
					//Debug_Output1(0, "SM 554 cardread: %d", cardread);
		            Idle_Screen(); //when card processing is taking too much long then without displaying decline or approve it will come to ideal screen so to infrom customer to reinsert card //vinay
				    //card_process_timeout_screen(); //
				}
			}
			//if((KeyPress_TimeOut_RTC == 0) && (coindrop_TimeOut_RTC == 0))
			Idle_Screen();
		}
	}

	//glSystem_cfg.allowed_trans_types |= (1<<13); //For temporary , pems not ready ; VT
	//glSystem_cfg.allowed_trans_types |= (1<<14); //For temporary , pems not ready ; VT
	if(glCoin_Index > 0)
	{
		//Debug_Output1(0,"Coin_Per_Udp_Packet = %d",Coin_Per_Udp_Packet);
		if((((glSystem_cfg.allowed_trans_types>>13) & 0x01) == 1) || (((glSystem_cfg.allowed_trans_types>>14) & 0x01) == 1))
			Coin_Per_Udp_Packet = COINS_PER_UDP_PACKET_CT125;
		else
			Coin_Per_Udp_Packet = COINS_PER_UDP_PACKET_CT55;

		//if(loCurrentRTCTime > (Last_Cash_Payment_RTC + 30))  VT if anyone pressing any key also cash update should not happen
		if((((loCurrentRTCTime > (coindrop_TimeOut_RTC + 30))) || (loCurrentRTCTime < coindrop_TimeOut_RTC))&&
			(((loCurrentRTCTime > (KeyPress_TimeOut_RTC + 30))) || (loCurrentRTCTime < KeyPress_TimeOut_RTC)))
		{
			if(coin_cal_sens_taken_from_last)
				{//>>VT<< Base Value taken from after to second Max value point due to fast coin drop
					push_event_to_cache(UDP_EVTTYP_COIN_BASE_VALUE_FROM_LAST);
					coin_cal_sens_taken_from_last = FALSE;
				}
			if(coin_second_check_for_Min_value)
				{//>>VT<< Second check for Min value in Coin profile
					push_event_to_cache(UDP_EVTTYP_COIN_MIN_PROFILE_SECOND_CHECK);
					coin_second_check_for_Min_value = FALSE;
				}
			Tasks_Priority_Register |= CASH_UPDATE_TASK;
		}
	}

	if(glSystem_cfg.vehicle_sensor_enable == TRUE)	//LNGSIT-567
		Zero_Out_On_vacant(); //Zero Out after 30 seconds of stable vacant

	if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == FALSE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))//3oct18
	{
		if((mdm_comm_status != 4) && (Last_Comms_Success_Flg == FALSE))
		{
			if(loCurrentRTCTime > (Last_Comms_Success_RTCTimestamp + glSystem_cfg.meter_hard_reset_interval))
			{
				if((glSystem_cfg.meter_hard_reset_interval != 0) && (Meter_Reset == FALSE))//Meter was resetting before even trying for RTC when junk RTC at bootup
				{
					Debug_TextOut(0,"Restart Meter due to no comms");
					telit_power_off();
					__disable_interrupt();
					//MAP_SysCtl_rebootDevice();
					//SYSCTL->REBOOT_CTL = 0x6901;
					WDTCTL = WDTPW+WDTCNTCL+WDTSSEL1+WDTIS_7;
					DelayMs(10);
				}
			}
		}
	}

	if(glSystem_cfg.vehicle_sensor_enable == 1) //LNGSIT-623
	{
		if(loCurrentRTCTime > (Last_Sensor_Success_RTCTimestamp + glSystem_cfg.meter_hard_reset_interval))
		{
			SENSOR_OUT_STATUS = 1;
		}
	}

	if(payment_on_hold == true)
	{
		if(loCurrentRTCTime > (payment_on_hold_timeout + DEFAULT_PAYMENT_HOLD_TIMEOUT))
		{
			//payment_on_hold = false;
			DSM_Screen5();
		}
	}

	if(LCD_BKLT_ON == TRUE)
	{
		if(loCurrentRTCTime > (BKLT_TimeOut_RTCTimestamp + DEFAULT_BKLT_TIMEOUT))
		{
			LCD_Backlight(FALSE);
		}
	}

	if(gAutoCalibration == TRUE)
	{
		if(loCurrentRTCTime > (Calibration_TimeOut_RTC + DEFAULT_CALIBRATION_TIMEOUT))
		{
			gAutoCalibration = false;
			Debug_TextOut(0, "Timeout, Exit Calib Mode");
			Idle_Screen();
		}
	}

	if((Tech_menu == TRUE) || (special_key_count > 0))
	{
		if(loCurrentRTCTime > (KeyPress_TimeOut_RTC + DEFAULT_KEYPRESS_TIMEOUT))
		{
			// Clear all CC flags
			//special_key_timeout=false;
			if(Tech_menu == TRUE)
			{
				Debug_TextOut(0, "Timeout, LNG Exited Tech Mode"); //LNGSIT-1003
			}
			initialise_tech_menu_param();//TODO: needs extensive test; Resolution for LNGSIT-822
			Tech_menu = false;
			Tech_key_seq = FALSE;
			special_key_count = 0;
			memset(special_keys,0,sizeof(special_keys));
			KeyPress_TimeOut_RTC = 0;

			Idle_Screen();
		}
	}

	if(CardJam_Flag  == TRUE)
	{
		if(glSystem_cfg.card_jam_detection_enable == TRUE)
		{
			if(loCurrentRTCTime > (CardJam_TimeOut_RTC + glSystem_cfg.Card_Jam_Timeout))
			{
				if(Connected_Card_reader == GEM_CLUB_READER)
				{
					if ((GPIO_getInputPinValue(MAGTEK_INTR_PORT,MAGTEK_INTR_PIN)) == 0)
					{
						CARD_JAM_RAISED = TRUE;
						Debug_TextOut(0,"GemClub CardJam Raised");
						glAlarm_req.alarm_type  =   92;
						glAlarm_req.alarm_size  =	3;
						glAlarm_req.alarm_ts    = 	RTC_epoch_now();
						glAlarm_req.alarm_state =	MB_ALARM_STATE_RAISED;    //MB_ALARM_STATE_RAISED
						glAlarm_req.alarm_id    =  	UDP_ALMTYP_CARD_JAM;
						gprs_do_Alarms(&glAlarm_req);
						Idle_Screen();
					}
				}
				else
				{
					if ((GPIO_getInputPinValue(MAGTEK_INTR_PORT,MAGTEK_INTR_PIN)) == 1)
					{
						CARD_JAM_RAISED = TRUE;
						Debug_TextOut(0,"IDTech CardJam Raised");
						glAlarm_req.alarm_type  =   92;
						glAlarm_req.alarm_size  =	3;
						glAlarm_req.alarm_ts    = 	RTC_epoch_now();
						glAlarm_req.alarm_state =	MB_ALARM_STATE_RAISED;    //MB_ALARM_STATE_RAISED
						glAlarm_req.alarm_id    =  	UDP_ALMTYP_CARD_JAM;
						gprs_do_Alarms(&glAlarm_req);
						Idle_Screen();
					}
				}
				CardJam_Flag = 0;
				CardJam_TimeOut_RTC = 0;
				Idle_Screen();
			}
		}
		else
		{
			//Debug_TextOut(2, "CardJam Detection is Disabled in Config");
		}
	}
	else if (CARD_JAM_RAISED == TRUE)
	{
		if(Connected_Card_reader == GEM_CLUB_READER)
		{
			if ((GPIO_getInputPinValue(MAGTEK_INTR_PORT,MAGTEK_INTR_PIN)) == 1)
			{
				CARD_JAM_RAISED = FALSE;
				Debug_TextOut(0,"GemClub CardJam Cleared");
				glAlarm_req.alarm_type  =   92;
				glAlarm_req.alarm_size  =	3;
				glAlarm_req.alarm_ts    = 	RTC_epoch_now();
				glAlarm_req.alarm_state =	MB_ALARM_STATE_CLEARED;    //MB_ALARM_STATE_CLEARED
				glAlarm_req.alarm_id    =  	UDP_ALMTYP_CARD_JAM;
				gprs_do_Alarms(&glAlarm_req);
				Idle_Screen();
			}
		}
		else
		{
			if ((GPIO_getInputPinValue(MAGTEK_INTR_PORT,MAGTEK_INTR_PIN)) == 0)
			{
				CARD_JAM_RAISED = FALSE;
				Debug_TextOut(0,"IDTech CardJam Cleared");
				glAlarm_req.alarm_type  =   92;
				glAlarm_req.alarm_size  =	3;
				glAlarm_req.alarm_ts    = 	RTC_epoch_now();
				glAlarm_req.alarm_state =	MB_ALARM_STATE_CLEARED;    //MB_ALARM_STATE_CLEARED
				glAlarm_req.alarm_id    =  	UDP_ALMTYP_CARD_JAM;
				gprs_do_Alarms(&glAlarm_req);
				Idle_Screen();
			}
		}
	}

	//if((Parking_Clock_Refresh > PARKING_CLOCK_STOP_MODE) && (gl_Commission_Card == FALSE))
	if((gl_Commission_Card == FALSE)) //parking expire msg was not printing when expired // so to fix it using this changes //vinay
	{
		for(i=0; i<MSM_MAX_PARKING_BAYS; i++)
		{
			if(Parking_Clock_Refresh[i] > PARKING_CLOCK_STOP_MODE)
			{
				//if(Running_Space_ID[i] == TRUE)	// It is OK to do here, because it comes here only if it was a 00:01 to 00:00 transition //vinay
				if((parking_time_left_on_meter[i] == 0))
				{
					//Debug_Output2(0,"1Running_Space_ID[%d] = %d", i , Running_Space_ID[i]);
					//Debug_Output2(0,"1parking_time_left_on_meter[%d] = %d", i , parking_time_left_on_meter[i]);
					if(Running_Space_ID[i] == TRUE)//if((parking_time_left_on_meter[i] == 0))// && (Running_Space_ID[i] == TRUE))	// It is OK to do here, because it comes here only if it was a 00:01 to 00:00 transition
					{
						if((glSystem_cfg.grace_time_type == NO_GRACE_TIME) || ((negetive_parking_time_left_on_meter[i] >= (glSystem_cfg.grace_time*60)
								&& (grace_time_trigger[i] == true))) || (in_prepay_parking[i] == TRUE))	// It is OK to do here, because it comes here only if it was a 00:01 to 00:00 transition
						{
							negetive_parking_time_left_on_meter[i] = 0;
							grace_time_trigger[i] = false;
							in_prepay_parking[i]  = FALSE;
							time_retain[i] = 0;
							parking_time_left_on_meter_prev[i] = 0;
							Debug_Output1(0,"Time Expired for Space: %d", (i));
							Running_Space_ID[i] = FALSE; //when spaces expires within few secs apart then expired exents for some were missing so placed FALSE here to avoid missing //vinay
							sensor_tnx_flag = TRUE;
							Current_bayStatus = i;
							push_event_to_cache(UDP_EVTTYP_PARKING_TIME_EXPIRED);
							sensor_tnx_flag = FALSE;
							Flash_RTC_ParkingClock_Storage(true, false);
							ANTI_FEED_in_effect[i] = false;
						}
						else if((grace_time_trigger[i] == false) && (real_payment_flag[i] == TRUE) && (in_prepay_parking [i]== FALSE))
						{
							negetive_parking_time_left_on_meter[i] = 0;
							//Debug_TextOut(0,"Time Expired Grace time Started \r\n");
							Debug_Output1(0,"Time Expired Grace time started for Space: %d", (i));
							grace_time_trigger[i] = true;
							real_payment_flag[i] = FALSE;
							Flash_RTC_ParkingClock_Storage(true, false);
						}
						Update_Parking_Clock_Screen();
					}
					else
					{
						if(Parking_Clock_Refresh[i] == PARKING_CLOCK_RESET_MODE)	//New Payment
						{
							//Debug_TextOut(0,"Parking Clock");
							Update_Parking_Clock_Screen();
							LCD_Backlight(TRUE);
						}
						else	//Regular Count Down
						{
							//Debug_TextOut(0,"Parking Clock");
							Update_Parking_Clock_Screen();
						}
					}
					Parking_Clock_Refresh[i] = PARKING_CLOCK_STOP_MODE;
					//DelayMs(500); //if expire time for both space are same then it would miss 1, so giving some delay to check both. //LNGSIT-2007 //vinay
				}
			}
		}
	}

	//if(wdt_base_Counter >= INTERVALS_CHECK_COUNTER)
	if(Seconds_Counter >= SECS_PER_TICK)
	{
		//Debug_Output6(2,"Q= %d,%d,%d", LoRa_Queue_Count, In_Queue_Count, Process_Flash_Queue, 0, 0, 0);
		//Debug_Output6(2,"flags= %d,%d,%d,%d,%d", glSystem_cfg.transactions_via_GPRS, glSystem_cfg.OLT_via_GPRS, glSystem_cfg.MODEM_POWER, TELIT_SHUTDOWN, Modem_Off_initiated, 0);
		//wdt_base_Counter = 0;
		Seconds_Counter = 0;
		//Debug_TextOut(0,"30SEC INTR");

		General_Sync_Interval += SECS_PER_TICK;
		Event_Upload_Interval += SECS_PER_TICK;
		Battery_Read_Interval += SECS_PER_TICK;
		File_Activation_interval ++;

		if((Tech_menu == FALSE) && (Tech_key_seq == TRUE))
		{
			Tech_key_seq = FALSE;
		}

		if(General_Sync_Interval >= glSystem_cfg.SYNC_INTERVAL)
		{
			Tasks_Priority_Register |= GENERAL_SYNC_TASK;
			General_Sync_Interval = 0;
		}

		if(Event_Upload_Interval >= glSystem_cfg.EVT_INTERVAL)
		{
			Tasks_Priority_Register |= EVENTS_UPDATE_TASK;
			Event_Upload_Interval = 0;
		}

		if(Battery_Read_Interval >= glSystem_cfg.DIAG_INTERVAL)
		{
			Tasks_Priority_Register |= VOLTAGE_READ_TASK;
			Battery_Read_Interval = 0;
		}

		if(File_Activation_interval >= FACTVN_INTERVAL)
		{
			Tasks_Priority_Register |= FILE_UPDATE_TASK;
			File_Activation_interval = 0;
		}

/*		if(glCoin_Index > 0)
		{
			Tasks_Priority_Register |= CASH_UPDATE_TASK;
		}*/

/*		if(PAM_Baystatus_update_count > 0)
		{
			//do_PAM_Baystatus_update = TRUE;
			//Tasks_Priority_Register |= SEVERE_EVENTS_TASK;//this was causing multiple modem on cycles
			Tasks_Priority_Register |= PAM_UPDATE_TASK;//this is used to trigger Start_UDP_Comms immediately which will go through LoRa not GPRS
		}*/

		/*if((gprs_general_sync_response.meter_action > 0XF0) && (gprs_general_sync_response.meter_action < 0XFF)) //25/09/2015-LOB
		{
			Debug_TextOut(2,"0XF1 - 0XFE - LNG LNG Special Actions\r\n");

			switch(gprs_general_sync_response.meter_action)
			{
			case 0XF1://reset MB and MPB
				Debug_TextOut(0,"0XF1 - LNG Reset CMD\r\n");
				Flash_RTC_ParkingClock_Storage(true, false);	//RTC append in flash every minute
				telit_power_off();
				__disable_interrupt();
				WDTCTL = WDTPW+WDTCNTCL+WDTSSEL1+WDTIS_7;
				DelayMs(10);
				break;
			case 0XF2://reset LNG only
				Debug_TextOut(0,"0XF2 - LNG Reset CMD\r\n");
				Flash_RTC_ParkingClock_Storage(true, false);	//RTC append in flash every minute
				telit_power_off();
				__disable_interrupt();
				WDTCTL = WDTPW+WDTCNTCL+WDTSSEL1+WDTIS_7;
				DelayMs(10);
				break;
			case 0XF3://reset sensor
				//FORCE_MC_RESET = TRUE;
				break;
			case 0XF4://meter inactive
				break;

			default :
				break;
			}

			gprs_general_sync_response.meter_action = 0;
		}*/
	}

	SET_NEXT_STATE(Idle_State);

	RETURN_STATE(NEXT_STATE);
}

/**************************************************************************/
//! This function is the Meter's Diagnostic State. It is called when the
//! diagnostic interval is reached. It takes all diagnostic reading like
//! battery voltage, temperature etc. and then returns the next State
//! \param void
//! \return pointer to next machine State
/**************************************************************************/
MACHINE_STATE Take_Meter_Diag_State(void)
{
	SET_DEFAULT_NEXT_STATE(Take_Meter_Diag_State);

	SET_NEXT_STATE(Idle_State);

	get_battery_voltage_temp();

	RETURN_STATE(NEXT_STATE);
}

/**************************************************************************/
//! This function is the Meter's Communication State. It is called when there
//! is any communication interval reached or there is anything to send to server.
//! \param void
//! \return pointer to next machine State
/**************************************************************************/
MACHINE_STATE Start_UDP_Comms_State(void)
{
	uint8_t  i,/*j,*/k,l;
	SET_DEFAULT_NEXT_STATE(Idle_State);

	if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))//3oct18
	{
		do_CC_OLT = FALSE;
		CC_Auth_Entered = FALSE;
		Add_CC_to_RAM_Q = FALSE;
		RETURN_STATE(NEXT_STATE); //Coin Only, No Comms (Standby meter)
	}

	if((do_CC_OLT == TRUE) && (Add_CC_to_RAM_Q == TRUE))
	{
		Add_CC_to_RAM_Q = FALSE;
		pre_process_CC_payment();
		initiate_OLT();
		InitForNewRead();	//Clear all CC related flags
		SET_NEXT_STATE(Idle_State);
	}

	if(do_Sensor_Update == TRUE)
	{
		poll_space_status();
		do_Sensor_Update = FALSE;
		SET_NEXT_STATE(Idle_State);
	}

	if(do_Cash_Update == TRUE)
	{
		/*if(glCoin_Index <= COINS_PER_UDP_PACKET)
		{
			//copy to a local buffer for posting and make the global buffer available for new coin transactions
			memcpy(lCoindata, glCoindata, sizeof(lCoindata));
			lCoinCount = glCoin_Index;

			memset(glCoindata, 0, sizeof(glCoindata));
			glCoin_Index = 0;
		}
		else
		{
			memcpy(lCoindata, glCoindata, sizeof(lCoindata));
			lCoinCount = COINS_PER_UDP_PACKET;

			memset(TempCoindata, 0, sizeof(TempCoindata));
			memcpy(TempCoindata, &glCoindata[COINS_PER_UDP_PACKET].coin_index, (sizeof(glCoindata) - sizeof(lCoindata)));
			memset(glCoindata, 0, sizeof(glCoindata));
			memcpy(glCoindata, TempCoindata, (sizeof(glCoindata) - sizeof(lCoindata)));
			glCoin_Index -= COINS_PER_UDP_PACKET;
		}*/

		for(l = 0; l < MSM_MAX_PARKING_BAYS; l++)
		{
			k = 0;
			for(i = 0; i < glCoin_Index; i++)
			{
				if(glCoindata[i].space_id == l)
				{
					lCoindata[k].coin_index = glCoindata[i].coin_index;
					lCoindata[k].cashtranstime = glCoindata[i].cashtranstime;
					lCoindata[k].coinval = glCoindata[i].coinval;
					lCoindata[k].cumulative_parktime_hr = glCoindata[i].cumulative_parktime_hr;
					lCoindata[k].cumulative_parktime_min = glCoindata[i].cumulative_parktime_min;
					lCoindata[k].earned_coin_hr = glCoindata[i].earned_coin_hr;
					lCoindata[k].earned_coin_min = glCoindata[i].earned_coin_min;
					lCoindata[k].max = glCoindata[i].max;  //to send invalid coin profile //NVK
					lCoindata[k].min = glCoindata[i].min;  //to send invalid coin profile //NVK
					lCoindata[k].rat1 = glCoindata[i].rat1;  //to send invalid coin profile //NVK
					lCoindata[k].rat2 = glCoindata[i].rat2;  //to send invalid coin profile //NVK
					k++;
				}

				if(k >= Coin_Per_Udp_Packet)
				{
					//Debug_TextOut(0,"Processing Please Wait.....CTStart");
					lCoinCount = k;
					gprs_do_CT(l);
					k = 0;
					//Debug_TextOut(0,"Processing Please Wait.....CTEnd");
				}
			}

			if(k != 0) //If there are less than 9 coins data
			{
				//Debug_TextOut(0,"Processing Please Wait.....CTStart");
				lCoinCount = k;
				gprs_do_CT(l);
				//Debug_TextOut(0,"Processing Please Wait.....CTEnd");
			}
		}
		glCoin_Index = 0; //Reset Coin Index
		do_Cash_Update = FALSE;

		SET_NEXT_STATE(Idle_State);
	}

	if(do_SCT_Update == TRUE)
	{
		do_SCT_Update = FALSE;
		gprs_do_SCT(&SCT_Trans_Data);
		SET_NEXT_STATE(Idle_State);
	}

/*	if(do_PAM_Baystatus_update == TRUE)
	{
		for(j = 0; j < PAM_Baystatus_update_count; j++)
		{
			PAM_Baystatus_update(&PAM_Bay_status_data[j]);
		}
		PAM_Baystatus_update_count = 0;
		do_PAM_Baystatus_update = FALSE;
	}*/

	if(do_General_Sync == TRUE)
	{
		do_General_Sync = FALSE;

		if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == FALSE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))//3oct18
		{
			Debug_TextOut(0,"GEN SYNC INTERVAL");
			gprs_do_get_General_Sync_request();
		}
		else
		{
			Debug_TextOut(0,"GET_RTC from GW");
			get_RTC();
		}

		SET_NEXT_STATE(Idle_State);
	}

	if((do_Events_Update == TRUE) || (severe_events_send == TRUE))
	{
		//Debug_Output1(0, "Total_Events_count = %ld" , Total_Events_count); //to check event counts //vinay
		//Debug_Output1( 0, "bkp_Events_count = %d", bkp_Events_count );
		if(severe_events_send == TRUE)
		{
			Debug_TextOut(0,"EVENTS INTERVAL SEVERE EVENTS TRUE");
			severe_events_send = FALSE;
			gprs_do_send_event_cache(false,0);
		}
		else if((Total_Events_count > 0) || (bkp_Events_count > 0)) //trying to send events if any in event interval //vinay testing
		{
			Debug_TextOut(0,"EVENTS INTERVAL BOTH");
			severe_events_send = FALSE;
			gprs_do_send_event_cache(false,0);
			do_Events_Update = FALSE;
			gprs_do_send_event_cache(true,0);
		}
		else
		{
			Debug_TextOut(0,"EVENTS INTERVAL SEVERE EVENTS FALSE");
			//Debug_TextOut(0,"EVENTS INTERVAL");
			do_Events_Update = FALSE;
			gprs_do_send_event_cache(true ,0);
		}

		SET_NEXT_STATE(Idle_State);
	}

	if(do_Alarms_Update == TRUE)
	{
		do_Alarms_Update = FALSE;
		gprs_do_Alarms(&glAlarm_req);
		SET_NEXT_STATE(Idle_State);
	}

    //LNGSIT-745,785,696: Moved below to Idle State
/*	if(do_FD == TRUE)
	{
		do_FD = FALSE;
		Asynchronous_FD = TRUE;
		Trigger_Asynchronous_FD();
		SET_NEXT_STATE(Idle_State);
	}*/

	if(do_file_update == TRUE)
	{
		do_file_update = FALSE;
		check_n_activate_files_from_flash();
		SET_NEXT_STATE(Idle_State);
	}
	/*if(mdm_comm_status > 3)
	{
		process_queue();
		SET_NEXT_STATE(Idle_State);
	}*/
/*	if(set_DLST==true)
	{
		//Debug_TextOut(0, "Adjust DLST Time");
		do_daylight_saving();
		SET_NEXT_STATE(Idle_State);
	}*/

	RETURN_STATE(NEXT_STATE);
}

/**************************************************************************/
//! This function is the Card Reading State. It calls the Card reader modules
//! and returns next state after reading the Card.
//! \param void
//! \return pointer to next machine State
/**************************************************************************/
MACHINE_STATE Read_Credit_Card_State(void)
{
	SET_DEFAULT_NEXT_STATE(Read_Credit_Card_State);
	cardread = 1;
	if((do_CC_OLT == FALSE) && (card_removed_flag == FALSE) && (smart_card_mode == FALSE)
			&& (scosta_smartcard_mode == FALSE))	//allow new card transaction only when it is not in between amount selection or in between OLT for previous CC or SC swipe
		credit_card_insert_isr();

	SET_NEXT_STATE(Idle_State);

	RETURN_STATE(NEXT_STATE);

}

/**************************************************************************/
//! This function is the Keypad detection State. It calls the Keypad processor
//! and returns next state after completing keypad action.
//! \param void
//! \return pointer to next machine State
/**************************************************************************/
MACHINE_STATE Identify_Key_State(void)
{
	SET_DEFAULT_NEXT_STATE(Identify_Key_State);

	Process_keypress();

	SET_NEXT_STATE(Idle_State);

	RETURN_STATE(NEXT_STATE);
}

/**************************************************************************/
//! This function is the RTC minutes change State. It does optical alarm check
//! writes current RTC and parking Clock to flash, checks and refreshes rates
//! and schedules as per current time, repaints the LCD and returns next state.
//! \param void
//! \return pointer to next machine State
/**************************************************************************/
MACHINE_STATE RTC_Minutes_ISR_State(void)
{
	SET_DEFAULT_NEXT_STATE(RTC_Minutes_ISR_State);
	uint32_t loCurrentRTCTime = RTC_epoch_now();
	uint8_t processwait2 = 0;
	//Debug_TextOut(2,"MIN INTR");

	rtc_c_calender = RTC_C_getCalendarTime();
	//LoRa recovery: if no LoRa packet received in last 6 hours, restart LoRa
	if(min_count++ == Minute)
	{
		min_count = 0;
		if(Lora_rx_Count == 0)
		{
			//LNGSIT-917
			LORA_PWR = FALSE;
			LORA_power_off();
			Debug_TextOut(0,"Refresh LoRa");
		}
		else
			Lora_rx_Count = 0;
	}
	//Debug_TextOut(0,"MIN INTR - 2");
#if	1
	if(glSystem_cfg.DLST_ON_OFF==1)
	{
		if( (rtc_c_calender.year==glDLST_year) && (rtc_c_calender.month==glDLST_month) && (rtc_c_calender.dayOfmonth==glDLST_day) && (rtc_c_calender.hours==glDLST_hour) )
		{
			if(rtc_c_calender.minutes == 0)
			{
				Debug_TextOut(0, "Set DLST Flag");
				set_DLST = true;
				do_daylight_saving();
			}
		}
	}
#endif

	Check_Coin_Jam_Alarm(NO_WAKEUP_COIL_CHECK);//Check Coin Jam status and then Raise alarm or clear alarm

	if((rtc_c_calender.minutes % 1) == 0)	//Sync with RTC every 1 minutes
	{
		Update_RTC_on_Display();
		//Debug_TextOut(0, "RTC Updated");
	}

	if((rtc_c_calender.minutes % (RTC_PARK_CLK_SYNC_INTERVAL/60)) == 0)	//Sync with RTC every 1(old 5min) minutes
	{
		Flash_RTC_ParkingClock_Storage(true, true);	//RTC append in flash every minute
	}

	if((rtc_c_calender.minutes % (RT_PARK_CLK_TO_FLASH_INTERVAL/60)) == 0) //save RTC and parking clock for every 10 mins
	{
		//process please wait is sreekanth's reqirement to check whether its sync or not // vinay
		//this is avoid please wait msg during CC payment //vinay)
		if((cardread == 1)||
				(loCurrentRTCTime < (Last_CC_Payment_RTC + 30))||
				((loCurrentRTCTime < (coindrop_TimeOut_RTC + 30))&&(coindrop_TimeOut_RTC < (loCurrentRTCTime + 30)))||
				((loCurrentRTCTime < (KeyPress_TimeOut_RTC + 30)) && (KeyPress_TimeOut_RTC < loCurrentRTCTime + 30)))	//this is avoid please wait msg during CC payment //vinay)
		{
			//Debug_TextOut(0,"Events295CardRead=1");
			Debug_TextOut(0,"Processing Please Wait.....");
		}
		else
		{
			processwait2 = 1;
			Debug_TextOut(0,"Processing Please Wait.....");
			Clear_Screen();
			printSharp32x48(" PROCESSING ",25,0,0);//printSharp32x48(" THIS METER ",35,0,0);
			printSharp32x48("   PLEASE    ", 80,0,0);//printSharp32x48("   COVERS   ", 80,0,0);
			printSharp32x48("    WAIT.....     ", 135,0,0);//printSharp32x48(" TWO SPACES  ", 125,0,0);
		}
		if((loCurrentRTCTime > (Last_Cash_Payment_RTC + 30))||(loCurrentRTCTime < (Last_Cash_Payment_RTC - 30)))
				Flash_RTC_ParkingClock_Storage(true, false);	//RTC append in flash every minute
		if(processwait2 == 1)
		{
			//Debug_TextOut(0,"6");
			DSM_Screen1();//Idle_Screen();  //this is causing long hold of processing please wait msg. //so uncmmted //vinay
			processwait2 = 0;
		}
	}

	read_and_fill_current_schedule(&rtc_c_calender);
	//Commented below for memory optimization: We never use modem ON config in LNG
/*	if(glSystem_cfg.MODEM_POWER == 1)	//save time by not entering in this at all
	{
		Check_Power_Save_Hours(rtc_c_calender.dayOfWeek);
	}*/
	SET_NEXT_STATE(Idle_State);

	RETURN_STATE(NEXT_STATE);
}

/**************************************************************************/
//! This function is the GPRS Communication State. It calls the modem processor
//! to send the queued GPRS communication packets(both online and offline).
//! \param void
//! \return pointer to next machine State
/**************************************************************************/
MACHINE_STATE Process_RAMQ_Comms_State(void)
{
	SET_DEFAULT_NEXT_STATE(Idle_State);
	uint32_t loCurrentRTCTime = RTC_epoch_now();
	uint16_t result = SOCERR_NONE;
	if((loCurrentRTCTime > (Last_Cash_Payment_RTC + 30))||(loCurrentRTCTime < (Last_Cash_Payment_RTC - 30)))
	{
	result = telit_sock_open_states();
	if((result == SOCERR_NONE) || (result != SOCERR_IP_STK_OPEN))
	{
		if(In_Queue_Count > 0)
		{
			process_RAM_Queue();
		}
		else if(Process_Flash_Queue == TRUE)
		{
			New_process_queue();
		}
	}
	}
	RETURN_STATE(NEXT_STATE);
}


/**************************************************************************/
//! This function is the LORA Communication State. It calls the LORA processor
//! to send the queued LORA communication packets.
//! \param void
//! \return pointer to next machine State
/**************************************************************************/
MACHINE_STATE Process_LoRa_Comms_State(void)
{
	SET_DEFAULT_NEXT_STATE(Idle_State);
	uint32_t loCurrentRTCTime = RTC_epoch_now();
	if((loCurrentRTCTime > (Last_Cash_Payment_RTC + 30))||(loCurrentRTCTime < (Last_Cash_Payment_RTC - 30)))//||((LoRa_Queue[Que_count].Packet[1]!=125)&&(LoRa_Queue[Que_count].Packet[1]!=55)))
	{
		process_LORA_Queue();
	}
	RETURN_STATE(NEXT_STATE);
}
/*void check_pending_requests()
{
	Start_UDP_Comms_State();
}*/

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

