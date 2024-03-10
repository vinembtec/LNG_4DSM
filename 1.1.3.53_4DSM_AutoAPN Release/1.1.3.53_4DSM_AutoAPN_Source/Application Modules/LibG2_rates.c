//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_rates.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_rates_api
//! @{
//
//*****************************************************************************

#include "LibG2_rates.h"
#include "../Peripheral Modules/LibG2_lcd.h"

//extern uint8_t card_valid; // vinay for max time in credit card payment in split rates //not using in this program //vinay

extern GPRSSystem      		glSystem_cfg;
//extern uint8_t              RTC_String[MAXBYTESINROW+1]; //it is not used in this program //vinay
extern GPRS_PamBayStatusUpdateRequest  PAM_Bay_status_data[MAX_PAM_BSU_QUEUE];
extern uint8_t 				PAM_Baystatus_update_count;
extern uint8_t 				Parking_Clock_Refresh[MSM_MAX_PARKING_BAYS];
//extern uint8_t 				ccard_dataread_flag, smart_card_mode; //it is not used in this program //vinay
extern uint8_t				glLast_reported_space_status[MSM_MAX_PARKING_BAYS], Meter_Full_Flg[MSM_MAX_PARKING_BAYS];
//extern uint32_t				total_Payment_for_the_day; //not used anywhere else //vinay
extern uint16_t 			Current_Space_Id;
extern RTC_C_Calendar 		rtc_c_calender;

volatile uint8_t 			current_rate_index=0, next_sched_rate_index[6] = { 0 }, Max_Amount_In_current_rate = FALSE;
volatile uint8_t			next_rate_index1 = 0, next_rate_index2 = 0, next_rate_index3 = 0, next_rate_index4 = 0, next_rate_index5 = 0;
volatile uint16_t 			minutes_left_in_current_schedule=0, minutes_left_in_next_schedule=0;
volatile uint32_t			parking_time_left_on_meter[MSM_MAX_PARKING_BAYS] = { 0 };//in seconds
//volatile uint32_t	  		negetive_parking_time_left_on_meter[MSM_MAX_PARKING_BAYS] = { 0 };// For grace time//not suing in this program //vinay
uint32_t 					max_time_in_current_rate = 0,santa_cruz_previous_paid_time = 0;
uint8_t						sch_message_index_1 = 0, sch_message_index_2 = 0, sch_message_index_3 = 0, sch_message_index_4 = 0;
uint8_t						holiday_spl_event_found = FALSE;//, in_prepay_parking = FALSE;//, ANTI_FEED_in_effect = false;
uint8_t						in_prepay_parking[MSM_MAX_PARKING_BAYS] = { 0 };
uint8_t						ANTI_FEED_in_effect[MSM_MAX_PARKING_BAYS] = { 0 }; // antifeed was effected to both spaces if 1 clear then both was cleared so using this //vinay
uint8_t						this_is_a_CC = false;

extern volatile uint16_t		time_retain[MSM_MAX_PARKING_BAYS]; //vinay // to retain parking clock to become zero trenton
extern uint32_t 			parking_time_left_on_meter_prev[MSM_MAX_PARKING_BAYS]; //to round off //vinay
//uint32_t 					coin_earned_seconds = 0; //Not used

volatile uint16_t			next_sched_minutes_left_in_current_schedule = 0;

