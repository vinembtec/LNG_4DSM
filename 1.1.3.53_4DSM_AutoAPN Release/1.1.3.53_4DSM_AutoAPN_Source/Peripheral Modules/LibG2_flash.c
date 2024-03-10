

//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_flash.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_flash_api
//! @{
//
//*****************************************************************************
#include "LibG2_flash.h"

extern FlashFieActvnTblEntry   		flash_file_actvn_tbl[ FILE_TYPE_TOTAL_FILES ];
extern uint8_t                 		DLST_FLAG_CHECK;
extern uint8_t         				glDLST_FLAGS[10];
extern GPRSSystem      				glSystem_cfg;
extern uint8_t         				glDLST_month, glDLST_day, glDLST_hour;
extern uint16_t        				glDLST_year;
extern uint8_t          			OLTAck_Q_count,
                        			Coin_Q_count,
									BayStatus_Q_count,
									OLT_Q_count,
									Alarms_Q_count,
									SCT_Q_count,    //25-06-12:SCT BATCH
									/*Audit_Q_count,*/
									PAMBayStatus_Q_count,
									EVENTS_Q_COUNT;//,
									//EVENTS_Q_FULL;//,//not used in this program //vinay
									//CLR_BATCH_ON_QFULL;//21-11-11//not used in this program //vinay

/*extern uint8_t          			OLT1_overwrite,
                        			Alarms_overwrite,
									OLTAck_overwrite,
									BayStatus_overwrite,
									CT_overwrite;*/
//extern uint8_t 						queue_events;
//extern uint16_t						Total_Events_count;//not used in this program //vinay
extern volatile uint32_t			parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];
//extern uint16_t 					Current_Space_Id;//not suing in this program //vinay
extern CoinSpecsType_New 			Coin_Calib_Params[MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT];
extern uint8_t						gAutoCalibration, gSelfCalibration;
extern Self_Calibration_Struct		Self_Calibration[MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT];
extern uint8_t						HARDWARE_REVISION;
extern uint16_t						coin_type0;
extern uint16_t						coin_type1;
extern uint16_t						coin_type2;
extern uint16_t						coin_type3;
extern uint16_t						coin_type[20];
extern uint16_t						coin_type_invalid,temp_coin_type_invalid;
extern uint16_t						temp_coin_type0, temp_coin_type1, temp_coin_type2, temp_coin_type3;
extern uint16_t						temp_coin_type[20];
extern uint16_t             		glTotal_Valid_Coin_Count;
extern uint32_t        		        temp_total_coin_values;
extern uint32_t        		        total_coin_values,Last_Audit_Time,glTotal_SCT_TransAmount,glTotal_SCT_TransCount,Last_Audit_Time1;
//extern uint8_t						enable_fiji_10C_coin;//not used in this program //vinay
extern uint32_t						Tasks_Priority_Register;

volatile uint8_t   					gl_Sensitivity1=0 , No_Modem_Coin_Only = 0 , REV4_LED_CONFIG = 0, Front_Bi_Color = 0;
uint8_t 							DLST_CURRENT_SLOT;
QUEUE                   			comm_queue;
uint8_t 							flash_data_read_buffer[ FLASH_DATA_READ_BUF_SIZE ];

RTC_ParkingClk						RTC_ParkingTime;
uint8_t								Self_calibrated_after_last_cfg = FALSE;
uint8_t								Coin_cal_index = 0xFF, add_2_seconds = false;
uint16_t							Coin_cal_max = 0, Coin_cal_min = 0;

static uint32_t						address_to_write_RTC = 0;
static uint32_t						Last_RTC_Park_clock_sync[MSM_MAX_PARKING_BAYS] = { 0 };//in seconds
//uint32_t  							Max_parking_time=0;
//uint32_t  							RTC_Update = 0;

static void Flash_Read_Cont(uint8_t Op_Code,uint32_t startAddress, uint16_t numBytesToRead);
static void Flash_WREN();
static void Flash_EWSR();
static void Flash_EWSR();
static uint8_t  Flash_WREN_Check();
static uint8_t Flash_Read_Status_Register();

extern uint32_t	Last_Cash_Payment_RTC;
//extern void printSharp_Card(const uint8_t *byte, uint8_t row, uint8_t column, uint8_t bytes4row, uint16_t numofrows);  //not used //vinay
//extern const uint8_t 			fontGIF5[488];
extern uint8_t					Screen_Clear;
extern uint8_t 					cardread;
extern uint32_t 				KeyPress_TimeOut_RTC; //to avoid screen refresh for every min when keypressed
extern uint32_t 				coindrop_TimeOut_RTC;
uint8_t 						Opto_Coin_Disable = 0;

extern uint32_t						Last_CC_Payment_RTC;
/**************************************************************************/
//! Enable flash write
//! \param void
//! \return void
/**************************************************************************/
static void Flash_WREN()
{
	Flash_SPI_Chip_Enable();
	uint8_t WREN_buf[1];
	WREN_buf[0]=Write_Enable;	/* send Flash_WREN command */

	Flash_SPI_TX(WREN_buf,1);
	//DelayUs(10);
	DelayUs(10); //san01jan19

	Flash_SPI_Chip_Disable();
}
/**************************************************************************/
//! Enable flash write to status register
//! \param void
//! \return void
/**************************************************************************/
static void Flash_EWSR()
{
	uint8_t EWSR_buf[1];
	EWSR_buf[0]=Enable_Write_Status_Register;	/* enable writing to the status register */
	Flash_SPI_Chip_Enable();
	Flash_SPI_TX(EWSR_buf,1);
	DelayMs(5);
	Flash_SPI_Chip_Disable();
}

/**************************************************************************/
//! Write to status register
//! \param void
//! \return void
/**************************************************************************/
static void Flash_WRSR(uint8_t byte)
{
	uint8_t WRSR_buf[2];
	WRSR_buf[0]=Write_Status_Register;		/* enable writing to the status register */
	WRSR_buf[1]=byte;						/* data that will change the status of BPx or BPL (only bits 2,3,7 can be written) */
	Flash_SPI_Chip_Enable();
	Flash_SPI_TX(WRSR_buf,2);
	DelayMs(5);
	Flash_SPI_Chip_Disable();
}

/**************************************************************************/
//! Check if Write Enable bit is set, reads status register and returns the read value
//! \param void
//! \return uint8_t status_register
/**************************************************************************/
static uint8_t  Flash_WREN_Check()
{
	uint16_t count = 0;
	uint8_t  byte = 0;
	uint16_t check_count = 0;

	if(HARDWARE_REVISION == LNG_REV3)
	{
		check_count = 100;
	}
	else
		check_count = 5;

	byte = Flash_Read_Status_Register();

	while ( (byte != 0x02) && (++count <= check_count) )       // verify that WEL bit is set //
	{
		byte = Flash_Read_Status_Register();
	}
	DelayUs(10);
	return byte;
}

/**************************************************************************/
//! Release flash from Power down mode, used in REV4 Main board flash chip
//! \param void
//! \return 0
/**************************************************************************/
uint32_t Release_Power_Down()
{
	DelayMs(1);
	uint8_t Status_Register_buf[5];
	//Flash_Read_Unique_ID();
	Status_Register_buf[0] = 0xAB;//
	Status_Register_buf[1] = 0x00;
	Status_Register_buf[2] = 0x00;
	Status_Register_buf[3] = 0x00;
	Flash_SPI_Chip_Enable();
	Flash_SPI_TX(Status_Register_buf,4);

	DelayMs(1);
	//Flash_SR = Flash_SPI_RX();
	return 0;
}

/**************************************************************************/
//! Put flash in Power down mode, used in REV4 Main board flash chip
//! \param void
//! \return 0
/**************************************************************************/
uint32_t Flash_Power_Down()
{
	uint8_t Status_Register_buf[2];
	Status_Register_buf[0] = 0xB9;//
	Status_Register_buf[1] = 0x00;
	Flash_SPI_Chip_Enable();
	Flash_SPI_TX(Status_Register_buf,2);
	Flash_SPI_Chip_Disable();
	DelayMs(1);
	return 0;
}

/**************************************************************************/
//! Reset flash chip, used in REV4 Main board flash chip
//! \param void
//! \return 0
/**************************************************************************/
uint32_t flash_Reset()
{
	uint8_t Status_Register_buf[5];
	//Flash_Read_Unique_ID();

	Status_Register_buf[0] = 0x99;//Read_Status_Register;
	Status_Register_buf[1] = 0x00;
	Status_Register_buf[2] = 0x00;
	Status_Register_buf[3] = 0x00;
	Flash_SPI_Chip_Enable();
	Flash_SPI_TX(Status_Register_buf,4);
	//Flash_SR = Flash_SPI_RX();

	return 0;
}

/**************************************************************************/
//! Erases all security registers in flash, used in REV4 Main board flash chip
//! \param void
//! \return void
/**************************************************************************/
void Erase_Security_Registers(void)
{

	uint8_t Status_Register_buf[5];
	//Flash_Read_Unique_ID();

	Status_Register_buf[0] = 0x44;//Read_Status_Register;
	Status_Register_buf[1] = 0x00;
	Status_Register_buf[2] = 0x00;
	Status_Register_buf[3] = 0x00;
	Flash_SPI_Chip_Enable();
	Flash_SPI_TX(Status_Register_buf,4);
	//Flash_SR = Flash_SPI_RX();
}

