//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_gpio_init.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_gpio_init_api
//! @{
//
//*****************************************************************************

#include "LibG2_gpio_init.h"

extern char             	special_keys[6];
extern uint8_t              Flash_Operation;
extern uint8_t 				RFLRState;
extern uint8_t			    COMMS_DOWN;
extern GPRSSystem      		glSystem_cfg;

uint8_t						WAITING_FOR_SMS_RESPONSE = FALSE;
uint8_t						HARDWARE_REVISION = LNG_REV4;

uint32_t          			LORA_PWR_OFF_Time = 0,LORA_PWR_On_Time = 0,Total_LORA_PWR_On_Time = 0 ;
uint32_t 					LCD_Backlight_On_Time = 0,LCD_Backlight_Off_Time = 0,Total_LCD_Backlight_On_Time = 0;
extern uint8_t 				Opto_Coin_Disable;
extern uint8_t   			No_Modem_Coin_Only;

/**************************************************************************/
//! Boot up initialization of all IO ports and pins to default
//! \param void
//! \return void
/**************************************************************************/
void gpio_default_init()
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

	GPIO_setAsOutputPin(SENSOR_CNTRL2_PORT, SENSOR_CNTRL2_PIN);
	GPIO_setOutputLowOnPin(SENSOR_CNTRL2_PORT, SENSOR_CNTRL2_PIN);

	GPIO_setAsInputPinWithPullUpResistor(EXT_UID_IO_PORT, EXT_UID_IO_PIN);	//EXT_UID
	GPIO_setAsInputPinWithPullUpResistor(INT_UID_IO_PORT, INT_UID_IO_PIN);	//INT_UID_IO
}

/**************************************************************************/
//! Coin Track Initialization, all IOs and interrupts related to coin track
//! are initialized here
//! \param void
//! \return void
/**************************************************************************/
void init_cointrack_GPIO ()
{
	GPIO_setAsOutputPin(T_PORT, T_PIN);	//T
	GPIO_setAsOutputPin(SNSON_PORT, SNSON_PIN);	//Coin SNSON
	GPIO_setAsOutputPin(CHIRP_PORT, CHIRP_PIN);	//CHIRP
	GPIO_setAsPeripheralModuleFunctionInputPin(CNTR_PORT, CNTR_PIN, GPIO_PRIMARY_MODULE_FUNCTION);	//CNTR
	//GPIO_setAsInputPinWithPullUpResistor(CNTR_PORT, CNTR_PIN);	//CNTR
	GPIO_setAsInputPinWithPullUpResistor(SNSRDY_PORT, SNSRDY_PIN);	//SNSREADY
	GPIO_setAsOutputPin(CLSNS_PORT, CLSNS_PIN);	//CLSNS
	//GPIO_setAsInputPinWithPullUpResistor(LED_RXD_PORT, LED_RXD_PIN);	//LED_RXD
	GPIO_setAsInputPin(LED_RXD_PORT, LED_RXD_PIN);
	GPIO_setAsOutputPin(NLED_DRIVE1_PORT, NLED_DRIVE1_PIN);	//NLED_DRIVE1
	nLed_Drive1_Low();
	GPIO_setAsOutputPin(NLED_DRIVE2_PORT, NLED_DRIVE2_PIN);	//NLED_DRIVE2
	nLed_Drive2_Low();
	if(HARDWARE_REVISION == LNG_REV3)
	{
		GPIO_setAsOutputPin(NLED_DRIVE3_PORT, NLED_DRIVE3_PIN);	//NLED_DRIVE3
		nLed_Drive3_Low();
	}
	else if(HARDWARE_REVISION == LNG_REV4)
	{
		GPIO_setAsOutputPin(NLED_DRIVE3_PORT_R4, NLED_DRIVE3_PIN_R4);	//NLED_DRIVE3
		nLed_Drive3_R4_Low();
	}
	GPIO_setAsOutputPin(PCLK_PORT, PCLK_PIN);	//PCLK

	coin_sensor_off();

	t_High();
	pclk_High();
	chirp_Low();
	clsns_Low();	//1
	//coin_init_ccf_Parameters();
	//start_coinwakeup_timer();
}