extern uint32_t 			KeyPress_TimeOut_RTC; //to avoid screen refresh every min when keypressed //vinay
//extern uint32_t 			coindrop_TimeOut_RTC;//not suing in this program //vinay
//extern uint8_t				expire[MSM_MAX_PARKING_BAYS]; //LNGSIT-2007 //vinay//not suing in this program //vinay
extern uint8_t 				Meter_Full[MSM_MAX_PARKING_BAYS]; //for antifeed to start //vinay
extern uint16_t				occupancygl;
extern uint8_t 				M1_M2_Counter;
extern CoinSpecsType_New 	loCoinParam; //for throumount customer //vinay
float 						last_amount_used_for_calc_in_CC = 0;
float 						First_amount_used_for_calc_in_CC = 0;
uint32_t 					First_lmax_time_allowed_in_CC = 0;
uint32_t 					last_lmax_time_allowed_in_CC = 0;
uint8_t						First_Time_CC_Entry_For_Next_Schedule = 0;
extern uint8_t 				coin_dropped;
extern uint8_t 				coin_insert_ok;
extern uint8_t              cardread;
/**************************************************************************/
//! This function will scan and fill the schedule details which will be used
//! in rate calculations, this includes holidays/ special days scanning,
//! schedules scanning, escalated rate scanning
//! \param structure with current time
//! \return void
/**************************************************************************/
void read_and_fill_current_schedule(RTC_C_Calendar * lrtc_c_calender)
{
	uint16_t current_minute_of_day = 0;
	uint8_t lday_of_week = 0, lschedule_index = 0, i, special_rate_day_found = false;
	uint8_t lastrate_index = current_rate_index;
	uint8_t	day_of_week_to_use = 0, next_schedule_param = 0;

	lday_of_week = lrtc_c_calender->dayOfWeek;
	current_minute_of_day = ((lrtc_c_calender->hours)*60) + (lrtc_c_calender->minutes);
	if(current_minute_of_day<5)M1_M2_Counter = 0; // to make M1_M2_Counter to zero at 00:00am time
	for(i=0; i<MAX_SPECIAL_DAYS_IN_CONFIG; i++)
	{
		if((lrtc_c_calender->dayOfmonth == glSystem_cfg.holidays[i].special_day_date) && (lrtc_c_calender->month == glSystem_cfg.holidays[i].special_day_month) && ((lrtc_c_calender->year - 2000) == glSystem_cfg.holidays[i].special_day_year))
		{
			//Debug_TextOut(0,"Debug-00\r\n");
			special_rate_day_found = true;
			holiday_spl_event_found = TRUE;
			lschedule_index = glSystem_cfg.holidays[i].special_day_schedule_index;

			if((current_minute_of_day < glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[lschedule_index-1].schedule_end_time) &&
								(current_minute_of_day >= glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[lschedule_index-1].schedule_start_time))
			{
				//Debug_TextOut(0,"Debug-01\r\n");
				current_rate_index = (uint8_t)glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[lschedule_index-1].schedule_rate_index;
				minutes_left_in_current_schedule = (glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[lschedule_index-1].schedule_end_time) - current_minute_of_day;

				sch_message_index_1 = glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[lschedule_index-1].schedule_message_ID_1;
				sch_message_index_2 = glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[lschedule_index-1].schedule_message_ID_2;
				sch_message_index_3 = glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[lschedule_index-1].schedule_message_ID_3;
				sch_message_index_4 = glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[lschedule_index-1].schedule_message_ID_4;
			}
		}
	}

	if(special_rate_day_found == false)
	{
		holiday_spl_event_found = FALSE;
		for(i=0; i<glSystem_cfg.daily_schedules[lday_of_week].no_of_valid_schedules; i++)
		{
			if(glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[i].schedule_end_time == 1439)
				glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[i].schedule_end_time = 1440;

			if((current_minute_of_day < glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[i].schedule_end_time) &&
					(current_minute_of_day >= glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[i].schedule_start_time))
			{
				//Debug_Output2( 0, "\nMonth:%d Date:%d",rtc_c_calender.month,rtc_c_calender.dayOfmonth);
				//Debug_Output1( 0, "Minutes:%d",current_minute_of_day);
				//Debug_TextOut(0,"Debug-02\r\n");
				current_rate_index = (uint8_t)glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[i].schedule_rate_index;
				minutes_left_in_current_schedule = (glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[i].schedule_end_time) - current_minute_of_day;

				if(glSystem_cfg.bleed_into_next_rate_enable == TRUE)
				{
					next_rate_index1 = (uint8_t)glSystem_cfg.progressive_rate_with_schedule[lday_of_week].progressive_rate[i].progressive_rate_index1;
					next_rate_index2 = (uint8_t)glSystem_cfg.progressive_rate_with_schedule[lday_of_week].progressive_rate[i].progressive_rate_index2;
					next_rate_index3 = (uint8_t)glSystem_cfg.progressive_rate_with_schedule[lday_of_week].progressive_rate[i].progressive_rate_index3;
					next_rate_index4 = (uint8_t)glSystem_cfg.progressive_rate_with_schedule[lday_of_week].progressive_rate[i].progressive_rate_index4;
					next_rate_index5 = (uint8_t)glSystem_cfg.progressive_rate_with_schedule[lday_of_week].progressive_rate[i].progressive_rate_index5;
				}
				else
				{
					next_rate_index1 = 0;
					next_rate_index2 = 0;
					next_rate_index3 = 0;
					next_rate_index4 = 0;
					next_rate_index5 = 0;
				}

				sch_message_index_1 = glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[i].schedule_message_ID_1;
				sch_message_index_2 = glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[i].schedule_message_ID_2;
				sch_message_index_3 = glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[i].schedule_message_ID_3;
				sch_message_index_4 = glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[i].schedule_message_ID_4;

				//New change for Denver - 15 July 2019
				//Populate the next schedule also here
				if((i+1)<glSystem_cfg.daily_schedules[lday_of_week].no_of_valid_schedules)//&&(glSystem_cfg.daily_schedules[lday_of_week].no_of_valid_schedules>1))	//Populate the next schedule of current day
				{
					next_schedule_param = i+1;
					day_of_week_to_use = lday_of_week;
					next_sched_rate_index[0] = (uint8_t)glSystem_cfg.daily_schedules[day_of_week_to_use].schedules_timing_rate[next_schedule_param].schedule_rate_index;
					if(next_sched_rate_index[0]>PREPAY_PARKING) // VT ,to avoid free time schedule for holidays placed in current day
						{
							if(((glSystem_cfg.daily_schedules[day_of_week_to_use].schedules_timing_rate[next_schedule_param].schedule_end_time) - (glSystem_cfg.daily_schedules[day_of_week_to_use].schedules_timing_rate[next_schedule_param].schedule_start_time))>=1440)
								{
									if(lday_of_week<6)	//On Sunday to Friday, populate next day's first index
										{						//No more schedules left in the day, populate the next day schedule
											day_of_week_to_use = lday_of_week+1;
											next_schedule_param = 0;
										}
									else	//On Saturday, populate Sunday's first index
										{
											day_of_week_to_use = 0;
											next_schedule_param = 0;
										}
								}
						}
				}
				else if(lday_of_week<6)	//On Sunday to Friday, populate next day's first index
				{						//No more schedules left in the day, populate the next day schedule
					day_of_week_to_use = lday_of_week+1;
					next_schedule_param = 0;
				}
				else	//On Saturday, populate Sunday's first index
				{
					day_of_week_to_use = 0;
					next_schedule_param = 0;
				}
				next_sched_rate_index[0] = (uint8_t)glSystem_cfg.daily_schedules[day_of_week_to_use].schedules_timing_rate[next_schedule_param].schedule_rate_index;
				next_sched_minutes_left_in_current_schedule = 0;
				if(next_sched_rate_index[0]<=PREPAY_PARKING)
					{
						if(day_of_week_to_use==lday_of_week)
						{
							if(next_sched_rate_index[0]==PREPAY_PARKING)
								next_sched_minutes_left_in_current_schedule = (glSystem_cfg.daily_schedules[day_of_week_to_use].schedules_timing_rate[next_schedule_param+1].schedule_end_time) - (glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[i].schedule_end_time);
							else
								next_sched_minutes_left_in_current_schedule = (glSystem_cfg.daily_schedules[day_of_week_to_use].schedules_timing_rate[next_schedule_param].schedule_end_time) - (glSystem_cfg.daily_schedules[lday_of_week].schedules_timing_rate[i].schedule_end_time);
						}
						else next_sched_minutes_left_in_current_schedule = (glSystem_cfg.daily_schedules[day_of_week_to_use].schedules_timing_rate[next_schedule_param].schedule_end_time);
					}
				if((glSystem_cfg.bleed_into_next_rate_enable == TRUE)&&(next_sched_rate_index[0]<=PREPAY_PARKING))
				{
					next_sched_rate_index[1] = (uint8_t)glSystem_cfg.progressive_rate_with_schedule[day_of_week_to_use].progressive_rate[next_schedule_param].progressive_rate_index1;
					next_sched_rate_index[2] = (uint8_t)glSystem_cfg.progressive_rate_with_schedule[day_of_week_to_use].progressive_rate[next_schedule_param].progressive_rate_index2;
					next_sched_rate_index[3] = (uint8_t)glSystem_cfg.progressive_rate_with_schedule[day_of_week_to_use].progressive_rate[next_schedule_param].progressive_rate_index3;
					next_sched_rate_index[4] = (uint8_t)glSystem_cfg.progressive_rate_with_schedule[day_of_week_to_use].progressive_rate[next_schedule_param].progressive_rate_index4;
					next_sched_rate_index[5] = (uint8_t)glSystem_cfg.progressive_rate_with_schedule[day_of_week_to_use].progressive_rate[next_schedule_param].progressive_rate_index5;
				}
				else
				{
					next_sched_rate_index[1] = 0;
					next_sched_rate_index[2] = 0;
					next_sched_rate_index[3] = 0;
					next_sched_rate_index[4] = 0;
					next_sched_rate_index[5] = 0;
				}

				break;
			}
		}
	}
	//Debug_Output1( 0, "Current Rate:%d ",current_rate_index);
	//if((current_rate_index == FREE_PARKING) || (current_rate_index == NO_PARKING))
	{
		if(KeyPress_TimeOut_RTC == 0) //to avoid screen going to idle screen everytime vinay
		{
			Idle_Screen();
			//Debug_TextOut(0,"Rates 2");
		}
	}

	if(current_rate_index != lastrate_index)
	{
		//Reset Santa Cruz last paid time when switching the schedule
		if(glSystem_cfg.santa_cruz_spl_feature_enable == TRUE)
			santa_cruz_previous_paid_time = 0;

		if(ANTI_FEED_in_effect[Current_Space_Id] == true)	//Anti Feed feature enabled in config
		{
			ANTI_FEED_in_effect[Current_Space_Id] = false;	//ready for next payment now since the last vehicle has left
			push_event_to_cache(UDP_EVTTYP_ANTI_FEED_CLEAR);
			//Debug_TextOut(0,"Anti Feed Cleared for Space 0");
			Debug_Output1(0,"Anti Feed clear-space [%d]",Current_Space_Id);
			Meter_Full[Current_Space_Id] = FALSE;
		}

		if(current_rate_index == FREE_PARKING)
		{
			for(i=0; i<MSM_MAX_PARKING_BAYS; i++)
			{
				parking_time_left_on_meter[i] = 0;
				time_retain[i] = 0;
				parking_time_left_on_meter_prev[i] = 0;
				Parking_Clock_Refresh[i] = PARKING_CLOCK_STOP_MODE;
			}
			ANTI_FEED_in_effect[0] = false;
			ANTI_FEED_in_effect[1] = false;
			push_event_to_cache(UDP_EVTTYP_ENTERING_FREE_PARKING);
			Debug_TextOut(0,"ENTERING_FREE_PARKING\r\n");
			Freeparking_Screen();//sometimes it will not show free parking screen immediately. if we select space it will show.so added this//vinay
		}
		else if(current_rate_index == NO_PARKING)
		{
			for(i=0; i<MSM_MAX_PARKING_BAYS; i++)
			{
				parking_time_left_on_meter[i] = 0;
				time_retain[i] = 0;
				parking_time_left_on_meter_prev[i] = 0;
				Parking_Clock_Refresh[i] = PARKING_CLOCK_STOP_MODE;
			}
			ANTI_FEED_in_effect[0] = false;
			ANTI_FEED_in_effect[1] = false;
			push_event_to_cache(UDP_EVTTYP_ENTERING_NO_PARKING);
			Debug_TextOut(0,"ENTERING_NO_PARKING\r\n");
			Noparking_Screen();//sometimes it will not show no parking screen immediately. if we select space it will show.so added this//vinay
		}
		else if(current_rate_index == PREPAY_PARKING )//pre-pay,paid-parking
		{
			in_prepay_parking[0] = TRUE;
			in_prepay_parking[1] = TRUE;
			push_event_to_cache(UDP_EVTTYP_ENTERING_PREPAY_PARKING);
			Debug_TextOut(0,"ENTERING_PREPAY_PARKING\r\n");
			for(i=0; i<MSM_MAX_PARKING_BAYS; i++)
			{
				parking_time_left_on_meter[i] += (minutes_left_in_current_schedule*60); //LNGSIT-636
				if(parking_time_left_on_meter[i] > 0)
					Update_Parking_Clock_Screen();
				else
					Update_Parking_Clock_Screen();
			}
			//PAM bay status update for pre-pay parking
/*			if(PAM_Baystatus_update_count<=(MAX_PAM_BSU_QUEUE-1))
			{
				PAM_Bay_status_data[PAM_Baystatus_update_count].txn_time = RTC_epoch_now();
				PAM_Bay_status_data[PAM_Baystatus_update_count].expiry_time = ((PAM_Bay_status_data[PAM_Baystatus_update_count].txn_time) +
						(parking_time_left_on_meter[Current_Space_Id]));
				PAM_Bay_status_data[PAM_Baystatus_update_count].amount_cents = 0;
				PAM_Bay_status_data[PAM_Baystatus_update_count].purchased_duration = 0;
				PAM_Bay_status_data[PAM_Baystatus_update_count].topup = 0;
				PAM_Bay_status_data[PAM_Baystatus_update_count].txn_type = Tx_Unknown;//todo:add appropriate txn type
				PAM_Baystatus_update_count++;
			}*/
		}
		else if(current_rate_index < PREPAY_PARKING)
		{
			//in_prepay_parking[0] = FALSE;
			//in_prepay_parking[1] = FALSE;
			push_event_to_cache(UDP_EVTTYP_ENTERING_PAID_PARKING);
			Debug_Output1(0,"ENTERING_PAID_PARKING:%d\r\n",current_rate_index);
			if(parking_time_left_on_meter[Current_Space_Id] > 0)
				Update_Parking_Clock_Screen();
			else
				Update_Parking_Clock_Screen();
		}
		else
		{
			ANTI_FEED_in_effect[0] = false;
			ANTI_FEED_in_effect[1] = false;
			Debug_TextOut(0,"INVALID RATE INDEX\r\n");
		}

		//to clear antifeed if active, because sensor In and Outs will not happen in other schedules and it cannot clear antifeed if its on//vinay
		if(((current_rate_index >= 20)&&(current_rate_index != PREPAY_PARKING))&&(glSystem_cfg.ANTI_FEED_ENABLE == TRUE))
		{
			//Debug_TextOut(0,"Not Enforcement time\r\n");
			ANTI_FEED_in_effect[0] = false;
			ANTI_FEED_in_effect[1] = false;	//ready for next payment now since the last vehicle has left
			Meter_Full[0] = FALSE;
			Meter_Full[1] = FALSE;
			Meter_Full_Flg[0] = FALSE; // antifeed for progressive rate was creating problem so added this //vinay
			Meter_Full_Flg[1] = FALSE; // antifeed for progressive rate was creating problem so added this //vinay
			occupancygl = 0; //if vehicle is in and the schedule is not enforced then this will not be cleared because vehicle in and out will not happen, so clearing it // vinay
			//init_LEDs_PWM();
			glLast_reported_space_status[0] = 0;
			glLast_reported_space_status[1] = 0;
		}
	}
}

