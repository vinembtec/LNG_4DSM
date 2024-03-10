/*
 * LibG2_otap.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef LOW_LEVEL_DRIVER_MODULES_LIBG2_OTAP_H_
#define LOW_LEVEL_DRIVER_MODULES_LIBG2_OTAP_H_

#include "../Main Module/LibG2_main.h"

//#define Sector_num(Shift_pos)		((uint32_t)(1<<Shift_pos)) //not used //vinay
#define MIN_LNG_CONFIG_FILE_SIZE	3072

void start_reprogram();
void check_n_activate_files_from_flash( void );
//void function_at_0x3F000(void);

uint8_t activate_config_file( uint8_t call_for_dnld );

#endif /* LOW_LEVEL_DRIVER_MODULES_LIBG2_OTAP_H_ */
