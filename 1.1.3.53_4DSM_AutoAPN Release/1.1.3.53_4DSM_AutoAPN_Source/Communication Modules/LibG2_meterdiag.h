/*
 * LibG2_meterdiag.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef COMMUNICATION_MODULES_LIBG2_METERDIAG_H_
#define COMMUNICATION_MODULES_LIBG2_METERDIAG_H_

#include "../Main Module/LibG2_main.h"

#define MAX_DIAG_READINGS					12

void Adc_Vltg_init(void);
uint16_t Rechargeable_battery_read(void);
//uint16_t Solar_voltage_read(void); //not used //vinay
int8_t Temperature_read();
void get_battery_voltage_temp();

#endif /* COMMUNICATION_MODULES_LIBG2_METERDIAG_H_ */
