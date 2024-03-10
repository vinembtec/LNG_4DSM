//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_main.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_main_api
//! @{
//
//*****************************************************************************

#include "LibG2_main.h"

extern volatile uint16_t 		minutes_left_in_current_schedule;

extern GPRSSystem      			glSystem_cfg;
extern GPRS_BayStatusRequest	glBay_req;
extern volatile uint32_t		parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];
extern volatile uint32_t	  	negetive_parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];
//extern uint8_t 					flash_data_read_buffer[ FLASH_DATA_READ_BUF_SIZE ]; //not using in this program //vinay
extern uint8_t     				rx_ans[ GPRS_GEN_BUF_SZ_SMALL ];
extern MBInfo					glMB_info;
extern uint8_t         			glComm_failure_count;
extern uint8_t         			COMMS_DOWN/*,gReInit_Mdm*/,Initial_boot, SENSOR_OUT_STATUS;//not using in this program //vinay
extern uint8_t                	mdm_comm_status;
//extern RTC_C_Calendar 			rtc_c_calender;//not suing in this program //vinay
//extern uint32_t					COMMS_DOWN_timestamp;//not using in this program //vinay
uint32_t						Last_Sensor_Success_RTCTimestamp = 0;//it was delcared in com.c and it was not used there, so declared it here//vinay
extern uint8_t 					PAM_Baystatus_update_count, grace_time_trigger[MSM_MAX_PARKING_BAYS];
extern uint8_t					Parking_Clock_Refresh[MSM_MAX_PARKING_BAYS], Current_bayStatus;
extern uint8_t					do_CC_OLT, card_removed_flag, smart_card_mode, ANTI_FEED_in_effect[MSM_MAX_PARKING_BAYS];
//extern tLoRaSettings 			LoRaSettings, LoRaSettings1;//not using in this program //vinay
extern GPRS_PamBayStatusUpdateRequest  PAM_Bay_status_data[MAX_PAM_BSU_QUEUE];
extern uint32_t					santa_cruz_previous_paid_time;
//extern uint16_t					Current_Space_Id;//not suing in this program //vinay
extern uint32_t					Last_processed_time[Max_Sensor];

const uint8_t  					PCI_VERSION = 1, MPB_VERSION = 1, sub_VERSION = 3, branch_version = 53;	//1.1.3.53_4DSM//REV3 + REV4/4.1 LNG Merge

volatile uint32_t				revert_parking_clock = 0;
uint8_t 						Space_Zero_Out[MSM_MAX_PARKING_BAYS] = {0,0}, VAC_ZERO_ELIGIBLE[MSM_MAX_PARKING_BAYS] = {0,0},
								glLast_reported_space_status[MSM_MAX_PARKING_BAYS] = {99,99};
								//Space_Zero_Out = FALSE, VAC_ZERO_ELIGIBLE = FALSE,
								//ZERO_OUT_ENABLE_AT_EXPIRY = FALSE, ZERO_OUT = TRUE,
uint8_t 						ZERO_OUT_ENABLE_AT_EXPIRY[MSM_MAX_PARKING_BAYS] = {0,0}, ZERO_OUT[MSM_MAX_PARKING_BAYS] = {1,1}; //to use it for dual space//vinay
uint32_t 						last_vacant_timestamp[MSM_MAX_PARKING_BAYS] = {0,0}, last_payment_try_time[MSM_MAX_PARKING_BAYS] = {0,0};
uint8_t 						/*last_minute = 99,*/SMS_MODE = FALSE;//not used in this program //vinay
uint8_t							/*ZERO_OUT_CONF = false,*/ ELIGIBLE_FOR_ZERO_OUT = 0;
uint8_t					        Normal_occupied=false,Clean_occupied=false, Unclean_occupied_count=0;
uint16_t					    occupancygl=99;//,last_occupancy=99;//not used in this program //vinay
int16_t 						glSNSR_TMP=0;
uint8_t 						glSNSRdiag_string[SENSOR_DIAG_DATA_LENGTH] = {0};
uint8_t 						DIAG_EXCEPTION = FALSE;
uint8_t 						sensor_tnx_flag = FALSE;
GPRS_BayStatusRequest  			glBay_req;

//extern uint8_t					No_Modem_Coin_Only; //for coin only meter check//not using in this program //vinay
extern volatile uint8_t 		current_rate_index; //sensor zero out was clearing the parking clock in prepay schedule to avoid that and it was customer requirement //vinay
extern uint16_t					received_meter_id; //vinay
extern uint8_t					Meter_Full_Flg[MSM_MAX_PARKING_BAYS];
extern uint8_t 					Meter_Full[MSM_MAX_PARKING_BAYS];
volatile uint32_t 				Test_Watchdog_Time = 0;
extern uint8_t                  APN_Assigned;
extern bool                     t_mobile_apn_switch_flag;
bool                            t_mobile_selected_apn_retry_flag = FALSE;
/**************************************************************************/
//! Main function for the user Code. The ARM Cortex processor will transfer
//! the control to this point after the internal processor initialization.
//! It does the Statemachine switching by calling the Switch_States() in
//! an infinite loop.
//! \param void
//! \return void
/**************************************************************************/
void main(void)
{
	WDT_A_holdTimer();           // Stop watchdog timer
	//best_LPM_settings();	//for boards testing only

	for(;;)
	{
		Switch_States();
	}
}

