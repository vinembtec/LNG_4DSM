//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_coin.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_coin_api
//! @{
//
//*****************************************************************************

#include "LibG2_coin.h"

extern volatile uint32_t 			parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];
extern volatile uint32_t			negetive_parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];
extern volatile uint8_t 			current_rate_index;
extern GPRSSystem       			glSystem_cfg;
extern uint16_t 					glTotal_Coin_Count, glTotal_0_cents, glTotal_Valid_Coin_Count;
extern uint8_t 						Parking_Clock_Refresh[MSM_MAX_PARKING_BAYS];
extern uint16_t 					Current_Space_Id;
//extern uint8_t 						IN_LPM_TEST; //not used in this code //vinay
extern uint32_t						last_payment_try_time[MSM_MAX_PARKING_BAYS];
extern uint32_t  					Refund_PreviousSN;
extern uint32_t  					Refund_PresentSN;
extern uint32_t  					Refund_timestamp;
extern uint32_t  					Refund_earned_mins,santa_cruz_previous_paid_time;
extern uint8_t	 					RefundOK, Meter_Reset;
extern GPRS_AlarmsRequest   		glAlarm_req;
extern uint8_t                      grace_time_trigger[MSM_MAX_PARKING_BAYS];
extern uint8_t                      /*grace_timeout, */in_prepay_parking[MSM_MAX_PARKING_BAYS];//not used in this program //vinay
extern uint8_t            	        /*one_time_loop_flag, */meter_out_of_service;//not used in this program //vinay
extern uint32_t 					Tasks_Priority_Register, KeyPress_TimeOut_RTC;
extern char							special_key_count;
extern uint8_t              		glTotal_5_cents, glTotal_10_cents, glTotal_25_cents, glTotal_100_cents;
extern uint8_t						HARDWARE_REVISION, glLast_reported_space_status[MSM_MAX_PARKING_BAYS], ZERO_OUT_ENABLE_AT_EXPIRY[MSM_MAX_PARKING_BAYS], ZERO_OUT[MSM_MAX_PARKING_BAYS];
extern uint32_t                     max_time_in_current_rate;
extern uint8_t                      Meter_Full_Flg[MSM_MAX_PARKING_BAYS];
//extern RTC_C_Calendar 				rtc_c_calender;//not suing in this program //vinay
extern uint8_t						payment_on_hold;
//extern uint8_t						key_press;//not used in this program //vinay
uint32_t 							payment_on_hold_timeout = 0;
//uint8_t								enable_fiji_10C_coin;//not used in this program //vinay

uint8_t 							dcd_error = 0;	// dcd error type register
										// bit 0 - nov error
										// bit 1 - coil sensor error
										// bit 2 - dead battery error
										// bit 3 - serial channel lockup -wakeup coil jam
										// bit 4 - recognition coil jam
										// bit 5 - data key on reset error
	  	  	  	  						// bit 6 - low battery
										// bit 7 - audit error on reset
uint16_t 							loSensorSamples[SENSOR_NUMBER_OF_READS*MAX_COIN_SAMPLING_TRIALS] = {0};
COINDATA        					glCoindata[MAX_COINS_TO_UDP] = { 0 };
uint16_t             				glCoin_Index=0;
uint8_t								gAutoCalibration = false, gSelfCalibration = false; //true;
uint16_t 							cal_sens = 0; // Sensor calibration lsb-msb
uint16_t 							max_sens = 0; // Sensor maximum point lsb-msb
                     	 	 	 		// local maximum period contributes to
                     	 	 	 		// MINIMUM deviation (coin in center of coils)
uint16_t 							min_sens = 0; 	// Sensor minimum point lsb-msb
                     					// local minimum period contributes to
                     					// MAXIMUM deviation (coin aligned with coil)
uint8_t 							sense[3]; 	 	// Sensor input lsb-msb (msb is 0 after process)
uint16_t 							sec_max = 0; 	// second maximum value
uint8_t 							ratio_1 = 0;
uint8_t 							ratio_2 = 0;

//CCF operational parameters
uint8_t 							noise_base;  	//min/max qualification parameter/16 in ms 4 bits
					 					//base deviation parameter/4 in ls 4 bits
uint8_t 							temp_co; 		// temperature coeffecient
uint16_t 							base_freq; 	// sensor base frequency at 25 C
uint8_t 							wake_rate; 		// wakeup sensor sample rate
uint8_t 							chirp_sd; 		// ms 4 bits number of samples for chirper settling
										// ls 4 bits, chirper switching delay
uint8_t 							trim;       	//TOD trim
uint8_t 							coin_validation_number; 	// 16 - 1
uint8_t 							coin_count; 	//  0 - 9
uint16_t 							MaxValue_point=0, MinValue_point = 0, MaxValue_point1 = 0;
uint16_t  							glIdx = 0;
uint32_t 							glTemp0;
uint16_t 							glTemp1, glTemp2, glTemp3, glTemp4;

uint8_t								Be4Bootup = 0;
uint16_t 							wakecounts_to_check_update  = WAKE_COUNT_START_VALUE;//start from a higher value
uint16_t							Last_WakeCount_downwards = WAKE_COUNT_START_VALUE;
uint16_t 							WakeCountAdjust = 0, fluctuating_counts = 0;
uint16_t							wakecounts = 0;
uint32_t							Last_Cash_Payment_RTC = 0;
uint32_t							Calibration_TimeOut_RTC = 0,glTotal_COIN_TransAmount = 0;//not used in this program //vinay
CoinSpecsType_New 					Coin_Calib_Params[MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT];
Self_Calibration_Struct				Self_Calibration[MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT];
uint8_t								OPTICAL_COIN_JAM_RAISED = FALSE, WAKEUP_COIL_JAM_RAISED = FALSE;

uint8_t								Wakecount_adjust_interval=0;
uint8_t								CoinIndex=0;
uint32_t							wakecountsTest = 0;   //5 minutes value.

Coin_min_max_diag					Failed_Coins_diag[MAX_FAILED_COINS_DIAG];
uint8_t								failed_coin_counter = 0;


uint16_t							coin_type0=0;
uint16_t							coin_type1=0;
uint16_t							coin_type2=0;
uint16_t							coin_type3=0;
uint16_t							coin_type_invalid=0, temp_coin_type_invalid=0;

uint16_t							temp_coin_type0=0;
uint16_t							temp_coin_type1=0;
uint16_t							temp_coin_type2=0;
uint16_t							temp_coin_type3=0;
uint16_t							temp_coin_type[20];

uint32_t                            total_coin_values=0, temp_total_coin_values = 0;// in cents
uint32_t 							Last_Audit_Time = 0;//RTC_epoch_now();

uint8_t								Optical_LED_touched = FALSE;

uint32_t							sum_max = 0, sum_min = 0, sum_rat1 = 0, sum_rat2 = 0,
									averaged_max = 0, averaged_min = 0, averaged_rat1 = 0, averaged_rat2 = 0;

//static uint32_t 					gCoinSensorStartTime = 0;
static uint16_t 					loMin_Sens[NUMBER_OF_CAL_DROPS] = { 0 };
static uint16_t 					loMax_Sens[NUMBER_OF_CAL_DROPS] = { 0 };
static uint16_t 					loRatio1[NUMBER_OF_CAL_DROPS] = { 0 };
static uint16_t 					loRatio2[NUMBER_OF_CAL_DROPS] = { 0 };

uint32_t  							earned_time_in_seconds = 0; //declared outside to use it in lcd.c //vinay for trenton
uint8_t								coin_valid = 0; //to screen refresh in lcd.c for min amount to display //vinay for trenton
uint16_t 				 			min_time_insec = 0; 	//vinay //make it check it from config Trenton
volatile uint8_t					time_retain[MSM_MAX_PARKING_BAYS] = { 0 }; //vinay // to retain parking clock to become zero trenton
uint32_t 							parking_time_left_on_meter_prev[MSM_MAX_PARKING_BAYS] = { 0 }; //to round off //vinay
extern volatile uint16_t 			minutes_left_in_current_schedule,minutes_left_in_next_schedule;
uint16_t							coin_type[20];
uint8_t 							coin_invalid = 0;
//uint32_t 							coin_earned_seconds = 0;//not suing in this program //vinay
uint32_t 							coindrop_TimeOut_RTC = 0;
uint8_t								jam = 0;
extern uint8_t						key_RL;
uint16_t 				  			wakeup_temp = 0;
volatile uint8_t 					wakeup_skip = 0;
volatile uint8_t 					second_check = 0;
extern uint32_t 					KeyPress_TimeOut_RTC;
//uint16_t 							temp_next_coin_value_to_show = 0;//not used in this program //vinay
//uint8_t 							temp_next_coin_calibration_index = 0;//not used in this program //vinay
//uint16_t 							old_min,new_min = 0;
uint8_t 							M1_M2_Counter = 0;
uint16_t 							/*cal_sens_tt=0,*/cal_sens_t=0;//not used in this program //vinay
//uint16_t 							t_cal_sens,t_max_sens,t_min_sens,t_sec_max,temp_cal_sens=0;//not used in this program //vinay
uint8_t 							coin_cal_sens_taken_from_last=0;
uint8_t 							coin_second_check_for_Min_value=0;
uint16_t 							temp1_max_sens, temp1_min_sens;
uint8_t								temp1_ratio_1, temp1_ratio_2;
uint16_t 							temp2_max_sens, temp2_min_sens;
uint8_t								temp2_ratio_1, temp2_ratio_2;
extern uint8_t 						Current_bayStatus;
CoinSpecsType_New 					loCoinParam; //made global to use it in rates.c for Thurmont //vinay
uint8_t coin_insert_ok = FALSE;
#if 0
void coin_wake_delay()
{
	DelayUs(500);  //wake_delay 3nops+ 12clk for call and rts = 15clk
}

void test_sensor_ready()
{
	while(1)
	{
		clsns_Low();	//1
		DelayUs(20);
		clsns_High();

		while(get_Snsrdy_val() == 1);
		clsns_Low();		//1
		DelayUs(20);
		clsns_High();
		while(get_Snsrdy_val() == 0);
	}

}

void coin_wakeup_done()
{
	if(wake_block != 0)
	{
		coin_wakeup_loop(); //no_wake
		return;
	}
	coin_dropped(); //CoinValidation();  //continue to validate the dropped coin;
}

void coin_wakeup_loop()
{
	uint16_t wakecounts = 0;
	uint8_t wakecounts_l;//, wakecounts_h;
	uint16_t i = 0;
	int16_t loTemp = 0;			//we will use temp as acc in MB

	for(i=0; i<=7; i++)
	{
		coin_wake_delay();  //Do we really need this delay?.
		reset_sensor_counter();
		t_Low();

		if(wake_block != 0)
		{
			//goto coin_wakeup_10twake_end;
			t_High();
			continue; //goto coin_wakeup_lt1wakeup;
		}

		wakecounts = (uint16_t)read_sensor_counter();

		//Only LSB is used in our wake up calc
		wakecounts_l = (uint8_t)(wakecounts & 0x00FF);
		//wakecounts_h = (uint8_t)((wakecounts & 0xFF00) >> 8);

		loTemp = (int16_t)(wakecounts_l - wake_1);
		if(loTemp <= 0)
		{
			//goto coin_wakeup_10twake_end;
			t_High();
			continue; //goto coin_wakeup_lt1wakeup;
		}

		loTemp--;
		if(loTemp > 0)
		{
			t_High();
			coin_dropped();  //continue to validate the dropped coin
			return;
		}
//coin_wakeup_10twake_end:
		t_High();
		//continue; //goto coin_wakeup_lt1wakeup;

//coin_wakeup_ltwake0_up:
		t_High();
		coin_dropped();  //continue to validate the dropped coin
		return;
	}
	t_High();
}

/**************************************************************************/
/*  Name        : sensor_check_auto_cal()                                 */
/*  Parameters  : void                                                    */
/*  Returns     : void                                                    */
/*  Function    : checks the auto cal condition                           */
/*------------------------------------------------------------------------*/
bool sensor_check_auto_cal()
{
	if(wake_1 == (WAKE_RESET_VAL & 0xFF))
	{
		return true;
	}
	return false;
}


uint8_t GetOpParameter(uint8_t opParam)
{
	switch (opParam)
	{
	case CCF_OP_PARAMS_MINMAX_QUALIFIER:
		return(glSystem_cfg.operational_parameters.min_max_qualifier);

	case CCF_OP_PARAMS_BASE_DEVIATION:
		return(glSystem_cfg.operational_parameters.base_deviation);

	case CCF_OP_PARAMS_BASE_FREQ_LSB:
		return(glSystem_cfg.operational_parameters.base_freq_lsb);

	case CCF_OP_PARAMS_BASE_FREQ_MSB:
		return(glSystem_cfg.operational_parameters.base_freq_msb);

	case CCF_OP_PARAMS_TEMP_CO:
		return(glSystem_cfg.operational_parameters.tempco);

	case CCF_OP_PARAMS_CHIRPER_SWITCH_DELAY:
		return(glSystem_cfg.operational_parameters.chirper_switch_delay);

	case CCF_OP_PARAMS_CHIRPER_SETTLING:
		return(glSystem_cfg.operational_parameters.chirper_settling);

	case CCF_OP_PARAMS_SAMPLES_BEFORE_2ND_BASE:
		return(glSystem_cfg.operational_parameters.samples_before_second_base);

	case CCF_OP_PARAMS_STARTUP_TIMEOUT:
		return(glSystem_cfg.operational_parameters.start_up_timeout);

	case CCF_OP_PARAMS_SENSOR_TIMEOUT:
		return(glSystem_cfg.operational_parameters.sensor_timeout);

	case CCF_OP_PARAMS_WAKEUP_RATE:
		return(glSystem_cfg.operational_parameters.wakeup_rate);

	case CCF_OP_PARAMS_TOD_TRIM:
		return(glSystem_cfg.operational_parameters.TOD_trim);

	case CCF_OP_PARAMS_VALUE_SCALE:
		return(DEFAULT_OP_PARAMS_VALUE_SCALE);

		//case CCF_OP_PARAMS_VALUE_UINT:
		//	return(DEFAULT_OP_PARAMS_VALUE_UNIT);
	}

	return 0;
}

void coin_init_ccf_Parameters()
{
	noise_base = GetOpParameter(CCF_OP_PARAMS_BASE_DEVIATION) +			//min/max qualification parameter/16 in ms 4 bits
			(GetOpParameter(CCF_OP_PARAMS_MINMAX_QUALIFIER) * 16);	//base deviation parameter/4 in ls 4 bits

	temp_co = GetOpParameter(CCF_OP_PARAMS_TEMP_CO); 			// temperature coeffecient

	base_freq = GetOpParameter(CCF_OP_PARAMS_BASE_FREQ_LSB) +
			(GetOpParameter(CCF_OP_PARAMS_BASE_FREQ_MSB) * 256); 	// sensor base frequency at 25 C

	wake_rate = GetOpParameter(CCF_OP_PARAMS_WAKEUP_RATE); 			// wakeup sensor sample rate

	chirp_sd = GetOpParameter(CCF_OP_PARAMS_CHIRPER_SWITCH_DELAY) +		// ls 4 bits, chirper switching delay
			(GetOpParameter(CCF_OP_PARAMS_CHIRPER_SETTLING) * 16); 	// ms 4 bits number of samples for chirper settling


	//sec_bases = GetOpParameter(CCF_OP_PARAMS_SAMPLES_BEFORE_2ND_BASE); 	// number of samples to take after second max
	// before taking second base

	trim = GetOpParameter(CCF_OP_PARAMS_TOD_TRIM);  		   			// TOD trim
}

/*****************************************************************************
* AutoCalibration: Calibrate routine replace nauto_cal in MB
*
* Parameters: none
*
* Return: none
*
* Created:
* Reviewed:
* Last mofdified date:
* Modified by:
*****************************************************************************/
void AutoCalibration()
{
	//Debug_TextOut( 0, "*** AutoCalibration ***");
	static uint8_t loIdx = 0;
	static uint32_t loMin_Sens = 0;
	static uint32_t loMax_Sens = 0;
	static uint32_t loRatio1 = 0;
	static uint32_t loRatio2 = 0;
	if(loIdx < NUMBER_OF_CAL_DROPS)
	{
		Debug_Output6( 0, "Coin Drop# *** %d ***", (loIdx+1),0,0,0,0,0);
		loMin_Sens = loMin_Sens + min_sens;
		loMax_Sens = loMax_Sens + max_sens;
		loRatio1 = loRatio1 + ratio_1;
		loRatio2 = loRatio2 + ratio_2;
		loIdx++;
	}
	if(loIdx >= NUMBER_OF_CAL_DROPS){
		loMin_Sens = loMin_Sens / NUMBER_OF_CAL_DROPS;
		loMax_Sens = loMax_Sens / NUMBER_OF_CAL_DROPS;
		loRatio1 = loRatio1 / NUMBER_OF_CAL_DROPS;
		loRatio2 = loRatio2 / NUMBER_OF_CAL_DROPS;
		//We have done, print the values now
		//Debug_Output6( 0, "min_sens[%04X] max_sens[%04X] ratio1[%02X] ratio2[%02X]", loMin_Sens, loMax_Sens, loRatio1, loRatio2,0,0);
		Debug_Output6( 0, "max:%d, min:%d, rat1:%d, rat2:%d", loMax_Sens, loMin_Sens, loRatio1, loRatio2, 0, 0);
		loIdx = 0;
		loMin_Sens = 0;
		loMax_Sens = 0;
		loRatio1 = 0;
		loRatio2 = 0;
	}
	return;
}