/**************************************************************************/
//! Card Reader Initialization, all IOs and interrupts related to Card Reader
//! are initialized here
//! \param Card_Reader_Version
//! - \b GEM_CLUB_READER
//! - \b ID_TECH_READER
//! \return void
/**************************************************************************/
void init_card_interrupt(uint8_t Card_Reader_Version)
{
	if(Card_Reader_Version == GEM_CLUB_READER)
	{
		GPIO_setAsInputPinWithPullUpResistor(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN);		//Interrupt Pin
		GPIO_clearInterruptFlag(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN);		//Clear interrupt flag
		GPIO_interruptEdgeSelect(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN, GPIO_HIGH_TO_LOW_TRANSITION);		//Interrupt Edge Select
		GPIO_enableInterrupt(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN);		//Interrupt Enable
	}
	else
	{
		GPIO_setAsInputPinWithPullDownResistor(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN);		//Interrupt Pin
		GPIO_clearInterruptFlag(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN);		//Clear interrupt flag
		GPIO_interruptEdgeSelect(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN, GPIO_LOW_TO_HIGH_TRANSITION);		//Interrupt Edge Select
		GPIO_enableInterrupt(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN);		//Interrupt Enable
	}
	//Register/Enable Port 3 interrupt on NVIC
	//__enable_interrupt();
	NVIC->ISER[1] = 1 << ((PORT3_IRQn) & 31); // Enable PORT3_IRQn interrupt in NVIC module

	MAG_1_POWER_OFF();
}

/**************************************************************************/
//! Enables the Optical LED interrupt which will be used during Coin
//! detection in REV4 boards
//! \param void
//! \return void
/**************************************************************************/
void enable_optical_LED_interrupt()
{
	if(Opto_Coin_Disable == FALSE) //if coin params found in Opto index 0,1,2 VT
	{
		GPIO_setAsInputPin(LED_RXD_PORT,LED_RXD_PIN);		//Interrupt Pin
		GPIO_clearInterruptFlag(LED_RXD_PORT,LED_RXD_PIN);		//Clear interrupt flag
		GPIO_interruptEdgeSelect(LED_RXD_PORT,LED_RXD_PIN, GPIO_LOW_TO_HIGH_TRANSITION);		//Interrupt Edge Select
		GPIO_enableInterrupt(LED_RXD_PORT,LED_RXD_PIN);		//Interrupt Enable

		NVIC->ISER[1] = 1 << ((PORT4_IRQn) & 31);

		nLed_Drive1_High();
		//nLed_Drive2_High();
		if(HARDWARE_REVISION == LNG_REV4)
		{
			nLed_Drive3_R4_High();
		}
		else
		{
			nLed_Drive3_High();
		}
	}

	if(HARDWARE_REVISION == LNG_REV4)
	{
		GPIO_setAsInputPin(SNSRDY_PORT,SNSRDY_PIN);		//Interrupt Pin
		GPIO_clearInterruptFlag(SNSRDY_PORT,SNSRDY_PIN);		//Clear interrupt flag
		GPIO_interruptEdgeSelect(SNSRDY_PORT,SNSRDY_PIN, GPIO_LOW_TO_HIGH_TRANSITION);		//Interrupt Edge Select
		GPIO_enableInterrupt(SNSRDY_PORT,SNSRDY_PIN);		//Interrupt Enable

		NVIC->ISER[1] = 1 << ((PORT6_IRQn) & 31);
	}
}

