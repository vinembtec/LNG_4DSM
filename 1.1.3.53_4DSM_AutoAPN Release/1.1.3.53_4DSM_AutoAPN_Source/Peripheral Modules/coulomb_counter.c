/*--------------------------------------------------------------------------*/
// COPYRIGHT Duncan Industries , 2013
//
// File: coulomb_counter.c
//
// driver code for DS2740 coulomb counter chip
//
// History:
// 02/15/2013		Ayman S.		Created.
/*--------------------------------------------------------------------------*/
#if 0  //for code optimization
//#include "msp430x54x.h"
#include <stdio.h>
#include <stdlib.h>
//#include "Serial.h"
//#include "mpb_devices.h"
//#include "mpb_stdio.h"
#include "coulomb_counter.h"

//#define _delay_1us()  asm(" nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t");
#define _delay_1us()  asm(" nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t");
// bit flags to conserve RAM
#define gCoulombCounterStatusBit_NotDetected 0x01
#define gCoulombCounterStatusBit_DebugHeaderPrinted 0x02
#define gCoulombCounterStatusBit_FailedReadOnce 0x04
#define gCoulombCounterStatusBit_FailedReadTwice 0x08

uint8_t gCoulombCounterStatusFlags = 0;
uint32_t	min_col_counter_read = 0xFFFFFFFF, accumulated_value_to_report = 0;
uint32_t last_Current_Reading = 0;
//static char debug_msg_1[];


//Delay function, it will delay the CPU for at least 1us in iTime = 1.
void wait_delay_Ds2740(uint16_t iTime)
{
	volatile uint16_t _l ;

	for (_l = 0; _l < iTime; _l++)
	{
		//_delay_1us();
 	}
	return;
}