/**************************************************************************/
//! This function is used to pat the external watchdog by sending an alive
//! pulse to it to prevent unexpected resets. This function needs to called
//! whenever there is any wait code or anything that takes longer execution time.
//! \param void
//! \return void
/**************************************************************************/
void watchdog_pat()
{
	//WDT_A_clearTimer();
	//Debug_TextOut(0,"Watchdog Pat");
	Test_Watchdog_Time = RTC_epoch_now(); //TESTING VT
	GPIO_setOutputLowOnPin(WD_DSP_PORT, WD_DSP_PIN);
	DelayUs(50);
	GPIO_setOutputHighOnPin(WD_DSP_PORT, WD_DSP_PIN);
}

/**************************************************************************/
//! This function is used to periodically check Meter's operational. It
//! attempts to recover from some failures like GPRS failures, No communication
//! for 24 hours etc.
//! \param void
//! \return void
/**************************************************************************/
void MSP_Status_Check()
{
	//watchdog_pat();
	if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == FALSE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))//3oct18
	{
		//All MSP Error recovery, watchdog refresh etc. to be written here
		if(glComm_failure_count > glSystem_cfg.Queue_failure_count)	//Error Recovery for continuous modem failures
		{
		    /******for tmobile SIM*******/
		    if(((APN_Assigned==3) || (APN_Assigned==4))&&(t_mobile_selected_apn_retry_flag == FALSE)) t_mobile_apn_switch_flag ^= TRUE; //Need to Toggle for chaning tmobile apn
		    t_mobile_selected_apn_retry_flag = FALSE; //this flag will set if any communication become success and next time retry with same apn without changing/swithcing apn
		    /*************/
		    APN_Assigned = 0; // for entering to Auto Apn steps -- 21/02/2022 -- VT

			//Debug_TextOut(2,"tried gprs modem recovery");
			Modem_Recovery_Loop();

#ifdef ENABLE_SMS_MODE
			if((Initial_boot == TRUE) && (COMMS_DOWN == TRUE))
			{
				if(Send_SMS_request() == 0)
				{
					Initial_boot = FALSE;
					Debug_TextOut(0,"SMS REQ SENT");

					enable_telit_RI_interrupt();
					//wait for sms response
				}
				else
				{
					disable_telit_RI_interrupt();
					Debug_TextOut(0,"SMS REQ FAIL");
				}
			}

			if(SMS_MODE == TRUE)
			{
				uint8_t index = 0,result =MDM_ERR_NONE;
				SMS_MODE = FALSE;
				if(mdm_comm_status >= 5)
				{
					//glClose_socket = TRUE;
					telit_sock_close();
					DelayMs(1000);
				}
				else if(mdm_comm_status < 1)
				{
					telit_power_off();
					DelayMs(800);
					telit_power_on();
					mdm_init();
				}
				watchdog_pat();
				DelayMs(1000);
				if(COMMS_DOWN == TRUE)
				{
					Read_Validate_SMS();
				}
				else
				{
					for(index=0;index<3;index++)
					{
						result=mdm_send_AT_command("AT+CMGD=1,4\r\n",rx_ans, GPRS_GEN_BUF_SZ_VSMALL, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
						DelayMs(1000);
						watchdog_pat();
						if(result == MDM_ERR_NONE)
							break;
					}
				}
			}
#endif
		}

#ifdef ENABLE_SMS_MODE
		if(SMS_MODE == TRUE)
		{
			SMS_MODE = FALSE;
			if(mdm_comm_status >= 5)
			{
				telit_sock_close();
				DelayMs(1000);
			}
			else if(mdm_comm_status < 1)
			{
				telit_power_off();
				DelayMs(800);
				telit_power_on();
				mdm_init();
			}

			watchdog_pat();
			DelayMs(1000);
			Read_Validate_SMS();
		}
#endif
	}
}

