/*
 * LibG2_rates.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef APPLICATION_MODULES_LIBG2_RATES_H_
#define APPLICATION_MODULES_LIBG2_RATES_H_

#include "../Main Module/LibG2_main.h"

#define FREE_PARKING 	255
#define NO_PARKING		254
#define PREPAY_PARKING	253

void read_and_fill_current_schedule(RTC_C_Calendar * lrtc_c_calender);
uint32_t get_earned_seconds(uint16_t paid_amount, uint8_t top_up_trans, uint8_t this_is_before_update);
void parking_clock_update();
float Refund_get_earned_amount(float Remainingtime);

#endif /* APPLICATION_MODULES_LIBG2_RATES_H_ */
