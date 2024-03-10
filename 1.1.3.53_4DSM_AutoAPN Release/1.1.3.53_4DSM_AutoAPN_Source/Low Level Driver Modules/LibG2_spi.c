//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_spi.c
//
//****************************************************************************

#include "LibG2_spi.h"

extern uint8_t                     	Flash_Operation;
extern uint16_t 					bits_count;
extern volatile uint8_t 			VCOM;
extern uint8_t						HARDWARE_REVISION;

void Flash_init_SPI()
{
	eUSCI_SPI_MasterConfig Flash_int;

	Flash_int.clockPhase = EUSCI_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
	Flash_int.clockPolarity = EUSCI_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
	Flash_int.clockSourceFrequency= 12000000;
	Flash_int.desiredSpiClock= 1000000;
	Flash_int.msbFirst = EUSCI_SPI_MSB_FIRST;
	Flash_int.selectClockSource= EUSCI_SPI_CLOCKSOURCE_SMCLK;
	Flash_int.spiMode = EUSCI_SPI_3PIN;// EUSCI_SPI_4PIN_UCxSTE_ACTIVE_LOW;//

	SPI_initMaster(EUSCI_B1_BASE,&Flash_int);
}

void Display_HWSPI_Init()
{
	eUSCI_SPI_MasterConfig 		spiDMasterConfig;

	spiDMasterConfig.selectClockSource=EUSCI_SPI_CLOCKSOURCE_SMCLK; // ACLK Clock Source
	spiDMasterConfig.clockSourceFrequency=12000000; // MCLK = DCO = 3Mhz
	spiDMasterConfig.desiredSpiClock=2000000; //1000000; // SPICLK = 1Mhz
	spiDMasterConfig.msbFirst=EUSCI_B_SPI_LSB_FIRST; // LSB First
	spiDMasterConfig.clockPhase=EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT; // Phase
	spiDMasterConfig.clockPolarity=EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW; // High polarity
	spiDMasterConfig.spiMode=EUSCI_B_SPI_3PIN; // 3Wire SPI Mode

	SPI_initMaster(EUSCI_B1_BASE, &spiDMasterConfig);
	//SPI_enableModule(EUSCI_B1_BASE);

	VCOM = 0;											// initialize VCOM, this flag controls LCD polarity
	// and has to be toggled every second or so
	Flash_Operation = LCD_SPI;
}

void DataKey_init_SPI()
{
	Flash_init_SPI();
	//Flash_Operation = DATAKEY_SPI; //DATAKEY FLASH
}

void Flash_SPI_TX(uint8_t *TX_buff,uint16_t TX_count)
{
	//__disable_interrupt();
	uint16_t buf_count=0,loop_count=0; //Datakey //VT

	for(loop_count=0;loop_count<TX_count;loop_count++)
	{
		EUSCI_B_CMSIS(EUSCI_B1_BASE)->TXBUF = *(TX_buff+loop_count);

		while((SPI_isBusy(EUSCI_B1_BASE))&&(buf_count<10))
			buf_count++;
	}
	//__enable_interrupt();
}

uint8_t Flash_SPI_RX(void)
{
	//__disable_interrupt();
	uint16_t buf_count=0,RX_buff=0;  //Datakey //VT

	RX_buff=EUSCI_B_CMSIS(EUSCI_B1_BASE)->RXBUF;

	while((SPI_isBusy(EUSCI_B1_BASE))&&(buf_count<10))
		buf_count++;

	//__enable_interrupt();
	return RX_buff;
}

// send one byte over SPI, does not handle SCS
// input: value		byte to be sent
//Output LSB bit first
void SPIWriteBitBangByte(uint8_t value)
{
	uint8_t i = 0;
	uint8_t last_bit = 0XFF;

	while ( ++i <= 8 )
	{
		if((value & 0x01) != last_bit)
		{
			if ((value & 0x01) == 0x01) //check if LSB is high
			{
				//GPIO_setOutputHighOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_MOSI_PIN );
				DISPLAY_SPI_DATA |= DISPLAY_SPI_MOSI_PIN;
				last_bit = 1;
			}
			else /* if not, set to low */
			{

				//GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_MOSI_PIN );
				DISPLAY_SPI_DATA &= ~DISPLAY_SPI_MOSI_PIN;
				last_bit = 0;
			}
		}

		//DelayUs(10);

		//GPIO_setOutputHighOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_SCK_PIN);/* toggle clock high */
		DISPLAY_SPI_DATA |= DISPLAY_SPI_SCK_PIN;
		value = (value >> 1);

		//DelayUs(50);

		//GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_SCK_PIN); /* toggle clock low */
		DISPLAY_SPI_DATA &= ~DISPLAY_SPI_SCK_PIN;

		//DelayUs(50);
	}
}

