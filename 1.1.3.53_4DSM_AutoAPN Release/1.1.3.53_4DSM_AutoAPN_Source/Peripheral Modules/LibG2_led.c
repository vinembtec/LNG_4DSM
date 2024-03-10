//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_led.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_led_api
//! @{
//
//*****************************************************************************
#include "LibG2_led.h"

extern volatile uint8_t 	current_rate_index;
extern volatile uint32_t	parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];
extern uint8_t 				/*Active_LED,*/ LED_ON, LED_selected_in_config;//not used in this program //vinay
extern GPRSSystem      		glSystem_cfg;
extern uint8_t				HARDWARE_REVISION, REV4_LED_CONFIG,Front_Bi_Color;
extern uint8_t              grace_time_trigger[MSM_MAX_PARKING_BAYS];
extern volatile uint8_t		time_retain[MSM_MAX_PARKING_BAYS]; // to retain parking clock to become zero trenton //vinay
extern uint8_t				key_RL;
extern uint8_t 				LED_SPACE_REAR, LEFT_REAR, RIGHT_REAR, LED_SPACE_FRONT, LEFT_FRONT, RIGHT_FRONT;
/**************************************************************************/
//! Initialize enforcement LEDs
//! \param void
//! \return void
/**************************************************************************/
void init_Enf_LED_GPIO (void)
{
	if(HARDWARE_REVISION == LNG_REV3)
	{
		GPIO_setAsOutputPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
		GPIO_setAsOutputPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
		GPIO_setAsOutputPin(RGLED_PORT, RGLED_PIN);
		GPIO_setAsOutputPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
		GPIO_setAsOutputPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);

		 if(LED_selected_in_config == REAR_ENF_LED)
		{
			GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
			GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);	// Low selects rear LEDs

			GPIO_setOutputHighOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
			GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
			GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
		}
		else
		{
			GPIO_setOutputHighOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
			GPIO_setOutputHighOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);	// High selects front LEDs

			GPIO_setOutputLowOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
			GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
			GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
		}
	}
	else if(HARDWARE_REVISION == LNG_REV4)
	{
		GPIO_setAsOutputPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
		GPIO_setAsOutputPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
		GPIO_setAsOutputPin(RGLED_PORT, RGLED_PIN);
		GPIO_setAsOutputPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
		GPIO_setAsOutputPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);
		GPIO_setAsOutputPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
		GPIO_setAsOutputPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);

		if((LED_selected_in_config == REAR_ENF_LED) || (LED_selected_in_config == REAR_RED_ONLY_ENF_LED) || (LED_selected_in_config == REAR_GREEN_ONLY_ENF_LED))
		{
			GPIO_setOutputHighOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
			GPIO_setOutputLowOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);

			if(REV4_LED_CONFIG == true)
			{
				GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
				GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
				GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
				GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
				GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
			}
			else
			{
				P8OUT &= ~0x08;	//P8.3=DISP_LED_HILO_PORT //LOW
				P8OUT |= 0x20;	//P8.5=COMMS_LED_HILO_PORT //HIGH
				P5OUT |= 0x42;	//P5.1=EXCESS_LED_PORT_R4 & P5.6=PENALTY_LED_PORT //HIGH
				P7OUT |= 0x20;	//P7.5=RGLED_PORT //HIGH
			}
		}
		else if((LED_selected_in_config == FRONT_ENF_LED) || (LED_selected_in_config == FRONT_RED_ONLY_ENF_LED) || (LED_selected_in_config == FRONT_GREEN_ONLY_ENF_LED))
		{
			GPIO_setOutputHighOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);
			GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);

			GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
			GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
			GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
			GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
			GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
		}
	}

	GPIO_setAsOutputPin(DISPLAY_BKLT_PORT, DISPLAY_BKLT_PIN);
	BKLT_OFF();

	init_LEDs_PWM();
}

