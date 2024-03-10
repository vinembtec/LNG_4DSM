//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_uid.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_uid_api
//! @{
//
//*****************************************************************************
#include "LibG2_uid.h"

extern GPRSSystem								glSystem_cfg;
extern uint8_t									HARDWARE_REVISION;
extern uint8_t 									flash_data_read_buffer[ FLASH_DATA_READ_BUF_SIZE ];

uint8_t 										err1 = 0;
UID 											External_Uid,Internal_Uid;
Data_Key 										External_Key, Internal_Key;
MBInfo      									glMB_info;
uint32_t 										Serial_No_ASCII = 0;

/**************************************************************************/
//! Delay function, it will delay the CPU for at least 1us in iTime = 1.
//! Very Time sensitive communication bus: Do not alter anything in this.
/**************************************************************************/
void wait_delay(uint16_t iTime)
{
	volatile uint16_t _l ;

	for (_l = 0; _l < iTime; _l++)
	{
		//_delay_1us();
	}
	return;
}

/**************************************************************************/
//! Write 1 to the 1-wire bus
//! Very Time sensitive communication bus: Do not alter anything in this.
/**************************************************************************/
void set_1wirebus_high(uint8_t Port_num)
{
	switch (Port_num)
	{
	case Internal_UID_chip:
		INT_UID_IO_DATA_DIR |= INT_UID_IO_DATA_BIT;
		INT_UID_IO_DATA_OUT |= INT_UID_IO_DATA_BIT;
		break;
	case External_UID_chip:
		EXT_UID_IO_DATA_DIR |= EXT_UID_IO_DATA_BIT;
		EXT_UID_IO_DATA_OUT |= EXT_UID_IO_DATA_BIT;
		break;
	}
}

/**************************************************************************/
//! Write 0 to the 1-wire bus
//! Very Time sensitive communication bus: Do not alter anything in this.
/**************************************************************************/
void set_1wirebus_low(uint8_t Port_num)
{
	switch (Port_num)
	{
	case Internal_UID_chip:
		INT_UID_IO_DATA_DIR |= INT_UID_IO_DATA_BIT;
		INT_UID_IO_DATA_OUT &= ~INT_UID_IO_DATA_BIT;
		break;
	case External_UID_chip:
		EXT_UID_IO_DATA_DIR |= EXT_UID_IO_DATA_BIT;
		EXT_UID_IO_DATA_OUT &= ~EXT_UID_IO_DATA_BIT;
		break;
	}
}

/**************************************************************************/
//! Init the 1-wire bus port
//! Very Time sensitive communication bus: Do not alter anything in this.
/**************************************************************************/
void bus_port_init(uint8_t Port_num)
{
	//Init the bus port
	switch (Port_num)
	{
	case Internal_UID_chip:
		INT_UID_IO_DATA_DIR &= ~INT_UID_IO_DATA_BIT;
		INT_UID_IO_DATA_OUT |= INT_UID_IO_DATA_BIT;
		err1 = INT_UID_IO_DATA_IN & INT_UID_IO_DATA_BIT;//GPIO_getInputPinValue(INT_UID_IO_PORT, INT_UID_IO_PIN);
		break;
	case External_UID_chip:
		EXT_UID_IO_DATA_DIR &= ~EXT_UID_IO_DATA_BIT;
		EXT_UID_IO_DATA_OUT |= EXT_UID_IO_DATA_BIT;
		err1 = EXT_UID_IO_DATA_IN & EXT_UID_IO_DATA_BIT;//GPIO_getInputPinValue(EXT_UID_IO_PORT, EXT_UID_IO_PIN);
		break;
	}
}