//Compare source and dest,  if (source > dest) then swap them  and clear carry (return false).
//If (dest > source) and (dest - source < noise), clear carry (return false).
//Else, set carry (return true) for completion of min or max search.

bool SensorCompare(uint16_t *pSource, uint16_t * pDest, bool *pSwapFlag)
{
	uint16_t loTempVal;
	*pSwapFlag = false;

	if(NULL == pSource || NULL == pDest)
	{
		return false;
	}
	//If source > dest then swap them and clear carry
	if(*pSource > *pDest)
	{
		loTempVal = *pDest;
		*pDest = *pSource;
		*pSource = loTempVal;
		*pSwapFlag = true;
		return false;
	}else if(*pSource < *pDest){
		loTempVal = (uint16_t)((noise_base & 0xF0) + *pSource);
		if(*pDest > loTempVal)
		{
			return true;
		}else{
			return false;
		}
	}
	//source = dest
	return false;
}

/**************************************************************************/
//! Used as a timeout while capturing Coin sample
//! \param void
//! \return void
/**************************************************************************/
void StartCoinSensorTimeout()
{
	gCoinSensorStartTime = RTC_epoch_now();
}

bool IsCoinSensorTimeout()
{
	uint32_t loCurrentTime = RTC_epoch_now();
	if (loCurrentTime > gCoinSensorStartTime + DEFAULT_COIN_SENSOR_TIMEOUT)
	{
		return 1;
	}
	return 0;
}
#endif

/**************************************************************************/
//! Reads the SNSRDY pin status and returns HIGH or LOW
//! \param void
//! \return HIGH or LOW
/**************************************************************************/
uint8_t get_Snsrdy_val()
{
	return GPIO_getInputPinValue(SNSRDY_PORT, SNSRDY_PIN);//BITVAL(SNSRDY_PORT,SNSRDY_PIN);
}

/**************************************************************************/
//! Clears the Coin sensor line (CLSNS) by giving a Low to High pulse
//! \param void
//! \return void
/**************************************************************************/
void coin_sensor_clear()
{
	clsns_Low();	//1
	DelayUs(1);
	clsns_High();
}

/**************************************************************************/
//! Turns On the the Coin sensor along with all other needed pins to start
//! coin reading
//! \param void
//! \return void
/**************************************************************************/
void coin_sensor_on()
{
	snson_High();
	DelayMs(SENSOR_SETTLE_TIMEOUT);
	clsns_Low();	//1
	pclk_Off();
	DelayUs(20);
	clsns_High();  //Enable sensor	//test
}

/**************************************************************************/
//! Turns Off the the Coin sensor along with all other needed pins
//! \param void
//! \return void
/**************************************************************************/
void coin_sensor_off()
{
	cancel_sensor_counter();
	clsns_Low(); //must be sure clsns is high before turning sensor off	//1
	pclk_Off();
	snson_Low();  //sensor off
}

/**************************************************************************/
//! Tests the coin track by powering on coin sensor and checking the SNSRDY signal
//! \param void
//! \return bool
//! - \b Test Pass
//! - \b Test Fail
/**************************************************************************/
bool coin_sensor_test()
{
	uint16_t loTimeout = 133; //magic number from MB ROM
	bool loTestRes = false;
	coin_sensor_clear();
	while(loTimeout > 0)
	{
		if (get_Snsrdy_val() == 0)
		{
			loTestRes = true;
			break;
		}
		DelayUs(1);
		loTimeout--;
	}
	//coin_sensor_off(); //make sure sensor is off
	return loTestRes;
}

/**************************************************************************/
//! Initializes the coin track and all modules needed to read coin profile,
//! includes the coin track testing also
//! \param void
//! \return void
/**************************************************************************/
void coin_sensor_init()
{
	coin_sensor_on();
	//init_sensor_counter_12mhz(); //turn on 12 mhz oscillator
	init_coin_sample_counter_12mhz();
	//start_sensor_counter();
	if(coin_sensor_test())
	{
		//Debug_TextOut( 0, "init_cointrack: Sensor Test *Passed*");
		//Clear coil error flag
		dcd_error &= ~(coil_err);
	}else{
		//Debug_TextOut( 0, "init_cointrack: Sensor Test  #Failed#");
		dcd_error |= coil_err;
	}
	reset_sensor_counter();
	clsns_High();  //enable sensor
	DelayUs(20);  //wait 20us for sensor clear action
}

/**************************************************************************/
//! Caller function of coin detection routine. Executes after a coin wake up
//! \param void
//! \return void
/**************************************************************************/
void coin_dropped()
{
	snson_High();
	DelayMs(SENSOR_SETTLE_TIMEOUT);
	CoinValidation();  //continue to validate the dropped coin
}

/**************************************************************************/
//! Base function for Coin detection. Gets called in every MSP432 wakeup to
//! check Coin Presence near wake up Coil
//! \param void
//! \return void
/**************************************************************************/
void coin_wakeup()
{
	uint16_t wakecounts_to_check = 0;//, k = 0, wakecounts2 = 0, wakecount_tolerance_check = 0;
	//uint16_t percentage_factor = 0;
	//to save power, no need to keep it as a peripheral module pin always
	GPIO_setAsPeripheralModuleFunctionInputPin(CNTR_PORT, CNTR_PIN, GPIO_PRIMARY_MODULE_FUNCTION);	//CNTR

	wakecounts_to_check = (wakecounts_to_check_update - glSystem_cfg.operational_parameters.wakeup_rate);
	init_sensor_counter_12mhz();

	TA0R=0X0000;//Clearing timer for accurate wakeup count >>VT<<
	TA0CCR3=0X0000;
	t_Low();
	wakecounts = (uint16_t)read_sensor_counter();
	t_High();

	if(((wakecounts_to_check_update-wakecounts)<50)&&((wakecounts_to_check_update-wakecounts)>10)||
			(wakecounts>(wakecounts_to_check_update+20)))//Condition for second check after 15ms >>VT<<
		{
			wakeup_temp = wakecounts;//First wake up count >>VT<<
			second_check = 1;//Coin wake up after 15ms >>VT<<
		}

	if((wakecounts < wakecounts_to_check) && (wakecounts > MINIMUM_WAKECOUNT_VALUE))	//filter junk readings
	{
		DelayMs(1);
		if(HARDWARE_REVISION == LNG_REV4)
			CS_setDCOFrequency(CS_12MHZ);
		if(Be4Bootup == 1)	//Do this only after boot up wakecount stabilization
		{
			coin_dropped(); //it is valid coin
			glTotal_Coin_Count++;	//for diagnostics and audit
			second_check = 1;//Coin wake up after 15ms >>VT<<
		}
		if(HARDWARE_REVISION == LNG_REV4)
			CS_setDCOFrequency(CS_4MHZ);
		if(fluctuating_counts >= glSystem_cfg.operational_parameters.wakeup_jam_confirmation_count)
			{ //Added one more time to check if given condition achieved >>VT<<
				DelayUs(500);
				init_sensor_counter_12mhz();
				TA0R=0X0000;
				TA0CCR3=0X0000;
				t_Low();
				wakecounts = (uint16_t)read_sensor_counter();
				t_High();
			}
		Coin_WakeCount_adjustment(REGULAR_DOWNWARDS_ADJUSTMENT);
		wakeup_skip = 1; //To skip wake-up threshold calculation >>VT<<
		//Debug_Output2( 2, "COIN WAKEUP COUNT = %d, %d", wakecounts, wakecounts_to_check);
		if(wakeup_temp > 0)//If second check occurs print both I and II wake-up counts >>VT<<
			Debug_Output2( 0, "I=%d,II=%d",wakeup_temp,wakecounts);
		wakeup_temp = 0;
	}

	cancel_sensor_counter();
	//to save power, no need to keep it as a peripheral module pin always
	GPIO_setAsInputPinWithPullUpResistor(CNTR_PORT, CNTR_PIN);	//CNTR
}

/**************************************************************************/
//! Reads the LED RxD pin status and returns HIGH or LOW
//! \param void
//! \return HIGH or LOW
/**************************************************************************/
uint8_t get_Led_Rxd_val()
{
	return GPIO_getInputPinValue(LED_RXD_PORT,LED_RXD_PIN);
}

/**************************************************************************/
//! Checks the Optical LEDs status and returns HIGH or LOW
//! \param iLed LED number to check
//! \return HIGH or LOW
/**************************************************************************/
uint8_t get_Coin_Position(uint8_t iLed)
{
	uint8_t loRes = 0;//, i;//get_Led_Rxd_val();
	//uint16_t loTime = 0;

	if(iLed == 1)
	{
		nLed_Drive1_High();
	}
	else if(iLed == 2)
	{
		nLed_Drive2_High();
	}
	else if(iLed == 3)
	{
		if(HARDWARE_REVISION == LNG_REV4)
		{
			nLed_Drive3_R4_High();
		}
		else
		{
			nLed_Drive3_High();
		}
	}
	DelayUs(15);
	loRes = get_Led_Rxd_val();

	//Make sure all leds are off
	nLed_Drive1_Low();
	nLed_Drive2_Low();
	if(HARDWARE_REVISION == LNG_REV4)
	{
		nLed_Drive3_R4_Low();
	}
	else
	{
		nLed_Drive3_Low();
	}

	return loRes;
}

/**************************************************************************/
//! Checks and generates Optical Jam Raise/ Clear Alarm first. Then it
//! checks and generates Wakeup Coil Jam Raise/ Clear Alarm. For Coin only
//! meters, the same coin jam alarm detection actually initiates communication
//! with gateways if configured in config file.
//! \param uint8_t wakeup_coil_alarm_check - this tells whether to check wakeup
//! coil jam also along with optical check
//! \return void
/**************************************************************************/
void Check_Coin_Jam_Alarm(uint8_t wakeup_coil_alarm_check)
{
	uint8_t coin_Jam_check1 = 0, coin_Jam_check2 = 0, coin_Jam_check3 = 0;

	//Optical Jam has higher priority than wakeup coil jam
	//Optical Jam detection start
	//Debug_Output6(0,"Coin Jam Check Before:%d,%d,%d", coin_Jam_check1, coin_Jam_check2, coin_Jam_check3, 0, 0, 0);
	coin_Jam_check1 = get_Coin_Position(1);
	coin_Jam_check2 = get_Coin_Position(2);
	coin_Jam_check3 = get_Coin_Position(3);

	//If jam detected, Reconfirm by checking all optical LEDs once again
	if((coin_Jam_check1 + coin_Jam_check2 + coin_Jam_check3) >= 2)
	{
		coin_Jam_check1 = get_Coin_Position(1);
		coin_Jam_check2 = get_Coin_Position(2);
		coin_Jam_check3 = get_Coin_Position(3);
	}
	if((coin_Jam_check1 + coin_Jam_check2 + coin_Jam_check3) >= 2)
	{
		if(glSystem_cfg.optical_jam_detection_enable == TRUE)
		{
			if(OPTICAL_COIN_JAM_RAISED == FALSE)
			{
				OPTICAL_COIN_JAM_RAISED = TRUE;
				glAlarm_req.alarm_type  =   UDP_EVTTYP_ALARM;
				glAlarm_req.alarm_size  =	3;
				glAlarm_req.alarm_ts    = 	RTC_epoch_now();
				glAlarm_req.alarm_state =	MB_ALARM_STATE_RAISED;
				glAlarm_req.alarm_id    =  	UDP_ALMTYP_OpticalJam_Error;
				//Debug_Output6(2,"Optical Jam Check: %d, %d, %d", coin_Jam_check1, coin_Jam_check2, coin_Jam_check3, 0, 0, 0);
				Debug_TextOut(0, "Optical Jam Detected");
				gprs_do_Alarms(&glAlarm_req);
				Idle_Screen();
			}
		}
		else
		{
			Debug_TextOut(2, "Optical Jam Disabled");
		}
	}
	else
	{
		if(OPTICAL_COIN_JAM_RAISED == TRUE)
		{
			OPTICAL_COIN_JAM_RAISED = FALSE;
			glAlarm_req.alarm_type  =   UDP_EVTTYP_ALARM;
			glAlarm_req.alarm_size  =	3;
			glAlarm_req.alarm_ts    = 	RTC_epoch_now();
			glAlarm_req.alarm_state =	MB_ALARM_STATE_CLEARED;
			glAlarm_req.alarm_id    =  	UDP_ALMTYP_OpticalJam_Error;
			//Debug_Output6(2,"Optical Jam Check: %d, %d, %d", coin_Jam_check1, coin_Jam_check2, coin_Jam_check3, 0, 0, 0);
			Debug_TextOut(0, "Optical Jam Cleared");
			gprs_do_Alarms(&glAlarm_req);
			Idle_Screen();
			Coin_WakeCount_adjustment(JAM_CLEAR_UPWARDS_ADJUSTMENT);
		}
		//Wakeup Coil Jam detection Start
		if((Be4Bootup == 1) && (Meter_Reset == FALSE))	//Do this only after boot up wakecount stabilization and do not raise if it is within 1 minute of upwards adjustment
		{
			if(wakeup_coil_alarm_check == RAISE_WAKEUP_COIL_ALARM)
			{
				if(((wakecounts_to_check_update - wakecounts) >= (WAKE_COUNTS_STEP_SIZE * 5))
						&& (wakecounts_to_check_update != WAKE_COUNT_START_VALUE))
				{
					if(glSystem_cfg.Coin_Only_Temporary_GW_Enable == TRUE)
					{
						//GW RTC, if success then send Audit Registry
						if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))
						{
							//Debug_TextOut(0,"GET_RTC from GW");
							get_RTC();
						}
					}
					else if(glSystem_cfg.wakeup_coil_jam_detection_enable == TRUE)
					{
						if(WAKEUP_COIL_JAM_RAISED == FALSE)
						{
							WAKEUP_COIL_JAM_RAISED = TRUE;
							glAlarm_req.alarm_type  =   UDP_EVTTYP_ALARM;
							glAlarm_req.alarm_size  =	3;
							glAlarm_req.alarm_ts    = 	RTC_epoch_now();
							glAlarm_req.alarm_state =	MB_ALARM_STATE_RAISED;
							glAlarm_req.alarm_id    =  	UDP_ALMTYP_Wakeup_CoilJam_Error;
							Debug_TextOut(0, "Wakeup Coil Jam Detected");
							gprs_do_Alarms(&glAlarm_req);
							Idle_Screen();
						}
					}
					else
					{
						Debug_TextOut(2, "Wakeup Coil Jam Disabled");
					}
				}
				//else
				//Debug_Output2(0, "wakeupcoil: %d, %d", CoinIndex, glSystem_cfg.operational_parameters.wakecount_adjust_interval);
			}
			else if(wakeup_coil_alarm_check == CLEAR_WAKEUP_COIL_ALARM)
			{
				if(WAKEUP_COIL_JAM_RAISED == TRUE)
				{
					WAKEUP_COIL_JAM_RAISED = FALSE;
					glAlarm_req.alarm_type  =   UDP_EVTTYP_ALARM;
					glAlarm_req.alarm_size  =	3;
					glAlarm_req.alarm_ts    = 	RTC_epoch_now();
					glAlarm_req.alarm_state =	MB_ALARM_STATE_CLEARED;
					glAlarm_req.alarm_id    =  	UDP_ALMTYP_Wakeup_CoilJam_Error;
					Debug_TextOut(0, "Wakeup Coil Jam Cleared");
					gprs_do_Alarms(&glAlarm_req);
					Idle_Screen();
					Coin_WakeCount_adjustment(JAM_CLEAR_UPWARDS_ADJUSTMENT);
				}
			}
		}
		//Wakeup Coil Jam detection end
	}
	//Optical Jam detection end
}

