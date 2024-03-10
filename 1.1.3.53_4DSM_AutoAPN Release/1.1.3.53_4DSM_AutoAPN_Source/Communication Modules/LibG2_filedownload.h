/*
 * LibG2_filedownload.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef COMMUNICATION_MODULES_LIBG2_FILEDOWNLOAD_H_
#define COMMUNICATION_MODULES_LIBG2_FILEDOWNLOAD_H_

#include "../Main Module/LibG2_main.h"

enum FDSTATUS_en
{
    FDSTATUS_OK = 10,
    FDSTATUS_FAILED_DOWNLOAD = 11,
//    FDSTATUS_FAILED_NO_MEMORY = 12, //not used //vinay
//    FDSTATUS_FAILED_TO_WRT2FLASH = 13, //not used //vinay
//    FDSTATUS_FAILED_NO_ATTEMP = 14, //not used //vinay
//    FDSTATUS_FAILED = 15, //not used //vinay
//    FDSTATUS_FAILED_IMG_UPGRADE = 16, //not used //vinay
//    FDSTATUS_DOWNLOAD_PENDING = 17, //not used //vinay
//    FDSTATUS_PROCESS_PENDING = 18, //not used //vinay
//    FDSTATUS_ZMODEM_PENDING = 19 //not used //vinay
};

uint32_t Calculate_File_CRC_4mflash(uint8_t file_type);
uint32_t calc_crc32(uint32_t crc, const uint8_t* bufptr, uint32_t len);
void call_xml_parse(uint8_t * xml_file, uint16_t xml_file_size);
void validate_asynchronous_FD();
void Prepare_Flash_4FD();

#endif /* COMMUNICATION_MODULES_LIBG2_FILEDOWNLOAD_H_ */