/**************************************************************************/
//! Disables the Optical LED interrupt which was used during Coin detection
//! in REV4 boards
//! \param void
//! \return void
/**************************************************************************/
void disable_optical_LED_interrupt()
{
	GPIO_clearInterruptFlag(LED_RXD_PORT,LED_RXD_PIN);		//Clear interrupt flag
	GPIO_disableInterrupt(LED_RXD_PORT,LED_RXD_PIN);		//Interrupt Disable

	GPIO_clearInterruptFlag(SNSRDY_PORT,SNSRDY_PIN);
	GPIO_disableInterrupt(SNSRDY_PORT,SNSRDY_PIN);
	nLed_Drive1_Low();
	nLed_Drive2_Low();
	if(HARDWARE_REVISION == LNG_REV4)
	{
		nLed_Drive3_R4_Low();
	}
	else
	{
		nLed_Drive3_Low();
	}
}

/**************************************************************************/
//! Magtek Card Reader Initialization, all IOs and interrupts related MagTek
//! reader are initialized here
//! \param void
//! \return void
/**************************************************************************/
void init_mag_card_reader_GPIO (void)
{
	GPIO_setAsOutputPin(MAGTEK_PWR_PORT, MAGTEK_PWR_PIN);	//Mag card reader power
	MAG_1_POWER_OFF();	//Power OFF card reader

	GPIO_setAsInputPinWithPullDownResistor(MAGTEK_SDA_PORT, MAGTEK_SDA_PIN);		//SDA Signal

	GPIO_setAsOutputPin(MAGTEK_SCL_PORT, MAGTEK_SCL_PIN);		//STROBE/SCL Signal
	GPIO_setOutputLowOnPin(MAGTEK_SCL_PORT, MAGTEK_SCL_PIN);
}

/**************************************************************************/
//! IDTech Card Reader Initialization, all IOs and interrupts related IDTech
//! reader are initialized here
//! \param void
//! \return void
/**************************************************************************/
void init_IDTech_card_reader_GPIO (void)
{
	GPIO_setAsOutputPin(IDTECH_PWR_PORT, IDTECH_PWR_PIN);	//IDTech card reader power
	IDTech_POWER_OFF();

	GPIO_setAsOutputPin(GEMCLUB_RST_PORT, GEMCLUB_RST_PIN);	//SC Reset Line
	GPIO_setOutputHighOnPin(GEMCLUB_RST_PORT, GEMCLUB_RST_PIN);

	init_IDTech_UART();
}

/**************************************************************************/
//! Keypad Initialization, all IOs and interrupts related Keypads
//! \param void
//! \return void
/**************************************************************************/
void init_keypad_GPIO (void)
{
	//if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))
	if((No_Modem_Coin_Only == true) || ((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))) //COIN Only LNG with NO Comms
	{
		GPIO_setAsInputPinWithPullUpResistor(KEYPAD_PORT, KEYPAD_RT_PIN|KEYPAD_LT_PIN|KEYPAD_CAN_PIN|KEYPAD_OKK_PIN);
		GPIO_clearInterruptFlag(KEYPAD_PORT, KEYPAD_RT_PIN|KEYPAD_LT_PIN|KEYPAD_CAN_PIN|KEYPAD_OKK_PIN);
		GPIO_interruptEdgeSelect(KEYPAD_PORT, KEYPAD_RT_PIN|KEYPAD_LT_PIN|KEYPAD_CAN_PIN|KEYPAD_OKK_PIN, GPIO_HIGH_TO_LOW_TRANSITION );
		GPIO_enableInterrupt(KEYPAD_PORT, KEYPAD_RT_PIN|KEYPAD_LT_PIN|KEYPAD_CAN_PIN|KEYPAD_OKK_PIN);
	}
	else
	{
		GPIO_setAsInputPinWithPullUpResistor(KEYPAD_PORT, KEYPAD_UP_PIN|KEYPAD_DOWN_PIN|KEYPAD_OK_PIN|KEYPAD_CANCEL_PIN);
		GPIO_clearInterruptFlag(KEYPAD_PORT, KEYPAD_UP_PIN|KEYPAD_DOWN_PIN|KEYPAD_OK_PIN|KEYPAD_CANCEL_PIN);
		GPIO_interruptEdgeSelect(KEYPAD_PORT, KEYPAD_UP_PIN|KEYPAD_DOWN_PIN|KEYPAD_OK_PIN|KEYPAD_CANCEL_PIN, GPIO_HIGH_TO_LOW_TRANSITION );
		GPIO_enableInterrupt(KEYPAD_PORT, KEYPAD_UP_PIN|KEYPAD_DOWN_PIN|KEYPAD_OK_PIN|KEYPAD_CANCEL_PIN);
	}

	memset( special_keys, 0, sizeof(special_keys) );

	NVIC->ISER[1] = 1 << ((PORT2_IRQn) & 31); // Enable PORT2_IRQn interrupt in NVIC module
}