/**************************************************************************/
//! Init the DS2740 coulomb counter
//! Very Time sensitive communication bus: Do not alter anything in this.
/**************************************************************************/
uint8_t UID_init(uint8_t Port_num)
{
	/*The initialization sequence required to begin any communication with the DS2740.
 	A presence pulse following a reset pulse indicates that the DS2740 is ready to accept a net address command.
 	1. The bus master transmits (Tx) a reset pulse for tRSTL.
 	2. The bus master then releases the line and goes into Receive mode (Rx).
 	3. The 1-Wire bus line is then pulled high by the pullup resistor.
    After detecting the rising edge on the DQ pin, the DS2740 waits for tPDH and then transmits the
    presence pulse for tPDL.
	 */
	uint8_t Bitdata;
	uint16_t loTime = 0;
	uint8_t loPresenceFlag = 0;

	//Init the 1-wire bus port
	bus_port_init(Port_num);
	//1.The bus master transmits (Tx) a reset pulse for tRSTL:
	set_1wirebus_low(Port_num);
	//Wait for tRSTL
	wait_delay(TIME_RESET_TIME_LOW);
	//DelayUs(TIME_RESET_TIME_LOW);
	//2.The bus master then releases the line:
	//set_1wirebus_rx();

	switch (Port_num)
	{
	case Internal_UID_chip:
		INT_UID_IO_DATA_DIR &= ~INT_UID_IO_DATA_BIT;
		break;
	case External_UID_chip:
		EXT_UID_IO_DATA_DIR &= ~EXT_UID_IO_DATA_BIT;
		break;
	}
	//3. The bus now should be pulled high by the pull up resistor.
	//   We should loop for tRSTH until DS2740 sends the presence pulse.
	while(loTime < (TIME_RESET_TIME_HIGH))
	{
		//Do it once
		if(!loPresenceFlag)
		{
			switch (Port_num)
			{
			case Internal_UID_chip:
				Bitdata = INT_UID_IO_DATA_IN & INT_UID_IO_DATA_BIT;//GPIO_getInputPinValue(INT_UID_IO_PORT, INT_UID_IO_PIN);
				break;
			case External_UID_chip:
				Bitdata = EXT_UID_IO_DATA_IN & EXT_UID_IO_DATA_BIT;//GPIO_getInputPinValue(EXT_UID_IO_PORT, EXT_UID_IO_PIN);
				break;
			}
			if( Bitdata == 0)
			{
				loPresenceFlag =TRUE;
			}
		}
		loTime++;
	}
	return loPresenceFlag;
}

/**************************************************************************/
//! Write one byte (8 bits)
//! Very Time sensitive communication bus: Do not alter anything in this.
/**************************************************************************/
void UID_wr_byte(uint8_t iData,uint8_t Port_num)
{
	uint8_t loLoop;

	// write out bits, lsb first.
	for(loLoop=0; loLoop < 8; loLoop++)
	{
		//A write-time slot is initiated when the bus master pulls the 1-Wire bus from a logic-high (inactive)
		//level to a logic-low level.
		if (iData & 0x01)
		{

			//set the bus low
			set_1wirebus_low(Port_num);
			set_1wirebus_high(Port_num);
			wait_delay(TIME_SLOT_MIN); //wait for tSLOT us
		}
		else
		{
			//set the bus low
			set_1wirebus_low(Port_num);
			wait_delay(TIME_SLOT_MIN); //wait for tSLOT us
		}

		switch (Port_num)
		{
		case Internal_UID_chip:
			INT_UID_IO_DATA_DIR &= ~INT_UID_IO_DATA_BIT;
			break;
		case External_UID_chip:
			EXT_UID_IO_DATA_DIR &= ~EXT_UID_IO_DATA_BIT;
			break;
		}
		wait_delay(1);//standard mode(); //wait for 1 us
		iData >>= 1;  //Shift the data 1 bit right
	}
}