//--------------------------------------------------------------------
uint8_t get_coulomb_counter_readings()
{
	// if we have don't have coulomb counter, we don't need to retry anymore
	if (( gCoulombCounterStatusFlags & gCoulombCounterStatusBit_NotDetected ) == gCoulombCounterStatusBit_NotDetected )
	{
		return 0;
	}

	//Read the coulomb counter registers
	//P6SEL =0XD0;//26-03-2013:P6.5 is used as GPIO first(for coulomb counter) then as ADC channel
	P10SEL0 &= ~BIT_0;//12-07-13
	uint8_t loStatusReg = coulomb_counter_read_status_reg();
	uint32_t loCurrentValue = coulomb_counter_read_current_reg();
	uint32_t loAccumulatedCurrentValue =	coulomb_counter_read_accumulated_current_reg();
	uint64_t loNetAddress = coulomb_counter_read_net_address();

	/*if(min_col_counter_read > loCurrentValue)
		min_col_counter_read = loCurrentValue;
*/
	Debug_Output1( 0, "last_Current_Reading =%d", last_Current_Reading );
	if(last_Current_Reading == 0)
	{
		last_Current_Reading = loCurrentValue;  // for the first read fake the avg .
	}
	min_col_counter_read = loCurrentValue;//last_Current_Reading;//(uint32_t)((uint32_t)((uint32_t)last_Current_Reading + (uint32_t)loCurrentValue)/(uint32_t)2);
	last_Current_Reading = min_col_counter_read;//  avg value

	accumulated_value_to_report = loAccumulatedCurrentValue;

	Debug_Output1( 0, "loStatusReg =%x", loStatusReg );
	Debug_Output1( 0, "loCurrentValue =%d", loCurrentValue );
	Debug_Output1( 0, "Avg_CurrentValue =%d", min_col_counter_read );
	Debug_Output2( 0, "loAccumulatedCurrentValue =%d.%d", loAccumulatedCurrentValue/1000,loAccumulatedCurrentValue%1000 );

	uint16_t loNetAddress_1 = (uint16_t)((loNetAddress >> 48) & 0xFFFF);
	uint16_t loNetAddress_2 = (uint16_t)((loNetAddress >> 32) & 0xFFFF);
	uint16_t loNetAddress_3 = (uint16_t)((loNetAddress >> 16) & 0xFFFF);
	uint16_t loNetAddress_4 = (uint16_t)(loNetAddress & 0xFFFF);
	Debug_Output6( 0, "loNetAddress_1,2,3,4, = %x %x %x %x %d %d", loNetAddress_1,loNetAddress_2,loNetAddress_3,loNetAddress_4,0,0 );

	/*//Ayman: Before we report our readings, We should check either the Net Addr or Status Reg is 0
	if(loStatusReg == 0 || loNetAddress == 0)
	{
		// One of the major factor is 0, no need to continue.

		if (( gCoulombCounterStatusFlags & gCoulombCounterStatusBit_FailedReadOnce ) != gCoulombCounterStatusBit_FailedReadOnce )
		{
			gCoulombCounterStatusFlags = (gCoulombCounterStatusFlags | gCoulombCounterStatusBit_FailedReadOnce);
			Debug_TextOut(0, "Coulomb Counter Read Failure (1st consecutive attempt)." );
			return 0;
		}

		if (( gCoulombCounterStatusFlags & gCoulombCounterStatusBit_FailedReadTwice ) != gCoulombCounterStatusBit_FailedReadTwice )
		{
			gCoulombCounterStatusFlags = (gCoulombCounterStatusFlags | gCoulombCounterStatusBit_FailedReadTwice);
			Debug_TextOut(0, "Coulomb Counter Read Failure (2nd consecutive attempt)." );
			return 0;
		}
		else
		{
			// Now it has failed three times in a row. we will stop trying
			gCoulombCounterStatusFlags = (gCoulombCounterStatusFlags | gCoulombCounterStatusBit_NotDetected);
			Debug_TextOut(0, "Coulomb Counter Read Failure (3rd consecutive attempt). Will not retry." );
			return 0;
		}

	}

	// good read - reset the consecutive failure counters
		gCoulombCounterStatusFlags = (gCoulombCounterStatusFlags & ~(gCoulombCounterStatusBit_FailedReadOnce | gCoulombCounterStatusBit_FailedReadTwice));


	//Display the results
	uint16_t loNetAddress_1 = (uint16_t)((loNetAddress >> 48) & 0xFFFF);
	uint16_t loNetAddress_2 = (uint16_t)((loNetAddress >> 32) & 0xFFFF);
	uint16_t loNetAddress_3 = (uint16_t)((loNetAddress >> 16) & 0xFFFF);
	uint16_t loNetAddress_4 = (uint16_t)(loNetAddress & 0xFFFF);

	// header info first time only
	if (( gCoulombCounterStatusFlags & gCoulombCounterStatusBit_DebugHeaderPrinted ) != gCoulombCounterStatusBit_DebugHeaderPrinted )
	{
		Debug_Output2(0, "Coulomb Counter Net Addr_H = 0x%04X%04X", loNetAddress_1, loNetAddress_2);
		Debug_Output2(0, "Coulomb Counter Net Addr_L = 0x%04X%04X", loNetAddress_3, loNetAddress_4);
		Debug_TextOut(0, " " );
		Debug_TextOut(0, "Coulomb Counter Data Follows" );
		Debug_TextOut(0, "Status - Accumulated Value - Current Value - Temperature (C)" );
		Debug_TextOut(0, "============================================================" );

		//Debug_TextOut(0, "Status - Accumulated Value - Acc uAmp - Current Value - Current uAmp - Temperature (C)" );
		//Debug_TextOut(0, "======================================================================================" );

		// don't need to see this again
		gCoulombCounterStatusFlags = (gCoulombCounterStatusFlags | gCoulombCounterStatusBit_DebugHeaderPrinted);
	}

	coulomb_counter.Col_cntr_Device_ID[0]=(uint8_t)(loNetAddress_1>>8);
	coulomb_counter.Col_cntr_Device_ID[1]=(uint8_t)(loNetAddress_1);
	coulomb_counter.Col_cntr_Device_ID[2]=(uint8_t)(loNetAddress_2>>8);
	coulomb_counter.Col_cntr_Device_ID[3]=(uint8_t)(loNetAddress_2);
	coulomb_counter.Col_cntr_Device_ID[4]=(uint8_t)(loNetAddress_3>>8);
	coulomb_counter.Col_cntr_Device_ID[5]=(uint8_t)(loNetAddress_3);
	coulomb_counter.Col_cntr_Device_ID[6]=(uint8_t)(loNetAddress_4>>8);
	coulomb_counter.Col_cntr_Device_ID[7]=(uint8_t)(loNetAddress_4);

	//float loCurrentTempF = msp_read_current_temp();
	uint8_t loCurrentTemp_d;
	uint16_t loCurrentTemp_f;
	if(loCurrentTempF < 0) //negative value
	{
		loCurrentTempF = (-1 * loCurrentTempF);
		loCurrentTemp_d = (uint8_t) loCurrentTempF;
		loCurrentTemp_f = (uint16_t)((loCurrentTempF - loCurrentTemp_d) * 1000.0F);
		sprintf(debug_msg_1, "0x%X 0x%X 0x%X -%d.%02d", loStatusReg, loAccumulatedCurrentValue, loCurrentValue, loCurrentTemp_d, loCurrentTemp_f );
	}
	else
	{
		loCurrentTemp_d = (uint8_t) loCurrentTempF;
		loCurrentTemp_f = (uint16_t)((loCurrentTempF - loCurrentTemp_d) * 1000.0F);
		sprintf(debug_msg_1, "0x%X 0x%X 0x%X %d.%02d", loStatusReg, loAccumulatedCurrentValue, loCurrentValue, loCurrentTemp_d, loCurrentTemp_f );
	}

	Debug_TextOut(0, debug_msg_1 );
	coulomb_counter.AccumulatedCurrentValue = loAccumulatedCurrentValue;
	coulomb_counter.CurrentValue = loCurrentValue;
	coulomb_counter.StatusReg = loStatusReg;*/
	//	coulomb_counter.CurrentTempF[temperature_counter++] = loCurrentTempF;

	//Ayman: If we are here that means the data should be ok
	return 1;
}
//-------------------------------------------------------------------------------

