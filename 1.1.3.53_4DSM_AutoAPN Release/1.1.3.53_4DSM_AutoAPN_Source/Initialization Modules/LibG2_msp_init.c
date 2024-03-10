//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_msp_init.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_msp_init_api
//! @{
//
//*****************************************************************************

#include "LibG2_msp_init.h"

extern volatile uint32_t	parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];

extern GPRSSystem      		glSystem_cfg;
extern uint8_t				/*RData[500],*/ Connected_Card_reader;//not using in this program //vinay
extern uint32_t				Last_Comms_Success_RTCTimestamp;
extern uint8_t				Be4Bootup;
extern uint16_t	            wakecounts_to_check_update;
extern RTC_ParkingClk		RTC_ParkingTime;
extern uint32_t				max_time_in_current_rate;
extern uint8_t				LORA_PWR;
extern uint16_t 			Current_Space_Id;
extern volatile uint8_t		time_retain[MSM_MAX_PARKING_BAYS]; //vinay // to retain parking clock to become zero trenton
extern uint8_t				Start_Hour_Coin_Diag;
extern uint8_t				Start_Minute_Coin_Diag;
extern uint8_t				Start_Day_Coin_Diag;
extern uint8_t				Start_Month_Coin_Diag;
extern uint8_t 				glLast_reported_space_status[MSM_MAX_PARKING_BAYS];
extern uint8_t 				Current_bayStatus;
//extern uint8_t				ANTI_FEED_in_effect[MSM_MAX_PARKING_BAYS], Meter_Full_Flg[MSM_MAX_PARKING_BAYS], Meter_Full[MSM_MAX_PARKING_BAYS];//not used in this program //vinay

extern uint16_t            	glTotal_5_cents,
							glTotal_10_cents,
							glTotal_25_cents,
							glTotal_100_cents,
							coin_type_invalid,
							glTotal_0_cents,
							glTotal_Coin_Count,
							glTotal_Valid_Coin_Count,
							glTotal_SCT_TransCount,
							glTotal_SCT_TransAmount;
/**************************************************************************/
//! Initializes watchdog module in timer mode. When the timer expires(51.2ms
//! in VLOCLK), it generates an interrupt. This interrupt is used as the base
//! timer of MSP432 operation
//! \param void
//! \return void
/**************************************************************************/
void watchdog_init()
{
	WDT_A_initIntervalTimer(WDT_A_CLOCKSOURCE_VLOCLK, WDT_A_CLOCKDIVIDER_64);//512
	GPIO_setAsOutputPin(WD_DSP_PORT, WD_DSP_PIN); //External watchdog
	GPIO_setOutputHighOnPin(WD_DSP_PORT, WD_DSP_PIN); //External watchdog(WD_DSP_PORT, WD_DSP_PIN); //External watchdog
	NVIC->ISER[0] = 1 << ((WDT_A_IRQn) & 31);
	WDT_A_startTimer();
}

/**************************************************************************/
//! Initializes watchdog module in watchdog mode. When the timer expires(1.95ms
//! in BCLK), it resets the MSP432. This function is used in known conditions
//! where MSP432 needs to reset itself
//! \param void
//! \return void
/**************************************************************************/
void Soft_Reset_watchdog() //looks like not used anywhere. //vinay
{
	//Debug_TextOut(0,"Soft_Reset_watchdog");

	WDT_A_holdTimer();

	SysCtl_setWDTTimeoutResetType(SYSCTL_SOFT_RESET);
	WDT_A_initWatchdogTimer(WDT_A_CLOCKSOURCE_BCLK, WDT_A_CLOCKDIVIDER_64);

	WDT_A_startTimer();
}

/**************************************************************************/
//! Initializes all the basic Clock Sources needed for MSP432 - ACLK, MCLK,
//! HSMCLK, SMCLK
//! \param void
//! \return void
/**************************************************************************/
void CS_init_CLK(void)
{
	CS_init_ACLK(CS_CTL1_DIVA__1);
	CS_setDCOFrequency(CS_4MHZ);	//Setting the DCO frequency to 4MHz is sufficient, MCLK will automatically pick it from DCO
	CS_init_HSMCLK(CS_CTL1_DIVHS__1);
	CS_init_SMCLK(CS_CTL1_DIVS__4);
	//DelayUs(100);
	//CS_init_MCLK(CS_CTL1_DIVM_4);
}