/**************************************************************************/
//! This function is used to adjust the wakecount threshold for detecting
//! coin presence periodically. This is done to accommodate drift in wakecount
//! due to environmental effect on wakeup coil
//! \param uint8_t upwards_adjust - to know the reason for adjusting wakecount
//! \return void
/**************************************************************************/
void Coin_WakeCount_adjustment(uint8_t upwards_adjust)
{
	uint16_t coin_count_reset_value = 0;
	if(upwards_adjust == REGULAR_UPWARDS_ADJUSTMENT)
	{
		if(Wakecount_adjust_interval++ == glSystem_cfg.operational_parameters.wakecount_adjust_interval)
		{
			wakecountsTest = wakecountsTest + wakecounts;
			if (CoinIndex++ == 59 )    //6*5=30 values for 5 minutes
			{
				wakecounts_to_check_update = ((wakecountsTest/60) - WAKE_COUNTS_STEP_SIZE);
				Debug_Output1(0, "wakecounts Upwards: %d", wakecounts_to_check_update);
				/*				if(fluctuating_counts == 0)	//No fluctuations, clear the coin jam alarm now if raised
					Check_Coin_Jam_Alarm(CLEAR_WAKEUP_COIL_ALARM);*/
				wakecountsTest = 0;
				CoinIndex = 0;
			}
			Wakecount_adjust_interval = 0;
		}
		if((((Wakecount_adjust_interval * CoinIndex) % 60) == 0) && (Wakecount_adjust_interval != 0))	//Reset count every 60 seconds
		{
			if(fluctuating_counts == 0)	//No fluctuations for a minute of wakecount upwards adjustment
			{
				//clear the coin jam alarm now if raised only if wakecount has come back to normal
				if(Last_WakeCount_downwards < wakecounts_to_check_update)
				{
					if((wakecounts_to_check_update - Last_WakeCount_downwards) >= (WAKE_COUNTS_STEP_SIZE * 5))
					{
						Check_Coin_Jam_Alarm(CLEAR_WAKEUP_COIL_ALARM);
						Last_WakeCount_downwards = wakecounts_to_check_update;
					}
				}
			}
			fluctuating_counts = 0;	//start over again
		}
		//}
	}
	else if(upwards_adjust == JAM_CLEAR_UPWARDS_ADJUSTMENT)
	{
		wakecounts_to_check_update = WAKE_COUNT_START_VALUE;//((wakecountsTest/60) - WAKE_COUNTS_STEP_SIZE);
		Debug_Output1(0, "wakecounts Jam Clear: %d", wakecounts_to_check_update);
		jam = 1; //to credit time for last coin jammed and released //vinay
		//Debug_Output1(0, "1jam: %d", jam);
		wakecountsTest = 0;
		CoinIndex = 0;
		Wakecount_adjust_interval = 0;
	}
	else
	{
		if(((WakeCountAdjust++ > WAKE_COUNT_BOOTUP_THRESHOLD) && (Be4Bootup != 1)) || (fluctuating_counts >= glSystem_cfg.operational_parameters.wakeup_jam_confirmation_count))
		{
			//First Check if it is a coin jam
			Check_Coin_Jam_Alarm(RAISE_WAKEUP_COIL_ALARM);

			//Debug_TextOut( 0, "WAKEUp count Adjustment");
			if(wakecounts_to_check_update > WAKE_COUNTS_STEP_SIZE)
				wakecounts_to_check_update = wakecounts - WAKE_COUNTS_STEP_SIZE;
			Last_WakeCount_downwards = wakecounts_to_check_update;
			Debug_Output1(0, "wakecounts downwards: %d", wakecounts_to_check_update);
			if(fluctuating_counts >= glSystem_cfg.operational_parameters.wakeup_jam_confirmation_count)	//If it came here due to fluctuating counts
				coin_count_reset_value = glSystem_cfg.operational_parameters.wakeup_jam_confirmation_count;
			else
				coin_count_reset_value = WAKE_COUNT_BOOTUP_THRESHOLD;
			fluctuating_counts = 0;
			WakeCountAdjust = 0;
			Be4Bootup = 1;

			if(glTotal_Coin_Count > coin_count_reset_value)	//We got fake interrupts, so ignore these 5 wakeup counts
				glTotal_Coin_Count -= coin_count_reset_value;
			else
				glTotal_Coin_Count = 0;
		}
	}
}

/**************************************************************************/
//! This function is used to extract the validation parameter from meter's
//! configuration for the coin index which will be used to identify the coin
//! \param pointer to structure where output will be given
//! \param uint8_t coin index which will be extracted
//! \return bool
//! - \b Invalid Coin index
//! - \b Successfully loaded
/**************************************************************************/
bool GetCoinParameters(CoinSpecsType_New *pCoinParam, uint8_t extract_coin_number)
{
	if(NULL == pCoinParam)
	{
		return false;
	}

	if (extract_coin_number > MAX_COINS_SUPPORTED)
	{
		return false;
	}

	memcpy(pCoinParam->params, glSystem_cfg.coin_parameters[extract_coin_number].params, sizeof(pCoinParam->params));
	pCoinParam->value_units = glSystem_cfg.coin_parameters[extract_coin_number].value_units;

	return true;
}

/**************************************************************************/
//! This function is used to extract the validation parameter from meter's
//! configuration for the coin index (multipart coins with narrow and expanded
//! ranges) which will be used to identify the coin
//! \param pointer to structure where output will be given
//! \param uint8_t coin index which will be extracted
//! \return bool
//! - \b Invalid Coin index
//! - \b Successfully loaded
/**************************************************************************/
bool GetMultipartCoinParameters(CoinSpecs_Multipart *pCoinParam, uint8_t extract_coin_number)
{
	if(NULL == pCoinParam)
	{
		return false;
	}

	if (extract_coin_number > MULTIPART_COINS_COUNT)
	{
		return false;
	}

	memcpy(pCoinParam->narrow_params, glSystem_cfg.multipart_coin_parameters[extract_coin_number].narrow_params, sizeof(pCoinParam->narrow_params));
	memcpy(pCoinParam->expanded_params, glSystem_cfg.multipart_coin_parameters[extract_coin_number].expanded_params, sizeof(pCoinParam->expanded_params));
	pCoinParam->value_units = glSystem_cfg.multipart_coin_parameters[extract_coin_number].value_units;
	pCoinParam->min_no_of_narrow_to_pass = glSystem_cfg.multipart_coin_parameters[extract_coin_number].min_no_of_narrow_to_pass;

	return true;
}

/**************************************************************************/
//! This function is used to capture the samples(time counters) while the
//! coin is passing through the Coin track
//! \param void
//! \return loCounts - Read sample value
/**************************************************************************/
uint16_t SensorRead()
{
	uint16_t loCounts = 0, wait_count = 10000;

	while(wait_count >0)
	{
		if(P6IFG & SNSRDY_PIN)
		{
			P6IFG &= ~ SNSRDY_PIN;
			break;
		}
		wait_count--;
	}

	loCounts = (TA0R ^ 0xFFFF);

	clsns_Low();  //clear sensor	//1
	reset_sensor_counter();
	clsns_High();  //Enable sensor

	return loCounts;
}

/**************************************************************************/
//! This function is used to calculate the ratio1 and ratio2 values for the
//! for the dropped coin using the max, min, sec_max parameters
//! \param void
//! \return void
/**************************************************************************/
void CalculateCoinRatios()
{
	uint16_t loTemp1;
	//ratio1:         ;(MAX1/MIN) * 64
	if((max_sens != 0) && (sec_max != 0))
	{
		loTemp1 = (min_sens * 64/max_sens);
		ratio_1 = (uint8_t)(loTemp1 & 0xFF);
		//ratio2:         ;(MAX1/MAX2) * 64
		loTemp1 = (min_sens * 64/sec_max);
		ratio_2 = (uint8_t)(loTemp1 & 0xFF);
		//Debug_Output6( 0, "Ratio_1[%d]    Ratio_2[%d]",ratio_1, ratio_2, 0,0,0,0);
	}
	else
	{
		ratio_1 = 0;
		ratio_2 = 0;
	}
}

/**************************************************************************/
//! This function is used to find the minimum value of a passed array
//! \param pointer to the buffer
//! \param uint16_t length
//! \param uint16_t Reference min value to start with
//! \return loMinValue - minimum value in the buffer
/**************************************************************************/
uint16_t FindMinSensorValue(uint16_t * iSensorSamples, uint16_t iNumberOfSamples, uint16_t iRefValue)
{
	uint16_t loMinValue = iRefValue;
	uint16_t i;
	if(iSensorSamples == NULL || iNumberOfSamples <= 0)
	{
		return loMinValue;
	}
	if(sec_max==0)MinValue_point = 0;//for first time to calculate min peak
	for(i=0; i<iNumberOfSamples; i++)
	{
		if(sec_max==0)//first time to calculate min peak
		{
			if(iSensorSamples[i] < loMinValue)
					{
						loMinValue = iSensorSamples[i];
						MinValue_point = i;
					}
					if((iSensorSamples[i] > loMinValue) && (iSensorSamples[i+1] > loMinValue)
							&& (iSensorSamples[i+2] > loMinValue) && (iSensorSamples[i+3] > loMinValue)
							&& (iSensorSamples[i+4] > loMinValue) && (iSensorSamples[i+5] > loMinValue)
							&& (iSensorSamples[i+6] > loMinValue) && (iSensorSamples[i+7] > loMinValue)
							&& (iSensorSamples[i+8] > loMinValue) && (iSensorSamples[i+9] > loMinValue))	// As soon as profile starts going up, break and look for next sec_max
						break;
		}
		else //second time to calculate min peak(if required)
		{
			if((iSensorSamples[i+1] > iSensorSamples[i])
				&& (iSensorSamples[i+2] > iSensorSamples[i]) && (iSensorSamples[i+3] > iSensorSamples[i])
				&& (iSensorSamples[i+4] > iSensorSamples[i]) && (iSensorSamples[i+5] > iSensorSamples[i])
				&& (iSensorSamples[i+6] > iSensorSamples[i]) && (iSensorSamples[i+7] > iSensorSamples[i])
				&& (iSensorSamples[i+8] > iSensorSamples[i]) && (iSensorSamples[i+9] > iSensorSamples[i]))
			{ //will take min peak from first peak to second peak area,if multiple min peak were there then meter overwrite/pick correct min peak
				if(iSensorSamples[i] < loMinValue)
					{
						loMinValue = iSensorSamples[i];
						MinValue_point = i;
					}
			}
		}

	}
	return loMinValue;
}

/**************************************************************************/
//! This function is used to find the maximum value of a passed array
//! \param pointer to the buffer
//! \param uint16_t length
//! \param uint16_t Reference max value to start with
//! \return loMinValue - maximum value in the buffer
/**************************************************************************/
uint16_t FindMaxSensorValue(uint16_t * iSensorSamples, uint16_t iNumberOfSamples, uint16_t iRefValue)
{
	uint16_t loMaxValue = iRefValue;
	uint16_t i;

	MaxValue_point = 0;
	if(iSensorSamples == NULL || iNumberOfSamples <= 0)
	{
		return loMaxValue;
	}
	for(i=0; i<iNumberOfSamples; i++)
	{
		if((iSensorSamples[i] > loMaxValue)&&(iSensorSamples[i]<max_sens))	//// VT
		{
			loMaxValue = iSensorSamples[i];
			MaxValue_point = i;
		}
		/*if((iSensorSamples[i] < loMaxValue) && (iSensorSamples[i+1] < loMaxValue)
						&& (iSensorSamples[i+2] < loMaxValue) && (iSensorSamples[i+3] < loMaxValue))	// As soon as profiloe starts going up, break and look for next sec_max
					break;*/
	}
	return loMaxValue;
}

/**************************************************************************/
//! This function captures the profile for the coin being dropped and cleans
//! the data. Then finds baseline, max, min and sec_max for the coin.
//! \param uint16_t sample point
//! \return uint8_t result
//! - \b Success
//! - \b ERROR
/**************************************************************************/
bool GetDroppedCoinParameters(uint16_t sample_point)
{
	uint8_t loBLstart, loLength, loFlag;//, max1_failure = false;
	uint16_t i = 0, k = 0, no_of_samples_toscan = 0, wait_count = 10000;
	uint16_t max_sens_t = 0;
	
	if(HARDWARE_REVISION == LNG_REV4)	//Do not change, need to differentiate before the for loop; otherwise matching the time is difficult after that
	{
		for(i=0; i<SENSOR_NUMBER_OF_READS; i++)
		{
			wait_count = 10000;
			while(wait_count--)
			{
				if(P6IFG & SNSRDY_PIN)
				{
					P6IFG &= ~SNSRDY_PIN;
					break;
				}
			}
			loSensorSamples[(i+sample_point)] = TA0R ^ 0xFFFF;
			clsns_Low();  //clear sensor	//1
			DelayUs(5);
			reset_sensor_counter();
			clsns_High();  //Enable sensor
			//DelayUs(1);
			if((i+sample_point)>(2+sample_point))	//// VT
			{
				loSensorSamples[i+sample_point] = (loSensorSamples[i+sample_point]+loSensorSamples[(i+sample_point)-1])/2;

				if(loSensorSamples[i+sample_point] > max_sens)
				{
					max_sens = loSensorSamples[i+sample_point];
					MaxValue_point1 = i+sample_point;
				}
			}
		}
		loBLstart = COIN_BASELINE_START_R4;
		loLength = POINTS_FOR_BASELINE_R4;
	}
	else
	{
		for(i=0; i<SENSOR_NUMBER_OF_READS; i++)
		{
			wait_count = 10000;
			while(wait_count--)
			{
				//if (get_Snsrdy_val() == 1)
				if((P6IN & BIT_0))
				{
					P6IFG &= ~SNSRDY_PIN;
					break;
				}
			}
			loSensorSamples[(i+sample_point)] = TA0R ^ 0xFFFF;
			clsns_Low();  //clear sensor	//1
			//DelayUs(5);
			reset_sensor_counter();
			clsns_High();  //Enable sensor
			DelayUs(1);

			//Adding the averaging here itself to save time later
			if(((i+sample_point)>AVERAGING_POINTS))
			{
				k = ((i+sample_point)-AVERAGING_POINTS);
				glTemp0 = 0;
				for(glIdx=0; glIdx < AVERAGING_POINTS; glIdx++)
				{
					glTemp0 += loSensorSamples[k+glIdx];
				}
				loSensorSamples[k] = glTemp0/AVERAGING_POINTS;
			}

			if((i+sample_point)>(2+sample_point))	//// VT
			{
				loSensorSamples[i+sample_point] = (loSensorSamples[i+sample_point]+loSensorSamples[(i+sample_point)-1])/2;

				if(loSensorSamples[i+sample_point] > max_sens)
				{
					max_sens = loSensorSamples[i+sample_point];
					MaxValue_point1 = i+sample_point;
				}
			}
		}
		loBLstart = COIN_BASELINE_START;
		loLength = POINTS_FOR_BASELINE;
	}


	cal_sens = 0;
	glTemp0 = 0;
	for(glIdx=0; glIdx < loLength; glIdx++)
	{
		glTemp0 += loSensorSamples[loBLstart+glIdx];
	}
	cal_sens = (uint16_t)(glTemp0 / loLength);

	//If required to Find second min
	min_sens = 0;
	sec_max = 0;//>>VT<<

	glIdx += (MaxValue_point1+40);

	//compute max difference (MIN)
	if(max_sens > cal_sens)
		{
			max_sens_t = max_sens - cal_sens;
		}
	else{
			max_sens_t = cal_sens - max_sens;
		}

	if((glIdx >= (SENSOR_NUMBER_OF_READS+sample_point))||(max_sens_t<100))	//M1 issue
	{
		return false;
	}
	//sec_max = FindMaxSensorValue(&loSensorSamples[glIdx],SENSOR_NUMBER_OF_READS-glIdx, 0/*max_sens*/);
	min_sens = FindMinSensorValue(&loSensorSamples[MaxValue_point1+15],((SENSOR_NUMBER_OF_READS+sample_point) - (MaxValue_point1+15)), max_sens);
	no_of_samples_toscan = MinValue_point+MaxValue_point1+15;
	if(no_of_samples_toscan < (SENSOR_NUMBER_OF_READS+sample_point))	//// VT
		sec_max = FindMaxSensorValue(&loSensorSamples[no_of_samples_toscan],((SENSOR_NUMBER_OF_READS+sample_point)-no_of_samples_toscan), 0);
	else
		sec_max = max_sens;

	//To calculate cal/base value after second peak,if first peak fall near to cal/base value >>VT<<
	if(MaxValue_point1<100)
		{
			cal_sens_t = cal_sens;
			glTemp0 = 0;
			for(glIdx=0; glIdx < loLength; glIdx++)
				{
					////Debug_Output2(0,"i=%d,cal_values=%ld",(loBLstart+glIdx+(no_of_samples_toscan+MaxValue_point+50)),loSensorSamples[loBLstart+glIdx+(no_of_samples_toscan+MaxValue_point+50)]);
					glTemp0 += loSensorSamples[loBLstart+glIdx+(no_of_samples_toscan+MaxValue_point+150)];
					if(loSensorSamples[loBLstart+glIdx+(no_of_samples_toscan+MaxValue_point+150)] == 0)
						{
							glTemp0 = 0; //if any sample value become zero, we will use old cal value
							break;
						}
				}
			if(glTemp0>100)
				{
					cal_sens = 0;
					cal_sens = (uint16_t)(glTemp0 / loLength);
					coin_cal_sens_taken_from_last = TRUE;
				}
		}

	if((MinValue_point+MaxValue_point1+15)<=(((MaxValue_point+MinValue_point+MaxValue_point1+15)+MaxValue_point1)/2))
		{//If wrong Min Point found in coin profile, double check Min value >>VT<<
			if((max_sens!=sec_max)&&((MinValue_point+MaxValue_point1+15)<((MaxValue_point+no_of_samples_toscan) - 9)))
				min_sens = FindMinSensorValue(&loSensorSamples[MinValue_point+MaxValue_point1+15],((MaxValue_point+no_of_samples_toscan) - (MaxValue_point1+15) - 9), min_sens);
			coin_second_check_for_Min_value = TRUE;
		}

	sense[0] = 0;
	sense[1] = 0;

	//compute min difference (MAX)
	if(min_sens > cal_sens)
	{
		min_sens = min_sens - cal_sens;
	}
	else
	{
		min_sens = cal_sens - min_sens;
	}

	//compute max difference (MIN)
	if(max_sens > cal_sens)
	{
		max_sens = max_sens - cal_sens;
	}else{
		max_sens = cal_sens - max_sens;
	}

	if(sec_max > cal_sens)
	{
		sec_max = sec_max - cal_sens;
	}else{
		sec_max = cal_sens - sec_max;
	}
#if 0
	uint8_t value[10]={0};
	//if((max_sens < min_sens) || (max_sens < 50) || (sec_max < 50))
	{
		Debug_TextOut(0," ");
		for(i=0;i<600/*(SENSOR_NUMBER_OF_READS+sample_point)*/;i++) //changes;
		{
			liberty_sprintf((char*)value," %d",loSensorSamples[i]);
			Debug_Display(0, (char*)value);
		}
	}
#endif
	//Debug_Output6( 0, "cal_sens[%d] min_sens[%d] max_sens[%d] sec_max[%d] sec_base[%d]", cal_sens, min_sens, max_sens, sec_max, 0,0);
	/********************************************/
	temp_co = 0;  //Let skip tcomp for now
	/********************************************/

	//temp_comp:  //Temperature compensation
	if(temp_co <= 0 || coin_validation_number >= 31)
	{
		return true;
	}

	//do_tcomp:
	loFlag = 0; //assume positive shift
	if(cal_sens >= base_freq)
	{
		//positive shift
		glTemp1 = cal_sens - base_freq;
	}else{
		//negative shift
		loFlag = 0x80;
		glTemp1 = base_freq - cal_sens;
	}

	glTemp0 = (uint32_t)(glTemp1 * (temp_co & 0x7F));
	glTemp1 = (uint16_t)glTemp0/1024;   //div by 1024  //glTemp1 = temp shift*coeff/1024 for MAX comp
	glTemp2 = max_sens / 8;
	//save temp shift*coeff/1024
	glTemp0 = (uint32_t)(glTemp2 * glTemp1);
	glTemp2 = (uint16_t)(glTemp0 / 16);  //glTemp2 = MIN tcomp

	glTemp0 = (uint32_t)(min_sens / 8);  //MAX
	glTemp0 = (uint32_t)(glTemp0 * glTemp1);
	glTemp3 = (uint16_t)(glTemp0 / 16);  //loTemp3 = MAX tcomp

	glTemp0 = (uint32_t)(sec_max / 8);  //MAX
	glTemp0 = (uint32_t)(glTemp0 * glTemp1);
	glTemp4 = (uint16_t)(glTemp0 / 16);  //loTemp4 = sec MAX tcomp

	if((temp_co ^ loFlag) != 0x00)//negative XOR positive  or positive XOR negative = negative temp co
	{
		//plus_tcomp
		//second max
		sec_max = sec_max + glTemp4;
		//MAX
		min_sens = min_sens + glTemp3;
		sense[0] = min_sens & 0xFF;
		sense[1] = (min_sens >> 8) & 0xFF;
		//MIN
		max_sens = max_sens + glTemp2;
		//Done
		return true;
	}
	//minus tcomp
	sec_max = sec_max - glTemp4;  //Second max
	//MAX
	min_sens = min_sens - glTemp3;
	//sense[0] = min_sens & 0xFF;
	//sense[1] = (min_sens >> 8) & 0xFF;
	//MIN
	max_sens = max_sens - glTemp2;
	//Done
	return true;
}

