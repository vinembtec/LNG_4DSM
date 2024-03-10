/*
 * LibG2_rtc.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef LOW_LEVEL_DRIVER_MODULES_LIBG2_RTC_H_
#define LOW_LEVEL_DRIVER_MODULES_LIBG2_RTC_H_

#include "../Main Module/LibG2_main.h"

#define REFERENCE_YR                    2000
#define LEAP_YEAR                       1
#define NO_LEAP_YEAR                    0
#define SECS_IN_ADAY                    86400
#define SECS_IN_HOUR                    3600
#define SECS_IN_MINT                    60

#define MAX_PARKING_TIME_LENGTH   		4 //Length for "Set Max Parking Time after LNG Binary activation"
#define	RTC_UPDATE_LENGTH 				4 //Length for "RTC after any reset until RTC updates from server"

typedef struct
{
   uint8_t  year;
   uint8_t  month;
   uint8_t  date;
   uint8_t  hours;
   uint8_t  minutes;
   uint8_t  seconds;
   uint8_t  day_of_week;      // Shu Calender update
} time_set_t;

uint32_t RTC_epoch_now();
uint16_t get_offset_days(uint16_t c_year);
uint8_t check_leep_year(uint16_t c_year);
uint8_t month_length(uint16_t c_year,uint8_t c_month);
uint32_t RTC_from_year(time_set_t *time_set_1);
void SetMSP_RTC(uint8_t default_rtc);
void do_daylight_saving();
void year_from_RTC(uint32_t RTC_secs,time_set_t *time_set_1);
void days_from_RTC(uint32_t RTC_secs,time_set_t *time_set_ptr);
uint32_t get_number_day(uint32_t RTC_seconds);
uint16_t get_day_inyear(uint16_t year_id);

#endif /* LOW_LEVEL_DRIVER_MODULES_LIBG2_RTC_H_ */