/**************************************************************************/
//! This function calculates the earned time based on the current rate
//! passing false in top_up_trans will return earned time for buys hours display etc.
//! \param paid_amount amount for which earned time will be calculated
//! \param top_up_trans flag to notify if it is a top up payment or a fresh payment
//! - \b TRUE
//! - \b FALSE
//! \param this_is_before_update flag to distinguish between credit card buys display or actual payment
//! - \b TRUE
//! - \b FALSE
//! \return uint32_t earned time in seconds
/**************************************************************************/
uint32_t get_earned_seconds(uint16_t paid_amount, uint8_t top_up_trans, uint8_t this_is_before_update)
{
	uint64_t earned_seconds = 0, lmax_time_allowed = 0,
			lseconds_left_in_current_schedule = 0, learned_seconds_to_small_rate = 0, max_to_check = 0,
			learned_seconds_second_rate = 0, lpaid_amount_second_rate = 0, lcheck_santa_cruz_allowed_time = 0;
	uint32_t lnext_max_time_allowed[MAX_SPLIT_RATES] = { 0 };
	uint8_t lcurrent_rate_index = 0, i, m, n, p, lnext_rate_index[MAX_SPLIT_RATES] = { 0 };
	uint8_t Rate_found = false, next_rate_already_applied = false;
	float lamount_per_hour = 0, amount_for_calc = 0, lnext_amount_per_hour[MAX_SPLIT_RATES] = { 0 }, last_amount_used_for_calc = 0;
	uint8_t j = 19,k=0;

	//Debug_Output2(0, "parking_time_left_on_meter[%d] = %ld", Current_Space_Id, parking_time_left_on_meter[Current_Space_Id]);

	//uint16_t current_minute_of_day = 0;
/*	if(paid_amount >= MAX_ALLOWED_AMOUNT_IN_A_DAY)	//for Denver, buys full day if $65 is paid
	{
		current_minute_of_day = ((rtc_c_calender.hours)*60) + (rtc_c_calender.minutes);
		return ((1440 - current_minute_of_day)*60 + 60);
	}*/
	//coin_earned_seconds	=	0;
	lcurrent_rate_index = current_rate_index;	//volatile rate index; so assign to local variable
	lseconds_left_in_current_schedule = minutes_left_in_current_schedule*60;
	//minutes_left_in_next_schedule = 0;
	minutes_left_in_next_schedule = next_sched_minutes_left_in_current_schedule;

	lnext_rate_index[0] = next_rate_index1;
	lnext_rate_index[1] = next_rate_index2;
	lnext_rate_index[2] = next_rate_index3;
	lnext_rate_index[3] = next_rate_index4;
	lnext_rate_index[4] = next_rate_index5;

	if(ANTI_FEED_in_effect[Current_Space_Id] == true)	//Anti Feed feature enabled in config
	{
		//Debug_TextOut(0,"Anti Feed, No parking time earned for space 0");
		Debug_Output1(0,"Anti Feed, No parking time earned for space [%d]",Current_Space_Id);
		AntiFeed_Screen();
		if(this_is_before_update == true)	//return the current parking clock
			return parking_time_left_on_meter[Current_Space_Id];
		else	//This is for earned time display, return 0
			return 0;
	}
	// Thurmont  custom //vinay
	//Debug_Output1(0,"glSystem_cfg.coin_parameters[3].value_units=%ld",glSystem_cfg.coin_parameters[3].value_units);
	//Debug_Output1(0,"glSystem_cfg.no_of_valid_rates = %d\n",glSystem_cfg.no_of_valid_rates);
	//for(k=0; k<glSystem_cfg.no_of_valid_rates;k++)
	//	Debug_Output2(0,"Before glSystem_cfg.all_rates[%d].amount_per_hour = %d\n",k,glSystem_cfg.all_rates[k].amount_per_hour);
	//Debug_Output1(0,"loCoinParam.value_units = %d\n",loCoinParam.value_units);
	if((glSystem_cfg.coin_parameters[3].value_units > 0) && (loCoinParam.value_units > 0))
	{
		for(j=19;j>=4;j--)
		{
			//Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].value_units = %d",j,glSystem_cfg.coin_parameters[j].value_units);
			if(glSystem_cfg.coin_parameters[j].value_units == loCoinParam.value_units)
			{
//				if(glSystem_cfg.coin_parameters[j].value_units > 0)
//				{
					for(k=0; k<glSystem_cfg.no_of_valid_rates;k++)
					{
						glSystem_cfg.all_rates[k].amount_per_hour = glSystem_cfg.coin_parameters[3].params[j-12];
						//Debug_Output2(0,"After glSystem_cfg.all_rates[%d].amount_per_hour = %d\n",k,glSystem_cfg.all_rates[k].amount_per_hour);
						//Debug_Output2(0,"glSystem_cfg.coin_parameters[3].params[j-12] = %d, %d",glSystem_cfg.coin_parameters[3].params[j-12],j);
					}
//				}
				break;
			}
		}
	}










	for(i=0; i<glSystem_cfg.no_of_valid_rates; i++)
	{
		if((uint8_t)(glSystem_cfg.all_rates[i].Rate_index) == lcurrent_rate_index)
		{
			Rate_found = true;
			if(glSystem_cfg.all_rates[i].number_of_hour == 0)	//backward compatibility
				glSystem_cfg.all_rates[i].number_of_hour = 1;
			lamount_per_hour = (float)((float)glSystem_cfg.all_rates[i].amount_per_hour/ (float)glSystem_cfg.all_rates[i].number_of_hour);
			lmax_time_allowed = (glSystem_cfg.all_rates[i].max_time_allowed)*60;//lseconds_left_in_current_schedule;//(glSystem_cfg.all_rates[i].max_time_allowed)*60;
			if(lcurrent_rate_index == PREPAY_PARKING )//pre-pay
			{
				//if(glSystem_cfg.bleed_into_next_schedule_enable == TRUE)	//In Prepay, forced to bleed into next schedule
				{
					////lmax_time_allowed = lseconds_left_in_current_schedule + (glSystem_cfg.all_rates[lnext_rate_index[0]].max_time_allowed*60);
					lmax_time_allowed = lseconds_left_in_current_schedule;//(glSystem_cfg.all_rates[i].max_time_allowed*60);
				}
			}
/*			else if((lcurrent_rate_index <= 17) && (glSystem_cfg.bleed_into_next_rate_enable == FALSE))
			{
				Debug_TextOut(2,"Rates: Enforcement Hours");
				lmax_time_allowed = lseconds_left_in_current_schedule;
			}*/
			max_time_in_current_rate = lmax_time_allowed;
			//Debug_Output2(0,"max_time_in_current_rate,%d",max_time_in_current_rate, 0);

			if(lcurrent_rate_index == PREPAY_PARKING )//pre-pay
			{
				lmax_time_allowed = lseconds_left_in_current_schedule + (glSystem_cfg.all_rates[lnext_rate_index[0]].max_time_allowed*60);
			}
			break;
		}
	}
	if((Rate_found == true) && (lnext_rate_index[0] != 0) && (glSystem_cfg.no_of_valid_rates >= 2))
	{
		for(m=0; m<MAX_SPLIT_RATES; m++)
		{
			for(i=0; i<glSystem_cfg.no_of_valid_rates; i++)
			{
				if((uint8_t)(glSystem_cfg.all_rates[i].Rate_index) == lnext_rate_index[m])
				{
					if(glSystem_cfg.all_rates[i].number_of_hour == 0)	//backward compatibility
						glSystem_cfg.all_rates[i].number_of_hour = 1;
					lnext_amount_per_hour[m] = (float)((float)glSystem_cfg.all_rates[i].amount_per_hour/ (float)glSystem_cfg.all_rates[i].number_of_hour);
					lnext_max_time_allowed[m] = glSystem_cfg.all_rates[i].max_time_allowed*60;
					max_time_in_current_rate += lnext_max_time_allowed[m];
					if(lcurrent_rate_index == PREPAY_PARKING )lmax_time_allowed = max_time_in_current_rate;
					//Debug_Output6(0,"111CA=%d,CM=%d,NA=%d,NM=%d",lamount_per_hour,lmax_time_allowed,lnext_amount_per_hour[m],lnext_max_time_allowed[m],0,0);
					break;
				}
			}
		}
	}
	else
	{
		if(lcurrent_rate_index == PREPAY_PARKING )//pre-pay
			{
				lmax_time_allowed = lseconds_left_in_current_schedule + (glSystem_cfg.all_rates[lnext_rate_index[0]].max_time_allowed*60);
				max_time_in_current_rate = lmax_time_allowed;
			}
	}
	//Debug_Output2(0,"max_time_in_current_rate222,%d",max_time_in_current_rate, 0);
	//Debug_Output2(0,"lmax_time_allowed222,%d",lmax_time_allowed, 0);
	//Debug_Output2(0,"lamount_per_hour222,%d,%d",lamount_per_hour, paid_amount);

	last_amount_used_for_calc_in_CC = lamount_per_hour;
	First_lmax_time_allowed_in_CC = lmax_time_allowed;

	if(Rate_found == true)
	{
		if(lamount_per_hour != 0)
		{
			if((glSystem_cfg.santa_cruz_spl_feature_enable == TRUE) && ((parking_time_left_on_meter[Current_Space_Id] + 60) < santa_cruz_previous_paid_time))
				lcheck_santa_cruz_allowed_time = santa_cruz_previous_paid_time + parking_time_left_on_meter[Current_Space_Id];
			else
				lcheck_santa_cruz_allowed_time = parking_time_left_on_meter[Current_Space_Id];

			//Debug_Output2(2,"Check allwd time=%ld, prv paid time=%ld",lcheck_santa_cruz_allowed_time,santa_cruz_previous_paid_time);
			//Debug_Output2(0,"lcheck_santa_cruz_allowed_time=%ld, santa_cruz_previous_paid_time=%ld",lcheck_santa_cruz_allowed_time,santa_cruz_previous_paid_time);

			if((lcheck_santa_cruz_allowed_time < lseconds_left_in_current_schedule) || (glSystem_cfg.bleed_into_next_schedule_enable == FALSE)|| (this_is_a_CC == true))	//If schedule overlap happens, then apply first rate only
			{
				if(((lcheck_santa_cruz_allowed_time)>=lmax_time_allowed) && (glSystem_cfg.bleed_into_next_rate_enable == TRUE)	// check if bleed rate is enabled and meter is already having time more than any progressive rate's max time
						&& (glSystem_cfg.bleed_into_next_rate_time_calc == TRUE))
				{
					amount_for_calc = 0;
					max_to_check = lmax_time_allowed - 60;
					for(p=0; p<MAX_SPLIT_RATES; p++)
					{
						if(lcheck_santa_cruz_allowed_time >= max_to_check)
						{
							amount_for_calc = lnext_amount_per_hour[p];
							next_rate_already_applied = true;
						}
						else	//Now it is not crossing the next max time, so current progressive rate will be applied
						{
							break;
						}
						//Debug_Output2(0,"Santa Cruz-01--C=%ld,%ld",max_to_check,lnext_max_time_allowed[p]);
						max_to_check += lnext_max_time_allowed[p];
					}
					earned_seconds = (uint64_t)(((float)3600/amount_for_calc) * (float)paid_amount);
					last_amount_used_for_calc = amount_for_calc;
					//Debug_Output1(0,"1 earned_seconds=%ld",earned_seconds);
					//Debug_Output2(0,"Santa Cruz-01--CCC=%ld,%ld",(uint64_t)(((float)3600/last_amount_used_for_calc) * (float)paid_amount),lnext_max_time_allowed[p]);
					//Debug_Output2(0,"Santa Cruz-01=%ld,%ld",earned_seconds,amount_for_calc);
				}
				else
				{
					//Debug_Output2(0,"222lamount_per_hour222,%d,%d",lamount_per_hour, paid_amount);

					earned_seconds = (uint64_t)(((float)3600/lamount_per_hour) * (float)paid_amount);
					last_amount_used_for_calc = lamount_per_hour;
					//Debug_Output1(0,"Santa Cruz-02=%ld",earned_seconds);
					//Debug_Output1(0,"2 earned_seconds=%ld",earned_seconds);
				}
			}
			else
			{
				earned_seconds = (uint64_t)(((float)3600/lamount_per_hour) * (float)paid_amount);
				last_amount_used_for_calc = lamount_per_hour;
				//Debug_Output1(0,"Santa Cruz-022=%ld",earned_seconds);
				//Debug_Output1(0,"3 earned_seconds=%ld",earned_seconds);
			}
			//coin_earned_seconds	=	earned_seconds;
			if(top_up_trans == true)	//to get Buys time only, pass top_up_trans=false
			{
				earned_seconds += (parking_time_left_on_meter[Current_Space_Id]);
				//Debug_Output2(0, "earned_seconds=%ld, parking_time_left_on_meter[Current_Space_Id]=%ld", earned_seconds, (parking_time_left_on_meter[Current_Space_Id] ));
				//Debug_Output1(0,"4 earned_seconds=%ld",earned_seconds);
			}

			//Debug_Output2(0, "222earned_seconds=%ld, parking_time_left_on_meter=%ld", earned_seconds, (parking_time_left_on_meter));
			if((glSystem_cfg.bleed_into_next_schedule_enable == TRUE)&&(lcurrent_rate_index < PREPAY_PARKING )&&(next_sched_rate_index[0] < PREPAY_PARKING))minutes_left_in_next_schedule = next_sched_minutes_left_in_current_schedule;
		}
		else
			earned_seconds = 0;
		//Debug_Output1(0,"5 earned_seconds=%ld",earned_seconds);
	}
	else
		earned_seconds = 0;
	//Debug_Output1(0,"6 earned_seconds=%ld",earned_seconds);

	if(glSystem_cfg.santa_cruz_spl_feature_enable == TRUE)
		lcheck_santa_cruz_allowed_time = santa_cruz_previous_paid_time + earned_seconds;
	else
		lcheck_santa_cruz_allowed_time = earned_seconds;

	//if(earned_seconds > lmax_time_allowed)	//Allow only max time in the current schedule
	//below using -60 because by the time they drop all the coins it would be some seconds delay and they have to drop one extra coin to satisfy
	//the below condition as it compares in seconds // so to avoid this and to get antifeed effect in first place so using it // vinay
	if((lcheck_santa_cruz_allowed_time >= (lmax_time_allowed-60)) || (Meter_Full[Current_Space_Id] == TRUE))	//Allow only max time in the current schedule
	{
		if((lcheck_santa_cruz_allowed_time <= lseconds_left_in_current_schedule) || ((glSystem_cfg.bleed_into_next_schedule_enable == FALSE)&&(lcurrent_rate_index < PREPAY_PARKING)) || ((this_is_a_CC == true)&&(lcurrent_rate_index < PREPAY_PARKING)&&(lseconds_left_in_current_schedule > First_lmax_time_allowed_in_CC)))	//If schedule overlap happens, then apply first rate only
		{
			if(glSystem_cfg.bleed_into_next_rate_enable == TRUE)	// check if bleed rate is enabled
			{
				if(glSystem_cfg.bleed_into_next_rate_time_calc == TRUE)//TODO: adjust earned time using both rates if enabled
				{
					//Debug_Output1(2,"31,%ld",earned_seconds);
					//max_to_check = lmax_time_allowed;
					//if((lnext_amount_per_hour[0] != 0) && (next_rate_already_applied == false) && (earned_seconds > lmax_time_allowed))
					if((lnext_amount_per_hour[0] != 0) && (next_rate_already_applied == false) && (earned_seconds > lmax_time_allowed))
					{
#if 1
						learned_seconds_to_small_rate = earned_seconds - lmax_time_allowed;//Find the excessive minute calculated with higher rate

						//Debug_Output1(0,"32,%d",learned_seconds_to_small_rate);
						lpaid_amount_second_rate = (uint64_t)((float)learned_seconds_to_small_rate * ((float)lamount_per_hour/(float)3600));
						Debug_Output1(2,"33,%d",lpaid_amount_second_rate);
						//Debug_Output1(0,"33--,%f",(float)((float)learned_seconds_to_small_rate * ((float)lamount_per_hour/(float)3600)));
						learned_seconds_second_rate = (uint64_t)(((float)3600 * (float)((float)learned_seconds_to_small_rate * ((float)lamount_per_hour/(float)3600)))/(float)lnext_amount_per_hour[0]);//Recalculate the minutes with new rate
						//Debug_Output2(0,"34,%d,%d",learned_seconds_second_rate, lnext_amount_per_hour[0]);
						last_amount_used_for_calc = lnext_amount_per_hour[0];
						last_amount_used_for_calc_in_CC = last_amount_used_for_calc;
						if((parking_time_left_on_meter[Current_Space_Id]) < lmax_time_allowed)
						{
							earned_seconds -= learned_seconds_to_small_rate;	//deduct those minutes and add recalculated minutes
							//Debug_Output1(0,"35,%ld",earned_seconds);
						}
						earned_seconds += learned_seconds_second_rate;
						//Debug_Output1(0,"3,%ld",earned_seconds);
						//Debug_Output1(0,"7 earned_seconds=%ld",earned_seconds);
						max_to_check = lmax_time_allowed+lnext_max_time_allowed[0]; //card payment check this parameter vinay
						//Debug_Output2(0,"3A,%d,%d",lmax_time_allowed,lnext_max_time_allowed[0]);
						//Debug_Output1(0,"3B,%d",earned_seconds);
#endif

						for(n=0; n<MAX_SPLIT_RATES; n++)
						{
							if((lnext_amount_per_hour[n+1] != 0) && (earned_seconds > max_to_check))//If still more than second rate
							{
								learned_seconds_to_small_rate = earned_seconds - max_to_check;//Find the excessive minute calculated with higher rate

								//Debug_Output1(0,"36,%d",learned_seconds_to_small_rate);
								lpaid_amount_second_rate = (uint64_t)((float)learned_seconds_to_small_rate * ((float)lnext_amount_per_hour[n]/(float)3600));
								//Debug_Output1(0,"37,%d",lpaid_amount_second_rate);
								learned_seconds_second_rate = (uint64_t)(((float)3600 * (float)((float)learned_seconds_to_small_rate * ((float)lnext_amount_per_hour[n]/(float)3600)))/(float)lnext_amount_per_hour[n+1]);//Recalculate the minutes with new rate
								//Debug_Output2(0,"38,%d,%d",learned_seconds_second_rate, lnext_amount_per_hour[n+1]);
								last_amount_used_for_calc = lnext_amount_per_hour[n+1];
								last_amount_used_for_calc_in_CC = last_amount_used_for_calc;
								if((parking_time_left_on_meter[Current_Space_Id]) < max_to_check)
								{
									earned_seconds -= learned_seconds_to_small_rate;	//deduct those minutes and add recalculated minutes
									//Debug_Output1(0,"39,%ld",earned_seconds);
								}
								earned_seconds += learned_seconds_second_rate;
								//Debug_Output1(0,"3A,%ld",earned_seconds);
								//Debug_Output1(0,"8 earned_seconds=%ld",earned_seconds);
							}
							else
							{
								if(earned_seconds <= lseconds_left_in_current_schedule) First_Time_CC_Entry_For_Next_Schedule = FALSE;
								break;
							}
							max_to_check += lnext_max_time_allowed[n+1];
							//Debug_Output2(0,"3AA,%ld,%ld",max_to_check,lnext_max_time_allowed[n]);
							//Debug_Output2(0,"3AAA,%ld,%ld",max_to_check,lnext_max_time_allowed[n+1]);
						}
					}

/*					if(lcheck_santa_cruz_allowed_time > lseconds_left_in_current_schedule)
					{
						if(earned_seconds > (lseconds_left_in_current_schedule - lnext_max_time_allowed[0]))last_amount_used_for_calc = lnext_amount_per_hour[0];
						if(earned_seconds > (lseconds_left_in_current_schedule - lnext_max_time_allowed[1]))last_amount_used_for_calc = lnext_amount_per_hour[1];
						if(earned_seconds > (lseconds_left_in_current_schedule - lnext_max_time_allowed[2]))last_amount_used_for_calc = lnext_amount_per_hour[2];
						if(earned_seconds > (lseconds_left_in_current_schedule - lnext_max_time_allowed[3]])last_amount_used_for_calc = lnext_amount_per_hour[3];
						if(earned_seconds > (lseconds_left_in_current_schedule - lnext_max_time_allowed[4]))last_amount_used_for_calc = lnext_amount_per_hour[4];
					}*/

					if((earned_seconds > max_time_in_current_rate) && (glSystem_cfg.bleed_into_next_schedule_enable == FALSE))//Do not allow above this time
						earned_seconds = max_time_in_current_rate;
					//Debug_Output1(0,"Santa Cruz-03=%ld",earned_seconds);
					//Debug_Output1(0,"9 earned_seconds=%ld",earned_seconds);
				}
				else
				{
					earned_seconds = earned_seconds;	//return the calculated earned time, no need to change
					//Debug_Output1(0,"4,%ld",earned_seconds);
					//Debug_Output1(2,"Santa Cruz-04=%ld",earned_seconds);
					//Debug_Output1(0,"10 earned_seconds=%ld",earned_seconds);
				}

			}
		}
		/*******************************************************/
		//For antifeed to work in all enforced and prepay schedules //vinay
		if((Meter_Full[Current_Space_Id] == TRUE) && (glSystem_cfg.ANTI_FEED_ENABLE == TRUE) && ((current_rate_index < 20) || (current_rate_index == PREPAY_PARKING)))
		{
			//if((max_to_check != 0) && (lcheck_santa_cruz_allowed_time >= max_to_check) && (Meter_Full_Flg[Current_Space_Id] == TRUE))
			//if(Meter_Full[Current_Space_Id] == TRUE)
			{
				//Debug_TextOut(0,"Anti Feed Start 1");
				if(paid_amount != 0xFF)
					Max_Amount_In_current_rate = TRUE;
				//earned_seconds = lseconds_left_in_current_schedule;
				//earned_seconds = lmax_time_allowed; //now after reaching max time it is still allowing for one more payment to start anti feed, to start antifeed when it reached max time using the above stmt //vinay
				//Debug_Output1(0,"earned_seconds,%ld",earned_seconds);
				if((glSystem_cfg.ANTI_FEED_ENABLE == TRUE) && (this_is_before_update == true) && (glLast_reported_space_status[Current_Space_Id] == 1))	//Anti Feed feature enabled in config
				{
					ANTI_FEED_in_effect[Current_Space_Id] = true;
					push_event_to_cache(UDP_EVTTYP_ANTI_FEED_START);
					Debug_Output1(0,"Anti Feed Start-space [%d]",Current_Space_Id);
				}
			}
		}
	}

	if(earned_seconds > lseconds_left_in_current_schedule)
	{
		uint8_t new_start_value = 0;
		uint8_t new_start_value_temp = 0;

		if((glSystem_cfg.bleed_into_next_schedule_enable == TRUE) || (lcurrent_rate_index == PREPAY_PARKING))	// check if bleed rate is enabled	//In Prepay, forced to bleed into next schedule
		{
			//For schedule overlapping, no need to recalculate time according to next rate
			earned_seconds = earned_seconds;	//return the calculated earned time, no need to change
			//Debug_Output1(0,"11 earned_seconds=%ld",earned_seconds);
			if((lcurrent_rate_index == PREPAY_PARKING) && (earned_seconds > lmax_time_allowed))
			{
				//earned_seconds -= lseconds_left_in_current_schedule;	//commented to resolve issue with payment in prepay
			}
			if(last_amount_used_for_calc_in_CC == 0) last_amount_used_for_calc_in_CC = last_amount_used_for_calc;
			if(coin_insert_ok) last_amount_used_for_calc_in_CC = last_amount_used_for_calc; //for coin insertion time this will take -- VK
			//Debug_Output2(0,"6,%ld,%d",earned_seconds,last_amount_used_for_calc_in_CC);
			//Denver Changes
			//Populate next schedules rates also
			if((next_sched_rate_index[0] > PREPAY_PARKING)&&(lcurrent_rate_index < PREPAY_PARKING))
			{
				if(paid_amount != 0xFF)
					Max_Amount_In_current_rate = TRUE;
				earned_seconds = lseconds_left_in_current_schedule;
				//Debug_Output1(0,"12 earned_seconds=%ld",earned_seconds);
				if((glSystem_cfg.ANTI_FEED_ENABLE == TRUE) && (this_is_before_update == true) && (glLast_reported_space_status[Current_Space_Id] == 1))	//Anti Feed feature enabled in config
					{
						ANTI_FEED_in_effect[Current_Space_Id] = true;
						//Debug_TextOut(0,"Anti Feed in effect1");
						Debug_Output1(0,"Anti Feed in Effect-space [%d]",Current_Space_Id);
					}
				goto skip;
			}

			//lnext_amount_per_hour[0] = last_amount_used_for_calc;
			//lnext_max_time_allowed[0] = lseconds_left_in_current_schedule;


			if((lcurrent_rate_index < PREPAY_PARKING )&&(glSystem_cfg.bleed_into_next_rate_time_calc == TRUE)&&(next_sched_rate_index[0] != 0)&&(glSystem_cfg.no_of_valid_rates >= 2))
				max_time_in_current_rate = 0;
			for(m=0; m<MAX_SPLIT_RATES; m++)
			{

				for(i=0; i<glSystem_cfg.no_of_valid_rates; i++)
				{
					if((uint8_t)(glSystem_cfg.all_rates[i].Rate_index) == next_sched_rate_index[m])
					{
						if(glSystem_cfg.all_rates[i].number_of_hour == 0)	//backward compatibility
							glSystem_cfg.all_rates[i].number_of_hour = 1;
						lnext_amount_per_hour[m] = (float)((float)glSystem_cfg.all_rates[i].amount_per_hour/ (float)glSystem_cfg.all_rates[i].number_of_hour);
						lnext_max_time_allowed[m] = glSystem_cfg.all_rates[i].max_time_allowed*60;
						if((lcurrent_rate_index < PREPAY_PARKING )&&(glSystem_cfg.bleed_into_next_rate_time_calc == TRUE))max_time_in_current_rate += lnext_max_time_allowed[m];
						/*Added Newly to ignore Prepay time in next schedule*/
						//if((next_sched_rate_index[0]==PREPAY_PARKING)&&(m==0)) max_time_in_current_rate -= lnext_max_time_allowed[m];
						//the above condition is not working properly for IOA customer, so added below condition to satisify present and IOA requirement //vinay
						if( ((next_sched_rate_index[0]==PREPAY_PARKING) && (m==0)) && ((glSystem_cfg.bleed_into_next_rate_time_calc == TRUE) || (glSystem_cfg.bleed_into_next_rate_enable == TRUE)) )
						        max_time_in_current_rate -= lnext_max_time_allowed[m];
						//max_time_in_current_rate += lnext_max_time_allowed[m];
						//Debug_Output6(0,"222CA=%ld,CM=%ld,NA=%ld,NM=%ld",lamount_per_hour,lmax_time_allowed,lnext_amount_per_hour[m],lnext_max_time_allowed[m],0,0);
						break;
					}
				}
			}

			//Debug_Output2(0,"6B BEFORE,%ld,%ld",lnext_max_time_allowed[0], lnext_max_time_allowed[1]);
			//Debug_Output2(0,"6B BEFORE,%ld,%ld",lseconds_left_in_current_schedule, lnext_max_time_allowed[0]);
			//Debug_Output2(0,"6B BEFORE,%ld",max_time_in_current_rate,0);

			max_to_check = lseconds_left_in_current_schedule;

			if(lcurrent_rate_index < PREPAY_PARKING )
				{
					minutes_left_in_next_schedule = next_sched_minutes_left_in_current_schedule;//max_time_in_current_rate;//

					if((max_time_in_current_rate>((minutes_left_in_current_schedule+minutes_left_in_next_schedule))*60))
						max_time_in_current_rate = ((minutes_left_in_current_schedule+minutes_left_in_next_schedule)*60);

					//Debug_Output2(0,"6B BB,%ld,%ld",minutes_left_in_current_schedule, minutes_left_in_next_schedule);
					//Debug_Output2(0,"6B BBB,%ld,%ld",minutes_left_in_current_schedule*60, minutes_left_in_next_schedule*60);
					//Debug_Output2(0,"6B BBBB,%ld,%ld",max_time_in_current_rate,((minutes_left_in_current_schedule + minutes_left_in_next_schedule)*60));
				}

			if((lcurrent_rate_index < PREPAY_PARKING )&&(glSystem_cfg.bleed_into_next_rate_enable == TRUE) && (next_sched_rate_index[0] != 0) && (glSystem_cfg.no_of_valid_rates >= 2))
			{
				lamount_per_hour = 0;
				for(i=0; i<glSystem_cfg.no_of_valid_rates; i++)
					{
						if((uint8_t)(glSystem_cfg.all_rates[i].Rate_index) == next_sched_rate_index[0])
						{
							lamount_per_hour = (float)((float)glSystem_cfg.all_rates[i].amount_per_hour/ (float)glSystem_cfg.all_rates[i].number_of_hour);
						}
					}
			}


			if(((glSystem_cfg.bleed_into_next_rate_enable == TRUE)	// check if bleed rate is enabled and meter is already having time more than any progressive rate's max time
									&& (glSystem_cfg.bleed_into_next_rate_time_calc == TRUE))||(lcurrent_rate_index == PREPAY_PARKING))
			{

				if(next_sched_rate_index[0]==PREPAY_PARKING) new_start_value = 1;
#if 1
				//Debug_Output2(0,"lseconds_left_in_current_schedule=%ld,First_lmax_time_allowed_in_CC=%ld",lseconds_left_in_current_schedule, First_lmax_time_allowed_in_CC);
				if(lseconds_left_in_current_schedule >= First_lmax_time_allowed_in_CC)
				{
					//Debug_Output2(0,"32A-C,%ld,%ld",earned_seconds, max_to_check);

					////learned_seconds_to_small_rate = earned_seconds - max_to_check;//Find the excessive minute calculated with higher rate

					if(parking_time_left_on_meter[Current_Space_Id] > max_to_check)
					{
						learned_seconds_to_small_rate = earned_seconds - parking_time_left_on_meter[Current_Space_Id];
						//Debug_Output1(0,"69B-T,%ld",learned_seconds_to_small_rate);
					}
					else
					{
						learned_seconds_to_small_rate = earned_seconds - max_to_check;//Find the excessive minute calculated with higher rate
						//Debug_Output1(0,"69C-T,%ld",learned_seconds_to_small_rate);
					}

					//Debug_Output1(0,"32-C,%ld",learned_seconds_to_small_rate);
					lpaid_amount_second_rate = (uint64_t)((float)learned_seconds_to_small_rate * ((float)last_amount_used_for_calc_in_CC/(float)3600));
					//Debug_Output1(0,"33-C,%ld",lpaid_amount_second_rate);
					learned_seconds_second_rate = (uint64_t)(((float)3600 * (float)((float)learned_seconds_to_small_rate * ((float)last_amount_used_for_calc_in_CC/(float)3600)))/(float)lnext_amount_per_hour[0]);//Recalculate the minutes with new rate
					//Debug_Output2(0,"34-C,%ld,%ld",learned_seconds_second_rate, lnext_amount_per_hour[0]);

					///if((parking_time_left_on_meter) < max_to_check)
					{
						earned_seconds -= learned_seconds_to_small_rate;	//deduct those minutes and add recalculated minutes
						//Debug_Output1(0,"35-C,%ld",earned_seconds);
					}
					earned_seconds += learned_seconds_second_rate;
					//Debug_Output1(0,"3-C,%ld",earned_seconds);
					//Debug_Output1(0,"13 earned_seconds=%ld",earned_seconds);
					if(next_sched_rate_index[0]==PREPAY_PARKING)
						{
							new_start_value = 2;
							new_start_value_temp = 1;
							max_to_check = lseconds_left_in_current_schedule + lnext_max_time_allowed[1] ;
						}
					else
						{
							new_start_value = 1;
							new_start_value_temp = 0;
							max_to_check = lseconds_left_in_current_schedule + lnext_max_time_allowed[0];
						}

					//Debug_Output2(0,"3A-C,%ld,%ld",max_to_check,lnext_max_time_allowed[0]);
					//Debug_Output1(0,"3B-C,%ld",earned_seconds);

					////if(earned_seconds > lseconds_left_in_current_schedule) Stop_CC_second_Time_Entry = TRUE;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					if((lnext_amount_per_hour[n+1] != 0) && (earned_seconds > max_to_check))//If still more than second rate
					{
						if(parking_time_left_on_meter[Current_Space_Id] > max_to_check)
						{
							learned_seconds_to_small_rate = earned_seconds - parking_time_left_on_meter[Current_Space_Id];
							//Debug_Output1(0,"69B-T,%ld",learned_seconds_to_small_rate);
						}
						else
						{
							learned_seconds_to_small_rate = earned_seconds - max_to_check;//Find the excessive minute calculated with higher rate
							//Debug_Output1(0,"69C-T,%ld",learned_seconds_to_small_rate);
						}
							{
								lpaid_amount_second_rate = (uint64_t)((float)learned_seconds_to_small_rate * ((float)lnext_amount_per_hour[new_start_value_temp]/(float)3600));
								learned_seconds_second_rate = (uint64_t)(((float)3600 * (float)((float)learned_seconds_to_small_rate * ((float)lnext_amount_per_hour[new_start_value_temp]/(float)3600)))/(float)lnext_amount_per_hour[new_start_value_temp+1]);//Recalculate the minutes with new rate
								//Debug_Output2(0,"68-T,%ld,%ld",learned_seconds_second_rate, lnext_amount_per_hour[new_start_value_temp+1]);
								//Debug_Output2(0,"68A-T,%ld,%ld",lpaid_amount_second_rate, lnext_amount_per_hour[new_start_value_temp]);
							}

						////}

						////if((parking_time_left_on_meter < max_to_check) && (this_is_a_CC == false))
							{
								//Debug_Output2(0,"parking_time_left_on_meter=%ld,max_to_check=%ld",parking_time_left_on_meter, max_to_check);
								//Debug_Output2(0,"learned_remain_first_rate=%ld,learned_second_rate=%ld",(max_to_check - parking_time_left_on_meter), learned_seconds_second_rate);
							}
						//if((parking_time_left_on_meter) < max_to_check)
						{
							earned_seconds -= learned_seconds_to_small_rate;	//deduct those minutes and add recalculated minutes
							//Debug_Output1(0,"69-T,%ld",earned_seconds);
						}
						earned_seconds += learned_seconds_second_rate;
						//Debug_Output1(0,"6A-T,%ld",earned_seconds);
						//Debug_Output1(0,"14 earned_seconds=%ld",earned_seconds);
						max_to_check += lnext_max_time_allowed[new_start_value_temp+1];
					}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				}
				else
				{
					First_Time_CC_Entry_For_Next_Schedule = TRUE;
					new_start_value = 0;
					max_to_check = lseconds_left_in_current_schedule + lnext_max_time_allowed[0];
					if(lcurrent_rate_index == PREPAY_PARKING) max_to_check = lseconds_left_in_current_schedule + lnext_max_time_allowed[0];
				}

#endif
				for(n=new_start_value; n<MAX_SPLIT_RATES; n++)
				{
					//if(next_sched_rate_index[0]==PREPAY_PARKING) n+=1;
					//Debug_Output2(0,"68D,%ld,%ld",earned_seconds, max_to_check);
					//Debug_Output2(0,"68E,%ld,%ld",lnext_amount_per_hour[n+1], n);
					//Debug_Output2(0,"68F,%ld,%ld",max_to_check,(max_to_check - lnext_max_time_allowed[n - 1]));
					//Debug_Output2(0,"68G,%ld,%ld",(lnext_max_time_allowed[n]),(lnext_max_time_allowed[n-1]));
					if((lnext_amount_per_hour[n+1] != 0) && (earned_seconds > max_to_check))//If still more than second rate
					{
						if(parking_time_left_on_meter[Current_Space_Id] > max_to_check)
						{
							learned_seconds_to_small_rate = earned_seconds - parking_time_left_on_meter[Current_Space_Id];
							//Debug_Output1(0,"69B,%ld",learned_seconds_to_small_rate);
						}
						else
						{
							learned_seconds_to_small_rate = earned_seconds - max_to_check;//Find the excessive minute calculated with higher rate
							//Debug_Output1(0,"69C,%ld",learned_seconds_to_small_rate);
						}
							{
								lpaid_amount_second_rate = (uint64_t)((float)learned_seconds_to_small_rate * ((float)lnext_amount_per_hour[n]/(float)3600));
								learned_seconds_second_rate = (uint64_t)(((float)3600 * (float)((float)learned_seconds_to_small_rate * ((float)lnext_amount_per_hour[n]/(float)3600)))/(float)lnext_amount_per_hour[n+1]);//Recalculate the minutes with new rate
								//Debug_Output2(0,"68,%ld,%ld",learned_seconds_second_rate, lnext_amount_per_hour[n+1]);
								//Debug_Output2(0,"68A,%ld,%ld",lpaid_amount_second_rate, lnext_amount_per_hour[n]);
							}

						////}

						////if((parking_time_left_on_meter < max_to_check) && (this_is_a_CC == false))
							{
								//Debug_Output2(0,"parking_time_left_on_meter=%ld,max_to_check=%ld",parking_time_left_on_meter, max_to_check);
								//Debug_Output2(0,"learned_remain_first_rate=%ld,learned_second_rate=%ld",(max_to_check - parking_time_left_on_meter), learned_seconds_second_rate);
							}
						//if((parking_time_left_on_meter) < max_to_check)
						{
							earned_seconds -= learned_seconds_to_small_rate;	//deduct those minutes and add recalculated minutes
							//Debug_Output1(0,"69,%ld",earned_seconds);
						}
						earned_seconds += learned_seconds_second_rate;
						//Debug_Output1(0,"6A,%ld",earned_seconds);
						//Debug_Output1(0,"15 earned_seconds=%ld",earned_seconds);
					}
					else
					{
						break;
					}

					max_to_check += lnext_max_time_allowed[n+1];

					//Debug_Output2(0,"6AA,%ld,%ld",max_to_check,lnext_max_time_allowed[n]);
					//Debug_Output2(0,"6AAA,%ld,%ld",lnext_amount_per_hour[n],lnext_amount_per_hour[n+1]);
					//Debug_Output2(0,"6AAAA,%ld,%ld",lnext_amount_per_hour[n+2],lnext_amount_per_hour[n+3]);
					//Debug_Output2(0,"6AAAAA,%ld,%ld",max_to_check,(max_to_check - lnext_max_time_allowed[n - 1]));
				}

			}
			else
			{
				earned_seconds	=	0;
				if(lamount_per_hour>0)
					earned_seconds = (uint64_t)(((float)3600/lamount_per_hour) * (float)paid_amount);
				earned_seconds += (parking_time_left_on_meter[Current_Space_Id]);
				//Debug_Output1(0,"16 earned_seconds=%ld",earned_seconds);
			}
			skip:
				if((lcurrent_rate_index == PREPAY_PARKING)&&(earned_seconds>=(max_time_in_current_rate/* - 60*/)))
					if(paid_amount != 0xFF)
						Max_Amount_In_current_rate = TRUE;
		}
		else
		{
		    //Debug_Output1(0,"17 paid_amount=%ld\r\n",paid_amount);
			if(paid_amount != 0xFF)
				Max_Amount_In_current_rate = TRUE;
			earned_seconds = lseconds_left_in_current_schedule;
			//Debug_Output1(0,"17 earned_seconds=%ld\r\n",earned_seconds);
			if((glSystem_cfg.ANTI_FEED_ENABLE == TRUE) && (this_is_before_update == true) && (glLast_reported_space_status[Current_Space_Id] == 1))	//Anti Feed feature enabled in config
			{
				ANTI_FEED_in_effect[Current_Space_Id] = true;
				//Debug_TextOut(0,"Anti Feed in effect");
				Debug_Output1(0,"Anti Feed in Effect-space [%d]",Current_Space_Id);
			}
		}
	}
	//LNGSIT-451,453: commented below to fix exact hour issue
/*	else if((earned_seconds == lseconds_left_in_current_schedule))
	{
		if((glSystem_cfg.bleed_into_next_rate_enable == FALSE) && (paid_amount != 0xFF))
		{
			Max_Amount_In_current_rate = TRUE;
		}
		//Debug_Output1(0,"8,%ld",earned_seconds);
	}*/
	//Debug_Output2(0,"lminutes:%ld earned:%ld\r\n",lseconds_left_in_current_schedule,earned_seconds);

	//Debug_Output1(2,"Santa Cruz-05=%ld",earned_seconds);

	if(max_time_in_current_rate > 86400)  //VT 24 hour max for max_time_in current_schedule
		max_time_in_current_rate = 86400;

	//Debug_Output2(0,"LAST,earned_seconds=%ld,max_time_in_current_rate=%ld",earned_seconds,max_time_in_current_rate);

	if((earned_seconds >= max_time_in_current_rate))// && (glSystem_cfg.bleed_into_next_rate_enable == FALSE))
		earned_seconds = max_time_in_current_rate;

	Debug_Output1(0,"Final Earned_seconds=%ld\r\n",earned_seconds);

	/*if(paid_amount >= MAX_ALLOWED_AMOUNT_IN_A_DAY)	//for Denver, buys full day if $65 is paid
	{
		current_minute_of_day = ((rtc_c_calender.hours)*60) + (rtc_c_calender.minutes);
		if(((current_minute_of_day + (earned_seconds/60)) <= 1440))	//give time till midnight only if buys is less than that, otherwise give whatever is earned
		{
			earned_seconds = ((1440 - current_minute_of_day)*60 + 60);
		}
	}*/

/*	current_minute_of_day = ((rtc_c_calender.hours)*60) + (rtc_c_calender.minutes);
	if((current_minute_of_day >= 480) && (glSystem_cfg.bleed_into_next_rate_enable == TRUE))	//New Payment starts in escalating schedule
	{
		if((current_minute_of_day + (earned_seconds/60)) >= 1920)	//The Payment is crossing over to next day's 8AM rate
		{
			Max_Amount_In_current_rate = TRUE;
			earned_seconds = (1920 - current_minute_of_day)*60;
			//Debug_Output2(2,"Stop Crossover to next day's escalation-05=%d, %ld", current_minute_of_day, earned_seconds);
			if(this_is_a_CC == FALSE)
			{
				Debug_TextOut(0,"Meter Full");
				MeterFull_Screen();
			}
		}
	}*/

	if(earned_seconds > 86400)	//Hard Check for never allowing payment more than 24 hours
		earned_seconds = 86400;


	return earned_seconds;
}