/**************************************************************************/
//! Coin Validation Routine - Reads coin, validates it, adds parking clock
//! and prepares the Coin transaction data.
//! \param void
//! \return void
/**************************************************************************/
void CoinValidation(void)
{
	uint8_t sample_count = 0, Max_not_found = false, Max_not_good = false;
	//If number of coins is zero, begin processing the coins
	//StartCoinSensorTimeout();

	//Clean start
	coin_validation_number = 0;
	coin_count = 0;

	//low frequency first
	chirp_High(); //p5.chirp = 1;
	//Turn on sensor, and 12mhz clock
	coin_sensor_init(); //sens_init();
	//If error this time, exit
	if((dcd_error & coil_err) == coil_err)
	{
		//Coil error, exit
		coin_sensor_off(); //sens_off();
		coin_validation_number = 0;
		Debug_TextOut( 0, "Coil Error");
		return;
	}
	memset(loSensorSamples, 0, sizeof(loSensorSamples));
	MaxValue_point = 0;
	MaxValue_point1 = 0;
	max_sens = 0;
	enable_optical_LED_interrupt();	//Not using optical LED anymore, only using the SNSREDY interrupt
	coin_sensor_on();
	Optical_LED_touched = FALSE;
	//Now read the sensor samples and calculate the parameters
	for(sample_count = 0; sample_count < glSystem_cfg.operational_parameters.max_sampling_count; sample_count++)
	{
		if(sample_count >= MAX_COIN_SAMPLING_TRIALS)	//Protection for Stack Overflow
		{
			Max_not_found = true;
			break;
		}
		if(GetDroppedCoinParameters(sample_count*SENSOR_NUMBER_OF_READS) == false)
		{
			Max_not_found = true;
			//Debug_TextOut( 0, "*** SENSOR TIMEOUT: First Max ***");
			//return;
		}
		else
		{
			//We successfully got the parameters of the dropped coin, now calc the ratios
			CalculateCoinRatios();
			//Debug_Output6( 0, "max:%d, min:%d, rat1:%d, rat2:%d", max_sens, min_sens, ratio_1, ratio_2, 0, 0);
			if((ratio_1>max_sens) || (ratio_2>max_sens) || (max_sens<100) || (max_sens<min_sens) || ((ratio_1 == 0) && (ratio_2 == 0)))	//If both ratio1 & ratio 2 are 0, then also consider it is fake wakeup
			{
				Max_not_good = true;
				//return;
			}
			else	//successful read, clear all failure flags and break
			{
				Max_not_found = false;
				Max_not_good = false;
				coin_valid = 1; 		//for refreshing the screen in lcd.c to check the min amount //vinay trenton
				//Debug_Output1( 0, "trial:%d", sample_count+1);
				if(wakecounts_to_check_update != WAKE_COUNT_START_VALUE) //fluctuating count clearing condition added >>VT<<
					fluctuating_counts = 0;	//a valid coin detected
				break;
			}
		}
	}
	coin_sensor_off();

	uint32_t gpio_read_for_led = 0;
	gpio_read_for_led = GPIO_getEnabledInterruptStatus(LED_RXD_PORT);
	if(gpio_read_for_led & LED_RXD_PIN)
	{
		Optical_LED_touched = TRUE;
	}
	disable_optical_LED_interrupt();

	if((Max_not_found == true) || (Max_not_good == true))
	{
		if(Max_not_found == true)
			Debug_TextOut( 0, "M1");
		else
			Debug_TextOut( 0, "M2");
		fluctuating_counts++;
		if((M1_M2_Counter<=100)&&(((glSystem_cfg.allowed_trans_types>>14) & 0x01) == 1))M1_M2_Counter++;
		//Debug_Output1(0, "fluc: %d", fluctuating_counts);
		watchdog_pat();	//It is needed here because we would have spent extra time while waiting for the M1/M2 issues
#if 0
		uint8_t value[10]={0};
		uint16_t i;
		for(i=0;i<(SENSOR_NUMBER_OF_READS*sample_count);i++) //changes;
		{
			liberty_sprintf((char*)value," %d",(loSensorSamples[i] - 51200));
			Debug_Display(0, (char*)value);
		}
#endif
		if((((glSystem_cfg.allowed_trans_types>>14) & 0x01) == 1)&&(Be4Bootup == 1)&&(M1_M2_Counter < 100)&&(OPTICAL_COIN_JAM_RAISED==FALSE)&&(glCoin_Index<5)) //for invalid coins only //vinay
			  { //(glCoin_Index<5) This condition added to reduce saving of M1 and M2 to not clear coin valid data
				//invalid coins value backup
				{
					glCoindata[glCoin_Index].cashtranstime = RTC_epoch_now();
					if(Max_not_found == true)glCoindata[glCoin_Index].coin_index = 997;	//M1
					else glCoindata[glCoin_Index].coin_index = 998;	//M2
					glCoindata[glCoin_Index].coinval = 0;
					glCoindata[glCoin_Index].earned_coin_hr = 0;
					glCoindata[glCoin_Index].earned_coin_min = 0;
					//glCoindata[glCoin_Index].cumulative_parktime_hr = 0;
					//glCoindata[glCoin_Index].cumulative_parktime_min = 0;
					//to avoid time paid zero //vinay
					{
						glCoindata[glCoin_Index].cumulative_parktime_hr = (parking_time_left_on_meter[Current_Space_Id] / 3600);
						glCoindata[glCoin_Index].cumulative_parktime_min = ((parking_time_left_on_meter[Current_Space_Id] % 3600)/60);

						if(((parking_time_left_on_meter[Current_Space_Id] % 3600) % 60) > 0)	//This is to fix the wrong update of parking clock
						{
							glCoindata[glCoin_Index].cumulative_parktime_min += 1;
							if(glCoindata[glCoin_Index].cumulative_parktime_min > 59)
							{
								glCoindata[glCoin_Index].cumulative_parktime_hr += 1;
								glCoindata[glCoin_Index].cumulative_parktime_min = 0;
							}
						}
					}
					glCoindata[glCoin_Index].max = max_sens;  //to send invalid coin profile
					glCoindata[glCoin_Index].min = min_sens;  //to send invalid coin profile
					glCoindata[glCoin_Index].rat1 = ratio_1;  //to send invalid coin profile
					glCoindata[glCoin_Index].rat2 = ratio_2;  //to send invalid coin profile
				}
				//Debug_Output6(0,"CT=%ld,CI=%d,CV=%d,E_hr=%ld,E_min=%ld,C_hr=%ld",glCoindata[glCoin_Index].cashtranstime,glCoindata[glCoin_Index].coin_index,glCoindata[glCoin_Index].coinval,glCoindata[glCoin_Index].earned_coin_hr,glCoindata[glCoin_Index].earned_coin_min,glCoindata[glCoin_Index].cumulative_parktime_hr);
				//Debug_Output6(0,"C_min=%ld,max=%ld,min=%ld,rat1=%ld,rat2=%d",glCoindata[glCoin_Index].cumulative_parktime_min,glCoindata[glCoin_Index].max,glCoindata[glCoin_Index].min,glCoindata[glCoin_Index].rat1,glCoindata[glCoin_Index].rat2,0);
				//Last_Cash_Payment_RTC = RTC_epoch_now();
				glCoin_Index++;							//this will be incremented for all coins
				if(glCoin_Index >= MAX_COINS_TO_UDP)
					glCoin_Index = 0;

				if(Max_not_found == true)push_event_to_cache( UDP_EVTTYP_COIN_M1_ERROR_OCCURED);//>>VT<< M1 EVENT
				else push_event_to_cache( UDP_EVTTYP_COIN_M2_ERROR_OCCURED);//>>VT<< M2 EVENT
			}
		return;
	}
	else	//valid coin detection, clear all alarms
		Check_Coin_Jam_Alarm(CLEAR_WAKEUP_COIL_ALARM);

	Debug_Output6(0, "max:%d, min:%d, rat1:%d, rat2:%d", max_sens, min_sens, ratio_1, ratio_2, 0, 0);
	coindrop_TimeOut_RTC =  RTC_epoch_now(); // for invalid coins the screen gets blank or for even coin jam //to avoid this using it//vinay
	//Debug_Output1( 0, "1coindrop_TimeOut_RTC: %d\r\n",coindrop_TimeOut_RTC);
	if((gAutoCalibration == true) )//|| ((gSelfCalibration == true)))
	{
		//Debug_TextOut( 0, "AutoCalibration");
		Coin_Calibration_Mode(IN_CALIBRATION_MODE);
		return;
	}
	//Now Validate coin parametrs
	if((current_rate_index != FREE_PARKING) && (current_rate_index != NO_PARKING))	// Credit time only in paid parking
	{
		//Debug_Output1(0, "2jam: %d", jam);
		if(jam == 1) //when wakeup coil jam and then cleared then the dropped coin was not crediting value to any space // vinay
		{
			jam = 0;
			Current_Space_Id = key_RL;
			Debug_Output1(0,"Current_Space_Id_RL = %d",Current_Space_Id);
			CoinOk();
			return;
		}
		//Debug_Output1(0,"Current_Space_Id_1 = %d",Current_Space_Id);
		if(((Current_Space_Id != LEFT_BAY) && (Current_Space_Id != RIGHT_BAY))&&(payment_on_hold==FALSE))
		{
			//Moving current coin profile to temp1  VT
			temp1_max_sens = max_sens;
			temp1_min_sens = min_sens;
			temp1_ratio_1 = ratio_1;
			temp1_ratio_2 = ratio_2;
			//Debug_Output1(0,"Current_Space_Id_2 = %d",Current_Space_Id);
			payment_on_hold = true;
			DSM_Screen4();
			payment_on_hold_timeout = RTC_epoch_now();
			return;
		}
		else if(payment_on_hold==TRUE)
		{
			//Moving current coin profile to temp2
			temp2_max_sens = max_sens;
			temp2_min_sens = min_sens;
			temp2_ratio_1 = ratio_1;
			temp2_ratio_2 = ratio_2;

			//Moving current temp1 coin profile to original profile
			max_sens = temp1_max_sens;
			min_sens = temp1_min_sens;
			ratio_1 = temp1_ratio_1;
			ratio_2 = temp1_ratio_2;

			CoinOk();
			DelayMs(100);
			payment_on_hold = false;

			//Moving current temp2 coin profile to original profile
			max_sens = temp2_max_sens;
			min_sens = temp2_min_sens;
			ratio_1 = temp2_ratio_1;
			ratio_2 = temp2_ratio_2;

			//Making temp1 and temp2 as zero
			temp1_max_sens = 0;
			temp1_min_sens = 0;
			temp1_ratio_1 = 0;
			temp1_ratio_2 = 0;
			temp1_max_sens = 0;
			temp1_min_sens = 0;
			temp1_ratio_1 = 0;
			temp1_ratio_2 = 0;
		}

		CoinOk();
	}
	else if(current_rate_index == NO_PARKING)
	{
		Noparking_Screen();
		Debug_TextOut( 0, "PA-C:no_parking" );
		if(((glSystem_cfg.allowed_trans_types>>14) & 0x01) == 1) //for invalid coins only
		{
			//invalid coins value backup
			{
				glCoindata[glCoin_Index].cashtranstime = RTC_epoch_now();
				glCoindata[glCoin_Index].coin_index = 996;
				glCoindata[glCoin_Index].coinval = 0;
				glCoindata[glCoin_Index].earned_coin_hr = 0;
				glCoindata[glCoin_Index].earned_coin_min = 0;
				//glCoindata[glCoin_Index].cumulative_parktime_hr = 0;
				//glCoindata[glCoin_Index].cumulative_parktime_min = 0;
				//to avoid time paid zero //vinay
				{
					glCoindata[glCoin_Index].cumulative_parktime_hr = (parking_time_left_on_meter[Current_Space_Id] / 3600);
					glCoindata[glCoin_Index].cumulative_parktime_min = ((parking_time_left_on_meter[Current_Space_Id] % 3600)/60);

					if(((parking_time_left_on_meter[Current_Space_Id] % 3600) % 60) > 0)	//This is to fix the wrong update of parking clock
					{
						glCoindata[glCoin_Index].cumulative_parktime_min += 1;
						if(glCoindata[glCoin_Index].cumulative_parktime_min > 59)
						{
							glCoindata[glCoin_Index].cumulative_parktime_hr += 1;
							glCoindata[glCoin_Index].cumulative_parktime_min = 0;
						}
					}
				}
				glCoindata[glCoin_Index].space_id = 0;
				glCoindata[glCoin_Index].max = max_sens;  //to send invalid coin profile
				glCoindata[glCoin_Index].min = min_sens;  //to send invalid coin profile
				glCoindata[glCoin_Index].rat1 = ratio_1;  //to send invalid coin profile
				glCoindata[glCoin_Index].rat2 = ratio_2;  //to send invalid coin profile
			}
			//Debug_Output6(0,"CT=%ld,CI=%d,CV=%d,E_hr=%ld,E_min=%ld,C_hr=%ld",glCoindata[glCoin_Index].cashtranstime,glCoindata[glCoin_Index].coin_index,glCoindata[glCoin_Index].coinval,glCoindata[glCoin_Index].earned_coin_hr,glCoindata[glCoin_Index].earned_coin_min,glCoindata[glCoin_Index].cumulative_parktime_hr);
			//Debug_Output6(0,"C_min=%ld,max=%ld,min=%ld,rat1=%ld,rat2=%d",glCoindata[glCoin_Index].cumulative_parktime_min,glCoindata[glCoin_Index].max,glCoindata[glCoin_Index].min,glCoindata[glCoin_Index].rat1,glCoindata[glCoin_Index].rat2,0);
			Last_Cash_Payment_RTC = RTC_epoch_now();
			glCoin_Index++;							//this will be incremented for all coins
			if(glCoin_Index >= MAX_COINS_TO_UDP)
				glCoin_Index = 0;
		}
		return;
	}
	else if(current_rate_index == FREE_PARKING)
	{
		Freeparking_Screen();
		Debug_TextOut( 0, "PA-C:free_parking" );
		if(((glSystem_cfg.allowed_trans_types>>14) & 0x01) == 1) //for invalid coins only
		{
			//invalid coins value backup
			{
				glCoindata[glCoin_Index].cashtranstime = RTC_epoch_now();
				glCoindata[glCoin_Index].coin_index = 995;
				glCoindata[glCoin_Index].coinval = 0;
				glCoindata[glCoin_Index].earned_coin_hr = 0;
				glCoindata[glCoin_Index].earned_coin_min = 0;
				//glCoindata[glCoin_Index].cumulative_parktime_hr = 0;
				//glCoindata[glCoin_Index].cumulative_parktime_min = 0;
				//to avoid time paid zero //vinay
				{
					glCoindata[glCoin_Index].cumulative_parktime_hr = (parking_time_left_on_meter[Current_Space_Id] / 3600);
					glCoindata[glCoin_Index].cumulative_parktime_min = ((parking_time_left_on_meter[Current_Space_Id] % 3600)/60);

					if(((parking_time_left_on_meter[Current_Space_Id] % 3600) % 60) > 0)	//This is to fix the wrong update of parking clock
					{
						glCoindata[glCoin_Index].cumulative_parktime_min += 1;
						if(glCoindata[glCoin_Index].cumulative_parktime_min > 59)
						{
							glCoindata[glCoin_Index].cumulative_parktime_hr += 1;
							glCoindata[glCoin_Index].cumulative_parktime_min = 0;
						}
					}
				}
				glCoindata[glCoin_Index].space_id = 0;
				glCoindata[glCoin_Index].max = max_sens;  //to send invalid coin profile
				glCoindata[glCoin_Index].min = min_sens;  //to send invalid coin profile
				glCoindata[glCoin_Index].rat1 = ratio_1;  //to send invalid coin profile
				glCoindata[glCoin_Index].rat2 = ratio_2;  //to send invalid coin profile
			}
			//Debug_Output6(0,"CT=%ld,CI=%d,CV=%d,E_hr=%ld,E_min=%ld,C_hr=%ld",glCoindata[glCoin_Index].cashtranstime,glCoindata[glCoin_Index].coin_index,glCoindata[glCoin_Index].coinval,glCoindata[glCoin_Index].earned_coin_hr,glCoindata[glCoin_Index].earned_coin_min,glCoindata[glCoin_Index].cumulative_parktime_hr);
			//Debug_Output6(0,"C_min=%ld,max=%ld,min=%ld,rat1=%ld,rat2=%d",glCoindata[glCoin_Index].cumulative_parktime_min,glCoindata[glCoin_Index].max,glCoindata[glCoin_Index].min,glCoindata[glCoin_Index].rat1,glCoindata[glCoin_Index].rat2,0);
			Last_Cash_Payment_RTC = RTC_epoch_now();
			glCoin_Index++;							//this will be incremented for all coins
			if(glCoin_Index >= MAX_COINS_TO_UDP)
				glCoin_Index = 0;
		}
		return;
	}
}

