//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_rtc.c
//
//****************************************************************************

#include "LibG2_rtc.h"

extern uint8_t 							RTC_String[/*MAXBYTESINROW*/50+1];
extern GPRS_General_Sync_Response      	gprs_general_sync_response;
extern uint8_t                     		glDo_rtc_update;
extern GPRSSystem  						glSystem_cfg;
extern uint8_t          				DLST_CURRENT_SLOT;
//extern uint32_t							RTC_Update;
extern RTC_ParkingClk					RTC_ParkingTime;
extern uint8_t         					glComm_response[GPRS_GEN_BUF_SZ_SMALL];

RTC_C_Calendar 							rtc_c_calender;
uint8_t         						DLST_HOUR_CHANGE = 0xFF;        //17-05-12:DLST
uint8_t         						set_DLST         = false;       //17-05-12:DLST
uint8_t         						glDLST_month     = 12, glDLST_day = 31, glDLST_hour = 20;
uint16_t        						glDLST_year      = 99;
uint8_t         						DLST_FLAG_CHECK  = 0xFF;
//uint8_t         						glDLST_FLAGS[10];//it is not used in this code //vinay

/**************************************************************************/
/*  Name        : RTC_epoch_now                                           */
/*  Parameters  : void                                                    */
/*  Returns     : uint32_t                                                */
/*  Function    : Get Time stamp for OLT                                  */
/*------------------------------------------------------------------------*/
uint32_t RTC_epoch_now()
{
	time_set_t       RTC_epoch_time_set;
	uint32_t         current_RTCepoch_time;

	rtc_c_calender = RTC_C_getCalendarTime();

	RTC_epoch_time_set.year     = (uint8_t) ((rtc_c_calender.year) - REFERENCE_YR);
	RTC_epoch_time_set.month    = rtc_c_calender.month;
	RTC_epoch_time_set.date     = rtc_c_calender.dayOfmonth;
	RTC_epoch_time_set.hours    = rtc_c_calender.hours;
	RTC_epoch_time_set.minutes  = rtc_c_calender.minutes;
	RTC_epoch_time_set.seconds  = rtc_c_calender.seconds;

	if(RTC_epoch_time_set.month > 12)
	{
		RTC_epoch_time_set.year++;
		RTC_epoch_time_set.month = 1;
	}

	current_RTCepoch_time=RTC_from_year(&RTC_epoch_time_set);
	return current_RTCepoch_time; //532000957;
}

/**************************************************************************/
/*  Name        : get_offset_days                                         */
/*  Parameters  : uint16_t      	                                      */
/*  Returns     : uint16_t		                                          */
/*  Function    : get total number of offs days                           */
/*------------------------------------------------------------------------*/
uint16_t get_offset_days(uint16_t c_year)
{
	uint16_t tmp_int;
	uint16_t a,b,c;

	/* Rule of leap year: a year is a leap year
    only if it is divisible by 4. However,
    if it’s also divisible by 100 then it cannot
    be a leap year unless it’s divisible by 400. */

	a = (c_year-1)/4   - (REFERENCE_YR-1)/4;   //get number of leap year since 2000
	b = (c_year-1)/100 - (REFERENCE_YR-1)/100;
	c = (c_year-1)/400 - (REFERENCE_YR-1)/400;
	tmp_int = a + c - b;
	return(tmp_int);
}

/**************************************************************************/
/*  Name        : check_leep_year                                         */
/*  Parameters  : uint16_t		                                          */
/*  Returns     : uint8_t		                                          */
/*  Function    : Check if Leap Year                                      */
/*------------------------------------------------------------------------*/
uint8_t check_leep_year(uint16_t c_year)
{
	uint8_t tmp_byte;

	if((c_year%400)== 0)
	{
		tmp_byte = LEAP_YEAR;
		return(tmp_byte);
	}

	if((c_year%100)==0)
	{
		tmp_byte = NO_LEAP_YEAR;
		return(tmp_byte);
	}

	if((c_year%4)==0)
	{
		tmp_byte = LEAP_YEAR;
		return(tmp_byte);
	}

	tmp_byte = NO_LEAP_YEAR;
	return(tmp_byte);
}