/**************************************************************************/
//! LED blink routine, selects appropriate GPIO for LEDs based on Main board
//! and Comms board revision and blinks as per selected configuration.
//! \param void
//! \return void
/**************************************************************************/
void init_LEDs_PWM()
{
	uint8_t led_number1 = 0, led_number2 = 0;

	if(HARDWARE_REVISION == LNG_REV4)//(REV4_LED_CONFIG == true)//it was skipping R3comms board //vinay
	{
		//Debug_TextOut(0,"LP2");
		if(glSystem_cfg.min_time_to_display > 0)
		{
			if(((parking_time_left_on_meter[1] > 0) && (time_retain[1]==1)) || (grace_time_trigger[1] == true))
			{
				//Debug_TextOut(0,"GREEN_LED2");
				led_number2 = GREEN_LED2;
			}
			else
			{
				//Debug_TextOut(0,"RED_LED2");
				led_number2 = RED_LED2;
			}

			//Debug_TextOut(0,"LP1");
			if(((parking_time_left_on_meter[0] > 0) && (time_retain[0]==1)) || (grace_time_trigger[0] == true))
			{
				//Debug_TextOut(0,"GREEN_LED1");
				led_number1 = GREEN_LED1;
			}
			else
			{
				//Debug_TextOut(0,"RED_LED1");
				led_number1 = RED_LED1;
			}
		}
		else
		{
			if((parking_time_left_on_meter[1] > 0) || (grace_time_trigger[1] == true))
			{
				//Debug_TextOut(0,"Init_GREEN_LED2");
				led_number2 = GREEN_LED2;
			}
			else
			{
				//Debug_TextOut(0,"Init_RED_LED2");
				led_number2 = RED_LED2;
			}

			//Debug_TextOut(0,"LP1");
			if((parking_time_left_on_meter[0] > 0) || (grace_time_trigger[0] == true))
			{
				//Debug_TextOut(0,"Init_GREEN_LED1");
				led_number1 = GREEN_LED1;
			}
			else
			{
				//Debug_TextOut(0,"Init_RED_LED1");
				led_number1 = RED_LED1;
			}
		}
	}
	else
	{
		if(glSystem_cfg.min_time_to_display > 0)
		{
		//Debug_TextOut(0,"LP3");
			if(((parking_time_left_on_meter[key_RL] > 0) && (time_retain[key_RL]==1))|| (grace_time_trigger[key_RL] == true))
			{
				//Debug_TextOut(0,"GREEN_LED");
				led_number1 = GREEN_LED;
			}
			else
			{
				//Debug_TextOut(0,"RED_LED");
				led_number1 = RED_LED;
			}
		}
		else
		{
			//Debug_TextOut(0,"LP2");
			if((parking_time_left_on_meter[key_RL] > 0) || (grace_time_trigger[key_RL] == true))
			{
				//Debug_TextOut(0,"GREEN_LED");
				led_number1 = GREEN_LED;
			}
			else
			{
				//Debug_TextOut(0,"RED_LED");
				led_number1 = RED_LED;
			}
		}
	}

	if(HARDWARE_REVISION == LNG_REV3)
	{
		//Debug_TextOut(0,"LED R3MB");
		if(led_number1 == GREEN_LED)
		{
			if((LED_selected_in_config == REAR_ENF_LED) || (LED_selected_in_config == REAR_RED_ONLY_ENF_LED) || (LED_selected_in_config == REAR_GREEN_ONLY_ENF_LED))
			{
				//Debug_TextOut(0,"inside 336");
				if(current_rate_index < 20)
				{
					if(LED_ON == TRUE)
					{
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
					}
					else
					{
						GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
						GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
					}
					GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
					GPIO_setOutputHighOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
				}
				else
				{
					GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
					GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
					GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
					GPIO_setOutputHighOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
				}
			}
			else if((LED_selected_in_config == FRONT_ENF_LED) || (LED_selected_in_config == FRONT_RED_ONLY_ENF_LED) || (LED_selected_in_config == FRONT_GREEN_ONLY_ENF_LED))
			{
				//Debug_TextOut(0,"inside 361");
				if((current_rate_index < 20) || (current_rate_index == PREPAY_PARKING))
				{
					if(LED_ON == TRUE)
					{
						GPIO_setOutputHighOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
						GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
					}
					else
					{
						GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
					}
					GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
					GPIO_setOutputLowOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
				}
				else
				{
					GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
					GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
					GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
					GPIO_setOutputLowOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
				}
			}
		}
		else if(led_number1 == RED_LED)//
		{
			if((LED_selected_in_config == REAR_ENF_LED) || (LED_selected_in_config == REAR_RED_ONLY_ENF_LED) || (LED_selected_in_config == REAR_GREEN_ONLY_ENF_LED))
			{
				//Debug_TextOut(0,"inside 390");
				if(current_rate_index < 20)
				{
					if(LED_ON == TRUE)
					{
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
					}
					else
					{
						GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
						GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
					}
					GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
					GPIO_setOutputHighOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
				}
				else
				{
					GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
					GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
					GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
					GPIO_setOutputHighOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
				}
			}
			else if((LED_selected_in_config == FRONT_ENF_LED) || (LED_selected_in_config == FRONT_RED_ONLY_ENF_LED) || (LED_selected_in_config == FRONT_GREEN_ONLY_ENF_LED))
			{
				//Debug_TextOut(0,"inside 415");
				if(current_rate_index < 20)
				{
					if(LED_ON == TRUE)
					{
						GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputHighOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
					}
					else
					{
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
					}
					GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
					GPIO_setOutputLowOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
				}
				else
				{
					GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
					GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
					GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
					GPIO_setOutputLowOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
				}
			}
		}
	}
	else if(HARDWARE_REVISION == LNG_REV4)
	{
		//Debug_TextOut(0,"LED R4MB");
		/*******************************RED1 START*********************************/
		if(led_number1 == RED_LED1)
		{
			//Debug_TextOut(0, "RED_LED1");
			if(((LED_selected_in_config == REAR_ENF_LED) || (LED_selected_in_config == REAR_RED_ONLY_ENF_LED)) && (LED_SPACE_REAR == LEFT_REAR))
			{
				GPIO_setOutputHighOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);	//Rear Leds ON
				GPIO_setOutputLowOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);		//Front Leds OFF
				//Debug_TextOut(0, "RED_LED1_LEFT_REAR");
				if(REV4_LED_CONFIG == true)
				{
//					GPIO_setOutputHighOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);	//Rear Leds ON
//					GPIO_setOutputLowOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);		//Front Leds OFF
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputHighOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
						}
						else
						{
							GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
						}
					}
					else
					{
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
						GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
					}
				}
				else if((REV4_LED_CONFIG != true) && (key_RL == 0)) //based on the space selected it will blink //vinay
				{
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						}
						else
						{
							GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						}
