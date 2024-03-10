/*
 * LibG2_datakey.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef PERIPHERAL_MODULES_LIBG2_DATAKEY_H_
#define PERIPHERAL_MODULES_LIBG2_DATAKEY_H_

#include "../Main Module/LibG2_main.h"


#define AD_MPB_BINARY_DK            	0x018000 //0X20000
#define AD_MPB_BINARY_PLUS_ONE_DK      	0x058000 //0X20000
#define AD_MPB_CONFIG_DK            	0X010000 //0X1E000
typedef struct _loc_id_key_info
{
    uint8_t     loc_id[20];  //30-01-12: from 10 to 20
    uint8_t     city_code;
    uint16_t    area_num;
    uint16_t    meter_id;
    uint16_t    cust_id;
    uint16_t    LID_Key_ver;

}LID_KEY_INFO;


#define DK_DATA_READ_BUF_SIZE 		900 //448//(512)//26-08-2013:RAM optimization
//#define LID_LENGTH					20  //not used //vinay
//#define LIDKEY_CONTENT_LENGTH		32  //not used //vinay

uint32_t DataKey_Write(const void * src_data_ptr, int data_lng, uint32_t flash_start_address, uint8_t batch_erase_flag);
uint32_t DataKey_Read(uint32_t flash_start_address, int num_bytes);
uint32_t Calculate_File_CRC_for_DK_copy(uint8_t file_type, uint8_t flash_memory_type);

#endif /* PERIPHERAL_MODULES_LIBG2_DATAKEY_H_ */