/**************************************************************************/
//! Read one byte (8 bits)
//! Very Time sensitive communication bus: Do not alter anything in this.
/**************************************************************************/
uint8_t UID_rd_byte(uint8_t Port_num)
{
	//1. A read-time slot is initiated when the bus master pulls the 1-Wire bus line from a logic-high level to a logic-low level.
	//2. The bus master must keep the bus line low for at least 1us and then release it to allow the DS2740 to present valid data.
	//3. The bus master can then sample the data tRDV from the start of the read-time slot.
	uint8_t Bitdata;
	uint8_t loLoop = 0;
	uint8_t loData = 0;
	for(loLoop=0; loLoop < 8; loLoop++)
	{
		loData >>= 1;
		//1. set the bus low.
		set_1wirebus_low(Port_num);
		//2. wait for 1 us.

		switch (Port_num)
		{
		case Internal_UID_chip:
			INT_UID_IO_DATA_DIR &= ~INT_UID_IO_DATA_BIT;
			Bitdata = INT_UID_IO_DATA_IN & INT_UID_IO_DATA_BIT;//GPIO_getInputPinValue(INT_UID_IO_PORT, INT_UID_IO_PIN);
			break;
		case External_UID_chip:
			EXT_UID_IO_DATA_DIR &= ~EXT_UID_IO_DATA_BIT;
			Bitdata = EXT_UID_IO_DATA_IN & EXT_UID_IO_DATA_BIT;//GPIO_getInputPinValue(EXT_UID_IO_PORT, EXT_UID_IO_PIN);
			break;
		}

		if( Bitdata == 1)
		{
			loData |= 0x80;
		}
		wait_delay(TIME_SLOT_MIN-1); //wait for tSLOT to finish
	}
	return loData;
}

/**************************************************************************/
//! Read the 64 bits net address  (hex value)
//! Very Time sensitive communication bus: Do not alter anything in this.
/**************************************************************************/
uint8_t UID_read_net_address(UID *chip_add,uint8_t Port_num)
{
	//To read a register we should:
	//1.Initialization
	//2.Net Address Command - 0x33 or 0x39
	//3.Read 8 bytes of the net address.

	//uint64_t loData = 0;
	//uint8_t  loByte[8] = {0};
	uint8_t i;
	if(UID_init(Port_num) == FALSE)
	{
		return 0; //Failed to init
	}

	//Send the Net Address Command
	UID_wr_byte(READ_NET_ADDRESS_CMD,Port_num);
	wait_delay(TIME_SLOT_MIN/2);
	//Now read the 64bits. MSB byte first
	for(i = 0; i < 8; i++)
	{
		chip_add->data[i] = UID_rd_byte(Port_num);
	}
	return chip_add->chip.Family_Code;
}

/**************************************************************************/
//! Writes Data to the scratchpad
//! Very Time sensitive communication bus: Do not alter anything in this.
/**************************************************************************/
uint16_t UID_Write_scrathpad(uint16_t W_Add,uint8_t *Data,uint8_t length,uint8_t Port_num)
{
	uint16_t CRC_Value=0;
	uint8_t L_Count=0;//, RXDATA[2]=0;
	if(UID_init(Port_num) == FALSE)
	{
		return 0; //Failed to init
	}

	UID_wr_byte(SKIP_NET_ADDRESS_CMD,Port_num);
	UID_wr_byte(Write_Scratch_Pad,Port_num);
	UID_wr_byte(W_Add & 0xFF,Port_num);
	UID_wr_byte((W_Add & 0xFF00)>>8,Port_num);
	for(L_Count=0;L_Count<length;L_Count++)
		UID_wr_byte(Data[L_Count],Port_num);

	CRC_Value=UID_rd_byte(Port_num);
	CRC_Value=CRC_Value|(UID_rd_byte(Port_num)<<8);

	return CRC_Value;
}

