/*
 * LibG2_main.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef MAIN_MODULE_LIBG2_MAIN_H_
#define MAIN_MODULE_LIBG2_MAIN_H_

#include "msp.h"
#include "../Low Level Driver Modules/driverlib/driverlib.h"

#include "../Application Modules/LibG2_power_mgmt.h"
#include "../Application Modules/LibG2_rates.h"
#include "../Application Modules/LibG2_techmenu.h"
#include "../Application Modules/LibG2_debug.h"

#include "../Main Module/LibG2_isr.h"

#include "../Communication Modules/LibG2_communication.h"
#include "../Communication Modules/LibG2_events.h"
#include "../Communication Modules/LibG2_filedownload.h"
#include "../Communication Modules/LibG2_lte.h"
#include "../Communication Modules/LibG2_meterdiag.h"
#include "../Communication Modules/LibG2_lob.h"
#include "../Communication Modules/LibG2_xmlparser.h"

#include "../Initialization Modules/LibG2_config.h"
#include "../Initialization Modules/LibG2_gpio_init.h"
#include "../Initialization Modules/LibG2_msp_init.h"

#include "../Low Level Driver Modules/LibG2_aes.h"
#include "../Low Level Driver Modules/LibG2_crc.h"
#include "../Low Level Driver Modules/LibG2_otap.h"
#include "../Low Level Driver Modules/LibG2_rtc.h"
#include "../Low Level Driver Modules/LibG2_spi.h"
#include "../Low Level Driver Modules/LibG2_timer.h"
#include "../Low Level Driver Modules/LibG2_uart.h"
#include "../Low Level Driver Modules/LibG2_IO_EXP_SPI.h"
#include "../Low Level Driver Modules/LibG2_card_d.h"

#include "../Peripheral Modules/LibG2_coin.h"
#include "../Peripheral Modules/LibG2_creditcard.h"
#include "../Peripheral Modules/LibG2_datakey.h"
#include "../Peripheral Modules/LibG2_flash.h"
#include "../Peripheral Modules/LibG2_smartcard.h"
#include "../Peripheral Modules/LibG2_uid.h"
#include "../Peripheral Modules/LibG2_lcd.h"
#include "../Peripheral Modules/LibG2_led.h"
#include "../Peripheral Modules/LibG2_keypad.h"
#include "../Peripheral Modules/LibG2_smartcard.h"
#include "../Peripheral Modules/SX1276_RADIO/radio_test.h"
#include "../Peripheral Modules/SX1276_RADIO/radio.h"

#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stdio.h"
#include "math.h"
#include "stdlib.h"

#ifndef Test_Flash_Batch_Erase
//#define Test_Flash_Batch_Erase
#endif

#define MPB_CODE_SIZE				0x3FFFF
#define DEFAULT_LQI_ADDER   		164

#define BITSET(_reg, _val)        	((_reg) |= (_val))
#define BITRESET(_reg, _val)    	((_reg) &= (~(_val)))

#define BIT_0    					0x01
#define BIT_1    					0x02
#define BIT_2   					0x04
#define BIT_3   					0x08
#define BIT_4   					0x10
#define BIT_5   					0x20
#define BIT_6   					0x40
#define BIT_7   					0x80
#define BIT_8   					0x100
#define BIT_9   					0x200
#define BIT_10   					0x400
#define BIT_11   					0x800
#define BIT_12   					0x1000
#define BIT_13   					0x2000
#define BIT_14   					0x4000
#define BIT_15   					0x8000
#define BIT_16   					0x10000
#define BIT_17   					0x20000
#define BIT_18   					0x40000
#define BIT_19   					0x80000
#define BIT_20   					0x100000
#define BIT_21   					0x200000
#define BIT_22   					0x400000
#define BIT_23   					0x800000
#define BIT_24   					0x1000000
#define BIT_25   					0x2000000
#define BIT_26   					0x4000000
#define BIT_27   					0x8000000
#define BIT_28   					0x10000000
#define BIT_29   					0x20000000
#define BIT_30   					0x40000000
#define BIT_31   					0x80000000

#define FALSE   					false
#define TRUE    					true

#define LNG_REV3					3
#define LNG_REV4					4

#define MB_LOCID_BUF_SZ             20

//12-03-2014:hardware sensor code merge
//#define SENSOR_BINARY_READ_CMD 		51  //not used //vinay
//#define SENSOR_SPACE_STATUS_CMD 	10  //not used //vinay

#define SAME_VEHICLE_AS_BEFORE      8
#define PAYMENT_ATTEMPT			 	9
#define SENSOR_DIAG_EXCEPTION		10
//#define ADDITIONAL_SENSOR_UPDATE	11  //not used //vinay
//12-03-2014:hardware sensor code merge

#define SECS_PER_TICK 				30
//#define DEFAULT_COIN_WAKEUP_TIMER	653//294//0x0024	//0x0064	//0x0028		//0x0064 //0x0028		//Wakeup sensor sample rate	(0x28 * 0.25mS = 10ms).
#define ONE_SECOND_TIMER			144//18//61//18//62//100	//1 second >VT<
//#define VCOM_TOGGLE_COUNTER			(ONE_SECOND_TIMER / 2)//31//50	//500 milliseconds  //not used //vinay
#define LORA_CHECK_COUNTER			(ONE_SECOND_TIMER / 3)//31//50	//250 milliseconds
//#define INTERVALS_CHECK_COUNTER		(ONE_SECOND_TIMER * 30)//520//1880//520//1800//1500//3000	//30seconds  //not used //vinay
//#define WATCHDOG_PAT_COUNTER		195
#define FACTVN_INTERVAL				10//5 minutes
#define RTC_PARK_CLK_SYNC_INTERVAL	60//120//300//5 minutes
#define MAX_ALLOWED_SYNC_SECONDS	60//20//90//70//20
#define RT_PARK_CLK_TO_FLASH_INTERVAL 600//10 minute RTC data to flash

#define PARKING_CLOCK_STOP_MODE		0
#define PARKING_CLOCK_RUN_MODE		1
#define PARKING_CLOCK_RESET_MODE	2
#define MSM_MAX_PARKING_BAYS		2
//#define MAX_ALLOWED_AMOUNT_IN_A_DAY	50000//6500	//change for Denver demo only  //not used //vinay

typedef struct _mbinfo
{
    uint16_t            city_code;
    uint32_t           serial_no;
    uint8_t            locid[MB_LOCID_BUF_SZ+1];
    uint8_t            serial_no_asci[7+1];
} MBInfo;

void Switch_States();
void MSP_Status_Check();
void watchdog_pat();
void poll_space_status();
//void process_sensor_data(uint16_t result); //this was there in regular DSM vinay  //not used //vinay
void Zero_Out_On_vacant();
//void check_pending_requests();  //not used //vinay
void Display_Meter_Info();

#endif /* MAIN_MODULE_LIBG2_MAIN_H_ */
