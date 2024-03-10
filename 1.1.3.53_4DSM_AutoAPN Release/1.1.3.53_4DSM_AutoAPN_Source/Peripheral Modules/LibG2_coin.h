/*
 * LibG2_coin.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef PERIPHERAL_MODULES_LIBG2_COIN_H_
#define PERIPHERAL_MODULES_LIBG2_COIN_H_

#include "../Main Module/LibG2_main.h"

#define USA_NICKEL_COIN						5
#define USA_DIME_COIN						10
#define USA_QUARTER_COIN					25
#define USA_DOLLAR_COIN						100

//#define WAKE_RESET_VAL			0x02C0 //0x2C0 //232   	//MB ROM magic number

//Default CCF parameters
#define SENSOR_SETTLE_TIMEOUT				1//10			//(in CCF this value is 10 * 0.5sec)
//#define DEFAULT_SENSOR_TIMEOUT				0x1770 	    //1.5sec
#define SENSOR_NUMBER_OF_READS				600 //600//600
//#define SENSOR_NUMBER_OF_READS_HF			(SENSOR_NUMBER_OF_READS/2)	//Number of sensor samples with Chirp  //not used //vinay
//#define SENSOR_READ_DELAY					1//500   	//Delay between reading coin sensor in uSec  //not used //vinay
//#define SENSOR_INIT_DISCARDED_SAMPLES		2//10  //not used //vinay
#define AVERAGING_POINTS					10
#define COIN_BASELINE_START					15
#define POINTS_FOR_BASELINE					50
#define COIN_BASELINE_START_R4				10
#define POINTS_FOR_BASELINE_R4				15
#define NUMBER_OF_CAL_DROPS					10
#define	DEFAULT_OP_PARAMS_VALUE_SCALE 		5			//Value  Scale, 5
#define DEFAULT_COIN_SENSOR_TIMEOUT			10			//changed to 10 seconds //2 sec
#define WAKE_COUNTS_STEP_SIZE				10
//#define WAKE_COUNT_FLUCTUATION_THRESHOLD	10  //not used //vinay
#define WAKE_COUNT_BOOTUP_THRESHOLD			5
#define MAX_COIN_SAMPLING_TRIALS			5

//#define COIN_MIN_MAX_TOLERANCE				16  //not used //vinay
//#define COIN_RATIO_TOLERANCE				8  //not used //vinay

//#define COINS_PER_UDP_PACKET				6//9   // to send coin profiles //Vinay
#define COINS_PER_UDP_PACKET_CT55			9
#define COINS_PER_UDP_PACKET_CT125			6

#define IN_CALIBRATION_MODE					0
#define AUTO_CALIBRATION_MODE				1
#define SELF_CALIBRATION_MODE				2

#define REGULAR_UPWARDS_ADJUSTMENT			1
#define JAM_CLEAR_UPWARDS_ADJUSTMENT		2
#define REGULAR_DOWNWARDS_ADJUSTMENT		3

#define NO_WAKEUP_COIL_CHECK				0
#define RAISE_WAKEUP_COIL_ALARM				1
#define CLEAR_WAKEUP_COIL_ALARM				2

#define WAKE_COUNT_START_VALUE				3000//1200
#define MINIMUM_WAKECOUNT_VALUE				300

#define READ_CALIBRATION					0
#define OVERWRITE_AUTO_CALIBRATION			1
#define ERASE_CALIBRATION					2
#define OVERWRITE_SELF_CALIBRATION			3

#define MAX_FAILED_COINS_DIAG				5
#define NORMAL_COIN_START_INDEX				3

//dcd_error definitions
//#define nov_err								0x01		//bit 0 - nov error  //not used //vinay
#define coil_err							0x02		//bit 1 - coil sensor error
//#define batd_err  							0x04		//bit 2 - dead battery error  //not used //vinay
//#define opto_err							0x04		//bit 2 - optical jam error  //not used //vinay
//#define wake_err  							0x08		//bit 3 - serial channel lockup -wakeup coil jam  //not used //vinay
//#define jam_err								0x10 		//bit 4 - recognition coil jam  //not used //vinay
//#define dk_err								0x20    	//bit 5 - data key error  //not used //vinay
//#define batl_err 							0x40		//bit 6 - low battery  - maybe this becomes opto jam? or a different set of flags  //not used //vinay
//#define aud_err 							0x80		//bit 7 - audit error on reset  //not used //vinay

#define nLed_Drive1_High()					{GPIO_setOutputHighOnPin(NLED_DRIVE1_PORT, NLED_DRIVE1_PIN);}
#define nLed_Drive1_Low()					{GPIO_setOutputLowOnPin(NLED_DRIVE1_PORT, NLED_DRIVE1_PIN);}

#define nLed_Drive2_High()					{GPIO_setOutputHighOnPin(NLED_DRIVE2_PORT, NLED_DRIVE2_PIN);}
#define nLed_Drive2_Low()					{GPIO_setOutputLowOnPin(NLED_DRIVE2_PORT, NLED_DRIVE2_PIN);}

#define nLed_Drive3_High()					{GPIO_setOutputHighOnPin(NLED_DRIVE3_PORT, NLED_DRIVE3_PIN);}
#define nLed_Drive3_Low()					{GPIO_setOutputLowOnPin(NLED_DRIVE3_PORT, NLED_DRIVE3_PIN);}

#define nLed_Drive3_R4_High()				{GPIO_setOutputHighOnPin(NLED_DRIVE3_PORT_R4, NLED_DRIVE3_PIN_R4);}
#define nLed_Drive3_R4_Low()				{GPIO_setOutputLowOnPin(NLED_DRIVE3_PORT_R4, NLED_DRIVE3_PIN_R4);}

#define pclk_High()							{/*GPIO_setOutputHighOnPin(PCLK_PORT, PCLK_PIN)*/;}
#define pclk_Low()							{GPIO_setOutputLowOnPin(PCLK_PORT, PCLK_PIN);}