/**************************************************************************/
//! This function calculates the amount from the parking time field passed into this function
//! \param Remainingtime
//! \return float amount
/**************************************************************************/
float Refund_get_earned_amount(float Remainingtime)
{
	uint32_t lmax_time_allowed = 0;
	uint8_t lcurrent_rate_index = 0, i;
	uint8_t Rate_found = false;
	uint16_t paid_amount = 0;
	float lamount_per_hour = 0, earned_minutes = 0;

	lcurrent_rate_index = current_rate_index;	//volatile rate index; so assign to local variable
	//lminutes_left_in_current_schedule = minutes_left_in_current_schedule;
	earned_minutes = Remainingtime / 60;  //sec to min

	for(i=0; i<glSystem_cfg.no_of_valid_rates; i++)
	{
		if((uint8_t)(glSystem_cfg.all_rates[i].Rate_index) == lcurrent_rate_index)
		{
			Rate_found = true;
			if(glSystem_cfg.all_rates[i].number_of_hour == 0)	//backward compatibility
				glSystem_cfg.all_rates[i].number_of_hour = 1;
			lamount_per_hour = (float)((float)glSystem_cfg.all_rates[i].amount_per_hour/ (float)glSystem_cfg.all_rates[i].number_of_hour);
			lmax_time_allowed = glSystem_cfg.all_rates[i].max_time_allowed;
			max_time_in_current_rate = lmax_time_allowed;
			break;
		}
	}
	if(Rate_found == true)
	{
		if(lamount_per_hour != 0)
		{
		      paid_amount = (uint16_t)((float)earned_minutes/(((float)60/lamount_per_hour)));  //only values after one decimal point  will be round off.
		}
	}
	else
		paid_amount = 0;

	if((paid_amount % MIN_REFUND_AMT_MULTIPLIER) != 0)
		paid_amount = ((paid_amount/MIN_REFUND_AMT_MULTIPLIER))*MIN_REFUND_AMT_MULTIPLIER;	//rounding off to the previous whole multiple of 5
	//Debug_Output2(0,"lminutes:%d earned:%d\r\n",lminutes_left_in_current_schedule,earned_minutes);
	return paid_amount;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
