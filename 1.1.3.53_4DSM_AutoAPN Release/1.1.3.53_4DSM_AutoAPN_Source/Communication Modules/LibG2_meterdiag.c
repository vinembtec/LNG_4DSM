//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_meterdiag.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_meterdiag_api
//! @{
//
//*****************************************************************************
#include "LibG2_meterdiag.h"

extern GPRSSystem      			glSystem_cfg;
extern uint32_t 				Tasks_Priority_Register;

//float 							normalizedADCRes[30];
uint16_t 						batt_vltg[MAX_DIAG_READINGS] = { 0 }; //, solar_voltage[MAX_DIAG_READINGS] = { 0 };
uint16_t 						last_minimum_vltg=9999;
uint8_t 						batt_voltage_read_count = 0;
int8_t 							tempC[MAX_DIAG_READINGS] = { 0 };  //to get negative value.
//uint8_t							batt_threshold_level[5]; //not using in this program //vinay
uint8_t 						read_count = 0;
uint16_t 						PreviousValue = 9999;
uint8_t         	   			Low_Battery_count = 0, Low_Battery_count_H = 0, Low_Battery_count_L = 0;

/**************************************************************************/
//! Reads the Rechargeable battery Voltage
//! \param void
//! \return uint16_t Battery Voltage
/**************************************************************************/
/*Battery voltage*/
uint16_t Rechargeable_battery_read(void)
{
	//uint32_t i;
	uint16_t vltg = 0;
	Adc_Vltg_init();
	init_rchbattery_GPIO();
	ADC14_configureConversionMemory(ADC_MEM0,ADC_VREFPOS_INTBUF_VREFNEG_VSS,
			ADC_INPUT_A1, false);
	/* Enabling/Toggling Conversion */
	ADC14_enableConversion();
	ADC14_toggleConversionTrigger();
	//i=0;
	//while(i<=12)
	{
		uint64_t status = ADC14_getEnabledInterruptStatus();
		ADC14_clearInterruptFlag(status);

		if (ADC_INT0 & status)
		{
			//vltg[i] = ADC14_getResult(ADC_MEM0);
			vltg =(((ADC14_getResult(ADC_MEM0) *2.5f) / 16384)*2)*100;
			//i++;
		}
	}
	ADC14_disableConversion();
	ADC14_disableInterrupt(ADC_INT0);
	GPIO_setOutputLowOnPin(SENSOR_CNTRL1_PORT, SENSOR_CNTRL1_PIN);
	return vltg;
}

#if	0
/*Solar-1 voltage*/
uint16_t Solar_voltage_read(void)
{
	uint16_t vltg = 0;

	Adc_Vltg_init();
	init_solarbattery_GPIO();
	ADC14_configureConversionMemory(ADC_MEM0,ADC_VREFPOS_INTBUF_VREFNEG_VSS,
			ADC_INPUT_A0, false);
	/* Enabling/Toggling Conversion */
	ADC14_enableConversion();
	ADC14_toggleConversionTrigger();
	//i=0;
	//while(i<=12)
	{
		uint64_t status = ADC14_getEnabledInterruptStatus();
		ADC14_clearInterruptFlag(status);

		if (ADC_INT0 & status)
		{
			//vltg[i] = ADC14_getResult(ADC_MEM0)*100;
			vltg =(((ADC14_getResult(ADC_MEM0) *2.5f) / 16384)*2)*100;
			//i++;
		}
	}
	ADC14_disableConversion();
	ADC14_disableInterrupt(ADC_INT0);
	GPIO_setOutputLowOnPin(SENSOR_CNTRL1_PORT, SENSOR_CNTRL1_PIN);
	return (vltg);
}
#endif

/**************************************************************************/
//! Reads the Temperature
//! \param void
//! \return int8_t Temperature
/**************************************************************************/
int8_t Temperature_read()
{
	volatile float temp;
	int8_t ltempC =0;
	uint32_t cal30, cal85;
	/* Setting reference voltage to 2.5 and enabling temperature sensor */
	REF_A_setReferenceVoltage(REF_A_VREF2_5V);
	REF_A_enableReferenceVoltage();
	REF_A_enableTempSensor();

	/* Initializing ADC (MCLK/1/1) with temperature sensor routed */
	ADC14_enableModule();
	ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1,
			ADC_TEMPSENSEMAP);
	/* Configuring ADC Memory (ADC_MEM0 A22 (Temperature Sensor) in repeat
	 * mode).
	 */
	ADC14_configureSingleSampleMode(ADC_MEM0, true);
	ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_INTBUF_VREFNEG_VSS ,
			ADC_INPUT_A22, false);
	/* Configuring the sample/hold time for TBD */
	ADC14_setSampleHoldTime(ADC_PULSE_WIDTH_192,ADC_PULSE_WIDTH_192);
	/* Enabling sample timer in auto iteration mode and interrupts*/
	ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
	ADC14_enableInterrupt(ADC_INT0);
	/* Triggering the start of the sample */
	ADC14_enableConversion();
	ADC14_toggleConversionTrigger();
	/* Going to sleep */
	int8_t i=0;
	while (i++<=12)
	{
		uint64_t status = ADC14_getEnabledInterruptStatus();
		ADC14_clearInterruptFlag(status);

		if(status & ADC_INT0)
		{
			cal30 = SysCtl_getTempCalibrationConstant(SYSCTL_2_5V_REF,
					SYSCTL_30_DEGREES_C);
			cal85 = SysCtl_getTempCalibrationConstant(SYSCTL_2_5V_REF,
					SYSCTL_85_DEGREES_C);

			temp = (float)((int32_t)(ADC14_getResult(ADC_MEM0) - cal30) * (85 - 30))
			                                / (cal85 - cal30) + 30.0f;
			ltempC = temp;
			//tempF = tempC * 9.0f / 5.0f + 32.0f;
		}
	}
	ADC14_disableConversion();
	ADC14_disableInterrupt(ADC_INT0);
	return ltempC;
}