/**************************************************************************/
//! Magtek Card Reader Initialization, all IOs and interrupts related GemClub
//! Smart Card reader are initialized here
//! \param void
//! \return void
/**************************************************************************/
void init_smartcard_GPIO(void)
{
	//GPIO_setAsPeripheralModuleFunctionOutputPin(GEMCLUB_CLK_PORT, GEMCLUB_CLK_PIN, GPIO_PRIMARY_MODULE_FUNCTION);	//MCLK Out
	GPIO_setAsOutputPin(PCLK_CN_PORT, PCLK_CN_PIN);	//PCLK_CN
	GPIO_setOutputLowOnPin(PCLK_CN_PORT, PCLK_CN_PIN);	//PCLK_CN

	GPIO_setAsOutputPin(GEMCLUB_RST_PORT, GEMCLUB_RST_PIN);	//SC Reset Line
	GPIO_setOutputHighOnPin(GEMCLUB_RST_PORT, GEMCLUB_RST_PIN);

	GPIO_setAsPeripheralModuleFunctionInputPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN, GPIO_PRIMARY_MODULE_FUNCTION);	//SC IO Line

    init_SmartCard_UART();
}

/**************************************************************************/
//! Turn Off all IOs and interrupts related to Card Reader in order to save power
//! \param Card_Reader_Version
//! - \b GEM_CLUB_READER
//! - \b ID_TECH_READER
//! \return void
/**************************************************************************/
void Turn_Off_card_IOs(uint8_t Card_Reader_Version)
{
	MAG_1_POWER_OFF();
	if(Card_Reader_Version == GEM_CLUB_READER)
	{
		GPIO_setOutputLowOnPin(GEMCLUB_RST_PORT, GEMCLUB_RST_PIN);
		GPIO_setOutputLowOnPin(PCLK_CN_PORT, PCLK_CN_PIN);	//Power Optimization
		pclk_Off();

		UART_disableModule(EUSCI_A3_BASE);
		GPIO_setAsOutputPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN);
		GPIO_setOutputLowOnPin(GEMCLUB_IO_PORT, GEMCLUB_IO_PIN);
	}
	GPIO_setAsInputPinWithPullDownResistor(MAGTEK_SDA_PORT, MAGTEK_SDA_PIN);	//SDA Signal
}

/**************************************************************************/
//! All IOs related to rechargeable battery reading are initialized here
//! \param void
//! \return void
/**************************************************************************/
void init_rchbattery_GPIO (void)
{
	GPIO_setAsOutputPin(SENSOR_CNTRL1_PORT, SENSOR_CNTRL1_PIN);
	GPIO_setOutputHighOnPin(SENSOR_CNTRL1_PORT, SENSOR_CNTRL1_PIN);
	GPIO_setAsPeripheralModuleFunctionInputPin(RECH_BATT_PORT, RECH_BATT_PIN, GPIO_TERTIARY_MODULE_FUNCTION);
}

#if 0
void init_solarbattery_GPIO (void)
{
	GPIO_setAsOutputPin(SENSOR_CNTRL1_PORT, SENSOR_CNTRL1_PIN);
	GPIO_setOutputHighOnPin(SENSOR_CNTRL1_PORT, SENSOR_CNTRL1_PIN);
	GPIO_setAsPeripheralModuleFunctionInputPin(SOLAR_BATT_PORT, SOLAR_BATT_PIN, GPIO_TERTIARY_MODULE_FUNCTION);
}