/**************************************************************************/
//! Reads flash chip device ID and few other pins to decide the Comms and
//! Main board revisions
//! \param void
//! \return void
/**************************************************************************/
void Flash_Read_Device_ID()
{
	uint8_t Flash_Device_ID=0;
	uint8_t flash_temp_cmd[6];
	uint8_t NCB_count = 0,i=0,inputpinstatus = 0, gpio_state=0;



	flash_temp_cmd[0] = 0x90;
	flash_temp_cmd[1] = 0x00;
	flash_temp_cmd[2] = 0x00;
	flash_temp_cmd[3] = 0x00;
	flash_temp_cmd[4] = 0xEF;
	flash_temp_cmd[5] = 0x00;

	Flash_SPI_Chip_Enable();
	Flash_SPI_TX(flash_temp_cmd,6);

	Flash_Device_ID = Flash_SPI_RX();
	DelayMs(10);

	Flash_SPI_Chip_Disable();
	DelayMs(50);
	Debug_Output1(0,"Flash Device ID:%02x",Flash_Device_ID);

	if(Flash_Device_ID == 0x13)
	{
		HARDWARE_REVISION = LNG_REV4;
		Debug_TextOut(0, "LNG REVISION 4 Hardware");
	}
	else
	{
		HARDWARE_REVISION = LNG_REV3;
		Debug_TextOut(0, "LNG REVISION 3 Hardware");
	}
	//return Flash_SR;

	GPIO_setAsOutputPin(MDM_PORT, MDM_TXD_PIN);	//MDM_RXD  #to check No coms board
	GPIO_setAsInputPin(MDM_PORT, MDM_RXD_PIN);	//MDM_TXD

	for(i = 0; i < 8; i++)
	{
		if((i%2) == 0)
		{
			gpio_state = 0;
			GPIO_setOutputLowOnPin(MDM_PORT, MDM_TXD_PIN);
		}
		else
		{
			gpio_state = 1;
			GPIO_setOutputHighOnPin(MDM_PORT, MDM_TXD_PIN);
		}
		DelayMs(1);
		inputpinstatus = GPIO_getInputPinValue(MDM_PORT, MDM_RXD_PIN);
		DelayMs(100);
		if(inputpinstatus == gpio_state)
		{
			NCB_count +=1;
		}
/*		GPIO_setOutputLowOnPin(MDM_PORT, MDM_RXD_PIN);
		inputpinstatus = GPIO_getInputPinValue(MDM_PORT, MDM_TXD_PIN);
		inputpinstatus = 0;*/
		//DelayMs(100);
	}

	if(NCB_count > 6)
	{
		No_Modem_Coin_Only = true;
		REV4_LED_CONFIG = true;
		Debug_TextOut(0,"REV4 LED Only Board");
	}
	else
	{
		GPIO_setAsInputPin(COMMS_DETECT1_PORT, COMMS_DETECT1_PIN);	//this pin is used for COMMS_DETECT in REV4 COMMS Board

		if(GPIO_getInputPinValue(COMMS_DETECT1_PORT, COMMS_DETECT1_PIN) == 1)	//Pulled Up in REV4 Comms Board
		{
			REV4_LED_CONFIG = true;
			Debug_TextOut(0,"REV4 Comms Board");

			GPIO_setAsInputPinWithPullUpResistor(COMMS_DETECT1_PORT, COMMS_DETECT1_PIN);

			GPIO_setAsOutputPin(GPIO_PORT_P9, GPIO_PIN5);
			GPIO_setOutputHighOnPin(GPIO_PORT_P9, GPIO_PIN5);
		}
		else
		{
			REV4_LED_CONFIG = false;
			Debug_TextOut(0,"REV3 Comms Board");
		}
		No_Modem_Coin_Only = false;
	}

	//to identify front display board whether it has bicolor LEDs or unicolor. If PIN2.0 is pulled up then its Bicolor modified board//vinay
	{
		GPIO_setAsInputPin(FRONT_BICOLOR_DETECT_PORT, FRONT_BICOLOR_DETECT_PIN);	//this pin is used for FrontBiColor Detect

		if(GPIO_getInputPinValue(FRONT_BICOLOR_DETECT_PORT, FRONT_BICOLOR_DETECT_PIN) == 1)	//Pulled Up for front bicolor led board
		{
			Front_Bi_Color = true;
			Debug_TextOut(0,"Front BiColor LEDs Board");

			GPIO_setAsInputPinWithPullUpResistor(FRONT_BICOLOR_DETECT_PORT, FRONT_BICOLOR_DETECT_PIN);
		}
		else
		{
			Front_Bi_Color = false;
			Debug_TextOut(0,"Front UiColor LEDs Board");
		}
	}


}

/**************************************************************************/
//! Used in Rev4 Main Boards, Reads flash chip's unique ID and frames meter
//! serial number from that
//! \param pointer to the structure where output will be given
//! \return void
/**************************************************************************/
void Flash_Read_Unique_ID(UID *chip_add)
{
	//uint8_t Flash_Unique_ID[8] = { 0 };
	uint8_t Flash_UID_cmd[6] = { 0 };
	uint8_t SPI_RX_count = 0;

	Flash_UID_cmd[0] = 0x4B;
	Flash_UID_cmd[1] = 0x00;
	Flash_UID_cmd[2] = 0x00;
	Flash_UID_cmd[3] = 0x00;
	Flash_UID_cmd[4] = 0x00;
	Flash_UID_cmd[5] = 0x00;

	Flash_SPI_Chip_Enable();
	Flash_SPI_TX(Flash_UID_cmd,6);

	for (SPI_RX_count = 0; SPI_RX_count < 8; SPI_RX_count++)
	{
		Flash_SPI_TX(&Flash_UID_cmd[5],1);
		chip_add->data[SPI_RX_count] = Flash_SPI_RX();
	}

	Flash_SPI_Chip_Disable();
	DelayMs(50);
	Debug_Output2(0, "Flash UID: %02x %02x", chip_add->data[0], chip_add->data[1]);
	Debug_Output6(0,"%02x %02x %02x %02x %02x %02x", chip_add->data[2], chip_add->data[3],
			chip_add->data[4], chip_add->data[5], chip_add->data[6], chip_add->data[7]);
	//return Flash_SR;
}

/**************************************************************************/
//! Writes one byte to flash
//! \param address where data will be written
//! \param data byte to be written
//! \return void
/**************************************************************************/
static void Flash_Byte_Program(uint32_t Dst, uint8_t byte)
{
	uint8_t Byte_Program_buf[5];
	Flash_SPI_Chip_Enable();
	Byte_Program_buf[0] = Byte_Program;/* send Sector Erase command */
	Byte_Program_buf[1] = (uint8_t) (Dst >> 16);/* send 3 address bytes */
	Byte_Program_buf[2] = (uint8_t) (Dst >> 8);
	Byte_Program_buf[3] = (uint8_t) (Dst);
	Byte_Program_buf[4] = byte;

	Flash_SPI_TX(Byte_Program_buf,5);
	Flash_SPI_Chip_Disable();
}
#if 0
/**************************************************************************/
/*  Name        : Flash_Sector_Erase                                      */
/*  Parameters  : uint32_t                                                */
/*  Returns     : void                                                    */
/*  Function    : Erase particular flash sector(4kb) .                    */
/*------------------------------------------------------------------------*/
static void Flash_Sector_Erase(uint32_t Dst)
{
	uint8_t Sector_Erase_buf[4];

	Sector_Erase_buf[0] = Flash_4KByte_Sector_Erase;/* send Sector Erase command */
	Sector_Erase_buf[1] = (uint8_t) (Dst >> 16);		/* send 3 address bytes */
	Sector_Erase_buf[2] = (uint8_t) (Dst >> 8);
	Sector_Erase_buf[3] = (uint8_t) (Dst);

	Flash_SPI_Chip_Enable();

	Flash_TX(Sector_Erase_buf,4);

	DelayMs(50);

	Flash_SPI_Chip_Disable();         /* disable device */
}

/**************************************************************************/
/*  Name        : Flash_Block_Erase                                       */
/*  Parameters  : uint32_t                                                */
/*  Returns     : void                                                    */
/*  Function    :Erase flash block(32 kb).                                */
/*------------------------------------------------------------------------*/

static void Flash_Block_Erase(uint32_t Dst)
{
	uint8_t SBlock_Erase_buf[4];

	SBlock_Erase_buf[0] = Flash_32KByte_Block_Erase;			/* send Sector Erase command */
	SBlock_Erase_buf[1] = (uint8_t) (Dst >> 16);		/* send 3 address bytes */
	SBlock_Erase_buf[2] = (uint8_t) (Dst >> 8);
	SBlock_Erase_buf[3] = (uint8_t) (Dst);

	Flash_SPI_Chip_Enable();       /* enable device */

	Flash_TX(SBlock_Erase_buf,4);
	DelayMs(50);
	Flash_SPI_Chip_Disable();        /* disable device */
}
#endif

/**************************************************************************/
//! Reads Status register and returns the read value
//! \param void
//! \return status register
/**************************************************************************/
static uint8_t Flash_Read_Status_Register()
{
	uint8_t Flash_SR=0;
	uint8_t Status_Register_buf[2];

	Status_Register_buf[0] = Read_Status_Register;
	Status_Register_buf[1] = 0x00;
	Flash_SPI_Chip_Enable();
	Flash_SPI_TX(Status_Register_buf,2);

	Flash_SR = Flash_SPI_RX();
	DelayMs(10);

	Flash_SPI_Chip_Disable();
	DelayMs(1);
	return Flash_SR;
}

#if 0
static uint8_t Flash_Read_ID()
{
	Flash_SPI_Chip_Enable();

	Flash_Read_Cont(Read_Device_ID,0x000000,2);

	Flash_SPI_Chip_Disable();
	return 0;
}
#endif

/**************************************************************************/
//! Reads bytes from continuous memory addresses in flash
//! \param Op_Code
//! \param uint32_t startAddress
//! \param uint32_t number of Bytes To Read
//! \return void
/**************************************************************************/
static void Flash_Read_Cont(uint8_t Op_Code,uint32_t startAddress, uint16_t numBytesToRead)
{
	//uint16_t i = 0;
	uint16_t SPI_RX_count=0;
	uint8_t Read_Cont_buf[5];

	Read_Cont_buf[0] = Op_Code;
	Read_Cont_buf[1] = (uint8_t) (startAddress >> 16);
	Read_Cont_buf[2] = (uint8_t) (startAddress >> 8);
	Read_Cont_buf[3] = (uint8_t) (startAddress);
	Read_Cont_buf[4] = 0x00;
	Flash_SPI_Chip_Enable();
	//  numBytesToRead = min ( sizeof(flash_data_read_buffer), numBytesToRead );

	memset( flash_data_read_buffer, 0, numBytesToRead );

	Flash_SPI_TX(Read_Cont_buf,4);

	for (SPI_RX_count = 0; SPI_RX_count < numBytesToRead; SPI_RX_count++)
	{
		Flash_SPI_TX(&Read_Cont_buf[4],1);

		flash_data_read_buffer[SPI_RX_count] = Flash_SPI_RX();
	}
	Flash_SPI_Chip_Disable();
}

