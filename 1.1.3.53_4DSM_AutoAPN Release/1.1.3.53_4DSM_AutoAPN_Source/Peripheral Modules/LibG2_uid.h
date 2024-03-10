/*
 * LibG2_uid.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef PERIPHERAL_MODULES_LIBG2_UID_H_
#define PERIPHERAL_MODULES_LIBG2_UID_H_

#include "../Main Module/LibG2_main.h"

#define	FAIL									0
#define	SUCCESS									1

#define OPERATING_MODE_OVD

#ifdef OPERATING_MODE_OVD

//Over drive speed. All times in us.
#define TIME_SLOT_MIN							6	//tSLOT min
//#define TIME_SLOT_MAX							16	//tSLOT max  //not used //vinay
//#define TIME_TECOVERY							1	//tREC  //not used //vinay
//#define TIME_WRITE_INIT							2       // 2 us  //not used //vinay
//#define TIME_WRITE_0_LOW_MIN					15//6	//tLOW0 min  //not used //vinay
//#define TIME_WRITE_0_LOW_MAX					16	//tLOW0 max  //not used //vinay
//#define TIME_WRITE_1_LOW_MIN					0//1	//tLOW1 min  //not used //vinay
//#define TIME_WRITE_1_LOW_MAX					2	//tLOW1 max  //not used //vinay
//#define TIME_READ_DATA_VALID					2	//tRDV  //not used //vinay
#define TIME_RESET_TIME_HIGH					48	//tRSTH
#define TIME_RESET_TIME_LOW						120//48	//tRSTL
//#define TIME_PRESENCE_DETECT_HIGH_MIN			15//2	//tPDH min  //not used //vinay
//#define TIME_PRESENCE_DETECT_HIGH_MAX			16//6	//tPDH max  //not used //vinay
//#define TIME_PRESENCE_DETECT_LOW_MIN			15//8	//tPDL min  //not used //vinay
//#define TIME_PRESENCE_DETECT_LOW_MAX			24	//tPDL max  //not used //vinay

#else
//Normal speed. All times in us.
#define TIME_SLOT_MIN							60	//tSLOT min
#define TIME_SLOT_MAX							120	//tSLOT max
#define TIME_TECOVERY							1	//tREC
#define TIME_WRITE_INIT							15      // 15 us
#define TIME_WRITE_0_LOW_MIN					60	//tLOW0 min
#define TIME_WRITE_0_LOW_MAX					120	//tLOW0 max
#define TIME_WRITE_1_LOW_MIN					1	//tLOW1 min
#define TIME_WRITE_1_LOW_MAX					15	//tLOW1 max
#define TIME_READ_DATA_VALID					15	//tRDV
#define TIME_RESET_TIME_HIGH					480	//tRSTH
#define TIME_RESET_TIME_LOW						480	//tRSTL
#define TIME_PRESENCE_DETECT_HIGH_MIN			15	//tPDH min
#define TIME_PRESENCE_DETECT_HIGH_MAX			60	//tPDH max
#define TIME_PRESENCE_DETECT_LOW_MIN			60	//tPDL min
#define TIME_PRESENCE_DETECT_LOW_MAX			240	//tPDL max
#endif

#define Internal_UID_chip							0x00
#define External_UID_chip							0x01

//Net address commands:
#define READ_NET_ADDRESS_CMD					0x33   //With RNAOP = 0
#define SKIP_NET_ADDRESS_CMD					0xCC
#define WR_Add									0x00
//Function commands:

#define Write_Scratch_Pad						0x0F
#define Read_Scratch_pad						0xAA		//Read Scratchpad
#define Copy_Scratch_pad						0x55		//Copy Scratchpad
#define Read_UID_Memory							0xF0		//Read Memory

//#define RESUME									0xA5	//Resume  //not used //vinay
//#define ODSKIPROM								0x3C	//Overdrive Skip Rom  //not used //vinay
//#define ODMATCHROM          					0x69	//Overdrive Match Rom  //not used //vinay

//#define _delay_1us()  							asm(" nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t");

//macros to be used for the 1-wire bus functions
//#define set_1wirebus_rx()						{BITRESET(BUS_PORT_DIR, BUS_PIN);}
//#define set_1wirebus_tx() 					{BITSET(BUS_PORT_DIR, BUS_PIN);}

typedef struct
{
	uint8_t	Family_Code;
	uint8_t Serial_No[6];
	uint8_t CRC_Code;  //not used //vinay //it is giving warning if commented //vinay
}chip1;

typedef union
{
	chip1 chip;
	uint8_t data[sizeof(chip1)];
}UID;

typedef struct
{
	uint8_t	Location_ID[20];
	uint16_t Meter_ID;
	uint16_t Customer_ID;
	uint16_t Area_ID;
	uint16_t City_Code;
	uint32_t reserved;  //not used //vinay
}Key_data1;
typedef union
{
	Key_data1 Key_data;
	uint8_t data[sizeof(Key_data1)];
}Data_Key;

//Functions
uint8_t  UID_read_net_address(UID *chip_add,uint8_t Port_num);
uint16_t UID_Write_scrathpad(uint16_t Add,uint8_t *Data,uint8_t length,uint8_t Port_num);
uint16_t UID_Read_scrathpad(uint16_t *T_Add,uint8_t *Eadd_Status,uint8_t *Data,uint8_t length,uint8_t Port_num);
uint8_t UID_Copy_Scratchpad(uint16_t *T_Add,uint8_t *Eadd_Status,uint8_t Port_num);
uint16_t UID_Read_Mem(uint16_t R_Add,Data_Key *Data,uint8_t length,uint8_t Port_num);
uint8_t UID_Write_Mem(uint16_t W_Add,Data_Key *Data,uint8_t length,uint8_t Port_num);
void Read_UIDKey();

#endif /* PERIPHERAL_MODULES_LIBG2_UID_H_ */
