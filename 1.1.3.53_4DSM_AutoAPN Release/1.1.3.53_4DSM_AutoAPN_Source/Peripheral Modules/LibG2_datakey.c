//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_datakey.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_datakey_api
//! @{
//
//*****************************************************************************
#include "LibG2_datakey.h"

/*
***********************************************************************
* Defines
***********************************************************************

***********************************************************************
*Imported Global  Variables
***********************************************************************
*/
extern uint8_t                  Flash_Operation;
extern uint8_t 					flash_data_read_buffer[ FLASH_DATA_READ_BUF_SIZE ];
extern uint8_t					HARDWARE_REVISION;
extern GPRSSystem      			glSystem_cfg;
/*
***********************************************************************
*Exported Global Variables
***********************************************************************
*/
uint8_t DK_data_read_buffer[ DK_DATA_READ_BUF_SIZE ]; //16-03-12

/**************************************************************************/
//! Writes data into datakey
//! \param pointer to the data buffer to be written
//! \param uint16_t size of the data to be written
//! \param uint16_t address where data to be written
//! \param uint8_t batch_erase_flag decides if sector erase is needed before writing the data
//! \return uint32_t flash_start_address - Next address of memory after the written data
/**************************************************************************/
uint32_t DataKey_Write(const void * src_data_ptr, int data_lng, uint32_t flash_start_address, uint8_t batch_erase_flag)
{
	if(HARDWARE_REVISION == LNG_REV3)
	{
		GPIO_setOutputHighOnPin(DK_CNTRL_PORT, DK_CNTRL_PIN);
		DelayMs(10);
	}
	Flash_Operation = DATAKEY_SPI;//datakey
	flash_start_address = Flash_Write(src_data_ptr,data_lng,flash_start_address,batch_erase_flag );
	Flash_Operation = FLASH_SPI;//back to flash
	if(HARDWARE_REVISION == LNG_REV3)
	{
		GPIO_setOutputLowOnPin(DK_CNTRL_PORT, DK_CNTRL_PIN);
		DelayMs(10);
	}

	return flash_start_address;
}

/**************************************************************************/
//! Reads data from datakey and returns in DK_data_read_buffer
//! \param uint32_t start address from where data to be read
//! \param uint32_t Number of bytes to read
//! \return uint32_t flash_start_address - Next address of memory after the Read data
/**************************************************************************/
uint32_t DataKey_Read(uint32_t flash_start_address, int num_bytes)
{
	if(HARDWARE_REVISION == LNG_REV3)
	{
		GPIO_setOutputHighOnPin(DK_CNTRL_PORT, DK_CNTRL_PIN);
		DelayMs(10);
	}
	Flash_Operation = DATAKEY_SPI;//datakey
	flash_start_address = Flash_Read(flash_start_address,num_bytes);
	memcpy(DK_data_read_buffer, flash_data_read_buffer, num_bytes);
	//Debug_Output6(0, "DK_data[0]:%ld, DK_data[1]:%ld, DK_data[2]:%ld,DK_data[3]:%ld,DK_data[4]:%ld,DK_data[5]:%ld,DK_data[6]:%ld", DK_data_read_buffer[0], DK_data_read_buffer[1],DK_data_read_buffer[2],DK_data_read_buffer[3],DK_data_read_buffer[4],DK_data_read_buffer[5]);
	Flash_Operation = FLASH_SPI;//back to flash
	if(HARDWARE_REVISION == LNG_REV3)
	{
		GPIO_setOutputLowOnPin(DK_CNTRL_PORT, DK_CNTRL_PIN);
		DelayMs(10);
	}

	return flash_start_address;
}