/**************************************************************************/
//! Erase particular sector or block of flash .
//! \param uint32_t address - Sector or Block address to be erased
//! \param batch_erase_flag
//! - \b MULTIPLE_BLOCK_ERASE_FLAG
//! - \b SINGLE_SECTOR_ERASE
//! - \b TWO_SECTOR_ERASE
//! \param uint8_t keep_intr_disabled - flag to decide whether to keep other
//! interrupts disabled while returning from this function
//! \return flash erase result
//! - \b Success
//! - \b Error
/**************************************************************************/
uint8_t Flash_Batch_Erase( uint32_t address, uint8_t batch_erase_flag, uint8_t keep_intr_disabled )
{
	if(keep_intr_disabled == false)
		__disable_interrupt();
	//Debug_TextOut( 0, "3" );
	init_flash_SPI_module_GPIO();

	uint8_t i=0, result = 0;
	uint8_t Batch_Erase_buf[4];

	if(HARDWARE_REVISION == LNG_REV4)
	{
		Release_Power_Down();
	}

	result = Flash_Read_Status_Register();

	if(result!=0x02)
	{
		if(HARDWARE_REVISION == LNG_REV4)
		{
			Release_Power_Down();
		}
		Flash_WREN();
		Flash_EWSR();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		if(HARDWARE_REVISION == LNG_REV4)
		{
			DelayMs(1);
			result = Flash_Read_Status_Register();
			if(result!=0x02)
			{
				DelayMs(14);
				Flash_WREN();
				result = Flash_Read_Status_Register();
			}
		}
		else
		{
			result = Flash_WREN_Check();//2612
		}
	}

	if(batch_erase_flag==1)//two sector erase rpg/ccg/config//25-05-12 or batch transactions//01-04-2013:DPLIBB-476
	{
		for(i=0;i<2;i++)
		{
			if(result==0x02)
			{
				//Flash_Sector_Erase(address);
				Batch_Erase_buf[0] = Flash_4KByte_Sector_Erase;/* send Sector Erase command */
				Batch_Erase_buf[1] = (uint8_t) (address >> 16);		/* send 3 address bytes */
				Batch_Erase_buf[2] = (uint8_t) (address >> 8);
				Batch_Erase_buf[3] = (uint8_t) (address);
				Flash_SPI_Chip_Enable();       /* enable device */
				Flash_SPI_TX(Batch_Erase_buf,4);
				//__enable_interrupt();
				//if(HARDWARE_REVISION == LNG_REV3)
				DelayMs(300);//2809
				//__disable_interrupt();
				Flash_SPI_Chip_Disable();       /* disable device */
				//if(HARDWARE_REVISION == LNG_REV4)
					DelayMs(50);
				Debug_Output1( 0, "Flash_Batch_Erase 2 sectors: %ld",  address);
			}
			else
			{
				Debug_TextOut( 0, "SPI error --- Flash_Batch_Erase - A" );
				if(keep_intr_disabled == false)
					__enable_interrupt();
				return 0;
			}
			address=(uint32_t)((uint32_t)(address)+(uint32_t)(4096));
			result=Flash_Read_Status_Register();

			if(result!=0x02)
			{
				if(HARDWARE_REVISION == LNG_REV4)
				{
					Release_Power_Down();
				}
				Flash_WREN();
				Flash_EWSR();
				Flash_WRSR(0x02);//enable write to status register
				Flash_WREN();
				if(HARDWARE_REVISION == LNG_REV4)
				{
					DelayMs(1);
					result = Flash_Read_Status_Register();
					if(result!=0x02)
					{
						DelayMs(14);
						Flash_WREN();
						result = Flash_Read_Status_Register();
					}
				}
				else
				{
					result = Flash_WREN_Check();//2612
				}
			}
		}
	}
	else if(batch_erase_flag==MULTIPLE_BLOCK_ERASE_FLAG)//mpb code// erase 5 blocks  each of 32 kb//25-05-12
	{
		for(i=0;i<8;i++)
		{
			if(result==0x02)
			{
				//Flash_Block_Erase(erase_adress);
				Batch_Erase_buf[0] = Flash_32KByte_Block_Erase;			/* send Sector Erase command */
				Batch_Erase_buf[1] = (uint8_t) (address >> 16);		/* send 3 address bytes */
				Batch_Erase_buf[2] = (uint8_t) (address >> 8);
				Batch_Erase_buf[3] = (uint8_t) (address);
				Flash_SPI_Chip_Enable();       /* enable device */
				Flash_SPI_TX(Batch_Erase_buf,4);
				//if(HARDWARE_REVISION == LNG_REV3)
				DelayMs(800);
				Flash_SPI_Chip_Disable();       /* disable device */
				//if(HARDWARE_REVISION == LNG_REV4)
					DelayMs(50);
				//Debug_Output1( 2, "Flash_Batch_Erase 8 blocks: %ld",  address);

				//Debug_Output1( 2, "Flash_Batch_Erased - B  i = %d ",i );
			}
			else
			{
				Debug_Output1( 0, "SPI error --- Flash_Batch_Erase - B  i = %d ",i );
				if(keep_intr_disabled == false)
					__enable_interrupt();
				return 0;
			}

			address=(uint32_t)((uint32_t)(address)+(uint32_t)(32768));
			result=Flash_Read_Status_Register();

			if(result!=0x02)
			{
				if(HARDWARE_REVISION == LNG_REV4)
				{
					Release_Power_Down();
				}
				Flash_WREN();
				Flash_EWSR();
				Flash_WRSR(0x02);//enable write to status register
				Flash_WREN();
				if(HARDWARE_REVISION == LNG_REV4)
				{
					DelayMs(1);
					result = Flash_Read_Status_Register();
					if(result!=0x02)
					{
						DelayMs(14);
						Flash_WREN();
						result = Flash_Read_Status_Register();
					}
				}
				else
				{
					result = Flash_WREN_Check();//2612
				}
			}
		}
	}
	else if(batch_erase_flag==SINGLE_SECTOR_ERASE)
	{
		if(result==0x02)
		{
			//Flash_Sector_Erase(address);
			Batch_Erase_buf[0] = Flash_4KByte_Sector_Erase;/* send Sector Erase command */
			Batch_Erase_buf[1] = (uint8_t) (address >> 16);		/* send 3 address bytes */
			Batch_Erase_buf[2] = (uint8_t) (address >> 8);
			Batch_Erase_buf[3] = (uint8_t) (address);
			Flash_SPI_Chip_Enable();       /* enable device */
			Flash_SPI_TX(Batch_Erase_buf,4);
			//__enable_interrupt();
			//if(HARDWARE_REVISION == LNG_REV3)
			DelayMs(300);//2809
			//__disable_interrupt();
			Flash_SPI_Chip_Disable();       /* disable device */
			//if(HARDWARE_REVISION == LNG_REV4)
				DelayMs(50);
			//Debug_Output1( 2, "Flash_Batch_Erase 1 sector: %ld",  address);
		}
		else
		{
			Debug_TextOut( 0, "SPI error --- Flash_Batch_Erase - C" );
			if(keep_intr_disabled == false)
				__enable_interrupt();
			return 0;
		}

		result=Flash_Read_Status_Register();

		if(result!=0x02)
		{
			if(HARDWARE_REVISION == LNG_REV4)
			{
				Release_Power_Down();
			}
			Flash_WREN();
			Flash_EWSR();
			Flash_WRSR(0x02);//enable write to status register
			Flash_WREN();
			if(HARDWARE_REVISION == LNG_REV4)
			{
				DelayMs(1);
				result = Flash_Read_Status_Register();
				if(result!=0x02)
				{
					DelayMs(14);
					Flash_WREN();
					result = Flash_Read_Status_Register();
				}
			}
			else
			{
				result = Flash_WREN_Check();//2612
			}
		}

	}
	if(keep_intr_disabled == false)
		__enable_interrupt();
	return 1;
}

/**************************************************************************/
//! Reads data from flash and gives output in  flash_data_read_buffer
//! \param uint32_t address - start address from where data to be read
//! \param uint16_t number of bytes to read
//! \return uint32_t Next address of memory after the Read data
/**************************************************************************/
uint32_t  Flash_Read(uint32_t flash_start_address, uint16_t num_bytes)
{
	__disable_interrupt();
	//Debug_TextOut( 0, "2" );
	init_flash_SPI_module_GPIO();
	//Flash_SPI_Chip_Enable();
	if(HARDWARE_REVISION == LNG_REV4)
	{
		//Release_Power_Down();
	}

	Flash_Read_Cont(Read_Memory, flash_start_address, num_bytes);

	//   Flash_SPI_Chip_Disable();
	if(HARDWARE_REVISION == LNG_REV4)
	{
		Flash_Power_Down();
	}
	__enable_interrupt();
	return (flash_start_address + num_bytes);
}

