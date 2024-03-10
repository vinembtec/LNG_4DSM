/*
 * LibG2_crc.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef LOW_LEVEL_DRIVER_MODULES_LIBG2_CRC_H_
#define LOW_LEVEL_DRIVER_MODULES_LIBG2_CRC_H_

#include "../Main Module/LibG2_main.h"

uint32_t calc_crc32(uint32_t crc, const uint8_t* bufptr, uint32_t len);
uint32_t calculate_chksum(const uint8_t* bufptr, uint32_t len);

#endif /* LOW_LEVEL_DRIVER_MODULES_LIBG2_CRC_H_ */