//Output LSB bit first
void SPIWrite_7bits(uint8_t value)
{
	uint8_t i = 0;
	uint8_t last_bit = 0XFF;

	while ( ++i <= 7 )
	{
		if((value & 0x01) != last_bit)
		{
			if ((value & 0x01) == 0x01) //check if LSB is high
			{
				//GPIO_setOutputHighOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_MOSI_PIN );
				DISPLAY_SPI_DATA |= DISPLAY_SPI_MOSI_PIN;
				last_bit = 1;
			}
			else /* if not, set to low */
			{

				//GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_MOSI_PIN );
				DISPLAY_SPI_DATA &= ~DISPLAY_SPI_MOSI_PIN;
				last_bit = 0;
			}
		}

		//DelayUs(10);

		//GPIO_setOutputHighOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_SCK_PIN);/* toggle clock high */
		DISPLAY_SPI_DATA |= DISPLAY_SPI_SCK_PIN;
		value = (value >> 1);

		//DelayUs(50);

		//GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_SCK_PIN); /* toggle clock low */
		DISPLAY_SPI_DATA &= ~DISPLAY_SPI_SCK_PIN;

		//DelayUs(50);
	}
}

void SPIWriteRLEByte(uint8_t value)
{
	uint8_t i = 0,RLE_pixel_value,RLE_repeat_count;
	uint8_t last_bit = 0xFF;

	if((value & 0x40) == 0x40)//check pixel value
		RLE_pixel_value = 0;//pixels are active low
	else
		RLE_pixel_value = 1; //pixels are active low

	RLE_repeat_count = (value & 0x3F);

	while ( ++i <= RLE_repeat_count )
	{
		if(RLE_pixel_value != last_bit)
		{
			if (RLE_pixel_value)
			{
				//GPIO_setOutputHighOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_MOSI_PIN );
				DISPLAY_SPI_DATA |= DISPLAY_SPI_MOSI_PIN;
				//last_bit = 1;
			}
			else
			{
				//GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_MOSI_PIN );
				DISPLAY_SPI_DATA &= ~DISPLAY_SPI_MOSI_PIN;
				//last_bit = 0;
			}
			last_bit = RLE_pixel_value;
		}

		//DelayUs(10);

		//GPIO_setOutputHighOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_SCK_PIN);/* toggle clock high */
		DISPLAY_SPI_DATA |= DISPLAY_SPI_SCK_PIN;

		//DelayUs(50);

		//GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_SCK_PIN); /* toggle clock low */
		DISPLAY_SPI_DATA &= ~DISPLAY_SPI_SCK_PIN;
		bits_count ++;
		//DelayUs(50);
	}
}

void SPIWriteByte(uint8_t value)
{
	EUSCI_A_CMSIS(EUSCI_B1_BASE)->TXBUF = value;
	while( EUSCI_B_SPI_isBusy(EUSCI_B1_BASE));
}
#if 0
//Output MSB bit first
void SPIWritedata(uint8_t value)
{
	uint8_t i = 0;

	while ( ++i <= 8 )
	{

		if ((value & 0x80) == 0x80)   /* check if MSB is high */

		{
			//GPIO_setOutputHighOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_MOSI_PIN );
			DISPLAY_SPI_DATA |= DISPLAY_SPI_MOSI_PIN;
		}
		else /* if not, set to low */
		{

			//GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_MOSI_PIN );
			DISPLAY_SPI_DATA &= ~DISPLAY_SPI_MOSI_PIN;
		}

		//DelayUs(10);

		//GPIO_setOutputHighOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_SCK_PIN);/* toggle clock high */
		DISPLAY_SPI_DATA |= DISPLAY_SPI_SCK_PIN;
		value = (value << 1);     /* shift 1 place for next bit */

		//DelayUs(50);

		//GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_SCK_PIN); /* toggle clock low */
		DISPLAY_SPI_DATA &= ~DISPLAY_SPI_SCK_PIN;

		//DelayUs(50);
	}
}
#endif
void Flash_SPI_Chip_Enable(void)
{
	SPI_enableModule(EUSCI_B1_BASE);        /* enable device */
	if(Flash_Operation == DATAKEY_SPI)
	{
		if(HARDWARE_REVISION == LNG_REV3)
		{
			GPIO_setOutputHighOnPin(DK_CNTRL_PORT, DK_CNTRL_PIN);			//Power to datakey
		}
		GPIO_setOutputLowOnPin(DataKey_SPI_IOPORT, DataKey_SPI_CS_PIN);	//enable datakey
		GPIO_setOutputLowOnPin(DISPLAY_SPI_CS_IOPORT, DISPLAY_SPI_CS_PIN);//disable display
		GPIO_setOutputHighOnPin(FLASH_SPI_IOPORT, FLASH_SPI_CS_PIN);//disable flash
	}
	else
	{
		Flash_Operation = FLASH_SPI;
		if(HARDWARE_REVISION == LNG_REV3)
		{
			GPIO_setOutputHighOnPin(DK_CNTRL_PORT, DK_CNTRL_PIN);			//Power to datakey
		}
		GPIO_setOutputLowOnPin(FLASH_SPI_IOPORT, FLASH_SPI_CS_PIN);			//enable flash
		GPIO_setOutputHighOnPin(DataKey_SPI_IOPORT, DataKey_SPI_CS_PIN);	//disable datakey
		GPIO_setOutputLowOnPin(DISPLAY_SPI_CS_IOPORT, DISPLAY_SPI_CS_PIN);	//disable display
	}
}

