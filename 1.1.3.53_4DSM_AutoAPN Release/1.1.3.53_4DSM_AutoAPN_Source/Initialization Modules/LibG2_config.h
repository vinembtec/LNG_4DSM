/*
 * LibG2_config.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef INITIALIZATION_MODULES_LIBG2_CONFIG_H_
#define INITIALIZATION_MODULES_LIBG2_CONFIG_H_

#include "../Main Module/LibG2_main.h"

#define MAX_SPECIAL_DAYS_IN_CONFIG				80 //90
#define MAX_SCHEDULES_IN_A_DAY					20
#define MAX_RATES_IN_CONFIG						20
#define MAX_SPLIT_RATES							5
#define DAYS_IN_WEEK							7
#define MAX_CHARACTERS_IN_DISPLAY				24
#define ALLOWED_CUSTOM_DISPLAY_MESSAGES			4 //3
#define ALLOWED_SCHEDULE_DISPLAY_MESSAGES		16
#define MAX_COINS_SUPPORTED						20
#define MULTIPART_COINS_COUNT					5
//#define TOTAL_MINUTES_IN_A_DAY					1440 //not used //vinay

#define FREE_PARKING_RATE_INDEX					255
#define NO_PARKING_RATE_INDEX					254
#define PREPAY_PARKING_RATE_INDEX				253

#define MAXNUM_COIN_PARAMS_NEW					8
//#define MAXNUM_CCF_COINS						20

#define FACTVN_MAX_TRIES						5
#define FACTORY_MPB_CONFIG_VERSION  			1999 //1111

//#define DEFAULT_CC_TIMEOUT						15 //15 sec //not used //vinay
#define DEFAULT_BKLT_TIMEOUT					2 //1	//3 sec
#define	DEFAULT_PAYMENT_HOLD_TIMEOUT			5
#define DEFAULT_KEYPRESS_TIMEOUT				30 //15 sec
#define DEFAULT_CALIBRATION_TIMEOUT				30
#define DEFAULT_MODEM_OFF_TIMEOUT				5

#define DEFAULT_SMS_READ_TIMEOUT				300	//5 minutes

#define US_SIT_AREA_NUM							1
#define US_SIT_CUST_ID							1055 //4190//8034//1055      --LNG Trial
#define METER_ID								10055 //32001 //32004//10052
#define Df_Location_ID							"10055" //"Liberty 001" //"LNG-Trial 4"//"10052"
#define Df_Serial_No							"12345"
#define Df_City_Code							1
#define BAY_NO                  				1       // bay no will always be 1

#define ZO_EXIT_BIT								BIT_0
#define ZO_ENTRY_BIT							BIT_1
#define FREE_TIME_BIT							BIT_2
#define NON_ZO_IN_BIT							BIT_3
#define NON_ZO_TO_UDP_BIT						BIT_4
#define Three_Min								180

#define NO_GRACE_TIME							0   //Grace time type selection
#define HOLD_EXPIRED							1
#define NEGATIVE_COUNTDOWN                      2

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct
{
	uint8_t                         font_size;	//1 = 16*24 font, 2 = 32*48 font
    uint8_t							display_string[MAX_CHARACTERS_IN_DISPLAY];
    //if 32*48 font is selected, then max chacter string length will be 12

}Custom_LCD;

typedef struct
{
    uint8_t							display_string1[5]; //5 characters for MSM space names
 }spaces;

typedef struct
{
	uint8_t                         special_day_date;
	uint8_t                         special_day_month;
	uint8_t                         special_day_year;
	uint8_t                         special_day_schedule_index; //special_day_rate_index;	//assign a rate index even if it is a holiday

}Holiday_Table;

typedef struct
{
	uint16_t                        schedule_start_time;	//in minutes; one day=1440 minutes
	uint16_t                        schedule_end_time;		//in minutes; one day=1440 minutes
	uint8_t							schedule_rate_index;	// this rate index will be matched with the rates table and used to get the rate
	uint8_t							progressive_rate_index;	// this is the second rate index which will be applicable when progressive rate is enabled
	uint8_t                         schedule_message_ID_1;
	uint8_t                         schedule_message_ID_2;
	uint8_t                         schedule_message_ID_3;
	uint8_t                         schedule_message_ID_4;
}Schedule_time;

typedef struct
{
	uint8_t                         no_of_valid_schedules;	//total number of schedules on this DOW
	Schedule_time					schedules_timing_rate[MAX_SCHEDULES_IN_A_DAY];
}Schedules_Table;

typedef struct
{
	uint8_t			Rate_index;			//255=free parking, 254=No Parking, 253=prepay parking, 1-17= enforced rate, 20-252= Unenforced Rate
	uint8_t			number_of_hour;
	uint16_t		amount_per_hour;	//in cents
	uint16_t		max_time_allowed;	//in minutes

	//fill 0,0 for amount and max time in case of rate index 255 and 254; fill valid amount and max time in case of prepay parking
}Rates_Table;

typedef struct
{
	uint8_t							progressive_rate_index1;	// this is the second rate index which will be applicable when progressive rate is enabled
	uint8_t							progressive_rate_index2;	// this is the third rate index which will be applicable when progressive rate is enabled
	uint8_t							progressive_rate_index3;	// this is the fourth rate index which will be applicable when progressive rate is enabled
	uint8_t							progressive_rate_index4;	// this is the fifth rate index which will be applicable when progressive rate is enabled
	uint8_t							progressive_rate_index5;	// this is the sixth rate index which will be applicable when progressive rate is enabled

}Progressive_Rates;

typedef struct
{
	Progressive_Rates				progressive_rate[MAX_SCHEDULES_IN_A_DAY];
}Progressive_Rates_Table;

typedef struct
{
	uint8_t			future_op_param1;//coin_max_tolerance;//min_max_qualifier;
	uint8_t			future_op_param2;//coin_ratio_tolerance;//base_deviation;
	uint8_t			future_op_param3;//coin_min_tolerance;//base_freq_lsb;
	uint8_t			max_sampling_count;//base_freq_msb;
	uint8_t			wakeup_jam_confirmation_count;//tempco;
	uint8_t			wakecount_adjust_interval;//chirper_switch_delay;
	uint8_t			self_calibration_enable;//chirper_settling;
	uint8_t			add_max_tolerance_selfcal;//samples_before_second_base;
	uint8_t			add_ratio_tolerance_selfcal;//start_up_timeout;
	uint8_t			add_min_tolerance_selfcal;//sensor_timeout;
	uint8_t			wakeup_rate;
	uint8_t			TOD_trim;

}CCF_Op_Params;

typedef struct
{
	uint8_t			coin_max_tolerance;//min_max_qualifier;
	uint8_t			coin_min_tolerance;//base_freq_lsb;
}Calib_tolerance;

//Normal Coins with no overlap
typedef struct _CoinSpecs_New
{
    uint16_t value_units;
    uint16_t params[MAXNUM_COIN_PARAMS_NEW]; // max_minVal, max_max_Val, min_minVal, min_maxVal, ratio_1_min, ratio_1_max, ratio_2_min, ratio_2_max
} CoinSpecsType_New;

//Coins with overlap that need narrow and wide bands
typedef struct _CoinSpecs_multipart
{
    uint16_t value_units;
    uint16_t min_no_of_narrow_to_pass;
    uint16_t narrow_params[MAXNUM_COIN_PARAMS_NEW]; // max_minVal, max_max_Val, min_minVal, min_maxVal, ratio_1_min, ratio_1_max, ratio_2_min, ratio_2_max
    uint16_t expanded_params[MAXNUM_COIN_PARAMS_NEW];	//max_minVal, max_max_Val, min_minVal, min_maxVal, ratio_1_min, ratio_1_max, ratio_2_min, ratio_2_max
} CoinSpecs_Multipart;

typedef struct _s_gprs
{
	uint32_t 			config_file_CRC;	// CRC of the entire file excluding the 4 CRC bytes
	uint8_t         	config_version;
	uint8_t         	config_main_version;
	uint8_t         	config_sub_version;
	uint8_t         	config_branch_version;

	Custom_LCD			default_display_messages[ALLOWED_CUSTOM_DISPLAY_MESSAGES];
	uint8_t         	future_dummy_bytes[4];					//for future use
	uint8_t				santa_cruz_spl_feature_enable; //1=enable, 0=disable
	uint8_t         	Coin_Only_Temporary_GW_Enable; //1=enable, 0=disable
	uint8_t				wakeup_coil_jam_detection_enable; //1=enable, 0=disable
	uint8_t				optical_jam_detection_enable; //1=enable, 0=disable
	uint8_t				card_jam_detection_enable; //1=enable, 0=disable
	uint8_t				special_feature_free_minutes; //Free time in minutes

	uint8_t         	MSM_max_spaces;

	uint8_t				NM_LCD_TIME_ON; //Nov-Mar LCD backlight ON Time
	uint8_t				NM_LCD_TIME_OFF; //Nov-Mar LCD backlight OFF Time
	uint8_t				AO_LCD_TIME_ON; //Apr-Oct LCD backlight ON Time
	uint8_t				AO_LCD_TIME_OFF; //Apr-Oct LCD backlight OFF Time

	Holiday_Table 		holidays[MAX_SPECIAL_DAYS_IN_CONFIG];	//this includes all holidays and special rate days for the next 4 years

	Schedules_Table 	daily_schedules[DAYS_IN_WEEK];

	uint8_t				no_of_valid_rates;
	Rates_Table			all_rates[MAX_RATES_IN_CONFIG];

	Custom_LCD			schedule_display_messages[ALLOWED_SCHEDULE_DISPLAY_MESSAGES];

	uint8_t				rounding_time_enable;					//0=disable, 1=enable
	uint16_t			seconds_to_round_off;
	uint8_t				rounding_amount_enable;					//0=disable, 1=enable
	uint16_t			amount_value_to_round_off;

	uint8_t				bleed_into_next_rate_enable;			//0=disable, 1=enable
	uint8_t				bleed_into_next_rate_time_calc;			//0=disable, 1=enable

	uint8_t				grace_time_type;						//0=No grace time,
																//1=Hold expired message after reaching 00:01 for grace time
																//2: Count and display negative parking clock for grace time before sending expired message
	uint16_t			grace_time;								//in minutes

	uint8_t				SC_refund_enable; 						//0=disable, 1=enable
	uint16_t			free_time;								//in minutes
	uint8_t				graphic_display_enable;					//0=Text Display, 1=Graphic Display

	uint8_t				Enf_LEDs_selection;						//0=Front LEDs, 1=Rear LEDs
	uint8_t				LED_On_time_counter;

	uint16_t			TIME_Check_AT_EXPIRY;

	uint8_t				bleed_into_next_schedule_enable;

	uint32_t        	Coin_self_calib_cfg;
	uint8_t         	rate_dummy_bytes;					//for future use, other rate related flags

	uint8_t				Disable_GPRS_Comms_Coin_Only;	//Default FALSE, TRUE: Disable GPRS comms for coin only meter
	uint8_t				Enable_GW_Comms_Coin_Only; //Default FALSE, TRUE: Enable GW comms for coin only meter

	uint8_t				All_Comms_Via_LORA_Enable; //Default FALSE, TRUE: Enable all comms via LoRa

	uint8_t				ANTI_FEED_ENABLE;
	uint8_t				Special_feature_enable;	//for Moorestown

	uint8_t				LORA_ENABLE_FOR_PBC;

	uint8_t         	Special_Auth_Key[8];
	CCF_Op_Params		operational_parameters;
	CoinSpecsType_New 	coin_parameters[MAX_COINS_SUPPORTED];
	uint8_t         	erase_coin_calibration;

	uint16_t			Active_Channel;
	uint16_t			Passive_Channel;
	uint8_t				LORA_SF;
	uint8_t				LORA_BW;
	uint8_t				LORA_TX_POWER;

	uint8_t         	debug_verbose_level;

	uint8_t         	apn[GPRS_GEN_BUF_SZ_TINY];
	uint8_t         	primary_operator[GPRS_GEN_BUF_SZ_TINY];
	uint8_t         	secondary_operator[GPRS_GEN_BUF_SZ_TINY];
	uint8_t         	UDP_server_ip[ GPRS_IPADD_BUF_SZ ];
	uint16_t        	UDP_server_port;

	uint16_t        	area_num;
	uint16_t        	meter_id;
	uint16_t        	cust_id;
	uint16_t        	bay_no;
	uint16_t        	gen_resp_hdr_sz;
	uint16_t        	Sensor_action_control;	// ZO_EXIT_BIT - BIT_0
												// ZO_ENTRY_BIT - BIT_1
												// FREE_TIME_BIT - BIT_2
												// NON_ZO_IN_BIT - BIT_3
												// NON_ZO_TO_UDP_BIT - BIT_4

	int16_t         	Card_min_amount;
	int16_t         	Card_max_amount;
	int16_t         	amt_incr;
	int16_t         	Card_default_amount;

	uint32_t        	DIAG_INTERVAL;
	uint32_t        	SYNC_INTERVAL;
	uint32_t        	EVT_INTERVAL;

	uint8_t         	gen_retries;
	uint8_t         	gen_timeout;
	uint8_t         	gen_retry_timeout;
	uint8_t         	olt_retries;
	uint8_t         	olt_timeout;
	uint8_t         	olt_retry_timeout;
	uint8_t         	olt_last_retry_timeout;
	uint8_t         	showamount_5sec_timeout;

	uint8_t				Threshold_config1;
	uint8_t				Threshold_config2;
	uint8_t         	Queue_failure_count;
	uint8_t         	transactions_via_GPRS;
	uint8_t         	failure_count;
	uint8_t         	COINS_IN_RT;
	uint8_t         	MODEM_POWER;
	uint8_t         	OLT_via_GPRS;//CC_MAX_FLAG;
	uint8_t         	MF_TIME_ON_1;
	uint8_t         	MF_TIME_ON_2;
	uint8_t         	MF_TIME_OFF_1;
	uint8_t         	MF_TIME_OFF_2;
	uint8_t         	SAT_TIME_ON_1;
	uint8_t         	SAT_TIME_ON_2;
	uint8_t         	SAT_TIME_OFF_1;
	uint8_t         	SAT_TIME_OFF_2;
	uint8_t         	SUN_TIME_ON_1;
	uint8_t         	SUN_TIME_ON_2;
	uint8_t         	SUN_TIME_OFF_1;
	uint8_t         	SUN_TIME_OFF_2;

	uint32_t        	ON_OFF_RANDOM_INTERVAL;
	uint16_t        	Low_Battery_Threshold;

	uint32_t			meter_hard_reset_interval;
	uint16_t			allowed_trans_types;	//14-06-12: Bit 0: Coins,   Bit 1: VISA,
												//14-06-12: Bit 2: MASTERCARD,    Bit 3: AMEX,
												//14-06-12: Bit 4: DINER,  Bit 5: ENROUTE,
												//14-06-12: Bit 6: DISCOVER,Bit 7: JCB,
												//14-06-12: Bit 8: Smart Card,//was reserved Bit 9- Bit 15: reserved for future use,
												//03-01-20: Bit 9: NFC //Bit 10- Bit 12: reserved for future use
												//03-01-20: Bit 13: Use_Detailed_Coin_Packet_Structure
												//03-01-20: Bit 14: Invalid_coins_in_detailed_coin_Packet
	//09-08-2012: sensor related config parameters
	uint16_t			detection_count;
	uint16_t			power_save_detect_count;

	uint8_t				Space_Geometry_mode;
	uint8_t				Mode_3_sens_to_space_distance;
	uint8_t				Mode_3_space_width;
	uint8_t				vehicle_sensor_enable;

	uint8_t				sensor_config[40];

	uint8_t 			BSU_IN_RT;
	uint8_t				CC_percentage_Surcharge;
	uint16_t			CC_fixed_Surcharge;
	uint16_t			SCT_default_min_amount;
	uint16_t			SCT_default_max_amount;
	uint16_t			SCT_amt_incr;
	uint16_t        	Card_Jam_Timeout;

	uint8_t        		show_alarm_on_lcd;
	uint8_t        		CC_TRACK1_ENABLE; //Delayed_BSU;
	uint8_t		   		MF_SENSOR_TIME_ON;
	uint8_t		   		MF_SENSOR_TIME_OFF;
	uint8_t		   		SAT_SENSOR_TIME_ON;
	uint8_t		   		SAT_SENSOR_TIME_OFF;
	uint8_t		   		SUN_SENSOR_TIME_ON;
	uint8_t		   		SUN_SENSOR_TIME_OFF;
	uint8_t         	DLST_ON_OFF;
	uint8_t         	DLST_Slot1[4];
	uint8_t         	DLST_Slot2[4];
	uint8_t         	DLST_Slot3[4];
	uint8_t         	DLST_Slot4[4];
	uint8_t         	DLST_Slot5[4];
	uint8_t         	DLST_Slot6[4];
	uint8_t         	DLST_Slot7[4];
	uint8_t         	DLST_Slot8[4];
	uint8_t         	DLST_Slot9[4];
	uint8_t         	DLST_Slot10[4];
	uint16_t        	DLST_Action_byte;

	CoinSpecs_Multipart multipart_coin_parameters[MULTIPART_COINS_COUNT];

	Progressive_Rates_Table progressive_rate_with_schedule[DAYS_IN_WEEK];	//This is inline with the schedules table, to make the config backward compatible, it is added here

	Calib_tolerance		coin_calib_tol[MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT];

	//	uint8_t				LNG_cfg_dummy[73];

		uint8_t		  		Display_Surcharges; //vinay //Default FALSE, TRUE: Disable GPRS comms for coin only meter//1byte
		uint8_t 			min_time_to_display; //vinay //it will be in minutes and max is 0-255 //in program we need to *60 and use it
		//	uint8_t				LNG_cfg_dummy[71]; //previously 73 bytes //vinay added Display Surcharges and min time to display//2bytes trenton

		uint8_t		  		MSM; //vinay //Default FALSE(not MSM), TRUE(MSM): whether its MSM or not //1byte
		uint8_t				extra; //extra byte is given but not reflected in PEMS config editor.
		uint8_t				MSM_Spaces; //no of spaces
		spaces			 	space_no[12]; //bay names stored in 60 bytes each space 5bytes 5*12=60
		uint8_t				LNG_cfg_dummy[8]; //previously 71 bytes //using this 60 bytes for MSM
} GPRSSystem;
#pragma pack(pop)   /* restore original alignment from stack */

void init_Config();
void Assign_Lora_parameters();
void Validate_LORA_parameters();
void Validate_Config_Parameters();

#endif /* INITIALIZATION_MODULES_LIBG2_CONFIG_H_ */
