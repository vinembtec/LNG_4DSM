/*
 * LibG2_flash.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef PERIPHERAL_MODULES_LIBG2_FLASH_H_
#define PERIPHERAL_MODULES_LIBG2_FLASH_H_

#include <stdint.h>
#include "../Main Module/LibG2_main.h"

/* Flash Opcode Cycle*/
#define Read_Memory						0x03
//#define High_Speed_Read_Memory			0x0B  //not used //vinay
#define Flash_4KByte_Sector_Erase		0x20
#define Flash_32KByte_Block_Erase		0x52
//#define Flash_64KByte_Block_Erase		0xD8  //not used //vinay
//#define Chip_Erase						0x60  //not used //vinay
#define Byte_Program					0x02
//#define AAI_Word_program				0xAD  //not used //vinay
#define Read_Status_Register			0x05
#define Enable_Write_Status_Register	0x50
#define Write_Status_Register			0x01
#define Write_Enable					0x06
//#define Write_Disable					0x04  //not used //vinay
#define Read_Device_ID					0x90
//#define JEDEC_ID						0x9F  //not used //vinay
//#define EBSY							0x70  //not used //vinay
//#define DBSY							0x80  //not used //vinay

//#define FLASH_MEM_BEGIN         0x45800             // Flash code starting address  //not used //vinay
//#define FLASH_MEM_LENGTH        0x0400              // Function segment size to be copied  //not used //vinay
//#define RAM_MEM_BEGIN           0x5800              // RAM code starting address  //not used //vinay
//#define FLASH_RAM_ADDRESS       0x5C00  //not used //vinay

#define AD_olt_w                0x000000	//OLT Batch
#define AD_olt_r                0x000000

#define AD_ACK_w                0x002000	//OLT reconciliation Batch
#define AD_ACK_r                0x002000

#define AD_COIN_w               0x004000	//Cash Batch
#define AD_COIN_r               0x004000

#define AD_SMART_CARD_w         0X006000	//Smart Card Batch
#define AD_SMART_CARD_r         0X006000

#define AD_BAYSTATUS_w          0x008000	//Baystatus Batch
#define AD_BAYSTATUS_r          0x008000

#define AD_ALARM_w              0x00A000	//Alarms Batch
#define AD_ALARM_r              0x00A000

#define AD_EVENTS_BATCH_RW      0X00C000	//Events Batch

#define AD_Batch_Flag_w			0x00E000	//Batch flag write
#define AD_Batch_Flag_r			0x00E000	//Batch flag read

#define AD_MPB_config_w         0x058000 //0X010000	//Config file
#define AD_MPB_config_r         0x010000

#define AD_Coin_Calibration_w   0X012000	//Coin Calibration
#define AD_Coin_Calibration_r   0x012000

#define AD_PAMBayStatus_w       0x014000
#define AD_PAMBayStatus_r		0x014000

#define AD_RTCParkingClk_w      0x016000
#define AD_RTCParkingClk_r		0x016000

#define AD_CAMID_LID_RW		    0x017000

#define AD_MSP_CODE_w           0x018000	//MSP432 Binary file
#define AD_MSP_CODE_r           0x018000



#define FLASH_FILE_SIZE_LEN         4
//#define MB_FILES_RESERVED_LENGTH    20		//25-05-12  //not used //vinay
//#define CONVERTED_MB_FILE_LENGTH    2		//25-05-12  //not used //vinay
#define FLASH_SECTOR_SIZE			4096
//#define MAX_MBPGM_SIZE				49152  //not used //vinay
//#define MB_FILES_EXTEN_LEN          4  //not used //vinay
//17-03-2014:hardware sensor code merge
//#define MAX_MCPGM_SIZE             65536L  //not used //vinay
//#define MC_FILE_TYPE    			5  //not used //vinay
//#define Sensor_Configs_Count		13  //not used //vinay
//#define FILE_TYPE_MC_PGM			24

#define MULTIPLE_BLOCK_ERASE_FLAG 	2
//#define TWO_BLOCKS_ERASE_FLAG 	  	3  //not used //vinay
//#define MC_BINARY_ERASE_FLAG      	4  //not used //vinay
#define SINGLE_SECTOR_ERASE			5
#define FLASH_ERROR_NONE            (1)
//#define FLASH_SPI_ERROR             (0)  //not used //vinay

#define FLASH_SEGMENT_SIZE          900//(448)//26-08-2013:RAM optimization
#define FLASH_DATA_READ_BUF_SIZE    900//(448)//(900)//26-08-2013:RAM optimization

enum FlashFileTypes
{
    FILE_TYPE_MPB_CONFIG  = 16,
    FILE_TYPE_MPB_CODE    = 17,
    //FILE_TYPE_MB_CCF      = 18,
    //FILE_TYPE_MB_RPG      = 19,
    //FILE_TYPE_MB_PGM      = 20,
    FILE_TYPE_TOTAL_FILES = 2
} ;


enum FlashFileStatus
{
	FF_UNKNOWN            = 0,
	FF_DOWNLOADED         = 1,
	FF_ACTIVATE_STARTED   = 2,
	FF_ACTIVATE_COMPLETED = 3,
	FF_STALE              = 4,
	FF_161_COMPLETED	  = 5,
	FF_161_RETRY		  = 6

} ;

enum FlashActivationTblEntryType
{
	FFACT_MPB_CONFIG  = 0,
	FFACT_MPB_CODE    = 1,
	//FFACT_MB_CCF      = 2,
	//FFACT_MB_RPG      = 3,
	//FFACT_MB_PGM      = 4,
	FFACT_TOTAL_FILES = 2
};

typedef struct _flash_file_actvn_tbl_entry
{
 uint8_t   	type;
 uint8_t   	status;
 uint8_t   	actvn_try_count;
 uint8_t   	reserved[5];//04-06-12
 uint32_t	id;
 uint32_t	actvn_ts;	// seconds
 uint32_t	size;
 //uint8_t   	reserved[5];//04-06-12

} FlashFieActvnTblEntry;

typedef struct
{
	uint8_t 			   CFG;
	uint8_t 			   RPG;
	uint8_t 			   CCF;
	uint8_t 			   PGM[8];
	uint8_t 			   SNSR[16];
	uint8_t 			   BIN[32];

}Chunk_DNLD_Status;

typedef struct
{
	//uint32_t Max_parking_time;
	uint8_t Max_parking_time[4];
	uint32_t RTC_Update;
}RTC_ParkingClk;

uint32_t Flash_Read( uint32_t flash_start_address, uint16_t num_bytes );    // reterns the "next" flash address
uint32_t Flash_Write( const void * src_data_ptr, int data_lng, uint32_t flash_start_address, char batch_erase_flag  );
//void Flash_Sector_Erase(uint32_t Dst);
//void Flash_Block_Erase(uint32_t Dst);
void Flash_Batch_Flag_Read();
void Flash_Batch_Flag_Write();
void Flash_Coin_Calibration_Params(uint8_t overwrite_calibration);
void Flash_RTC_ParkingClock_Storage(uint8_t write_rtc_into_flash, uint8_t sync_RTC_park_time);
uint8_t Flash_Batch_Erase( uint32_t address, uint8_t batch_erase_flag, uint8_t keep_intr_disabled );
uint32_t flash_Reset();
uint32_t Release_Power_Down();
uint32_t Flash_Power_Down();
void Flash_Read_Unique_ID();
void Flash_Read_Device_ID();
void Coin_Only_Audit_to_Flash (void);

#endif /* PERIPHERAL_MODULES_LIBG2_FLASH_H_ */
