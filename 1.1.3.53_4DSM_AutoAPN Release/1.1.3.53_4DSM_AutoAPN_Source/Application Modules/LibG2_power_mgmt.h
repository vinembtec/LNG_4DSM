/*
 * LibG2_power_mgmt.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef APPLICATION_MODULES_LIBG2_POWER_MGMT_H_
#define APPLICATION_MODULES_LIBG2_POWER_MGMT_H_
#include "../Main Module/LibG2_main.h"

void Meter_regular_sleep();
void Exit_from_LPM();
//void Meter_Power_save_mode();
//void Check_Power_Save_Hours( uint8_t loc_DOW);
//void Check_MPB_Power_Save_Hours(uint8_t ON_HOUR1,uint8_t ON_HOUR2,uint8_t OFF_HOUR1,uint8_t OFF_HOUR2);
//void best_LPM_settings();

#endif /* APPLICATION_MODULES_LIBG2_POWER_MGMT_H_ */
