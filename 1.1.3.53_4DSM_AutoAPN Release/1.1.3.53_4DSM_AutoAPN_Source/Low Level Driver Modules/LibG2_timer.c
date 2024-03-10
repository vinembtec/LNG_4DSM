//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_timer.c
//
//****************************************************************************

#include "LibG2_timer.h"

//uint8_t 						coinwakeup_timer_running = false,//not used in this program //vinay
//								intervals_timer_running = false;//not used in this program //vinay
extern uint8_t 							RespIndex;
void DelayMs(uint32_t count)
{
	volatile uint32_t _k, _l;

	for(_l = 0; _l < count; _l++)
	{
		for(_k = 0; _k < 360; _k++)
		{
			continue;
		}
	}
	return;
}


void DelayMs_1(uint32_t count)
{
	volatile uint32_t _k, _l;

	for(_l = 0; _l < count; _l++)
	{
		for(_k = 0; _k < 360; _k++)
		{
			if (0 == GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN))
			{
				return;
			}
			continue;
		}
	}
	return;
}

void DelayMs_2(uint32_t count)
{
	volatile uint32_t _k, _l;

	for(_l = 0; _l < count; _l++)
	{
		for(_k = 0; _k < 360; _k++)
		{
			if (6 == RespIndex)
			{
				return;
			}
			continue;
		}
	}
	return;
}

void DelayUs(uint32_t count)
{
	volatile uint32_t _k, _l;

	for(_l = 0; _l < (count/3); _l++)
	{

	}
	return;
}
#if 0
/******************************************************************************/
/*  Name        : start_coinwakeup_timer                                      */
/*  Parameters  : void                                                        */
/*  Returns     : void                                                        */
/*  Function    : Timer A0CCR1 register configuration and start timeout timer */
/*----------------------------------------------------------------------------*/
void start_coinwakeup_timer()
{
	//return ;
	TA0CTL=0x0100;//0X01C0;//independent latch,16 bit,ACLK,divider=8,timer halted,TAIE disabled
	TA0R=0X0000;
	TA0CCR0= DEFAULT_COIN_WAKEUP_TIMER;// 10ms INTERVAL
	TA0CCTL0=0X0000;//synchronous,compare mode ,CCIE disabled

	TA0CTL &= ~BIT1;
	TA0CCTL0 |= CCIE;    // enable CCR1 interrupt CCIE
	TA0CTL |= BIT4;     // now start in up mode
	//coinwakeup_timer_running=true;
	NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);
	//__enable_interrupt();
}

/**************************************************************************/
/*  Name        : cancel coin wake up timer                               */
/*  Parameters  : void                                                    */
/*  Returns     : void                                                    */
/*  Function    : Stop coin wake up timer                                 */
/*------------------------------------------------------------------------*/
void cancel_coinwakeup_timer()
{
	//return ;
	TA0CTL=0X01C0;
	TA0CCTL0=0X0000;
	//coinwakeup_timer_running = false;
}
#endif
void start_sensor_counter()
{
	TA0CTL |= BIT_5; //continuous mode
}

uint16_t read_sensor_counter()
{
	return((uint16_t)TA0CCR3);
}

//uint16_t read_coin_sample_counter() //not used //vinay
//{
//	return((uint16_t)TA0R);
//}

/**************************************************************************/
/*  Name        : reset_sensor_counter                                    */
/*  Parameters  : void                                                    */
/*  Returns     : void                                                    */
/*  Function    : Reset Timer AxCCIn                                      */
/*------------------------------------------------------------------------*/
void reset_sensor_counter()
{
	//TA0CTL &= ~(BIT_5);
	TA0R=0XFFFF;
	TA0CCR3=0XFFFF;
}

/**************************************************************************/
/*  Name        : cancel_sensor_counter                                   */
/*  Parameters  : void                                                    */
/*  Returns     : void                                                    */
/*  Function    : Stop Timer AxCCIn                                       */
/*------------------------------------------------------------------------*/
void cancel_sensor_counter()
{
	TA0CCTL3 = 0x0000;
	TA0CTL=0X01C0;
	TA0CCTL0=0X0000;
}

//initialize coin track module
/**************************************************************************/
/*  Name        : init_sensor_counter_12mhz                               */
/*  Parameters  : void                                                    */
/*  Returns     : void                                                    */
/*  Function    : Init Timer AxCCIn register configuration counter 12MHz  */
/*------------------------------------------------------------------------*/
void init_sensor_counter_12mhz()
{
	//First make sure we cancel it
	cancel_sensor_counter();
	//TA0CTL=0X0000;//independent latch,16 bit,TAxCLK,divider=1, stop mode,TAIE disabled
	TA0CTL = TASSEL_2 + MC_2 + ID_0;  //TASSEL_1 = ACLK (32.768KHz) ,TASSEL_2 = SMCLK (12MHz)
	TA0R=0XFFFF;
	TA0CCR3=0XFFFF;
	TA0CCTL3= CM_1 + SCS + CCIS_0 + CAP;// + CCIE; //0X4100;//CM=01 (capture on rising edge), CCIS = 00 (CCI3A), SCS=0 (Asyn), SCCI = 0, CAP = 1 (cpature mode), OUTMOD = 000, CCIE = 0, CCI = 0, OUT = 0
	//And start it as well
	start_sensor_counter();//no need to start; keep it configured and leave, we can start and stop when needed
}

void init_coin_sample_counter_12mhz()
{
	//First make sure we cancel it
	cancel_sensor_counter();
	//TA0CTL=0X0000;//independent latch,16 bit,TAxCLK,divider=1, stop mode,TAIE disabled
	TA0CTL = TASSEL_2 + MC_2 + ID_0;  //TASSEL_1 = ACLK (32.768KHz) ,TASSEL_2 = SMCLK (12MHz)
	TA0R=0XFFFF;
	TA0CCR3=0XFFFF;
	TA0CCTL3= 0;//0x4020;//CM_0 + SCS + CCIS_0;// + CCIE; //0X4100;//CM=01 (capture on rising edge), CCIS = 00 (CCI3A), SCS=0 (Asyn), SCCI = 0, CAP = 1 (cpature mode), OUTMOD = 000, CCIE = 0, CCI = 0, OUT = 0
	//And start it as well
	start_sensor_counter();//no need to start; keep it configured and leave, we can start and stop when needed
}

//initialize coin track module
/**************************************************************************/
/*  Name        : init_sensor_counter                                     */
/*  Parameters  : void                                                    */
/*  Returns     : void                                                    */
/*  Function    : Init Timer AxCCIn register configuration counter 32KHz  */
/*------------------------------------------------------------------------*/
//void init_sensor_counter_32khz() //not used //vinay
//{
//	//First make sure we cancel it
//	cancel_sensor_counter();
//	//TA0CTL=0X0000;//independent latch,16 bit,TAxCLK,divider=1, stop mode,TAIE disabled
//	TA0CTL = TASSEL_1 + MC_2 + ID_0;  //TASSEL_1 = ACLK (32.768KHz) ,TASSEL_2 = SMCLK (16MHz). We need 8MHz
//	TA0R=0XFFFF;
//	TA0CCR3=0XFFFF;
//	TA0CCTL3= CM_1 + SCS + CCIS_0 + CAP;// + CCIE; //0X4100;//CM=01 (capture on rising edge), CCIS = 00 (CCI3A), SCS=0 (Asyn), SCCI = 0, CAP = 1 (cpature mode), OUTMOD = 000, CCIE = 0, CCI = 0, OUT = 0
//	//And start it as well
//	start_sensor_counter();//no need to start; keep it configured and leave, we can start and stop when needed
//}