/**************************************************************************/
/*  Name        : month_length                                            */
/*  Parameters  : uint16_t,uint8_t                              	      */
/*  Returns     : uint8_t		                                          */
/*  Function    : number of days in the month                             */
/*------------------------------------------------------------------------*/
uint8_t month_length(uint16_t c_year,uint8_t c_month)
{
	uint8_t tmp_lengh;
	uint8_t tmp_year;
	//uint8_t tmp_month;

	if (c_month == 2)
	{
		tmp_year = check_leep_year(c_year);

		if(tmp_year == LEAP_YEAR)
		{
			tmp_lengh = 29;
		}
		else
		{
			tmp_lengh = 28;
		}
	}
	else
	{
		if(c_month < 8)
		{
			if((c_month%2)==0)
			{
				tmp_lengh = 30;
			}
			else
			{
				tmp_lengh = 31;
			}
		}
		else
		{
			if((c_month%2)==0)
			{
				tmp_lengh = 31;
			}
			else
			{
				tmp_lengh = 30;
			}
		}
	}

	return(tmp_lengh);
}

/**************************************************************************/
/*  Name        : RTC_from_year                                           */
/*  Parameters  : time_set_t                                              */
/*  Returns     : uint32_t                                                */
/*  Function    : return an absolute time in seconds                      */
/*------------------------------------------------------------------------*/
uint32_t RTC_from_year(time_set_t *time_set_1)
{
	uint8_t i;
	uint8_t tmp_mh;
	uint16_t tmp_yr,tmp_offset;
	uint32_t tmp_RTC,tmp_days,tmp_long;

	tmp_yr = (time_set_1 -> year) + REFERENCE_YR;
	tmp_mh = time_set_1 -> month;

	//get number of days from 2000.1.1 to current year Jan.1
	tmp_days   = ((uint32_t)(tmp_yr - REFERENCE_YR))* 365;
	tmp_offset = get_offset_days(tmp_yr);
	tmp_days   = tmp_days + tmp_offset;

	// get number of days from current year Jan.1 to current day
	tmp_long = 0;
	for(i=1; i<tmp_mh; i++)
	{
		tmp_long = month_length(tmp_yr,i) + tmp_long;
	}

	// number of days from 2000.1.1 to current days
	tmp_days = tmp_days + tmp_long + ((time_set_1 -> date) - 1);

	tmp_RTC = tmp_days * SECS_IN_ADAY
			+ ((uint32_t)(time_set_1 -> hours)) * SECS_IN_HOUR
			+ ((uint32_t)(time_set_1 -> minutes)) * SECS_IN_MINT
			+ (uint32_t)(time_set_1  -> seconds);

	return(tmp_RTC);
}