/**************************************************************************/
//! Read Data from the scratchpad
//! Very Time sensitive communication bus: Do not alter anything in this.
/**************************************************************************/
uint16_t UID_Read_scrathpad(uint16_t *R_Add,uint8_t *Eadd_Status,uint8_t *Data,uint8_t length,uint8_t Port_num)
{
	uint16_t CRC_Value=0;
	uint8_t L_Count=0;

	if(UID_init(Port_num) == FALSE)
	{
		return 0; //Failed to init
	}

	UID_wr_byte(SKIP_NET_ADDRESS_CMD,Port_num);
	UID_wr_byte(Read_Scratch_pad,Port_num);
	*R_Add=UID_rd_byte(Port_num);
	*R_Add=*R_Add|(UID_rd_byte(Port_num)<<8);
	*Eadd_Status=UID_rd_byte(Port_num);

	for(L_Count=0;L_Count<length;L_Count++)
		Data[L_Count]=UID_rd_byte(Port_num);

	CRC_Value=UID_rd_byte(Port_num);
	CRC_Value=CRC_Value|(UID_rd_byte(Port_num)<<8);

	return CRC_Value;
}

/**************************************************************************/
//! Copies Data from the scratchpad
//! Very Time sensitive communication bus: Do not alter anything in this.
/**************************************************************************/
uint8_t UID_Copy_Scratchpad(uint16_t *T_Add,uint8_t *Eadd_Status,uint8_t Port_num)
{
	if(UID_init(Port_num) == FALSE)
	{
		return 0; //Failed to init
	}

	UID_wr_byte(SKIP_NET_ADDRESS_CMD,Port_num);
	UID_wr_byte(Copy_Scratch_pad,Port_num);
	UID_wr_byte((*T_Add & 0x00FF),Port_num);
	UID_wr_byte((*T_Add & 0xFF00)>>8,Port_num);
	UID_wr_byte(*Eadd_Status,Port_num);

	wait_delay(12500);

	if(UID_rd_byte(Port_num)==0xAA)
		return SUCCESS;
	else
		return FAIL;
}


#if 0
uint8_t UID_Write_Mem(uint16_t W_Add,Data_Key *Data,uint8_t length,uint8_t Port_num)
{
	uint8_t RSP_data[8];
	uint8_t WSP_data[8];
	uint16_t Write_add=W_Add;
	uint16_t L_Read_add=0;
	uint8_t L_ES_reg=0,i=0,j=0,result=0;

	for(j=0;j<32;j=j+8)
	{
		memcpy(WSP_data,&Data->data[j],8);
		memset(RSP_data,0,8);
		Write_add=W_Add+j;
		L_ES_reg=0;
		L_Read_add=0;
		UID_Write_scrathpad(Write_add,WSP_data,8,Port_num);
		UID_Read_scrathpad(&L_Read_add,&L_ES_reg,RSP_data,8,Port_num);

		if(Write_add!=L_Read_add)
			return FAIL;

		if(L_ES_reg!=0x07)
			return FAIL;

		for(i=0;i<8;i++)
		{
			if(WSP_data[i]!=RSP_data[i])
				return FAIL;
		}

		result= UID_Copy_Scratchpad(&L_Read_add,&L_ES_reg,Port_num);
		if(result==0)
			return result;
	}
	return result;
}