void init_adc_module_GPIO (void)
{
	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN6 | GPIO_PIN7, GPIO_TERTIARY_MODULE_FUNCTION);
	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN0 | GPIO_PIN1, GPIO_TERTIARY_MODULE_FUNCTION);
}

void init_IO_exp_SPI_module_GPIO (void)
{
	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6,GPIO_PIN6 | GPIO_PIN7 , GPIO_SECONDARY_MODULE_FUNCTION);
	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P8,GPIO_PIN1  , GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsOutputPin(GPIO_PORT_P10,GPIO_PIN0);
	GPIO_setOutputHighOnPin(GPIO_PORT_P10,GPIO_PIN0);
}

void init_IO_exp_module_GPIO (void)
{
	GPIO_setAsOutputPin(GPIO_PORT_P8,GPIO_PIN6);
	GPIO_setOutputLowOnPin(GPIO_PORT_P8,GPIO_PIN6);
	DelayMs(50);
	GPIO_setOutputHighOnPin(GPIO_PORT_P8,GPIO_PIN6);
}
#endif

/**************************************************************************/
//! All IOs related to external flash are initialized here
//! \param void
//! \return void
/**************************************************************************/
void init_flash_SPI_module_GPIO (void)
{
	Flash_init_SPI();
	GPIO_setAsPeripheralModuleFunctionInputPin(FLASH_SPI_IOPORT,FLASH_SPI_MOSI_PIN | FLASH_SPI_MISO_PIN | FLASH_SPI_SCK_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsOutputPin(FLASH_SPI_IOPORT, FLASH_SPI_CS_PIN);
	GPIO_setOutputHighOnPin(FLASH_SPI_IOPORT, FLASH_SPI_CS_PIN);
	GPIO_setAsOutputPin(DataKey_SPI_IOPORT, DataKey_SPI_CS_PIN);
	GPIO_setOutputHighOnPin(DataKey_SPI_IOPORT, DataKey_SPI_CS_PIN);
}

/**************************************************************************/
//! All IOs related to Sharp LCD display are initialized here
//! \param void
//! \return void
/**************************************************************************/
void init_display_SPI_module_GPIO (void)
{
	Display_HWSPI_Init();
	GPIO_setAsPeripheralModuleFunctionInputPin(DISPLAY_SPI_IOPORT,DISPLAY_SPI_SCK_PIN |
			DISPLAY_SPI_MOSI_PIN | DISPLAY_SPI_MISO_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsOutputPin(DISPLAY_SPI_CS_IOPORT,DISPLAY_SPI_CS_PIN);
	GPIO_setOutputLowOnPin(DISPLAY_SPI_CS_IOPORT,DISPLAY_SPI_CS_PIN);
}

/**************************************************************************/
//! All IOs related to Datakey are initialized here
//! \param void
//! \return void
/**************************************************************************/
void init_DataKey_SPI_module_GPIO (void)
{
	DataKey_init_SPI();
	GPIO_setAsPeripheralModuleFunctionInputPin(FLASH_SPI_IOPORT,FLASH_SPI_MOSI_PIN | FLASH_SPI_MISO_PIN | FLASH_SPI_SCK_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsOutputPin(DataKey_SPI_IOPORT, DataKey_SPI_CS_PIN);
	GPIO_setOutputHighOnPin(DataKey_SPI_IOPORT, DataKey_SPI_CS_PIN); //enable Datakey.

	if(HARDWARE_REVISION == LNG_REV3)
	{
		GPIO_setAsOutputPin(DK_CNTRL_PORT, DK_CNTRL_PIN);
		GPIO_setOutputLowOnPin(DK_CNTRL_PORT, DK_CNTRL_PIN);

		GPIO_setAsOutputPin(KEYDETECT_PORT, KEYDETECT_PIN);
		GPIO_setOutputHighOnPin(KEYDETECT_PORT, KEYDETECT_PIN);
	}
	else if(HARDWARE_REVISION == LNG_REV4)
	{
		GPIO_setAsInputPin(KEYDETECT_PORT, KEYDETECT_PIN);
	}
}

/**************************************************************************/
//! All IOs related to Debug Log are initialized here
//! \param void
//! \return void
/**************************************************************************/
void init_debug_GPIO (void)
{
	init_debug_UART();
	GPIO_setAsPeripheralModuleFunctionInputPin(DEBUG_PORT, DEBUG_RX_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsPeripheralModuleFunctionOutputPin(DEBUG_PORT, DEBUG_TX_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
}

/**************************************************************************/
//! All IOs related to Telit Modem are initialized here
//! \param void
//! \return void
/**************************************************************************/
void init_Telit_GPIO (void)
{
	GPIO_setAsOutputPin(MDM_REG_CNTRL_PORT, MDM_REG_CNTRL_PIN);	//MDM_REG_CNTRL
	GPIO_setAsOutputPin(MDM_ON_PORT, MDM_ON_PIN);	//MDM_ON
	GPIO_setAsOutputPin(MDM_DTR_PORT, MDM_DTR_PIN);	//MDM_DTR
	GPIO_setAsOutputPin(MDM_RST_PORT, MDM_RST_PIN);	//MDM_RST

	init_Telit_UART();
	GPIO_setAsPeripheralModuleFunctionInputPin(MDM_PORT, MDM_RXD_PIN, GPIO_PRIMARY_MODULE_FUNCTION);	//MDM_RXD
	GPIO_setAsPeripheralModuleFunctionOutputPin(MDM_PORT, MDM_TXD_PIN, GPIO_PRIMARY_MODULE_FUNCTION);	//MDM_TXD
}

/**************************************************************************/
//! Disables the Ring Indicator interrupt from Telit
//! \param void
//! \return void
/**************************************************************************/
void disable_telit_RI_interrupt()
{
	COMMS_DOWN = FALSE;
	WAITING_FOR_SMS_RESPONSE = FALSE;
	GPIO_disableInterrupt(MDM_RING_PORT, MDM_RING_PIN);		//Interrupt Disable

	//GPIO_setAsOutputPin(MDM_RING_PORT, MDM_RING_PIN);
	//GPIO_setOutputLowOnPin(MDM_RING_PORT, MDM_RING_PIN);
	GPIO_setAsInputPinWithPullDownResistor(MDM_RING_PORT, MDM_RING_PIN);	//MDM_RING Interrupt Pin
}

/**************************************************************************/
//! Enables the Ring Indicator interrupt from Telit
//! \param void
//! \return void
/**************************************************************************/
void enable_telit_RI_interrupt()
{
	GPIO_setAsInputPinWithPullDownResistor(MDM_RING_PORT, MDM_RING_PIN);	//MDM_RING Interrupt Pin
	GPIO_clearInterruptFlag(MDM_RING_PORT, MDM_RING_PIN);		//Clear interrupt flag
	GPIO_interruptEdgeSelect(MDM_RING_PORT, MDM_RING_PIN, GPIO_HIGH_TO_LOW_TRANSITION);		//Interrupt Edge Select
	GPIO_enableInterrupt(MDM_RING_PORT, MDM_RING_PIN);		//Interrupt Enable

	NVIC->ISER[1] = 1 << ((PORT3_IRQn) & 31); // Enable PORT3_IRQn interrupt in NVIC module
	WAITING_FOR_SMS_RESPONSE = TRUE;
}

/**************************************************************************/
//! Turns On LCD Backlight
//! \param void
//! \return void
/**************************************************************************/
void BKLT_ON(void)
{
	GPIO_setOutputHighOnPin(DISPLAY_BKLT_PORT, DISPLAY_BKLT_PIN);
	LCD_Backlight_On_Time = RTC_epoch_now();
}

/**************************************************************************/
//! Turns Off LCD Backlight
//! \param void
//! \return void
/**************************************************************************/
void BKLT_OFF(void)
{
	GPIO_setOutputLowOnPin(DISPLAY_BKLT_PORT, DISPLAY_BKLT_PIN);

	LCD_Backlight_Off_Time = RTC_epoch_now();
	if(LCD_Backlight_On_Time == 0)
	{
		LCD_Backlight_On_Time = LCD_Backlight_Off_Time;
	}
	Total_LCD_Backlight_On_Time +=((uint32_t)LCD_Backlight_Off_Time - (uint32_t)LCD_Backlight_On_Time);
	//Debug_Output1(2,"Total_LCD_Backlight_On_Time =:%d",Total_LCD_Backlight_On_Time);
	LCD_Backlight_On_Time = 0;
}

/**************************************************************************/
//! Turns On Magtek Card Reader
//! \param void
//! \return void
/**************************************************************************/
void MAG_1_POWER_ON(void)
{
	GPIO_setOutputHighOnPin(MAGTEK_PWR_PORT, MAGTEK_PWR_PIN);
}

/**************************************************************************/
//! Turns Off Magtek Card Reader
//! \param void
//! \return void
/**************************************************************************/
void MAG_1_POWER_OFF(void)
{
	GPIO_setOutputLowOnPin(MAGTEK_PWR_PORT, MAGTEK_PWR_PIN);
}

/**************************************************************************/
//! Turns Off IDTech Card Reader
//! \param void
//! \return void
/**************************************************************************/
void IDTech_POWER_OFF(void)
{
	GPIO_setOutputLowOnPin(IDTECH_PWR_PORT, IDTECH_PWR_PIN);
}

/**************************************************************************/
//! Turns On IDTech Card Reader
//! \param void
//! \return void
/**************************************************************************/
void IDTech_POWER_ON(void)
{
	GPIO_setOutputHighOnPin(IDTECH_PWR_PORT, IDTECH_PWR_PIN);
	DelayMs(50);
}

/**************************************************************************/
//! Sets DTR line to Telit Modem
//! \param void
//! \return void
/**************************************************************************/
void MDM_DTR_set()
{
	GPIO_setOutputLowOnPin(MDM_DTR_PORT, MDM_DTR_PIN);
}

/**************************************************************************/
//! Clears DTR line to Telit Modem
//! \param void
//! \return void
/**************************************************************************/
void MDM_DTR_clr()
{
	GPIO_setOutputHighOnPin(MDM_DTR_PORT, MDM_DTR_PIN);
}

/**************************************************************************/
//! Turns On the Power Regulator in Comms Board
//! \param void
//! \return void
/**************************************************************************/
void MDM_REG_CNTRL_set()
{
	GPIO_setOutputHighOnPin(MDM_REG_CNTRL_PORT, MDM_REG_CNTRL_PIN);
}

/**************************************************************************/
//! Turns Off the Power Regulator in Comms Board
//! \param void
//! \return void
/**************************************************************************/
void MDM_REG_CNTRL_clr()
{
	GPIO_setOutputLowOnPin(MDM_REG_CNTRL_PORT, MDM_REG_CNTRL_PIN);
}

/**************************************************************************/
//! Presses MDM_ON switch for Telit modem
//! \param void
//! \return void
/**************************************************************************/
void MDM_Tel_ON()
{
	GPIO_setOutputHighOnPin(MDM_ON_PORT, MDM_ON_PIN);
}

/**************************************************************************/
//! Releases MDM_ON switch for Telit modem
//! \param void
//! \return void
/**************************************************************************/
void MDM_Tel_OFF()
{
	GPIO_setOutputLowOnPin(MDM_ON_PORT, MDM_ON_PIN);
}

/**************************************************************************/
//! Sets Reset pin for Telit modem
//! \param void
//! \return void
/**************************************************************************/
void MDM_RST_set()
{
	GPIO_setOutputHighOnPin(MDM_RST_PORT, MDM_RST_PIN);
	/*IO_EXP_SPI_Chip_Enable();

	Iogpio_A.IOGPIO_set.GP2=1;
	IOGPIO_Set(&Iogpio_A,GPIOA);

	IO_EX_SPI_Chip_Disable();*/
}

/**************************************************************************/
//! Clears Reset pin for Telit modem
//! \param void
//! \return void
/**************************************************************************/
void MDM_RST_clr()
{
	GPIO_setOutputLowOnPin(MDM_RST_PORT, MDM_RST_PIN);
	/*IO_EXP_SPI_Chip_Enable();

	Iogpio_A.IOGPIO_set.GP2=0;
	IOGPIO_Set(&Iogpio_A,GPIOA);

	IO_EX_SPI_Chip_Disable();*/
}

/**************************************************************************/
//! All IOs related to Sharp LCD display to operate in a software SPI mode
//! are initialized here
//! \param void
//! \return void
/**************************************************************************/
void Display_Init_SWSPI(void)
{
	//CS
	GPIO_setAsOutputPin(DISPLAY_SPI_CS_IOPORT,DISPLAY_SPI_CS_PIN);
	GPIO_setOutputLowOnPin(DISPLAY_SPI_CS_IOPORT,DISPLAY_SPI_CS_PIN);

	//CLK
	GPIO_setAsOutputPin(DISPLAY_SPI_IOPORT , DISPLAY_SPI_SCK_PIN);
	GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_SCK_PIN);

	//MOSI
	GPIO_setAsOutputPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_MOSI_PIN );
	GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_MOSI_PIN );

	Flash_Operation = LCD_SPI;
}

/**************************************************************************/
//! Turns On LORA module
//! \param void
//! \return void
/**************************************************************************/
void LORA_power_on()
{
	//if(glSystem_cfg.LORA_POWER_ENABLE == TRUE)
	{
		Debug_TextOut(0, "LORA PWR ON");
		GPIO_setAsOutputPin(RF_CNTRL_PORT, RF_CNTRL_PIN);
		GPIO_setOutputHighOnPin(RF_CNTRL_PORT, RF_CNTRL_PIN);
		LORA_PWR_On_Time = RTC_epoch_now();
		DelayUs(200);
	}
	//else
	//	Debug_TextOut(0, "LORA PWR ON - Disabled in Config");
}

/**************************************************************************/
//! Turns Off LORA module
//! \param void
//! \return void
/**************************************************************************/
void LORA_power_off()
{
	Debug_TextOut(0, "LORA PWR OFF");
	GPIO_setAsOutputPin(RF_CNTRL_PORT, RF_CNTRL_PIN);
	GPIO_setOutputLowOnPin(RF_CNTRL_PORT, RF_CNTRL_PIN);
	SPI_disableModule(EUSCI_B2_BASE);
	if(HARDWARE_REVISION == LNG_REV3)
	{
		GPIO_setAsOutputPin(LORA_SPI_IOPORT, LORA_NSS_PIN);	//LRA_NSS
		GPIO_setOutputLowOnPin(LORA_SPI_IOPORT, LORA_NSS_PIN);	//LRA_NSS
		GPIO_setAsOutputPin(LORA_RESET_IOPORT, LORA_RESET_PIN);	//LRA_RESET
		GPIO_setOutputLowOnPin(LORA_RESET_IOPORT, LORA_RESET_PIN);	//LRA_RESET
	}
	else if(HARDWARE_REVISION == LNG_REV4)
	{
		GPIO_setAsInputPinWithPullDownResistor(LORA_SPI_IOPORT, LORA_NSS_PIN);	//LRA_NSS
		GPIO_setAsInputPin(LORA_RESET_IOPORT, LORA_RESET_PIN);	//LRA_RESET
	}
	LORA_PWR_OFF_Time = RTC_epoch_now();
	if(LORA_PWR_On_Time == 0)
	{
		LORA_PWR_On_Time = LORA_PWR_OFF_Time;
	}
	Total_LORA_PWR_On_Time +=((uint32_t)LORA_PWR_OFF_Time - (uint32_t)LORA_PWR_On_Time);
	//Debug_Output1(3,"Total_LORA_PWR_On_Time =:%d",Total_LORA_PWR_On_Time);
	LORA_PWR_On_Time = 0;
	DelayUs(200);

	RFLRState = RFLR_STATE_IDLE;	// fix for LORA getting stuck
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

