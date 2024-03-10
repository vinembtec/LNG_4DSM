/*
 * LibG2_IO_EXP_SPI.h
 *
 *  Created on: Nov 3, 2016
 *      Author: EMQOS
 */

#ifndef LOW_LEVEL_DRIVER_MODULES_LIBG2_IO_EXP_SPI_H_
#define LOW_LEVEL_DRIVER_MODULES_LIBG2_IO_EXP_SPI_H_

#include "../Main Module/LibG2_main.h"


#define SPI_Write 				0x42 // SPI write
//#define SPI_Read 				0x43 // SPI read //not used //vinay

#define Bank0

#ifdef Bank0
	#define IODIRA 				0x00
//	#define IPOLA 				0x02 //not used //vinay
//	#define GPINTENA 			0x04 //not used //vinay
//	#define DEFVALA 			0x06 //not used //vinay
//	#define INTCONA 			0x08 //not used //vinay
	#define IOCONA 				0x0A
//	#define GPPUA 				0x0C //not used //vinay
//	#define INTFA 				0x0E //not used //vinay
//	#define INTCAPA 			0x10 //not used //vinay
	#define GPIOA 				0x12
//	#define OLATA 				0x14 //not used //vinay
//	#define IODIRB 				0x01 //not used //vinay
//	#define IPOLB 				0x03 //not used //vinay
//	#define GPINTENB 			0x05 //not used //vinay
//	#define DEFVALB 			0x07 //not used //vinay
//	#define INTCONB 			0x09 //not used //vinay
//	#define IOCONB 				0x0B //not used //vinay
//	#define GPPUB 				0x0D //not used //vinay
//	#define INTFB 				0x0F //not used //vinay
//	#define INTCAPB 			0x11 //not used //vinay
	#define GPIOB 				0x13
//	#define OLATB 				0x15 //not used //vinay
#else	//Bank-1
	#define IODIRA 				0x00
	#define IPOLA 				0x01
	#define GPINTENA 			0x02
	#define DEFVALA 			0x03
	#define INTCONA 			0x04
	#define IOCONA 				0x05
	#define GPPUA 				0x06
	#define INTFA 				0x07
	#define INTCAPA 			0x08
	#define GPIOA 				0x09
	#define OLATA 				0x0A
	#define IODIRB 				0x10
	#define IPOLB 				0x11
	#define GPINTENB 			0x12
	#define DEFVALB 			0x13
	#define INTCONB 			0x14
	#define IOCONB 				0x15
	#define GPPUB 				0x16
	#define INTFB 				0x17
	#define INTCAPB 			0x18
	#define GPIOB 				0x19
	#define OLATB 				0x1A
#endif

typedef union
{
	uint8_t IOCON_value;
	struct
	{
	 uint8_t Reserved	:1;
	 uint8_t Intpol		:1;
	 uint8_t Odr		:1;
	 uint8_t Haen		:1;
	 uint8_t Disslw		:1;
	 uint8_t Seqop		:1;
	 uint8_t Mirror		:1;
	 uint8_t Bank		:1;
	} IOCON_set;
} IOCON;

typedef union
{
	uint8_t IODIR_value;
	struct
	{
	 uint8_t IO0	:1;
	 uint8_t IO1	:1;
	 uint8_t IO2	:1;
	 uint8_t IO3	:1;
	 uint8_t IO4	:1;
	 uint8_t IO5	:1;
	 uint8_t IO6	:1;
	 uint8_t IO7	:1;
	} IODIR_set;
} IODIR;

typedef union
{
	uint8_t IOGPIO_value;
	struct
	{
	 uint8_t GP0	:1;
	 uint8_t GP1	:1;
	 uint8_t GP2	:1;
	 uint8_t GP3	:1;
	 uint8_t GP4	:1;
	 uint8_t GP5	:1;
	 uint8_t GP6	:1;
	 uint8_t GP7	:1;
	} IOGPIO_set;
} IOGPIO;

//void IO_EXP_SPI_init(); //not used //vinay
//void IO_EXP_init(); //not used //vinay
//void Boost_enable(); //not used //vinay
//void Boost_disable(); //not used //vinay
void IOGPIO_Set(IOGPIO *iogpio,uint8_t gpio_add);
//void IO_EXP_SPI_Chip_Enable(void); //not used //vinay
void IO_EX_SPI_Chip_Disable(void);

#endif /* LOW_LEVEL_DRIVER_MODULES_LIBG2_IO_EXP_SPI_H_ */