void Flash_SPI_Chip_Disable(void)
{
	if(Flash_Operation == DATAKEY_SPI)
	{
		GPIO_setOutputHighOnPin(DataKey_SPI_IOPORT, DataKey_SPI_CS_PIN);
		if(HARDWARE_REVISION == LNG_REV3)
		{
			GPIO_setOutputLowOnPin(DK_CNTRL_PORT, DK_CNTRL_PIN);			//Power to datakey
		}
	}
	else
	{
		Flash_Operation = FLASH_SPI;
		GPIO_setOutputHighOnPin(FLASH_SPI_IOPORT, FLASH_SPI_CS_PIN);
		if(HARDWARE_REVISION == LNG_REV3)
		{
			GPIO_setOutputLowOnPin(DK_CNTRL_PORT, DK_CNTRL_PIN);			//Power to datakey
		}
	}
	SPI_disableModule(EUSCI_B1_BASE);          /* disable device */
}

void Display_SPI_Chip_Enable(void)
{
	SPI_enableModule(EUSCI_B1_BASE);        /* enable device */
	if(HARDWARE_REVISION == LNG_REV3)
		GPIO_setOutputHighOnPin(DK_CNTRL_PORT, DK_CNTRL_PIN);			//Power to datakey

	GPIO_setOutputHighOnPin(DISPLAY_SPI_CS_IOPORT, DISPLAY_SPI_CS_PIN);
	GPIO_setOutputHighOnPin(DataKey_SPI_IOPORT, DataKey_SPI_CS_PIN);	//disable datakey
	GPIO_setOutputHighOnPin(FLASH_SPI_IOPORT, FLASH_SPI_CS_PIN);//disable flash
	DelayUs(50);
}

void Display_SPI_Chip_Disable(void)
{
	GPIO_setOutputLowOnPin(DISPLAY_SPI_CS_IOPORT, DISPLAY_SPI_CS_PIN);
	if(HARDWARE_REVISION == LNG_REV3)
		GPIO_setOutputLowOnPin(DK_CNTRL_PORT, DK_CNTRL_PIN);
	DelayUs(1);
	SPI_disableModule(EUSCI_B1_BASE);          /* disable device */
}


void InitspiMasterConfig()
{
	eUSCI_SPI_MasterConfig 				LoRa_spiMasterConfig;

	LoRa_spiMasterConfig.selectClockSource=EUSCI_SPI_CLOCKSOURCE_SMCLK; // ACLK Clock Source
	LoRa_spiMasterConfig.clockSourceFrequency=3000000; // ACLK = LFXT = 32.768khz
	LoRa_spiMasterConfig.desiredSpiClock=1000000; // SPICLK = 500khz
	LoRa_spiMasterConfig.msbFirst=EUSCI_B_SPI_MSB_FIRST; // MSB First
	LoRa_spiMasterConfig.clockPhase=EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT; // Phase
	LoRa_spiMasterConfig.clockPolarity=EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW; // High polarity
	LoRa_spiMasterConfig.spiMode=EUSCI_B_SPI_3PIN; // 3Wire SPI Mode

	SPI_initMaster(EUSCI_B2_BASE, &LoRa_spiMasterConfig);
}

void LORA_SPI_Init()
{
	GPIO_setAsPeripheralModuleFunctionInputPin(LORA_SPI_IOPORT,LORA_SCK_PIN |
			LORA_MOSI_PIN | LORA_MISO_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsOutputPin(LORA_SPI_IOPORT,LORA_NSS_PIN);
    GPIO_setOutputHighOnPin(LORA_SPI_IOPORT,LORA_NSS_PIN);

	InitspiMasterConfig();

	SPI_enableModule(EUSCI_B2_BASE);
}
