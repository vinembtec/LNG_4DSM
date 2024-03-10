//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_power_mgmt.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_power_mgmt_api
//! @{
//
//*****************************************************************************

#include "LibG2_power_mgmt.h"

extern uint8_t 			MDM_AWAKE,FIRST_COMM;
extern GPRSSystem  		glSystem_cfg;
extern uint8_t			Meter_Reset;
extern uint8_t			WAITING_FOR_SMS_RESPONSE;

uint8_t				   	rtc_hour_flag[24],gl_RTCHOUR=0;
volatile uint8_t 		IN_LPM_TEST = FALSE;
uint8_t 				Init_Complete_flag = FALSE;
uint8_t					Modem_Off_initiated = 0;
uint32_t				Mdm_off_start_timestamp = 0;
//uint8_t              	TELIT_SHUTDOWN = TRUE;	//Not using this flag anymore, We never use modem ON config in LNG

/**************************************************************************/
//! Used to put the MSP432 to LPM3 mode
//! \param void
//! \return void
/**************************************************************************/
void Meter_regular_sleep()
{
	//Debug_TextOut(0,"In LPM");
	if(/*(TELIT_SHUTDOWN == TRUE) && */(MDM_AWAKE == TRUE) &&
			(WAITING_FOR_SMS_RESPONSE == FALSE) && (Modem_Off_initiated == 0))	//Do not switch off modem immediately if meter is in SMS recovery mode
	{
		Debug_TextOut(0,"Mdm Shut");
		Modem_Off_initiated = 1;
		Mdm_off_start_timestamp = RTC_epoch_now();
		telit_power_off();
	}
	else if((MDM_AWAKE == TRUE) && (Modem_Off_initiated == 0) && (WAITING_FOR_SMS_RESPONSE == FALSE))	//LNGSIT-747
	{
		Debug_TextOut(0,"Mdm Slept");
		telit_sock_close();	//put the modem to sleep
	}

	if(Init_Complete_flag == FALSE)
	{
		if(glSystem_cfg.Coin_Only_Temporary_GW_Enable == TRUE)
		{
			Meter_Reset = FALSE;
		}
		Init_Complete_flag = TRUE;
		diag_text_Screen( "Meter Init Complete", TRUE, FALSE );
	}

	IN_LPM_TEST = TRUE;
	PCM_gotoLPM3();
}

/**************************************************************************/
//! Used to bring the MSP432 out of LPM3 mode
//! \param void
//! \return void
/**************************************************************************/
void Exit_from_LPM()
{
	if(IN_LPM_TEST == TRUE)
	{
		CS_init_HSMCLK(CS_CTL1_DIVHS__1);
		//DelayUs(500);
		IN_LPM_TEST = FALSE;
	}
}

#if 0
void Meter_Power_save_mode()
{
	//Turn off all peripherals and sleep
	PCM_gotoLPM3InterruptSafe();
}

void Check_MPB_Power_Save_Hours(uint8_t ON_HOUR1,uint8_t ON_HOUR2,uint8_t OFF_HOUR1,uint8_t OFF_HOUR2)
{
	uint8_t first_position=0xff,i=0;
	memset(rtc_hour_flag,0xFF,sizeof(rtc_hour_flag));

	if(OFF_HOUR1<=23)
		rtc_hour_flag[OFF_HOUR1]=0x10;
	if(OFF_HOUR2<=23)
		rtc_hour_flag[OFF_HOUR2]=0x20;
	if(ON_HOUR1<=23)
		rtc_hour_flag[ON_HOUR1]=0x31;
	if(ON_HOUR2<=23)
		rtc_hour_flag[ON_HOUR2]=0x41;

	for(i=0;i<23;i++)
	{
		if(((rtc_hour_flag[i]&0x0F)==0)||((rtc_hour_flag[i]&0X0F)==1))
		{
			if(first_position==0xff)
			first_position=i;
			if(rtc_hour_flag[i+1]==0xff)
			rtc_hour_flag[i+1]=rtc_hour_flag[i];
		}
	}
	if(first_position!=0)
	{
		for(i=0;i<first_position;i++)
		{
			if(rtc_hour_flag[i]==0xff)
			{
				if(i==0)
				{
					rtc_hour_flag[i]=rtc_hour_flag[i+23];
				}
				else
				{
					rtc_hour_flag[i]=rtc_hour_flag[i-1];
				}
			}
		}
	}
}