//Write 1 to the 1-wire bus
void set_1wirebus_high_Ds2740()
{
	set_1wirebus_tx();
	BITSET(BUS_PORT_OUT, BUS_PIN);
}

//Write 0 to the 1-wire bus
void set_1wirebus_low_Ds2740()
{
	set_1wirebus_tx();
	BITRESET(BUS_PORT_OUT, BUS_PIN);
}

//Init the 1-wire bus port
void bus_port_init_Ds2740()
{
	//Init the bus port
	BITRESET(BUS_PORT_SEL, BUS_PIN); //Port pin function is digital I/O
	//Disable the pullup resistor. We have external pull up resistor.
	BITRESET(BUS_PORT_REN, BUS_PIN);
	//Set the 1-wire bus direction to rx
	set_1wirebus_rx();
}

//Init the DS2740 coulomb counter
uint8_t coulomb_counter_init()
{
	/*The initialization sequence required to begin any communication with the DS2740.
 	A presence pulse following a reset pulse indicates that the DS2740 is ready to accept a net address command.
 	1. The bus master transmits (Tx) a reset pulse for tRSTL.
 	2. The bus master then releases the line and goes into Receive mode (Rx).
 	3. The 1-Wire bus line is then pulled high by the pullup resistor.
    After detecting the rising edge on the DQ pin, the DS2740 waits for tPDH and then transmits the
    presence pulse for tPDL.
	*/
	uint16_t loTime = 0;
	uint8_t loPresenceFlag = 0;

	//Init the 1-wire bus port
	bus_port_init_Ds2740();
	//1.The bus master transmits (Tx) a reset pulse for tRSTL:
	set_1wirebus_low_Ds2740();
	//Wait for tRSTL
	wait_delay_Ds2740(TIME_RESET_TIME_LOW);

	//2.The bus master then releases the line:
	set_1wirebus_rx();

	//3. The bus now should be pulled high by the pull up resistor.
	//   We should loop for tRSTH until DS2740 sends the presence pulse.
  	while(loTime < (TIME_RESET_TIME_HIGH))
	{
		//Do it once
		if(!loPresenceFlag)
		{
			if( (BUS_PORT_IN & BUS_PIN) == 0)
			{
				loPresenceFlag =TRUE;
			}
		}
		loTime++;
		_delay_1us();
	}
	return loPresenceFlag;
}

//Write one byte (8 bits)
void coulomb_counter_wr_byte(uint8_t iData)
{
	uint8_t loLoop;

   	// write out bits, lsb first.
 	for(loLoop=0; loLoop < 8; loLoop++)
  	{
    	//A write-time slot is initiated when the bus master pulls the 1-Wire bus from a logic-high (inactive)
		//level to a logic-low level.

    	//set the bus low
		set_1wirebus_low_Ds2740();
    	_delay_1us(); //wait for 1 us
		if (iData & 0x01) set_1wirebus_high_Ds2740();
       	wait_delay_Ds2740(TIME_SLOT_MIN); //wait for tSLOT us
		set_1wirebus_rx(); //Release the bus
		_delay_1us(); //wait for 1 us
		iData >>= 1;  //Shit the data 1 bit right
  	}
}

