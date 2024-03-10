/*
 * LibG2_spi.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef LOW_LEVEL_DRIVER_MODULES_LIBG2_SPI_H_
#define LOW_LEVEL_DRIVER_MODULES_LIBG2_SPI_H_

#include "../Main Module/LibG2_main.h"

#define FLASH_SPI						1
#define LCD_SPI							2
#define DATAKEY_SPI						3

void Flash_init_SPI();
void Display_HWSPI_Init();
void DataKey_init_SPI();
void Flash_SPI_Chip_Disable(void);
void Flash_SPI_Chip_Enable(void);
void Display_SPI_Chip_Enable(void);
void Display_SPI_Chip_Disable(void);
uint8_t Flash_SPI_RX(void);
void LORA_SPI_Init();
void Flash_SPI_TX(uint8_t *TX_buff,uint16_t TX_count);
//void SPIWritedata(uint8_t value); //not used //vinay
void SPIWriteByte(uint8_t value);
void SPIWriteBitBangByte(uint8_t value);
void SPIWrite_7bits(uint8_t value);
void SPIWriteRLEByte(uint8_t value);
//void SPIWriteWord(uint8_t value); //not used //vinay

#endif /* LOW_LEVEL_DRIVER_MODULES_LIBG2_SPI_H_ */