void Check_Power_Save_Hours( uint8_t loc_DOW)
{
	uint8_t ON_HOUR_1=0, ON_HOUR_2=0, OFF_HOUR_1=0, OFF_HOUR_2=0, ON_MIN_1=0, ON_MIN_2=0, OFF_MIN_1=0, OFF_MIN_2=0;

	switch(loc_DOW)
	{
	case 0:            //monday
	case 1:            //tuesday
	case 2:            //wednesday
	case 3:            //thursday
	case 4:            //friday
		ON_HOUR_1 = (glSystem_cfg.MF_TIME_ON_1)/10;
		OFF_HOUR_1 = (glSystem_cfg.MF_TIME_OFF_1)/10;
		ON_HOUR_2 = (glSystem_cfg.MF_TIME_ON_2)/10;
		OFF_HOUR_2 = (glSystem_cfg.MF_TIME_OFF_2)/10;

		ON_MIN_1 = (glSystem_cfg.MF_TIME_ON_1%10) * 15;
		OFF_MIN_1 = (glSystem_cfg.MF_TIME_OFF_1%10) * 15;
		ON_MIN_2 = (glSystem_cfg.MF_TIME_ON_2%10) * 15;
		OFF_MIN_2 = (glSystem_cfg.MF_TIME_OFF_2%10) * 15;
		break;
	case 5:            //saturday
		ON_HOUR_1 = (glSystem_cfg.SAT_TIME_ON_1)/10;
		OFF_HOUR_1 = (glSystem_cfg.SAT_TIME_OFF_1)/10;
		ON_HOUR_2 = (glSystem_cfg.SAT_TIME_ON_2)/10;
		OFF_HOUR_2 = (glSystem_cfg.SAT_TIME_OFF_2)/10;

		ON_MIN_1 = (glSystem_cfg.SAT_TIME_ON_1%10) * 15;
		OFF_MIN_1 = (glSystem_cfg.SAT_TIME_OFF_1%10) * 15;
		ON_MIN_2 = (glSystem_cfg.SAT_TIME_ON_2%10) * 15;
		OFF_MIN_2 = (glSystem_cfg.SAT_TIME_OFF_2%10) * 15;
		break;
	case 6:            //sunday
		ON_HOUR_1 = (glSystem_cfg.SUN_TIME_ON_1)/10;
		OFF_HOUR_1 = (glSystem_cfg.SUN_TIME_OFF_1)/10;
		ON_HOUR_2 = (glSystem_cfg.SUN_TIME_ON_2)/10;
		OFF_HOUR_2 = (glSystem_cfg.SUN_TIME_OFF_2)/10;

		ON_MIN_1 = (glSystem_cfg.SUN_TIME_ON_1%10) * 15;
		OFF_MIN_1 = (glSystem_cfg.SUN_TIME_OFF_1%10) * 15;
		ON_MIN_2 = (glSystem_cfg.SUN_TIME_ON_2%10) * 15;
		OFF_MIN_2 = (glSystem_cfg.SUN_TIME_OFF_2%10) * 15;
		break;
	default:
		break;
	}

	gl_RTCHOUR=RTCHOUR;
	Check_MPB_Power_Save_Hours(ON_HOUR_1,ON_HOUR_2,OFF_HOUR_1,OFF_HOUR_2);
	if(((rtc_hour_flag[gl_RTCHOUR]&0X0F)==0)&&(TELIT_SHUTDOWN==FALSE))
	{
		if((gl_RTCHOUR!=0)&&((rtc_hour_flag[gl_RTCHOUR-1]&0X0F)==0))
		{
			TELIT_SHUTDOWN=TRUE;
		}
		else if((((rtc_hour_flag[gl_RTCHOUR]&0xf0)==0X10)&&(RTCMIN>=OFF_MIN_1))||(((rtc_hour_flag[gl_RTCHOUR]&0xf0)==0X20)&&(RTCMIN>=OFF_MIN_2)))
		{
			TELIT_SHUTDOWN=TRUE;
		}
	}
	else if(((rtc_hour_flag[gl_RTCHOUR]&0X0F)==1)&&(TELIT_SHUTDOWN==TRUE))
	{
		if((((rtc_hour_flag[gl_RTCHOUR]&0xf0)==0X30)&&(RTCMIN>=ON_MIN_1))||(((rtc_hour_flag[gl_RTCHOUR]&0xf0)==0X40)&&(RTCMIN>=ON_MIN_2)))
		{
			TELIT_SHUTDOWN=FALSE;
			FIRST_COMM = TRUE;
		}
	}
}
#endif
#if 0
void best_LPM_settings()
{
	P1DIR = 0; P1REN = 0xFF; P1OUT = 0;
	P2DIR = 0; P2REN = 0xFF; P2OUT = 0;
	P3DIR = 0; P3REN = 0xFF; P3OUT = 0;
	P4DIR = 0; P4REN = 0xFF; P4OUT = 0;
	P5DIR = 0; P5REN = 0xFF; P5OUT = 0;
	P6DIR = 0; P6REN = 0xFF; P6OUT = 0;
	P7DIR = 0; P7REN = 0xFF; P7OUT = 0;
	P8DIR = 0; P8REN = 0xFF; P8OUT = 0;
	P9DIR = 0; P9REN = 0xFF; P9OUT = 0;
	P10DIR = 0; P10REN = 0xFF; P10OUT = 0;
	PJDIR = 0; PJREN = 0xFF; PJOUT = 0;
	GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0);	//EXT_UID
	GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN0);	//INT_UID_IO
	GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2, GPIO_PIN1);	//KEYPADS
	GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2, GPIO_PIN4);	//KEYPADS
	GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2, GPIO_PIN5);	//KEYPADS
	GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2, GPIO_PIN7);	//KEYPADS

	GPIO_setAsOutputPin(FLASH_SPI_IOPORT, FLASH_SPI_CS_PIN);
	GPIO_setOutputHighOnPin(FLASH_SPI_IOPORT, FLASH_SPI_CS_PIN);

	GPIO_setAsOutputPin(DISPLAY_SPI_CS_IOPORT,DISPLAY_SPI_CS_PIN);
	GPIO_setOutputLowOnPin(DISPLAY_SPI_CS_IOPORT,DISPLAY_SPI_CS_PIN);

	GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN1);	//CARD_INT_MSP

	GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);	//SMCRD_RST

	GPIO_setAsOutputPin(MDM_REG_CNTRL_PORT, MDM_REG_CNTRL_PIN);	//MDM_REG_CNTRL
	MDM_REG_CNTRL_clr();

	GPIO_setAsInputPinWithPullUpResistor(MDM_PORT, MDM_RXD_PIN);

	GPIO_setAsOutputPin(MDM_DTR_PORT, MDM_DTR_PIN);
	GPIO_setOutputLowOnPin(MDM_DTR_PORT, MDM_DTR_PIN);

	GPIO_setAsOutputPin(MDM_ON_PORT, MDM_ON_PIN);
	GPIO_setOutputLowOnPin(MDM_ON_PORT, MDM_ON_PIN);

	GPIO_setAsOutputPin(MDM_RST_PORT, MDM_RST_PIN);
	GPIO_setOutputLowOnPin(MDM_RST_PORT, MDM_RST_PIN);

	GPIO_setAsOutputPin(T_PORT, T_PIN);	//T
	t_High();

	GPIO_setAsOutputPin(SNSON_PORT, SNSON_PIN);	//Coin SNSON
	GPIO_setAsOutputPin(CLSNS_PORT, CLSNS_PIN);	//CLSNS
	coin_sensor_off();//CLSNS,PCLK,SNSON

	GPIO_setAsOutputPin(PCLK_PORT, PCLK_PIN);	//PCLK
	GPIO_setOutputLowOnPin(PCLK_PORT, PCLK_PIN);		//PCLK

	GPIO_setAsOutputPin(CHIRP_PORT, CHIRP_PIN);	//CHIRP
	GPIO_setOutputLowOnPin(CHIRP_PORT, CHIRP_PIN);		//CHIRP

	GPIO_setAsInputPinWithPullUpResistor(CNTR_PORT, CNTR_PIN);	//CNTR
	GPIO_setAsInputPinWithPullUpResistor(SNSRDY_PORT, SNSRDY_PIN);	//SNSREADY

	GPIO_setAsInputPinWithPullUpResistor(LED_RXD_PORT, LED_RXD_PIN);	//LED_RXD

	GPIO_setAsOutputPin(NLED_DRIVE1_PORT, NLED_DRIVE1_PIN);	//NLED_DRIVE1
	GPIO_setOutputLowOnPin(NLED_DRIVE1_PORT, NLED_DRIVE1_PIN);	//NLED_DRIVE1

	GPIO_setAsOutputPin(NLED_DRIVE2_PORT, NLED_DRIVE2_PIN);	//NLED_DRIVE2
	GPIO_setOutputLowOnPin(NLED_DRIVE2_PORT, NLED_DRIVE2_PIN);	//NLED_DRIVE2

	GPIO_setAsOutputPin(NLED_DRIVE3_PORT, NLED_DRIVE3_PIN);	//NLED_DRIVE3
	GPIO_setOutputLowOnPin(NLED_DRIVE3_PORT, NLED_DRIVE3_PIN);	//NLED_DRIVE3

	GPIO_setAsOutputPin(SENSOR_CNTRL1_PORT, SENSOR_CNTRL1_PIN);
	GPIO_setOutputLowOnPin(SENSOR_CNTRL1_PORT, SENSOR_CNTRL1_PIN);

	GPIO_setAsOutputPin(SENSOR_CNTRL2_PORT, SENSOR_CNTRL2_PIN);
	GPIO_setOutputLowOnPin(SENSOR_CNTRL2_PORT, SENSOR_CNTRL2_PIN);

	GPIO_setAsOutputPin(MAGTEK_PWR_PORT, MAGTEK_PWR_PIN);	//Mag card reader power
	MAG_1_POWER_OFF();	//Power OFF card reader

	GPIO_setAsOutputPin(IDTECH_PWR_PORT, IDTECH_PWR_PIN);	//IDTECH card reader power
	IDTech_POWER_OFF();	//Power OFF ID Tech Reader

	GPIO_setAsOutputPin(PCLK_CN_PORT, PCLK_CN_PIN);
	GPIO_setOutputLowOnPin(PCLK_CN_PORT, PCLK_CN_PIN);	//PCLK_CN

	GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN2);
	GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN2);	//CARD_CNTRL

	GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN6);
	GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN6);	//DK_CNTRL

	GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN4);
	GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN4);	//LRA_NSS

	GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN1);
	GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN1);	//RF_CNTRL

	GPIO_setAsOutputPin(MAGTEK_SDA_PORT, MAGTEK_SDA_PIN);
	GPIO_setOutputLowOnPin(MAGTEK_SDA_PORT, MAGTEK_SDA_PIN);

	GPIO_setAsOutputPin(MAGTEK_SCL_PORT, MAGTEK_SCL_PIN);
	GPIO_setOutputLowOnPin(MAGTEK_SCL_PORT, MAGTEK_SCL_PIN);

	GPIO_setAsOutputPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
	GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);