#define t_High()							{GPIO_setOutputLowOnPin(T_PORT, T_PIN);} //T is inverted on Lib2 board
#define t_Low()								{GPIO_setOutputHighOnPin(T_PORT, T_PIN); DelayUs(100);}

#define chirp_High()		  				{/*GPIO_setOutputHighOnPin(CHIRP_PORT, CHIRP_PIN);*/}
#define chirp_Low()							{GPIO_setOutputLowOnPin(CHIRP_PORT, CHIRP_PIN);}

#define snson_High()		  				{GPIO_setOutputHighOnPin(SNSON_PORT, SNSON_PIN);}
#define snson_Low()							{GPIO_setOutputLowOnPin(SNSON_PORT, SNSON_PIN);}

#define clsns_Low()			  				{GPIO_setOutputLowOnPin(CLSNS_PORT, CLSNS_PIN);}	//altered, we have an inverter in LNG
#define clsns_High()						{GPIO_setOutputHighOnPin(CLSNS_PORT, CLSNS_PIN);}	//altered, we have an inverter in LNG

#define pclk_On()							{GPIO_setAsPeripheralModuleFunctionOutputPin(PCLK_PORT, PCLK_PIN, GPIO_PRIMARY_MODULE_FUNCTION);}
#define pclk_Off()							{GPIO_setAsOutputPin(PCLK_PORT, PCLK_PIN);GPIO_setOutputLowOnPin(PCLK_PORT, PCLK_PIN);}

// Operational Parameters
// Constants for coin acceptance
enum CCFOpParameters
{
	CCF_OP_PARAMS_MINMAX_QUALIFIER,			//min/max qualifier, 6
	CCF_OP_PARAMS_BASE_DEVIATION,  			//base deviation, 2
	CCF_OP_PARAMS_BASE_FREQ_LSB,  			//base freq, LSB = 0
	CCF_OP_PARAMS_BASE_FREQ_MSB,  			//base freq, MSB = 0
	CCF_OP_PARAMS_TEMP_CO, 					//tempco, 0
	CCF_OP_PARAMS_CHIRPER_SWITCH_DELAY,		// chirper switch delay, 2
	CCF_OP_PARAMS_CHIRPER_SETTLING,			// chirper settling, 2
	CCF_OP_PARAMS_SAMPLES_BEFORE_2ND_BASE, 	//samples before 2nd base, 24
	CCF_OP_PARAMS_STARTUP_TIMEOUT, 			//startup timeout, 10
	CCF_OP_PARAMS_SENSOR_TIMEOUT, 			//sensor timeout, 10
	CCF_OP_PARAMS_WAKEUP_RATE, 				//wakeup rate, 25
	CCF_OP_PARAMS_TOD_TRIM, 				//TOD trim, 24
	CCF_OP_PARAMS_VALUE_SCALE//, 				//Value  Scale, 5
// 	CCF_OP_PARAMS_VALUE_UINT,				//Value Unit, 0.01  //not used //vinay
//	CCF_OP_PARAMS_COUNTS  //not used //vinay
};

typedef struct
{
	uint16_t                        SC_counter;
    uint16_t						SCMin_Sens[NUMBER_OF_CAL_DROPS];
    uint16_t 						SCMax_Sens[NUMBER_OF_CAL_DROPS];
    uint16_t 						SCRatio1[NUMBER_OF_CAL_DROPS];
    uint16_t 						SCRatio2[NUMBER_OF_CAL_DROPS];

}Self_Calibration_Struct;

typedef struct
{
	uint16_t						Cmax_val;
	uint16_t						Cmin_val;
	uint16_t						Crat1_val;
	uint16_t						Crat2_val;
}Coin_min_max_diag;

uint8_t get_Led_Rxd_val();
uint8_t get_Coin_Position(uint8_t iLed);
uint8_t get_Snsrdy_val();
void Check_Coin_Jam_Alarm(uint8_t wakeup_coil_alarm_check);
void Coin_WakeCount_adjustment(uint8_t upwards_adjust);

void coin_wakeup();
void coin_sensor_init();
void coin_sensor_on();
void coin_sensor_off();
bool sensor_check_auto_cal();
//void coin_init_ccf_Parameters(); //not used //vinay
void CoinValidation();
uint8_t GetOpParameter(uint8_t opParam);
void CoinOk(void);
//void AutoCalibration(void);
void Coin_Calibration_Mode(uint8_t validated_coin);
//void StartCoinSensorTimeout(void); //not used //vinay
//bool IsCoinSensorTimeout(void);  //not used //vinay

#endif /* PERIPHERAL_MODULES_LIBG2_COIN_H_ */