void SortanArray_ByValue(uint16_t *sort_buf, uint16_t sort_length)
{
	uint16_t sort_index1 = 0, sort_index2 = 0, temporary_variable = 0;

	for(sort_index1 = 0; sort_index1 < sort_length; sort_index1++)
	{
		for(sort_index2 = (sort_index1+1); sort_index2 < sort_length; sort_index2++)
		{
			if(sort_buf[sort_index1] > sort_buf[sort_index2])
			{
				temporary_variable = sort_buf[sort_index1];
				sort_buf[sort_index1] = sort_buf[sort_index2];
				sort_buf[sort_index2] = temporary_variable;
			}
		}
	}
	//for(sort_index1 = 0; sort_index1< sort_length; sort_index1++)
	//	Debug_Output1(0, "%d", sort_buf[sort_index1]);
}

/**************************************************************************/
//! Coin Manual Calibration Routine - This is called in Manual Calibration
//! \param uint8_t validated_coin - flag to notify if it is first time calibration
//! entry or in between calibration
//! \return void
/**************************************************************************/
void Coin_Calibration_Mode(uint8_t validated_coin)
{
	static uint8_t loIdx = 0;
	static uint8_t coin_calibration_index = (MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT) - 1;
	uint8_t i = 0, max_calib_tolerance = 0, min_calib_tolerance = 0;
	uint16_t coin_value_to_show = 0;
	CoinSpecsType_New loCoinParam;
	CoinSpecs_Multipart lomultipartCoinParam;
	uint8_t wrong_calibration_flag = 0;

	if(validated_coin == IN_CALIBRATION_MODE)
	{
		if(loIdx < NUMBER_OF_CAL_DROPS)
		{
			//Sanity check of the detected value is needed before using it for further averaging
			if(coin_calibration_index < MAX_COINS_SUPPORTED)
			{
				if(GetCoinParameters(&loCoinParam, coin_calibration_index) == false)
				{
					;
				}

				//Give an additional range for auto calibration
				//Max tolerance
				if(loCoinParam.params[0] > (glSystem_cfg.coin_calib_tol[coin_calibration_index].coin_max_tolerance*2 + glSystem_cfg.operational_parameters.add_max_tolerance_selfcal))
					loCoinParam.params[0] -= (glSystem_cfg.coin_calib_tol[coin_calibration_index].coin_max_tolerance*2 + glSystem_cfg.operational_parameters.add_max_tolerance_selfcal);
				else
					loCoinParam.params[0] = 0;
				loCoinParam.params[1] += (glSystem_cfg.coin_calib_tol[coin_calibration_index].coin_max_tolerance*2 + glSystem_cfg.operational_parameters.add_max_tolerance_selfcal);

				//Min tolerance
				if(loCoinParam.params[2] > (glSystem_cfg.coin_calib_tol[coin_calibration_index].coin_min_tolerance*2 + glSystem_cfg.operational_parameters.add_min_tolerance_selfcal))
					loCoinParam.params[2] -= (glSystem_cfg.coin_calib_tol[coin_calibration_index].coin_min_tolerance*2 + glSystem_cfg.operational_parameters.add_min_tolerance_selfcal);
				else
					loCoinParam.params[2] = 0;
				loCoinParam.params[3] += (glSystem_cfg.coin_calib_tol[coin_calibration_index].coin_min_tolerance*2 + glSystem_cfg.operational_parameters.add_min_tolerance_selfcal);

				//Ratio1 tolerance
				if(loCoinParam.params[4] > (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal))
					loCoinParam.params[4] -= (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal);
				else
					loCoinParam.params[4] = 0;
				loCoinParam.params[5] += (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal);

				//Ratio2 tolerance
				if(loCoinParam.params[6] > (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal))
					loCoinParam.params[6] -= (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal);
				else
					loCoinParam.params[6] = 0;
				loCoinParam.params[7] += (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal);

				if((max_sens >= loCoinParam.params[0] && max_sens <= loCoinParam.params[1]) &&
						(min_sens >= loCoinParam.params[2] && min_sens <= loCoinParam.params[3]) &&
						(ratio_1 >= loCoinParam.params[4] && ratio_1 <= loCoinParam.params[5]) &&
						(ratio_2 >= loCoinParam.params[6] && ratio_2 <= loCoinParam.params[7]))
				{
					//Debug_Output6( 0, "Coin Drop# *** %d ***", (loIdx+1),0,0,0,0,0);
					loMin_Sens[loIdx] = min_sens;
					loMax_Sens[loIdx] = max_sens;
					loRatio1[loIdx] = ratio_1;
					loRatio2[loIdx] = ratio_2;
					loIdx++;
				}
				else
				{
					Debug_TextOut(0,"Wrong Coin");
				}
			}
			else
			{
				//For multipart coins, the sanity check will be done in expanded range only
				if(GetMultipartCoinParameters(&lomultipartCoinParam, (coin_calibration_index-MAX_COINS_SUPPORTED)) == false)
				{
					;
				}

				if((max_sens >= lomultipartCoinParam.expanded_params[0] && max_sens <= lomultipartCoinParam.expanded_params[1]) &&
						(min_sens >= lomultipartCoinParam.expanded_params[2] && min_sens <= lomultipartCoinParam.expanded_params[3]) &&
						(ratio_1 >= lomultipartCoinParam.expanded_params[4] && ratio_1 <= lomultipartCoinParam.expanded_params[5]) &&
						(ratio_2 >= lomultipartCoinParam.expanded_params[6] && ratio_2 <= lomultipartCoinParam.expanded_params[7]))
				{
					//Debug_Output6( 0, "Coin Drop# *** %d ***", (loIdx+1),0,0,0,0,0);
					loMin_Sens[loIdx] = min_sens;
					loMax_Sens[loIdx] = max_sens;
					loRatio1[loIdx] = ratio_1;
					loRatio2[loIdx] = ratio_2;
					loIdx++;
				}
				else
				{
					Debug_TextOut(0,"Wrong Coin");
				}
			}
		}
		if(loIdx >= NUMBER_OF_CAL_DROPS)
		{
			//We'll self calibrate max and min only, ratio1, ratio2 will be copied as they are from config file
			sum_max = 0;
			sum_min = 0;
			sum_rat1 = 0;
			sum_rat2 = 0;
			averaged_max = 0;
			averaged_min = 0;
			averaged_rat1 = 0;
			averaged_rat2 = 0;

			for(i=0; i<NUMBER_OF_CAL_DROPS; i++)
			{
				sum_max += loMax_Sens[i];
				sum_min += loMin_Sens[i];
				sum_rat1 += loRatio1[i];
				sum_rat2 += loRatio2[i];
			}
			averaged_max = sum_max / NUMBER_OF_CAL_DROPS;
			averaged_min = sum_min / NUMBER_OF_CAL_DROPS;
			averaged_rat1 = sum_rat1 / NUMBER_OF_CAL_DROPS;
			averaged_rat2 = sum_rat2 / NUMBER_OF_CAL_DROPS;



			Debug_Output6( 0, "avg_max:%d, avg_min:%d, avg_rat1:%d, avg_rat2:%d", averaged_max,	averaged_min, averaged_rat1, averaged_rat2, 0, 0);

			if(coin_calibration_index < MAX_COINS_SUPPORTED)
			{
				Coin_Calib_Params[coin_calibration_index].value_units = glSystem_cfg.coin_parameters[coin_calibration_index].value_units;

				//Set Ratio1 Copying from config
				Coin_Calib_Params[coin_calibration_index].params[4] = glSystem_cfg.coin_parameters[coin_calibration_index].params[4];
				Coin_Calib_Params[coin_calibration_index].params[5] = glSystem_cfg.coin_parameters[coin_calibration_index].params[5];

				//Set Ratio1 Copying from config
				Coin_Calib_Params[coin_calibration_index].params[6] = glSystem_cfg.coin_parameters[coin_calibration_index].params[6];
				Coin_Calib_Params[coin_calibration_index].params[7] = glSystem_cfg.coin_parameters[coin_calibration_index].params[7];

				 max_calib_tolerance = (glSystem_cfg.coin_calib_tol[coin_calibration_index].coin_max_tolerance*2);
				 min_calib_tolerance = (glSystem_cfg.coin_calib_tol[coin_calibration_index].coin_min_tolerance*2);
			}
			else
			{
				Coin_Calib_Params[coin_calibration_index].value_units = glSystem_cfg.multipart_coin_parameters[coin_calibration_index - MAX_COINS_SUPPORTED].value_units;
				//Set Ratio1 Copying from config
				Coin_Calib_Params[coin_calibration_index].params[4] = glSystem_cfg.multipart_coin_parameters[coin_calibration_index - MAX_COINS_SUPPORTED].narrow_params[4];
				Coin_Calib_Params[coin_calibration_index].params[5] = glSystem_cfg.multipart_coin_parameters[coin_calibration_index - MAX_COINS_SUPPORTED].narrow_params[5];

				//Set Ratio1 Copying from config
				Coin_Calib_Params[coin_calibration_index].params[6] = glSystem_cfg.multipart_coin_parameters[coin_calibration_index - MAX_COINS_SUPPORTED].narrow_params[6];
				Coin_Calib_Params[coin_calibration_index].params[7] = glSystem_cfg.multipart_coin_parameters[coin_calibration_index - MAX_COINS_SUPPORTED].narrow_params[7];

				 max_calib_tolerance = (glSystem_cfg.coin_calib_tol[coin_calibration_index].coin_max_tolerance*2);
				 min_calib_tolerance = (glSystem_cfg.coin_calib_tol[coin_calibration_index].coin_min_tolerance*2);
			}

			//Set Max Range
			if(averaged_max > max_calib_tolerance)
				Coin_Calib_Params[coin_calibration_index].params[0] = averaged_max - max_calib_tolerance;	//additional range
			else
				Coin_Calib_Params[coin_calibration_index].params[0] = 0;
			Coin_Calib_Params[coin_calibration_index].params[1] = averaged_max + max_calib_tolerance;	//additional range

			//Set Min Range
			if(averaged_min > min_calib_tolerance)	//// MULTIPLIED TOLERANCE BY 1.8
				Coin_Calib_Params[coin_calibration_index].params[2] = averaged_min - min_calib_tolerance;	//additional range
			else
				Coin_Calib_Params[coin_calibration_index].params[2] = 0;
			Coin_Calib_Params[coin_calibration_index].params[3] = averaged_min + min_calib_tolerance;	//additional range

			for(i=0; i<NUMBER_OF_CAL_DROPS; i++)
				{
					if((loMax_Sens[i] >= Coin_Calib_Params[coin_calibration_index].params[0] && loMax_Sens[i] <= Coin_Calib_Params[coin_calibration_index].params[1]) &&
										(loMin_Sens[i] >= Coin_Calib_Params[coin_calibration_index].params[2] && loMin_Sens[i] <= Coin_Calib_Params[coin_calibration_index].params[3]) &&
										(loRatio1[i] >= Coin_Calib_Params[coin_calibration_index].params[4] && loRatio1[i] <= Coin_Calib_Params[coin_calibration_index].params[5]) &&
										(loRatio2[i] >= Coin_Calib_Params[coin_calibration_index].params[6] && loRatio2[i] <= Coin_Calib_Params[coin_calibration_index].params[7]))
						{
							Debug_Output2(0,"coin %d index %d fall in current average range",i,coin_calibration_index);
						}
					else
						{
							Debug_Output2(0,"coin %d index %d not fall in current average range",i,coin_calibration_index);
							wrong_calibration_flag = TRUE;
						}

				}

			////SENDING AVERAGE VALUE THROUGH CD125 IF REQUIRED///////////////////////
			if((((glSystem_cfg.allowed_trans_types>>13) & 0x01) == 1) || (((glSystem_cfg.allowed_trans_types>>14) & 0x01) == 1))
						{
							{
								glCoindata[glCoin_Index].cashtranstime = RTC_epoch_now();
								if(wrong_calibration_flag==FALSE)glCoindata[glCoin_Index].coin_index = 1000+coin_calibration_index;	//auto calibration
								else glCoindata[glCoin_Index].coin_index = 1000+coin_calibration_index+100;
								glCoindata[glCoin_Index].coinval = 0;
								glCoindata[glCoin_Index].earned_coin_hr = 0;
								glCoindata[glCoin_Index].earned_coin_min = 0;
								//glCoindata[glCoin_Index].cumulative_parktime_hr = 0;
								//glCoindata[glCoin_Index].cumulative_parktime_min = 0;
								//to avoid time paid zero //vinay
								{
									glCoindata[glCoin_Index].cumulative_parktime_hr = (parking_time_left_on_meter[Current_Space_Id] / 3600);
									glCoindata[glCoin_Index].cumulative_parktime_min = ((parking_time_left_on_meter[Current_Space_Id] % 3600)/60);

									if(((parking_time_left_on_meter[Current_Space_Id] % 3600) % 60) > 0)	//This is to fix the wrong update of parking clock
									{
										glCoindata[glCoin_Index].cumulative_parktime_min += 1;
										if(glCoindata[glCoin_Index].cumulative_parktime_min > 59)
										{
											glCoindata[glCoin_Index].cumulative_parktime_hr += 1;
											glCoindata[glCoin_Index].cumulative_parktime_min = 0;
										}
									}
								}
								glCoindata[glCoin_Index].space_id = Current_Space_Id;
								glCoindata[glCoin_Index].max = (uint16_t)averaged_max;  //to send invalid coin profile
								glCoindata[glCoin_Index].min = (uint16_t)averaged_min;  //to send invalid coin profile
								glCoindata[glCoin_Index].rat1 = (uint8_t)averaged_rat1;  //to send invalid coin profile
								glCoindata[glCoin_Index].rat2 = (uint8_t)averaged_rat2;  //to send invalid coin profile
							}
							//Debug_Output6(0,"CT=%ld,CI=%d,CV=%d,E_hr=%ld,E_min=%ld,C_hr=%ld",glCoindata[glCoin_Index].cashtranstime,glCoindata[glCoin_Index].coin_index,glCoindata[glCoin_Index].coinval,glCoindata[glCoin_Index].earned_coin_hr,glCoindata[glCoin_Index].earned_coin_min,glCoindata[glCoin_Index].cumulative_parktime_hr);
							//Debug_Output6(0,"C_min=%ld,max=%ld,min=%ld,rat1=%ld,rat2=%d",glCoindata[glCoin_Index].cumulative_parktime_min,glCoindata[glCoin_Index].max,glCoindata[glCoin_Index].min,glCoindata[glCoin_Index].rat1,glCoindata[glCoin_Index].rat2,0);
							Last_Cash_Payment_RTC = RTC_epoch_now();
							glCoin_Index++;							//this will be incremented for all coins
							if(glCoin_Index >= MAX_COINS_TO_UDP)
								glCoin_Index = 0;
						}
			//////////////////////////////////////////////////////////////////////////
			//We have done, print the values now
			Debug_Output6( 0, "Index:%d, Value:%d, Max:%d,%d, Min:%d,%d", coin_calibration_index,
					Coin_Calib_Params[coin_calibration_index].value_units,
					Coin_Calib_Params[coin_calibration_index].params[0],
					Coin_Calib_Params[coin_calibration_index].params[1],
					Coin_Calib_Params[coin_calibration_index].params[2],
					Coin_Calib_Params[coin_calibration_index].params[3]);
			Debug_Output6( 0, "Ratio1:%d,%d Ratio2:%d,%d",
					Coin_Calib_Params[coin_calibration_index].params[4],
					Coin_Calib_Params[coin_calibration_index].params[5],
					Coin_Calib_Params[coin_calibration_index].params[6],
					Coin_Calib_Params[coin_calibration_index].params[7], 0, 0);

			loIdx = 0;
			memset(loMin_Sens, 0, sizeof(loMin_Sens));
			memset(loMax_Sens, 0, sizeof(loMax_Sens));
			memset(loRatio1, 0, sizeof(loRatio1));
			memset(loRatio2, 0, sizeof(loRatio2));
			coin_calibration_index--;
		}
	}
	else	//First time calibration mode entry
	{
		gAutoCalibration = true;	//enable calibration mode
		coin_calibration_index = (MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT) - 1;	//Keep it ready for next cycle
		//Reset all calibration buffers
		loIdx = 0;
		memset(loMin_Sens, 0, sizeof(loMin_Sens));
		memset(loMax_Sens, 0, sizeof(loMax_Sens));
		memset(loRatio1, 0, sizeof(loRatio1));
		memset(loRatio2, 0, sizeof(loRatio2));

		for(i=0; i<(MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT); i++)
		{
			memset(Coin_Calib_Params[i].params, 0, sizeof(Coin_Calib_Params[i].params));
			Coin_Calib_Params[i].value_units = 0;
		}
	}

	for(i=0; i<(MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT); i++)	//scan until a valid coin is found in config
	{
		if(coin_calibration_index < MAX_COINS_SUPPORTED)
		{
			if((glSystem_cfg.coin_parameters[coin_calibration_index].params[1]>0)&&(glSystem_cfg.coin_parameters[coin_calibration_index].params[3]>0)&&
			   (glSystem_cfg.coin_parameters[coin_calibration_index].params[5]>0)&&(glSystem_cfg.coin_parameters[coin_calibration_index].params[7]>0))
				 coin_value_to_show = glSystem_cfg.coin_parameters[coin_calibration_index].value_units;
			else coin_value_to_show = 0;
		}
		else
		{
			if((glSystem_cfg.multipart_coin_parameters[coin_calibration_index].expanded_params[1]>0)&&(glSystem_cfg.multipart_coin_parameters[coin_calibration_index].expanded_params[3]>0)&&
			   (glSystem_cfg.multipart_coin_parameters[coin_calibration_index].expanded_params[5]>0)&&(glSystem_cfg.multipart_coin_parameters[coin_calibration_index].expanded_params[7]>0))
				 coin_value_to_show = glSystem_cfg.multipart_coin_parameters[coin_calibration_index - MAX_COINS_SUPPORTED].value_units;
			else coin_value_to_show = 0;
		}
		if((coin_value_to_show == 0)||(coin_value_to_show == 0xFFFF))
		{
			coin_value_to_show = 0;
			coin_calibration_index--;
		}
		else
			break;
	}


	if(coin_value_to_show != 0)
	{
		Coin_Calibration_Screen(coin_value_to_show, (loIdx+1), NUMBER_OF_CAL_DROPS, coin_calibration_index);

		Calibration_TimeOut_RTC =  RTC_epoch_now();	//Refresh timer to avoid timeout
	}
	else	//Coin Calibration Complete, now write the new calibrated values
	{
		if(wrong_calibration_flag == TRUE)
		{
			Flash_Coin_Calibration_Params(READ_CALIBRATION);
			gAutoCalibration = false;
			//gSelfCalibration = false;
			wrong_calibration_flag = FALSE;
			Debug_TextOut(0,"Coins AutoCal FAILED");
			Clear_Screen();
			//diag_text_Screen( "Coins AutoCal FAILED", TRUE, FALSE);
			printSharp32x48("   COIN     ",25,0,0);//printSharp32x48(" THIS METER ",35,0,0);
			printSharp32x48("   AUTOCAL   ", 80,0,0);//printSharp32x48("   COVERS   ", 80,0,0);
			printSharp32x48("    FAILED       ", 135,0,0);//printSharp32x48(" TWO SPACES  ", 125,0,0);
			DelayMs(6000);
			DSM_Screen1();//Idle_Screen();
		}
		else
		{
			Flash_Coin_Calibration_Params(OVERWRITE_AUTO_CALIBRATION);
			gAutoCalibration = false;
			gSelfCalibration = false;
			Debug_TextOut(0,"Coins AutoCal SUCCESS");
			Clear_Screen();
			//diag_text_Screen("Coins AutoCal SUCCESS", TRUE, FALSE);
			printSharp32x48("   COIN     ",25,0,0);//printSharp32x48(" THIS METER ",35,0,0);
			printSharp32x48("   AUTOCAL   ", 80,0,0);//printSharp32x48("   COVERS   ", 80,0,0);
			printSharp32x48("    SUCCESS        ", 135,0,0);//printSharp32x48(" TWO SPACES  ", 125,0,0);
			DelayMs(6000);
			DSM_Screen1();//Idle_Screen();
		}

	}

	return;
}