/**************************************************************************/
//! Erase particular sector or block of flash and write. Reads back and verify after write
//! \param pointer to the data buffer to be written
//! \param length of data to be written
//! \param uint32_t address - address where data to be written
//! \param batch_erase_flag
//! - \b NO_ERASE
//! - \b MULTIPLE_BLOCK_ERASE_FLAG
//! - \b SINGLE_SECTOR_ERASE
//! - \b TWO_SECTOR_ERASE
//! \return uint32_t Next address of memory after the written data
/**************************************************************************/
uint32_t Flash_Write( const void * src_data_ptr, int data_lng, uint32_t flash_start_address, char batch_erase_flag  )
{
	__disable_interrupt();
	//Debug_TextOut( 0, "1" );
	init_flash_SPI_module_GPIO();

	uint8_t  result=0;//,k=0;
	int      i=0,read_offset=0,no_of_read_chunks,l=0;
	const uint8_t* data_ptr = (const uint8_t *) src_data_ptr;
	uint32_t lflash_start_address;
	uint16_t read_bytes_left=0,read_Chunk_size=0;
	lflash_start_address=flash_start_address;
	if(HARDWARE_REVISION == LNG_REV4)
	{
		Release_Power_Down();
		DelayMs(2);
		Flash_WREN();
		Flash_EWSR();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		result = Flash_WREN_Check();//261
		DelayMs(10);
	}
	if ( batch_erase_flag != false )
	{
		result=Flash_Batch_Erase( flash_start_address, batch_erase_flag, true );//25-05-12
		Debug_TextOut(0,"Flash Erased for Write");
		if(result!=1)
		{
			__enable_interrupt();
			return 0;
		}
	}
	result=Flash_Read_Status_Register();
	if(result!=0x02)
	{
		if(HARDWARE_REVISION == LNG_REV4)
		{
			Release_Power_Down();
		}
		DelayMs(2);
		Flash_WREN();
		Flash_EWSR();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		if(HARDWARE_REVISION == LNG_REV4)
		{
			DelayMs(1);
			if(result != 0x02)
			{
				//Debug_Output1(0,"Flash_Read_Status_Register Result TD! = %d",result);//san01jan19
				DelayMs(4);
				Flash_WREN();
				DelayMs(1);// missing from v48-50
				result=Flash_Read_Status_Register();
				//Debug_TextOut( 0, "TD1" );
				DelayMs(10);
			}
		}
		else
		{
			result = Flash_WREN_Check();//261
		}
	}
	//Flash_SPI_Chip_Enable();

	if(HARDWARE_REVISION == LNG_REV4)
	{
		result=Flash_Read_Status_Register();
		//DelayMs(10);
		DelayMs(10);//san01jan19
	}
	if(result==0x02)
	{
		for ( i=0;  i < data_lng;  ++i)
		{
			Flash_Byte_Program(flash_start_address++, data_ptr[i]);
			DelayUs(20);//2809
			Flash_WREN();
		}
		//12-02-2013
		if(data_lng > sizeof(flash_data_read_buffer))
			no_of_read_chunks = (uint16_t) (data_lng/sizeof(flash_data_read_buffer))+1;
		else
			no_of_read_chunks = 1;
		read_Chunk_size=sizeof(flash_data_read_buffer);
		for(l=0;l<no_of_read_chunks;l++)
		{
			//k=0;
			read_bytes_left = data_lng - read_offset;

			if(read_bytes_left < sizeof(flash_data_read_buffer))
			{
				read_Chunk_size = (uint16_t) read_bytes_left;
			}
			Flash_Read_Cont(0x03,(lflash_start_address+read_offset), read_Chunk_size);

			for(i=0;i<read_Chunk_size;i++)
			{
				if((data_ptr[i+read_offset]!=flash_data_read_buffer[i]))
				{
					Debug_TextOut( 0, "SPI error --- Flash_Write/Read_back" );
					__enable_interrupt();
					return 0;
				}
			}
			read_offset +=read_Chunk_size;
		}
		//12-02-2013
	}
	else
	{
		Debug_TextOut( 0, "SPI error --- Flash_Write" );
		//Debug_Output1(0,"Flash_Read_Status_Register Result = %X",result);//san01jan19
		__enable_interrupt();
		return 0;
	}

	Flash_SPI_Chip_Disable();

	__enable_interrupt();

	return flash_start_address;
}

/**************************************************************************/
//! Reads the Batch flag sector. This sector contains all the run time flags
//! which need to be retained, this keeps updating frequently
//! \param void
//! \return void
/**************************************************************************/
void Flash_Batch_Flag_Read()
{
	uint32_t   flag_read_address = AD_Batch_Flag_r;
	uint16_t   lDLST_year = 0, index=0;
	uint8_t    slot = 0, lDLST_month = 0;
	uint8_t    *ptr;
	uint8_t    i = 0;

	flag_read_address = Flash_Read(flag_read_address, FLASH_DATA_READ_BUF_SIZE);

	memcpy(comm_queue.OLT_batch_flag, flash_data_read_buffer, NUM_QUEUE_BATCH_FLAGS);
	OLT_Q_count = comm_queue.OLT_batch_flag[NUM_QUEUE_BATCH_FLAGS-1];
	index +=NUM_QUEUE_BATCH_FLAGS;

	memcpy(comm_queue.coin_batch_flag, (flash_data_read_buffer+index), NUM_QUEUE_BATCH_FLAGS);
	Coin_Q_count = comm_queue.coin_batch_flag[NUM_QUEUE_BATCH_FLAGS-1];
	index +=NUM_QUEUE_BATCH_FLAGS;

	memcpy(comm_queue.OLTAck_batch_flag, (flash_data_read_buffer+index), NUM_QUEUE_BATCH_FLAGS);
	OLTAck_Q_count = comm_queue.OLTAck_batch_flag[NUM_QUEUE_BATCH_FLAGS-1];
	index +=NUM_QUEUE_BATCH_FLAGS;

	memcpy(comm_queue.BayStatus_batch_flag, (flash_data_read_buffer+index), NUM_QUEUE_BATCH_FLAGS);
	BayStatus_Q_count = comm_queue.BayStatus_batch_flag[NUM_QUEUE_BATCH_FLAGS-1];
	index +=NUM_QUEUE_BATCH_FLAGS;

	memcpy(comm_queue.Alarms_batch_flag, (flash_data_read_buffer+index), NUM_QUEUE_BATCH_FLAGS);
	Alarms_Q_count = comm_queue.Alarms_batch_flag[NUM_QUEUE_BATCH_FLAGS-1];
	index +=NUM_QUEUE_BATCH_FLAGS;

	memcpy(&flash_file_actvn_tbl,(flash_data_read_buffer+index),(sizeof(FlashFieActvnTblEntry)*FILE_TYPE_TOTAL_FILES));
	index +=(sizeof(FlashFieActvnTblEntry)*FILE_TYPE_TOTAL_FILES);

	//17-05-12:DLST
	if(DLST_FLAG_CHECK==1)
	{
		memset(glDLST_FLAGS,0,sizeof(glDLST_FLAGS));
		memcpy(glDLST_FLAGS,(flash_data_read_buffer+index),sizeof(glDLST_FLAGS));
		ptr= &glSystem_cfg.DLST_Slot1[0];
		for(slot=0;slot<10;slot++)
		{
			if(glDLST_FLAGS[slot]==0)//08-10-12
			{
				ptr= &glSystem_cfg.DLST_Slot1[0];
				ptr += (4 * slot);
				lDLST_year = *ptr;
				lDLST_year = lDLST_year+2000;
				ptr++;
				lDLST_month = *ptr;
				ptr++;
				if((lDLST_year==RTCYEAR)&&(lDLST_month==RTCMON))
				{
					break;
				}
			}
		}
		DLST_CURRENT_SLOT=slot;
		if(DLST_CURRENT_SLOT==10)
		{
			glDLST_month= 12;
			glDLST_day  = 31;
			glDLST_hour = 23;
			glDLST_year = 99;
		}
		else
		{
			glDLST_year=lDLST_year;
			glDLST_month=lDLST_month;
			glDLST_day=*ptr;
			ptr++;
			glDLST_hour=*ptr;
		}
	}
	else
	{
		memset(glDLST_FLAGS,0,sizeof(glDLST_FLAGS));
		memcpy(glDLST_FLAGS,(flash_data_read_buffer+index),sizeof(glDLST_FLAGS));
	}
	index +=sizeof(glDLST_FLAGS);
	//17-05-12:DLST

	//25-06-12:SCT Batch
	memcpy(comm_queue.SCT_batch_flag, (flash_data_read_buffer+index), NUM_QUEUE_BATCH_FLAGS);
	SCT_Q_count = comm_queue.SCT_batch_flag[NUM_QUEUE_BATCH_FLAGS-1];
	index +=NUM_QUEUE_BATCH_FLAGS;
	//25-06-12:SCT Batch

	//14-03-2014:hardware sensor code merge
	memcpy(comm_queue.EVENTS_batch_flag, (flash_data_read_buffer+index), NUM_QUEUE_BATCH_FLAGS);//events batch
	EVENTS_Q_COUNT = comm_queue.EVENTS_batch_flag[NUM_QUEUE_BATCH_FLAGS-1];
	index +=NUM_QUEUE_BATCH_FLAGS;

	memcpy(comm_queue.PAMBayStatus_batch_flag, (flash_data_read_buffer+index), NUM_QUEUE_BATCH_FLAGS);
	PAMBayStatus_Q_count = comm_queue.PAMBayStatus_batch_flag[NUM_QUEUE_BATCH_FLAGS-1];
	index +=NUM_QUEUE_BATCH_FLAGS;

//********************Coin audit R ******************

	memcpy(&coin_type0, (flash_data_read_buffer+index), sizeof(coin_type0));
	index += sizeof(coin_type0);
	if(coin_type0 == 0xFFFF)
		coin_type0 = 0;

	memcpy(&coin_type1, (flash_data_read_buffer+index), sizeof(coin_type1));
	index += sizeof(coin_type1);
	if(coin_type1 == 0xFFFF)
		coin_type1 = 0;

	memcpy(&coin_type2, (flash_data_read_buffer+index), sizeof(coin_type2));
	index += sizeof(coin_type2);
	if(coin_type2 == 0xFFFF)
		coin_type2 = 0;

	memcpy(&coin_type3, (flash_data_read_buffer+index), sizeof(coin_type3));
	index += sizeof(coin_type3);
	if(coin_type3 == 0xFFFF)
		coin_type3 = 0;

	memcpy(&coin_type_invalid, (flash_data_read_buffer+index), sizeof(coin_type_invalid));
	index += sizeof(coin_type_invalid);
	if(coin_type_invalid == 0xFFFF)
		coin_type_invalid = 0;

	memcpy(&glTotal_SCT_TransCount, (flash_data_read_buffer+index), sizeof(glTotal_SCT_TransCount));
	index += sizeof(glTotal_SCT_TransCount);
	if((glTotal_SCT_TransCount > 999)||(glTotal_SCT_TransCount <= 0))//(glTotal_SCT_TransCount == 0xFFFF)
		glTotal_SCT_TransCount = 0;

	memcpy(&glTotal_SCT_TransAmount, (flash_data_read_buffer+index), sizeof(glTotal_SCT_TransAmount));
	index += sizeof(glTotal_SCT_TransAmount);
	if(glTotal_SCT_TransAmount == 0xFFFFFFFF)
		glTotal_SCT_TransAmount = 0;

	//Debug_Output2(0, "Flash read SCT_TransCount:%ld, SCT_TransAmount:%ld", glTotal_SCT_TransCount, glTotal_SCT_TransAmount);

//	memcpy(&Last_Audit_Time, (flash_data_read_buffer+index), sizeof(Last_Audit_Time));
	memcpy(&Last_Audit_Time1, (flash_data_read_buffer+index), sizeof(Last_Audit_Time1)); //for audit card timestamp//Vinay 
//	Debug_Output1(0, "Last_Audit_Time: %ld", Last_Audit_Time);
//	Debug_Output1(0, "flash_data_read_buffer: %ld", flash_data_read_buffer);
//	Debug_Output1(0, "index: %ld", index);
	index += sizeof(Last_Audit_Time);
//	if(Last_Audit_Time == 0xFFFFFFFF)
//		Last_Audit_Time = 0;
//	Debug_Output1(0, "Read Last_Audit_Time1: %ld", Last_Audit_Time1);
//	Debug_Output1(0, "Read flash_data_read_buffer: %ld", flash_data_read_buffer);
//	Debug_Output1(0, "Read index: %ld", index);
	if(Last_Audit_Time1 == 0xFFFFFFFF)
	{
		Last_Audit_Time1 = 0;
		//Debug_TextOut(0,"Came here to clear Last_Audit_Time");
	}

	memcpy(&total_coin_values, (flash_data_read_buffer+index), sizeof(total_coin_values));
	index += sizeof(total_coin_values);
	if(total_coin_values == 0xFFFFFFFF)
		total_coin_values = 0;

	memcpy(&Self_calibrated_after_last_cfg, (flash_data_read_buffer+index), sizeof(Self_calibrated_after_last_cfg));
	index += sizeof(Self_calibrated_after_last_cfg);
	if(Self_calibrated_after_last_cfg == 0xFF)
		Self_calibrated_after_last_cfg = FALSE;

	for(i=0;i<20;i++)
	{
		memcpy(&coin_type[i], (flash_data_read_buffer+index), sizeof(coin_type[i]));
				index += sizeof(coin_type[i]);
		if(coin_type[i] == 0xFFFF)
					coin_type[i] = 0;
	}

//*********************************************************
/*	memcpy(&enable_fiji_10C_coin, (flash_data_read_buffer+index), sizeof(enable_fiji_10C_coin));
	index += sizeof(enable_fiji_10C_coin);
	if(enable_fiji_10C_coin == 0xFF)
		enable_fiji_10C_coin = TRUE;*/

//	index=0;
	/*memcpy(&Max_parking_time,(flash_data_read_buffer+index),MAX_PARKING_TIME_LENGTH);
	index +=MAX_PARKING_TIME_LENGTH;

	memcpy(&RTC_Update,(flash_data_read_buffer+index),RTC_UPDATE_LENGTH );
	index +=RTC_UPDATE_LENGTH;*/

	index=0;

	return;
}

