//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_config.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_config_api
//! @{
//
//*****************************************************************************
#include "LibG2_config.h"
#define COIN_TEST_IN_LAB

GPRSSystem      		glSystem_cfg;

extern tLoRaSettings    LoRaSettings,LoRaSettings1;
extern uint8_t			HARDWARE_REVISION;
extern volatile uint8_t			No_Modem_Coin_Only;
/**************************************************************************/
//! Initializes the Meter's Configuration with some default parameters to
//! operate, this configuration will be overwritten when a valid configuration
//! file is available in meter's external flash
//! \param void
//! \return void
/**************************************************************************/
void init_Config()
{
	uint8_t i = 0;

	glSystem_cfg.config_file_CRC = 0;
	glSystem_cfg.config_version = 0;
	glSystem_cfg.config_main_version = 0;
	glSystem_cfg.config_sub_version = 0;
	glSystem_cfg.config_branch_version = 0;

	//India SIT
	memcpy(glSystem_cfg.apn, "www", sizeof("www")); //Vodafone
	//memcpy(glSystem_cfg.apn, "jionet", sizeof("jionet")); //Jio
	//memcpy(glSystem_cfg.apn, "airtelgprs.com", sizeof("airtelgprs.com"));
	//memcpy(glSystem_cfg.apn, "internet.sierrawireless.com", sizeof("internet.sierrawireless.com"));
	//memcpy(glSystem_cfg.UDP_server_ip, "106.51.39.214", sizeof("106.51.39.214")); //SIT
	memcpy(glSystem_cfg.UDP_server_ip, "1.23.182.90", sizeof("1.23.182.90")); //SIT
	glSystem_cfg.UDP_server_port = 6050; //SIT
	//memcpy(glSystem_cfg.UDP_server_ip, "64.132.70.171", sizeof("64.132.70.171")); //SIT Sierra
	//glSystem_cfg.UDP_server_port = 8787; //SIT Sierra
	memcpy(glSystem_cfg.primary_operator, "IND airtel", sizeof("IND airtel"));
	memcpy(glSystem_cfg.secondary_operator,"Vodafone IN", sizeof("Vodafone IN"));

	//US AT&T, T-mobile
/*	memcpy(glSystem_cfg.apn, "METERSPRO02.DUNCAN-USA.COM", sizeof("METERSPRO02.DUNCAN-USA.COM"));
	//memcpy(glSystem_cfg.apn, "m2m.t-mobile.com", sizeof("m2m.t-mobile.com"));
	memcpy(glSystem_cfg.UDP_server_ip, "172.25.41.43", sizeof("172.25.41.43"));
	glSystem_cfg.UDP_server_port = 7878;*/
	//T-Mobile Kore
/*	memcpy(glSystem_cfg.apn, "c1.korem2m.com", sizeof("c1.korem2m.com")); //TMobile New APN and they want with Alternate UDP
	memcpy(glSystem_cfg.UDP_server_ip, "64.132.70.171", sizeof("64.132.70.171"));
	glSystem_cfg.UDP_server_port = 8787;*/

	//US Sierra
/*	memcpy(glSystem_cfg.apn, "internet.sierrawireless.com", sizeof("internet.sierrawireless.com"));
	//memcpy(glSystem_cfg.apn, "m2m.com.attz", sizeof("m2m.com.attz"));//12
	//memcpy(glSystem_cfg.apn, "vzwinternet", sizeof("vzwinternet"));//13
	memcpy(glSystem_cfg.UDP_server_ip, "64.132.70.171", sizeof("64.132.70.171"));
	glSystem_cfg.UDP_server_port = 8787;
	memcpy(glSystem_cfg.primary_operator, "AT&T", sizeof("AT&T"));
	memcpy(glSystem_cfg.secondary_operator,"T-Mobile", sizeof("T-Mobile"));*/

/*	glSystem_cfg.default_display_messages[0].font_size = FONT_SIZE_SMALL;
	memcpy(glSystem_cfg.default_display_messages[0].display_string, "MON-SUN           24 HRS", sizeof(glSystem_cfg.default_display_messages[0].display_string));
	glSystem_cfg.default_display_messages[1].font_size = FONT_SIZE_SMALL;
	memcpy(glSystem_cfg.default_display_messages[1].display_string, "$1.00/HR       24 HR MAX", sizeof(glSystem_cfg.default_display_messages[1].display_string));
	glSystem_cfg.default_display_messages[2].font_size = FONT_SIZE_SMALL;
	memcpy(glSystem_cfg.default_display_messages[2].display_string, "  WELCOME TO CIVICSMART ", sizeof(glSystem_cfg.default_display_messages[2].display_string));
	glSystem_cfg.default_display_messages[3].font_size = FONT_SIZE_SMALL;
	if(HARDWARE_REVISION == LNG_REV4)
	{
		memcpy(glSystem_cfg.default_display_messages[3].display_string, " LIBERTY NEXT GEN REV-4 ", sizeof(glSystem_cfg.default_display_messages[3].display_string));
	}
	else
	{
		memcpy(glSystem_cfg.default_display_messages[3].display_string, " LIBERTY NEXT GEN REV-3 ", sizeof(glSystem_cfg.default_display_messages[3].display_string));
	}*/

	glSystem_cfg.schedule_display_messages[0].font_size = FONT_SIZE_SMALL;
	memcpy(glSystem_cfg.schedule_display_messages[0].display_string, "  WELCOME TO CIVICSMART ", sizeof(glSystem_cfg.default_display_messages[2].display_string));
	glSystem_cfg.schedule_display_messages[1].font_size = FONT_SIZE_SMALL;
	if(HARDWARE_REVISION == LNG_REV4)
	{
		memcpy(glSystem_cfg.schedule_display_messages[1].display_string, " LIBERTY NEXT GEN REV-4 ", sizeof(glSystem_cfg.default_display_messages[2].display_string));
	}
	else
	{
		memcpy(glSystem_cfg.schedule_display_messages[1].display_string, " LIBERTY NEXT GEN REV-3 ", sizeof(glSystem_cfg.default_display_messages[2].display_string));
	}
	glSystem_cfg.schedule_display_messages[2].font_size = FONT_SIZE_SMALL;
	memcpy(glSystem_cfg.schedule_display_messages[2].display_string, "MON-SUN           24 HRS", sizeof(glSystem_cfg.default_display_messages[2].display_string));
	glSystem_cfg.schedule_display_messages[3].font_size = FONT_SIZE_SMALL;
	memcpy(glSystem_cfg.schedule_display_messages[3].display_string, "$1.00/HR       24 HR MAX", sizeof(glSystem_cfg.default_display_messages[2].display_string));

#ifdef COIN_TEST_IN_LAB
	for(i=0; i<7; i++)	//Lab default Config is same schedule and rate everyday
	{
		glSystem_cfg.daily_schedules[i].no_of_valid_schedules = 1;
		glSystem_cfg.daily_schedules[i].schedules_timing_rate[0].schedule_start_time = 0;
		glSystem_cfg.daily_schedules[i].schedules_timing_rate[0].schedule_end_time = 1440;
		glSystem_cfg.daily_schedules[i].schedules_timing_rate[0].schedule_rate_index = 1;//255;//1;255=Free,254=Noparking,1=Enforce,21=Unenforce

		glSystem_cfg.daily_schedules[i].schedules_timing_rate[0].schedule_message_ID_1 = 1;
		glSystem_cfg.daily_schedules[i].schedules_timing_rate[0].schedule_message_ID_2 = 2;
		glSystem_cfg.daily_schedules[i].schedules_timing_rate[0].schedule_message_ID_3 = 3;
		glSystem_cfg.daily_schedules[i].schedules_timing_rate[0].schedule_message_ID_4 = 4;
	}

	glSystem_cfg.no_of_valid_rates = 1;
	//Rate 1
	glSystem_cfg.all_rates[0].Rate_index = 1;
	glSystem_cfg.all_rates[0].amount_per_hour = 100;//200;//100;
	glSystem_cfg.all_rates[0].max_time_allowed = 1440;//360;//180;//1440;

#else
	//Sunday Schedule Start
	glSystem_cfg.daily_schedules[0].no_of_valid_schedules = 1;
	glSystem_cfg.daily_schedules[0].schedules_timing_rate[0].schedule_start_time = 0;	//12AM
	glSystem_cfg.daily_schedules[0].schedules_timing_rate[0].schedule_end_time = 1440;//479;	//7:59AM
	glSystem_cfg.daily_schedules[0].schedules_timing_rate[0].schedule_rate_index = 21;	//Free Parking

	/*glSystem_cfg.daily_schedules[0].schedules_timing_rate[1].schedule_start_time = 60;//480;	//8AM
	glSystem_cfg.daily_schedules[0].schedules_timing_rate[1].schedule_end_time = 120;	//8:59AM
	glSystem_cfg.daily_schedules[0].schedules_timing_rate[1].schedule_rate_index = PREPAY_PARKING_RATE_INDEX;	//Prepay Parking

	glSystem_cfg.daily_schedules[0].schedules_timing_rate[2].schedule_start_time = 120;	//9AM
	glSystem_cfg.daily_schedules[0].schedules_timing_rate[2].schedule_end_time = 1380;	//4:59PM
	glSystem_cfg.daily_schedules[0].schedules_timing_rate[2].schedule_rate_index = 1;	//Paid Parking Rate1

	glSystem_cfg.daily_schedules[0].schedules_timing_rate[3].schedule_start_time = 1380;	//5PM
	glSystem_cfg.daily_schedules[0].schedules_timing_rate[3].schedule_end_time = 1440;	//11:59PM
	glSystem_cfg.daily_schedules[0].schedules_timing_rate[3].schedule_rate_index = NO_PARKING_RATE_INDEX;	//No Parking
	 */
	glSystem_cfg.daily_schedules[0].schedules_timing_rate[0].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[0].schedules_timing_rate[0].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[0].schedules_timing_rate[0].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[0].schedules_timing_rate[0].schedule_message_ID_4 = 4;

	//Sunday Schedule End

	//Monday Schedule Start
	glSystem_cfg.daily_schedules[1].no_of_valid_schedules = 3;
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[0].schedule_start_time = 0;//540;	//9AM
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[0].schedule_end_time = 480;	//4:59PM
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[0].schedule_rate_index = 21;	//Paid Parking Rate1

	glSystem_cfg.daily_schedules[1].schedules_timing_rate[0].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[0].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[0].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[0].schedule_message_ID_4 = 4;

	glSystem_cfg.daily_schedules[1].schedules_timing_rate[1].schedule_start_time = 480;	//12AM
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[1].schedule_end_time = 1200;	//7:59AM
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[1].schedule_rate_index = 1;	//Free Parking

	glSystem_cfg.daily_schedules[1].schedules_timing_rate[1].progressive_rate_index = 2;

	glSystem_cfg.daily_schedules[1].schedules_timing_rate[1].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[1].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[1].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[1].schedule_message_ID_4 = 4;

	glSystem_cfg.daily_schedules[1].schedules_timing_rate[2].schedule_start_time = 1200;	//8AM
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[2].schedule_end_time = 1440;	//8:59AM
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[2].schedule_rate_index = 21;	//Prepay Parking

	glSystem_cfg.daily_schedules[1].schedules_timing_rate[2].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[2].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[2].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[2].schedule_message_ID_4 = 4;

	/*
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[3].schedule_start_time = 1380;	//5PM
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[3].schedule_end_time = 1440;	//11:59PM
	glSystem_cfg.daily_schedules[1].schedules_timing_rate[3].schedule_rate_index = NO_PARKING_RATE_INDEX;	//No Parking
	 */
	//Monday Schedule End

	//Tuesday Schedule Start
	glSystem_cfg.daily_schedules[2].no_of_valid_schedules = 3;
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[0].schedule_start_time = 0;//540;	//9AM
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[0].schedule_end_time = 480;	//4:59PM
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[0].schedule_rate_index = 21;	//Paid Parking Rate1

	glSystem_cfg.daily_schedules[2].schedules_timing_rate[0].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[0].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[0].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[0].schedule_message_ID_4 = 4;

	glSystem_cfg.daily_schedules[2].schedules_timing_rate[1].schedule_start_time = 480;	//12AM
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[1].schedule_end_time = 1200;	//7:59AM
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[1].schedule_rate_index = 1;	//Free Parking

	glSystem_cfg.daily_schedules[2].schedules_timing_rate[1].progressive_rate_index = 2;

	glSystem_cfg.daily_schedules[2].schedules_timing_rate[1].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[1].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[1].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[1].schedule_message_ID_4 = 4;

	glSystem_cfg.daily_schedules[2].schedules_timing_rate[2].schedule_start_time = 1200;	//8AM
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[2].schedule_end_time = 1440;	//8:59AM
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[2].schedule_rate_index = 21;	//Prepay Parking

	glSystem_cfg.daily_schedules[2].schedules_timing_rate[2].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[2].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[2].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[2].schedule_message_ID_4 = 4;

	/*
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[3].schedule_start_time = 1380;//1020;	//5PM
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[3].schedule_end_time = 1440;	//11:59PM
	glSystem_cfg.daily_schedules[2].schedules_timing_rate[3].schedule_rate_index = NO_PARKING_RATE_INDEX;	//No Parking
	 */
	//Tuesday Schedule End

	//Wednesday Schedule Start
	glSystem_cfg.daily_schedules[3].no_of_valid_schedules = 3;
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[0].schedule_start_time = 0;//540;	//9AM
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[0].schedule_end_time = 480;	//4:59PM
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[0].schedule_rate_index = 21;	//Paid Parking Rate1

	glSystem_cfg.daily_schedules[3].schedules_timing_rate[0].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[0].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[0].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[0].schedule_message_ID_4 = 4;

	glSystem_cfg.daily_schedules[3].schedules_timing_rate[1].schedule_start_time = 480;	//12AM
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[1].schedule_end_time = 1200;	//7:59AM
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[1].schedule_rate_index = 1;	//Free Parking

	glSystem_cfg.daily_schedules[3].schedules_timing_rate[1].progressive_rate_index = 2;

	glSystem_cfg.daily_schedules[3].schedules_timing_rate[1].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[1].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[1].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[1].schedule_message_ID_4 = 4;

	glSystem_cfg.daily_schedules[3].schedules_timing_rate[2].schedule_start_time = 1200;	//8AM
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[2].schedule_end_time = 1440;	//8:59AM
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[2].schedule_rate_index = 21;	//Prepay Parking

	glSystem_cfg.daily_schedules[3].schedules_timing_rate[2].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[2].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[2].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[2].schedule_message_ID_4 = 4;

	/*
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[3].schedule_start_time = 1380;	//5PM
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[3].schedule_end_time = 1440;	//11:59PM
	glSystem_cfg.daily_schedules[3].schedules_timing_rate[3].schedule_rate_index = NO_PARKING_RATE_INDEX;	//No Parking
	 */
	//Wednesday Schedule End

	//Thursday Schedule Start
	glSystem_cfg.daily_schedules[4].no_of_valid_schedules = 3;
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[0].schedule_start_time = 0;//540;	//9AM
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[0].schedule_end_time = 480;	//4:59PM
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[0].schedule_rate_index = 21;	//Paid Parking Rate1

	glSystem_cfg.daily_schedules[4].schedules_timing_rate[0].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[0].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[0].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[0].schedule_message_ID_4 = 4;


	glSystem_cfg.daily_schedules[4].schedules_timing_rate[1].schedule_start_time = 480;	//12AM
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[1].schedule_end_time = 1200;	//7:59AM
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[1].schedule_rate_index = 1;

	glSystem_cfg.daily_schedules[4].schedules_timing_rate[1].progressive_rate_index = 2;

	glSystem_cfg.daily_schedules[4].schedules_timing_rate[1].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[1].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[1].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[1].schedule_message_ID_4 = 4;


	glSystem_cfg.daily_schedules[4].schedules_timing_rate[2].schedule_start_time = 1200;	//8AM
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[2].schedule_end_time = 1440;	//8:59AM
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[2].schedule_rate_index = 21;	//Prepay Parking

	glSystem_cfg.daily_schedules[4].schedules_timing_rate[2].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[2].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[2].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[2].schedule_message_ID_4 = 4;

	/*
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[3].schedule_start_time = 1380;	//5PM
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[3].schedule_end_time = 1440;	//11:59PM
	glSystem_cfg.daily_schedules[4].schedules_timing_rate[3].schedule_rate_index = NO_PARKING_RATE_INDEX;	//No Parking
	 */
	//Thursday Schedule End

	//Friday Schedule Start
	glSystem_cfg.daily_schedules[5].no_of_valid_schedules = 3;
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[0].schedule_start_time = 0;//540;	//9AM
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[0].schedule_end_time = 480;	//4:59PM
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[0].schedule_rate_index = 21;	//Paid Parking Rate1

	glSystem_cfg.daily_schedules[5].schedules_timing_rate[0].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[0].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[0].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[0].schedule_message_ID_4 = 4;

	glSystem_cfg.daily_schedules[5].schedules_timing_rate[1].schedule_start_time = 480;	//12AM
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[1].schedule_end_time = 1200;	//7:59AM
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[1].schedule_rate_index = 1;	//Free Parking

	glSystem_cfg.daily_schedules[5].schedules_timing_rate[1].progressive_rate_index = 2;

	glSystem_cfg.daily_schedules[5].schedules_timing_rate[1].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[1].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[1].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[1].schedule_message_ID_4 = 4;

	glSystem_cfg.daily_schedules[5].schedules_timing_rate[2].schedule_start_time = 1200;	//8AM
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[2].schedule_end_time = 1440;	//8:59AM
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[2].schedule_rate_index = 21;	//Prepay Parking

	glSystem_cfg.daily_schedules[5].schedules_timing_rate[2].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[2].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[2].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[2].schedule_message_ID_4 = 4;

	/*
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[3].schedule_start_time = 1380;	//5PM
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[3].schedule_end_time = 1440;	//11:59PM
	glSystem_cfg.daily_schedules[5].schedules_timing_rate[3].schedule_rate_index = NO_PARKING_RATE_INDEX;	//No Parking
	 */
	//Friday Schedule End

	//Saturday Schedule Start
	glSystem_cfg.daily_schedules[6].no_of_valid_schedules = 3;
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[0].schedule_start_time = 0;	//9AM
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[0].schedule_end_time = 480;	//4:59PM
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[0].schedule_rate_index = 21;	//Paid Parking Rate1

	glSystem_cfg.daily_schedules[6].schedules_timing_rate[0].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[0].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[0].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[0].schedule_message_ID_4 = 4;

	glSystem_cfg.daily_schedules[6].schedules_timing_rate[1].schedule_start_time = 480;	//12AM
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[1].schedule_end_time = 1200;	//7:59AM
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[1].schedule_rate_index = 1;	//Free Parking

	glSystem_cfg.daily_schedules[6].schedules_timing_rate[1].progressive_rate_index = 2;

	glSystem_cfg.daily_schedules[6].schedules_timing_rate[1].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[1].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[1].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[1].schedule_message_ID_4 = 4;

	glSystem_cfg.daily_schedules[6].schedules_timing_rate[2].schedule_start_time = 1200;	//8AM
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[2].schedule_end_time = 1440;	//8:59AM
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[2].schedule_rate_index = 21;	//Prepay Parking

	glSystem_cfg.daily_schedules[6].schedules_timing_rate[2].schedule_message_ID_1 = 1;
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[2].schedule_message_ID_2 = 2;
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[2].schedule_message_ID_3 = 3;
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[2].schedule_message_ID_4 = 4;

	/*
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[3].schedule_start_time = 1380;	//5PM
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[3].schedule_end_time = 1440;	//11:59PM
	glSystem_cfg.daily_schedules[6].schedules_timing_rate[3].schedule_rate_index = NO_PARKING_RATE_INDEX;	//No Parking
	 */
	//Saturday Schedule End

	for(i=0; i<MAX_RATES_IN_CONFIG; i++)
	{
		glSystem_cfg.all_rates[i].Rate_index = 0;
		glSystem_cfg.all_rates[i].amount_per_hour = 0;
		glSystem_cfg.all_rates[i].max_time_allowed = 0;
	}

	glSystem_cfg.no_of_valid_rates = 3;
	//Rate 1
	glSystem_cfg.all_rates[0].Rate_index = 1;
	glSystem_cfg.all_rates[0].amount_per_hour = 150;
	glSystem_cfg.all_rates[0].max_time_allowed = 120;

	//Free Parking Rate
	glSystem_cfg.all_rates[1].Rate_index = 2;
	glSystem_cfg.all_rates[1].amount_per_hour = 300;
	glSystem_cfg.all_rates[1].max_time_allowed = 600;

	glSystem_cfg.all_rates[2].Rate_index = 21;
	glSystem_cfg.all_rates[2].amount_per_hour = 150;
	glSystem_cfg.all_rates[2].max_time_allowed = 120;

	//Rate 2
	/*glSystem_cfg.all_rates[1].Rate_index = 2;
	glSystem_cfg.all_rates[1].amount_per_hour = 200;
	glSystem_cfg.all_rates[1].max_time_allowed = 60;

	//Prepay Parking Rate
	glSystem_cfg.all_rates[3].Rate_index = PREPAY_PARKING_RATE_INDEX;
	glSystem_cfg.all_rates[3].amount_per_hour = 100;
	glSystem_cfg.all_rates[3].max_time_allowed = 120;*/

	//No Parking Rate
	/*glSystem_cfg.all_rates[2].Rate_index = NO_PARKING_RATE_INDEX;
	glSystem_cfg.all_rates[2].amount_per_hour = 0;
	glSystem_cfg.all_rates[2].max_time_allowed = 0;*/
#endif
	glSystem_cfg.rounding_time_enable = 0;
	glSystem_cfg.seconds_to_round_off = 0;
	glSystem_cfg.rounding_amount_enable = 0;
	glSystem_cfg.amount_value_to_round_off = 0;
	glSystem_cfg.bleed_into_next_rate_enable = TRUE;
	glSystem_cfg.bleed_into_next_rate_time_calc = TRUE;
	glSystem_cfg.bleed_into_next_schedule_enable = TRUE;
	glSystem_cfg.grace_time_type = NO_GRACE_TIME ;//NEGATIVE_COUNTDOWN;//NO_GRACE_TIME ;
	glSystem_cfg.grace_time = 10;

	glSystem_cfg.SC_refund_enable = TRUE;
	glSystem_cfg.free_time = 5;
	glSystem_cfg.graphic_display_enable = 0;

	//CCF Parameters
	//glSystem_cfg.operational_parameters.coin_max_tolerance = 50;//30;//COIN_MAX_TOLERANCE;
	//glSystem_cfg.operational_parameters.coin_ratio_tolerance = 12;//COIN_RATIO_TOLERANCE;
	//glSystem_cfg.operational_parameters.coin_min_tolerance = 30;//10;	//COIN_MIN_TOLERANCE
	glSystem_cfg.operational_parameters.max_sampling_count = 4;	//samples before declaring M1 or M2 failure
	glSystem_cfg.operational_parameters.wakeup_jam_confirmation_count = 2;	//Total number of M1 or M2 to raise wakeup coil jam alarm
	glSystem_cfg.operational_parameters.wakecount_adjust_interval = 4;	//WakeCount Check interval in minutes

	glSystem_cfg.operational_parameters.self_calibration_enable = false;//true;	//false will disable self calibration//true will enable self calibration
	glSystem_cfg.Coin_self_calib_cfg = 0;//983040; //if self calibration is false then keep it 0 // if self cal is enabled then use this value 983040

	glSystem_cfg.operational_parameters.add_max_tolerance_selfcal = 5;	//Self calibration max tolerance
	glSystem_cfg.operational_parameters.add_ratio_tolerance_selfcal = 5;	//Self calibration ratio1,ratio2 tolerance
	glSystem_cfg.operational_parameters.add_min_tolerance_selfcal = 5;	//Self calibration min tolerance
	glSystem_cfg.operational_parameters.wakeup_rate = 50;	//From 18-05-20 wakeup rate = 50 become standard >VT<
	glSystem_cfg.operational_parameters.TOD_trim = 24;

	glSystem_cfg.erase_coin_calibration = false;

	for(i=0; i<(MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT); i++)
	{
		glSystem_cfg.coin_calib_tol[i].coin_max_tolerance = 0;
		glSystem_cfg.coin_calib_tol[i].coin_min_tolerance = 0;
	}

	for(i=0; i<MAX_COINS_SUPPORTED; i++)
	{
		memset(glSystem_cfg.coin_parameters[i].params, 0, sizeof(glSystem_cfg.coin_parameters[i].params));
		glSystem_cfg.coin_parameters[i].value_units = 0;
	}

	for(i=0; i<MULTIPART_COINS_COUNT; i++)
	{
		memset(glSystem_cfg.multipart_coin_parameters[i].narrow_params, 0, sizeof(glSystem_cfg.multipart_coin_parameters[i].narrow_params));
		memset(glSystem_cfg.multipart_coin_parameters[i].expanded_params, 0, sizeof(glSystem_cfg.multipart_coin_parameters[i].expanded_params));
		glSystem_cfg.multipart_coin_parameters[i].value_units = 0;
	}

//Billings Tokens as $0.25
/*	glSystem_cfg.coin_parameters[14].params[0] = 780;
	glSystem_cfg.coin_parameters[14].params[1] = 920;
	glSystem_cfg.coin_parameters[14].params[2] = 130;
	glSystem_cfg.coin_parameters[14].params[3] = 250;
	glSystem_cfg.coin_parameters[14].params[4] = 0;
	glSystem_cfg.coin_parameters[14].params[5] = 30;
	glSystem_cfg.coin_parameters[14].params[6] = 0;
	glSystem_cfg.coin_parameters[14].params[7] = 45;
	glSystem_cfg.coin_parameters[14].value_units 	= 25;

//Helena, Montana Tokens as $0.25
	glSystem_cfg.coin_parameters[15].params[0]	=	800;
	glSystem_cfg.coin_parameters[15].params[1]	=	1180;
	glSystem_cfg.coin_parameters[15].params[2]	=	280;
	glSystem_cfg.coin_parameters[15].params[3]	=	405;
	glSystem_cfg.coin_parameters[15].params[4]	=	15;
	glSystem_cfg.coin_parameters[15].params[5]	=	40;
	glSystem_cfg.coin_parameters[15].params[6]	=	15;
	glSystem_cfg.coin_parameters[15].params[7]	=	50;
	glSystem_cfg.coin_parameters[15].value_units 	= 25;
*/

/*	//Canada $0.05 (1982-2001)
	glSystem_cfg.coin_parameters[8].params[0] = 800;
	glSystem_cfg.coin_parameters[8].params[1] = 995;
	glSystem_cfg.coin_parameters[8].params[2] = 105;
	glSystem_cfg.coin_parameters[8].params[3] = 170;
	glSystem_cfg.coin_parameters[8].params[4] = 4;
	glSystem_cfg.coin_parameters[8].params[5] = 12;
	glSystem_cfg.coin_parameters[8].params[6] = 8;
	glSystem_cfg.coin_parameters[8].params[7] = 18;
	glSystem_cfg.coin_parameters[8].value_units 	= 5;

	//Canada $0.05 (<=1982, 2000-Present)
	glSystem_cfg.coin_parameters[9].params[0] = 2020;
	glSystem_cfg.coin_parameters[9].params[1] = 3200;
	glSystem_cfg.coin_parameters[9].params[2] = 260;
	glSystem_cfg.coin_parameters[9].params[3] = 490;
	glSystem_cfg.coin_parameters[9].params[4] = 5;
	glSystem_cfg.coin_parameters[9].params[5] = 15;
	glSystem_cfg.coin_parameters[9].params[6] = 5;
	glSystem_cfg.coin_parameters[9].params[7] = 20;
	glSystem_cfg.coin_parameters[9].value_units 	= 5;

	//Canada $0.10 (2001-Present)
	glSystem_cfg.coin_parameters[10].params[0] = 1900;
	glSystem_cfg.coin_parameters[10].params[1] = 2500;
	glSystem_cfg.coin_parameters[10].params[2] = 120;
	glSystem_cfg.coin_parameters[10].params[3] = 240;
	glSystem_cfg.coin_parameters[10].params[4] = 0;
	glSystem_cfg.coin_parameters[10].params[5] = 10;
	glSystem_cfg.coin_parameters[10].params[6] = 0;
	glSystem_cfg.coin_parameters[10].params[7] = 15;
	glSystem_cfg.coin_parameters[10].value_units 	= 10;

	//Canada $0.10 (1969-1999)
	glSystem_cfg.coin_parameters[11].params[0] = 1500;
	glSystem_cfg.coin_parameters[11].params[1] = 2050;
	glSystem_cfg.coin_parameters[11].params[2] = 95;
	glSystem_cfg.coin_parameters[11].params[3] = 180;
	glSystem_cfg.coin_parameters[11].params[4] = 0;
	glSystem_cfg.coin_parameters[11].params[5] = 10;
	glSystem_cfg.coin_parameters[11].params[6] = 0;
	glSystem_cfg.coin_parameters[11].params[7] = 15;
	glSystem_cfg.coin_parameters[11].value_units 	= 10;

	//Canada $0.25 (2000-Present)
	glSystem_cfg.coin_parameters[12].params[0] = 2700;
	glSystem_cfg.coin_parameters[12].params[1] = 3150;
	glSystem_cfg.coin_parameters[12].params[2] = 640;
	glSystem_cfg.coin_parameters[12].params[3] = 860;
	glSystem_cfg.coin_parameters[12].params[4] = 5;
	glSystem_cfg.coin_parameters[12].params[5] = 20;
	glSystem_cfg.coin_parameters[12].params[6] = 10;
	glSystem_cfg.coin_parameters[12].params[7] = 25;
	glSystem_cfg.coin_parameters[12].value_units 	= 25;

	//Canada $0.25 (1968-1999)
	glSystem_cfg.coin_parameters[13].params[0] = 2150;
	glSystem_cfg.coin_parameters[13].params[1] = 2800;
	glSystem_cfg.coin_parameters[13].params[2] = 480;
	glSystem_cfg.coin_parameters[13].params[3] = 680;
	glSystem_cfg.coin_parameters[13].params[4] = 5;
	glSystem_cfg.coin_parameters[13].params[5] = 20;
	glSystem_cfg.coin_parameters[13].params[6] = 10;
	glSystem_cfg.coin_parameters[13].params[7] = 25;
	glSystem_cfg.coin_parameters[13].value_units 	= 25;

	//Canada $1 (2012-Present)
	glSystem_cfg.coin_parameters[14].params[0] = 4000;
	glSystem_cfg.coin_parameters[14].params[1] = 4750;
	glSystem_cfg.coin_parameters[14].params[2] = 1600;
	glSystem_cfg.coin_parameters[14].params[3] = 1950;
	glSystem_cfg.coin_parameters[14].params[4] = 10;
	glSystem_cfg.coin_parameters[14].params[5] = 35;
	glSystem_cfg.coin_parameters[14].params[6] = 15;
	glSystem_cfg.coin_parameters[14].params[7] = 45;
	glSystem_cfg.coin_parameters[14].value_units 	= 100;

	//Canada $1 (<2000, 2003, 2007-2011)
	glSystem_cfg.coin_parameters[15].params[0] = 2600;
	glSystem_cfg.coin_parameters[15].params[1] = 3200;
	glSystem_cfg.coin_parameters[15].params[2] = 950;
	glSystem_cfg.coin_parameters[15].params[3] = 1250;
	glSystem_cfg.coin_parameters[15].params[4] = 15;
	glSystem_cfg.coin_parameters[15].params[5] = 35;
	glSystem_cfg.coin_parameters[15].params[6] = 15;
	glSystem_cfg.coin_parameters[15].params[7] = 45;
	glSystem_cfg.coin_parameters[15].value_units 	= 100;

	//Canada $2 (1996,2013)
	glSystem_cfg.coin_parameters[16].params[0]	=	2000;
	glSystem_cfg.coin_parameters[16].params[1]	=	3200;
	glSystem_cfg.coin_parameters[16].params[2]	=	1180;
	glSystem_cfg.coin_parameters[16].params[3]	=	2050;
	glSystem_cfg.coin_parameters[16].params[4]	=	15;
	glSystem_cfg.coin_parameters[16].params[5]	=	60;
	glSystem_cfg.coin_parameters[16].params[6]	=	15;
	glSystem_cfg.coin_parameters[16].params[7]	=	70;
	glSystem_cfg.coin_parameters[16].value_units 	= 200;*/
#if 1
	//US $1
	glSystem_cfg.coin_parameters[16].params[0]	=	780; //800;
	glSystem_cfg.coin_parameters[16].params[1]	=	1120; //1100;
	glSystem_cfg.coin_parameters[16].params[2]	=	300; //320;
	glSystem_cfg.coin_parameters[16].params[3]	=	660; //650;
	glSystem_cfg.coin_parameters[16].params[4]	=	10; //13;
	glSystem_cfg.coin_parameters[16].params[5]	=	50; //48;
	glSystem_cfg.coin_parameters[16].params[6]	=	10; //13;
	glSystem_cfg.coin_parameters[16].params[7]	=	60; //63;
	glSystem_cfg.coin_parameters[16].value_units	=	100;
	glSystem_cfg.coin_calib_tol[16].coin_max_tolerance = 100;
	glSystem_cfg.coin_calib_tol[16].coin_min_tolerance = 50;

	glSystem_cfg.coin_parameters[17].params[0]	=	520;
	glSystem_cfg.coin_parameters[17].params[1]	=	770;
	glSystem_cfg.coin_parameters[17].params[2]	=	125;
	glSystem_cfg.coin_parameters[17].params[3]	=	340;
	glSystem_cfg.coin_parameters[17].params[4]	=	4;
	glSystem_cfg.coin_parameters[17].params[5]	=	33;
	glSystem_cfg.coin_parameters[17].params[6]	=	11;
	glSystem_cfg.coin_parameters[17].params[7]	=	42;
	glSystem_cfg.coin_parameters[17].value_units	=	25;
	glSystem_cfg.coin_calib_tol[17].coin_max_tolerance = 75;
	glSystem_cfg.coin_calib_tol[17].coin_min_tolerance = 50;

	glSystem_cfg.coin_parameters[18].params[0]	=	280; //300;
	glSystem_cfg.coin_parameters[18].params[1]	=	500;       //560
	glSystem_cfg.coin_parameters[18].params[2]	=	0;
	glSystem_cfg.coin_parameters[18].params[3]	=	110; //110;       //130
	glSystem_cfg.coin_parameters[18].params[4]	=	0;
	glSystem_cfg.coin_parameters[18].params[5]	=	60; //58;
	glSystem_cfg.coin_parameters[18].params[6]	=	0;
	glSystem_cfg.coin_parameters[18].params[7]	=	75; //74;
	glSystem_cfg.coin_parameters[18].value_units	=	10;
	glSystem_cfg.coin_calib_tol[18].coin_max_tolerance = 60;
	glSystem_cfg.coin_calib_tol[18].coin_min_tolerance = 50;

	glSystem_cfg.coin_parameters[19].params[0]	=	880; //900;
	glSystem_cfg.coin_parameters[19].params[1]	=	1280;//1220; //1200;
	glSystem_cfg.coin_parameters[19].params[2]	=	50; //70;
	glSystem_cfg.coin_parameters[19].params[3]	=	260; //250;
	glSystem_cfg.coin_parameters[19].params[4]	=	0;
	glSystem_cfg.coin_parameters[19].params[5]	=	25; //21;
	glSystem_cfg.coin_parameters[19].params[6]	=	0;
	glSystem_cfg.coin_parameters[19].params[7]	=	30; //26;
	glSystem_cfg.coin_parameters[19].value_units	=	5;
	glSystem_cfg.coin_calib_tol[19].coin_max_tolerance = 100;
	glSystem_cfg.coin_calib_tol[19].coin_min_tolerance = 50;
#else
	//Petosky Token with Multipart config
	glSystem_cfg.multipart_coin_parameters[3].narrow_params[0]	=	800;
	glSystem_cfg.multipart_coin_parameters[3].narrow_params[1]	=	874;
	glSystem_cfg.multipart_coin_parameters[3].narrow_params[2]	=	182;
	glSystem_cfg.multipart_coin_parameters[3].narrow_params[3]	=	250;
	glSystem_cfg.multipart_coin_parameters[3].narrow_params[4]	=	12;
	glSystem_cfg.multipart_coin_parameters[3].narrow_params[5]	=	20;
	glSystem_cfg.multipart_coin_parameters[3].narrow_params[6]	=	16;
	glSystem_cfg.multipart_coin_parameters[3].narrow_params[7]	=	25;
	glSystem_cfg.multipart_coin_parameters[3].expanded_params[0]	=	800;
	glSystem_cfg.multipart_coin_parameters[3].expanded_params[1]	=	985;
	glSystem_cfg.multipart_coin_parameters[3].expanded_params[2]	=	155;
	glSystem_cfg.multipart_coin_parameters[3].expanded_params[3]	=	250;
	glSystem_cfg.multipart_coin_parameters[3].expanded_params[4]	=	10;
	glSystem_cfg.multipart_coin_parameters[3].expanded_params[5]	=	20;
	glSystem_cfg.multipart_coin_parameters[3].expanded_params[6]	=	11;
	glSystem_cfg.multipart_coin_parameters[3].expanded_params[7]	=	25;
	glSystem_cfg.multipart_coin_parameters[3].value_units	=	10;
	glSystem_cfg.multipart_coin_parameters[3].min_no_of_narrow_to_pass = 1;
	glSystem_cfg.coin_calib_tol[MAX_COINS_SUPPORTED + 3].coin_max_tolerance = 50;
	glSystem_cfg.coin_calib_tol[MAX_COINS_SUPPORTED + 3].coin_min_tolerance = 15;

	//US Nickel with Multipart config
	glSystem_cfg.multipart_coin_parameters[4].narrow_params[0]	=	985;
	glSystem_cfg.multipart_coin_parameters[4].narrow_params[1]	=	1220;
	glSystem_cfg.multipart_coin_parameters[4].narrow_params[2]	=	100;
	glSystem_cfg.multipart_coin_parameters[4].narrow_params[3]	=	155;
	glSystem_cfg.multipart_coin_parameters[4].narrow_params[4]	=	4;
	glSystem_cfg.multipart_coin_parameters[4].narrow_params[5]	=	9;
	glSystem_cfg.multipart_coin_parameters[4].narrow_params[6]	=	5;
	glSystem_cfg.multipart_coin_parameters[4].narrow_params[7]	=	10;
	glSystem_cfg.multipart_coin_parameters[4].expanded_params[0]	=	874;
	glSystem_cfg.multipart_coin_parameters[4].expanded_params[1]	=	1220;
	glSystem_cfg.multipart_coin_parameters[4].expanded_params[2]	=	100;
	glSystem_cfg.multipart_coin_parameters[4].expanded_params[3]	=	182;
	glSystem_cfg.multipart_coin_parameters[4].expanded_params[4]	=	4;
	glSystem_cfg.multipart_coin_parameters[4].expanded_params[5]	=	12;
	glSystem_cfg.multipart_coin_parameters[4].expanded_params[6]	=	5;
	glSystem_cfg.multipart_coin_parameters[4].expanded_params[7]	=	16;
	glSystem_cfg.multipart_coin_parameters[4].value_units	=	5;
	glSystem_cfg.multipart_coin_parameters[4].min_no_of_narrow_to_pass = 1;
	glSystem_cfg.coin_calib_tol[MAX_COINS_SUPPORTED + 4].coin_max_tolerance = 50;
	glSystem_cfg.coin_calib_tol[MAX_COINS_SUPPORTED + 4].coin_min_tolerance = 15;

	//US $1
	glSystem_cfg.coin_parameters[17].params[0]	=	780; //800;
	glSystem_cfg.coin_parameters[17].params[1]	=	1120; //1100;
	glSystem_cfg.coin_parameters[17].params[2]	=	300; //320;
	glSystem_cfg.coin_parameters[17].params[3]	=	660; //650;
	glSystem_cfg.coin_parameters[17].params[4]	=	10; //13;
	glSystem_cfg.coin_parameters[17].params[5]	=	50; //48;
	glSystem_cfg.coin_parameters[17].params[6]	=	10; //13;
	glSystem_cfg.coin_parameters[17].params[7]	=	60; //63;
	glSystem_cfg.coin_parameters[17].value_units	=	100;
	glSystem_cfg.coin_calib_tol[17].coin_max_tolerance = 75;
	glSystem_cfg.coin_calib_tol[17].coin_min_tolerance = 30;

	glSystem_cfg.coin_parameters[18].params[0]	=	520;
	glSystem_cfg.coin_parameters[18].params[1]	=	770;
	glSystem_cfg.coin_parameters[18].params[2]	=	125;
	glSystem_cfg.coin_parameters[18].params[3]	=	340;
	glSystem_cfg.coin_parameters[18].params[4]	=	4;
	glSystem_cfg.coin_parameters[18].params[5]	=	33;
	glSystem_cfg.coin_parameters[18].params[6]	=	11;
	glSystem_cfg.coin_parameters[18].params[7]	=	42;
	glSystem_cfg.coin_parameters[18].value_units	=	25;
	glSystem_cfg.coin_calib_tol[18].coin_max_tolerance = 50;
	glSystem_cfg.coin_calib_tol[18].coin_min_tolerance = 20;

	glSystem_cfg.coin_parameters[19].params[0]	=	280; //300;
	glSystem_cfg.coin_parameters[19].params[1]	=	500;       //560
	glSystem_cfg.coin_parameters[19].params[2]	=	0;
	glSystem_cfg.coin_parameters[19].params[3]	=	110;       //130
	glSystem_cfg.coin_parameters[19].params[4]	=	0;
	glSystem_cfg.coin_parameters[19].params[5]	=	60; //58;
	glSystem_cfg.coin_parameters[19].params[6]	=	0;
	glSystem_cfg.coin_parameters[19].params[7]	=	75; //74;
	glSystem_cfg.coin_parameters[19].value_units	=	10;
	glSystem_cfg.coin_calib_tol[19].coin_max_tolerance = 40;
	glSystem_cfg.coin_calib_tol[19].coin_min_tolerance = 20;

#endif

	//Index 0, 1 & 2 are now reserved for expanded 25C and Opto logic will be applied on this
/*	glSystem_cfg.coin_parameters[0].params[0]	=	500;
	glSystem_cfg.coin_parameters[0].params[1]	=	870;
	glSystem_cfg.coin_parameters[0].params[2]	=	80;//110;//100;
	glSystem_cfg.coin_parameters[0].params[3]	=	350;
	glSystem_cfg.coin_parameters[0].params[4]	=	0;
	glSystem_cfg.coin_parameters[0].params[5]	=	35;
	glSystem_cfg.coin_parameters[0].params[6]	=	0;
	glSystem_cfg.coin_parameters[0].params[7]	=	45;
	glSystem_cfg.coin_parameters[0].value_units	=	25;*/

	//Index 3 is now used for coin values, because customer is asking for different values for different coins//vinay
/*	glSystem_cfg.coin_parameters[3].params[0]	=	0; //Coin Index 12
	glSystem_cfg.coin_parameters[3].params[1]	=	0; //Coin Index 13
	glSystem_cfg.coin_parameters[3].params[2]	=	0; //Coin Index 14
	glSystem_cfg.coin_parameters[3].params[3]	=	0; //Coin Index 15
	glSystem_cfg.coin_parameters[3].params[4]	=	100;//100; //Coin Index 16
	glSystem_cfg.coin_parameters[3].params[5]	=	33;//25;//0;//25; //Coin Index 17
	glSystem_cfg.coin_parameters[3].params[6]	=	20;//10; //Coin Index 18
	glSystem_cfg.coin_parameters[3].params[7]	=	20;//5; //Coin Index 19
	glSystem_cfg.coin_parameters[3].value_units	=	1;//0;//1;*/

	glSystem_cfg.bay_no        					= BAY_NO;
	glSystem_cfg.SYNC_INTERVAL 					= 3600;//900;//3600;
	glSystem_cfg.EVT_INTERVAL 					= 3600;//600;//3600;
	glSystem_cfg.DIAG_INTERVAL 					= 900;//300;//900;

	glSystem_cfg.gen_retries 					= 2;
	glSystem_cfg.gen_timeout 					= 150;
	glSystem_cfg.gen_retry_timeout 				= 100;
	glSystem_cfg.olt_retries 					= 2;
	glSystem_cfg.olt_timeout 					= 150;
	glSystem_cfg.olt_retry_timeout 				= 50;
	glSystem_cfg.olt_last_retry_timeout 		= 30;

	glSystem_cfg.MF_TIME_OFF_1                  = 110;
	glSystem_cfg.MF_TIME_OFF_2                  = 162;
	glSystem_cfg.MF_TIME_ON_1                   = 113;
	glSystem_cfg.MF_TIME_ON_2                   = 171;
	glSystem_cfg.SAT_TIME_OFF_1                 = 200;
	glSystem_cfg.SAT_TIME_OFF_2                 = 200;
	glSystem_cfg.SAT_TIME_ON_1                  = 60;
	glSystem_cfg.SAT_TIME_ON_2                  = 60;
	glSystem_cfg.SUN_TIME_OFF_1                 = 200;
	glSystem_cfg.SUN_TIME_OFF_2                 = 200;
	glSystem_cfg.SUN_TIME_ON_1                  = 60;
	glSystem_cfg.SUN_TIME_ON_2                  = 60;
	glSystem_cfg.MODEM_POWER                    = FALSE;
	glSystem_cfg.show_alarm_on_lcd              = FALSE;
	//glSystem_cfg.CC_TRACK1_ENABLE				= FALSE;
	glSystem_cfg.transactions_via_GPRS 			= TRUE;//this both false will make transaction via gateway
	glSystem_cfg.OLT_via_GPRS 					= TRUE;//this both false will make transaction via gateway
	glSystem_cfg.allowed_trans_types 			= 0xFFFF; //In this bit9-NFC, bit13-Use_Detailed_Coin_Packet_Structure, bit14-Invalid_coins_in_detailed_coin_Packet.
													//0x9FFF // disable the new features for coin packet
													//0xBFFF // enable only coin detail packet for valid coins
													//0xFFFF // enable for both valid and invalid coins
	glSystem_cfg.Card_default_amount 			= 100;
	glSystem_cfg.Card_max_amount 				= 300;//400;
	glSystem_cfg.Card_min_amount 				= 100;
	glSystem_cfg.amt_incr 						= 25;
	glSystem_cfg.showamount_5sec_timeout 		= 3;

	glSystem_cfg.CC_percentage_Surcharge		= 0;
	glSystem_cfg.CC_fixed_Surcharge				= 0;//to check give some value else make it 0 //vinay
	glSystem_cfg.debug_verbose_level			= 1;
	glSystem_cfg.gen_resp_hdr_sz				= 10;

	glSystem_cfg.SCT_default_min_amount 		= 100;
	glSystem_cfg.SCT_default_max_amount			= 400;
	glSystem_cfg.SCT_amt_incr					= 25;

	glSystem_cfg.Queue_failure_count			= 3;
	glSystem_cfg.DLST_ON_OFF                    = 0;

	glSystem_cfg.Low_Battery_Threshold			= 340;

	glSystem_cfg.Card_Jam_Timeout				= 150;

	glSystem_cfg.Enf_LEDs_selection 			= REAR_ENF_LED;//FRONT_GREEN_ONLY_ENF_LED;//FRONT_RED_ONLY_ENF_LED;//REAR_GREEN_ONLY_ENF_LED;//REAR_RED_ONLY_ENF_LED;//REAR_ENF_LED;//FRONT_AND_REAR_ENF_LED;//FRONT_ENF_LED;
	glSystem_cfg.LED_On_time_counter			= 1;	//no. of base counter cycles to keep LED on, this is the LED brightness control decider
	glSystem_cfg.meter_hard_reset_interval      = (uint32_t)(86400);

	glSystem_cfg.vehicle_sensor_enable 			= FALSE;//TRUE;//FALSE;

	glSystem_cfg.LORA_SF      					= LoRaSettings1.SpreadingFactor;//7;
	glSystem_cfg.LORA_BW      					= LoRaSettings1.SignalBw;//9;
	glSystem_cfg.LORA_TX_POWER     				= LoRaSettings1.Power;//17;
	glSystem_cfg.Active_Channel     			= 915;//905;//905;//915;//865;//LoRaSettings1.RFFrequency/1000000;//865;
	glSystem_cfg.Passive_Channel    			= 918;//ALTERNATE_FREQUENCY/1000000;//875;
	glSystem_cfg.LORA_ENABLE_FOR_PBC			= TRUE;

	glSystem_cfg.Special_Auth_Key[0] 			= 212;
	glSystem_cfg.Special_Auth_Key[1] 			= 47;
	glSystem_cfg.Special_Auth_Key[2]			= 220;
	glSystem_cfg.Special_Auth_Key[3] 			= 100;
	glSystem_cfg.Special_Auth_Key[4] 			= 53;
	glSystem_cfg.Special_Auth_Key[5] 			= 118;
	glSystem_cfg.Special_Auth_Key[6] 			= 105;
	glSystem_cfg.Special_Auth_Key[7] 			= 253;

	glSystem_cfg.NM_LCD_TIME_ON                 = 0; //1AM
	glSystem_cfg.NM_LCD_TIME_OFF                = 0; //11PM
	glSystem_cfg.AO_LCD_TIME_ON                 = 0; //1AM
	glSystem_cfg.AO_LCD_TIME_OFF                = 0; //11PM

	glSystem_cfg.ANTI_FEED_ENABLE 				= FALSE;//FALSE;//TRUE;//FALSE;
	glSystem_cfg.Special_feature_enable			= FALSE;
	glSystem_cfg.Sensor_action_control			= ZO_EXIT_BIT|ZO_ENTRY_BIT|NON_ZO_IN_BIT|NON_ZO_TO_UDP_BIT;//FREE_TIME_BIT
	glSystem_cfg.MSM_max_spaces 				= 2;
	glSystem_cfg.TIME_Check_AT_EXPIRY			= Three_Min; // 180;

	glSystem_cfg.wakeup_coil_jam_detection_enable 	= TRUE;
	glSystem_cfg.optical_jam_detection_enable		= TRUE;
	glSystem_cfg.card_jam_detection_enable			= TRUE;
	glSystem_cfg.special_feature_free_minutes		= 5;

	if(No_Modem_Coin_Only)	//VT
		glSystem_cfg.Disable_GPRS_Comms_Coin_Only	= TRUE; //Default gprs comms enabled////3oct18
	else
		glSystem_cfg.Disable_GPRS_Comms_Coin_Only	= FALSE;//TRUE;//FALSE; //Default gprs comms enabled////3oct18
	glSystem_cfg.Enable_GW_Comms_Coin_Only		= FALSE; //Default GW comms disabled
	glSystem_cfg.All_Comms_Via_LORA_Enable		= FALSE; //Default comms via LoRa disabled

	glSystem_cfg.Coin_Only_Temporary_GW_Enable 	= FALSE; //Default Audit via LoRa disabled
	glSystem_cfg.santa_cruz_spl_feature_enable 	= FALSE; //Not applicable for DSM, Default Santa Cruz feature disabled

	glSystem_cfg.Display_Surcharges  = FALSE; //vinay //Default FALSE = No Display, True = Display
	glSystem_cfg.min_time_to_display = 0; //if this =0 then no min time to start parking clock, >0 then min time is there. //vinay

    glSystem_cfg.MSM = 0; //for enableing and disableing regular config for Albany customer //vinay
    glSystem_cfg.extra = 0;

}