/**************************************************************************/
//! Coin Self Calibration Routine - This is called in Self Calibration after
//! validating a coin successfully
//! \param uint8_t SCindex - coin index for which this coin matches
//! \return void
/**************************************************************************/
void Coin_Self_Calibration_Mode(uint8_t SCindex)
{
	uint8_t i = 0, Self_calib_complete = false;
	uint16_t value_for_self_cal = 0;
	uint16_t max_calib_tolerance = 0, min_calib_tolerance = 0;
	//uint8_t j = 0;

	if(Self_Calibration[SCindex].SC_counter < NUMBER_OF_CAL_DROPS)
	{
		Debug_Output1( 0, "Self Calibration# *** %d ***", ((Self_Calibration[SCindex].SC_counter)+1));
		Self_Calibration[SCindex].SCMin_Sens[Self_Calibration[SCindex].SC_counter] = min_sens;
		Self_Calibration[SCindex].SCMax_Sens[Self_Calibration[SCindex].SC_counter] = max_sens;
		Self_Calibration[SCindex].SCRatio1[Self_Calibration[SCindex].SC_counter] = ratio_1;
		Self_Calibration[SCindex].SCRatio2[Self_Calibration[SCindex].SC_counter] = ratio_2;
		Self_Calibration[SCindex].SC_counter++;
	}
	if((Self_Calibration[SCindex].SC_counter >= NUMBER_OF_CAL_DROPS) && (Self_Calibration[SCindex].SC_counter != 0xFF))	//0xFF now is marked for already precalibrated coins
	{
		//We'll self calibrate max and min only, ratio1, ratio2 will be copied as they are from config file
		sum_max = 0;
		sum_min = 0;
		sum_rat1 = 0;
		sum_rat2 = 0;
		averaged_max = 0;
		averaged_min = 0;
		averaged_rat1 = 0;
		averaged_rat2 = 0;

		for(i=0; i<NUMBER_OF_CAL_DROPS; i++)
		{
			sum_max += Self_Calibration[SCindex].SCMax_Sens[i];
			sum_min += Self_Calibration[SCindex].SCMin_Sens[i];
			sum_rat1 += Self_Calibration[SCindex].SCRatio1[i];
			sum_rat2 += Self_Calibration[SCindex].SCRatio2[i];
		}
		averaged_max = sum_max / NUMBER_OF_CAL_DROPS;
		averaged_min = sum_min / NUMBER_OF_CAL_DROPS;
		averaged_rat1 = sum_rat1 / NUMBER_OF_CAL_DROPS;
		averaged_rat2 = sum_rat2 / NUMBER_OF_CAL_DROPS;

		////SENDING AVERAGE VALUE THROUGH CD125 IF REQUIRED///////////////////////
		if((((glSystem_cfg.allowed_trans_types>>13) & 0x01) == 1) || (((glSystem_cfg.allowed_trans_types>>14) & 0x01) == 1))
					{
						{
							glCoindata[glCoin_Index].cashtranstime = RTC_epoch_now();
							glCoindata[glCoin_Index].coin_index = 2000+SCindex;	//self calibration
							glCoindata[glCoin_Index].coinval = 0;
							glCoindata[glCoin_Index].earned_coin_hr = 0;
							glCoindata[glCoin_Index].earned_coin_min = 0;
							//glCoindata[glCoin_Index].cumulative_parktime_hr = 0;
							//glCoindata[glCoin_Index].cumulative_parktime_min = 0;
							//to avoid time paid zero //vinay
							{
								glCoindata[glCoin_Index].cumulative_parktime_hr = (parking_time_left_on_meter[Current_Space_Id] / 3600);
								glCoindata[glCoin_Index].cumulative_parktime_min = ((parking_time_left_on_meter[Current_Space_Id] % 3600)/60);

								if(((parking_time_left_on_meter[Current_Space_Id] % 3600) % 60) > 0)	//This is to fix the wrong update of parking clock
								{
									glCoindata[glCoin_Index].cumulative_parktime_min += 1;
									if(glCoindata[glCoin_Index].cumulative_parktime_min > 59)
									{
										glCoindata[glCoin_Index].cumulative_parktime_hr += 1;
										glCoindata[glCoin_Index].cumulative_parktime_min = 0;
									}
								}
							}
							glCoindata[glCoin_Index].space_id = Current_Space_Id;
							glCoindata[glCoin_Index].max = (uint16_t)averaged_max;  //to send invalid coin profile
							glCoindata[glCoin_Index].min = (uint16_t)averaged_min;  //to send invalid coin profile
							glCoindata[glCoin_Index].rat1 = (uint8_t)averaged_rat1;  //to send invalid coin profile
							glCoindata[glCoin_Index].rat2 = (uint8_t)averaged_rat2;  //to send invalid coin profile
						}
						//Debug_Output6(0,"CT=%ld,CI=%d,CV=%d,E_hr=%ld,E_min=%ld,C_hr=%ld",glCoindata[glCoin_Index].cashtranstime,glCoindata[glCoin_Index].coin_index,glCoindata[glCoin_Index].coinval,glCoindata[glCoin_Index].earned_coin_hr,glCoindata[glCoin_Index].earned_coin_min,glCoindata[glCoin_Index].cumulative_parktime_hr);
						//Debug_Output6(0,"C_min=%ld,max=%ld,min=%ld,rat1=%ld,rat2=%d",glCoindata[glCoin_Index].cumulative_parktime_min,glCoindata[glCoin_Index].max,glCoindata[glCoin_Index].min,glCoindata[glCoin_Index].rat1,glCoindata[glCoin_Index].rat2,0);
						Last_Cash_Payment_RTC = RTC_epoch_now();
						glCoin_Index++;							//this will be incremented for all coins
						if(glCoin_Index >= MAX_COINS_TO_UDP)
							glCoin_Index = 0;
					}
		//////////////////////////////////////////////////////////////////////////
		Debug_Output6( 0, "avg_max:%d, avg_min:%d, avg_rat1:%d, avg_rat2:%d", averaged_max,	averaged_min, averaged_rat1, averaged_rat2, 0, 0);

		if(SCindex < MAX_COINS_SUPPORTED)
		{
			Coin_Calib_Params[SCindex].value_units = glSystem_cfg.coin_parameters[SCindex].value_units;

			//Set Ratio1 Copying from config
			Coin_Calib_Params[SCindex].params[4] = glSystem_cfg.coin_parameters[SCindex].params[4];
			Coin_Calib_Params[SCindex].params[5] = glSystem_cfg.coin_parameters[SCindex].params[5];

			//Set Ratio1 Copying from config
			Coin_Calib_Params[SCindex].params[6] = glSystem_cfg.coin_parameters[SCindex].params[6];
			Coin_Calib_Params[SCindex].params[7] = glSystem_cfg.coin_parameters[SCindex].params[7];

			max_calib_tolerance = glSystem_cfg.coin_calib_tol[SCindex].coin_max_tolerance*2;
			min_calib_tolerance = glSystem_cfg.coin_calib_tol[SCindex].coin_min_tolerance*2;
		}
		else
		{
			Coin_Calib_Params[SCindex].value_units = glSystem_cfg.multipart_coin_parameters[SCindex - MAX_COINS_SUPPORTED].value_units;
			//Set Ratio1 Copying from config
			Coin_Calib_Params[SCindex].params[4] = glSystem_cfg.multipart_coin_parameters[SCindex - MAX_COINS_SUPPORTED].narrow_params[4];
			Coin_Calib_Params[SCindex].params[5] = glSystem_cfg.multipart_coin_parameters[SCindex - MAX_COINS_SUPPORTED].narrow_params[5];

			//Set Ratio1 Copying from config
			Coin_Calib_Params[SCindex].params[6] = glSystem_cfg.multipart_coin_parameters[SCindex - MAX_COINS_SUPPORTED].narrow_params[6];
			Coin_Calib_Params[SCindex].params[7] = glSystem_cfg.multipart_coin_parameters[SCindex - MAX_COINS_SUPPORTED].narrow_params[7];

			max_calib_tolerance = glSystem_cfg.coin_calib_tol[SCindex].coin_max_tolerance*2;
			min_calib_tolerance = glSystem_cfg.coin_calib_tol[SCindex].coin_min_tolerance*2;
		}

		//Set Max Range
		if(averaged_max > max_calib_tolerance)
			Coin_Calib_Params[SCindex].params[0] = averaged_max - max_calib_tolerance;	//additional range
		else
			Coin_Calib_Params[SCindex].params[0] = 0;
		Coin_Calib_Params[SCindex].params[1] = averaged_max + max_calib_tolerance;	//additional range

		//Set Min Range
		if(averaged_min > min_calib_tolerance)
			Coin_Calib_Params[SCindex].params[2] = averaged_min - min_calib_tolerance;	//additional range
		else
			Coin_Calib_Params[SCindex].params[2] = 0;
		Coin_Calib_Params[SCindex].params[3] = averaged_min + min_calib_tolerance;	//additional range

		//We have done, print the values now
		Debug_Output6( 0, "Index:%d, Value:%d, Max:%d,%d, Min:%d,%d", SCindex, Coin_Calib_Params[SCindex].value_units,
				Coin_Calib_Params[SCindex].params[0], Coin_Calib_Params[SCindex].params[1],
				Coin_Calib_Params[SCindex].params[2], Coin_Calib_Params[SCindex].params[3]);
		Debug_Output6( 0, "Ratio1:%d,%d Ratio2:%d,%d", Coin_Calib_Params[SCindex].params[4], Coin_Calib_Params[SCindex].params[5],
				Coin_Calib_Params[SCindex].params[6], Coin_Calib_Params[SCindex].params[7], 0, 0);

		/*loIdx = 0;
			memset(loMin_Sens, 0, sizeof(loMin_Sens));
			memset(loMax_Sens, 0, sizeof(loMax_Sens));
			memset(loRatio1, 0, sizeof(loRatio1));
			memset(loRatio2, 0, sizeof(loRatio2));
			coin_calibration_index--;*/
	}

	//Scan to check if all valid coins are calibrated
	Self_calib_complete = true;
	for(i = 0; i < (MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT); i++)
	{
		if(i < MAX_COINS_SUPPORTED)
		{
			value_for_self_cal = glSystem_cfg.coin_parameters[i].value_units;
		}
		else
		{
			value_for_self_cal = glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].value_units;
		}
		if((Self_Calibration[i].SC_counter < NUMBER_OF_CAL_DROPS) && (value_for_self_cal != 0))	//A valid coin, but has not reached NUMBER_OF_CAL_DROPS
		{
			//Debug_Output2(0, "Calib miss: %d, %d", Self_Calibration[i].SC_counter, Coin_Calib_Params[SCindex].value_units);
			Self_calib_complete = false;//mark false for any single missing coin calibration
		}
	}
	/*if(Self_calib_complete == true)
		{
	for(j=0; j<NUMBER_OF_CAL_DROPS; j++)
											{
												if((Self_Calibration[SCindex].SCMax_Sens[j] >= Coin_Calib_Params[SCindex].params[0] && Self_Calibration[SCindex].SCMax_Sens[j] <= Coin_Calib_Params[SCindex].params[1]) &&
																	(Self_Calibration[SCindex].SCMin_Sens[j] >= Coin_Calib_Params[SCindex].params[2] && Self_Calibration[SCindex].SCMin_Sens[j] <= Coin_Calib_Params[SCindex].params[3]) &&
																	(Self_Calibration[SCindex].SCRatio1[j] >= Coin_Calib_Params[SCindex].params[4] && Self_Calibration[SCindex].SCRatio1[j] <= Coin_Calib_Params[SCindex].params[5]) &&
																	(Self_Calibration[SCindex].SCRatio2[j] >= Coin_Calib_Params[SCindex].params[6] && Self_Calibration[SCindex].SCRatio2[j] <= Coin_Calib_Params[SCindex].params[7]))
													{
														Debug_Output2(0,"coin %d index %d fall in current self average range",i,SCindex);
														Self_calib_complete = TRUE;
													}
												else
													{
														Debug_Output2(0,"coin %d index %d not fall in current self average range",i,SCindex);
														Self_calib_complete = FALSE;
														Self_Calibration[SCindex].SC_counter = 0;
														Flash_Coin_Calibration_Params(READ_CALIBRATION);
														Idle_Screen();
													}

											}
		}*/
	if(Self_calib_complete == true)
	{
		//Coin Calibration Complete, now write the new calibrated values
		Flash_Coin_Calibration_Params(OVERWRITE_SELF_CALIBRATION);
		gAutoCalibration = false;
		gSelfCalibration = false;
		diag_text_Screen( "Coins Self Calibration Complete", FALSE, FALSE);

		Idle_Screen();
	}

	return;
}