/**************************************************************************/
//! Writes the Batch flag sector. This sector contains all the run time flags
//! which need to be retained, this keeps updating frequently
//! \param void
//! \return void
/**************************************************************************/
void Flash_Batch_Flag_Write()
{
	uint32_t   flag_write_address = AD_Batch_Flag_w;
	uint16_t   index_W=0;
	uint8_t    slot = 0;
	uint8_t    *ptr;
	uint8_t    i = 0;

	memset(flash_data_read_buffer,0xff,FLASH_DATA_READ_BUF_SIZE);

	comm_queue.OLT_batch_flag[NUM_QUEUE_BATCH_FLAGS-1]       	= OLT_Q_count;
	comm_queue.coin_batch_flag[NUM_QUEUE_BATCH_FLAGS-1]      	= Coin_Q_count ;
	comm_queue.OLTAck_batch_flag[NUM_QUEUE_BATCH_FLAGS-1]    	= OLTAck_Q_count ;
	comm_queue.Alarms_batch_flag[NUM_QUEUE_BATCH_FLAGS-1]    	= Alarms_Q_count;
	comm_queue.BayStatus_batch_flag[NUM_QUEUE_BATCH_FLAGS-1] 	= BayStatus_Q_count ;
	comm_queue.SCT_batch_flag[NUM_QUEUE_BATCH_FLAGS-1]       	= SCT_Q_count;//25-06-12:SCT Batch
	comm_queue.EVENTS_batch_flag[NUM_QUEUE_BATCH_FLAGS-1]    	= EVENTS_Q_COUNT;//17-03-2014:hardware sensor code merge
	comm_queue.PAMBayStatus_batch_flag[NUM_QUEUE_BATCH_FLAGS-1] = PAMBayStatus_Q_count;
	//	RTCCTL0=0X01;

	memcpy((flash_data_read_buffer+index_W),comm_queue.OLT_batch_flag,NUM_QUEUE_BATCH_FLAGS);
	index_W +=NUM_QUEUE_BATCH_FLAGS;

	memcpy((flash_data_read_buffer+index_W),comm_queue.coin_batch_flag,NUM_QUEUE_BATCH_FLAGS);
	index_W +=NUM_QUEUE_BATCH_FLAGS;

	memcpy((flash_data_read_buffer+index_W),comm_queue.OLTAck_batch_flag,NUM_QUEUE_BATCH_FLAGS);
	index_W +=NUM_QUEUE_BATCH_FLAGS;

	memcpy((flash_data_read_buffer+index_W),comm_queue.BayStatus_batch_flag,NUM_QUEUE_BATCH_FLAGS);
	index_W +=NUM_QUEUE_BATCH_FLAGS;

	memcpy((flash_data_read_buffer+index_W),comm_queue.Alarms_batch_flag,NUM_QUEUE_BATCH_FLAGS);
	index_W +=NUM_QUEUE_BATCH_FLAGS;

	memcpy((flash_data_read_buffer+index_W),flash_file_actvn_tbl,(sizeof(FlashFieActvnTblEntry)*FILE_TYPE_TOTAL_FILES));
	index_W +=(sizeof(FlashFieActvnTblEntry)*FILE_TYPE_TOTAL_FILES);
	//17-05-12:DLST
	if (DLST_FLAG_CHECK == 1)
	{
		ptr= &glSystem_cfg.DLST_Slot1[0];
		for(slot=0;slot<10;slot++)
		{
			if(glDLST_FLAGS[slot]==0)
			{
				ptr=ptr+(4*slot);
				glDLST_year=*ptr;
				glDLST_year=glDLST_year+2000;
				ptr++;
				glDLST_month=*ptr;
				ptr++;
				if((glDLST_year==RTCYEAR)&&(glDLST_month<=RTCMON))
				{
					break;
				}
				if((glDLST_year==RTCYEAR)&&(glDLST_month>=RTCMON))
				{
					break;
				}
			}
		}

		if (DLST_CURRENT_SLOT != 10)
		{
			glDLST_FLAGS[DLST_CURRENT_SLOT] = 1;
			memcpy((flash_data_read_buffer+index_W),glDLST_FLAGS,sizeof(glDLST_FLAGS));
			index_W +=sizeof(glDLST_FLAGS);
		}
		else
		{
			index_W +=sizeof(glDLST_FLAGS);
		}
	}
	else
	{
		memcpy((flash_data_read_buffer+index_W),glDLST_FLAGS,sizeof(glDLST_FLAGS));
		index_W +=sizeof(glDLST_FLAGS);
	}

	//17-05-12:DLST

	//25-06-12:SCT Batch
	memcpy((flash_data_read_buffer+index_W),comm_queue.SCT_batch_flag,NUM_QUEUE_BATCH_FLAGS);
	index_W +=NUM_QUEUE_BATCH_FLAGS;
	//25-06-12:SCT Batch

	memcpy((flash_data_read_buffer+index_W),comm_queue.EVENTS_batch_flag,NUM_QUEUE_BATCH_FLAGS);
	index_W +=NUM_QUEUE_BATCH_FLAGS;

	memcpy((flash_data_read_buffer+index_W),comm_queue.PAMBayStatus_batch_flag,NUM_QUEUE_BATCH_FLAGS);
	index_W +=NUM_QUEUE_BATCH_FLAGS;
//****************************Coin data W********************
	memcpy((flash_data_read_buffer+index_W),&coin_type0, sizeof(coin_type0));
	index_W += sizeof(coin_type0);

	memcpy((flash_data_read_buffer+index_W),&coin_type1, sizeof(coin_type1));
	index_W += sizeof(coin_type1);

	memcpy((flash_data_read_buffer+index_W),&coin_type2, sizeof(coin_type2));
	index_W += sizeof(coin_type2);

	memcpy((flash_data_read_buffer+index_W),&coin_type3, sizeof(coin_type3));
	index_W += sizeof(coin_type3);

	memcpy((flash_data_read_buffer+index_W),&coin_type_invalid, sizeof(coin_type_invalid));
	index_W += sizeof(coin_type_invalid);

	memcpy((flash_data_read_buffer+index_W),&glTotal_SCT_TransCount, sizeof(glTotal_SCT_TransCount));
	index_W += sizeof(glTotal_SCT_TransCount);

	memcpy((flash_data_read_buffer+index_W),&glTotal_SCT_TransAmount, sizeof(glTotal_SCT_TransAmount));
	index_W += sizeof(glTotal_SCT_TransAmount);

//	memcpy((flash_data_read_buffer+index_W),&Last_Audit_Time, sizeof(Last_Audit_Time)); //for audit card//vinay
//	index_W += sizeof(Last_Audit_Time);
//	Debug_Output1(0, "Write Last_Audit_Time1: %ld", Last_Audit_Time1);
//	Debug_Output1(0, "Write flash_data_read_buffer: %ld", flash_data_read_buffer); //for audit card//vinay 
//	Debug_Output1(0, "Write index_W: %ld", index_W);
	memcpy((flash_data_read_buffer+index_W),&Last_Audit_Time1, sizeof(Last_Audit_Time1));
	index_W += sizeof(Last_Audit_Time1);

	memcpy((flash_data_read_buffer+index_W), &total_coin_values, sizeof(total_coin_values));
	index_W += sizeof(total_coin_values);

	memcpy((flash_data_read_buffer+index_W), &Self_calibrated_after_last_cfg, sizeof(Self_calibrated_after_last_cfg));
	index_W += sizeof(Self_calibrated_after_last_cfg);

	for(i=0;i<20;i++)
		{
			memcpy((flash_data_read_buffer+index_W),&coin_type[i], sizeof(coin_type[i]));
			index_W += sizeof(coin_type[i]);	////VT added all coin seperate data for coin only meter
		}
	
	flag_write_address = Flash_Write( flash_data_read_buffer,index_W, flag_write_address, SINGLE_SECTOR_ERASE );
	index_W=0;   //Total 513 bytes index.
	if(No_Modem_Coin_Only) //to avoid screen stuck in other screens //vinay
	{
		//Debug_TextOut(0,"7");
		DSM_Screen1();
	}
	return;

}

