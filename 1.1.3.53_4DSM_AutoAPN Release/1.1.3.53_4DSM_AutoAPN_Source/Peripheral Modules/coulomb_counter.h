/*--------------------------------------------------------------------------*/
// COPYRIGHT Duncan Industries , 2013
//
// File: coulomb_counter.h
//
// header for the driver code of DS2740 coulomb counter chip
//
// History:
// 02/15/2013		Ayman S.		Created.
/*--------------------------------------------------------------------------*/

#ifndef _COULOMB_COUNTER_H_
#define _COULOMB_COUNTER_H_

#if 0  //for code optimization
#include "../Main Module/LibG2_main.h"

//#define OPERATING_MODE_OVD
//
//#ifdef OPERATING_MODE_OVD
//Over drive speed. All times in us.
/*
#define TIME_SLOT_MIN_Ds2740					6	//tSLOT min
#define TIME_SLOT_MAX_Ds2740					16	//tSLOT max
#define TIME_TECOVERY_Ds2740					1	//tREC
#define TIME_WRITE_INIT_Ds2740					2   // 2 us
#define TIME_WRITE_0_LOW_MIN_Ds2740				6	//tLOW0 min
#define TIME_WRITE_0_LOW_MAX_Ds2740				16	//tLOW0 max
#define TIME_WRITE_1_LOW_MIN_Ds2740				1	//tLOW1 min
#define TIME_WRITE_1_LOW_MAX_Ds2740				2	//tLOW1 max
#define TIME_READ_DATA_VALID_Ds2740				2	//tRDV
#define TIME_RESET_TIME_HIGH_Ds2740				48	//tRSTH
#define TIME_RESET_TIME_LOW_Ds2740				48	//tRSTL
#define TIME_PRESENCE_DETECT_HIGH_MIN_Ds2740	2	//tPDH min
#define TIME_PRESENCE_DETECT_HIGH_MAX_Ds2740	6	//tPDH max
#define TIME_PRESENCE_DETECT_LOW_MIN_Ds2740		8	//tPDL min
#define TIME_PRESENCE_DETECT_LOW_MAX_Ds2740		24	//tPDL max
*/


#define TIME_SLOT_MIN_Ds2740					6	//tSLOT min  //not used //vinay
#define TIME_SLOT_MAX_Ds2740					16	//tSLOT max  //not used //vinay
#define TIME_TECOVERY_Ds2740					1	//tREC  //not used //vinay
#define TIME_WRITE_INIT_Ds2740					2   // 2 us  //not used //vinay
#define TIME_WRITE_0_LOW_MIN_Ds2740				15	//tLOW0 min  //not used //vinay
#define TIME_WRITE_0_LOW_MAX_Ds2740				16	//tLOW0 max  //not used //vinay
#define TIME_WRITE_1_LOW_MIN_Ds2740				0	//tLOW1 min  //not used //vinay
#define TIME_WRITE_1_LOW_MAX_Ds2740				2	//tLOW1 max  //not used //vinay
#define TIME_READ_DATA_VALID_Ds2740				2	//tRDV  //not used //vinay
#define TIME_RESET_TIME_HIGH_Ds2740				48	//tRSTH  //not used //vinay
#define TIME_RESET_TIME_LOW_Ds2740				120	//tRSTL  //not used //vinay
#define TIME_PRESENCE_DETECT_HIGH_MIN_Ds2740	15	//tPDH min  //not used //vinay
#define TIME_PRESENCE_DETECT_HIGH_MAX_Ds2740	16	//tPDH max  //not used //vinay
#define TIME_PRESENCE_DETECT_LOW_MIN_Ds2740		15	//tPDL min  //not used //vinay
#define TIME_PRESENCE_DETECT_LOW_MAX_Ds2740		24	//tPDL max  //not used //vinay

