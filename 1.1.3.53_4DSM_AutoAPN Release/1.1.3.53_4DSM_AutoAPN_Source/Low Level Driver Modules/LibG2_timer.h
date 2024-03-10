/*
 * LibG2_timer.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef LOW_LEVEL_DRIVER_MODULES_LIBG2_TIMER_H_
#define LOW_LEVEL_DRIVER_MODULES_LIBG2_TIMER_H_

#include "../Main Module/LibG2_main.h"

void DelayMs(uint32_t count);
void DelayUs(uint32_t count);
void DelayMs_1(uint32_t count);
void DelayMs_2(uint32_t count);

void start_sensor_counter();
uint16_t read_sensor_counter();
//uint16_t read_coin_sample_counter(); //not used //vinay
void reset_sensor_counter();

void cancel_sensor_counter();
void init_sensor_counter_12mhz();
void init_coin_sample_counter_12mhz();
//void init_sensor_counter_32khz(); //not used //vinay
void start_coinwakeup_timer();
//void stop_coinwakeup_timer(); //not used //vinay
//void resume_coinwakeup_timer(); //not used //vinay
//void cancel_coinwakeup_timer(); //not used //vinay
//void start_intervals_timer(); //not used //vinay
//void cancel_intervals_timer(); //not used //vinay
//void Check_For_Intervals(); //not used //vinay

#endif /* LOW_LEVEL_DRIVER_MODULES_LIBG2_TIMER_H_ */