void SetMSP_RTC(uint8_t default_rtc)
{
	time_set_t Sec_RTC;
	if(default_rtc == TRUE)//boot up default RTC
	{
		RTC_C_disableInterrupt(RTC_C_TIME_EVENT_INTERRUPT);
		//Flash_Batch_Flag_Read();
		Flash_RTC_ParkingClock_Storage(false, false);	//RTC append in flash every minute
		if (RTC_ParkingTime.RTC_Update==0xffffffff)
		{ //Updated default time from 1/1/2017 to 1/1/2021
			rtc_c_calender.seconds = 10;//0;
			rtc_c_calender.minutes = 10;//0;
			rtc_c_calender.hours   = 10;
			rtc_c_calender.dayOfWeek = 6;//1;
			rtc_c_calender.dayOfmonth = 1 ;
			rtc_c_calender.month = 1;
			rtc_c_calender.year = 2022;//2017;
		}
		else
		{
			year_from_RTC(RTC_ParkingTime.RTC_Update,&Sec_RTC);
			rtc_c_calender.seconds =Sec_RTC.seconds;
			rtc_c_calender.minutes = Sec_RTC.minutes+5;//balu sir told to add 5 if reset happens before 10mins window of writting to flash //vinay
			rtc_c_calender.hours   = Sec_RTC.hours;
			rtc_c_calender.dayOfWeek = Sec_RTC.day_of_week;
			rtc_c_calender.dayOfmonth = Sec_RTC.date ;
			rtc_c_calender.month = Sec_RTC.month;
			rtc_c_calender.year = Sec_RTC.year + REFERENCE_YR;
		}
		RTC_C_holdClock();
		RTC_C_initCalendar(&rtc_c_calender,RTC_C_FORMAT_BINARY);
		//RTC_C_setCalibrationData(RTC_C_CALIBRATION_UP1PPM, 240);
		RTC_C_setCalendarEvent(RTC_C_CALENDAREVENT_MINUTECHANGE);
		RTC_C_startClock();
		//RTC_C_registerInterrupt(RTC_C_IRQHandler);
		NVIC->ISER[0] = 1 << ((RTC_C_IRQn) & 31);// Enable RTC_C_IRQn interrupt in NVIC module
		RTC_C_enableInterrupt(RTC_C_TIME_EVENT_INTERRUPT);
	}
	else if(default_rtc == 2) // RTC From GW
	{
		uint32_t rtc_time;
		memcpy(&rtc_time, &glComm_response[12],4);

		RTC_C_disableInterrupt(RTC_C_TIME_EVENT_INTERRUPT);
		time_set_t Sec_RTC;
		year_from_RTC(rtc_time,&Sec_RTC);
		rtc_c_calender.seconds =Sec_RTC.seconds;
		rtc_c_calender.minutes = Sec_RTC.minutes;
		rtc_c_calender.hours   = Sec_RTC.hours;
		rtc_c_calender.dayOfWeek = Sec_RTC.day_of_week;
		rtc_c_calender.dayOfmonth = Sec_RTC.date ;
		rtc_c_calender.month = Sec_RTC.month;
		rtc_c_calender.year = Sec_RTC.year + REFERENCE_YR;
		RTC_C_holdClock();
		RTC_C_initCalendar(&rtc_c_calender,RTC_C_FORMAT_BINARY);
		RTC_C_setCalendarEvent(RTC_C_CALENDAREVENT_MINUTECHANGE);
		RTC_C_startClock();
		//RTC_C_registerInterrupt(RTC_C_IRQHandler);
		NVIC->ISER[0] = 1 << ((RTC_C_IRQn) & 31);// Enable RTC_C_IRQn interrupt in NVIC module
		RTC_C_enableInterrupt(RTC_C_TIME_EVENT_INTERRUPT);
	}
	else //set with RTC rxd from server
	{
		if( (glDo_rtc_update == TRUE) || (set_DLST==true))
		{
			if(glDo_rtc_update == TRUE)
			{
				RTC_C_disableInterrupt(RTC_C_TIME_EVENT_INTERRUPT);
				rtc_c_calender.seconds = ((gprs_general_sync_response.RTC_String[17] & 0x0F)*10) + (gprs_general_sync_response.RTC_String[18]& 0x0F);
				rtc_c_calender.minutes = ((gprs_general_sync_response.RTC_String[14] & 0x0F)*10) + (gprs_general_sync_response.RTC_String[15]& 0x0F);
				rtc_c_calender.hours   = ((gprs_general_sync_response.RTC_String[11] & 0x0F)*10) + (gprs_general_sync_response.RTC_String[12]& 0x0F);
				rtc_c_calender.dayOfWeek = (gprs_general_sync_response.RTC_String[24]& 0x0F);
				rtc_c_calender.dayOfmonth = ((gprs_general_sync_response.RTC_String[8] & 0x0F)*10) + (gprs_general_sync_response.RTC_String[9]& 0x0F);
				rtc_c_calender.month = ((gprs_general_sync_response.RTC_String[5] & 0x0F)*10) + (gprs_general_sync_response.RTC_String[6]& 0x0F);
				rtc_c_calender.year = ((gprs_general_sync_response.RTC_String[0]& 0x0F)*1000) + ((gprs_general_sync_response.RTC_String[1]& 0x0F)*100) + ((gprs_general_sync_response.RTC_String[2]& 0x0F)*10) + (gprs_general_sync_response.RTC_String[3]& 0x0F);
				RTC_C_holdClock();
				RTC_C_initCalendar(&rtc_c_calender,RTC_C_FORMAT_BINARY);
				//RTC_C_setCalibrationData(RTC_C_CALIBRATION_UP1PPM, 240);
				RTC_C_setCalendarEvent(RTC_C_CALENDAREVENT_MINUTECHANGE);
				RTC_C_startClock();
				//RTC_C_registerInterrupt(RTC_C_IRQHandler);
				NVIC->ISER[0] = 1 << ((RTC_C_IRQn) & 31);// Enable RTC_C_IRQn interrupt in NVIC module
				RTC_C_enableInterrupt(RTC_C_TIME_EVENT_INTERRUPT);
			}
			else
			{
				//Debug_TextOut(0, "Updating RTC Time to DLST");
				RTC_C_disableInterrupt(RTC_C_TIME_EVENT_INTERRUPT);
				rtc_c_calender = RTC_C_getCalendarTime();

				if (DLST_HOUR_CHANGE == 1)
				{
					//Debug_TextOut(0, "Updating RTC Time to DLST-01");
					rtc_c_calender.hours += 1;

				}
				else if (DLST_HOUR_CHANGE == 0)
				{
					//Debug_TextOut(0, "Updating RTC Time to DLST-02");
					rtc_c_calender.hours -= 1;

				}
				else
				{
					//Debug_TextOut(0, "Updating RTC Time to DLST-03");
					return;
				}

				RTC_C_holdClock();
				RTC_C_initCalendar(&rtc_c_calender,RTC_C_FORMAT_BINARY);
				//RTC_C_setCalibrationData(RTC_C_CALIBRATION_UP1PPM, 240);
				RTC_C_setCalendarEvent(RTC_C_CALENDAREVENT_MINUTECHANGE);
				RTC_C_startClock();
				//RTC_C_registerInterrupt(RTC_C_IRQHandler);
				NVIC->ISER[0] = 1 << ((RTC_C_IRQn) & 31);// Enable RTC_C_IRQn interrupt in NVIC module
				RTC_C_enableInterrupt(RTC_C_TIME_EVENT_INTERRUPT);

				if (DLST_HOUR_CHANGE == 1) //LNGSIT-715, 716, 749
				{
					push_event_to_cache(UDP_EVTTYP_DLST_FORWARD);
				}
				else if (DLST_HOUR_CHANGE == 0)
				{
					push_event_to_cache(UDP_EVTTYP_DLST_BACKWARD);
				}
			}
		}
	}
	if(rtc_c_calender.hours > 12)
		rtc_c_calender.hours = rtc_c_calender.hours - 12;
	if(rtc_c_calender.hours > 11)
		sprintf((char*)RTC_String, " %d/%02d/%02d      %02d:%02dPM ",rtc_c_calender.month,rtc_c_calender.dayOfmonth,rtc_c_calender.year,rtc_c_calender.hours,rtc_c_calender.minutes);
	else
		sprintf((char*)RTC_String, " %d/%02d/%02d      %02d:%02dAM ",rtc_c_calender.month,rtc_c_calender.dayOfmonth,rtc_c_calender.year,rtc_c_calender.hours,rtc_c_calender.minutes);

}