/*

//Over drive speed. All times in us.
#define TIME_SLOT_MIN							6	//tSLOT min
#define TIME_SLOT_MAX							16	//tSLOT max
#define TIME_TECOVERY							1	//tREC
#define TIME_WRITE_INIT							2       // 2 us
#define TIME_WRITE_0_LOW_MIN					15//6	//tLOW0 min
#define TIME_WRITE_0_LOW_MAX					16	//tLOW0 max
#define TIME_WRITE_1_LOW_MIN					0//1	//tLOW1 min
#define TIME_WRITE_1_LOW_MAX					2	//tLOW1 max
#define TIME_READ_DATA_VALID					2	//tRDV
#define TIME_RESET_TIME_HIGH					48	//tRSTH
#define TIME_RESET_TIME_LOW						120//48	//tRSTL
#define TIME_PRESENCE_DETECT_HIGH_MIN			15//2	//tPDH min
#define TIME_PRESENCE_DETECT_HIGH_MAX			16//6	//tPDH max
#define TIME_PRESENCE_DETECT_LOW_MIN			15//8	//tPDL min
#define TIME_PRESENCE_DETECT_LOW_MAX			24	//tPDL max

*/

//#else
//
//
//
//Normal speed. All times in us.
#define TIME_SLOT_MIN					60	//tSLOT min
#define TIME_SLOT_MAX					120	//tSLOT max
#define TIME_TECOVERY					1	//tREC
#define TIME_WRITE_INIT					15  // 15 us
#define TIME_WRITE_0_LOW_MIN			60	//tLOW0 min
#define TIME_WRITE_0_LOW_MAX			120	//tLOW0 max
#define TIME_WRITE_1_LOW_MIN			1	//tLOW1 min
#define TIME_WRITE_1_LOW_MAX			15	//tLOW1 max
#define TIME_READ_DATA_VALID			15	//tRDV
#define TIME_RESET_TIME_HIGH			480	//tRSTH
#define TIME_RESET_TIME_LOW				480	//tRSTL
#define TIME_PRESENCE_DETECT_HIGH_MIN	15	//tPDH min
#define TIME_PRESENCE_DETECT_HIGH_MAX	60	//tPDH max
#define TIME_PRESENCE_DETECT_LOW_MIN	60	//tPDL min
#define TIME_PRESENCE_DETECT_LOW_MAX	240	//tPDL max
//#endif
//Port and PIN to be used for 1-wire bus
#define BUS_PIN			BIT_0
#define BUS_PORT_SEL	P10SEL0
#define BUS_PORT_DIR	P10DIR
#define BUS_PORT_OUT	P10OUT
#define BUS_PORT_IN		P10IN
#define BUS_PORT_REN	P10REN

//Net address commands:
#define READ_NET_ADDRESS_CMD	0x33   //With RNAOP = 0
#define SKIP_NET_ADDRESS_CMD	0xCC

//Function commands:
#define READ_DATA_CMD			0x69
#define WRITE_DATA_CMD			0x6C

//Memory map
#define STATUS_REGISTER_ADDR						0x01
//#define SPEICAL_FEATURE_REGISTER_ADDR				0x08  //not used //vinay
#define CURRENT_REGISTER_MSB_ADDR					0x0E
//#define CURRENT_REGISTER_LSB_ADDR					0x0F  //not used //vinay
#define ACCUMULATED_CURRENT_REGISTER_MSB_ADDR		0x10
//#define ACCUMULATED_CURRENT_REGISTER_LSB_ADDR		0x11  //not used //vinay

//macros to be used for the 1-wire bus functions
#define set_1wirebus_rx()	{BITRESET(BUS_PORT_DIR, BUS_PIN);}
#define set_1wirebus_tx()	{BITSET(BUS_PORT_DIR, BUS_PIN);}


//Functions
//void coulomb_counter_1US(uint16_t iDelay);  //not used //vinay
uint8_t coulomb_counter_init();
uint8_t coulomb_counter_read_status_reg();
uint32_t coulomb_counter_read_current_reg();
uint32_t coulomb_counter_read_accumulated_current_reg();
uint64_t coulomb_counter_read_net_address();
float get_current_value();
float get_accumulated_current_value();
//void get_battery_voltages(uint16_t DCELL_Vg);//23-05-12  //not used //vinay
uint8_t get_coulomb_counter_readings();

#endif //_COULOMB_COUNTER_H_