/**************************************************************************/
//! Post Processing of a validated coin. includes earned parking time
//! calculation and coin transaction structure generation
//! \param coin_val_amount - detected coin value
//! \param self_cal_index - matches with which coin and can be calibrated
//! \param self_cal_eligible - flag to say if the current coin is eligible
//! to be used to self calibrate the meter
//! \return void
/**************************************************************************/
void ProcessValidatedCoin(uint16_t coin_val_amount, uint8_t self_cal_index, uint8_t	self_cal_eligible)
{
	uint32_t pre_existing_seconds = 0;
	uint32_t temp_parking_clock_for_display[MSM_MAX_PARKING_BAYS] = {0};
	uint8_t show_meter_full[MSM_MAX_PARKING_BAYS] = {0};
	min_time_insec = glSystem_cfg.min_time_to_display * 60;
	uint8_t	i	=	0;

	if((grace_time_trigger[Current_Space_Id] == true) && (glSystem_cfg.grace_time_type != 0)) // For Grace time feature
	{
		parking_time_left_on_meter[Current_Space_Id] = 0;
		negetive_parking_time_left_on_meter[Current_Space_Id] = 0;
		grace_time_trigger[Current_Space_Id] = false;
	}

	pre_existing_seconds = parking_time_left_on_meter[Current_Space_Id];

	//Debug_Output2(0, "Before parking_time_left_on_meter[%d] = %ld", Current_Space_Id, parking_time_left_on_meter[Current_Space_Id]);

	//Debug_Output1(0, "Before coin_val_amount %d", coin_val_amount);

	coin_insert_ok = TRUE;
	temp_parking_clock_for_display[Current_Space_Id] = get_earned_seconds(coin_val_amount, true, true);
	coin_insert_ok = FALSE;

	//Debug_Output2(0, "After parking_time_left_on_meter[%d] = %ld", Current_Space_Id, parking_time_left_on_meter[Current_Space_Id]);


	if(temp_parking_clock_for_display[Current_Space_Id] > pre_existing_seconds) //LNGSIT-654
	{
		earned_time_in_seconds = temp_parking_clock_for_display[Current_Space_Id] - pre_existing_seconds;
	}
	else
	{
		earned_time_in_seconds = 0;
		////if(temp_parking_clock_for_display > 0) time_retain[Current_Space_Id] = 1;
	}

	parking_time_left_on_meter[Current_Space_Id] = temp_parking_clock_for_display[Current_Space_Id];

	if(time_retain[Current_Space_Id] == 0)
			{
				if((parking_time_left_on_meter_prev[Current_Space_Id] < min_time_insec)&&(min_time_insec != 0))//((minutes_to_display < 15)&&(min_amount != 0)) //vinay to check min charges to start parking clock for Trenton customer
					{
						time_retain[Current_Space_Id] = 0;
						////parking_time_left_on_meter[Current_Space_Id] = parking_time_left_on_meter_prev[Current_Space_Id] + earned_time_in_seconds;
						////parking_time_left_on_meter_prev[Current_Space_Id] = parking_time_left_on_meter[Current_Space_Id];
						parking_time_left_on_meter_prev[Current_Space_Id] += earned_time_in_seconds;
						////Debug_Output1(0,"111 parking_time_left_on_meter[Current_Space_Id]=%ld",parking_time_left_on_meter[Current_Space_Id]);
						////Debug_Output1(0,"111 parking_time_left_on_meter_prev[Current_Space_Id]=%ld",parking_time_left_on_meter_prev[Current_Space_Id]);
						////Debug_Output1(0,"111 time_retain[Current_Space_Id]=%d",time_retain[Current_Space_Id]);
						////Debug_Output1(0,"111 [Current_Space_Id]=%d",Current_Space_Id);

					}
			    if((parking_time_left_on_meter_prev[Current_Space_Id] >= min_time_insec)&&(min_time_insec != 0))//((minutes_to_display >= 15)&&(min_amount != 0)) //vinay to check min charges to start parking clock for Trenton customer
					{
						time_retain[Current_Space_Id] = 1;
						////parking_time_left_on_meter_prev[Current_Space_Id] = 0;
						////Debug_Output1(0,"000 parking_time_left_on_meter[Current_Space_Id]=%ld",parking_time_left_on_meter[Current_Space_Id]);
						////Debug_Output1(0,"000 parking_time_left_on_meter_prev[Current_Space_Id]=%ld",parking_time_left_on_meter_prev[Current_Space_Id]);
						////Debug_Output1(0,"000 time_retain[Current_Space_Id]=%d",time_retain[Current_Space_Id]);
						////Debug_Output1(0,"000 [Current_Space_Id]=%d",Current_Space_Id);

					}
			}
	else if(time_retain[Current_Space_Id] == 1)
			{
				if(parking_time_left_on_meter[Current_Space_Id] != 0)//((minutes_to_display >= 15)&&(min_amount != 0)) //vinay to check min charges to start parking clock for Trenton customer
					{
						time_retain[Current_Space_Id] = 1;
					}
				else if(parking_time_left_on_meter[Current_Space_Id] == 0)
					{
						time_retain[Current_Space_Id] = 0;
						parking_time_left_on_meter_prev[Current_Space_Id] = 0;
					}
			}

	in_prepay_parking[Current_Space_Id] = FALSE;

	if(glSystem_cfg.santa_cruz_spl_feature_enable == TRUE)
		santa_cruz_previous_paid_time += earned_time_in_seconds;

	if(parking_time_left_on_meter[Current_Space_Id] > 0)
	{
		Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_RESET_MODE;	//Avoid spending time in parking clock display here, it will be done outside
		Tasks_Priority_Register |= INTERVALS_ISR;	//LNGSIT-381: faster parking clock update for coin payments
	}
	glCoindata[glCoin_Index].cashtranstime = RTC_epoch_now();
	glCoindata[glCoin_Index].coin_index = glCoin_Index;
	glCoindata[glCoin_Index].coinval = coin_val_amount;
	glCoindata[glCoin_Index].earned_coin_hr = (earned_time_in_seconds / 3600);
	glCoindata[glCoin_Index].earned_coin_min = ((earned_time_in_seconds % 3600)/60);
	glCoindata[glCoin_Index].space_id = Current_Space_Id;

	if(((earned_time_in_seconds % 3600) % 60) > 0)	//This is to fix the wrong update of parking clock
	{
		glCoindata[glCoin_Index].earned_coin_min += 1;
		if(glCoindata[glCoin_Index].earned_coin_min > 59)
		{
			glCoindata[glCoin_Index].earned_coin_hr += 1;
			glCoindata[glCoin_Index].earned_coin_min = 0;
		}
	}

	glCoindata[glCoin_Index].cumulative_parktime_hr = (parking_time_left_on_meter[Current_Space_Id] / 3600);
	glCoindata[glCoin_Index].cumulative_parktime_min = ((parking_time_left_on_meter[Current_Space_Id] % 3600)/60);

	if(((parking_time_left_on_meter[Current_Space_Id] % 3600) % 60) > 0)	//This is to fix the wrong update of parking clock
	{
		glCoindata[glCoin_Index].cumulative_parktime_min += 1;
		if(glCoindata[glCoin_Index].cumulative_parktime_min > 59)
		{
			glCoindata[glCoin_Index].cumulative_parktime_hr += 1;
			glCoindata[glCoin_Index].cumulative_parktime_min = 0;
		}
	}
	//if(((glSystem_cfg.allowed_trans_types>>13) & 0x01) == 1) //write only if set in config for valid coins
	{
		//Debug_TextOut(0,"Regular+maxmin");
		glCoindata[glCoin_Index].max = max_sens;  //to send invalid coin profile
		glCoindata[glCoin_Index].min = min_sens;  //to send invalid coin profile
		glCoindata[glCoin_Index].rat1 = ratio_1;  //to send invalid coin profile
		glCoindata[glCoin_Index].rat2 = ratio_2;  //to send invalid coin profile
	}

	Last_Cash_Payment_RTC = RTC_epoch_now();
	glCoin_Index++;
	if(glCoin_Index >= MAX_COINS_TO_UDP)
		glCoin_Index = 0;

	glTotal_Valid_Coin_Count++;	//for diagnostics and audit
	//Debug_Output1(0,"Coin 2351 glTotal_Valid_Coin_Count = %d",glTotal_Valid_Coin_Count);
	//glTotal_Coin_Count++;
	LCD_Backlight(TRUE);
	if((gSelfCalibration == true) && (self_cal_eligible == true))
		Coin_Self_Calibration_Mode(self_cal_index);
	//TODO:comment out the below code after testing, increment appropriate coin counts after validation
	//Debug_Output6( 0, "Idx: %d, Space: %d, It's a %dC Coin & earned %d minutes\r\n",self_cal_index, (glCoindata[glCoin_Index-1].space_id+1),coin_val_amount, (earned_time_in_seconds/60), 0, 0);
	Debug_Output6( 0, "Idx: %d, Space: %d, It's a %dC Coin & earned %d minutes\r\n",self_cal_index, (glCoindata[glCoin_Index-1].space_id),coin_val_amount, (earned_time_in_seconds/60), 0, 0);
	coindrop_TimeOut_RTC =  RTC_epoch_now();
	//Debug_Output1( 0, "2coindrop_TimeOut_RTC: %d\r\n",coindrop_TimeOut_RTC);

	//**********************Coin audit **************************
	//This is only needed for COIN ONLY customers
	if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))
	{
		if(coin_val_amount>0)
			{
				temp_coin_type[self_cal_index] += 1;
			}

		////Added all coins details to Audit registry (total 20 coins data will be saved)
		////Debug_Output1(0,"temp_coin_type = %d",temp_coin_type[self_cal_index]);

		/*switch(coin_val_amount)
		{
		case USA_NICKEL_COIN : temp_coin_type0 += 1;
		break;

		case USA_DIME_COIN: temp_coin_type1 += 1;
		break;

		case USA_QUARTER_COIN : temp_coin_type2 += 1;
		break;

		case USA_DOLLAR_COIN : temp_coin_type3 += 1;
		break;

		default:
			if(coin_val_amount>0) temp_coin_type_common += 1; //// Mexican peso or other coin other than US coins in config VT
		break;
		}*/
		temp_total_coin_values += coin_val_amount;
		////Debug_Output1(0,"temp_total_coin_values = %d",temp_total_coin_values);
		glTotal_Valid_Coin_Count = 0;
		//Debug_Output1(0,"Coin 2395 glTotal_Valid_Coin_Count = %d",glTotal_Valid_Coin_Count);
		for(i=0;i<20;i++)
			{
				glTotal_Valid_Coin_Count += (temp_coin_type[i] + coin_type[i]);
				//Debug_Output1(0,"Coin 2399 glTotal_Valid_Coin_Count = %d",glTotal_Valid_Coin_Count);
	    		//Debug_Output2(0, "coin temp_coin_type[%d]: %d",i, temp_coin_type[i]);
	    		//Debug_Output2(0, "coin coin_type[%d]: %d",i, coin_type[i]);
			}
		//Debug_Output1(0,"Coin 2403 glTotal_Valid_Coin_Count = %d",glTotal_Valid_Coin_Count);
		/*glTotal_Valid_Coin_Count = temp_coin_type[] + temp_coin_type1 + temp_coin_type2 + temp_coin_type3 + temp_coin_type_common +
								   coin_type0 + coin_type1 + coin_type2 + coin_type3;*/
	}
	//***********************************************************
	if(coin_val_amount == USA_NICKEL_COIN)
	{
		glTotal_5_cents++;
	}
	else if(coin_val_amount == USA_DIME_COIN)
	{
		glTotal_10_cents++;
	}
	else if(coin_val_amount == USA_QUARTER_COIN)
	{
		glTotal_25_cents++;
	}
	else if(coin_val_amount == USA_DOLLAR_COIN)
	{
		glTotal_100_cents++;
	}
	/**************METER FULL****************************/
	//Debug_Output2(0,"current = %ld next = %ld",minutes_left_in_current_schedule,minutes_left_in_next_schedule);
	//Debug_Output2(0,"current+next = %ld max_time_in_current_rate=%ld",((minutes_left_in_current_schedule+minutes_left_in_next_schedule)*60),max_time_in_current_rate);
	if((((minutes_left_in_current_schedule+minutes_left_in_next_schedule)*60) >= max_time_in_current_rate)||(current_rate_index == PREPAY_PARKING))
		show_meter_full[Current_Space_Id] = 1;
	else
		show_meter_full[Current_Space_Id] = 0; //for showing max time paid in coin drop // VT

	if((((parking_time_left_on_meter[Current_Space_Id] >= (max_time_in_current_rate - 60))&&(show_meter_full[Current_Space_Id]==1))||
			((parking_time_left_on_meter[Current_Space_Id] >= (((minutes_left_in_current_schedule+minutes_left_in_next_schedule)*60) - 60))&&(show_meter_full[Current_Space_Id]==0)))
			&& (glSystem_cfg.santa_cruz_spl_feature_enable == FALSE) )//&& (time_retain[Current_Space_Id] == 1))//sometimes maxtime is not shown or first time max time paid msg is not shown, to show msg commented timeretain//vinay
	{
		Debug_TextOut(0,"Meter Full");
		MeterFull_Screen();
		Meter_Full_Flg[Current_Space_Id] = TRUE;
		return;
	}
	/***************************************************/
	//get the current RTC time into RTCTime structure
	//year_from_RTC(glCoindata[glCoin_Index].cashtranstime, &RTCTime);
	//SMS_Print = TRUE;
	KeyPress_TimeOut_RTC = RTC_epoch_now();
	special_key_count = 0;
	DSM_Screen2();

	if(earned_time_in_seconds > 0)
	{
		//Reset SmartCard Refund Data after successful Coin payment
		Refund_PreviousSN = 0;
		Refund_PresentSN = 0;
		Refund_timestamp = 0;
		Refund_earned_mins = 0;
		RefundOK = 0;
	}

	return;
}