/**************************************************************************/
//! Writes the RTC and Parking Clock in flash, Reads at bootup. Generally gets called every
//! minute. Two sectors are allocated for this in flash. New records are
//! only appended to the end until the complete slot is used up. Then erases
//! and starts from beginning.
//! \param write_rtc_into_flash - flag to decide whether to read or write
//! \param sync_RTC_park_time - If enabled, tries to synchronize parking clock countdown offsets with RTC
//! \return void
/**************************************************************************/
void Flash_RTC_ParkingClock_Storage(uint8_t write_rtc_into_flash, uint8_t sync_RTC_park_time)
{
	uint32_t flash_rtc_address = address_to_write_RTC, seconds_difference = 0;
	uint16_t rtc_read = 0, rtc_comp = 0;
	uint8_t rtc_erase_flag = SINGLE_SECTOR_ERASE;
	uint8_t prktime_hrs_left_space = 0, prktime_mins_left_space = 0;
	uint8_t prktime_hrs_right_space = 0, prktime_mins_right_space = 0;
	uint8_t i=0;

	Coin_Only_Audit_to_Flash(); //To write coin values to flash for Coin Only LNG meters

	if((write_rtc_into_flash == true) && (address_to_write_RTC < ((AD_RTCParkingClk_r+FLASH_SECTOR_SIZE)-sizeof(RTC_ParkingTime)))&&(sync_RTC_park_time==FALSE))
	{
		flash_rtc_address = Flash_Read(flash_rtc_address, 2*(sizeof(RTC_ParkingTime)));

		for(rtc_comp = 0; rtc_comp < 2*(sizeof(RTC_ParkingTime)); rtc_comp+=sizeof(RTC_ParkingTime))
		{
			if((flash_data_read_buffer[rtc_comp] == 0xFF) && (flash_data_read_buffer[rtc_comp+1] == 0xFF) && (flash_data_read_buffer[rtc_comp+2] == 0xFF) && (flash_data_read_buffer[rtc_comp+3] == 0xFF)
					&& (flash_data_read_buffer[rtc_comp+4] == 0xFF) && (flash_data_read_buffer[rtc_comp+5] == 0xFF) && (flash_data_read_buffer[rtc_comp+6] == 0xFF) && (flash_data_read_buffer[rtc_comp+7] == 0xFF))	//free memory found, break and write in this address
			{
				//Debug_Output1(0,"found at 1:%d",rtc_comp);
				rtc_erase_flag = false;
				address_to_write_RTC += rtc_comp;
				break;
			}
		}
	}

	if((write_rtc_into_flash == false) || ((rtc_erase_flag == SINGLE_SECTOR_ERASE)&&(sync_RTC_park_time==FALSE)))	//Some error while searching to write or while reading only: scan full sector
	{
		flash_rtc_address = AD_RTCParkingClk_r;

		for(rtc_read = 0; rtc_read < (FLASH_SECTOR_SIZE/GPRS_GEN_BUF_SZ_LARGE); rtc_read++)	// Read full sector to maintain timing
		{
			//Note that no break condition for the outside for loop, this is to make sure full sector is read every time and we maintain the timing; this was the major reason for parking clock count down issue as we disable interrupt inside the flash_read
			flash_rtc_address = Flash_Read(flash_rtc_address, GPRS_GEN_BUF_SZ_LARGE);

			if(rtc_erase_flag == SINGLE_SECTOR_ERASE)	// Not found yet, keep searching
			{
				for(rtc_comp = 0; rtc_comp < GPRS_GEN_BUF_SZ_LARGE; rtc_comp+=sizeof(RTC_ParkingTime))
				{
					if((flash_data_read_buffer[rtc_comp] == 0xFF) && (flash_data_read_buffer[rtc_comp+1] == 0xFF) && (flash_data_read_buffer[rtc_comp+2] == 0xFF) && (flash_data_read_buffer[rtc_comp+3] == 0xFF)
							&& (flash_data_read_buffer[rtc_comp+4] == 0xFF) && (flash_data_read_buffer[rtc_comp+5] == 0xFF) && (flash_data_read_buffer[rtc_comp+6] == 0xFF) && (flash_data_read_buffer[rtc_comp+7] == 0xFF))	//free memory found, break and write in this address
					{
						//Debug_Output2(2, "found: %d, %d", rtc_comp, rtc_read);
						rtc_erase_flag = false;
						address_to_write_RTC = (rtc_read*GPRS_GEN_BUF_SZ_LARGE) + rtc_comp + AD_RTCParkingClk_r;
						break;
					}
				}
			}
/*			else
			{
				DelayUs(60);
			}*/
		}
	}

	if(sync_RTC_park_time==FALSE)
		{
			if(rtc_erase_flag == false)	//write address found
			{
				//flash_rtc_address -= sizeof(RTC_ParkingTime);
				flash_rtc_address = address_to_write_RTC;
			}
			else	//Full sector used up, erase and reuse now
			{
				//Debug_TextOut(0, "RTC Parking Clock Page used, Overwrite");
				//RTC_ParkingTime.RTC_Update = RTC_epoch_now();
				//to store the RTC before erasing the block. and then store the same RTC from this above variables//vinay
				flash_rtc_address = AD_RTCParkingClk_w;
			}
		}

	if(write_rtc_into_flash == true)	//Write to memory only if erased flash memory is found
	{

		for(i=0;i<glSystem_cfg.MSM_max_spaces ;i++)
		{
		if((Last_RTC_Park_clock_sync[i] < RTC_PARK_CLK_SYNC_INTERVAL) && (parking_time_left_on_meter[i] > RTC_PARK_CLK_SYNC_INTERVAL))
			Last_RTC_Park_clock_sync[i] = parking_time_left_on_meter[i];	//May be a new payment, so update in any case

		if(sync_RTC_park_time == TRUE)
		{
			//Debug_Output2(0,"Check PC offset: %ld, %ld", parking_time_left_on_meter[i], Last_RTC_Park_clock_sync[i]);
			if((Last_RTC_Park_clock_sync[i] > parking_time_left_on_meter[i]) && (Last_RTC_Park_clock_sync[i] > RTC_PARK_CLK_SYNC_INTERVAL))
			{
				seconds_difference = Last_RTC_Park_clock_sync[i] - parking_time_left_on_meter[i];
				//Debug_Output1( 0, "seconds_difference : %ld\r\n",seconds_difference);
				if((seconds_difference > RTC_PARK_CLK_SYNC_INTERVAL) && (seconds_difference < (RTC_PARK_CLK_SYNC_INTERVAL+MAX_ALLOWED_SYNC_SECONDS)))	//Allow sync only upto 10 seconds to avoid a complete invalid sync
				{
					parking_time_left_on_meter[i] = parking_time_left_on_meter[i] + (seconds_difference - RTC_PARK_CLK_SYNC_INTERVAL);
					//Debug_TextOut(0,"PC offset adjusted Down");
				}
				else if((seconds_difference < RTC_PARK_CLK_SYNC_INTERVAL) && (seconds_difference > (RTC_PARK_CLK_SYNC_INTERVAL - MAX_ALLOWED_SYNC_SECONDS)))
				{
					parking_time_left_on_meter[i] = parking_time_left_on_meter[i] - (RTC_PARK_CLK_SYNC_INTERVAL - seconds_difference);
					//Debug_TextOut(0,"PC offset adjusted Up");
				}
				else
				{
					//Debug_TextOut(0,"PC Running OK");
				}
			}
			Last_RTC_Park_clock_sync[i] = parking_time_left_on_meter[i];
		}
		}
		
//#if	1
if(sync_RTC_park_time==FALSE)
	{
		//Left Space
		prktime_hrs_left_space = (uint8_t)(parking_time_left_on_meter[0] / 3600);
		prktime_mins_left_space = (uint8_t)((parking_time_left_on_meter[0] % 3600)/60);
		if((parking_time_left_on_meter[0]>0)&&(parking_time_left_on_meter[0]<60)&&(prktime_mins_left_space==0))
			{
				prktime_mins_left_space += 1;
			}

		//Right Space
		prktime_hrs_right_space = (uint8_t)(parking_time_left_on_meter[1] / 3600);
		prktime_mins_right_space = (uint8_t)((parking_time_left_on_meter[1] % 3600)/60);
		if((parking_time_left_on_meter[1]>0)&&(parking_time_left_on_meter[1]<60)&&(prktime_mins_right_space==0))
			{
				prktime_mins_right_space += 1;
			}
		//HRS/MINS of both spaces Write to Flash
		RTC_ParkingTime.Max_parking_time[0] = prktime_hrs_left_space;
		RTC_ParkingTime.Max_parking_time[1] = prktime_mins_left_space;
		RTC_ParkingTime.Max_parking_time[2] = prktime_hrs_right_space;
		RTC_ParkingTime.Max_parking_time[3] = prktime_mins_right_space;
//#endif
		//RTC_ParkingTime.Max_parking_time = parking_time_left_on_meter[Current_Space_Id];
		RTC_ParkingTime.RTC_Update = RTC_epoch_now();
		flash_rtc_address = Flash_Write( &RTC_ParkingTime.Max_parking_time[0], sizeof(RTC_ParkingTime), flash_rtc_address, rtc_erase_flag );
		Debug_Output1(0,"Write address:%ld", flash_rtc_address);
		//Debug_Output2(0,"Left Hrs:%d, Left Mns:%d", RTC_ParkingTime.Max_parking_time[0],RTC_ParkingTime.Max_parking_time[1]);
		//Debug_Output2(0,"Right Hrs:%d, Right Mns:%d", RTC_ParkingTime.Max_parking_time[2],RTC_ParkingTime.Max_parking_time[3]);
		//Debug_Output2(2,"rtc:%ld, parking clock:%ld", RTC_ParkingTime.RTC_Update, RTC_ParkingTime.Max_parking_time);
		//Debug_Output1(2,"address:%ld", flash_rtc_address);
		if(No_Modem_Coin_Only)
		{
			//Debug_TextOut(0,"8");
			DSM_Screen1();
		}

	}
}
	else
	{
		if(flash_rtc_address > AD_RTCParkingClk_r + (sizeof(RTC_ParkingTime)))
			flash_rtc_address -= sizeof(RTC_ParkingTime);	// read the previous 8 bytes to FFFFFFFF

		flash_rtc_address = Flash_Read(flash_rtc_address, sizeof(RTC_ParkingTime));
		memcpy(&RTC_ParkingTime.Max_parking_time[0],flash_data_read_buffer,MAX_PARKING_TIME_LENGTH);
		memcpy(&RTC_ParkingTime.RTC_Update,(flash_data_read_buffer+MAX_PARKING_TIME_LENGTH),RTC_UPDATE_LENGTH );
		Debug_Output1(0,"Read address:%ld", flash_rtc_address);
		//Debug_Output2(0,"Left Hrs:%d, Left Mns:%d", RTC_ParkingTime.Max_parking_time[0],RTC_ParkingTime.Max_parking_time[1]);
		//Debug_Output2(0,"Right Hrs:%d, Right Mns:%d", RTC_ParkingTime.Max_parking_time[2],RTC_ParkingTime.Max_parking_time[3]);
		//Debug_Output1(2,"read address:%ld", flash_rtc_address);
	}

	//__enable_interrupt();
}