/**************************************************************************/
//! Initializes the LoRa communication structure which is used during LORA
//! modules initialization
//! \param void
//! \return void
/**************************************************************************/
void Assign_Lora_parameters()
{
	LoRaSettings.SpreadingFactor = glSystem_cfg.LORA_SF;
	LoRaSettings.SignalBw = glSystem_cfg.LORA_BW;
	LoRaSettings.Power = glSystem_cfg.LORA_TX_POWER	;
	LoRaSettings.CrcOn = LoRaSettings1.CrcOn;
	LoRaSettings.ErrorCoding = LoRaSettings1.ErrorCoding;
	LoRaSettings.FreqHopOn = LoRaSettings1.FreqHopOn;
	LoRaSettings.HopPeriod = LoRaSettings1.HopPeriod;
	LoRaSettings.ImplicitHeaderOn = LoRaSettings1.ImplicitHeaderOn;
	LoRaSettings.PayloadLength = LoRaSettings1.PayloadLength;
	//LoRaSettings.RxPacketTimeout = LoRaSettings1.RxPacketTimeout;
	LoRaSettings.TxPacketTimeout = LoRaSettings1.TxPacketTimeout;
	LoRaSettings.RxSingleOn = LoRaSettings1.RxSingleOn;
	//LoRaSettings.RFFrequency = (uint32_t)(glSystem_cfg.Active_Channel*1000000);
	LoRaSettings.RFFrequency = (uint32_t)((uint32_t)glSystem_cfg.Active_Channel*(uint32_t)1000000L);	//primary communication channel
}