//						GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
//						GPIO_setOutputHighOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
					}
					else
					{
						GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputHighOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
					}
				}
			}
			else if(((LED_selected_in_config == FRONT_ENF_LED) || (LED_selected_in_config == FRONT_RED_ONLY_ENF_LED)) && (LED_SPACE_FRONT == LEFT_FRONT))
			{
				//Debug_TextOut(0, "RED_LED1_LEFT_FRONT");
				if(Front_Bi_Color == true)
				{
					GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
					GPIO_setOutputHighOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						}
						else
						{
							GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						}
					}
					else
					{
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
						GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
					}
				}
				else if((Front_Bi_Color != true) && (key_RL == 0)) //based on the space selected it will blink //vinay
				{
					GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);	//Rear Leds OFF
					GPIO_setOutputHighOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);		//Front Leds ON
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						}
						else
						{
							GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						}
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
					}
					else
					{
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
					}
				}
			}
		}
		/*******************************RED1 END*************************************/
		/*******************************GREEN1 START*********************************/
		else if((led_number1 == GREEN_LED1))
		{
			//Debug_TextOut(0, "GREEN_LED1");
			if(((LED_selected_in_config == REAR_ENF_LED) || (LED_selected_in_config == REAR_GREEN_ONLY_ENF_LED)) && (LED_SPACE_REAR == LEFT_REAR))
			{
				GPIO_setOutputHighOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);	//Rear Leds ON
				GPIO_setOutputLowOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);		//Front Leds OFF
				//Debug_TextOut(0, "GREEN_LED1_LEFT_REAR");
				if(REV4_LED_CONFIG == true)
				{
//					GPIO_setOutputHighOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);	//Rear Leds ON
//					GPIO_setOutputLowOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);		//Front Leds OFF
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
						}
						else
						{
							GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						}
					}
					else
					{
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
						GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
					}
				}
				else if((REV4_LED_CONFIG != true) && (key_RL == 0)) //based on the space selected it will blink //vinay
				{
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						}
						else
						{
							GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
						}
