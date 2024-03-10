//*****************************************************************************
//*  Created on: Nov 3, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432  LibG2_IO_EXP_SPI.c
//
//****************************************************************************

#include "LibG2_IO_EXP_SPI.h"

IOCON 								Iocon_A;
IODIR 								Iodir_A;
IOGPIO 								Iogpio_A;

static void IO_EXP_SPI_TX(uint8_t *TX_buff,uint16_t TX_count);
//static uint8_t IO_EXP_SPI_RX(void);
//static void IOCON_set(IOCON *iocon,uint8_t con_add);  //not used //vinay
//static void IODIR_Set(IODIR *iodir,uint8_t dir_add); //not used //vinay

//void IO_EXP_SPI_init() //not used //vinay
//{
//	//uint16_t i = 0;
//	eUSCI_SPI_MasterConfig MDM_int;
//
//	init_IO_exp_SPI_module_GPIO();
//
//	MDM_int.clockPhase = EUSCI_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
//	MDM_int.clockPolarity = EUSCI_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
//	MDM_int.clockSourceFrequency= 12000000;
//	MDM_int.desiredSpiClock=400000;
//	MDM_int.msbFirst = EUSCI_SPI_MSB_FIRST;
//	MDM_int.selectClockSource= EUSCI_SPI_CLOCKSOURCE_SMCLK;
//	MDM_int.spiMode = EUSCI_SPI_3PIN;// EUSCI_SPI_4PIN_UCxSTE_ACTIVE_LOW;//
//
//	SPI_initMaster(EUSCI_B3_BASE,&MDM_int);
//}

//void IO_EXP_init() //not used //vinay
//{
//	init_IO_exp_module_GPIO();
//
//	IO_EXP_SPI_Chip_Enable();
//
//	Iocon_A.IOCON_set.Haen=1;
//	IOCON_set(&Iocon_A,IOCONA);
//	IO_EX_SPI_Chip_Disable();
//	DelayMs(50);
//
//	IO_EXP_SPI_Chip_Enable();
//	Iodir_A.IODIR_set.IO1=0;	//BOOST_EN
//	Iodir_A.IODIR_set.IO2=0;	//MDM_RST
//	IODIR_Set(&Iodir_A,IODIRA);
//	IO_EX_SPI_Chip_Disable();
//	DelayMs(50);
//
//	IO_EXP_SPI_Chip_Enable();
//	Iogpio_A.IOGPIO_set.GP1=0;	//BOOST_EN is low at start
//	Iogpio_A.IOGPIO_set.GP2=1;	//MDM_RST is high at start
//	IOGPIO_Set(&Iogpio_A,GPIOA);
//	DelayMs(50);
//	IO_EX_SPI_Chip_Disable();
//}

//void Boost_enable() //not used //vinay
//{
//	IO_EXP_SPI_Chip_Enable();
//
//	Iogpio_A.IOGPIO_set.GP1=1;
//	IOGPIO_Set(&Iogpio_A,GPIOA);
//
//	IO_EX_SPI_Chip_Disable();
//}

//void Boost_disable() //not used //vinay
//{
//	IO_EXP_SPI_Chip_Enable();
//
//	Iogpio_A.IOGPIO_set.GP1=0;
//	IOGPIO_Set(&Iogpio_A,GPIOA);
//
//	IO_EX_SPI_Chip_Disable();
//}

//static void IOCON_set(IOCON *iocon,uint8_t con_add)  //not used //vinay
//{
//	uint8_t iocon_buff[3];
//
//	iocon_buff[0]=SPI_Write;
//	iocon_buff[1]=con_add;
//	iocon_buff[2]=iocon->IOCON_value;
//	IO_EXP_SPI_TX(iocon_buff,3);
//}

//static void IODIR_Set(IODIR *iodir,uint8_t dir_add)  //not used //vinay
//{
//	uint8_t iodir_buff[3];
//
//	iodir_buff[0]=SPI_Write;
//	iodir_buff[1]=dir_add;
//	iodir_buff[2]=iodir->IODIR_value;
//	IO_EXP_SPI_TX(iodir_buff,3);
//}

void IOGPIO_Set(IOGPIO *iogpio,uint8_t gpio_add)
{
	uint8_t iogipo_buff[3];

	iogipo_buff[0]=SPI_Write;
	iogipo_buff[1]=gpio_add;
	iogipo_buff[2]=iogpio->IOGPIO_value;
	IO_EXP_SPI_TX(iogipo_buff,3);
}

static void IO_EXP_SPI_TX(uint8_t *TX_buff,uint16_t TX_count)
{
	uint8_t buf_count=0,loop_count=0;

	for(loop_count=0;loop_count<TX_count;loop_count++)
	{
		EUSCI_B_CMSIS(EUSCI_B3_BASE)->TXBUF = *(TX_buff+loop_count);

		while((SPI_isBusy(EUSCI_B3_BASE))&&(buf_count<10))
			buf_count++;
	}
}

#if 0
static uint8_t IO_EXP_SPI_RX(void)
{
	uint8_t buf_count=0,RX_buff=0;

	RX_buff=EUSCI_B_CMSIS(EUSCI_B3_BASE)->RXBUF;

	while((SPI_isBusy(EUSCI_B3_BASE))&&(buf_count<10))
		buf_count++;

	return RX_buff;
}
void set()
{
	IO_EXP_SPI_Chip_Enable();

	Iogpio_A.IOGPIO_set.GP1=1;
	IOGPIO_Set(&Iogpio_A,GPIOA);

	IO_EX_SPI_Chip_Disable();
}

void clr()
{
	IO_EXP_SPI_Chip_Enable();

	Iogpio_A.IOGPIO_set.GP1=0;
	IOGPIO_Set(&Iogpio_A,GPIOA);

	IO_EX_SPI_Chip_Disable();
}
#endif

//void IO_EXP_SPI_Chip_Enable(void) //not used //vinay
//{
//	SPI_enableModule(EUSCI_B3_BASE);        /* enable device */
//	GPIO_setOutputLowOnPin(IO_EXP_CS_PORT, IO_EXP_CS_PIN);
//
//}

//void IO_EX_SPI_Chip_Disable(void) //not used //vinay
//{
//	GPIO_setOutputHighOnPin(IO_EXP_CS_PORT, IO_EXP_CS_PIN);
//	SPI_disableModule(EUSCI_B3_BASE);          /* disable device */
//}