//Read one byte (8 bits)
uint8_t coulomb_counter_rd_byte()
{
	//1. A read-time slot is initiated when the bus master pulls the 1-Wire bus line from a logic-high level to a logic-low level.
	//2. The bus master must keep the bus line low for at least 1us and then release it to allow the DS2740 to present valid data.
	//3. The bus master can then sample the data tRDV from the start of the read-time slot.
	uint8_t loLoop = 0;
	uint8_t loData = 0;
	for(loLoop=0; loLoop < 8; loLoop++)
	{
		loData >>= 1;
		//1. set the bus low.
		set_1wirebus_low_Ds2740();
		//2. wait for 1 us.
		_delay_1us();
		//2. release the bus
		set_1wirebus_rx();
		//wait for 1 us.
		_delay_1us();
		//3. now start sample the input data, lsb first
		if((BUS_PORT_IN & BUS_PIN) == BUS_PIN)
		{
			loData |= 0x80;
		}
		wait_delay_Ds2740(TIME_SLOT_MIN-1); //wait for tSLOT to finish
	}

	return loData;
}

//Read the status register
uint8_t coulomb_counter_read_status_reg()
{
	//To read a register we should:
	//1.Initialization
	//2.Net Address Command
	//3.Function Command
	//4.Transaction/Data
	uint8_t loData;
	//stop_timer_isr();
	//init the coulomb counter
	if(coulomb_counter_init() == FALSE)
	{
		//////start_timer_isr();
		return 0; //Failed to init
	}
	//Send the Net Address Command
	coulomb_counter_wr_byte(SKIP_NET_ADDRESS_CMD);
	wait_delay_Ds2740(TIME_SLOT_MIN/2);
	//Send the read command
	coulomb_counter_wr_byte(READ_DATA_CMD);
	//Send the address of the MSB of the current register
	coulomb_counter_wr_byte(STATUS_REGISTER_ADDR);
	//Now read reg and return
	loData = coulomb_counter_rd_byte();
	////start_timer_isr();
	return (loData);
}

//Read the current register (hex value)
uint32_t coulomb_counter_read_current_reg()
{
	//To read a register we should:
	//1.Initialization
	//2.Net Address Command
	//3.Function Command
	//4.Transaction/Data
	uint32_t loDatamul = 0;

	uint16_t loData = 0;
	uint8_t  loDataMSB = 0;
	uint8_t  loDataLSB = 0;
	//stop_timer_isr();
	//init the coulomb counter
	if(coulomb_counter_init() == FALSE)
	{
		////start_timer_isr();
		return 0; //Failed to init
	}

	//Send the Net Address Command
	coulomb_counter_wr_byte(SKIP_NET_ADDRESS_CMD);
	wait_delay_Ds2740(TIME_SLOT_MIN/2);
	//Send the read command
	coulomb_counter_wr_byte(READ_DATA_CMD);
	//Send the address of the MSB of the current register
	coulomb_counter_wr_byte(CURRENT_REGISTER_MSB_ADDR);
	//Now read MSB then LSB bytes
	loDataMSB = coulomb_counter_rd_byte();
	loDataLSB = coulomb_counter_rd_byte();
	////start_timer_isr();
	//now format the data
	loData = loDataMSB;
	loData = (loData<<8) + (0x00ff & loDataLSB);
	loData = (0x7fff & loData);
	loData = 32768 - loData;
	//loDatamul = 62 * loData;//20E
	loDatamul = 78 * loData;//25E
	return loDatamul;
}