//						GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
//						GPIO_setOutputHighOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
					}
					else
					{
						GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputHighOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
					}
				}
			}
			else if(((LED_selected_in_config == FRONT_ENF_LED) || (LED_selected_in_config == FRONT_GREEN_ONLY_ENF_LED)) && (LED_SPACE_FRONT == LEFT_FRONT))
			{
				//Debug_TextOut(0, "GREEN_LED1_LEFT_FRONT");
				if(Front_Bi_Color == true)
				{
					GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
					GPIO_setOutputHighOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputHighOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
						}
						else
						{
							GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
						}
					}
					else
					{
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
						GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
					}
				}
				else if((Front_Bi_Color != true) && (key_RL == 0)) //based on the space selected it will blink //vinay
				{
					GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);	//Rear Leds OFF
					GPIO_setOutputHighOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);		//Front Leds ON
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
						}
						else
						{
							GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						}
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
					}
					else
					{
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
					}
				}
			}
		}
		/*******************************GREEN1 END*********************************/
		/*******************************RED2 START************************************/
		if((led_number2 == RED_LED2))
		{
			//Debug_TextOut(0, "RED_LED2");
			if(((LED_selected_in_config == REAR_ENF_LED) || (LED_selected_in_config == REAR_RED_ONLY_ENF_LED)) && (LED_SPACE_REAR == RIGHT_REAR))
			{
				GPIO_setOutputHighOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);	//Rear Leds ON
				GPIO_setOutputLowOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);		//Front Leds OFF
				//Debug_TextOut(0, "RED_LED2_RIGHT_REAR");
				if(REV4_LED_CONFIG == true)
				{
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						}
						else
						{
							GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						}
					}
					else
					{
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
						GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
					}
				}
				else if((REV4_LED_CONFIG != true) && (key_RL == 1)) //based on the space selected it will blink //vinay
				{
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						}
						else
						{
							GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						}
					}
					else
					{
						GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputHighOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
					}
				}
			}
			else if(((LED_selected_in_config == FRONT_ENF_LED) || (LED_selected_in_config == FRONT_RED_ONLY_ENF_LED)) && (LED_SPACE_FRONT == RIGHT_FRONT))
			{
				//Debug_TextOut(0, "RED_LED2_RIGHT_FRONT");
				if(Front_Bi_Color == true)
				{
					GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
					GPIO_setOutputHighOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
						}
						else
						{
							GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						}
					}
					else
					{
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
						GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
					}
				}
				else if((Front_Bi_Color != true) && (key_RL == 1)) //based on the space selected it will blink //vinay
				{
					GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);	//Rear Leds OFF
					GPIO_setOutputHighOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);		//Front Leds ON
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						}
						else
						{
							GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						}
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
					}
					else
					{
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
					}
				}
			}
		}
		/*******************************RED2 END*************************************/
		/*******************************GREEN2 START*********************************/
		else if((led_number2 == GREEN_LED2))
		{
			//Debug_TextOut(0, "GREEN_LED2");
			if(((LED_selected_in_config == REAR_ENF_LED) || (LED_selected_in_config == REAR_GREEN_ONLY_ENF_LED)) && (LED_SPACE_REAR == RIGHT_REAR))
			{
				GPIO_setOutputHighOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);	//Rear Leds ON
				GPIO_setOutputLowOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);		//Front Leds OFF
				//Debug_TextOut(0, "GREEN_LED2_RIGHT_REAR");
				if(REV4_LED_CONFIG == true)
				{
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputHighOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
						}
						else
						{
							GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
						}
					}
					else
					{
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
						GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
					}
				}
				else if((REV4_LED_CONFIG != true) && (key_RL == 1)) //based on the space selected it will blink //vinay
				{
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						}
						else
						{
							GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
						}
					}
					else
					{
						GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputHighOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputHighOnPin(EXCESS_LED_PORT, EXCESS_LED_PIN);
					}
				}
			}
			if(((LED_selected_in_config == FRONT_ENF_LED) || (LED_selected_in_config == FRONT_GREEN_ONLY_ENF_LED)) && (LED_SPACE_FRONT == RIGHT_FRONT))
			{
				//Debug_TextOut(0, "GREEN_LED2_RIGHT_FRONT");
				if(Front_Bi_Color == true)
				{
					GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);
					GPIO_setOutputHighOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputHighOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
						}
						else
						{
							GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
						}
					}
					else
					{
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputLowOnPin(LED_SWITCH_PORT, LED_SWITCH_PIN);
						GPIO_setOutputLowOnPin(DISP_LED_SWITCH_PORT, DISP_LED_SWITCH_PIN);
					}
				}
				else if((Front_Bi_Color != true) && (key_RL == 1)) //based on the space selected it will blink //vinay
				{
					GPIO_setOutputLowOnPin(COMMS_LED_HILO_PORT, COMMS_LED_HILO_PIN);	//Rear Leds OFF
					GPIO_setOutputHighOnPin(DISP_LED_HILO_PORT, DISP_LED_HILO_PIN);		//Front Leds ON
					if(current_rate_index < 20)
					{
						if(LED_ON == TRUE)
						{
							GPIO_setOutputHighOnPin(RGLED_PORT, RGLED_PIN);
						}
						else
						{
							GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						}
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
					}
					else
					{
						GPIO_setOutputLowOnPin(RGLED_PORT, RGLED_PIN);
						GPIO_setOutputLowOnPin(PENALTY_LED_PORT, PENALTY_LED_PIN);
						GPIO_setOutputLowOnPin(EXCESS_LED_PORT_R4, EXCESS_LED_PIN_R4);
					}
				}
			}
		}

		/*******************************GREEN2 END************************************/

	}
 }
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************