void do_daylight_saving()
{
    uint16_t Action_byte;

    if (DLST_CURRENT_SLOT == 10)
    {
    	//Debug_TextOut(0, "DLST_CURRENT_SLOT = 10");
        return;
    }
    //glSystem_cfg.DLST_Action_byte=(glSystem_cfg.DLST_Action_byte<<8)|(glSystem_cfg.DLST_Action_byte>>8);//24-05-12
	Action_byte = glSystem_cfg.DLST_Action_byte;
	Action_byte = Action_byte >> DLST_CURRENT_SLOT;//04-06-12
    if ( (Action_byte & 0x0001) == 1)  //+60 mins
    {
        DLST_HOUR_CHANGE = 1;
    }
    else if( (Action_byte & 0x0001) == 0)   //-60 mins
    {
        DLST_HOUR_CHANGE = 0;
    }

    SetMSP_RTC(FALSE);

    DLST_FLAG_CHECK = 1;
    Flash_Batch_Flag_Write();	//Mark the DLST bit as done and write back
    Flash_Batch_Flag_Read();	//Read Back again to keep the newly written values in RAM
    DLST_FLAG_CHECK = 0;
    set_DLST        = false;

    //Debug_TextOut(0, "DLST Confirmation log");	//compare this log with the printed RTC to confirm DLST applied

    return;
}