/**************************************************************************/
//! Reads and writes Coin Calibrated data at the end of calibration
//! \param overwrite_calibration - flag to decide whether to overwrite or
//! read calibrated data from flash
//! \return void
/**************************************************************************/
void Flash_Coin_Calibration_Params(uint8_t overwrite_calibration)
{
	uint32_t 		flash_calibration_address = 0;
	uint8_t 		i;//, calibration_found = false;
	uint16_t		Coin_Value_to_compare = 0;
	uint16_t 		max_calib_tolerance = 0, min_calib_tolerance = 0;

	if(overwrite_calibration == ERASE_CALIBRATION)//erase calibration code
	{
		flash_calibration_address = AD_Coin_Calibration_w;
		flash_calibration_address = Flash_Batch_Erase( flash_calibration_address, SINGLE_SECTOR_ERASE, false );

		//Mark as Calibration Not done so that meter enters calibration with the next config download
		//Read back the full sector, change only the required flag and write back
		Flash_Batch_Flag_Read();
		Self_calibrated_after_last_cfg = FALSE;
		Flash_Batch_Flag_Write();
	}
	else if((overwrite_calibration == OVERWRITE_AUTO_CALIBRATION) || (overwrite_calibration == OVERWRITE_SELF_CALIBRATION))
	{
		flash_calibration_address = AD_Coin_Calibration_w;
		flash_calibration_address = Flash_Write( Coin_Calib_Params, sizeof(Coin_Calib_Params), flash_calibration_address, SINGLE_SECTOR_ERASE );

		//Mark as Calibration Completed in flash so that meter doesn't enter calibration repeatedly with same config
		//Read back the full sector, change only the required flag and write back
		Flash_Batch_Flag_Read();
		Self_calibrated_after_last_cfg = TRUE;
		Flash_Batch_Flag_Write();
		if(overwrite_calibration == OVERWRITE_AUTO_CALIBRATION)	//Send a common event for AUTO CALIBRATION
		{
			//generate payload for this event here
			//Pass all 0xFF to show that all coins calibrated
			Coin_cal_index = 0xFF;
			Coin_cal_max = 0xFFFF;
			Coin_cal_min = 0xFFFF;
			push_event_to_cache(UDP_EVTTYP_COIN_CALIBRATION_VALUES);
			Tasks_Priority_Register |= SEVERE_EVENTS_TASK;
		}
		else //find the index and then generate payload only for that index in case of bit wise self calibration
		{
			Coin_cal_index = 0xFF;
			for(i=0; i<(MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT); i++)
			{
				max_calib_tolerance = glSystem_cfg.coin_calib_tol[i].coin_max_tolerance*2;
				min_calib_tolerance = glSystem_cfg.coin_calib_tol[i].coin_min_tolerance*2;

				//Check if self calibration for particular index is enabled and it is not a repeat calibration and it is for a valid coin value
				if(((glSystem_cfg.Coin_self_calib_cfg>>i) & BIT_0) == BIT_0)
				{
					Debug_Output1(0,"Self Calibration done for index: %d", i);
					//generate payload for this event here
					Coin_cal_index = i;
					Coin_cal_max = Coin_Calib_Params[i].params[0] + max_calib_tolerance;
					Coin_cal_min = Coin_Calib_Params[i].params[2] + min_calib_tolerance;
					push_event_to_cache(UDP_EVTTYP_COIN_CALIBRATION_VALUES);
					add_2_seconds += 2;
					Tasks_Priority_Register |= SEVERE_EVENTS_TASK;
				}
			}
			add_2_seconds = 0;
			if(Coin_cal_index == 0xFF)	//All Coins went through self calibration
			{
				Coin_cal_index = 0xFF;
				Coin_cal_max = 0xFFFF;
				Coin_cal_min = 0xFFFF;
				push_event_to_cache(UDP_EVTTYP_COIN_CALIBRATION_VALUES);
				Tasks_Priority_Register |= SEVERE_EVENTS_TASK;
			}
		}
	}

	flash_calibration_address = AD_Coin_Calibration_r;
	flash_calibration_address = Flash_Read(flash_calibration_address, sizeof(Coin_Calib_Params));
	memcpy(Coin_Calib_Params, flash_data_read_buffer, sizeof(Coin_Calib_Params));

	//calibration_found = true;
	for(i=0; i<(MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT); i++)	//Copy the calibrated values to RAM config
	{
		if(i < MAX_COINS_SUPPORTED)
		{
			if((glSystem_cfg.coin_parameters[i].value_units == Coin_Calib_Params[i].value_units) && (Coin_Calib_Params[i].value_units != 0) && (Coin_Calib_Params[i].value_units != 0xFFFF))
			{
				memcpy(glSystem_cfg.coin_parameters[i].params, Coin_Calib_Params[i].params, sizeof(Coin_Calib_Params[i].params));
				if((overwrite_calibration == OVERWRITE_AUTO_CALIBRATION) || (overwrite_calibration == OVERWRITE_SELF_CALIBRATION))	//print these only while calibrating
					Debug_Output1(0,"Calibration written for index: %d", i);
				else
					Debug_Output1(0,"Calibration found and copied for index: %d", i);

//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[0] = %d;", i, glSystem_cfg.coin_parameters[i].params[0]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[1] = %d;", i, glSystem_cfg.coin_parameters[i].params[1]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[2] = %d;", i, glSystem_cfg.coin_parameters[i].params[2]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[3] = %d;", i, glSystem_cfg.coin_parameters[i].params[3]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[4] = %d;", i, glSystem_cfg.coin_parameters[i].params[4]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[5] = %d;", i, glSystem_cfg.coin_parameters[i].params[5]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[6] = %d;", i, glSystem_cfg.coin_parameters[i].params[6]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[7] = %d;", i, glSystem_cfg.coin_parameters[i].params[7]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].value_units = %d;\r\n", i, glSystem_cfg.coin_parameters[i].value_units);
			}
			else if((glSystem_cfg.coin_parameters[i].value_units != 0) && (glSystem_cfg.coin_parameters[i].value_units != 0xFFFF))
			{
				//calibration_found = false;
				Debug_Output1(0,"Calibration Not found, using from config for index: %d", i);

//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[0] = %d;", i, glSystem_cfg.coin_parameters[i].params[0]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[1] = %d;", i, glSystem_cfg.coin_parameters[i].params[1]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[2] = %d;", i, glSystem_cfg.coin_parameters[i].params[2]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[3] = %d;", i, glSystem_cfg.coin_parameters[i].params[3]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[4] = %d;", i, glSystem_cfg.coin_parameters[i].params[4]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[5] = %d;", i, glSystem_cfg.coin_parameters[i].params[5]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[6] = %d;", i, glSystem_cfg.coin_parameters[i].params[6]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].params[7] = %d;", i, glSystem_cfg.coin_parameters[i].params[7]);
//				Debug_Output2(0,"glSystem_cfg.coin_parameters[%d].value_units = %d;\r\n", i, glSystem_cfg.coin_parameters[i].value_units);
			}
		}
		else	//Copy multipart here
		{
			if((glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].value_units == Coin_Calib_Params[i].value_units) && (Coin_Calib_Params[i].value_units != 0) && (Coin_Calib_Params[i].value_units != 0xFFFF))
			{
				//for multipart, only the narrow params are part of self calibration
				memcpy(glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params, Coin_Calib_Params[i].params, sizeof(Coin_Calib_Params[i].params));
				if((overwrite_calibration == OVERWRITE_AUTO_CALIBRATION) || (overwrite_calibration == OVERWRITE_SELF_CALIBRATION))	//print these only while calibrating
					Debug_Output1(0,"Calibration written for index: %d", i);
				else
					Debug_Output1(0,"Calibration found and copied for index: %d", i);

//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[0] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[0]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[1] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[1]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[2] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[2]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[3] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[3]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[4] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[4]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[5] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[5]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[6] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[6]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[7] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[7]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[0] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[0]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[1] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[1]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[2] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[2]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[3] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[3]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[4] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[4]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[5] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[5]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[6] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[6]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[7] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[7]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].value_units = %d;\r\n", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].value_units);

			}
			else if((glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].value_units != 0)&&(glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].value_units != 0xFFFF))
			{
				//calibration_found = false;
				Debug_Output1(0,"Calibration Not found, using from config for index: %d", i);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[0] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[0]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[1] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[1]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[2] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[2]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[3] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[3]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[4] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[4]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[5] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[5]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[6] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[6]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].narrow_params[7] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params[7]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[0] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[0]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[1] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[1]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[2] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[2]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[3] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[3]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[4] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[4]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[5] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[5]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[6] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[6]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].expanded_params[7] = %d;", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].expanded_params[7]);
