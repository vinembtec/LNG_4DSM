/*
 * LibG2_uart.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef LOW_LEVEL_DRIVER_MODULES_LIBG2_UART_H_
#define LOW_LEVEL_DRIVER_MODULES_LIBG2_UART_H_

#include "../Main Module/LibG2_main.h"

void init_debug_UART();
void init_Telit_UART();
uint16_t debug_out_UART(const char *debug_uart_buf);
uint16_t Modem_out_UART(uint8_t * cmdstr, uint16_t cmdlen);
void init_SmartCard_UART();
void init_IDTech_UART(void);
#endif /* LOW_LEVEL_DRIVER_MODULES_LIBG2_UART_H_ */