void year_from_RTC(uint32_t RTC_secs,time_set_t *time_set_1)
{
	uint8_t tmp_char;
	uint8_t tmp_month,tmp_date;
	//MEMORY_OPTIMIZATION
	//unsigned char tmp_hour,tmp_sec,tmp_day_of_week;
	uint16_t tmp_year;
	uint16_t tmp_int;
	uint32_t tmp_num_day,tmp_long;

	/* get format in day_of_week,hour,minute and seconds */
	days_from_RTC(RTC_secs,time_set_1);

	tmp_num_day = get_number_day(RTC_secs);

	tmp_year = REFERENCE_YR; //start from Year 2000

	tmp_long = get_day_inyear(tmp_year);

	/* get current year */
	while (tmp_long <= tmp_num_day)
	{
		tmp_year ++;
		tmp_int = get_day_inyear(tmp_year);
		tmp_long = tmp_long + tmp_int;
	}

	/* get current month */

	tmp_long = tmp_long - get_day_inyear(tmp_year);

	tmp_num_day = tmp_num_day - tmp_long;  //number of days in current year
	tmp_month = 1;
	tmp_int = month_length(tmp_year,tmp_month);

	while(tmp_int <= tmp_num_day)
	{
		tmp_month++;
		tmp_char = month_length(tmp_year,tmp_month);
		tmp_int = tmp_int + tmp_char;
	}

	/* get current date */
	tmp_int = tmp_int - month_length(tmp_year,tmp_month);
	tmp_date = tmp_num_day - tmp_int + 1;         //number of days in current month

	time_set_1 -> year = tmp_year - 2000;
	time_set_1 -> month = tmp_month;
	time_set_1 -> date = tmp_date;

}

/**************************************************************************/
/*  Name        : get_RTC_in_days                                         */
/*  Parameters  : long RTC_time                                           */
/*  Returns     : void                                                    */
/*  Scope       : system                                                  */
/*  Function    : To convert RTC time in Day of Week,Hours,               */
/*                Min,Seconds                                             */
/*------------------------------------------------------------------------*/
void days_from_RTC(uint32_t RTC_secs,time_set_t *time_set_ptr)
{
	uint8_t d_val;
	uint32_t t_val;
	uint32_t s_val;
	uint32_t t_credit;

	t_credit = RTC_secs;

	t_val = t_credit % (7*86400);  //seconds per week
	d_val = t_val/86400 + 6;       // day of week offset, 2000.1.1 -> Saturday

	if (d_val > 6)
	{d_val = d_val - 7;}

	(time_set_ptr -> day_of_week) = d_val;

	t_val = t_credit % 86400 ;        /* Seconds per day */

	s_val = t_val / 3600;
	(time_set_ptr -> hours) = s_val;

	t_val = t_val % 3600;
	(time_set_ptr -> minutes) = t_val / 60;

	(time_set_ptr-> seconds) = t_val % 60;

}

uint32_t get_number_day(uint32_t RTC_seconds)
{
	uint32_t temp_long;
	temp_long = RTC_seconds/SECS_IN_ADAY;
	return(temp_long);
}

uint16_t get_day_inyear(uint16_t year_id)
{
	uint8_t tmp_char;
	uint16_t tmp_int;

	tmp_char = check_leep_year(year_id);
	if(tmp_char == LEAP_YEAR)
	{tmp_int = 366;}
	else
	{tmp_int = 365;}

	return(tmp_int);
}