//				Debug_Output2(0,"glSystem_cfg.multipart_coin_parameters[%d].value_units = %d;\r\n", (i - MAX_COINS_SUPPORTED), glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].value_units);
			}
		}
	}

	if(((Coin_Calib_Params[0].value_units != 0) && (Coin_Calib_Params[0].value_units != 0xFFFF))||
	   ((Coin_Calib_Params[1].value_units != 0) && (Coin_Calib_Params[1].value_units != 0xFFFF))||
	   ((Coin_Calib_Params[2].value_units != 0) && (Coin_Calib_Params[2].value_units != 0xFFFF)))
		{
			Opto_Coin_Disable = FALSE;	// Opto will be disabled if no coins params entered in index 0,1,2  VT
			Debug_TextOut(0,"Opto Coin Params Found, Opto Enabled");
		}
	else
		{
			Opto_Coin_Disable = TRUE;	// Opto will be enabled if coins params found in index 0,1,2  VT
			Debug_TextOut(0,"Opto Coin Params Not Found, Opto Disabled");
		}
	//Index 3 is now used for coin values, because customer is asking for different values for different coins//vinay
	if(glSystem_cfg.coin_parameters[3].value_units > 0)
	{
		if(((glSystem_cfg.coin_parameters[3].params[0] > 0) || (glSystem_cfg.coin_parameters[3].params[1] > 0) || (glSystem_cfg.coin_parameters[3].params[2] > 0) || (glSystem_cfg.coin_parameters[3].params[3] > 0) || (glSystem_cfg.coin_parameters[3].params[4] > 0) || (glSystem_cfg.coin_parameters[3].params[5] > 0) || (glSystem_cfg.coin_parameters[3].params[6] > 0) || (glSystem_cfg.coin_parameters[3].params[7] > 0)) == 0)
		{
			glSystem_cfg.coin_parameters[3].value_units = 0;
			Debug_TextOut(0,"COIN VALUES ENABLED BUT 'coin_parameters[3]' are set to zero");
			Debug_TextOut(0,"So 'amount_per_hour' ARE TAKEN FROM 'no_of_valid_rates'");
		}
		else
			Debug_TextOut(0,"COIN VALUES ENABLED SO 'amount_per_hour' ARE TAKEN FROM 'coin_parameters[3]'");
//		Debug_Output1(0,"glSystem_cfg.coin_parameters[3].value_units = %d", glSystem_cfg.coin_parameters[3].value_units);
//		Debug_Output1(0,"glSystem_cfg.coin_parameters[3].params[0] = %d", glSystem_cfg.coin_parameters[3].params[0]);
//		Debug_Output1(0,"glSystem_cfg.coin_parameters[3].params[1] = %d", glSystem_cfg.coin_parameters[3].params[1]);
//		Debug_Output1(0,"glSystem_cfg.coin_parameters[3].params[2] = %d", glSystem_cfg.coin_parameters[3].params[2]);
//		Debug_Output1(0,"glSystem_cfg.coin_parameters[3].params[3] = %d", glSystem_cfg.coin_parameters[3].params[3]);
//		Debug_Output1(0,"glSystem_cfg.coin_parameters[3].params[4] = %d", glSystem_cfg.coin_parameters[3].params[4]);
//		Debug_Output1(0,"glSystem_cfg.coin_parameters[3].params[5] = %d", glSystem_cfg.coin_parameters[3].params[5]);
//		Debug_Output1(0,"glSystem_cfg.coin_parameters[3].params[6] = %d", glSystem_cfg.coin_parameters[3].params[6]);
//		Debug_Output1(0,"glSystem_cfg.coin_parameters[3].params[7] = %d", glSystem_cfg.coin_parameters[3].params[7]);

	}
	else
	{
		Debug_TextOut(0,"COIN VALUES DISABLED SO 'amount_per_hour' ARE TAKEN FROM 'no_of_valid_rates'");
	}


/*	if((calibration_found == false) && (glSystem_cfg.operational_parameters.self_calibration_enable == true))
	{
		//Coin_Calibration_Mode(SELF_CALIBRATION_MODE);	//Do self calibration
		gAutoCalibration = false;	//disable auto calibration mode
		gSelfCalibration = true;	//enable self calibration mode

		for(i=0; i<(MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT); i++)
		{
			memset(Coin_Calib_Params[i].params, 0, sizeof(Coin_Calib_Params[i].params));
			Coin_Calib_Params[i].value_units = 0;

			memset(Self_Calibration[i].SCMin_Sens, 0, sizeof(Self_Calibration[i].SCMin_Sens));
			memset(Self_Calibration[i].SCMax_Sens, 0, sizeof(Self_Calibration[i].SCMax_Sens));
			memset(Self_Calibration[i].SCRatio1, 0, sizeof(Self_Calibration[i].SCRatio1));
			memset(Self_Calibration[i].SCRatio2, 0, sizeof(Self_Calibration[i].SCRatio2));

			Self_Calibration[i].SC_counter = 0;
		}

		Debug_TextOut(0,"Coins Self Calibration Starts");
	}
	else */if(glSystem_cfg.operational_parameters.self_calibration_enable == true)
	{
		//calibration is found, now check and enable if any specific coin is enabled to be self calibrated

		for(i=0; i<(MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT); i++)
		{
			if(i < MAX_COINS_SUPPORTED)
			{
				Coin_Value_to_compare = glSystem_cfg.coin_parameters[i].value_units;
			}
			else
			{
				Coin_Value_to_compare = glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].value_units;
			}

			//Check if self calibration for particular index is enabled and it is not a repeat calibration and it is for a valid coin value
			if((((glSystem_cfg.Coin_self_calib_cfg>>i) & BIT_0) == BIT_0) && (Self_calibrated_after_last_cfg == FALSE) && (Coin_Value_to_compare != 0) && (Coin_Value_to_compare != 0xFFFF))
			{
				memset(Coin_Calib_Params[i].params, 0, sizeof(Coin_Calib_Params[i].params));
				Coin_Calib_Params[i].value_units = 0;

				memset(Self_Calibration[i].SCMin_Sens, 0, sizeof(Self_Calibration[i].SCMin_Sens));
				memset(Self_Calibration[i].SCMax_Sens, 0, sizeof(Self_Calibration[i].SCMax_Sens));
				memset(Self_Calibration[i].SCRatio1, 0, sizeof(Self_Calibration[i].SCRatio1));
				memset(Self_Calibration[i].SCRatio2, 0, sizeof(Self_Calibration[i].SCRatio2));

				Self_Calibration[i].SC_counter = 0;

				gAutoCalibration = false;	//disable auto calibration mode
				gSelfCalibration = true;	//enable self calibration mode
				Debug_Output1(0,"Self Calibrating for index: %d", i);
			}
			else
			{
				//Fake the other coins as calibration completed
				if(i < MAX_COINS_SUPPORTED)
				{
					memcpy(Coin_Calib_Params[i].params, glSystem_cfg.coin_parameters[i].params, sizeof(Coin_Calib_Params[i].params));
					Coin_Calib_Params[i].value_units = Coin_Value_to_compare;
				}
				else
				{
					memcpy(Coin_Calib_Params[i].params, glSystem_cfg.multipart_coin_parameters[i - MAX_COINS_SUPPORTED].narrow_params, sizeof(Coin_Calib_Params[i].params));
					Coin_Calib_Params[i].value_units = Coin_Value_to_compare;
				}
				Self_Calibration[i].SC_counter = 0xFF;
				//Debug_Output1(0,"calibration found for index: %d\r\n", i);
			}
		}

	}
	else gSelfCalibration = false;	//disable self calibration mode,if we didn't enable self cal bit in config; VT
}

/**************************************************************************/
//! Saves the audit data in flash for coin only meters
//! \param void
//! \return void
/**************************************************************************/
void Coin_Only_Audit_to_Flash (void)
{
	uint32_t loCurrentRTCTime = RTC_epoch_now();
	uint8_t i = 0;
	uint8_t processwait1 = 0;
	////if((temp_coin_type0 == 0) && (temp_coin_type1 == 0) && (temp_coin_type2 == 0) && (temp_coin_type3 == 0) && (temp_coin_type_common == 0))
	for(i=0;i<20;i++)
	{
		if(temp_coin_type[i] > 0)
			break;
	}
	if(i >= 20)
		{
		   return;
		}

	if(glTotal_Valid_Coin_Count > 0)
		{
			if(loCurrentRTCTime > (Last_Cash_Payment_RTC + 30))	//idle for 30sec and RTC interrupt occurs
			{
				//diag_text_Screen("Processing....Please Wait", true);
				if((cardread == 1)||
						((loCurrentRTCTime < (coindrop_TimeOut_RTC + 30))&&(coindrop_TimeOut_RTC < (loCurrentRTCTime + 30)))||
							((loCurrentRTCTime < (KeyPress_TimeOut_RTC + 30)) && (KeyPress_TimeOut_RTC < loCurrentRTCTime + 30)))	//this is avoid please wait msg during CC payment //vinay)
				{
					//Debug_TextOut(0,"Flash1677CardRead=1");
					Debug_TextOut(0,"Processing Please Wait.....");
				}
				else
				{
					//Debug_TextOut(0,"Flash1682CardRead=0");
					Debug_TextOut(0,"Processing Please Wait.....");
					Clear_Screen();
					printSharp32x48(" PROCESSING ",25,0,0);//printSharp32x48(" THIS METER ",35,0,0);
					printSharp32x48("   PLEASE    ", 80,0,0);//printSharp32x48("   COVERS   ", 80,0,0);
					printSharp32x48("    WAIT.....     ", 135,0,0);//printSharp32x48(" TWO SPACES  ", 125,0,0);
					//printSharp16x24("PROCESSING PLEASE WAIT....",180,0,0); //They wanted in big font //vinay
					////Update_Parking_Clock_Screen();
					//printSharp_Card(fontGIF5, 80, 22, 8, 488/8);
					//printSharp16x24("PROCESSING....PLEASE WAIT", 170, 0, 0);
					processwait1 = 1;
					cardread = 0;
				}


				Flash_Batch_Flag_Read();

				coin_type0 += temp_coin_type0;
				coin_type1 += temp_coin_type1;
				coin_type2 += temp_coin_type2;
				coin_type3 += temp_coin_type3;
				coin_type_invalid += temp_coin_type_invalid;
				total_coin_values += temp_total_coin_values;
				for(i=0;i<20;i++)
					{
						coin_type[i] += temp_coin_type[i];
			    		//Debug_Output2(0, "temp_coin_type[%d]: %d",i, temp_coin_type[i]);
			    		//Debug_Output2(0, "coin_type[%d]: %d",i, coin_type[i]);
					}
				Flash_Batch_Flag_Write();

				temp_coin_type0 = 0;
				temp_coin_type1 = 0;
				temp_coin_type2 = 0;
				temp_coin_type3 = 0;
				temp_coin_type_invalid = 0;
				temp_total_coin_values = 0;
				for(i=0;i<20;i++)
					{
						temp_coin_type[i] = 0;
					}
			/*	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)	//Revert back the last blank line
				{
					diag_text_Screen(" ", true);
				}
				else	//Revert back the RTC line
				{
					diag_text_Screen(" ", true);
					Update_RTC_on_Display();
				}*/
				Update_RTC_on_Display();
				////Update_Parking_Clock_Screen();
				Screen_Clear = true;
				//Debug_Output1(0,"KeyPress_TimeOut_RTC:%ld", KeyPress_TimeOut_RTC);
				//if(KeyPress_TimeOut_RTC == 0) //to avoid screen refresh every min when keypressed //vinay
				if((KeyPress_TimeOut_RTC == 0) && (coindrop_TimeOut_RTC == 0))
				DSM_Screen1();//Idle_Screen();
				if(processwait1 == 1)
				{
					DSM_Screen1();//Idle_Screen();  //this is causing long hold of processing please wait msg. //so uncmmted //vinay
					processwait1 = 0;
				}
			}
		}
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

