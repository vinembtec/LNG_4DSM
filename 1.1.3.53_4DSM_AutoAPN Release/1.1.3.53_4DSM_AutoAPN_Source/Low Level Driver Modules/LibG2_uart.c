//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_uart.c
//
//****************************************************************************

#include "LibG2_uart.h"

void init_debug_UART()
{
	eUSCI_UART_Config debug_config;

	debug_config.clockPrescalar = 6;
	debug_config.firstModReg = 8;
	debug_config.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
	debug_config.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
	debug_config.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
	debug_config.parity = EUSCI_A_UART_NO_PARITY;
	debug_config.secondModReg = 32;
	debug_config.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
	debug_config.uartMode = EUSCI_A_UART_MODE;

	UART_initModule(EUSCI_A1_BASE, &debug_config);

	UART_enableModule(EUSCI_A1_BASE);
}

void init_Telit_UART()
{
	eUSCI_UART_Config Telit_Mdm_config;
	Telit_Mdm_config.clockPrescalar = 6;
	Telit_Mdm_config.firstModReg = 8;
	Telit_Mdm_config.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
	Telit_Mdm_config.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
	Telit_Mdm_config.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
	Telit_Mdm_config.parity = EUSCI_A_UART_NO_PARITY;
	Telit_Mdm_config.secondModReg = 32;
	Telit_Mdm_config.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
	Telit_Mdm_config.uartMode = EUSCI_A_UART_MODE;

	//__enable_interrupt();
	NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);

	UART_initModule(EUSCI_A0_BASE, &Telit_Mdm_config);
	UCA0CTLW0 |= EUSCI_A_UART_RECEIVE_ERRONEOUSCHAR_INTERRUPT;
	//UART_enableInterrupt(EUSCI_A0_BASE,EUSCI_A_UART_RECEIVE_INTERRUPT);
	UCA0CTLW0 &= ~UCSWRST;                  // Initialize eUSCI
	UCA0IE |= UCRXIE;                       // Enable USCI_A0 RX interrupt
	UART_enableModule(EUSCI_A0_BASE);
}

void init_SmartCard_UART()
{
	eUSCI_UART_Config Card_config;
	Card_config.clockPrescalar = 72;//24;//26; //changed from 73 to 72 for new R4.1/2/3 for smart card//VT 
	Card_config.firstModReg = 0;//2;//0; //for new R4.1/2/3 for smart card//VT
	Card_config.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
	Card_config.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
	Card_config.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
	Card_config.parity = EUSCI_A_UART_EVEN_PARITY;
	Card_config.secondModReg = 0;//187;//32;//214; //for new R4.1/2/3 for smart card//VT
	Card_config.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
	Card_config.uartMode = EUSCI_A_UART_MODE;

    //__enable_interrupt();
    NVIC->ISER[0] = 1 << ((EUSCIA3_IRQn) & 31);

    UART_initModule(EUSCI_A3_BASE, &Card_config);
	UCA3CTLW0 |= EUSCI_A_UART_RECEIVE_ERRONEOUSCHAR_INTERRUPT;
    //UART_enableInterrupt(EUSCI_A3_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
   	UCA3CTLW0 &= ~UCSWRST;                  // Initialize eUSCI
  	UCA3IE |= UCRXIE;                       // Enable USCI_A0 RX interrupt
    UART_enableModule(EUSCI_A3_BASE);
}

uint16_t debug_out_UART(const char *debug_uart_buf)
{
	uint16_t debug_uart_bytes_counter;
	uint16_t debug_uart_length = 0;
	debug_uart_length = (uint16_t) strlen(debug_uart_buf);
	for(debug_uart_bytes_counter=0; debug_uart_bytes_counter<debug_uart_length; debug_uart_bytes_counter++)
	{
		UART_transmitData(EUSCI_A1_BASE, debug_uart_buf[debug_uart_bytes_counter]);
	}
	DelayMs(1);
	return debug_uart_length;
}

uint16_t Modem_out_UART(uint8_t * cmdstr, uint16_t cmdlen)
{
	uint16_t modem_uart_bytes_counter;
	DelayMs(1);
	for(modem_uart_bytes_counter=0; modem_uart_bytes_counter<cmdlen; modem_uart_bytes_counter++)
	{
		UART_transmitData(EUSCI_A0_BASE, cmdstr[modem_uart_bytes_counter]);
		DelayUs(1);
	}
	//DelayMs(1);
	return MDM_ERR_NONE;
}


void init_IDTech_UART()
{
	/* Selecting P1.2 and P1.3 in UART mode and P1.0 as output (LED) */
	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P9,
			GPIO_PIN6 | GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

	eUSCI_UART_Config IDtech_Config;
	IDtech_Config.clockPrescalar =19;// 6;//19;//24;//26;
	IDtech_Config.firstModReg = 8;//0;
	IDtech_Config.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
	IDtech_Config.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
	IDtech_Config.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
	IDtech_Config.parity = EUSCI_A_UART_NO_PARITY;
	IDtech_Config.secondModReg = 85;//32;//85;//32;//214;
	IDtech_Config.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
	IDtech_Config.uartMode = EUSCI_A_UART_MODE;

    NVIC->ISER[0] = 1 << ((EUSCIA3_IRQn) & 31);
	/* Configuring UART Module */
	UART_initModule(EUSCI_A3_BASE, &IDtech_Config);

   	UCA3CTLW0 &= ~UCSWRST;                  // Initialize eUSCI
  	UCA3IE |= UCRXIE;                       // Enable USCI_A0 RX interrupt
	/* Enable UART module */
	UART_enableModule(EUSCI_A3_BASE);
	//Interrupt_enableInterrupt(INT_EUSCIA3);
	//Interrupt_enableMaster();
}