/**************************************************************************/
//! Reads the files from flash and datakey, then calculates 32-bit CRC
//! \param uint8_t file type
//! - \b Configuration File
//! - \b Binary File
//! \param uint8_t flash memory type
//! - \b On-board External Flash
//! - \b Datakey
//! \return 32-bit calculated CRC
/**************************************************************************/
uint32_t Calculate_File_CRC_for_DK_copy(uint8_t file_type, uint8_t flash_memory_type)
{
	uint32_t CRC = 0XFFFFFFFF, File_Size = 0, offset = 0, bytes_left = 0, Start_address = 0;
	uint16_t i = 0,glChunk_size = 0,no_of_chunks;

	switch(file_type)
	{
	case FILE_TYPE_MPB_CONFIG:
		if(flash_memory_type == FLASH_SPI)
		{
			Start_address   = AD_MPB_config_w;
			Start_address   = Flash_Read(Start_address,8);
			File_Size 	    = (uint32_t)(((flash_data_read_buffer[0]&0xffffffff)<<24) | ((flash_data_read_buffer[1]&0xffffffff)<<16) | ((flash_data_read_buffer[2]&0xffffffff)<<8) | ((flash_data_read_buffer[3])&0xffffffff));
			//Debug_Output6(0, "Flash CRC:%02x,%02x,%02x,%02x", flash_data_read_buffer[4], flash_data_read_buffer[5], flash_data_read_buffer[6], flash_data_read_buffer[7], 0, 0);
		}
		else
		{
			Start_address   = AD_MPB_CONFIG_DK;
			Start_address   = DataKey_Read(Start_address,8);
			File_Size 	    = (uint32_t)(((DK_data_read_buffer[0]&0xffffffff)<<24) | ((DK_data_read_buffer[1]&0xffffffff)<<16) | ((DK_data_read_buffer[2]&0xffffffff)<<8) | ((DK_data_read_buffer[3])&0xffffffff));
			//Debug_Output6(0, "DK CRC:%02x,%02x,%02x,%02x", DK_data_read_buffer[4], DK_data_read_buffer[5], DK_data_read_buffer[6], DK_data_read_buffer[7], 0, 0);
		}

		if(File_Size > sizeof( glSystem_cfg ))
		{
			File_Size = 0;
		}
		else
			File_Size -= 4;
		break;

	case FILE_TYPE_MPB_CODE:
		if(flash_memory_type == FLASH_SPI)
		{
			Start_address = AD_MSP_CODE_w;
		}
		else
		{
			Start_address = AD_MPB_BINARY_DK+FLASH_FILE_SIZE_LEN+3+4;
		}
		File_Size 		= MPB_CODE_SIZE;
		break;

	default:
		CRC = 0;
		break;
	}

	//TO CALCULATE CRC
	if((CRC!=0) &&(File_Size!=0xFFFFFFFF) && (File_Size!=0))
	{
		Debug_Output1(0,"File Size:%ld",File_Size);
		offset=0;
		CRC=0;
		glChunk_size  = sizeof(DK_data_read_buffer);
		if(File_Size > (sizeof(DK_data_read_buffer)))
			no_of_chunks = (uint16_t) (File_Size/glChunk_size)+1;
		else
			no_of_chunks = 1;
		for (i = 0; i < no_of_chunks; i++)
		{
			bytes_left = File_Size - offset;

			if(bytes_left < glChunk_size)
			{
				glChunk_size = (uint16_t) bytes_left;
			}
			if(flash_memory_type == FLASH_SPI)
			{
				Start_address=Flash_Read(Start_address, glChunk_size);
				CRC = calc_crc32(CRC, flash_data_read_buffer, glChunk_size);
			}
			else
			{
				Start_address=DataKey_Read(Start_address, glChunk_size);
				CRC = calc_crc32(CRC, DK_data_read_buffer, glChunk_size);
			}

/*			uint8_t value[10]={0};
			uint16_t i;
			for(i=0;i<glChunk_size;i++) //changes;
			{
				liberty_sprintf((char*)value," %02x",flash_data_read_buffer[i]);
				Debug_Display(0, (char*)value);
			}*/
			Debug_Output6(0,"CRC%d:%x %x %x %x %d",i,((CRC>>24)& 0xFF),((CRC>>16)& 0xFF),((CRC>>8) & 0xFF),(CRC & 0xFF),0);
			offset += glChunk_size;
		}
		Debug_Output6(0,"Calculated File CRC:%x %x %x %x",((CRC>>24)& 0xFF),((CRC>>16)& 0xFF),((CRC>>8) & 0xFF),(CRC & 0xFF),0,0);
	}
	//Debug_Output6(0,"File CRC:%x %x %x %x %d %d",((CRC>>24)& 0xFF),((CRC>>16)& 0xFF),((CRC>>8) & 0xFF),(CRC & 0xFF),0,0);
	return CRC;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