/**************************************************************************/
//! Initializes the MSP432 ADC module which is used to read battery voltage
//! \param void
//! \return void
/**************************************************************************/
void Adc_Vltg_init(void)
{
	REF_A_disableReferenceVoltageOutput();
	REF_A_enableReferenceVoltage();
	REF_A_setReferenceVoltage(REF_A_VREF2_5V);
	ADC14_setResolution(ADC_14BIT );

	/* Initializing ADC (SMCLK) */
	ADC14_enableModule();
	ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1,0);

	/* Configuring ADC Memory */
	ADC14_enableReferenceBurst();
	ADC14_configureSingleSampleMode(ADC_MEM0, true);

	/* Configuring Sample Timer */
	ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

	/* Enabling interrupts */
	ADC14_enableInterrupt(ADC_INT0);
	//Interrupt_enableInterrupt(INT_ADC14);
	//Interrupt_enableMaster();
}

/**************************************************************************/
//! Reads the Rechargeable battery Voltage, Temperature and populates the
//! diagnostic buffer, It also generates Low battery events by comparing the
//! read battery voltage with the threshold set in meter's configuration file
//! \param void
//! \return void
/**************************************************************************/
void get_battery_voltage_temp()
{
	uint8_t i=0, j=0;
	uint8_t temp =0;
	batt_vltg[batt_voltage_read_count] = Rechargeable_battery_read();
	Debug_Output1( 0, "Rch Batt V=%d", batt_vltg[batt_voltage_read_count] );
	if((batt_vltg[batt_voltage_read_count] < last_minimum_vltg) && (batt_vltg[batt_voltage_read_count] != 0))
		last_minimum_vltg = batt_vltg[batt_voltage_read_count];

	if ( (batt_vltg[batt_voltage_read_count] <= (glSystem_cfg.Low_Battery_Threshold+20)) && ((batt_vltg[batt_voltage_read_count] < PreviousValue)) )//Threshold-level
	{
		if(read_count++ == 3)
		{
			//LNGSIT-563,731
			 if((Low_Battery_count_H != 255) && (batt_vltg[batt_voltage_read_count] < (glSystem_cfg.Low_Battery_Threshold+20)) && (batt_vltg[batt_voltage_read_count] >= (glSystem_cfg.Low_Battery_Threshold)))
			 {
				 //Debug_TextOut(0,"LOW_BATT_LEVEL1");
				 push_event_to_cache( UDP_EVTTYP_LOW_BATT_LEVEL1);
				 Tasks_Priority_Register |= SEVERE_EVENTS_TASK;

				 Low_Battery_count_H = 255;
			 }
			 else if((Low_Battery_count != 255) && (batt_vltg[batt_voltage_read_count] < glSystem_cfg.Low_Battery_Threshold) && (batt_vltg[batt_voltage_read_count] > (glSystem_cfg.Low_Battery_Threshold-20)))
			 {
				 //Debug_TextOut(0,"LOW_BATT_LEVEL2");
				 push_event_to_cache( UDP_EVTTYP_LOW_BATT_LEVEL2);
				 Tasks_Priority_Register |= SEVERE_EVENTS_TASK;

				 Low_Battery_count = 255;
			 }
			 else if((Low_Battery_count_L != 255) && (batt_vltg[batt_voltage_read_count] < (glSystem_cfg.Low_Battery_Threshold-20)))
			 {
				 //Debug_TextOut(0,"LOW_BATT_POWER_SAVE");
				 push_event_to_cache( UDP_EVTTYP_LOW_BATT_POWER_SAVE);
				 Tasks_Priority_Register |= SEVERE_EVENTS_TASK;

				 Low_Battery_count_L = 255;
			 }
			read_count = 0;
			PreviousValue = batt_vltg[batt_voltage_read_count];
		}
	}
	else
		read_count = 0;                                   //to get exact 5 itteration.

	/*solar_voltage[batt_voltage_read_count] = Solar_voltage_read();
	Debug_Output1( 0, "Solar V=%d", solar_voltage[batt_voltage_read_count] );*/
	tempC[batt_voltage_read_count] = Temperature_read();
	Debug_Output1( 0, "Temperature=%d", tempC[batt_voltage_read_count] );

	batt_voltage_read_count++;
	if(batt_voltage_read_count >= MAX_DIAG_READINGS)
		batt_voltage_read_count = 0;

	for(j=0;j<MAX_DIAG_READINGS;j++)
	{
		for (i=0;i<(MAX_DIAG_READINGS-1);i++)
		{
			if (tempC[i]>tempC[i+1])
			{
				temp=tempC[i+1];
				tempC[i+1]=tempC[i];
				tempC[i]=temp;
			}
		}
	}

	if(Low_Battery_count_H != 255)
		Low_Battery_count_H = 0;

	if(Low_Battery_count_L != 255)
		Low_Battery_count_L = 0;

	if(Low_Battery_count != 255)
		Low_Battery_count = 0;
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