uint16_t UID_Read_Mem(uint16_t R_Add,Data_Key *Data,uint8_t length,uint8_t Port_num)
{
	uint8_t L_Count=0;
	if(UID_init(Port_num) == FALSE)
	{
		return 0; //Failed to init
	}

	UID_wr_byte(SKIP_NET_ADDRESS_CMD,Port_num);
	UID_wr_byte(Read_UID_Memory,Port_num);
	UID_wr_byte((R_Add & 0x00FF),Port_num);
	UID_wr_byte((R_Add & 0xFF00)>>8,Port_num);

	for(L_Count=0;L_Count<length;L_Count++)
		Data->data[L_Count]=UID_rd_byte(Port_num);

	return SUCCESS;
}
#endif
/**************************************************************************/
//! Writes Data to the UID memory
//! Very Time sensitive communication bus: Do not alter anything in this.
//! \param uint16_t Write address
//! \param pointer to the data buffer to be written
//! \param uint16_t length of data to be written
//! \param uint16_t Port_num - UID key identifier
//! - \b External_UID_chip
//! - \b Internal_UID_chip
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t UID_Write_Mem(uint16_t W_Add,Data_Key *Data,uint8_t length,uint8_t Port_num)
{
	//uint8_t RSP_data[8];
	uint8_t WSP_data[8];
	uint16_t Write_add=W_Add;
	//uint16_t L_Read_add=0;
	uint8_t /*L_ES_reg=0,*/i=0,j=0,result=0;
	//uint8_t uid_segment = 0;
	//uint8_t uid_page = 0;

	if(UID_init(Port_num) == FALSE)
	{
		return 0; //Failed to init
	}

	for(j=0; j<2; j++)
	{
		UID_wr_byte(SKIP_NET_ADDRESS_CMD,Port_num);

		Write_add = ((j<<4) | (0<<1));
		UID_wr_byte(Copy_Scratch_pad,Port_num);
		UID_wr_byte(Write_add,Port_num);
		UID_wr_byte(0xFF,Port_num);
		for(i=0; i<16; i++)
		{
			WSP_data[0] = Data->data[i + (j*16)];
			WSP_data[1] = Data->data[i + (j*16) + 1];
			UID_wr_byte(WSP_data[0],Port_num);
			UID_wr_byte(WSP_data[1],Port_num);
			//Debug_Output2(0, "written:%02x,%02x", WSP_data[0], WSP_data[1]);
			i++;
			result = UID_rd_byte(Port_num);
			result = UID_rd_byte(Port_num);
			UID_wr_byte(0xFF,Port_num);
			DelayMs(70);
			result = UID_rd_byte(Port_num);

			//if(RSP_data[0] != 0xAA)
			//	;
			//	break;
		}

		if(UID_init(Port_num) == FALSE)
		{
			result = 0; //Failed to init
		}
		else
			result = 1;
		DelayMs(70);
	}

	/*set_1wirebus_low(Port_num);
		//Wait for tRSTL
		wait_delay(TIME_RESET_TIME_LOW);

		uid_segment++;
		if(uid_segment > 7)
		{
			uid_segment = 0;
			result = 1;
			uid_page++;
			if(uid_page > 7)
			{
				uid_page = 0;
			}
		}*/

	return result;
}

/**************************************************************************/
//! Reads Data from the UID memory
//! Very Time sensitive communication bus: Do not alter anything in this.
//! \param uint16_t Read address
//! \param pointer to the data buffer where output will be given
//! \param uint16_t length of data to be read
//! \param uint16_t Port_num - UID key identifier
//! - \b External_UID_chip
//! - \b Internal_UID_chip
//! \return uint16_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint16_t UID_Read_Mem(uint16_t R_Add,Data_Key *Data,uint8_t length,uint8_t Port_num)
{
	uint8_t L_Count=0;
	if(UID_init(Port_num) == FALSE)
	{
		return 0; //Failed to init
	}
	memset(Data->data, 0, sizeof(Data->data));
	UID_wr_byte(SKIP_NET_ADDRESS_CMD,Port_num);
	UID_wr_byte(Read_UID_Memory,Port_num);
	UID_wr_byte((R_Add & 0x00FF),Port_num);
	UID_wr_byte((R_Add & 0xFF00)>>8,Port_num);

	for(L_Count=0;L_Count<length;L_Count++)
		Data->data[L_Count]=UID_rd_byte(Port_num);

	return SUCCESS;
}