/**************************************************************************/
//! Hardware and software initialisation
//! \param void
//! \return void
/**************************************************************************/
void initialize_all()
{
	//uint8_t i = 0;
	gpio_default_init();
	CS_init_CLK();
	////IO_EXP_SPI_init();
	////IO_EXP_init();
	init_event_cache(0xff);
	init_debug_GPIO();
	init_flash_SPI_module_GPIO();
	Flash_Read_Device_ID();
	init_DataKey_SPI_module_GPIO();
	SetMSP_RTC(TRUE);

	Start_Day_Coin_Diag = RTC_C->DATE & RTC_C_DATE_DAY_MASK;//>>VT<<
	Start_Month_Coin_Diag = (RTC_C->DATE & RTC_C_DATE_MON_MASK)>>RTC_C_DATE_MON_OFS;//>>VT<<
	Start_Hour_Coin_Diag = RTC_C->TIM1 & RTC_C_TIM1_HOUR_MASK;//>>VT<<
	Start_Minute_Coin_Diag = (RTC_C->TIM0 & RTC_C_TIM0_MIN_MASK)>>RTC_C_TIM0_MIN_OFS;//>>VT<<

	initialise_queue();
	activate_config_file( FALSE );

	glSystem_cfg.MSM_max_spaces = 2;//to avoid meter crash if they download SSM config //Vinay

	init_Enf_LED_GPIO();//Temp

	//glSystem_cfg.Disable_GPRS_Comms_Coin_Only	= TRUE; //to override config and make the meter to coin only meter //vinay

//	ANTI_FEED_in_effect[0] = false;
//	ANTI_FEED_in_effect[1] = false;
//	Meter_Full[0] = FALSE;
//	Meter_Full[1] = FALSE;
//	Meter_Full_Flg[0] = FALSE;
//	Meter_Full_Flg[1] = FALSE;
	//Debug_Output1(0,"glSystem_cfg.min_time_to_display=%ld",glSystem_cfg.min_time_to_display);
	//Debug_Output1(0,"glSystem_cfg.Display_Surcharges=%ld",glSystem_cfg.Display_Surcharges);
	//Debug_Output1(0,"glSystem_cfg.vehicle_sensor_enable=%ld",glSystem_cfg.vehicle_sensor_enable);
	//Debug_Output1(0,"glSystem_cfg.ANTI_FEED_ENABLE=%ld",glSystem_cfg.ANTI_FEED_ENABLE);
	//Debug_Output1(0,"glSystem_cfg.Sensor_action_control=%ld",glSystem_cfg.Sensor_action_control);
//	Debug_Output1(0,"glLast_reported_space_status[0] = %d",glLast_reported_space_status[0]);
//	Debug_Output1(0,"glLast_reported_space_status[1] = %d",glLast_reported_space_status[1]);
//	Debug_Output1(0,"Current_Space_Id = %d",Current_Space_Id);
//	Debug_Output1(0,"Current_bayStatus = %d",Current_bayStatus);
//	Debug_Output1(0,"glSystem_cfg.Enf_LEDs_selection=%d",glSystem_cfg.Enf_LEDs_selection);
	//Debug_Output1(0,"Boot up glSystem_cfg.coin_parameters[3].value_units=%ld",glSystem_cfg.coin_parameters[3].value_units);
	//for(k=0; k<glSystem_cfg.no_of_valid_rates;k++)
	//	Debug_Output2(0,"Boot up glSystem_cfg.all_rates[%d].amount_per_hour = %d\n",k,glSystem_cfg.all_rates[k].amount_per_hour);

	//if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == FALSE)//3oct18// Need to initiaze GPIO properly to avoid power leak
	{
		init_Telit_GPIO();
		telit_power_off();
	}
	//disable_telit_RI_interrupt();//already done inside telit_power_off

	//Parking Clock append in flash every minute

	if (!((RTC_ParkingTime.Max_parking_time[0] == 0)&&(RTC_ParkingTime.Max_parking_time[1] == 0)))
		{
			get_earned_seconds(100, false, true); //dummy call to refresh the max time allowed variable
			parking_time_left_on_meter[0] = ((RTC_ParkingTime.Max_parking_time[0] * 3600) + (RTC_ParkingTime.Max_parking_time[1] * 60) + 60);
			//Debug_Output1( 0, "parking_time_left_on_meter[0]: %ld\r\n",parking_time_left_on_meter[0]);
			if(parking_time_left_on_meter[0] > max_time_in_current_rate)
			{
				parking_time_left_on_meter[0] = max_time_in_current_rate;
				//Debug_Output1( 0, "1parking_time_left_on_meter[0]: %ld\r\n",parking_time_left_on_meter[0]);
			}
			if(parking_time_left_on_meter[0] == 86400)
			{
				parking_time_left_on_meter[0] = 0;
				//Previously they were filling the parking clock with max time when it could not find previous parking clock LNGSIT-1967 and they wanted 0//vinay
				//Debug_Output1( 0, "2parking_time_left_on_meter[0]: %ld\r\n",parking_time_left_on_meter[0]);
			}
			time_retain[0] = 1;
		}
	if (!((RTC_ParkingTime.Max_parking_time[2] == 0)&&(RTC_ParkingTime.Max_parking_time[3] == 0)))  //once we are given time we clearing for next time.
		{
			get_earned_seconds(100, false, true); //dummy call to refresh the max time allowed variable
			parking_time_left_on_meter[1] = ((RTC_ParkingTime.Max_parking_time[2] * 3600) + (RTC_ParkingTime.Max_parking_time[3] * 60) + 60);
			//Debug_Output1( 0, "parking_time_left_on_meter[1]: %ld\r\n",parking_time_left_on_meter[1]);
			if(parking_time_left_on_meter[1] > max_time_in_current_rate)
			{
				parking_time_left_on_meter[1] = max_time_in_current_rate;
				//Debug_Output1( 0, "1parking_time_left_on_meter[1]: %ld\r\n",parking_time_left_on_meter[1]);
			}
			if(parking_time_left_on_meter[1] == 86400)
			{
				parking_time_left_on_meter[1] = 0;
				//Previously they were filling the parking clock with max time when it could not find previous parking clock LNGSIT-1967 and they wanted 0//vinay
				//Debug_Output1( 0, "2parking_time_left_on_meter[1]: %ld\r\n",parking_time_left_on_meter[1]);
			}
			time_retain[1] = 1;
		}

	init_display_SPI_module_GPIO();
	Idle_Screen();

	init_keypad_GPIO();
	//watchdog_init();

	init_mag_card_reader_GPIO();
	init_IDTech_card_reader_GPIO();

	if(Start())
	{
		LoadDefaultValue();
		IDTech_POWER_OFF();
		Connected_Card_reader = ID_TECH_READER;
		init_card_interrupt(Connected_Card_reader);
		Debug_TextOut(0,"ID TECH READER Initialised");
	}
	else
	{
		init_smartcard_GPIO();
		IDTech_POWER_OFF();    //required
		Connected_Card_reader = GEM_CLUB_READER;
		init_card_interrupt(Connected_Card_reader);
		Debug_TextOut(0,"GEM CLUB READER Initialised");
	}
	//IDTech_POWER_ON();
	//init_Enf_LED_GPIO();
	init_cointrack_GPIO();
	watchdog_init();
	Display_Meter_Info();

	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == FALSE)//3oct18
	{
		reinit_telit_Modem();
		telit_init();
		telit_sock_open(0);
	}

	//TODO: find the reason for the below issue, why regulator control doesn't work
	Init_LORA();
	SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );
	LORA_PWR = FALSE;
	LORA_power_off();

	//Debug_TextOut(0,"Device Booted\r\n");
	Be4Bootup=1;
	Debug_Output1(0, "Wakeupcounts Settled on this Meter: %d", wakecounts_to_check_update);
	Last_Comms_Success_RTCTimestamp = RTC_epoch_now(); //For testing
	__enable_interrupt();
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

