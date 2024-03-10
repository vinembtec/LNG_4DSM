/*
 * LibG2_msp_init.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef INITIALIZATION_MODULES_LIBG2_MSP_INIT_H_
#define INITIALIZATION_MODULES_LIBG2_MSP_INIT_H_

#include "../Main Module/LibG2_main.h"

void initialize_all();
void watchdog_init();
void watchdog_pat();
void Soft_Reset_watchdog();
void CS_init_CLK(void);

#endif /* INITIALIZATION_MODULES_LIBG2_MSP_INIT_H_ */