//Read the accumulated current register (hex value)
uint32_t coulomb_counter_read_accumulated_current_reg()
{
	//To read a register we should:
	//1.Initialization
	//2.Net Address Command
	//3.Function Command
	//4.Transaction/Data

	uint16_t loData = 0;
	uint32_t loDatamul = 0;
	uint8_t  loDataMSB = 0;
	uint8_t  loDataLSB = 0;
	//stop_timer_isr();
	//init the coulomb counter
	if(coulomb_counter_init() == FALSE)
	{
		////start_timer_isr();
		return 0; //Failed to init
	}
	//Send the Net Address Command
	coulomb_counter_wr_byte(SKIP_NET_ADDRESS_CMD);
	wait_delay_Ds2740(TIME_SLOT_MIN/2);
	//Send the read command
	coulomb_counter_wr_byte(READ_DATA_CMD);
	//Send the address of the MSB of the current register
	coulomb_counter_wr_byte(ACCUMULATED_CURRENT_REGISTER_MSB_ADDR);
	//Now read MSB then LSB bytes
	loDataMSB = coulomb_counter_rd_byte();
	loDataLSB = coulomb_counter_rd_byte();
	////start_timer_isr();
	//now format the data
	loData = loDataMSB;
	loData = (loData<<8) + (0x00ff & loDataLSB);
	loData = (0x7fff & loData);
	loData = 32768 - loData;
	loDatamul = 250 * loData;

	return loDatamul;
}

//Read the 64 bits net address  (hex value)
uint64_t coulomb_counter_read_net_address()
{
	//To read a register we should:
	//1.Initialization
	//2.Net Address Command - 0x33 or 0x39
	//3.Read 8 bytes of the net address.

	uint64_t loData = 0;
	uint8_t  loByte[8] = {0};
	uint8_t i;
	//stop_timer_isr();
	//init the coulomb counter
	if(coulomb_counter_init() == FALSE)
	{
		////start_timer_isr();
		return 0; //Failed to init
	}

	//Send the Net Address Command
	coulomb_counter_wr_byte(READ_NET_ADDRESS_CMD);
	wait_delay_Ds2740(TIME_SLOT_MIN/2);
 	//Now read the 64bits. MSB byte first
	for(i = 0; i < 8; i++)
	{
		loByte[i] = coulomb_counter_rd_byte();
	}
	//Format the 64bits address
	for(i = 0; i < 8; i++)
	{
		//Debug_Output2(0, "Net Addr[%d] = 0x%X", i,loByte[i]);
		loData = ((loData<<8) & 0xFFFFFFFFFFFFFF00) + loByte[7-i];
	}

	////start_timer_isr();
	return loData;
}

//27-03-2013
//#define CURRENT_LSB_VALUE  78.13F //uA based on 0.020 ohm sense resistor
#define CURRENT_LSB_VALUE  104.2F //uA based on 0.015 ohm sense resistor
#define ACURRENT_LSB_VALUE  416.7F//uA based on 0.015 ohm sense resistor
//27-03-2013

//read the currecnt reg and calc the value in uA
float get_current_value()
{
	//uint8_t current_valD=0;
	//uint16_t current_valF=0;
	volatile float loCurrentVal;
	volatile signed int loCurrentRegVal = (signed int)coulomb_counter_read_current_reg();
	loCurrentVal = (loCurrentRegVal * CURRENT_LSB_VALUE);
   /* if(loCurrentVal<0)
    {
    	loCurrentVal=(-1*loCurrentVal);
    	current_valD=(uint8_t)(loCurrentVal);
    	current_valF=(uint16_t)((loCurrentVal-current_valD)*1000);
    	Debug_Output2(0,"Current:-%d.%02d",current_valD,current_valF);
    }
    else
    {
    	current_valD=(uint8_t)(loCurrentVal);
    	current_valF=(uint16_t)((loCurrentVal-current_valD)*1000);
    	Debug_Output2(0,"Current:%d.%02d",current_valD,current_valF);
    }*/
	return loCurrentVal;

}

//read the acc currecnt reg and calc the value in uA
float get_accumulated_current_value()
{
	//uint8_t current_valD=0;
	//uint16_t current_valF=0;
	volatile float loCurrentVal;
	volatile signed int loCurrentRegVal = (signed int)coulomb_counter_read_accumulated_current_reg();
	loCurrentVal = (loCurrentRegVal * ACURRENT_LSB_VALUE);
	/*if(loCurrentVal<0)
	{
		loCurrentVal=(-1*loCurrentVal);
		current_valD=(uint8_t)(loCurrentVal);
		current_valF=(uint16_t)((loCurrentVal-current_valD)*1000);
		Debug_Output2(0,"Acc:-%d.%02d",current_valD,current_valF);
	}
	else
	{
		current_valD=(uint8_t)(loCurrentVal);
		current_valF=(uint16_t)((loCurrentVal-current_valD)*1000);
		Debug_Output2(0,"Acc:%d.%02d",current_valD,current_valF);
	}*/
	return loCurrentVal;
}

#endif