/**************************************************************************/
//! Checks for out of bound values assigned in LORA configuration and if found,
//! defaults it to an acceptable value, prints an error on debug log.
//! \param void
//! \return void
/**************************************************************************/
void Validate_LORA_parameters()
{
	if((glSystem_cfg.LORA_SF < 7) || ((glSystem_cfg.LORA_SF > 12)))
	{
		//Debug_TextOut(0,"LORA CFG_ERR1");
		glSystem_cfg.LORA_SF = LoRaSettings1.SpreadingFactor;
	}
	if((glSystem_cfg.LORA_BW < 7) || (glSystem_cfg.LORA_BW > 9))
	{
		//Debug_TextOut(0,"LORA CFG_ERR2");
		glSystem_cfg.LORA_BW = LoRaSettings1.SignalBw;
	}
	if((glSystem_cfg.LORA_TX_POWER < 2) || (glSystem_cfg.LORA_TX_POWER > 20))
	{
		//Debug_TextOut(0,"LORA CFG_ERR3");
		glSystem_cfg.LORA_TX_POWER = LoRaSettings1.Power;
	}
	if((glSystem_cfg.Active_Channel < 865) || (glSystem_cfg.Active_Channel > 928))
	{
		//Debug_TextOut(0,"LORA CFG_ERR4");
		glSystem_cfg.Active_Channel = LoRaSettings1.RFFrequency/1000000L;
	}
	if((glSystem_cfg.Passive_Channel < 865) || (glSystem_cfg.Passive_Channel > 928))
	{
		//Debug_TextOut(0,"LORA CFG_ERR5");
		glSystem_cfg.Passive_Channel = ALTERNATE_FREQUENCY/1000000L;
	}
}