/*	GPIO_setAsOutputPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
	GPIO_setOutputHighOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);

	GPIO_setAsOutputPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
	GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);

	GPIO_setAsOutputPin(RGLED_PORT, RGLED_PIN);
	GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);*/

	GPIO_setAsInputPinWithPullUpResistor(EXCESS_LED_PORT, EXCESS_LED_PIN);
	GPIO_setAsInputPinWithPullUpResistor(PENALTY_LED_PORT, PENALTY_LED_PIN);
	GPIO_setAsInputPinWithPullUpResistor(RGLED_PORT, RGLED_PIN);

	GPIO_setAsOutputPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
	GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);

	GPIO_setAsOutputPin(KEYDETECT_PORT, KEYDETECT_PIN);
	GPIO_setOutputHighOnPin(KEYDETECT_PORT, KEYDETECT_PIN);

	GPIO_setAsOutputPin(GEMCLUB_RST_PORT, GEMCLUB_RST_PIN);	//SC Reset Line
	GPIO_setOutputLowOnPin(GEMCLUB_RST_PORT, GEMCLUB_RST_PIN);

	//GPIO_setAsInputPinWithPullUpResistor(MDM_RING_PORT, MDM_RING_PIN);
	GPIO_setAsOutputPin(MDM_RING_PORT, MDM_RING_PIN);
	GPIO_setOutputLowOnPin(MDM_RING_PORT, MDM_RING_PIN);

	LORA_power_off();

	UART_disableModule(EUSCI_A0_BASE);
	UART_disableModule(EUSCI_A1_BASE);
	UART_disableModule(EUSCI_A3_BASE);

	WDT_A_holdTimer();           // Stop watchdog timer

	PCM_gotoLPM3();
}
#endif

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
