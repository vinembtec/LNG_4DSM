/*
 * LibG2_isr.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef COMMON_MODULES_LIBG2_ISR_H_
#define COMMON_MODULES_LIBG2_ISR_H_

#include "../Main Module/LibG2_main.h"

#define SENSOR_UPDATE_TASK				BIT_0
#define MAGTEK_CARD_ISR					BIT_1
#define KEYPADS_ISR 					BIT_2
#define INTERVALS_ISR 					BIT_3
#define PAM_UPDATE_TASK					BIT_4
#define RTC_ISR							BIT_5

#define CREDIT_CARD_OLT_TASK			BIT_6
#define CASH_UPDATE_TASK				BIT_7
#define GENERAL_SYNC_TASK				BIT_8
#define VOLTAGE_READ_TASK				BIT_9
#define EVENTS_UPDATE_TASK				BIT_10
#define FILE_UPDATE_TASK				BIT_11
#define SEVERE_EVENTS_TASK				BIT_12

//#define EUSCIA1_ISR						BIT_13  //not used //vinay
//#define EUSCIA2_ISR						BIT_14  //not used //vinay
//#define EUSCIA3_ISR						BIT_15  //not used //vinay
//#define EUSCIB0_ISR						BIT_16  //not used //vinay
//#define EUSCIB1_ISR						BIT_17  //not used //vinay
//#define EUSCIB2_ISR						BIT_18  //not used //vinay
//#define EUSCIB3_ISR						BIT_19  //not used //vinay
#define PROCESS_LORA_QUEUE_TASK			BIT_20
//#define PORT4_ISR						BIT_21  //not used //vinay
//#define PORT5_ISR						BIT_22  //not used //vinay
//#define PORT6_ISR						BIT_23  //not used //vinay
//#define TIMER_A1_ISR					BIT_24  //not used //vinay
//#define TIMER_A3_ISR					BIT_25  //not used //vinay
//#define ADC_ISR							BIT_26  //not used //vinay

#define PROCESS_RAM_QUEUE_TASK			BIT_27
#define FILE_DOWNLOAD_TASK				BIT_28
#define SCT_UPDATE_TASK					BIT_29

#endif /* COMMON_MODULES_LIBG2_ISR_H_ */