/**************************************************************************/
//! This function is used to print some boot up messages both in debug logs
//! and meter LCD.
//! \param void
//! \return void
/**************************************************************************/
void Display_Meter_Info()
{
	uint32_t lRFFrequency = 0;

	liberty_sprintf((char *)rx_ans,"LNG_Version:%d.%d.%d.%d_4DSM", (int)PCI_VERSION, (int)MPB_VERSION, (int)sub_VERSION, (int)branch_version);
	diag_text_Screen( rx_ans, TRUE, FALSE );

	liberty_sprintf((char *)rx_ans,"CUST_ID: %u", glSystem_cfg.cust_id);
	diag_text_Screen( rx_ans, TRUE, FALSE );

	liberty_sprintf((char *)rx_ans,"METER_ID: %u", glSystem_cfg.meter_id);
	diag_text_Screen( rx_ans, TRUE, FALSE );

	liberty_sprintf((char *)rx_ans,"AREA_NUM: %u", glSystem_cfg.area_num);
	diag_text_Screen( rx_ans, TRUE, FALSE );

	liberty_sprintf((char *)rx_ans,"LOCATION_ID: %s", glMB_info.locid);
	diag_text_Screen( rx_ans, TRUE, FALSE );

	liberty_sprintf((char *)rx_ans,"CITY_CODE: %d", glMB_info.city_code);
	diag_text_Screen( rx_ans, TRUE, FALSE );

	lRFFrequency = IDBasedFreqCalculation(glSystem_cfg.meter_id ); // Freq Calculation
	liberty_sprintf((char *)rx_ans, "LoRa: %d(G), %d(S)", glSystem_cfg.Active_Channel, (uint32_t)lRFFrequency/1000000L);
	diag_text_Screen( rx_ans, TRUE, FALSE );

	if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == FALSE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))//3oct18
	{
		liberty_sprintf((char *)rx_ans,"UDP_IP: %s", glSystem_cfg.UDP_server_ip);
		diag_text_Screen( rx_ans, TRUE, FALSE );

		liberty_sprintf((char *)rx_ans,"UDP_PORT: %d", glSystem_cfg.UDP_server_port);
		diag_text_Screen( rx_ans, TRUE, FALSE );

		liberty_sprintf((char *)rx_ans,"APN: %s", glSystem_cfg.apn);
		diag_text_Screen( rx_ans, TRUE, FALSE );
	}

	//LNGSIT-1024, 1025, 1026
	if(glSystem_cfg.wakeup_coil_jam_detection_enable == TRUE)
		Debug_TextOut(0, "Wakeup Coil Jam Enabled");
	else
		Debug_TextOut(0, "Wakeup Coil Jam Disabled");


	if(glSystem_cfg.optical_jam_detection_enable == TRUE)
		Debug_TextOut(0, "Optical Jam Enabled");
	else
		Debug_TextOut(0, "Optical Jam Disabled");


	if(glSystem_cfg.card_jam_detection_enable == TRUE)
		Debug_TextOut(0, "CardJam Enabled");
	else
		Debug_TextOut(0, "CardJam Disabled");

	/*liberty_sprintf((char *)rx_ans,"%s", "Meter Init Complete");
	diag_text_Screen( rx_ans, TRUE );

	Idle_Screen();*/
}