/**************************************************************************/
//! Validates the dropped coin with the configuration file. Index 0-2 are used
//! for expanded ranges along with Optical LED check, index 3-19 are used for
//! regular ranges and multipart configuration are used for overlapping coins
//! to use a narrow and expanded range
//! \param void
//! \return void
/**************************************************************************/
void CoinOk(void)
{
//	CoinSpecsType_New loCoinParam; //made global to use it in rates.c for Thurmont //vinay
	CoinSpecs_Multipart lomultipartCoinParam;
	uint8_t loCoinIdx = 0;

	uint8_t max_narrow = false,	min_narrow = false,	rat1_narrow = false, rat2_narrow = false,
			max_expanded = false, min_expanded = false, rat1_expanded = false, rat2_expanded = false, coin_narrow_count = 0;

	//Debug_Output1( 0, "1payment_on_hold = %d\n",payment_on_hold);

	last_payment_try_time[Current_bayStatus] = RTC_epoch_now();
	//if(glLast_reported_space_status[Current_Space_Id] == 0)
	if((glSystem_cfg.vehicle_sensor_enable == TRUE)&&((glLast_reported_space_status[Current_Space_Id] == 0) && ((current_rate_index < 20) || (current_rate_index == PREPAY_PARKING))) )
	{
		ZERO_OUT_ENABLE_AT_EXPIRY[Current_Space_Id] = TRUE;
		ZERO_OUT[Current_Space_Id] = FALSE;
		//Debug_TextOut(0,"PA on Vac, Won't ZO till next 801");
		Debug_Output1(0,"PA on Vac, CO No ZO till next 801 for space [%d]",Current_Space_Id);
	}

	if((coin_validation_number & 31) != 0)
	{
		Debug_TextOut( 0, "Coin Not OK");
		return;
	}

	if(meter_out_of_service == FALSE) //LNGSIT-590
	{
		//Index 0, 1 & 2 are now reserved for expanded 25C and Opto logic will be applied on this
		//so start normal comparison from 3
		for(loCoinIdx = NORMAL_COIN_START_INDEX; loCoinIdx <= MAX_COINS_SUPPORTED; loCoinIdx++)
		{
			//loCoinParam.coin_number = loCoinIdx;
			if(GetCoinParameters(&loCoinParam, loCoinIdx) == false)
			{
				;
			}

			if(gSelfCalibration == true)	//Give an additional range for self calibration
			{
				//Max tolerance
				/*if(loCoinParam.params[0] > (glSystem_cfg.coin_calib_tol[loCoinIdx].coin_max_tolerance*2 + glSystem_cfg.operational_parameters.add_max_tolerance_selfcal))
					loCoinParam.params[0] -= (glSystem_cfg.coin_calib_tol[loCoinIdx].coin_max_tolerance*2 + glSystem_cfg.operational_parameters.add_max_tolerance_selfcal);
				else
					loCoinParam.params[0] = 0;
				loCoinParam.params[1] += (glSystem_cfg.coin_calib_tol[loCoinIdx].coin_max_tolerance*2 + glSystem_cfg.operational_parameters.add_max_tolerance_selfcal);

				//Min tolerance
				if(loCoinParam.params[2] > (glSystem_cfg.coin_calib_tol[loCoinIdx].coin_min_tolerance*2 + glSystem_cfg.operational_parameters.add_min_tolerance_selfcal))
					loCoinParam.params[2] -= (glSystem_cfg.coin_calib_tol[loCoinIdx].coin_min_tolerance*2 + glSystem_cfg.operational_parameters.add_min_tolerance_selfcal);
				else
					loCoinParam.params[2] = 0;
				loCoinParam.params[3] += (glSystem_cfg.coin_calib_tol[loCoinIdx].coin_min_tolerance*2 + glSystem_cfg.operational_parameters.add_min_tolerance_selfcal);

				//Ratio1 tolerance
				if(loCoinParam.params[4] > (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal))
					loCoinParam.params[4] -= (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal);
				else
					loCoinParam.params[4] = 0;
				loCoinParam.params[5] += (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal);

				//Ratio2 tolerance
				if(loCoinParam.params[6] > (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal))
					loCoinParam.params[6] -= (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal);
				else
					loCoinParam.params[6] = 0;
				loCoinParam.params[7] += (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal);*/
			}

			if((max_sens >= loCoinParam.params[0] && max_sens <= loCoinParam.params[1]) &&
					(min_sens >= loCoinParam.params[2] && min_sens <= loCoinParam.params[3]) &&
					(ratio_1 >= loCoinParam.params[4] && ratio_1 <= loCoinParam.params[5]) &&
					(ratio_2 >= loCoinParam.params[6] && ratio_2 <= loCoinParam.params[7]))
			{
				ProcessValidatedCoin(loCoinParam.value_units, loCoinIdx, true);
				return;
			}
		}

		//If the code reached here, then none of the normal coin has been detected
		//so try expanded range coins if it is there in config
		//New location coin processing using multipart config
		for(loCoinIdx = 0; loCoinIdx <= MULTIPART_COINS_COUNT; loCoinIdx++)
		{
			max_narrow = false;
			min_narrow = false;
			rat1_narrow = false;
			rat2_narrow = false;

			max_expanded = false;
			min_expanded = false;
			rat1_expanded = false;
			rat2_expanded = false;
			coin_narrow_count = 0;

			if(GetMultipartCoinParameters(&lomultipartCoinParam, loCoinIdx) == false)
			{
				;
			}

			//Only the narrow range is part of self calibration, expanded range will be taken from config only
			if(gSelfCalibration == true)	//Give an additional range for self calibration
			{
				//Do not use a high tolerance as normal coins for multipart coins because they are overlapping with other coins
				//Max tolerance
				/*if(lomultipartCoinParam.narrow_params[0] > (glSystem_cfg.operational_parameters.add_max_tolerance_selfcal))
					lomultipartCoinParam.narrow_params[0] -= (glSystem_cfg.operational_parameters.add_max_tolerance_selfcal);
				else
					lomultipartCoinParam.narrow_params[0] = 0;
				lomultipartCoinParam.narrow_params[1] += (glSystem_cfg.operational_parameters.add_max_tolerance_selfcal);

				//Min tolerance
				if(lomultipartCoinParam.narrow_params[2] > (glSystem_cfg.operational_parameters.add_min_tolerance_selfcal))
					lomultipartCoinParam.narrow_params[2] -= (glSystem_cfg.operational_parameters.add_min_tolerance_selfcal);
				else
					lomultipartCoinParam.narrow_params[2] = 0;
				lomultipartCoinParam.narrow_params[3] += (glSystem_cfg.operational_parameters.add_min_tolerance_selfcal);

				//Ratio1 tolerance
				if(lomultipartCoinParam.narrow_params[4] > (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal))
					lomultipartCoinParam.narrow_params[4] -= (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal);
				else
					lomultipartCoinParam.narrow_params[4] = 0;
				lomultipartCoinParam.narrow_params[5] += (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal);

				//Ratio2 tolerance
				if(lomultipartCoinParam.narrow_params[6] > (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal))
					lomultipartCoinParam.narrow_params[6] -= (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal);
				else
					lomultipartCoinParam.narrow_params[6] = 0;
				lomultipartCoinParam.narrow_params[7] += (glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal);*/
			}

			//Do not use any else condition for all these tests
			//Check for the narrow range
			if(max_sens >= lomultipartCoinParam.narrow_params[0] && max_sens <= lomultipartCoinParam.narrow_params[1])
			{
				max_narrow = true;
				coin_narrow_count++;
			}
			if(min_sens >= lomultipartCoinParam.narrow_params[2] && min_sens <= lomultipartCoinParam.narrow_params[3])
			{
				min_narrow = true;
				coin_narrow_count++;
			}
			if(ratio_1 >= lomultipartCoinParam.narrow_params[4] && ratio_1 <= lomultipartCoinParam.narrow_params[5])
			{
				rat1_narrow = true;
				coin_narrow_count++;
			}
			if(ratio_2 >= lomultipartCoinParam.narrow_params[6] && ratio_2 <= lomultipartCoinParam.narrow_params[7])
			{
				rat2_narrow = true;
				coin_narrow_count++;
			}
			//Debug_Output6(0, "multipart narrow: %d,%d,%d,%d, total narrow: %d", max_narrow, min_narrow, rat1_narrow, rat2_narrow, coin_narrow_count, 0);

			//Check for expanded range
			if(max_sens >= lomultipartCoinParam.expanded_params[0] && max_sens <= lomultipartCoinParam.expanded_params[1])
			{
				max_expanded = true;
			}
			if(min_sens >= lomultipartCoinParam.expanded_params[2] && min_sens <= lomultipartCoinParam.expanded_params[3])
			{
				min_expanded = true;
			}
			if(ratio_1 >= lomultipartCoinParam.expanded_params[4] && ratio_1 <= lomultipartCoinParam.expanded_params[5])
			{
				rat1_expanded = true;
			}
			if(ratio_2 >= lomultipartCoinParam.expanded_params[6] && ratio_2 <= lomultipartCoinParam.expanded_params[7])
			{
				rat2_expanded = true;
			}
			//Debug_Output6(0, "multipart expanded: %d,%d,%d,%d", max_expanded, min_expanded, rat1_expanded, rat2_expanded, 0, 0);

			//Now take a decision, if at least configured number of the narrow bands pass and others pass in wide range, it is OK
			//Pass the case where all 4 tests pass through the narrow range
			//Do not pass the case where all 4 tests pass through the expanded range
			if(coin_narrow_count >= lomultipartCoinParam.min_no_of_narrow_to_pass)	//all other cases will get covered below
			{
				if(((max_narrow == true) && (min_narrow == true) && (rat1_narrow == true) && (rat2_narrow == true)) ||				//If all narrow ranges are satisfied (a,b,c,d) OR
						((max_narrow == true) && (min_expanded == true) && (rat1_expanded == true) && (rat2_expanded == true)) ||	//If max narrow and other 3 expanded ranges are satisfied (a,B,C,D) OR
						((max_expanded == true) && (min_narrow == true) && (rat1_expanded == true) && (rat2_expanded == true)) ||	//If min narrow and other 3 expanded ranges are satisfied (A,b,C,D) OR
						((max_expanded == true) && (min_expanded == true) && (rat1_narrow == true) && (rat2_expanded == true)) ||	//If rat1 narrow and other 3 expanded ranges are satisfied (A,B,c,D) OR
						((max_expanded == true) && (min_expanded == true) && (rat1_expanded == true) && (rat2_narrow == true)))		//If rat2 narrow and other 3 expanded ranges are satisfied (A,B,C,d)

				{
					if(coin_narrow_count > 2)	//eligible for self calibration only if the satisfies more than 2 narrow ranges
						ProcessValidatedCoin(lomultipartCoinParam.value_units, (loCoinIdx + MAX_COINS_SUPPORTED), true);
					else
						ProcessValidatedCoin(lomultipartCoinParam.value_units, (loCoinIdx + MAX_COINS_SUPPORTED), false);
					return;
				}
			}
		}

		//Index 0, 1 & 2 are now reserved for expanded 25C and Opto logic will be applied on this
		//so do expanded comparison from 0 to 2
		for(loCoinIdx = 0; loCoinIdx <= NORMAL_COIN_START_INDEX; loCoinIdx++)
		{
			if(GetCoinParameters(&loCoinParam, loCoinIdx) == false)
			{
				;
			}

			if((max_sens >= loCoinParam.params[0] && max_sens <= loCoinParam.params[1]) &&
					(min_sens >= loCoinParam.params[2] && min_sens <= loCoinParam.params[3]) &&
					(ratio_1 >= loCoinParam.params[4] && ratio_1 <= loCoinParam.params[5]) &&
					(ratio_2 >= loCoinParam.params[6] && ratio_2 <= loCoinParam.params[7]))
			{
				if(Optical_LED_touched == FALSE)	// If it is detected as 25C, but has not touched the optical LED, then it may be a penny or dime
				{
					Debug_TextOut(0,"Penny/Dime detected as Quarter, rejecting it");
					if(((glSystem_cfg.allowed_trans_types>>14) & 0x01) == 1) //for invalid coins only
					{
						//invalid coins value backup
						{
							glCoindata[glCoin_Index].cashtranstime = RTC_epoch_now();
							glCoindata[glCoin_Index].coin_index = 0;
							glCoindata[glCoin_Index].coinval = 0;
							glCoindata[glCoin_Index].earned_coin_hr = 0;
							glCoindata[glCoin_Index].earned_coin_min = 0;
							//glCoindata[glCoin_Index].cumulative_parktime_hr = 0;
							//glCoindata[glCoin_Index].cumulative_parktime_min = 0;
							//to avoid time paid zero //vinay
							{
								glCoindata[glCoin_Index].cumulative_parktime_hr = (parking_time_left_on_meter[Current_Space_Id] / 3600);
								glCoindata[glCoin_Index].cumulative_parktime_min = ((parking_time_left_on_meter[Current_Space_Id] % 3600)/60);

								if(((parking_time_left_on_meter[Current_Space_Id] % 3600) % 60) > 0)	//This is to fix the wrong update of parking clock
								{
									glCoindata[glCoin_Index].cumulative_parktime_min += 1;
									if(glCoindata[glCoin_Index].cumulative_parktime_min > 59)
									{
										glCoindata[glCoin_Index].cumulative_parktime_hr += 1;
										glCoindata[glCoin_Index].cumulative_parktime_min = 0;
									}
								}
							}
							glCoindata[glCoin_Index].space_id = Current_Space_Id;
							glCoindata[glCoin_Index].max = max_sens;  //to send invalid coin profile
							glCoindata[glCoin_Index].min = min_sens;  //to send invalid coin profile
							glCoindata[glCoin_Index].rat1 = ratio_1;  //to send invalid coin profile
							glCoindata[glCoin_Index].rat2 = ratio_2;  //to send invalid coin profile
						}
						//Debug_Output6(0,"CT=%ld,CI=%d,CV=%d,E_hr=%ld,E_min=%ld,C_hr=%ld",glCoindata[glCoin_Index].cashtranstime,glCoindata[glCoin_Index].coin_index,glCoindata[glCoin_Index].coinval,glCoindata[glCoin_Index].earned_coin_hr,glCoindata[glCoin_Index].earned_coin_min,glCoindata[glCoin_Index].cumulative_parktime_hr);
						//Debug_Output6(0,"C_min=%ld,max=%ld,min=%ld,rat1=%ld,rat2=%d",glCoindata[glCoin_Index].cumulative_parktime_min,glCoindata[glCoin_Index].max,glCoindata[glCoin_Index].min,glCoindata[glCoin_Index].rat1,glCoindata[glCoin_Index].rat2,0);
						Last_Cash_Payment_RTC = RTC_epoch_now();
						glCoin_Index++;							//this will be incremented for all coins
						if(glCoin_Index >= MAX_COINS_TO_UDP)
							glCoin_Index = 0;
						if(payment_on_hold == true)
							DSM_Screen2();
						//else
							//DSM_Screen1();
					}
				}
				else
				{
					ProcessValidatedCoin(loCoinParam.value_units, loCoinIdx, false);	//expanded range coins are not eligible for self calibration
				}
				return;
			}
		}
	}

	if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))
	{
		temp_coin_type_invalid++; //for coin only audit
		//Debug_Output1( 0, "temp_coin_type_invalid = %d\n",temp_coin_type_invalid);
	}
	glTotal_0_cents++;	//for diagnostics and audit
	//No more Missing Coins event, this was causing config erase
	//push_event_to_cache(UDP_EVTTYP_COIN_CALIBRATION_VALUES); //Used to send missing coin calibration values
	//Tasks_Priority_Register |= SEVERE_EVENTS_TASK;
	Failed_Coins_diag[failed_coin_counter].Cmax_val  = max_sens/10;
	Failed_Coins_diag[failed_coin_counter].Cmin_val  = min_sens/10;
	Failed_Coins_diag[failed_coin_counter].Crat1_val = ratio_1;
	Failed_Coins_diag[failed_coin_counter].Crat2_val = ratio_2;
	if(failed_coin_counter++ >= MAX_FAILED_COINS_DIAG)
		failed_coin_counter = 0;
	Debug_TextOut( 0, "#### Not a Valid coin  ####");    // for invalid coins vinay
	coindrop_TimeOut_RTC = 0;   //for invalid coins the screen was going blank to avoid this in idle screen //vinay
	//Debug_Output1( 0, "payment_on_hold = %d\n",payment_on_hold);
	if(payment_on_hold == true)
	{
		DSM_Screen2();
	}
	//else
		//DSM_Screen1();
	if(((glSystem_cfg.allowed_trans_types>>14) & 0x01) == 1) //for invalid coins only
	{
		//invalid coins value backup
		{
			glCoindata[glCoin_Index].cashtranstime = RTC_epoch_now();
			glCoindata[glCoin_Index].coin_index = 0;
			glCoindata[glCoin_Index].coinval = 0;
			glCoindata[glCoin_Index].earned_coin_hr = 0;
			glCoindata[glCoin_Index].earned_coin_min = 0;
			//glCoindata[glCoin_Index].cumulative_parktime_hr = 0;
			//glCoindata[glCoin_Index].cumulative_parktime_min = 0;
			//to avoid time paid zero //vinay
			{
				glCoindata[glCoin_Index].cumulative_parktime_hr = (parking_time_left_on_meter[Current_Space_Id] / 3600);
				glCoindata[glCoin_Index].cumulative_parktime_min = ((parking_time_left_on_meter[Current_Space_Id] % 3600)/60);

				if(((parking_time_left_on_meter[Current_Space_Id] % 3600) % 60) > 0)	//This is to fix the wrong update of parking clock
				{
					glCoindata[glCoin_Index].cumulative_parktime_min += 1;
					if(glCoindata[glCoin_Index].cumulative_parktime_min > 59)
					{
						glCoindata[glCoin_Index].cumulative_parktime_hr += 1;
						glCoindata[glCoin_Index].cumulative_parktime_min = 0;
					}
				}
			}
			glCoindata[glCoin_Index].space_id = Current_Space_Id;
			glCoindata[glCoin_Index].max = max_sens;  //to send invalid coin profile
			glCoindata[glCoin_Index].min = min_sens;  //to send invalid coin profile
			glCoindata[glCoin_Index].rat1 = ratio_1;  //to send invalid coin profile
			glCoindata[glCoin_Index].rat2 = ratio_2;  //to send invalid coin profile
		}
		//Debug_Output6(0,"CT=%ld,CI=%d,CV=%d,E_hr=%ld,E_min=%ld,C_hr=%ld",glCoindata[glCoin_Index].cashtranstime,glCoindata[glCoin_Index].coin_index,glCoindata[glCoin_Index].coinval,glCoindata[glCoin_Index].earned_coin_hr,glCoindata[glCoin_Index].earned_coin_min,glCoindata[glCoin_Index].cumulative_parktime_hr);
		//Debug_Output6(0,"C_min=%ld,max=%ld,min=%ld,rat1=%ld,rat2=%d",glCoindata[glCoin_Index].cumulative_parktime_min,glCoindata[glCoin_Index].max,glCoindata[glCoin_Index].min,glCoindata[glCoin_Index].rat1,glCoindata[glCoin_Index].rat2,0);
		Last_Cash_Payment_RTC = RTC_epoch_now();
		glCoin_Index++;							//this will be incremented for all coins
		if(glCoin_Index >= MAX_COINS_TO_UDP)
			glCoin_Index = 0;
	}

	return;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