/**************************************************************************/
//! This function reads and populates the C-A-M ID, Location ID, Meter Serial
//! Number in the appropriate structures which will be used for further
//! communication of meter with server.
//! For REV3 Main Boards, Reads Unique ID from Internal UID chip and frames
//! Meter serial Number with that. Reads CustomerID, Area ID, Meter ID from
//! External UID and writes into Internal UID chip.
//! For REV3 Main Boards, Reads Unique ID from on-board external flash and frames
//! Meter serial Number with that. Reads CustomerID, Area ID, Meter ID from
//! External UID and writes into on-board external flash.
//! \param void
//! \return void
/**************************************************************************/
void Read_UIDKey()
{
	uint8_t Ext_Family_code = 0, Int_Family_code = 0;
	uint8_t result = 0, /*i = 0, j = 0, Serial_no = 0,*/ ID_found = 0;
	__disable_interrupt();
	CS_setDCOFrequency(CS_24MHZ);	//DS28E05 works at high frequency only
	Ext_Family_code=UID_read_net_address(&External_Uid,External_UID_chip);
	if(Ext_Family_code == 0x0D)
	{
		Debug_TextOut( 0, "Ext UID detected" );
		result=UID_Read_Mem(WR_Add,&External_Key,32,External_UID_chip);
		if(result)
		{
			if((External_Key.Key_data.City_Code!=0xFFFF)&&(External_Key.Key_data.City_Code!=0x00))
			{
				ID_found = TRUE;
				glMB_info.city_code=External_Key.Key_data.City_Code;
				memcpy(glMB_info.locid,External_Key.Key_data.Location_ID,20);
				glSystem_cfg.area_num =External_Key.Key_data.Area_ID;
				glSystem_cfg.cust_id = External_Key.Key_data.Customer_ID;
				glSystem_cfg.meter_id = External_Key.Key_data.Meter_ID;
				memcpy(Internal_Key.data,External_Key.data,32);

				if(HARDWARE_REVISION == LNG_REV3)
				{
					Int_Family_code=UID_read_net_address(&Internal_Uid,Internal_UID_chip);
					if(Int_Family_code==0x0D)
					{
						result=UID_Write_Mem(WR_Add,&Internal_Key,32,Internal_UID_chip);
					}
				}
				else
				{
					CS_setDCOFrequency(CS_4MHZ);	//For REV4 and flash read/write, we need 4MHz
					Flash_Write(&Internal_Key, 32, AD_CAMID_LID_RW, SINGLE_SECTOR_ERASE);
				}
			}
		}
	}
	else
		Debug_Output1( 0, "Ext UID %d", Ext_Family_code);
	if(HARDWARE_REVISION == LNG_REV3)
	{
		Int_Family_code=UID_read_net_address(&Internal_Uid,Internal_UID_chip);
		if(Int_Family_code == 0x0D)
		{
			Debug_TextOut( 0, "Int UID detected" );

			memset(glMB_info.serial_no_asci, 0, sizeof(glMB_info.serial_no_asci));
			memcpy(glMB_info.serial_no_asci, Internal_Uid.chip.Serial_No, sizeof(Internal_Uid.chip.Serial_No));

			result=UID_Read_Mem(WR_Add,&Internal_Key,32,Internal_UID_chip);
			if(result)
			{
				if((Internal_Key.Key_data.City_Code!=0xFFFF)&&(Internal_Key.Key_data.City_Code!=0x00))
				{
					if(((Internal_Key.Key_data.Meter_ID) != 0xFFFF) && ((Internal_Key.Key_data.Meter_ID) != 0))
					{
						glMB_info.city_code=Internal_Key.Key_data.City_Code;
						memcpy(glMB_info.locid,Internal_Key.Key_data.Location_ID,20);
						glSystem_cfg.area_num =Internal_Key.Key_data.Area_ID;
						glSystem_cfg.cust_id = Internal_Key.Key_data.Customer_ID;
						glSystem_cfg.meter_id = Internal_Key.Key_data.Meter_ID;
						ID_found = TRUE;
					}
				}
			}

			/*Debug_Output6(0, "Board Serial Number: %02x%02x%02x%02x%02x%02x", Internal_Uid.chip.Serial_No[0], Internal_Uid.chip.Serial_No[1],
									Internal_Uid.chip.Serial_No[2], Internal_Uid.chip.Serial_No[3], Internal_Uid.chip.Serial_No[4],
									Internal_Uid.chip.Serial_No[5]);*/

			/*Debug_Output6(0, "Serial Number: %02x%02x%02x%02x%02x%02x", glMB_info.serial_no_asci[0], glMB_info.serial_no_asci[1],
									glMB_info.serial_no_asci[2], 0, 0, 0);*/

			Serial_No_ASCII = (((0xffffffff & glMB_info.serial_no_asci[2]) << 16)
					| ((0xffffffff & glMB_info.serial_no_asci[1]) << 8)
					| (0xffffffff  & glMB_info.serial_no_asci[0]));

			sprintf((char*)glMB_info.serial_no_asci,"%lx", Serial_No_ASCII);
			Debug_Output1( 0, "Mech Serial Number: %s", (uint32_t)glMB_info.serial_no_asci);
		}
		else	//Fill default serial number if no internal UID
		{
			Debug_Output1( 0, "Int UID %d", Int_Family_code);
			sprintf((char *)glMB_info.serial_no_asci, "%s", Df_Serial_No);
			sprintf((char *)glMB_info.city_code, "%d", Df_City_Code);
		}
	}
	else
	{
		CS_setDCOFrequency(CS_4MHZ);	//For REV4 and flash read/write, we need 4MHz
		Flash_Read_Unique_ID(&Internal_Uid);
		memcpy(glMB_info.serial_no_asci, Internal_Uid.data, sizeof(Internal_Uid.data));

		Serial_No_ASCII = (((0xffffffff & glMB_info.serial_no_asci[4]) << 16)
				| ((0xffffffff & glMB_info.serial_no_asci[5]) << 8)
				| (0xffffffff  & glMB_info.serial_no_asci[6]));

		sprintf((char*)glMB_info.serial_no_asci,"%lx", Serial_No_ASCII);
		Debug_Output1( 0, "Mech Serial Number: %s", (uint32_t)glMB_info.serial_no_asci);

		Flash_Read(AD_CAMID_LID_RW, 32);
		memcpy(&Internal_Key, flash_data_read_buffer, 32);

		if((Internal_Key.Key_data.City_Code!=0xFFFF)&&(Internal_Key.Key_data.City_Code!=0x00))
		{
			if(((Internal_Key.Key_data.Meter_ID) != 0xFFFF) && ((Internal_Key.Key_data.Meter_ID) != 0))
			{
				glMB_info.city_code=Internal_Key.Key_data.City_Code;
				memcpy(glMB_info.locid,Internal_Key.Key_data.Location_ID,20);
				glSystem_cfg.area_num =Internal_Key.Key_data.Area_ID;
				glSystem_cfg.cust_id = Internal_Key.Key_data.Customer_ID;
				glSystem_cfg.meter_id = Internal_Key.Key_data.Meter_ID;
				ID_found = TRUE;
			}
		}
	}

	if(ID_found != TRUE)
	{
		Debug_TextOut( 0, "Default ID" );
		sprintf((char *)glMB_info.locid, "%s", Df_Location_ID);
		glMB_info.city_code = Df_City_Code;
		glSystem_cfg.area_num = US_SIT_AREA_NUM;
		glSystem_cfg.cust_id = US_SIT_CUST_ID;
		glSystem_cfg.meter_id = METER_ID;
	}
	GPIO_setAsInputPinWithPullUpResistor(EXT_UID_IO_PORT, EXT_UID_IO_PIN);	//EXT_UID
	GPIO_setAsInputPinWithPullUpResistor(INT_UID_IO_PORT, INT_UID_IO_PIN);	//INT_UID_IO
	CS_setDCOFrequency(CS_4MHZ);	//Revert back to 4MHz clock
	__enable_interrupt();
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

