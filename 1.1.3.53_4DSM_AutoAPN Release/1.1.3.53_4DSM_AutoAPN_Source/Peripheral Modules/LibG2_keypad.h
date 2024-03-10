/*
 * LibG2_keypad.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef COMMON_MODULES_LIBG2_KEYPAD_H_
#define COMMON_MODULES_LIBG2_KEYPAD_H_

#include "../Main Module/LibG2_main.h"

#define KEY_UNKNOWN                         0
#define KEY_OK                              1
#define KEY_CANCEL                          2
#define KEY_UP                              3
#define KEY_DOWN                            4

#define SPECIAL_KEYS_ENTER_TECH_MENU        "32422"     // up,cancel,down,cancel,cancel
//#define SPECIAL_KEYS_DIAG_MODE              "32423"     // up,cancel,down,cancel,up  //not used //vinay
#define SPECIAL_KEYS_METER_STATUS			"32424"		// up,cancel,down,cancel,down
//#define SPECIAL_KEYS_METER_AUDIT			"32421"		// up,cancel,down,cancel,ok  //not used //vinay

uint8_t discriminate_which_key();
void Debounce(void);
void Process_keypress();

#endif /* COMMON_MODULES_LIBG2_KEYPAD_H_ */