/**************************************************************************/
//! This function handles the Bay Status updates received from Sensor via LoRa.
//! It generates LOB_BayStatus packets to be sent to Server, it logs appropriate
//! events for Occupied, Vacant. It handles the Meter's business logic related
//! to Sensor like - this does Zero Out on Entry, Free Time On Entry, Revert
//! Parking Clock in case of errors and logs events for each action taken.
//! \param void
//! \return void
/**************************************************************************/
void poll_space_status()
{
	//uint8_t temp_bay_status_count=99;
	uint32_t current_space_timestamp = 0;

	//Debug_Output1(0,"glSystem_cfg.meter_id = %d",glSystem_cfg.meter_id);
	//Debug_Output1(0,"Current_bayStatus = %d",Current_bayStatus);
	if(received_meter_id == glSystem_cfg.meter_id)
		Current_bayStatus = 0;
	else
		Current_bayStatus = 1;
	Debug_Output1(0,"Received_Meter_ID = %d",received_meter_id);

	//Initialize the BSU structure
	glBay_req.SNSR_TMP = 0;
	memset(glBay_req.diag_string, 0, sizeof(glBay_req.diag_string));
	glBay_req.expiry_time = 0;
	glBay_req.occupancy = 0; //99;
	glBay_req.purchased_duration = 0;
	glBay_req.time_stamp = Last_processed_time[Current_bayStatus];	//There was RTC mismatch between sensor BSU and Meter BSU causing issues in PSOA table, so use the same time as received from sensor
	glBay_req.PAMtxn_time = glBay_req.time_stamp;
	glBay_req.txn_type = Tx_Grace_Period; //0;

	if(glSystem_cfg.vehicle_sensor_enable == 1)
	{
		SENSOR_OUT_STATUS = 1; //LNGSIT-623
		Last_Sensor_Success_RTCTimestamp = RTC_epoch_now();

		if((occupancygl == 1) && (ELIGIBLE_FOR_ZERO_OUT != 0))//13-12-2013
		{
			Unclean_occupied_count++;
		}
		else if((occupancygl == 1) && (ELIGIBLE_FOR_ZERO_OUT == 0))
		{
			Clean_occupied = true;
		}
		else if(occupancygl == 0)
		{
			Clean_occupied	= false;
			Normal_occupied	= false;
			Unclean_occupied_count = 0;
		}
		if(Unclean_occupied_count >= 1)//26-12-2013
		{
			//ZERO_OUT_CONF 	= TRUE;
			Normal_occupied	= true;
			Clean_occupied	= false;
			Unclean_occupied_count = 0;
		}
		if(occupancygl == 0)
		{
			current_space_timestamp	=	RTC_epoch_now();
			last_vacant_timestamp[Current_bayStatus] 	= 	current_space_timestamp;

			//If vacant came within 3 mins of occupied, it is unlikely that vehicle moved out of space, driver might adjust his parking again
			if(((current_space_timestamp - last_payment_try_time[Current_bayStatus]) > glSystem_cfg.TIME_Check_AT_EXPIRY) && (ZERO_OUT_ENABLE_AT_EXPIRY[Current_bayStatus] == FALSE))//03-01-2014
			{
				ZERO_OUT[Current_bayStatus] = TRUE; // mark it eligible for zero out, if key pressed, not eligible again
				ZERO_OUT_ENABLE_AT_EXPIRY[Current_bayStatus] = FALSE;
			}
			else if(((current_space_timestamp - last_payment_try_time[Current_bayStatus]) <= glSystem_cfg.TIME_Check_AT_EXPIRY) && (parking_time_left_on_meter[Current_bayStatus] != 0))
			{
				ZERO_OUT_ENABLE_AT_EXPIRY[Current_bayStatus] = TRUE;
				ZERO_OUT[Current_bayStatus] = FALSE;
				//Debug_TextOut(0,"Vehicle Out before TIME_Check_AT_EXPIRY, Won't Zero Out till next 801 event");
				Debug_Output1( 0, "Vehicle Out before TIME_Check, Won't Zero Out till 801 event for Space[%d]", Current_bayStatus );
			}

			if(glLast_reported_space_status[Current_bayStatus] != 0)
			{
				//Update_RTC_on_Display();
				sensor_tnx_flag = TRUE;
				push_event_to_cache( UDP_EVTTYP_VEHICLE_OUT );
				sensor_tnx_flag = FALSE;
				//Debug_TextOut( 0, "Vacant" );
				Debug_Output1( 0, "Vacant for Space[%d]", Current_bayStatus);

				if(glSystem_cfg.santa_cruz_spl_feature_enable == TRUE)
					santa_cruz_previous_paid_time = 0;

				if(ANTI_FEED_in_effect[Current_bayStatus] == true)	//Anti Feed feature enabled in config
				{
					ANTI_FEED_in_effect[Current_bayStatus] = false;	//ready for next payment now since the last vehicle has left
					push_event_to_cache(UDP_EVTTYP_ANTI_FEED_CLEAR);
					//Debug_TextOut(0,"Anti Feed Cleared");
					Debug_Output1( 0, "Anti Feed Cleared for Space[%d]", Current_bayStatus );
				}

				glBay_req.occupancy	=	occupancygl;
				//Do not Update the following two here, if they are not updated, let those go as 0
				//glBay_req.purchased_duration = 0;
				//glBay_req.txn_type = Tx_Unknown;  //Unknown because nothing changed in parking clock, to make it backward compatible
				current_space_timestamp	=	RTC_epoch_now();
				glBay_req.PAMtxn_time = glBay_req.time_stamp = current_space_timestamp;
				glBay_req.expiry_time = current_space_timestamp + parking_time_left_on_meter[Current_bayStatus];
				glBay_req.SNSR_TMP = glSNSR_TMP;
				memcpy(glBay_req.diag_string, glSNSRdiag_string, sizeof(glSNSRdiag_string));
				gprs_do_LOB_BayStatus();

				glLast_reported_space_status[Current_bayStatus] = 0;	//set last reported status and reset the confirmation counters
			}
		}
		else if(occupancygl	==	1)
		{
			//Debug_TextOut( 2, "occupancygl-01 == 1" );
			//if(((glLast_reported_space_status != 1) || (ZERO_OUT_CONF == TRUE)))// need to avoid resending the same status in the case where BS counters got reset before one status got confirmed
			if(glLast_reported_space_status[Current_bayStatus] != 1)
			{
				last_vacant_timestamp[Current_bayStatus] = 0;

				//Debug_TextOut( 2, "occupancygl-02 == 1" );
				if(((glSystem_cfg.Sensor_action_control & NON_ZO_IN_BIT) == NON_ZO_IN_BIT) || (ELIGIBLE_FOR_ZERO_OUT == 0))
				{
					//Update_RTC_on_Display();
					sensor_tnx_flag = TRUE;
					push_event_to_cache( UDP_EVTTYP_VEHICLE_IN );
					sensor_tnx_flag = FALSE;
					//Debug_TextOut( 0, "Occupied" );
					Debug_Output1( 0, "Occupied for Space[%d]", Current_bayStatus);
					if(glSystem_cfg.santa_cruz_spl_feature_enable == TRUE)
						santa_cruz_previous_paid_time = 0;

					if(ANTI_FEED_in_effect[Current_bayStatus] == true)	//Anti Feed feature enabled in config
					{
						ANTI_FEED_in_effect[Current_bayStatus] = false;	//ready for next payment now since the last vehicle has left
						push_event_to_cache(UDP_EVTTYP_ANTI_FEED_CLEAR);
						//Debug_TextOut(0,"Anti Feed Cleared");
						Debug_Output1( 0, "Antifeed Cleared for Space[%d]", Current_bayStatus);
					}
				}

				if(Clean_occupied == true)
				{
					//Debug_Output1(0,"Current_bayStatus = %d", Current_bayStatus);
					//Debug_Output1(0,"ZERO_OUT[Current_bayStatus] = %d", ZERO_OUT[Current_bayStatus]);
					//Debug_Output2(0,"On Entry ZERO_OUT[%d] = %d", Current_bayStatus, ZERO_OUT[Current_bayStatus]);
					//Debug_Output2(0,"glLast_reported_space_status[%d] = %d", Current_bayStatus, glLast_reported_space_status[Current_bayStatus]);
					//if((ZERO_OUT[Current_bayStatus] == TRUE) && ((glLast_reported_space_status != 1) || (ZERO_OUT_CONF == TRUE)))
					//if((ZERO_OUT[Current_bayStatus] == TRUE) && (glLast_reported_space_status[Current_bayStatus] != 1))
					if((ZERO_OUT[Current_bayStatus] == TRUE) && (glLast_reported_space_status[Current_bayStatus] != 1))
					{
						if((glSystem_cfg.Sensor_action_control & ZO_ENTRY_BIT) == ZO_ENTRY_BIT)//ZEROTIME_on_VEHICLE_ARRIVAL
						{
							Space_Zero_Out[Current_bayStatus] = TRUE;
							sensor_tnx_flag = TRUE;
							push_event_to_cache( UDP_EVTTYP_ZEROOUT_ON_OCCUPIED );//Raise the event first and then zero Out
							revert_parking_clock = parking_time_left_on_meter[Current_bayStatus];
							//Debug_Output1(0,"revert_parking_clock= %d", revert_parking_clock);
							//sensor zero out was clearing the parking clock in prepay schedule to avoid zero out in prepay and it was customer requirement //vinay
							if(current_rate_index != PREPAY_PARKING)
							{

								if((parking_time_left_on_meter[Current_bayStatus] > 0) || (negetive_parking_time_left_on_meter[Current_bayStatus] > 0))
								{
									push_event_to_cache(UDP_EVTTYP_PARKING_TIME_EXPIRED);	//LNGSIT-570
								}
								sensor_tnx_flag = FALSE;
								parking_time_left_on_meter[Current_bayStatus] = 0;
								negetive_parking_time_left_on_meter[Current_bayStatus] = 0; //LNGSIT-640
								grace_time_trigger[Current_bayStatus] = false;
								Parking_Clock_Refresh[Current_bayStatus] = PARKING_CLOCK_STOP_MODE;

								//Debug_TextOut( 0, "Zero Out on Entry" );
								Debug_Output1( 0, "Zero Out On Entry for Space[%d]", Current_bayStatus);

								ANTI_FEED_in_effect[Current_bayStatus] = false;	//ready for next payment now since the last vehicle has left
								Meter_Full[Current_bayStatus] = FALSE;
								Meter_Full_Flg[Current_bayStatus] = FALSE; // antifeed for progressive rate was creating problem so added this //vinay
								current_space_timestamp	=	RTC_epoch_now();
								//glBay_req.PAMtxn_time 	= 	glBay_req.time_stamp = current_space_timestamp;
								//glBay_req.occupancy		=	occupancygl;
								glBay_req.purchased_duration = parking_time_left_on_meter[Current_bayStatus];
								glBay_req.expiry_time 	= 	current_space_timestamp + parking_time_left_on_meter[Current_bayStatus];
								glBay_req.txn_type 		= 	Tx_Sensor_Reset;
								//glBay_req.SNSR_TMP 		= 	glSNSR_TMP;
								//memcpy(glBay_req.diag_string, glSNSRdiag_string, sizeof(glSNSRdiag_string));
							}
							else
							{
								//Debug_TextOut( 0, "No 'Zero out on Entry' its prepay schedule" );
								Debug_Output1( 0, "No 'Zero Out On Entry' for Space[%d] its prepay schedule", Current_bayStatus);
								//Debug_Output1(0,"minutes_left_in_current_schedule=%ld",minutes_left_in_current_schedule);
								parking_time_left_on_meter[Current_bayStatus] = minutes_left_in_current_schedule*60; //if customer has paid some money and if he moves out before time expire, then the remaining time in prepay will only reflect the extra time will removed //vinay
								//Debug_Output1(0,"parking_time_left_on_meter=%ld",parking_time_left_on_meter);
								Debug_TextOut( 0, "Removing Extra time if paid in prepay" );
								ANTI_FEED_in_effect[Current_bayStatus] = false;	//ready for next payment now since the last vehicle has left
								Meter_Full[Current_bayStatus] = FALSE;
								Meter_Full_Flg[Current_bayStatus] = FALSE; // antifeed for progressive rate was creating problem so added this //vinay
								Update_Parking_Clock_Screen();
							}
						}
						if((glSystem_cfg.Sensor_action_control & FREE_TIME_BIT) == FREE_TIME_BIT)//Grant_FREETIME_on_VEHICLE_ARRIVAL
						{
							//TODO: Free Time on Entry Logic
							parking_time_left_on_meter[Current_bayStatus] = (glSystem_cfg.free_time * 60);
							if(parking_time_left_on_meter[Current_bayStatus] > 0)
							{
								Parking_Clock_Refresh[Current_bayStatus] = PARKING_CLOCK_RESET_MODE;	//Avoid spending time in parking clock display here, it will be done outside
							}
							//Debug_TextOut( 0, "Free Time on Entry" );
							Debug_Output1( 0, "Free Time On Entry for Space[%d]", Current_bayStatus);
							current_space_timestamp	=	RTC_epoch_now();
							//glBay_req.PAMtxn_time 	= 	glBay_req.time_stamp = current_space_timestamp;
							//glBay_req.occupancy		=	occupancygl;
							glBay_req.purchased_duration = parking_time_left_on_meter[Current_bayStatus];
							glBay_req.expiry_time 	= current_space_timestamp + parking_time_left_on_meter[Current_bayStatus];
							glBay_req.txn_type 		= Tx_Grace_Period;
							//glBay_req.SNSR_TMP 		= glSNSR_TMP;
							//memcpy(glBay_req.diag_string, glSNSRdiag_string, sizeof(glSNSRdiag_string));

							sensor_tnx_flag = TRUE;
							push_event_to_cache(UDP_EVTTYP_FREETIME_ON_ENTRY);//LOB
							sensor_tnx_flag = FALSE;
							
						}
						//ZERO_OUT_CONF = FALSE;
						VAC_ZERO_ELIGIBLE[Current_bayStatus] = TRUE;
					}
					else
					{
						//Debug_Output1(0,"glLast_reported_space_status =%d", glLast_reported_space_status);
						glLast_reported_space_status[Current_bayStatus]=1;
						//ZERO_OUT_CONF = FALSE;
						glSNSR_TMP &= 0x00FF;
						glSNSR_TMP |= (PAYMENT_ATTEMPT<<8);
						if(ZERO_OUT[Current_bayStatus]==false)
							Debug_Output1( 0, "no ZO on Entry:PA for Space[%d]", Current_bayStatus);
							//Debug_TextOut( 0, "no ZO:PA" );
						Space_Zero_Out[Current_bayStatus] = FALSE;
					}
				}
				else
				{
					/*if((ELIGIBLE_FOR_ZERO_OUT == SENSOR_DIAG_EXCEPTION) || (ELIGIBLE_FOR_ZERO_OUT == SAME_VEHICLE_AS_BEFORE))
						ZERO_OUT_CONF = FALSE;*/
					VAC_ZERO_ELIGIBLE[Current_bayStatus] = FALSE;
					if(glLast_reported_space_status[Current_bayStatus]!=1)
						Debug_Output1( 0, "ZO=FALSE for Space[%d]", Current_bayStatus);
						//Debug_TextOut( 0, "ZO=FALSE" );
					if(((ELIGIBLE_FOR_ZERO_OUT == SENSOR_DIAG_EXCEPTION) || (ELIGIBLE_FOR_ZERO_OUT == SAME_VEHICLE_AS_BEFORE)) && (Space_Zero_Out[Current_bayStatus] == TRUE))
					{
						Debug_Output1(0,"Revert_parking_clock= %d", revert_parking_clock);
						if((glLast_reported_space_status[Current_bayStatus] != 1) || (Clean_occupied == true))
						{
							parking_time_left_on_meter[Current_bayStatus] += revert_parking_clock;
							push_event_to_cache(UDP_EVTTYP_DIAG_EXCEPTION);
							current_space_timestamp	=	RTC_epoch_now();
							//glBay_req.occupancy		=	occupancygl;
							//glBay_req.PAMtxn_time 	= 	glBay_req.time_stamp = current_space_timestamp;
							glBay_req.expiry_time 	= 	current_space_timestamp + parking_time_left_on_meter[Current_bayStatus];
							glBay_req.purchased_duration = revert_parking_clock;
							glBay_req.txn_type 		= Tx_Grace_Period;
							//glBay_req.SNSR_TMP 		= 	glSNSR_TMP;
							//memcpy(glBay_req.diag_string, glSNSRdiag_string, sizeof(glSNSRdiag_string));

							glLast_reported_space_status[Current_bayStatus] = 1;	//set last reported status and reset the confirmation counters
							if(Normal_occupied == true)
							{
								Normal_occupied=false;
							}
						}
					}
					else
					{
						DIAG_EXCEPTION = FALSE;
					}
					Space_Zero_Out[Current_bayStatus] = FALSE;
				}
				if((ELIGIBLE_FOR_ZERO_OUT<4) || ((glSystem_cfg.Sensor_action_control & NON_ZO_TO_UDP_BIT) == NON_ZO_TO_UDP_BIT))
				{
					if((glLast_reported_space_status[Current_bayStatus] != 1) || (Clean_occupied == true))
					{
						glBay_req.occupancy		=	occupancygl;
						current_space_timestamp	=	RTC_epoch_now();
						//glBay_req.PAMtxn_time 	= 	glBay_req.time_stamp = current_space_timestamp;

						//The following 3 would have got filled based on action taken, do not refresh these here
						//glBay_req.expiry_time 	= 	current_space_timestamp + parking_time_left_on_meter;
						//glBay_req.purchased_duration = parking_time_left_on_meter;
						//glBay_req.txn_type 		= Tx_Unknown;  //Unknown because nothing changed in parking clock, to make it backward compatible
						glBay_req.SNSR_TMP 		= 	glSNSR_TMP;
						memcpy(glBay_req.diag_string, glSNSRdiag_string, sizeof(glSNSRdiag_string));

						glLast_reported_space_status[Current_bayStatus] = 1;	//set last reported status and reset the confirmation counters
						if(Normal_occupied==true)
						{
							Normal_occupied=false;
						}
						gprs_do_LOB_BayStatus();
					}
				}
			}
			else //vinay
			{
				//Debug_Output2( 2, "LastReport=%d, %d", glLast_reported_space_status, occupancygl);
			}
		}
		if((do_CC_OLT == FALSE) && (card_removed_flag == FALSE) && (smart_card_mode == FALSE))	//LNGSIT-753: allow screen refresh only when it is not in between amount selection or in between OLT for previous CC or SC swipe
				Idle_Screen();	//to update parking clock and @ display, refresh screen
//		Debug_Output1( 0, "Bay Status=%d", occupancygl );
		Debug_Output2( 0, "Bay Status for Space[%d]=%d",Current_bayStatus, occupancygl );
		//gprs_do_LOB_BayStatus(); //Moved up, this was creating diagdata 0x00 issue
	}
}