/**************************************************************************/
//! Checks for out of bound values assigned in configuration and if found,
//! defaults it to an acceptable value, prints an error on debug log.
//! \param void
//! \return void
/**************************************************************************/
void Validate_Config_Parameters()
{
	Validate_LORA_parameters();

	/*if((glSystem_cfg.gen_retries < 2) || (glSystem_cfg.gen_retries > 10))
	{
		Debug_TextOut(0,"CFG_ERR 13");
		glSystem_cfg.gen_retries = 2;
	}
	if((glSystem_cfg.gen_timeout < 100) || (glSystem_cfg.gen_timeout > 150))
	{
		Debug_TextOut(0,"CFG_ERR 14");
		glSystem_cfg.gen_timeout = 150;
	}
	if((glSystem_cfg.gen_retry_timeout < 50) || (glSystem_cfg.gen_retry_timeout > 100))
	{
		Debug_TextOut(0,"CFG_ERR 15");
		glSystem_cfg.gen_timeout = 100;
	}*/
/*	if((glSystem_cfg.olt_retries < 1) || (glSystem_cfg.olt_retries > 2))
	{
		Debug_TextOut(0,"CFG_ERR 16");
		glSystem_cfg.olt_retries = 2;
	}
	if((glSystem_cfg.olt_timeout < 100) || (glSystem_cfg.olt_timeout > 150))
	{
		Debug_TextOut(0,"CFG_ERR 17");
		glSystem_cfg.olt_timeout = 150;
	}
	if((glSystem_cfg.olt_retry_timeout < 50) || (glSystem_cfg.olt_retry_timeout > 150))
	{
		Debug_TextOut(0,"CFG_ERR 18");
		glSystem_cfg.olt_retry_timeout = 50;
	}
	if((glSystem_cfg.olt_last_retry_timeout < 30) || (glSystem_cfg.olt_last_retry_timeout > 50))
	{
		Debug_TextOut(0,"CFG_ERR 19");
		glSystem_cfg.olt_last_retry_timeout = 30;
	}*/
	/*if((glSystem_cfg.Enf_LEDs_selection != FRONT_ENF_LED) || (glSystem_cfg.Enf_LEDs_selection != REAR_ENF_LED))
	{
		Debug_TextOut(0,"CFG_ERR 20");
		glSystem_cfg.Enf_LEDs_selection = REAR_ENF_LED;
	}*/
	if((glSystem_cfg.LED_On_time_counter < 1) || (glSystem_cfg.LED_On_time_counter > 10))
	{
		//Debug_TextOut(0,"CFG_ERR 21");
		glSystem_cfg.LED_On_time_counter = 1;
	}
	if((glSystem_cfg.meter_hard_reset_interval < (uint32_t)3600) || (glSystem_cfg.meter_hard_reset_interval > (uint32_t)86400))
	{
		//Debug_TextOut(0,"CFG_ERR 22");
		glSystem_cfg.meter_hard_reset_interval = 86400;
	}
	if((glSystem_cfg.Low_Battery_Threshold < 200) || (glSystem_cfg.Low_Battery_Threshold > 400))
	{
		//Debug_TextOut(0,"CFG_ERR 23");
		glSystem_cfg.Low_Battery_Threshold = 300;
	}
	/*if(glSystem_cfg.MODEM_POWER != FALSE)
	{
		Debug_TextOut(0,"CFG_ERR 24");
		glSystem_cfg.MODEM_POWER = FALSE;
	}
	if((glSystem_cfg.MF_TIME_OFF_1 < 200) || (glSystem_cfg.MF_TIME_OFF_2 < 200))
	{
		Debug_TextOut(0,"CFG_ERR 25");
		glSystem_cfg.MF_TIME_OFF_1 = 200;
		glSystem_cfg.MF_TIME_OFF_2 = 200;
	}*/
	if((glSystem_cfg.Queue_failure_count < 2) || (glSystem_cfg.Queue_failure_count > 10))
	{
		//Debug_TextOut(0,"CFG_ERR 26");
		glSystem_cfg.Queue_failure_count = 3;
	}
	if((glSystem_cfg.SYNC_INTERVAL < (uint32_t)300) || (glSystem_cfg.SYNC_INTERVAL > (uint32_t)86400))
	{
		//Debug_TextOut(0,"CFG_ERR 27");
		glSystem_cfg.SYNC_INTERVAL = 3600;
	}
	if((glSystem_cfg.DIAG_INTERVAL < (uint32_t)300) || (glSystem_cfg.DIAG_INTERVAL > (uint32_t)86400))
	{
		//Debug_TextOut(0,"CFG_ERR 28");
		glSystem_cfg.DIAG_INTERVAL = 10800;
	}
	if((glSystem_cfg.EVT_INTERVAL < (uint32_t)300) || (glSystem_cfg.EVT_INTERVAL > (uint32_t)86400))
	{
		//Debug_TextOut(0,"CFG_ERR 29");
		glSystem_cfg.EVT_INTERVAL = 3600;
	}
	/*if((glSystem_cfg.MF_TIME_ON_1 > 230) || (glSystem_cfg.MF_TIME_ON_2 > 230))
	{
		Debug_TextOut(0,"CFG_ERR 30");
		glSystem_cfg.MF_TIME_ON_1 = 60;
		glSystem_cfg.MF_TIME_ON_2 = 60;
	}*/
	if(glSystem_cfg.operational_parameters.wakeup_rate < 50) // >>VT<<
	{
		//Debug_TextOut(0,"CFG_ERR 31");
		glSystem_cfg.operational_parameters.wakeup_rate = 50;
	}
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
