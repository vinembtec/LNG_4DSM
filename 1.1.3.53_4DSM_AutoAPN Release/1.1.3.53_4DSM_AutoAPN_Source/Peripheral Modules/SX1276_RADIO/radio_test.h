/*
 * radio_test.h
 *
 *  Created on: Aug 2, 2016
 *      Author: admin
 */

#ifndef PERIPHERAL_MODULES_SX1276_RADIO_RADIO_TEST_H_
#define PERIPHERAL_MODULES_SX1276_RADIO_RADIO_TEST_H_

#include "../../Main Module/LibG2_main.h"

#define USE_SX1276_RADIO
#define INPUT_LENGTH			21

void OnMaster( void );
void OnSlave( void );
void Init_LORA();
int test_radio( void );
uint8_t Validate_LORA_Parametrs();
#endif /* PERIPHERAL_MODULES_SX1276_RADIO_RADIO_TEST_H_ */