/**************************************************************************/
//! This function handles the Zero Out on Exit after 30 seconds of stable
//! Vacant and logs appropriate event for the action taken.
//! \param void
//! \return void
/**************************************************************************/
void Zero_Out_On_vacant()
{
	uint32_t current_space_timestamp = 0,loop_space_id = 0;
	for(loop_space_id = 0; loop_space_id < glSystem_cfg.MSM_max_spaces; loop_space_id++)
	{
		//Debug_TextOut(2,"Zero Out On Vacant-01");
		if(glLast_reported_space_status[loop_space_id] == 0)
		{
			//Debug_TextOut(2,"Zero Out On Vacant-02");
			if(last_vacant_timestamp[loop_space_id] != 0)
			{
				//Debug_TextOut(2,"Zero Out On Vacant-03");
				current_space_timestamp = RTC_epoch_now();
				if(current_space_timestamp > last_vacant_timestamp[loop_space_id])
				{
					//Debug_Output1(0,"loop_space_id = %d", loop_space_id);
					//Debug_Output2(0,"On Vacant ZERO_OUT[%d] = %d", loop_space_id,ZERO_OUT[loop_space_id]);
					//Debug_TextOut(2,"Zero Out On Vacant-04");
					//if(((uint32_t)(current_space_timestamp - last_vacant_timestamp[loop_space_id]) > 30) && ((uint32_t)(current_space_timestamp - last_vacant_timestamp[loop_space_id]) < 60)  && (last_vacant_timestamp[loop_space_id] > 30) && (VAC_ZERO_ELIGIBLE[Current_bayStatus] == TRUE))	// If space is vacant for more than 30 seconds and this vacant is after a large object, zero it out
					if(ZERO_OUT[loop_space_id] == TRUE)
					{
						if(((uint32_t)(current_space_timestamp - last_vacant_timestamp[loop_space_id]) > 30) && ((uint32_t)(current_space_timestamp - last_vacant_timestamp[loop_space_id]) < 60)  && (last_vacant_timestamp[loop_space_id] > 30) && (VAC_ZERO_ELIGIBLE[Current_bayStatus] == TRUE) && (ZERO_OUT[Current_bayStatus] == TRUE))	// If space is vacant for more than 30 seconds and this vacant is after a large object, zero it out
						{
							//Debug_TextOut(2,"Zero Out On Vacant-05");
							last_vacant_timestamp[loop_space_id] = 0;
							if((glSystem_cfg.Sensor_action_control & ZO_EXIT_BIT) == ZO_EXIT_BIT)//ZEROTIME_on_VEHICLE_DEPARTURE
							{
								sensor_tnx_flag = TRUE;
								Space_Zero_Out[loop_space_id] = TRUE;
								push_event_to_cache( UDP_EVTTYP_ZEROOUT_ON_VACANT ); //Raise the event first, then Zero Out
								revert_parking_clock = parking_time_left_on_meter[loop_space_id]; //Saving Parking Clock to revert in case of SVB
								//sensor zero out was clearing the parking clock in prepay schedule to avoid zero out in prepay and it was customer requirement //vinay
								if(current_rate_index != PREPAY_PARKING)
								{
									if((parking_time_left_on_meter[loop_space_id] > 0) || (negetive_parking_time_left_on_meter[loop_space_id] > 0))
									{
										push_event_to_cache(UDP_EVTTYP_PARKING_TIME_EXPIRED);	//LNGSIT-570
									}
									sensor_tnx_flag = FALSE;
									parking_time_left_on_meter[loop_space_id] = 0;
									negetive_parking_time_left_on_meter[loop_space_id] = 0; //LNGSIT-640
									grace_time_trigger[loop_space_id] = false;
									Parking_Clock_Refresh[loop_space_id] = PARKING_CLOCK_STOP_MODE;
									Update_Parking_Clock_Screen();
									Meter_Full_Flg[loop_space_id] = FALSE; // antifeed for progressive rate was creating problem so added this //vinay
									ANTI_FEED_in_effect[loop_space_id] = false;	//ready for next payment now since the last vehicle has left
									Meter_Full[loop_space_id] = FALSE;
									//Debug_TextOut(0,"Zero Out On Vacant");
									Debug_Output1( 0, "Zero Out On Vacant for Space[%d]",loop_space_id );
									//PAM bay status update for ZO on Vacant; ZO on Occ is already taken care with the BSU+PAM BS combined update
		/*							PAM_Bay_status_data[PAM_Baystatus_update_count].txn_time = (RTC_epoch_now());
									PAM_Bay_status_data[PAM_Baystatus_update_count].expiry_time = ((PAM_Bay_status_data[PAM_Baystatus_update_count].txn_time)+parking_time_left_on_meter[loop_space_id]);
									PAM_Bay_status_data[PAM_Baystatus_update_count].amount_cents = 0;
									PAM_Bay_status_data[PAM_Baystatus_update_count].purchased_duration = 0;
									PAM_Bay_status_data[PAM_Baystatus_update_count].topup = 0;
									PAM_Bay_status_data[PAM_Baystatus_update_count].txn_type = Tx_Sensor_Reset;
									PAM_Baystatus_update_count++;*/
								}
								else
								{
									Debug_Output1( 0, "No 'Zero Out On Vacant' for Space[%d] its Prepay Schedule",loop_space_id );
									//Debug_Output1(0,"minutes_left_in_current_schedule=%ld",minutes_left_in_current_schedule);
									parking_time_left_on_meter[loop_space_id] = minutes_left_in_current_schedule*60; //if customer has paid some money and if he moves out before time expire, then the remaining time in prepay will only reflect the extra time will removed //vinay
									//Debug_Output1(0,"parking_time_left_on_meter=%ld",parking_time_left_on_meter);
									Debug_TextOut( 0, "Removing Extra time if paid in prepay" );
									ANTI_FEED_in_effect[loop_space_id] = false;	//ready for next payment now since the last vehicle has left
									Meter_Full[loop_space_id] = FALSE;
									Meter_Full_Flg[loop_space_id] = FALSE; // antifeed for progressive rate was creating problem so added this //vinay
									Update_Parking_Clock_Screen();
								}
							}
						}
					}
					else if(ZERO_OUT[loop_space_id] == FALSE)
					{
						//Debug_TextOut( 0, "no ZO:PA" );
						//Debug_Output1( 0, "no ZO on Vacant:PA for Space[%d]",loop_space_id );
					}
				}
			}
		}
	}
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

