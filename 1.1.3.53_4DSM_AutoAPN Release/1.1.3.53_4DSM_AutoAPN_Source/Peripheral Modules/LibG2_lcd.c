//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_lcd.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_lcd_api
//! @{
//
//*****************************************************************************
#include "LibG2_lcd.h"

//***************************************************************************************
//  SHARP LS013B4DN04 Memory Display
//
//  Simple library for TI MSP430G2 LaunchPad to write characters to SHARP LS013B4DN04
//  Memory Display using hardware SPI.
//
//  This code also works with the predecessor LS013B4DN02 and should work with any
//  display of that series with compatible pinouts.
//
//  ACLK = n/a, MCLK = SMCLK = default DCO. Note that display specifies 1MHz max for SCLK
//
//                MSP430G2553
//             -----------------
//            |             P1.0|-->LED  (VCOM status display)
//			  |                 |
//            |             P1.5|-->SCLK (SPI clock)
//			  |                 |
//            |             P1.7|-->SI   (SPI data)
//			  |                 |
//            |             P2.0|-->DISP (display on/off)
//			  |                 |
//            |             P2.5|-->SCS  (SPI chip select)
//
//  Display VDD and VDDA connected to LaunchPad VCC
//  Display GND connected to LaunchPad GND
//
//  LS013B4DN02 is specified for 5V VDD/VDDA but seems to work well with 3V as provided by
//  the LaunchPad. LS013B4DN04 specifies VDD/VDDA at 3V.
//
//  This code works with the 43oh SHARP Memory LCD BoosterPack
//  http://forum.43oh.com/topic/4979-sharp-memory-display-booster-pack/
//
//  Adrian Studer
//  March 2014
//***************************************************************************************

#include "Fonts/Liberty_Font_16x24.h"
#include "Fonts/Liberty_Font_32x48.h"
#include "Fonts/Liberty_Font_48x72.h"
#include "Fonts/Liberty_GIF_Card.h"
#include "Fonts/Liberty_GIF_CardCoin.h"
#include "Fonts/Liberty_GIF_FullCoin.h"
#include "Fonts/Liberty_GIF_HourGlass.h"
#include "Fonts/Liberty_GIF_InsertCard.h"
#include "Fonts/Liberty_GIF_NoParking.h"
#include "Fonts/Liberty_GIF_RemoveCard.h"

#define BayAdjustValue		0//557     			//to adjust bay value

extern RTC_C_Calendar 		rtc_c_calender;
extern volatile uint32_t	parking_time_left_on_meter[MSM_MAX_PARKING_BAYS] ;
extern volatile uint32_t	negetive_parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];
extern volatile uint16_t 	minutes_left_in_current_schedule,minutes_left_in_next_schedule;
extern GPRSSystem      		glSystem_cfg;
extern volatile uint8_t 	current_rate_index,Max_Amount_In_current_rate;
extern uint8_t				ccard_dataread_flag, card_removed_flag, check_card_read, smart_card_mode;
extern uint8_t				Tech_menu;
extern uint8_t         		glIn_diag_mode;
extern uint8_t              meter_out_of_service;
extern uint32_t  			Max_parking_time;
//extern uint8_t				Meter_Reset;
extern uint8_t				gl_Commission_Card;
extern TSmart_card 			smart_card;
extern uint32_t				max_time_in_current_rate, CC_TimeOut_RTCTimestamp;
extern uint8_t              do_CC_OLT;
extern uint8_t     			Tech_key_seq;
extern uint8_t				sch_message_index_1, sch_message_index_2, sch_message_index_3, sch_message_index_4;
extern uint8_t     			rx_ans[ GPRS_GEN_BUF_SZ_SMALL ];
//extern Maintenance_Evt		Maintenance_Log;//not used in this program //vinay
extern MBInfo  				glMB_info;
extern uint8_t				gAutoCalibration;
extern uint16_t				occupancygl;
//extern RTC_ParkingClk		RTC_ParkingTime;
extern uint8_t				rtc_hour_flag[24], gl_RTCHOUR;
extern uint8_t				OPTICAL_COIN_JAM_RAISED, WAKEUP_COIL_JAM_RAISED, CARD_JAM_RAISED;
extern uint8_t              grace_time_trigger[MSM_MAX_PARKING_BAYS];
//extern uint8_t              grace_timeout;//not used in this program //vinay
//extern uint8_t				SMS_Print;
extern uint8_t				holiday_spl_event_found;
extern uint8_t				ANTI_FEED_in_effect[MSM_MAX_PARKING_BAYS], this_is_a_CC;

extern uint8_t 				key_press;
//extern uint16_t         	glTotal_Valid_Coin_Count,glTotal_Coin_Count;//not used in this program //vinay
//extern uint16_t				coin_type0;//not used in this program //vinay
//extern uint16_t				coin_type1;//not used in this program //vinay
//extern uint16_t				coin_type2;//not used in this program //vinay
//extern uint16_t				coin_type3;//not used in this program //vinay
//extern uint32_t             total_coin_values;//not used in this program //vinay

volatile uint8_t 			VCOM;			// current state of VCOM (0x04 or 0x00)
uint8_t 					LineBuff[PIXELS_X/8];		// line buffer
uint16_t 					bits_count = 0;
uint8_t                     Flash_Operation = 0, CC_Auth_Entered = FALSE;//NONE;

//uint8_t 					TextBuff[9];	// buffer to build line of text//not used in this program //vinay
uint8_t 					RTC_String[MAXBYTESINROW+1];
uint32_t 					BKLT_TimeOut_RTCTimestamp = 0;
uint8_t						LCD_BKLT_ON = FALSE, Meter_Full_Flg[MSM_MAX_PARKING_BAYS] = {0}, do_not_allow_cc_auth = FALSE;  //to use it for dual space//vinay
uint8_t 					calibration_display_buf[40]={0};
uint8_t						Screen_Clear;
uint16_t 					Current_Space_Id = 0;

//Temp Variables
uint8_t 					hours_to_display = 0;
uint8_t 					minutes_to_display = 0;
uint8_t 					hours_to_display_L = 0;
uint8_t 					minutes_to_display_L = 0;
uint8_t 					hours_to_display_R = 0;
uint8_t 					minutes_to_display_R = 0;
uint16_t 					temp_minutes_to_disp = 0;
uint8_t 					parking_clock[15] = { 0 };

uint8_t                     /*current_bay_index = 3,*/payment_on_hold = false; //it is not used in this program //vinay

uint16_t 					bay_number_to_display = 0;

extern uint32_t 			Last_Cash_Payment_RTC;

//uint8_t		  				Display_Surcharges = FALSE; //wait till config changes and then delete it and uncomment from config.h vinay
extern uint16_t 			min_time_insec; 	//vinay //make it check it from config Trenton
extern volatile uint8_t		time_retain[MSM_MAX_PARKING_BAYS]; //vinay // to retain parking clock to become zero trenton
extern uint32_t 			parking_time_left_on_meter_prev[MSM_MAX_PARKING_BAYS]; //to round off //vinay

extern uint32_t 			earned_time_in_seconds;
uint32_t 					earned_time_in_seconds_temp = 0;

uint8_t						key_RL = 0;		//vinay to credit coin value time to last selected space

extern uint8_t 				Payment_card;   //To avoid surcharges for payment cards. //vinay
extern uint8_t 				cardread;

//extern uint16_t 			max_sens;
//extern uint16_t 			min_sens;
//extern uint8_t 				ratio_1;
//extern uint8_t 				ratio_2;
extern uint32_t 			coindrop_TimeOut_RTC;
extern uint32_t 			KeyPress_TimeOut_RTC;
uint32_t 					Screen_TimeOut_RTC = 0;
uint8_t 					Meter_Full[MSM_MAX_PARKING_BAYS] = {0};  //to use it for dual space//vinay
extern volatile uint8_t     No_Modem_Coin_Only;

void lcd_vcom_pat()
{
	// put display into low-power static mode
	VCOM ^= MLCD_VCOM;
	GPIO_setOutputHighOnPin(DISPLAY_SPI_CS_IOPORT,DISPLAY_SPI_CS_PIN);
	DelayUs(3);// SCS high, ready talking to display
	SPIWriteByte(MLCD_SM | VCOM);					// send static mode command
	SPIWriteByte(0);								// send command trailer
	GPIO_setOutputLowOnPin(DISPLAY_SPI_CS_IOPORT,DISPLAY_SPI_CS_PIN);
	// SCS lo, finished talking to display
}

// write a string to display, truncated after 50 characters
// input: text		0-terminated string
//        line		vertical position of text, 0-240
//        options	can be combined using OR
//					DISP_INVERT	inverted text
//					DISP_WIDE double-width text (except for SPACE)
//					DISP_HIGH double-height text
//FONT:16X24

void printSharp16x24(const uint8_t* text, uint8_t line, uint8_t column, uint8_t options)
{
	// c = char
	// b = bitmap
	// i = text index
	// j = line buffer index
	// k = char line
	uint8_t c, b, i, j, k, loop;

	// rendering happens line-by-line because this display can only be written by line
	k = 0;
	//idx = 0;
	while((k < 24) && (line < PIXELS_Y))// loop for 24 character lines while within display
	{
		i = 0;
		j = 0;

		if((column > 0) && (column < MAXBYTESINROW))
		{
			for(loop = 0;loop < column;loop++)
			{
				b = 0;
				// pixels are LOW active
				b = ~b;
				LineBuff[j] = b;						// store pixels in line buffer
				j++;									// we've written one byte to buffer

			}
		}
		while(j < (PIXELS_X/8) && (c = text[i]) != 0)	// while we did not reach end of line or string
		{
			if(c < ' ' || c > '~')						// invalid characters are replace with SPACE
			{
				c = ' ';
			}

			c = c - 32;									// convert character to index in font table
			for(loop = 0;loop < 2;loop++)//width of each charecter
			{
				b = font16x24[(c)][loop+(k*2)];						// retrieve byte defining one line of character

				if(!(options & DISP_INVERT))				// invert bits if DISP_INVERT is _NOT_ selected
				{											// pixels are LOW active
					b = ~b;
				}

				if((options & DISP_WIDE) && (c != 0))		// double width rendering if DISP_WIDE and character is not SPACE
				{
					//doubleWideAsm(b, &LineBuff[j]);			// implemented in assembly for efficiency/space reasons
					LineBuff[j] = b;
					LineBuff[j+1] = b;

					j += 2;									// we've written two bytes to buffer
				}
				else										// else regular rendering
				{
					LineBuff[j] = b;						// store pixels in line buffer
					j++;									// we've written one byte to buffer
				}
			}
			i++;										// next character
		}

		while(j < (PIXELS_X/8))							// pad line for empty characters
		{
			LineBuff[j] = 0xff;
			j++;
		}

		SPIWriteLine(line++);							// write line buffer

		if(options & DISP_HIGH && line < PIXELS_Y)	// repeat line if DISP_HIGH is selected
		{
			SPIWriteLine(line++);
		}

		k++;											// next pixel line
	}
}

//FONT:32X48
void printSharp32x48(const uint8_t* text, uint8_t line, uint8_t column, uint8_t options)
{
	// c = char
	// b = bitmap
	// i = text index
	// j = line buffer index
	// k = char line
	uint8_t c, b, i, j, k, loop;

	// rendering happens line-by-line because this display can only be written by line
	k = 0;
	//idx = 0;
	while((k < 48) && (line < PIXELS_Y))// loop for 48 character lines while within display
	{
		i = 0;
		j = 0;
		//idx = 0;
		if((column > 0) && (column < MAXBYTESINROW))
		{
			for(loop = 0;loop < column;loop++)
			{
				b = 0;

				{											// pixels are LOW active
					b = ~b;
				}

				{
					LineBuff[j] = b;						// store pixels in line buffer
					j++;									// we've written one byte to buffer
				}
			}
		}
		while(j < (PIXELS_X/8) && (c = text[i]) != 0)	// while we did not reach end of line or string
		{
			if((c < '.') || (c > '['))						// invalid characters are replace with SPACE
			{
				c = '.';
			}

			c = c - '.';									// convert character to index in font table
			for(loop = 0;loop < 4;loop++)//width of each charecter
			{
				if(text[i] != ' ')
				{
					b = font32x48[(c)][loop+(k*4)];						// retrieve byte defining one line of character

					if(!(options & DISP_INVERT))				// invert bits if DISP_INVERT is _NOT_ selected
					{											// pixels are LOW active
						b = ~b;
					}

					if((options & DISP_WIDE) && (c != 0))		// double width rendering if DISP_WIDE and character is not SPACE
					{
						//doubleWideAsm(b, &LineBuff[j]);			// implemented in assembly for efficiency/space reasons
						LineBuff[j] = b;
						LineBuff[j+1] = b;

						j += 2;									// we've written two bytes to buffer
					}
					else										// else regular rendering
					{
						LineBuff[j] = b;						// store pixels in line buffer
						j++;									// we've written one byte to buffer
					}
				}
				else//this is to support space in 32x48 font
				{
					LineBuff[j] = 0XFF;						// store pixels in line buffer
					j++;
				}
			}
			i++;										// next character
		}

		while(j < (PIXELS_X/8))							// pad line for empty characters
		{
			LineBuff[j] = 0xff;
			j++;
		}

		SPIWriteLine(line++);							// write line buffer

		if(options & DISP_HIGH && line < PIXELS_Y)	// repeat line if DISP_HIGH is selected
		{
			SPIWriteLine(line++);
		}

		k++;											// next pixel line
	}
}

void printSharp32x48HfScr(const uint8_t* text, uint8_t line, uint8_t column, uint8_t options)
{
	// c = char
		// b = bitmap
		// i = text index
		// j = line buffer index
		// k = char line
		uint8_t c, b, i, j, k, loop;

		// rendering happens line-by-line because this display can only be written by line
		k = 0;
		//idx = 0;
		while((k < 48) && (line < PIXELS_Y))// loop for 48 character lines while within display
		{
			i = 0;
			j = 0;
			//idx = 0;
			if((column > 0) && (column < MAXBYTESINROW))
			{
				for(loop = 0;loop < column;loop++)
				{
					b = 0;

					{											// pixels are LOW active
						b = ~b;
					}

					{
						LineBuff[j] = b;						// store pixels in line buffer
						j++;									// we've written one byte to buffer
					}
				}
			}

		while(j < (PIXELS_X/8) && (c = text[i]) != 0)	// while we did not reach end of line or string
		{
			if((c < '.') || (c > '['))						// invalid characters are replace with SPACE
			{
				c = '.';
			}

			c = c - '.';									// convert character to index in font table

			 if(i<=5)									    // Half screen Divide screen
			 {
				 for(loop = 0;loop < 4;loop++)//width of each charecter
				 {
					 if(text[i] != ' ')
					 {
						 b = font32x48[(c)][loop+(k*4)];						// retrieve byte defining one line of character

						 if((options != DISP_INVERT_LEFT))				// invert bits if DISP_INVERT is _NOT_ selected
						 {											// pixels are LOW active
							 b = ~b;
						 }
						 if((options == DISP_WIDE) && (c != 0))		// double width rendering if DISP_WIDE and character is not SPACE
						 {
							 //doubleWideAsm(b, &LineBuff[j]);			// implemented in assembly for efficiency/space reasons
							 LineBuff[j] = b;
							 LineBuff[j+1] = b;
							 j += 2;									// we've written two bytes to buffer
						 }
						 else 										// else regular rendering
						 {
							 LineBuff[j] = b;						// store pixels in line buffer
							 j++;									// we've written one byte to buffer
						 }
					 }
					 else//this is to support space in 32x48 font
					 {
						 LineBuff[j] = 0XFF;						// store pixels in line buffer
						 j++;
					 }

				 }
			 }

			 else if(i>=6)
			 {
				for(loop = 0;loop < 4;loop++)//width of each charecter
				{
					if(text[i] != ' ')
					{
						b = font32x48[(c)][loop+(k*4)];			    // retrieve byte defining one line of character

						if(options != DISP_INVERT_RIGHT)			// invert bits if DISP_INVERT is _NOT_ selected
						{											// pixels are LOW active
							b = ~b;
						}
						if((options == DISP_WIDE) && (c != 0))		// double width rendering if DISP_WIDE and character is not SPACE
						{
							//doubleWideAsm(b, &LineBuff[j]);			// implemented in assembly for efficiency/space reasons
							LineBuff[j] = b;
							LineBuff[j+1] = b;
							j += 2;									// we've written two bytes to buffer
						}
						else   									   // else regular rendering
						{
							LineBuff[j] = b;						// store pixels in line buffer
							j++;									// we've written one byte to buffer
						}
					}
					else//this is to support space in 32x48 font
					{
						LineBuff[j] = 0XFF;						// store pixels in line buffer
						j++;
					}
				}
			  }
				i++;										// next character
			}


		while(j < (PIXELS_X/8))							// pad line for empty characters
		{
			LineBuff[j] = 0xff;
			j++;
		}

		SPIWriteLine(line++);							// write line buffer

		if((options == DISP_HIGH) && (line < PIXELS_Y))	// repeat line if DISP_HIGH is selected
		{
			SPIWriteLine(line++);
		}

		k++;											// next pixel line
	}
}

//#if	0 //vinay code opt
////FONT:48x72
//void printSharp48x72(const uint8_t* text, uint8_t line, uint8_t column, uint8_t options)
//{
//	// c = char
//	// b = bitmap
//	// i = text index
//	// j = line buffer index
//	// k = char line
//	uint8_t c, b, i, j, k, loop;
//	//memset(LineBuff,0,MAXBYTESINROW);
//	//GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_MOSI_PIN );
//
//	// rendering happens line-by-line because this display can only be written by line
//	k = 0;
//
//	while((k < 72) && (line < PIXELS_Y))// loop for 72 character lines while within display
//	{
//		i = 0;
//		j = 0;
//		if((column > 0) && (column < MAXBYTESINROW))
//		{
//			for(loop = 0;loop < column;loop++)
//			{
//				b = 0;
//
//				{											// pixels are LOW active
//					b = ~b;
//				}
//
//				{
//					LineBuff[j] = b;						// store pixels in line buffer
//					j++;									// we've written one byte to buffer
//				}
//			}
//		}
//		while(j < (PIXELS_X/8) && (c = text[i]) != 0)	// while we did not reach end of line or string
//		{
//			if(c < '0' || c > ';')						// invalid characters are replace with SPACE
//			{
//				c = '0';
//			}
//
//			c = c - '0';									// convert character to index in font table
//			for(loop = 0;loop < 6;loop++)//width of each charecter
//			{
//				b = font48x72[(c)][loop+(k*6)];						// retrieve byte defining one line of character
//				if( (b != 0xFF) && (loop != 0))//first byte is length, it is ignored as length is fixed to 6 and 0xff are extra fills
//				{
//					LineBuff[j] = b;						// store pixels in line buffer
//					j++;									// we've written one byte to buffer
//				}
//			}
//			i++;										// next character
//		}
//
//		/*while(j < (PIXELS_X/8))							// pad line for empty characters
//		{
//			LineBuff[j] = 0xff;
//			j++;
//		}*/
//
//		SPIWriteRLELine_mod(line++,j,column);							// write line buffer
//
//		if(options & DISP_HIGH && line < PIXELS_Y)	// repeat line if DISP_HIGH is selected
//		{
//			SPIWriteRLELine_mod(line++,j,column);
//		}
//		//GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_MOSI_PIN );
//		k++;											// next pixel line
//	}
//
//}
//#else
////FONT:48x72
//void printSharp48x72(const uint8_t* text, uint8_t line, uint8_t column, uint8_t options)
//{
//	// c = char
//	// b = bitmap
//	// i = text index
//	// j = line buffer index
//	// k = char line
//	uint8_t c, b, i, j, k, loop;
//	//memset(LineBuff,0,MAXBYTESINROW);
//	//GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_MOSI_PIN );
//
//	// rendering happens line-by-line because this display can only be written by line
//	k = 0;
//
//	while((k < 72) && (line < PIXELS_Y))// loop for 72 character lines while within display
//	{
//		i = 0;
//		j = 0;
//		if((column > 0) && (column < MAXBYTESINROW))
//		{
//			for(loop = 0;loop < column;loop++)
//			{
//				b = 0;
//
//				{											// pixels are LOW active
//					b = ~b;
//				}
//
//				{
//					LineBuff[j] = b;						// store pixels in line buffer
//					j++;									// we've written one byte to buffer
//				}
//			}
//		}
//		while(j < (PIXELS_X/8) && (c = text[i]) != 0)	// while we did not reach end of line or string
//		{
//			if(c < '0' || c > ';')						// invalid characters are replace with SPACE
//			{
//				c = '0';
//			}
//
//			c = c - '0';									// convert character to index in font table
//			for(loop = 0;loop < 6;loop++)//width of each charecter
//			{
//				b = font48x72[(c)][loop+(k*6)];						// retrieve byte defining one line of character
//				if(!(options & DISP_INVERT))				// invert bits if DISP_INVERT is _NOT_ selected
//				{											// pixels are LOW active
//					b = ~b;
//				}
//
//				if((options & DISP_WIDE) && (c != 0))		// double width rendering if DISP_WIDE and character is not SPACE
//				{
//					//doubleWideAsm(b, &LineBuff[j]);			// implemented in assembly for efficiency/space reasons
//					LineBuff[j] = b;
//					LineBuff[j+1] = b;
//
//					j += 2;									// we've written two bytes to buffer
//				}
//				else										// else regular rendering
//
//					//if( (b != 0xFF) && (loop != 0))//first byte is length, it is ignored as length is fixed to 6 and 0xff are extra fills
//				{
//					LineBuff[j] = b;						// store pixels in line buffer
//					j++;									// we've written one byte to buffer
//				}
//			}
//			i++;										// next character
//		}
//
//		while(j < (PIXELS_X/8))							// pad line for empty characters
//		{
//			LineBuff[j] = 0xff;
//			j++;
//		}
//
//		//SPIWriteRLELine_mod(line++,j,column);							// write line buffer
//		SPIWriteLine(line++);
//
//		if(options & DISP_HIGH && line < PIXELS_Y)	// repeat line if DISP_HIGH is selected
//		{
//			//SPIWriteRLELine_mod(line++,j,column);
//			SPIWriteLine(line++);
//		}
//		//GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_MOSI_PIN );
//		k++;											// next pixel line
//	}
//
//}
//#endif

// generic function for CARD,COIN,HOURGLASS
void printSharp_Card(const uint8_t *byte, uint8_t row, uint8_t column, uint8_t bytes4row, uint16_t numofrows)
{
	// c = char
	// b = bitmap
	// i = text index
	// j = line buffer index
	// k = char line
	uint8_t b, j, k, loop;
	uint16_t line;

	// rendering happens line-by-line because this display can only be written by line
	k = 0;
	line = row;
	while((k < numofrows) && (line < PIXELS_Y)) // loop for n character lines while within display
	{
		j = 0;
		//idx = 0;
		if(bytes4row < MAXBYTESINROW)	// while we did not reach end of line
		{
			if((column > 0) && (column < MAXBYTESINROW))
			{
				for(loop = 0;loop < column;loop++)
				{
					b = 0;

					{											// pixels are LOW active
						b = ~b;
					}

					{
						LineBuff[j] = b;						// store pixels in line buffer
						j++;									// we've written one byte to buffer
					}
				}
			}

			if((j+bytes4row) < MAXBYTESINROW)
			{
				for(loop = 0;loop < bytes4row;loop++)
				{
					b = byte[(k*bytes4row)+loop];						// retrieve byte defining one line of character


					{											// pixels are LOW active
						b = ~b;
					}

					{
						LineBuff[j] = b;						// store pixels in line buffer
						j++;									// we've written one byte to buffer
					}
				}
			}

		}

		while(j < (PIXELS_X/8))							// pad line for empty characters
		{
			LineBuff[j] = 0xff;
			j++;
		}

		SPIWriteLine(line++);							// write line buffer

		k++;											// next pixel line
	}
}

//void printSharp_CardCoin(const uint8_t *byte, uint8_t row, uint8_t column, uint8_t bytes4row, uint16_t numofrows)  //not used //vinay
//{
//	// c = char
//	// b = bitmap
//	// i = text index
//	// j = line buffer index
//	// k = char line
//	uint8_t b, j, k, loop;
//	uint16_t line;
//
//	// rendering happens line-by-line because this display can only be written by line
//	k = 0;
//	line = row;
//	while((k < numofrows) && (line < PIXELS_Y))	// loop for n character lines while within display
//	{
//		j = 0;
//		//idx = 0;
//		if(bytes4row < MAXBYTESINROW)	// while we did not reach end of line
//		{
//			if((column > 0) && (column < MAXBYTESINROW))
//			{
//				for(loop = 0;loop < column;loop++)
//				{
//					b = 0;
//
//					{											// pixels are LOW active
//						b = ~b;
//					}
//
//					{
//						LineBuff[j] = b;						// store pixels in line buffer
//						j++;									// we've written one byte to buffer
//					}
//				}
//			}
//
//			if((j+bytes4row) < MAXBYTESINROW)
//			{
//				for(loop = 0;loop < bytes4row;loop++)
//				{
//					b = byte[(k*bytes4row)+loop];						// retrieve byte defining one line of character
//
//
//					{											// pixels are LOW active
//						b = ~b;
//					}
//
//					{
//						LineBuff[j] = b;						// store pixels in line buffer
//						j++;									// we've written one byte to buffer
//					}
//				}
//			}
//
//		}
//
//		while(j < (PIXELS_X/8))							// pad line for empty characters
//		{
//			LineBuff[j] = 0xff;
//			j++;
//		}
//
//		SPIWriteLine(line++);							// write line buffer
//
//		k++;											// next pixel line
//	}
//}

//void printSharpnoparking(const uint8_t *byte, uint8_t row, uint8_t column, uint8_t bytes4row, uint16_t numofrows)  //not used //vinay
//{
//	SPI_disableModule(EUSCI_B1_BASE);
//	Display_Init_SWSPI();
//	// c = char
//	// b = bitmap
//	// i = text index
//	// j = line buffer index
//	// k = char line
//	uint8_t b, j, k, loop/*,RLE_pixel_value,bytes,pixel_loop*/;
//	uint16_t line/*,RLE_repeat_count*/;
//
//	// rendering happens line-by-line because this display can only be written by line
//	k = 0;
//	line = row;
//	while((k < numofrows) && (line < PIXELS_Y)) // loop for n character lines while within display
//	{
//		j = 0;
//		//idx = 0;
//		if(bytes4row < MAXBYTESINROW)	// while we did not reach end of line or string
//		{
//			if((column > 0) && (column < MAXBYTESINROW))
//			{
//				for(loop = 0;loop < column;loop++)
//				{
//					b = 0;
//
//					{											// pixels are LOW active
//						b = ~b;
//					}
//
//					{
//						LineBuff[j] = b;						// store pixels in line buffer
//						j++;									// we've written one byte to buffer
//					}
//				}
//			}
//
//			if((j+bytes4row) < MAXBYTESINROW)
//			{
//				for(loop = 0;loop < bytes4row;loop++)
//				{
//					b = byte[(k*bytes4row)+loop];						// retrieve byte defining one line of character
//					if( (b != 0xFF) && (loop != 0))// FF's are extra fills to make fixed array size and first byte of each row is length
//					{
//						{											// pixels are LOW active
//							if(j < MAXBYTESINROW)
//							{
//								LineBuff[j] = b;						// store pixels in line buffer
//								j++;
//							}// we've written one byte to buffer
//						}
//
//					}
//				}
//			}
//
//		}
//
//		/*while(j < (PIXELS_X/8))							// pad line for empty characters
//		{
//			LineBuff[j] = 0;
//			j++;
//		}*/
//
//		SPIWriteRLELine(line++,j);							// write line buffer
//
//		k++;											// next pixel line
//	}
//	init_display_SPI_module_GPIO();
//}

// transfer line buffer to display using SPI
// input: line	position where line buffer is rendered
void SPIWriteLine(uint8_t line)
{
	if(Flash_Operation != LCD_SPI)
		init_display_SPI_module_GPIO();

	uint8_t j = 0;
	Display_SPI_Chip_Enable();

	SPIWriteByte(MLCD_WR | VCOM);						// send command to write line(s)
	SPIWriteByte(line+1);								// send line address

	while(j < (PIXELS_X/8))								// write pixels / 8 bytes
	{
		SPIWriteByte( LineBuff[j++]);						// transfer byte
	}

	SPIWriteByte(0);									// send 16 bit to latch buffers and end transfer
	Display_SPI_Chip_Disable();
}

////RLE format using hardware SPI
void SPIWriteRLELine_mod(uint8_t line,uint8_t size,uint8_t column )
{
	if(Flash_Operation != LCD_SPI)
		init_display_SPI_module_GPIO();
	/*SPI_disableModule(EUSCI_B1_BASE);
	if(Flash_Operation != LCD_SPI)
		Display_Init_SWSPI();
*/
	uint8_t repeat = 0,j = 0,shift_count = 0;
	bits_count = 0;
	uint64_t dummy = 0,dummy1 = 0;
	uint8_t RLE_pixel_value,RLE_repeat_count;
	uint8_t LineBuff1[PIXELS_X/8],line_index = 0;

	memset(LineBuff1,0,50);
	memcpy(LineBuff1,LineBuff,column);
	line_index = column;
	j = column;
	while((j < size) && (bits_count < PIXELS_X))								// write pixels
	{
		if(((LineBuff[j] & 0x80) != 0x80) || (LineBuff[j] == 0XFF))
		{
			if(LineBuff[j] != 0xFF)
				LineBuff[j] = ~ LineBuff[j];

			dummy1 = (((uint64_t)LineBuff[j] & 0x3F) << (shift_count));
			dummy |= dummy1;
			bits_count += 7;
			shift_count += 7;
			j++;
		}
		else
		{
			if((LineBuff[j] & 0x40) == 0x40)//check pixel value
				RLE_pixel_value = 0;//pixels are active low
			else
				RLE_pixel_value = 1; //pixels are active low

			RLE_repeat_count = (LineBuff[j] & 0x3F);
			if(RLE_pixel_value == 1)
			{
				for(repeat=0;repeat<RLE_repeat_count;repeat++)
				{
					dummy1 = ((uint64_t)RLE_pixel_value << (shift_count++));
					dummy |= dummy1;
					bits_count++;
				}
			}
			else
			{
				for(repeat=0;repeat<RLE_repeat_count;repeat++)
				{
					dummy1 = ~((uint64_t)BIT0 << (shift_count++));
					dummy &= dummy1;
					bits_count++;
				}
			}
				j++;
		}
		if((bits_count%48)==0)//check end of each char
		{
			memcpy(&LineBuff1[line_index],&dummy,6);//Each CHAR in RLE 48x72 font is 6 bytes width
			dummy = 0;
			shift_count = 0;
			line_index += 6;
		}
	}


	while(line_index < (PIXELS_X/8))							// pad line for empty characters
	{
		LineBuff1[line_index] = 0xff;
		line_index++;
	}

	Display_SPI_Chip_Enable();

	SPIWriteByte(MLCD_WR | VCOM);						// send command to write line(s)
	SPIWriteByte(line+1);								// send line address

	j = 0;
	while(j < (PIXELS_X/8))								// write pixels / 8 bytes
	{
		SPIWriteByte( LineBuff1[j++]);						// transfer byte
	}

    // send 16 bit to latch buffers and end transfer
	SPIWriteByte(0);
	Display_SPI_Chip_Disable();
}

//RLE format using software SPI
void SPIWriteRLELine(uint8_t line,uint8_t size )
{
	SPI_disableModule(EUSCI_B1_BASE);
	if(Flash_Operation != LCD_SPI)
		Display_Init_SWSPI();

	bits_count = 0;
	uint8_t last_bit = 0, j = 0;
	Display_SPI_Chip_Enable();

	SPIWriteBitBangByte(MLCD_WR | VCOM);						// send command to write line(s)
	SPIWriteBitBangByte(line+1);								// send line address

	while((j < size) && (bits_count < PIXELS_X))								// write pixels / 8 bytes
	{
		if(((LineBuff[j] & 0x80) != 0x80) || (LineBuff[j] == 0XFF))
		{
			if(LineBuff[j] != 0xFF)
				LineBuff[j] = ~ LineBuff[j];
			SPIWrite_7bits( LineBuff[j++]);
			bits_count += 7;
		}
		else
			SPIWriteRLEByte( LineBuff[j++]);
	}

	while((bits_count < PIXELS_X))//Fill remaining bits with 1
	{
		if(last_bit != 1)
		{
			//GPIO_setOutputHighOnPin(DISPLAY_SPI_IOPORT, DISPLAY_SPI_MOSI_PIN);
			DISPLAY_SPI_DATA |= DISPLAY_SPI_MOSI_PIN;
			last_bit = 1;
		}
		//DelayUs(10);

		//GPIO_setOutputHighOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_SCK_PIN);// toggle clock high
		DISPLAY_SPI_DATA |= DISPLAY_SPI_SCK_PIN;

		//DelayUs(50);

		//GPIO_setOutputLowOnPin(DISPLAY_SPI_IOPORT ,DISPLAY_SPI_SCK_PIN); // toggle clock low
		DISPLAY_SPI_DATA &= ~DISPLAY_SPI_SCK_PIN;
		bits_count ++;
	}

	SPIWriteBitBangByte(0);									// send 16 bit to latch buffers and end transfer
	//SPIWriteByte(0);
	Display_SPI_Chip_Disable();
}

void Clear_Screen()
{
	init_display_SPI_module_GPIO();
	Display_SPI_Chip_Enable();
	SPIWriteByte(MLCD_CM | VCOM);						// send clear display memory command
	SPIWriteByte(0);									// send command trailer
	Display_SPI_Chip_Disable();
	Update_RTC_on_Display();
}

void Update_RTC_on_Display()
{
	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
		return;

	uint8_t hours_AMPM = 0;
	rtc_c_calender = RTC_C_getCalendarTime();
	if(rtc_c_calender.hours > 12)
		hours_AMPM = rtc_c_calender.hours - 12;
	else
		hours_AMPM = rtc_c_calender.hours;

	if(rtc_c_calender.hours > 11)
	{
		/*if(occupancygl == 1)
			sprintf((char*)RTC_String, " %02d/%02d/%02d  @   %02d:%02dPM ",rtc_c_calender.month,rtc_c_calender.dayOfmonth,rtc_c_calender.year,hours_AMPM,rtc_c_calender.minutes);
		else*/
			sprintf((char*)RTC_String, " %02d/%02d/%02d      %02d:%02dPM ",rtc_c_calender.month,rtc_c_calender.dayOfmonth,rtc_c_calender.year,hours_AMPM,rtc_c_calender.minutes);
	}
	else
	{
		if (rtc_c_calender.hours == 0)
			hours_AMPM = 12; //LNGSIT-634

		/*if(occupancygl == 1)
			sprintf((char*)RTC_String, " %02d/%02d/%02d  @   %02d:%02dAM ",rtc_c_calender.month,rtc_c_calender.dayOfmonth,rtc_c_calender.year,hours_AMPM,rtc_c_calender.minutes);
		else*/
			sprintf((char*)RTC_String, " %02d/%02d/%02d      %02d:%02dAM ",rtc_c_calender.month,rtc_c_calender.dayOfmonth,rtc_c_calender.year,hours_AMPM,rtc_c_calender.minutes);

	}
	printSharp16x24(RTC_String, 220, 0, 1);
}

void Update_Parking_Clock_Screen()
{
	/*uint8_t hours_to_display = 0;
		uint8_t minutes_to_display = 0;
		uint16_t temp_minutes_to_disp = 0;
		uint8_t parking_clock[15] = { 0 };
		uint16_t bay_number_to_display = 0;*/
	/*uint8_t Coin_Update_Ok = 0;
	if(earned_time_in_seconds_temp|=earned_time_in_seconds)
		{
			Coin_Update_Ok = 1;
			earned_time_in_seconds_temp = earned_time_in_seconds;
		}*/
//	Debug_Output1(0,"cardread1069:%ld", cardread);

	if(parking_time_left_on_meter[0]>min_time_insec) time_retain[0] = 1;
	else if(parking_time_left_on_meter[0] <= 0)
			{
				time_retain[0] = 0;
				parking_time_left_on_meter_prev[0] = 0;
			}

	if(parking_time_left_on_meter[1]>min_time_insec) time_retain[1] = 1;
	else if(parking_time_left_on_meter[1] <= 0)
			{
				time_retain[1] = 0;
				parking_time_left_on_meter_prev[1] = 0;
			}

	if(gAutoCalibration == true)	//In calibration mode
		return;

	if((!(((WAKEUP_COIL_JAM_RAISED == TRUE) && (CARD_JAM_RAISED == TRUE))
			|| ((OPTICAL_COIN_JAM_RAISED == TRUE) && (CARD_JAM_RAISED == TRUE)))) || (glSystem_cfg.show_alarm_on_lcd == FALSE))	//LNGSIT-838
	{
		if((Tech_menu == FALSE) && (Tech_key_seq == FALSE) && (meter_out_of_service == FALSE))
		{

			if(((time_retain[Current_Space_Id]==0)&&(min_time_insec<=0))||(time_retain[Current_Space_Id]==1)||(parking_time_left_on_meter[Current_Space_Id]>=min_time_insec))
				{
					////Debug_Output1(0,"222 parking_time_left_on_meter[Current_Space_Id]=%ld",parking_time_left_on_meter[Current_Space_Id]);
					////Debug_Output1(0,"222 parking_time_left_on_meter_prev[Current_Space_Id]=%ld",parking_time_left_on_meter_prev[Current_Space_Id]);
					////Debug_Output1(0,"222 time_retain[Current_Space_Id]=%d",time_retain[Current_Space_Id]);
					////Debug_Output1(0,"222 [Current_Space_Id]=%d",Current_Space_Id);

					hours_to_display = parking_time_left_on_meter[Current_Space_Id]/3600;
					temp_minutes_to_disp = (parking_time_left_on_meter[Current_Space_Id] % 3600);
					minutes_to_display = temp_minutes_to_disp/60;

					if((temp_minutes_to_disp % 60) > 0)	//This is to fix the wrong update of parking clock
					{
						minutes_to_display += 1;
						if(minutes_to_display > 59)
						{
							hours_to_display += 1;
							minutes_to_display = 0;
						}
					}
				}
			else if((min_time_insec != 0)&&((minutes_left_in_current_schedule*60) <= min_time_insec))
			{
				hours_to_display = parking_time_left_on_meter[Current_Space_Id]/3600;
				temp_minutes_to_disp = (parking_time_left_on_meter[Current_Space_Id] % 3600);
				minutes_to_display = temp_minutes_to_disp/60;
				if((temp_minutes_to_disp % 60) > 0)	//This is to fix the wrong update of parking clock
				{
					minutes_to_display += 1;
					if(minutes_to_display > 59)
					{
						hours_to_display += 1;
						minutes_to_display = 0;
					}
				}
			}
			else
				{
					hours_to_display = 0;
					minutes_to_display = 0;
				}

			if((glSystem_cfg.grace_time_type == NEGATIVE_COUNTDOWN) && (grace_time_trigger[Current_Space_Id] == true))
			{
				hours_to_display = (negetive_parking_time_left_on_meter[Current_Space_Id]+1)/3600;
				temp_minutes_to_disp = ((negetive_parking_time_left_on_meter[Current_Space_Id]+1) % 3600);
				minutes_to_display = temp_minutes_to_disp/60;

				if((temp_minutes_to_disp % 60) > 0)	//This is to fix the wrong update of parking clock
				{
					minutes_to_display += 1;
					if(minutes_to_display > 59)
					{
						hours_to_display += 1;
						minutes_to_display = 0;
					}
				}
				/*			bay_number_to_display = ((Current_Space_Id+glSystem_cfg.meter_id) % 1000) - BayAdjustValue;
				sprintf((char*)parking_clock, "SPACE%01d ;%01d:%02d", bay_number_to_display, hours_to_display, minutes_to_display);
				printSharp32x48(parking_clock, 75,1,0);

				bay_number_to_display = ((Current_Space_Id+glSystem_cfg.meter_id) % 1000) - (BayAdjustValue+1);
				sprintf((char*)parking_clock, "SPACE%01d ;%01d:%02d", bay_number_to_display, hours_to_display, minutes_to_display);
				printSharp32x48(parking_clock, 125,1,1);*/
			}
			else if((glSystem_cfg.grace_time_type == HOLD_EXPIRED) && (grace_time_trigger[Current_Space_Id] == true)) // && grace_timeout == 0)
			{
				/*			bay_number_to_display = ((Current_Space_Id+glSystem_cfg.meter_id) % 1000) - BayAdjustValue;
				sprintf((char*)parking_clock, "SPACE%01d %02d:%02d", bay_number_to_display, hours_to_display, minutes_to_display);
				printSharp32x48(parking_clock, 75,1,0);

				bay_number_to_display = ((Current_Space_Id+glSystem_cfg.meter_id) % 1000) - (BayAdjustValue+1);
				sprintf((char*)parking_clock, "SPACE%01d %02d:%02d", bay_number_to_display, hours_to_display, minutes_to_display);
				printSharp32x48(parking_clock, 125,1,1);*/
			}
			else
			{
				/*			bay_number_to_display = ((Current_Space_Id+glSystem_cfg.meter_id) % 1000) - BayAdjustValue;
				sprintf((char*)parking_clock, "SPACE%01d %02d:%02d", bay_number_to_display, hours_to_display, minutes_to_display);
				printSharp32x48(parking_clock, 75,1,0);

				bay_number_to_display = ((Current_Space_Id+glSystem_cfg.meter_id) % 1000) - (BayAdjustValue+1);
				sprintf((char*)parking_clock, "SPACE%01d %02d:%02d", bay_number_to_display, hours_to_display, minutes_to_display);
				printSharp32x48(parking_clock, 125,1,1);*/
			}

	 		switch ( key_press )//up arrow
			{
			case KEY_UP:
				Current_Space_Id = RIGHT_BAY;
				key_RL = RIGHT_BAY;
				if(payment_on_hold == true)
				{
					//Debug_TextOut( 0, "1216KEYUP" );
					Clear_Screen();
					CoinOk();
					DelayMs(1000);
					payment_on_hold = false;

				}
				else
				{
					DSM_Screen2();
				}
				break;

			case KEY_DOWN://down arrow
				Current_Space_Id = LEFT_BAY;
				key_RL = LEFT_BAY;
				if(payment_on_hold == true)
				{
					//Debug_TextOut( 0, "1241KEYDOWN" );
					Clear_Screen();
					CoinOk();
					DelayMs(1000);
					payment_on_hold = false;

				}
				else
				{
					DSM_Screen2();
				}
				break;

			case KEY_CANCEL: //cancel key
				if(payment_on_hold != true)
				{
					DSM_Screen1();
				}
				else if(payment_on_hold == true)
				{
					//Debug_TextOut(0,"1274\n");
					DSM_Screen1();
				}
				else
				{
					DSM_Screen4();
				}
				break;

			case KEY_OK://key ok
				if(payment_on_hold != true)
				{
					DSM_Screen3();
				}
				else if(payment_on_hold == true)
				{
					//Debug_TextOut(0,"1289\n");
					Clear_Screen();
					CoinOk();
					payment_on_hold = false;
					DSM_Screen2();
				}
				else
				{
					DSM_Screen4();
				}
				break;

			}
			init_LEDs_PWM();
		}
	}
}

//void LCD_Test(void)
//{
//	Clear_Screen();
//
///*	memset(rx_ans, 0, sizeof(rx_ans));
//	liberty_sprintf( (char*)rx_ans,"Meter Name: %s", glMB_info.locid );
//	printSharp16x24(rx_ans, 100, 10, 0);*/
////	printSharp48x72("123LCD TEST[/.", 5, 1, 0);
//	printSharp32x48("LCD TEST[/.", 140, 5, 0);
//}

void AUDIT_Screen()
{

// Clear_Screen();
//
// //Update_Parking_Clock_Screen(0);
// memset(rx_ans, 0, sizeof(rx_ans));
// liberty_sprintf( (char*)rx_ans,"Meter Name:%s", glMB_info.locid );
// printSharp16x24(rx_ans, 100, 0, 0); // printSharp16x24(rx_ans, 100, 10, 0); // LNGSIT-850 //vinay
// printSharp32x48("AUDIT DONE", 140, 5, 0);
// memset(rx_ans, 0, sizeof(rx_ans));
//
// Update_RTC_on_Display();
// Screen_Clear = true;

	//to display baatery voltage on display //vinay
	uint8_t ldisplay_string[25] = { 0 };
	//uint16_t Bat_vltg;
	float Bat_vltg;
	Clear_Screen();

	//Update_Parking_Clock_Screen();
	memset(rx_ans, 0, sizeof(rx_ans));
	liberty_sprintf( (char*)rx_ans,"Meter Name: %s", glMB_info.locid );
	printSharp16x24(rx_ans, 100, 1, 0);	//LNGSIT-850//printSharp16x24(rx_ans, 100, 7, 0);	//LNGSIT-850
	printSharp32x48("AUDIT DONE", 135, 5, 0);
	memset(rx_ans, 0, sizeof(rx_ans));
	Bat_vltg = Rechargeable_battery_read();
	Bat_vltg = Bat_vltg/100;
	sprintf((char*)ldisplay_string, "BATTERY VOLTAGE:%1.2fV", Bat_vltg);
	printSharp16x24(ldisplay_string,185,1,0);

	Update_RTC_on_Display();
	Screen_Clear = true;
}

//Print card or coin
//void InsertCard_Screen()  //not used //vinay
//{
//	Clear_Screen();
//
//	//Update_Parking_Clock_Screen();
//	printSharp16x24("Insert Card or Coins", 80, 5, 0);
//	printSharp_Card(fontGIF1, 110, 21, 8, 488/8);
//	printSharp16x24("Insert Card as Shown", 180, 5, 0);
//	Update_RTC_on_Display();
//	Screen_Clear = true;
//}

//Print card+coin
//void InsertCardCoin_Screen()  //not used //vinay
//{
//	//Debug_TextOut(0,"Insertcardcoin screen");
//	//printSharp_Card(fontGIF6, 80, 12, 27, 1944/27);
//	printSharp32x48("   INSERT ", 100, 1, 0);
//	printSharp32x48(" CARD or COIN", 150, 1, 0);
//	//Update_RTC_on_Display();
//	Screen_Clear = true;
//}

void Noparking_Screen()
{
	Clear_Screen();
	//Debug_TextOut(0,"Noparking screen");
	//printSharpnoparking(fontGIF7,30,23,11,64); //LNGSIT-670, LNGSIT-839
	//printSharp32x48("NO PARKING", 100, 5, 0); //LNGSIT-839
	printSharp32x48("    NO ", 50, 5, 0); //LNGSIT-839
	printSharp32x48("  PARKING", 100, 5, 0); //LNGSIT-839
	//printSharp16x24("OUT OF SERVICE", 150, 10, 0);	//TODO:OUT of Service is only for Alarm, we can enable this with a check later
	Update_RTC_on_Display();
	Screen_Clear = true;
}

void Freeparking_Screen()
{
	Clear_Screen();
	//Debug_TextOut(0,"Freeparking screen");
	printSharp32x48("FREE PARKING", 100, 1, 0);
	Update_RTC_on_Display();
	Screen_Clear = true;
}

uint8_t CardReadFail_Screen()
{
	uint8_t z=0;
	Clear_Screen();
	//Update_Parking_Clock_Screen();
	//Debug_TextOut(0,"cardread fail screen");
	printSharp32x48(" ORIENT CARD", 1, 0, 0);
	printSharp32x48(" STRIPE LEFT", 50, 0, 0);
	printSharp_Card(fontGIF3, 100, 21, 9, 648/9);
	printSharp16x24(" REINSERT CARD AS SHOWN", 180, 2, 0);
//	DelayMs(5000);
//	printSharp16x24("Unable to Read Card", 80, 5, 0);
//	printSharp_Card(fontGIF1, 110, 21, 8, 488/8);
//	printSharp16x24("Please Re-Insert Card", 180, 5, 0);
	for(z=0;z<100;z++)
	{
		DelayMs(50);
		if(GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN) == 0)
		{
	//		Clear_Screen();
			cardread = 0;
			return 1;
			//break;
		}
	}
	Update_RTC_on_Display();
	Screen_Clear = true;
	cardread = 0; //this is avoid please wait msg during CC payment //vinay
	return 0;
	//Debug_Output1(0,"cardreadfailscreen:%ld", cardread);

}

void ShowAmount_Screen(uint16_t lamount_selected, uint8_t min_max_display,uint8_t screen_refresh)
{
	//Clear_Screen();
	init_display_SPI_module_GPIO();
	uint8_t ldisplay_string[15] = { 0 };
	uint32_t lseconds_to_display = 0; //, pre_existing_seconds = 0;
	uint8_t show_meter_full[MSM_MAX_PARKING_BAYS] = {0};
	uint16_t Surcharges1, Surcharges2; // vinay
	//Debug_TextOut(0,"Showamount screen");
	cardread = 1; //this is avoid please wait msg during CC payment //vinay
	//Debug_Output1(0,"cardread showamount screen:%ld", cardread);

	get_earned_seconds(0XFF, false, false);//dummy call to refresh the max time allowed variable
	if(ANTI_FEED_in_effect[Current_Space_Id] == true)	//Anti Feed feature enabled in config
	{
		//Debug_TextOut(0,"Anti Feed for space 0");
		Debug_Output1(0,"Anti Feed for space [%d]",Current_Space_Id);
		AntiFeed_Screen();
		Meter_Full_Flg[Current_Space_Id] = TRUE;	//to make sure payment is not processed on key press
		return;
	}

	if((((minutes_left_in_current_schedule+minutes_left_in_next_schedule)*60) >= max_time_in_current_rate)||(current_rate_index == PREPAY_PARKING))show_meter_full[Current_Space_Id] = 1;
	else show_meter_full[Current_Space_Id] = 0; //for showing max time paid in coin drop // VT



	if((((parking_time_left_on_meter[Current_Space_Id] >= (max_time_in_current_rate - 60))&&(show_meter_full[Current_Space_Id]==1))||
			((parking_time_left_on_meter[Current_Space_Id] >= (((minutes_left_in_current_schedule+minutes_left_in_next_schedule)*60) - 60))&&(show_meter_full[Current_Space_Id]==0))))
	{
		Debug_TextOut(0,"Meter Full");
		MeterFull_Screen();
		Meter_Full_Flg[Current_Space_Id] = TRUE;
		if(parking_time_left_on_meter[Current_Space_Id] > 0) time_retain[Current_Space_Id] = 1;
		return;
	}

	if(screen_refresh == TRUE)
	{
		Clear_Screen();
		//Update_Parking_Clock_Screen(0);
	}

	if(smart_card_mode == TRUE)
	{
		//sprintf((char*)ldisplay_string, "Card Balance:Rs.%02d.%02d", (smart_card.balance/100), (smart_card.balance % 100));
		sprintf((char*)ldisplay_string, "Card Balance:$%02d.%02d", (smart_card.balance/100), (smart_card.balance % 100));
		printSharp16x24(ldisplay_string,75,5,0);
	}
	sprintf((char*)ldisplay_string, "< ;%02d.%02d >", (lamount_selected/100), (lamount_selected % 100));
//	printSharp32x48(ldisplay_string,100,5,0);
	if(smart_card_mode == TRUE)
		printSharp32x48(ldisplay_string,100,5,0);  //this was currpting smart card payment screen //vinay
	else
		printSharp32x48(ldisplay_string,80,5,0);  //this was currpting smart card payment screen //vinay

	/*pre_existing_seconds = (parking_time_left_on_meter[Current_Space_Id]);

	lseconds_to_display = get_earned_seconds(lamount_selected, true);

	if ((parking_time_left_on_meter[Current_Space_Id] > 0 ) && (lseconds_to_display >= pre_existing_seconds))
	{
		lseconds_to_display -= pre_existing_seconds;
	}

	if((parking_time_left_on_meter[Current_Space_Id] + lseconds_to_display) >= (max_time_in_current_rate-1))
			Max_Amount_In_current_rate = TRUE;

	if((lseconds_to_display == 0) && (parking_time_left_on_meter[Current_Space_Id] > 0)) //for cc auth
			do_not_allow_cc_auth = TRUE;*/

	this_is_a_CC = true;
	if (parking_time_left_on_meter[Current_Space_Id] > 0 )
	{
		lseconds_to_display = get_earned_seconds(lamount_selected, true, false);
		//this below code is making top ups incorrect like for $1 buys is 1 but it gives .59 //LNGSIT-2004 & 2011 //vinay
		//if((lseconds_to_display > 0)&&(current_rate_index < PREPAY_PARKING))
			//lseconds_to_display--;
	}
	else
		lseconds_to_display = get_earned_seconds(lamount_selected, false, false);
	this_is_a_CC = false;

	if ((parking_time_left_on_meter[Current_Space_Id] > 0 ) && (lseconds_to_display >= (parking_time_left_on_meter[Current_Space_Id])))
	{
		lseconds_to_display -= (parking_time_left_on_meter[Current_Space_Id]);
	}

	if(((parking_time_left_on_meter[Current_Space_Id]) + lseconds_to_display) >= (max_time_in_current_rate-1))
		Max_Amount_In_current_rate = TRUE;

	if((lseconds_to_display == 0) && (parking_time_left_on_meter[Current_Space_Id] > 0)) //for cc auth
		do_not_allow_cc_auth = TRUE;
	//some times we are not getting correct lseconds its 0:59 instead of 1:00 //so adjusting it to correct value //LNGSIT-2003 & 2010//vinay
	//Debug_Output1( 1, "before lseconds_to_display %ld", lseconds_to_display );
	while(lseconds_to_display%60 != 0)
	{
		//Debug_Output1( 1, " while lseconds_to_display %ld", lseconds_to_display );
		lseconds_to_display = lseconds_to_display + 1;
	}
	//Debug_Output1( 1, "after lseconds_to_display %ld", lseconds_to_display );
	if(min_max_display == MIN_AMOUNT_DISPLAY)	//if minimum
	{
		////sprintf((char*)ldisplay_string, "BUYS '() %02d:%02d-Min ", (lseconds_to_display/3600), ((lseconds_to_display % 3600)/60));
		//sprintf((char*)ldisplay_string, "BUYS %02d:%02d(Min Allowed)", (lseconds_to_display/3600), ((lseconds_to_display % 3600)/60));
		//printSharp16x24(ldisplay_string, 160, 2, 0);
		if(smart_card_mode == TRUE)
		{
			sprintf((char*)ldisplay_string, "BUYS %02d:%02d(Min Allowed)", (lseconds_to_display/3600), ((lseconds_to_display % 3600)/60));
			printSharp16x24(ldisplay_string, 160, 2, 0);
		}
		else
		{
			//sprintf((char*)ldisplay_string, "MIN %1d:%02d HRS", (lseconds_to_display/3600), ((lseconds_to_display % 3600)/60));//this is COS requirement
			sprintf((char*)ldisplay_string, " MIN  %02d:%02d ", (lseconds_to_display/3600), ((lseconds_to_display % 3600)/60));//this is Birmingham requirement
			printSharp32x48(ldisplay_string, 140, 1, 0);
		}
	}
	else if(min_max_display == MAX_AMOUNT_DISPLAY)
	{
		////sprintf((char*)ldisplay_string, "BUYS '() %02d:%02d-Max ", (lminutes_to_display/60), (lminutes_to_display % 60));
		//sprintf((char*)ldisplay_string, "BUYS %02d:%02d(Max Allowed)", (lseconds_to_display/3600), ((lseconds_to_display % 3600)/60));
		//printSharp16x24(ldisplay_string, 160, 2, 0);
		if(smart_card_mode == TRUE)
		{
			sprintf((char*)ldisplay_string, "BUYS %02d:%02d(Max Allowed)", (lseconds_to_display/3600), ((lseconds_to_display % 3600)/60));
			printSharp16x24(ldisplay_string, 160, 2, 0);
		}
		else
		{
			//sprintf((char*)ldisplay_string, "MAX %02d:%02dHRS", (lseconds_to_display/3600), ((lseconds_to_display % 3600)/60));//this is COS requirement
			sprintf((char*)ldisplay_string, " MAX  %02d:%02d", (lseconds_to_display/3600), ((lseconds_to_display % 3600)/60));//this is Birmingham requirement
			printSharp32x48(ldisplay_string, 140, 1, 0);
		}
	}
	else
	{
		////sprintf((char*)ldisplay_string, "BUYS '() %02d:%02d ", (lminutes_to_display/60), (lminutes_to_display % 60));
		//sprintf((char*)ldisplay_string, "BUYS %02d:%02d", (lseconds_to_display/3600), ((lseconds_to_display % 3600)/60));
		//printSharp16x24(ldisplay_string, 160, 15, 0);
		sprintf((char*)ldisplay_string, " BUYS %02d:%02d", (lseconds_to_display/3600), ((lseconds_to_display % 3600)/60)); //this is Birmingham requirement
		if(smart_card_mode == TRUE)
			printSharp16x24(ldisplay_string, 160, 15, 0);
		else
			printSharp32x48(ldisplay_string, 140, 0, 0);
	}

	if(screen_refresh == TRUE)
	{
		Surcharges1 = (glSystem_cfg.CC_fixed_Surcharge / 100); // vinay calculated outside, to avoid meter crash by going in to infinte loop
		Surcharges2	= (glSystem_cfg.CC_fixed_Surcharge % 100); // vinay calculated outside, to avoid meter crash by going in to infinte loop
		//printSharp16x24("OK-*+         C-,;", 190, 5, 0);
		//printSharp16x24("$0.50 Surcharge Press Ok", 190, 1, 0);//Rehoboth Special screen
		if(glSystem_cfg.Display_Surcharges	== TRUE) //(glSystem_cfg.Display_Surcharges	== TRUE) //wait for config editor //vinay
		{
			if(Payment_card == 1)     //to avoid sucharges display for payment card
			{
				printSharp16x24("Press OK to continue", 190, 5, 0);
				Payment_card = 0;
			}
			else
			{
				sprintf((char*)ldisplay_string, "$%d.%02d Surcharge Press Ok", Surcharges1, Surcharges2); // vinay calculated outside, to avoid meter crash by going in to infinte loop
				printSharp16x24(ldisplay_string, 190, 1, 0);//Chester Special screen
			}
		}
		else
		{
			printSharp16x24("Press OK to continue", 190, 5, 0);
		}
		Update_RTC_on_Display();
	}
	Screen_Clear = true;
}

void Authorizing_Screen()
{
	Clear_Screen();
	//Update_Parking_Clock_Screen(0);
	//Debug_TextOut(0,"Authorization screen");
	//printSharp_Card(fontGIF5, 80, 22, 8, 488/8);
	//printSharp16x24("Authorizing Card...", 170, 5, 0);
	printSharp32x48("AUTHORIZING ", 100, 1, 0);
	printSharp32x48("   CARD...", 150, 1, 0);
	Update_RTC_on_Display();
	CC_Auth_Entered = TRUE;
	CC_TimeOut_RTCTimestamp = RTC_epoch_now();
	Screen_Clear = true;
	cardread = 1; //this is avoid please wait msg during CC payment //vinay
	//Debug_Output1(0,"cardread authorization screen :%ld", cardread);
}

void ReadCard_Screen()
{
	Clear_Screen();
	//Update_Parking_Clock_Screen(0);
	//Debug_TextOut(0,"Readcard screen");
	//printSharp_Card(fontGIF5, 80, 22, 8, 488/8);
	//printSharp16x24("Reading Card...", 170, 10, 0);
	printSharp32x48("  READING ", 100, 1, 0);
	printSharp32x48("   CARD...", 150, 1, 0);
	Update_RTC_on_Display();
	Screen_Clear = true;
	cardread = 1; //this is avoid please wait msg during CC payment //vinay
	//Debug_Output1(0,"cardread readcard screen:%ld", cardread);
}

void MeterFull_Screen()
{
	Clear_Screen();
	//Update_Parking_Clock_Screen(0);
	//Debug_TextOut(0,"Meterfull screen");
	printSharp32x48(" MAX TIME", 100, 5, 0);
	printSharp32x48("   PAID", 150, 5, 0);
	Debug_TextOut(0,"MAX TIME PAID");
	//printSharp32x48("METER FULL", 150, 5, 0);
	cardread = 0; //this is avoid please wait msg during CC payment //vinay
	//Debug_Output1(0,"cardread meterfull screen:%ld", cardread);
	if(glSystem_cfg.ANTI_FEED_ENABLE == TRUE)
	{
		//Debug_TextOut( 0, "Calling Antifeed" );
		Meter_Full[Current_Space_Id] = TRUE;
		get_earned_seconds(100, false, true); //dummy call to refresh the max time allowed variable// for antifeed //vinay
	}
	DelayMs(500);
	//Debug_TextOut(0,"Meterfull screen going to DSM_screen3");
	DSM_Screen3();
	Update_RTC_on_Display();
	//Screen_Clear = true;
}

void AntiFeed_Screen()
{
	Clear_Screen();
	//Update_Parking_Clock_Screen();
	//Debug_TextOut(0,"Antifeed screen");
	printSharp32x48("ANTI FEED", 150, 7, 0);
	DelayMs(500);
	Update_RTC_on_Display();
	Screen_Clear = true;
}

void AUDIT_In_Progress_Screen()
{
	Clear_Screen();
	//Update_Parking_Clock_Screen();
	//Debug_TextOut(0,"Audit in progress screen");
	printSharp16x24(" METER AUDIT IN PROGRESS", 120, 0, 0);
	printSharp16x24("PLEASE DO NOT REMOVE CARD", 160, 0, 0);
	Update_RTC_on_Display();
	Screen_Clear = true;
}

void AUDIT_Card_Full_Screen()
{
	Clear_Screen();
	//Update_Parking_Clock_Screen();
	//Debug_TextOut(0,"audit card full screen");
	printSharp16x24(" AUDIT CARD MEMORY FULL", 120, 0, 0);
	printSharp16x24(" PLEASE USE ANOTHER CARD", 160, 0, 0);
	Update_RTC_on_Display();
	Screen_Clear = true;
}

void AUDIT_Append_Failed_Screen()
{
	uint8_t ldisplay_string[25] = { 0 };
	float Bat_vltg;
	Clear_Screen();
	//Update_Parking_Clock_Screen();
	//Debug_TextOut(0,"audit append failed screen");
	memset(rx_ans, 0, sizeof(rx_ans));
	liberty_sprintf( (char*)rx_ans,"Meter Name: %s", glMB_info.locid );
	printSharp16x24(rx_ans, 100, 1, 0);	//LNGSIT-850//printSharp16x24(rx_ans, 100, 7, 0);	//LNGSIT-850
	printSharp32x48("AUDIT FAILED", 135, 2, 0);
	memset(rx_ans, 0, sizeof(rx_ans));
	Bat_vltg = Rechargeable_battery_read();
	Bat_vltg = Bat_vltg/100;
	sprintf((char*)ldisplay_string, "BATTERY VOLTAGE:%1.2fV", Bat_vltg);
	printSharp16x24(ldisplay_string,185,1,0);

	Update_RTC_on_Display();
	Screen_Clear = true;
}

//Insert Only Card
//void InsertOnlyCard_Screen()  //not used //vinay
//{
//	//Clear_Screen();
//	//Update_Parking_Clock_Screen(0);
//	//Debug_TextOut(0,"Insert only card screen");
//	printSharp_Card(fontGIF3, 80, 21, 9, 648/9);
//	printSharp32x48(" INSERT ONLY", 100, 5, 0);
//	printSharp32x48("     CARD    ", 150, 5, 0);
//	//printSharp16x24("Insert Card", 180, 10, 0);
//	//Update_RTC_on_Display();
//	Screen_Clear = true;
//}

//Insert Only Coin
//void InsertCoin_Screen()  //not used //vinay
//{
//	//Clear_Screen();
//	//Update_Parking_Clock_Screen(0);
//	//Debug_TextOut(0,"Insert coin screen");
//	//printSharp_Card(fontGIF4, 80, 21, 9, 648/9);
//	printSharp16x24("Insert Coin", 180, 10, 0);
//	//Update_RTC_on_Display();
//	Screen_Clear = true;
//}

void CC_Payment_Approved_Screen()
{
	Clear_Screen();
	//Update_Parking_Clock_Screen(0);
	//Debug_TextOut(0,"CC payment approved screen");
	printSharp32x48("APPROVED", 150, 10, 0);
	if(parking_time_left_on_meter[Current_Space_Id] > 0) time_retain[Current_Space_Id] = 1;
	Update_RTC_on_Display();
	// Clear all CC flags
	ccard_dataread_flag = FALSE;
	card_removed_flag = FALSE;
	check_card_read = FALSE;
	Screen_Clear = true;
	DelayMs(2000);
	cardread = 0; //this is avoid please wait msg during CC payment //vinay
	//Debug_Output1(0,"cardread cc pay appr screen :%ld", cardread);
	//Debug_TextOut(0,"CC pay appr going to DSMscr3 screen");
	DSM_Screen3(); //this is not there in regular DSM
}

void SC_Payment_Approved_Screen(uint8_t refund_done)
{
	uint8_t ldisplay_string[30] = { 0 };
	Clear_Screen();
	//Update_Parking_Clock_Screen(0);
	//Debug_TextOut(0,"SC payment approved screen");
	printSharp32x48("APPROVED", 100, 10, 0);
	if(parking_time_left_on_meter[Current_Space_Id] > 0) time_retain[Current_Space_Id] = 1;
	if(refund_done == true)
	{
		sprintf((char*)ldisplay_string, "Amount Refunded:$%02d.%02d", (smart_card.deduct_amount/100), (smart_card.deduct_amount%100));
		printSharp16x24(ldisplay_string,150,2,0);
	}
	else
	{
		sprintf((char*)ldisplay_string, "Amount Paid:$%02d.%02d", (smart_card.deduct_amount/100), (smart_card.deduct_amount%100));
		printSharp16x24(ldisplay_string,150,5,0);
	}
	sprintf((char*)ldisplay_string, "New Balance:$%02d.%02d", (acdb.balance/100), (acdb.balance%100));
	printSharp16x24(ldisplay_string,190,5,0);

	Update_RTC_on_Display();
	// Clear all CC flags
	ccard_dataread_flag = FALSE;
	card_removed_flag = FALSE;
	check_card_read = FALSE;
	Screen_Clear = true;
	cardread = 0; //this is avoid please wait msg during CC payment //vinay
	//Debug_Output1(0,"cardread SC pay  app screen:%ld", cardread);

}

void CC_Payment_Declined_Screen()
{
	Clear_Screen();
	//Update_Parking_Clock_Screen(0);
	//Debug_TextOut(0,"CC payment declined screen");
	printSharp32x48("DECLINED", 150, 10, 0);
	Update_RTC_on_Display();
	// Clear all CC flags
	ccard_dataread_flag = FALSE;
	card_removed_flag = FALSE;
	check_card_read = FALSE;
	Screen_Clear = true;
	cardread = 0; //this is avoid please wait msg during CC payment //vinay
	//Debug_Output1(0,"cardread cc pay decl scren:%ld", cardread);
}

void CC_Card_Expired_Screen()
{
	Clear_Screen();
	//Update_Parking_Clock_Screen(0);
	//Debug_TextOut(0,"CC card expired screen");
	printSharp32x48("CARD EXPIRED", 120, 1, 0);
	Update_RTC_on_Display();
	// Clear all CC flags
	ccard_dataread_flag = FALSE;
	card_removed_flag = FALSE;
	check_card_read = FALSE;
	Screen_Clear = true;
	cardread = 0; //this is avoid please wait msg during CC payment //vinay
	//Debug_Output1(0,"cardread cc card exp scr:%ld", cardread);
}

void SC_Card_Invalid_Screen()
{
	Clear_Screen();
	//Update_Parking_Clock_Screen(0);
	//Debug_TextOut(0,"SC card invalid screen");
	printSharp32x48("CARD INVALID", 120, 1, 0);
	Update_RTC_on_Display();
	// Clear all CC flags
	ccard_dataread_flag = FALSE;
	card_removed_flag = FALSE;
	check_card_read = FALSE;
	Screen_Clear = true;
	cardread = 0; //this is avoid please wait msg during CC payment //vinay
	//Debug_Output1(0,"cardread sc card iinvalid scr:%ld", cardread);
}

void SC_Card_Low_Fund_Screen()
{
	Clear_Screen();
	//Update_Parking_Clock_Screen(0);
	//Debug_TextOut(0,"SC card low fund screen");
	printSharp16x24("INSUFFICIENT FUND IN CARD", 100, 0, 0);
	printSharp16x24("   PLEASE PAY BY CASH", 140, 0, 0);
	printSharp16x24("  	  OR CREDIT CARD", 180, 0, 0);
	Update_RTC_on_Display();
	// Clear all CC flags
	ccard_dataread_flag = FALSE;
	card_removed_flag = FALSE;
	check_card_read = FALSE;
	Screen_Clear = true;
	cardread = 0; //this is avoid please wait msg during CC payment //vinay
	//Debug_Output1(0,"cardread SC card low fund scr:%ld", cardread);
}

void SC_Payment_Disabled_Screen()
{
	Clear_Screen();
	//Update_Parking_Clock_Screen(0);
	//Debug_TextOut(0,"SC payment disabled screen");
	printSharp16x24("    SMARTCARD PAYMENT   ", 120, 0, 0);
	printSharp16x24("    DISABLED IN CONFIG  ", 160, 0, 0);
	Update_RTC_on_Display();
	// Clear all CC flags
	ccard_dataread_flag = FALSE;
	card_removed_flag = FALSE;
	check_card_read = FALSE;
	smart_card_mode = FALSE; //LNGSIT-490
	Screen_Clear = true;
	cardread = 0; //this is avoid please wait msg during CC payment //vinay
	//Debug_Output1(0,"cardread SC payment disabled scr:%ld", cardread);
}

void Card_Not_Supported_Screen()
{
	Clear_Screen();
	//Update_Parking_Clock_Screen(0);
	//Debug_TextOut(0,"Card not supported screen");
	printSharp16x24("  CARD NOT SUPPORTED  ", 120, 2, 0);
	Update_RTC_on_Display();
	// Clear all CC flags
	ccard_dataread_flag = FALSE;
	card_removed_flag = FALSE;
	check_card_read = FALSE;
	Screen_Clear = true;
	cardread = 0; //this is avoid please wait msg during CC payment //vinay
	//Debug_Output1(0,"cardread card not sup scr:%ld", cardread);
}

void Coin_Calibration_Screen(uint16_t coin_value_to_show, uint8_t coin_value_index, uint8_t total_coins_to_show, uint8_t currently_calib_index)
{
	if(coin_value_index == 1)
	{
		Clear_Screen();

		printSharp16x24("Calibration Mode", 2, 10, 0);

		if(currently_calib_index >= MAX_COINS_SUPPORTED)
		{
			printSharp16x24("Multipart(Overlap) Coin", 28, 1, 0);
		}

		//printSharp_Card(fontGIF4, 80, 21, 9, 648/9);
		printSharp16x24("Insert Coin", 180, 10, 0);

		liberty_sprintf((char *)calibration_display_buf,"Coin Value: $%d.%02d",(coin_value_to_show/100), (coin_value_to_show%100));
		printSharp16x24(calibration_display_buf, 160, 10, 0);

		Update_RTC_on_Display();
	}

	liberty_sprintf((char *)calibration_display_buf,"Please Insert %d of %d", coin_value_index, total_coins_to_show);
	diag_text_Screen( calibration_display_buf, TRUE, FALSE );
}



void Idle_Screen()
{
	Screen_TimeOut_RTC =  RTC_epoch_now();
	//Debug_Output1(0,"Screen_TimeOut_RTC Idle_Screen:%ld", Screen_TimeOut_RTC);
	//Debug_TextOut( 0, "Idle_Screen" );
	//Debug_Output1(0,"cardread idlescr:%ld", cardread);

	uint32_t loCurrentRTCTime = RTC_epoch_now();
	//Debug_Output1( 0, "coindrop_TimeOut_RTC: %d",coindrop_TimeOut_RTC);
	//Debug_Output1( 0, "KeyPress_TimeOut_RTC: %d",KeyPress_TimeOut_RTC);
	//Debug_Output1( 0, "loCurrentRTCTime: %d",loCurrentRTCTime);
	//Debug_Output1( 0, "Current_Space_Id: %d",Current_Space_Id);
	//Debug_Output1(0,"Screen_TimeOut_RTC Idle_Screen:%d", Screen_TimeOut_RTC);


	if(gAutoCalibration == true)	//In calibration mode
		return;

	uint8_t hours_to_display = 0;
	uint8_t minutes_to_display = 0;
	uint16_t temp_minutes_to_disp = 0;
	//uint8_t ldisplay_string[15] = { 0 };
	//uint16_t bay_number_to_display = 0;

	if((Tech_menu == FALSE) && (card_removed_flag == FALSE) && (smart_card_mode == FALSE) && (meter_out_of_service == FALSE) && (gl_Commission_Card == FALSE)
			&&(do_CC_OLT == FALSE))
	{
		if(current_rate_index == FREE_PARKING)
		{
			//Debug_TextOut(0,"METER IN FREE PARKING\n");
			Freeparking_Screen();

			//if(holiday_spl_event_found == TRUE) //Allowing bottom 2 lines msg display for FREE_PARKING
			{
				//Display Message 3
				printSharp16x24(glSystem_cfg.schedule_display_messages[sch_message_index_3 - 1].display_string, 152, 1, 0);

				//Display Message 4
				printSharp16x24(glSystem_cfg.schedule_display_messages[sch_message_index_4 - 1].display_string, 184, 1, 0);
			}
		}
		else if(current_rate_index == NO_PARKING)
		{
			//Debug_TextOut(0,"METER IN NO PARKING\n");
			Noparking_Screen();
			//if(holiday_spl_event_found == TRUE) //Allowing bottom 2 lines msg display for NO_PARKING
			{
				//Display Message 3
				printSharp16x24(glSystem_cfg.schedule_display_messages[sch_message_index_3 - 1].display_string, 152, 1, 0);

				//Display Message 4
				printSharp16x24(glSystem_cfg.schedule_display_messages[sch_message_index_4 - 1].display_string, 184, 1, 0);
			}
		}
		else
		{
			hours_to_display = parking_time_left_on_meter[Current_Space_Id]/3600;
			temp_minutes_to_disp = (parking_time_left_on_meter[Current_Space_Id] % 3600);
			minutes_to_display = temp_minutes_to_disp/60;

			if((temp_minutes_to_disp % 60) > 0)	//This is to fix the wrong update of parking clock
			{
				minutes_to_display += 1;
				if(minutes_to_display > 59)
				{
					hours_to_display += 1;
					minutes_to_display = 0;
				}
			}

			if((Current_Space_Id >= (glSystem_cfg.MSM_max_spaces)) || (Current_Space_Id <= 0)|| (Screen_Clear == true) )
			{
				//Debug_TextOut( 0, "Came here" );
				//Clear_Screen();					//making creating a blank screen for invalid coins
				Screen_Clear=false;
			}
			if((((coindrop_TimeOut_RTC > 0) && (loCurrentRTCTime > (coindrop_TimeOut_RTC + 30))) || (loCurrentRTCTime < coindrop_TimeOut_RTC)) &&
					(((KeyPress_TimeOut_RTC > 0) && (loCurrentRTCTime > (KeyPress_TimeOut_RTC + 30))) || (loCurrentRTCTime < KeyPress_TimeOut_RTC)))
			{
				//Debug_Output1( 0, "coindrop_TimeOut_RTC: %d\r\n",coindrop_TimeOut_RTC);
				//Debug_Output1( 0, "KeyPress_TimeOut_RTC: %d\r\n",KeyPress_TimeOut_RTC);
				//Debug_Output1( 0, "loCurrentRTCTime: %d\r\n",loCurrentRTCTime);
				coindrop_TimeOut_RTC = 0;
				KeyPress_TimeOut_RTC = 0;
				DSM_Screen1();
			}
			else if((((loCurrentRTCTime > (coindrop_TimeOut_RTC + 30))) || (loCurrentRTCTime < coindrop_TimeOut_RTC)) &&
					(((KeyPress_TimeOut_RTC > 0) && (loCurrentRTCTime > (KeyPress_TimeOut_RTC + 30))) || (loCurrentRTCTime < KeyPress_TimeOut_RTC)))
			{
				//Debug_Output1( 0, "coindrop_TimeOut_RTC: %d\r\n",coindrop_TimeOut_RTC);
				//Debug_Output1( 0, "KeyPress_TimeOut_RTC: %d\r\n",KeyPress_TimeOut_RTC);
				//Debug_Output1( 0, "loCurrentRTCTime: %d\r\n",loCurrentRTCTime);
				coindrop_TimeOut_RTC = 0;
				KeyPress_TimeOut_RTC = 0;
				DSM_Screen1();
			}
			else if((((coindrop_TimeOut_RTC > 0) && (loCurrentRTCTime > (coindrop_TimeOut_RTC + 30))) || (loCurrentRTCTime < coindrop_TimeOut_RTC)) &&
					(((loCurrentRTCTime > (KeyPress_TimeOut_RTC + 30))) || (loCurrentRTCTime < KeyPress_TimeOut_RTC)))
			{
				//Debug_Output1( 0, "coindrop_TimeOut_RTC: %d\r\n",coindrop_TimeOut_RTC);
				//Debug_Output1( 0, "KeyPress_TimeOut_RTC: %d\r\n",KeyPress_TimeOut_RTC);
				//Debug_Output1( 0, "loCurrentRTCTime: %d\r\n",loCurrentRTCTime);
				coindrop_TimeOut_RTC = 0;
				KeyPress_TimeOut_RTC = 0;
				DSM_Screen1();
			}
			else if((!key_press)&&(coindrop_TimeOut_RTC == 0)&&(loCurrentRTCTime > (Last_Cash_Payment_RTC + 30)))	//// VT
				DSM_Screen1();
			else if((!key_press)&&(coindrop_TimeOut_RTC == 0))
				{
					DSM_Screen1();//DSM_Screen2(); // its giving random time like 136:22 something in left space //vinay
				}

			Update_Parking_Clock_Screen();


			Update_RTC_on_Display();
		}
	}

	if((meter_out_of_service == TRUE) && (Tech_menu == FALSE))
	{
		Meter_Out_Of_Serice_Screen();
	}
}

#if	0
void Idle_Screen()
{
	if(Meter_Reset == TRUE)
	{
		if ((Max_parking_time > 0) && (Max_parking_time != 0xFFFFFFFF))   //once we are given time we clearing for next time.
		{
			Flash_Batch_Flag_Read();
			get_earned_seconds(100, false); //dummy call to refresh the max time allowed variable
			if(Max_parking_time > max_time_in_current_rate)
			{
				parking_time_left_on_meter[Current_Space_Id] = (max_time_in_current_rate*60);
			}
			else
			{
				parking_time_left_on_meter[Current_Space_Id] = Max_parking_time;
			}

			Max_parking_time=0;
			Flash_Batch_Flag_Write();
		}
	}

	if((Tech_menu == FALSE) && (card_removed_flag == FALSE) && (smart_card_mode == FALSE) && (meter_out_of_service == FALSE) && (gl_Commission_Card == FALSE)
			&&(do_CC_OLT == FALSE))
	{
		Clear_Screen();

		if(current_rate_index == FREE_PARKING)
		{
			//Debug_TextOut(0,"METER IN FREE PARKING\n");
			Freeparking_Screen();
		}
		else if(current_rate_index == NO_PARKING)
		{
			//Debug_TextOut(0,"METER IN NO PARKING\n");
			Noparking_Screen();
			if(holiday_spl_event_found == TRUE)
			{
				//Display Message 3
				printSharp16x24(glSystem_cfg.schedule_display_messages[sch_message_index_3 - 1].display_string, 152, 1, 0);

				//Display Message 4
				printSharp16x24(glSystem_cfg.schedule_display_messages[sch_message_index_4 - 1].display_string, 184, 1, 0);
			}
		}
		else
		{
			Update_Parking_Clock_Screen(0);
			if(glSystem_cfg.graphic_display_enable == 1)
			{
				InsertCardCoin_Screen();
			}
			else
			{
				if(glSystem_cfg.display_messages[2].font_size == FONT_SIZE_SMALL)
					printSharp16x24(glSystem_cfg.display_messages[2].display_string, 88, 1, 0);
				else
					printSharp32x48(glSystem_cfg.display_messages[2].display_string, 88, 1, 0);

				if(glSystem_cfg.display_messages[2].font_size == FONT_SIZE_SMALL)
					printSharp16x24(glSystem_cfg.display_messages[3].display_string, 120, 1, 0);
				else
					printSharp32x48(glSystem_cfg.display_messages[3].display_string, 120, 1, 0);
			}

			if(rtc_c_calender.dayOfWeek == 5)
			{
				//Debug_Output1(0, "FRI DOW = %d", rtc_c_calender.dayOfWeek);
				memcpy(glSystem_cfg.display_messages[0].display_string, "FRIDAY           9AM-9PM", sizeof(glSystem_cfg.display_messages[0].display_string));
				if(glSystem_cfg.display_messages[0].font_size == FONT_SIZE_SMALL)
					printSharp16x24(glSystem_cfg.display_messages[0].display_string, 152, 1, 0);
				else
					printSharp32x48(glSystem_cfg.display_messages[0].display_string, 152, 1, 0);
			}
			else if(rtc_c_calender.dayOfWeek == 6)
			{
				//Debug_Output1(0, "SAT DOW = %d", rtc_c_calender.dayOfWeek);
				memcpy(glSystem_cfg.display_messages[0].display_string, "SATURDAY         9AM-6PM", sizeof(glSystem_cfg.display_messages[0].display_string));
				if(glSystem_cfg.display_messages[0].font_size == FONT_SIZE_SMALL)
					printSharp16x24(glSystem_cfg.display_messages[0].display_string, 152, 1, 0);
				else
					printSharp32x48(glSystem_cfg.display_messages[0].display_string, 152, 1, 0);
			}
			else
			{
				//Debug_Output1(0, "OTHER DOW = %d", rtc_c_calender.dayOfWeek);
				memcpy(glSystem_cfg.display_messages[0].display_string, "MON-THU          9AM-6PM", sizeof(glSystem_cfg.display_messages[0].display_string));
				if(glSystem_cfg.display_messages[0].font_size == FONT_SIZE_SMALL)
					printSharp16x24(glSystem_cfg.display_messages[0].display_string, 152, 1, 0);
				else
					printSharp32x48(glSystem_cfg.display_messages[0].display_string, 152, 1, 0);
			}

			if(glSystem_cfg.display_messages[1].font_size == FONT_SIZE_SMALL)
				printSharp16x24(glSystem_cfg.display_messages[1].display_string, 184, 1, 0);
			else
				printSharp32x48(glSystem_cfg.display_messages[1].display_string, 184, 1, 0);

			Update_RTC_on_Display();
		}
	}

	if((meter_out_of_service == TRUE) && (Tech_menu == FALSE))
	{
		Meter_Out_Of_Serice_Screen();
	}
}
#endif

void Remove_Card_Screen()
{
	Clear_Screen();
	//Update_Parking_Clock_Screen(0);
	//Debug_TextOut( 0, "Remove card Screen" );
	printSharp_Card(fontGIF2, 80, 21, 8, 488/8);
	printSharp16x24("Please Remove Card", 170, 8, 0);
	Update_RTC_on_Display();
	Screen_Clear = true;
	cardread = 1; //this is avoid please wait msg during CC payment //vinay
	//Debug_Output1(0,"cardread remove card scr:%ld", cardread);
}

void Meter_Out_Of_Serice_Screen()
{
	Clear_Screen();
	Noparking_Screen();
	printSharp16x24("OUT OF SERVICE", 150, 10, 0); //LNGSIT-1001, LNGSIT-839
	Update_RTC_on_Display();
	Screen_Clear = true;
}

void diag_text_Screen(uint8_t * text_to_lcd, uint8_t force_on_lcd, uint8_t quick_text)
{
	uint8_t diag_length = strlen((const char*)text_to_lcd);
	uint8_t spaces_to_add = 0;

	Debug_TextOut(0, (const char*)text_to_lcd); //LNGSIT-1037

	init_display_SPI_module_GPIO();
	if(diag_length > MAX_CHAR_ON_LCD)
	{
		diag_length = MAX_CHAR_ON_LCD;
		text_to_lcd[diag_length-1] = 0;	//Null Terminate
	}
	//Debug_TextOut(0, (const char*)text_to_lcd); //LNGSIT-1037
	spaces_to_add = (MAX_CHAR_ON_LCD - diag_length)/2;	//Centre formatting
	if((force_on_lcd == TRUE) || (glIn_diag_mode == TRUE))
	{
		if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
		{
			printSharp16x24(text_to_lcd, 210, (1+spaces_to_add), 0);
		}
		else if(quick_text == true)	//Overwriting the RTC line only to show processing
		{
			printSharp16x24(text_to_lcd, 210, 0, 0);
			printSharp16x24(" ", 234, 0, 0);
		}
		else
		{
			printSharp16x24(text_to_lcd, 184, (1+spaces_to_add), 0);
		}
		if(quick_text == false)
			DelayMs(250);
	}
	Screen_Clear = true;
}

void LCD_Backlight(uint8_t backlight_on)
{
	uint8_t NM_LCD_ON_HOUR = 0, NM_LCD_OFF_HOUR = 0;
	uint8_t AO_LCD_ON_HOUR = 0, AO_LCD_OFF_HOUR = 0;

	NM_LCD_ON_HOUR = (glSystem_cfg.NM_LCD_TIME_ON)/10;
	NM_LCD_OFF_HOUR = (glSystem_cfg.NM_LCD_TIME_OFF)/10;

	AO_LCD_ON_HOUR = (glSystem_cfg.AO_LCD_TIME_ON)/10;
	AO_LCD_OFF_HOUR = (glSystem_cfg.AO_LCD_TIME_OFF)/10;

	if((rtc_c_calender.month == 11) || (rtc_c_calender.month == 12) || (rtc_c_calender.month == 1) ||
		(rtc_c_calender.month == 2) || (rtc_c_calender.month == 3)) //winter months (Nov-Mar)
	{
		gl_RTCHOUR=RTCHOUR;
		Check_LCD_Backlight_Power_Save_Hours(NM_LCD_ON_HOUR, NM_LCD_OFF_HOUR);

		if((rtc_hour_flag[gl_RTCHOUR]&0X0F)==0)
		{
			if((gl_RTCHOUR!=0)&&((rtc_hour_flag[gl_RTCHOUR-1]&0X0F)==0))
			{
				//Debug_TextOut(0, "Nov-Mar LCD backlight OFF Time-01");
				LCD_BKLT_ON = FALSE;
				BKLT_OFF();
			}
			else if((rtc_hour_flag[gl_RTCHOUR] & 0xf0) == 0X10)
			{
				//Debug_TextOut(0, "Nov-Mar LCD backlight OFF Time-02");
				LCD_BKLT_ON = FALSE;
				BKLT_OFF();
			}
		}
		else if((rtc_hour_flag[gl_RTCHOUR]&0X0F)==1)
		{
			if(((rtc_hour_flag[gl_RTCHOUR]&0xf0)==0X30))
			{
				if(backlight_on == TRUE)
				{
					//Debug_TextOut(0, "Nov-Mar LCD backlight ON Time-01");
					LCD_BKLT_ON = TRUE;
					BKLT_TimeOut_RTCTimestamp = RTC_epoch_now();
					BKLT_ON();
				}
				else
				{
					LCD_BKLT_ON = FALSE;
					BKLT_OFF();
				}
			}
		}
	}
	else if((rtc_c_calender.month >= 4) && (rtc_c_calender.month <= 10)) //summer months (Apr-Oct)
	{
		gl_RTCHOUR=RTCHOUR;
		Check_LCD_Backlight_Power_Save_Hours(AO_LCD_ON_HOUR, AO_LCD_OFF_HOUR);

		if((rtc_hour_flag[gl_RTCHOUR]&0X0F)==0)
		{
			if((gl_RTCHOUR!=0)&&((rtc_hour_flag[gl_RTCHOUR-1]&0X0F)==0))
			{
				//Debug_TextOut(0, "Apr-Oct LCD backlight OFF Time-01");
			    if((No_Modem_Coin_Only == true) || (glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE))
			    {
                    LCD_BKLT_ON = TRUE;
                    BKLT_TimeOut_RTCTimestamp = RTC_epoch_now();
                    BKLT_ON();
			    }
			    else
			    {
			        LCD_BKLT_ON = FALSE;
			        BKLT_OFF();
			    }
			}
			else if((rtc_hour_flag[gl_RTCHOUR] & 0xf0) == 0X10)
			{
				//Debug_TextOut(0, "Apr-Oct LCD backlight OFF Time-02");
                if((No_Modem_Coin_Only == true) || (glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE))
                {
                    LCD_BKLT_ON = TRUE;
                    BKLT_TimeOut_RTCTimestamp = RTC_epoch_now();
                    BKLT_ON();
                }
                else
                {
                    LCD_BKLT_ON = FALSE;
                    BKLT_OFF();
                }
			}
		}
		else if((rtc_hour_flag[gl_RTCHOUR]&0X0F)==1)
		{
			if(((rtc_hour_flag[gl_RTCHOUR]&0xf0)==0X30))
			{
				if(backlight_on == TRUE)
				{
					//Debug_TextOut(0, "Apr-Oct LCD backlight ON Time-01");
					LCD_BKLT_ON = TRUE;
					BKLT_TimeOut_RTCTimestamp = RTC_epoch_now();
					BKLT_ON();
				}
				else
				{
	                if((No_Modem_Coin_Only == true) || (glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE))
	                {
	                    LCD_BKLT_ON = TRUE;
	                    BKLT_TimeOut_RTCTimestamp = RTC_epoch_now();
	                    BKLT_ON();
	                }
	                else
	                {
	                    LCD_BKLT_ON = FALSE;
	                    BKLT_OFF();
	                }
				}
			}
		}
	}
	else
	{
		Debug_TextOut(0, "Non-Season, shouldn't be here");
	}
}

void Check_LCD_Backlight_Power_Save_Hours(uint8_t ON_HOUR,uint8_t OFF_HOUR)
{
	uint8_t first_position=0xff,i=0;
	memset(rtc_hour_flag,0xFF,sizeof(rtc_hour_flag));

	if(OFF_HOUR<=23)
		rtc_hour_flag[OFF_HOUR]=0x10;
	if(ON_HOUR<=23)
		rtc_hour_flag[ON_HOUR]=0x31;

	for(i=0;i<23;i++)
	{
		if(((rtc_hour_flag[i] & 0x0F)==0)||((rtc_hour_flag[i] & 0X0F)==1))
		{
			if(first_position==0xff)
				first_position=i;
			if(rtc_hour_flag[i+1]==0xff)
				rtc_hour_flag[i+1]=rtc_hour_flag[i];
		}
	}
	if(first_position!=0)
	{
		for(i=0;i<first_position;i++)
		{
			if(rtc_hour_flag[i]==0xff)
			{
				if(i==0)
				{
					rtc_hour_flag[i]=rtc_hour_flag[i+23];
				}
				else
				{
					rtc_hour_flag[i]=rtc_hour_flag[i-1];
				}
			}
		}
	}
}

void RefundShowAmount_Screen(uint16_t lamount_selected, uint32_t min_max_display,uint8_t screen_refresh)
{
	init_display_SPI_module_GPIO();
	uint8_t ldisplay_string[15] = { 0 };

	get_earned_seconds(0XFF, false, false);//dummy call to refresh the max time allowed variable
	if(parking_time_left_on_meter[Current_Space_Id] >= (max_time_in_current_rate - 60))
	{
		Debug_TextOut(0,"Refund: Meter Full");
		MeterFull_Screen();
		Meter_Full_Flg[Current_Space_Id] = TRUE;
		return;
	}

	if(screen_refresh == TRUE)
	{
		Clear_Screen();
		Update_Parking_Clock_Screen();
	}

	if(smart_card_mode == TRUE)
	{
		sprintf((char*)ldisplay_string, "Card Balance:$%02d.%02d", (smart_card.balance/100), (smart_card.balance % 100));
		printSharp16x24(ldisplay_string,75,5,0);
	}

	if(smart_card_mode == TRUE)	//if minimum
	{
		sprintf((char*)ldisplay_string, "Will Refund:$%02d.%02d", (min_max_display/100),(min_max_display %100));
		printSharp16x24(ldisplay_string, 120, 5, 0);
	}

	if(screen_refresh == TRUE)
	{
		printSharp16x24("Press OK to Refund", 160, 5, 0);
		printSharp16x24("OR Cancel to Top Up", 190, 5, 0);
	}

}

void RemoveDatakey_Screen()  //vinay for coin only meter to copy from datakey
{
	Clear_Screen();
	//Update_Parking_Clock_Screen();
	printSharp32x48("   REMOVE     ", 0, 0, 0);
	printSharp32x48("   DATAKEY    ", 60, 0, 0);
	printSharp32x48("TO REPROGRAM  ", 120, 0, 0);
	//printSharp32x48("METER FULL", 150, 5, 0);
	//Update_RTC_on_Display();
}

void DSM_Screen7()
{
	Screen_TimeOut_RTC =  RTC_epoch_now();
	//Debug_Output1(0,"Screen_TimeOut_RTC DSM_Scr7:%ld", Screen_TimeOut_RTC);
	//Debug_TextOut( 0, "DSM_Screen7" );
	Clear_Screen();
	printSharp16x24("  USE ARROW BUTTONS TO", 10, 1, 0);
	printSharp16x24("      SELECT SPACE ",60,0,0);

	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
		sprintf((char*)parking_clock, "?LEFT RIGHT@");
	else
		sprintf((char*)parking_clock, "[LEFT RIGHT/");
	printSharp32x48HfScr(parking_clock, 120,1,0); //40 to 60
	printSharp16x24("  THEN RE-INSERT CARD   ", 184, 1, 0);
	cardread = 0;
	//Debug_Output1(0,"cardread DSM Scr7:%ld", cardread);
}

void DSM_Screen6()
{
	Screen_TimeOut_RTC =  RTC_epoch_now();
	//Debug_Output1(0,"Screen_TimeOut_RTC DSM_Scr6:%ld", Screen_TimeOut_RTC);
	Clear_Screen();
	//Debug_TextOut( 0, "DSM_Screen6" );
	cardread = 0;
	//Debug_Output1(0,"cardread DSM_scr6:%ld", cardread);
	Current_Space_Id = key_RL;

	printSharp16x24("  USE ARROW BUTTONS TO", 30, 1, 0);
	printSharp16x24("      SELECT SPACE ",80,0,0);
	//printSharp32x48("USE BUTTONS", 2, 1, 0);
	//printSharp32x48("SELECT SPACE ",60,0,0);

	if(Current_Space_Id ==  RIGHT_BAY)
	{
		//Debug_TextOut( 0, "DSM_Screen6 right" );
		if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
			sprintf((char*)parking_clock, "?LEFT RIGHT@");
		else
			sprintf((char*)parking_clock, "[LEFT RIGHT/");
		printSharp32x48HfScr(parking_clock, 120,1,DISP_INVERT_RIGHT); //40 to 60
	}
	else if(Current_Space_Id == LEFT_BAY)
	{
		//Debug_TextOut( 0, "DSM_Screen6 left" );
		if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
			sprintf((char*)parking_clock, "?LEFT RIGHT@");
		else
			sprintf((char*)parking_clock, "[LEFT RIGHT/");
		printSharp32x48HfScr(parking_clock, 120,1,DISP_INVERT_LEFT); //40 to 60
	}
	else
	{
		//Debug_TextOut( 0, "DSM_Screen6 nothing" );
		if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
			sprintf((char*)parking_clock, "?LEFT RIGHT@");
		else
			sprintf((char*)parking_clock, "[LEFT RIGHT/");
		printSharp32x48HfScr(parking_clock, 120,1,0); //40 to 60
	}

	//printSharp16x24("  INSERT CARD OR COINS  ", 190, 1, 0);
	//printSharp16x24("   INSERT COINS ONLY    ", 190, 1, 0);
	//sprintf((char*)parking_clock, "%02d:%02d %02d:%02d",hours_to_display_L, minutes_to_display_L, hours_to_display_R, minutes_to_display_R);
	//printSharp32x48HfScr(parking_clock, 170,1,0); //100 to 130
	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
		printSharp16x24(" C-CANCEL     OK-SELECT ", 184, 1, 0);
	else
		printSharp16x24(" OK-SELECT     C-CANCEL", 184, 1, 0);
}

void DSM_Screen5()
{
	Screen_TimeOut_RTC =  RTC_epoch_now();
	//Debug_Output1(0,"Screen_TimeOut_RTC DSM_Scr5:%ld", Screen_TimeOut_RTC);
	//Debug_TextOut( 0, "DSM_Screen5 payment on hold" );
	Clear_Screen();
	Current_Space_Id = key_RL;
	CoinOk();
	DelayMs(1000);
	payment_on_hold = false;
	//DSM_Screen3();
	//printSharp16x24("USE BUTTON TO SELECT BAY", 2, 1, 0);
	//printSharp16x24("  LAST PAYMENT FAILED   ", 80, 1, 0);
	//printSharp16x24("   DUE TO NO ACTION     ", 130, 1, 0);


}

void DSM_Screen4()
{
	Screen_TimeOut_RTC =  RTC_epoch_now();
	//Debug_Output1(0,"Screen_TimeOut_RTC DSM_Scr4:%ld", Screen_TimeOut_RTC);
	//Debug_TextOut( 0, "DSM_Screen4" );
	Clear_Screen();
	Current_Space_Id = key_RL;
	printSharp16x24("  USE ARROW BUTTONS TO", 10, 1, 0);
	printSharp16x24("      SELECT SPACE ",60,0,0);

	if(Current_Space_Id ==  RIGHT_BAY)
	{
		//Debug_TextOut( 0, "DSM_Screen4 right" );
		if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
			sprintf((char*)parking_clock, "?LEFT RIGHT@");
		else
			sprintf((char*)parking_clock, "[LEFT RIGHT/");
		printSharp32x48HfScr(parking_clock, 120,1,DISP_INVERT_RIGHT); //40 to 60
	}
	else if(Current_Space_Id == LEFT_BAY)
	{
		//Debug_TextOut( 0, "DSM_Screen4 left" );
		if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
			sprintf((char*)parking_clock, "?LEFT RIGHT@");
		else
			sprintf((char*)parking_clock, "[LEFT RIGHT/");
		printSharp32x48HfScr(parking_clock, 120,1,DISP_INVERT_LEFT); //40 to 60
	}
	else
	{
		//Debug_TextOut( 0, "DSM_Screen4 nothing" );
		if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
			sprintf((char*)parking_clock, "?LEFT RIGHT@");
		else
			sprintf((char*)parking_clock, "[LEFT RIGHT/");
		printSharp32x48HfScr(parking_clock, 120,1,0); //40 to 60
	}

	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
		printSharp16x24(" C-CANCEL     OK-SELECT ", 184, 1, 0);
	else
		printSharp16x24(" OK-SELECT     C-CANCEL", 184, 1, 0);

}

void DSM_Screen3()
{
	Screen_TimeOut_RTC =  RTC_epoch_now();
	Clear_Screen();
	//Debug_Output1(0,"Screen_TimeOut_RTC DSM_Scr3:%ld", Screen_TimeOut_RTC);
	//Debug_TextOut( 0, "DSM_Screen3" );
	//Debug_Output1(0,"cardread DSM scr3:%ld", cardread);
	if(Current_Space_Id == RIGHT_BAY)
	{
		//Debug_TextOut( 0, "DSM_Screen3_RIGHT" );
		Clear_Screen();
		//printSharp16x24("METER COVERS TWO SPACES", 2, 1, 0);
		//printSharp32x48("RIT SELECTED", 2, 1, 0);
		printSharp16x24("  RIGHT SPACE SELECTED  ", 2, 1, 0);
		//printSharp16x24("USE ARROW 2 CHANGE SPACE", 40, 1, 0);
		if(((time_retain[LEFT_BAY]==0)&&(min_time_insec<=0))||(time_retain[LEFT_BAY]==1)||(parking_time_left_on_meter[LEFT_BAY]>=min_time_insec))
		{
			hours_to_display_L = (parking_time_left_on_meter[LEFT_BAY])/3600;
			temp_minutes_to_disp = ((parking_time_left_on_meter[LEFT_BAY]) % 3600);
			minutes_to_display_L = temp_minutes_to_disp/60;
			if((temp_minutes_to_disp % 60) > 0)	//This is to fix the wrong update of parking clock
			{
				minutes_to_display_L += 1;
				if(minutes_to_display_L > 59)
				{
					hours_to_display_L += 1;
					minutes_to_display_L = 0;
				}
			}
		}
		else if((min_time_insec != 0)&&((minutes_left_in_current_schedule*60) <= min_time_insec))
		{
			hours_to_display_L = (parking_time_left_on_meter[LEFT_BAY])/3600;
			temp_minutes_to_disp = ((parking_time_left_on_meter[LEFT_BAY]) % 3600);
			minutes_to_display_L = temp_minutes_to_disp/60;
			if((temp_minutes_to_disp % 60) > 0)	//This is to fix the wrong update of parking clock
			{
				minutes_to_display_L += 1;
				if(minutes_to_display_L > 59)
				{
					hours_to_display_L += 1;
					minutes_to_display_L = 0;
				}
			}
			time_retain[LEFT_BAY]=1;
		}
		else
		{
			hours_to_display_L = 0;
			minutes_to_display_L  = 0;
		}

		//printSharp16x24("  INSERT CARD OR COINS  ", 190, 1, 0);
		//printSharp16x24("   INSERT COINS ONLY    ", 190, 1, 0);
		if(((time_retain[RIGHT_BAY]==0)&&(min_time_insec<=0))||(time_retain[RIGHT_BAY]==1)||(parking_time_left_on_meter[RIGHT_BAY]>=min_time_insec))
		{
			hours_to_display_R = (parking_time_left_on_meter[RIGHT_BAY])/3600;
			temp_minutes_to_disp = ((parking_time_left_on_meter[RIGHT_BAY]) % 3600);
			minutes_to_display_R = temp_minutes_to_disp/60;
			if((temp_minutes_to_disp % 60) > 0)	//This is to fix the wrong update of parking clock
			{
				minutes_to_display_R += 1;
				if(minutes_to_display_R > 59)
				{
					hours_to_display_R += 1;
					minutes_to_display_R = 0;
				}
			}
		}
		else if((min_time_insec != 0)&&((minutes_left_in_current_schedule*60) <= min_time_insec))
		{
			hours_to_display_R = (parking_time_left_on_meter[RIGHT_BAY])/3600;
			temp_minutes_to_disp = ((parking_time_left_on_meter[RIGHT_BAY]) % 3600);
			minutes_to_display_R = temp_minutes_to_disp/60;
			if((temp_minutes_to_disp % 60) > 0)	//This is to fix the wrong update of parking clock
			{
				minutes_to_display_R += 1;
				if(minutes_to_display_R > 59)
				{
					hours_to_display_R += 1;
					minutes_to_display_R = 0;
				}
			}
			time_retain[RIGHT_BAY]=1;
		}
		else
		{
			hours_to_display_R = 0;
			minutes_to_display_R  = 0;
		}

		if((parking_time_left_on_meter_prev[RIGHT_BAY] == min_time_insec) && (min_time_insec > 0))
		{
			printSharp16x24("Minimum Purchase Reached   ", 40, 1, 0);
			parking_time_left_on_meter_prev[RIGHT_BAY] = 0;
		}
		else if((parking_time_left_on_meter[RIGHT_BAY]==0)||(time_retain[RIGHT_BAY]==1)||(min_time_insec==0))
			 printSharp16x24("  INSERT CARD OR COINS  ", 40, 1, 0); //110 to 88
		else printSharp16x24("Min Purchase not Reached", 40, 1, 0);

		if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
			sprintf((char*)parking_clock, "?LEFT RIGHT@");
		else
			sprintf((char*)parking_clock, "[LEFT RIGHT/");
		printSharp32x48HfScr(parking_clock, 80,1,DISP_INVERT_RIGHT); //40 to 60

		sprintf((char*)parking_clock, "%02d:%02d %02d:%02d ",hours_to_display_L, minutes_to_display_L, hours_to_display_R, minutes_to_display_R);
		printSharp32x48HfScr(parking_clock, 130,1,DISP_INVERT_RIGHT); //100 to 130

	//	printSharp16x24("  INSERT CARD OR COINS  ", 190, 1, 0);
		Debug_Output2( 0, "ParkingClock Right: %02d:%02d\r\n",hours_to_display_R,minutes_to_display_R);
	}
	else if(Current_Space_Id == LEFT_BAY)
	{
		//Debug_TextOut( 0, "DSM_Screen3_LEFT" );
		Clear_Screen();
		//printSharp16x24("METER COVERS TWO SPACES", 2, 1, 0);
		//printSharp32x48("LEF SELECTED", 2, 1, 0);
		  printSharp16x24("  LEFT SPACE SELECTED  ", 2, 1, 0);
		 // printSharp16x24("USE ARROW 2 CHANGE SPACE", 40, 1, 0);
		if(((time_retain[RIGHT_BAY]==0)&&(min_time_insec<=0))||(time_retain[RIGHT_BAY]==1)||(parking_time_left_on_meter[RIGHT_BAY]>=min_time_insec))
		{
			hours_to_display_R = (parking_time_left_on_meter[RIGHT_BAY])/3600;
			temp_minutes_to_disp = ((parking_time_left_on_meter[RIGHT_BAY]) % 3600);
			minutes_to_display_R = temp_minutes_to_disp/60;
			if((temp_minutes_to_disp % 60) > 0)	//This is to fix the wrong update of parking clock
			{
				minutes_to_display_R += 1;
				if(minutes_to_display_R > 59)
				{
					hours_to_display_R += 1;
					minutes_to_display_R = 0;
				}
			}
		}
		else if((min_time_insec != 0)&&((minutes_left_in_current_schedule*60) <= min_time_insec))
		{
			hours_to_display_R = (parking_time_left_on_meter[RIGHT_BAY])/3600;
			temp_minutes_to_disp = ((parking_time_left_on_meter[RIGHT_BAY]) % 3600);
			minutes_to_display_R = temp_minutes_to_disp/60;
			if((temp_minutes_to_disp % 60) > 0)	//This is to fix the wrong update of parking clock
			{
				minutes_to_display_R += 1;
				if(minutes_to_display_R > 59)
				{
					hours_to_display_R += 1;
					minutes_to_display_R = 0;
				}
			}
			time_retain[RIGHT_BAY]=1;
		}
		else
		{
			hours_to_display_R = 0;
			minutes_to_display_R = 0;
		}

		//printSharp16x24("   INSERT COINS ONLY    ", 190, 1, 0);
		if(((time_retain[LEFT_BAY]==0)&&(min_time_insec<=0))||(time_retain[LEFT_BAY]==1)||(parking_time_left_on_meter[LEFT_BAY]>=min_time_insec))
		{
			hours_to_display_L = (parking_time_left_on_meter[LEFT_BAY])/3600;
			temp_minutes_to_disp = ((parking_time_left_on_meter[LEFT_BAY]) % 3600);
			minutes_to_display_L = temp_minutes_to_disp/60;
			if((temp_minutes_to_disp % 60) > 0)	//This is to fix the wrong update of parking clock
			{
				minutes_to_display_L += 1;
				if(minutes_to_display_L > 59)
				{
					hours_to_display_L += 1;
					minutes_to_display_L = 0;
				}
			}
		}
		else if((min_time_insec != 0)&&((minutes_left_in_current_schedule*60) <= min_time_insec))
		{
			hours_to_display_L = (parking_time_left_on_meter[LEFT_BAY])/3600;
			temp_minutes_to_disp = ((parking_time_left_on_meter[LEFT_BAY]) % 3600);
			minutes_to_display_L = temp_minutes_to_disp/60;
			if((temp_minutes_to_disp % 60) > 0)	//This is to fix the wrong update of parking clock
			{
				minutes_to_display_L += 1;
				if(minutes_to_display_L > 59)
				{
					hours_to_display_L += 1;
					minutes_to_display_L = 0;
				}
			}
			time_retain[LEFT_BAY]=1;
		}
		else
		{
			hours_to_display_L = 0;
			minutes_to_display_L = 0;
		}

		if((parking_time_left_on_meter_prev[LEFT_BAY] == min_time_insec) && (min_time_insec > 0))
		{
			printSharp16x24("Minimum Purchase Reached   ", 40, 1, 0);
			parking_time_left_on_meter_prev[LEFT_BAY] = 0;
		}
		else if((parking_time_left_on_meter[LEFT_BAY]==0)||(time_retain[LEFT_BAY]==1)||(min_time_insec==0))
			 printSharp16x24("  INSERT CARD OR COINS  ", 40, 1, 0); //110 to 88
		else printSharp16x24("Min Purchase not Reached", 40, 1, 0);

		if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
			sprintf((char*)parking_clock, "?LEFT RIGHT@");
		else
			sprintf((char*)parking_clock, "[LEFT RIGHT/");
		printSharp32x48HfScr(parking_clock, 80,1,DISP_INVERT_LEFT); //40 to 60

		sprintf((char*)parking_clock, "%02d:%02d %02d:%02d ",hours_to_display_L, minutes_to_display_L, hours_to_display_R, minutes_to_display_R);
		printSharp32x48HfScr(parking_clock, 130,1,DISP_INVERT_LEFT); //100 to 130

	//	printSharp16x24("  INSERT CARD OR COINS  ", 190, 1, 0);
		Debug_Output2( 0, "ParkingClock Left: %02d:%02d\r\n",hours_to_display_L,minutes_to_display_L);
	}
	else
	{
		DSM_Screen6();
	}
}

void DSM_Screen2()
{
	Screen_TimeOut_RTC =  RTC_epoch_now();
	//Debug_Output1(0,"Screen_TimeOut_RTC DSM_Scrn2:%ld", Screen_TimeOut_RTC);
	//Debug_TextOut( 0, "DSM_Screen2" );
	//Debug_Output1(0,"cardread DSM_Scr2:%ld", cardread);
	Clear_Screen();
	payment_on_hold = false;
	if(((time_retain[Current_Space_Id]==0)&&(min_time_insec<=0))||(time_retain[Current_Space_Id]==1)||(parking_time_left_on_meter[Current_Space_Id]>=min_time_insec))
		{

			hours_to_display = parking_time_left_on_meter[Current_Space_Id]/3600;
			temp_minutes_to_disp = (parking_time_left_on_meter[Current_Space_Id] % 3600);
			minutes_to_display = temp_minutes_to_disp/60;

			if((temp_minutes_to_disp % 60) > 0)	//This is to fix the wrong update of parking clock
				{
					minutes_to_display += 1;
					if(minutes_to_display > 59)
						{
							hours_to_display += 1;
							minutes_to_display = 0;
						}
				}
		}
	else if((min_time_insec != 0)&&((minutes_left_in_current_schedule*60) <= min_time_insec))
	{
		hours_to_display = parking_time_left_on_meter[Current_Space_Id]/3600;
		temp_minutes_to_disp = (parking_time_left_on_meter[Current_Space_Id] % 3600);
		minutes_to_display = temp_minutes_to_disp/60;
		if((temp_minutes_to_disp % 60) > 0)	//This is to fix the wrong update of parking clock
		{
			minutes_to_display += 1;
			if(minutes_to_display > 59)
			{
				hours_to_display += 1;
				minutes_to_display = 0;
			}
		}
		time_retain[Current_Space_Id]=1;
	}
	else
		{
			hours_to_display = 0;
			minutes_to_display = 0;
		}

	if(Current_Space_Id == (glSystem_cfg.MSM_max_spaces-1))
	{
		//Debug_TextOut( 0, "DSM_Screen2_KEYUP" );
		Clear_Screen();
		//printSharp16x24("USE ARROWS TO SELECT BAY", 2, 1, 0);
		bay_number_to_display = ((Current_Space_Id+glSystem_cfg.meter_id) % 1000) - (BayAdjustValue);
		if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
			sprintf((char*)parking_clock, "%02d:%02d RIGHT@",hours_to_display, minutes_to_display);
		else
			sprintf((char*)parking_clock, "%02d:%02d RIGHT/",hours_to_display, minutes_to_display);
		printSharp32x48(parking_clock, 20,1,1);  //40 to 20
		if((parking_time_left_on_meter_prev[Current_Space_Id] == min_time_insec) && (min_time_insec > 0))
			{
				printSharp16x24("Minimum Purchase Reached   ", 88, 1, 0);
				parking_time_left_on_meter_prev[Current_Space_Id] = 0;
			}
		else if((parking_time_left_on_meter[Current_Space_Id]==0)||(time_retain[Current_Space_Id]==1)||(min_time_insec==0))
		{
			if(glSystem_cfg.schedule_display_messages[0].font_size == FONT_SIZE_SMALL)
				printSharp16x24(glSystem_cfg.schedule_display_messages[sch_message_index_1 - 1].display_string, 88, 1, 0); //this is to fix LNGSIT-1735 //vinay
			else
				printSharp16x24("  INSERT CARD OR COINS  ", 88, 1, 0); //110 to 88
		}
		else printSharp16x24("Min Purchase not Reached", 88, 1, 0);
		////printSharp16x24("  INSERT CARD OR COINS  ", 110, 1, 0);
		//printSharp16x24("   INSERT COINS ONLY    ", 110, 1, 0);
		//printSharp16x24("PRESS C TO CHANGE SPACE", 160, 1, 0);
		//Debug_Output2(0,"123:: %d, %d",Current_Space_Id, bay_number_to_display);
		if(glSystem_cfg.schedule_display_messages[1].font_size == FONT_SIZE_SMALL)
			printSharp16x24(glSystem_cfg.schedule_display_messages[sch_message_index_2 - 1].display_string, 120, 1, 0);
		if(glSystem_cfg.schedule_display_messages[2].font_size == FONT_SIZE_SMALL)
			printSharp16x24(glSystem_cfg.schedule_display_messages[sch_message_index_3 - 1].display_string, 152, 1, 0);
		if(glSystem_cfg.schedule_display_messages[3].font_size == FONT_SIZE_SMALL)
			printSharp16x24(glSystem_cfg.schedule_display_messages[sch_message_index_4 - 1].display_string, 184, 1, 0);

		Debug_Output2( 0, "ParkingClock Right: %02d:%02d\r\n",hours_to_display,minutes_to_display);
	}
	else
	{
		//Debug_TextOut( 0, "DSM_Screen2_KEYDOWN" );
		Clear_Screen();
		//printSharp16x24("USE ARROWS TO SELECT BAY", 2, 1, 0);
		bay_number_to_display = ((Current_Space_Id+glSystem_cfg.meter_id) % 1000) - (BayAdjustValue);
		if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
			sprintf((char*)parking_clock, "?LEFT  %02d:%02d", hours_to_display, minutes_to_display);
		else
			sprintf((char*)parking_clock, "[LEFT  %02d:%02d", hours_to_display, minutes_to_display);
		printSharp32x48(parking_clock, 20,1,1);//40 to 20
		if((parking_time_left_on_meter_prev[Current_Space_Id] == min_time_insec) && (min_time_insec > 0))
			{
				printSharp16x24("Minimum Purchase Reached   ", 88, 1, 0);
				parking_time_left_on_meter_prev[Current_Space_Id] = 0;
			}
		else if((parking_time_left_on_meter[Current_Space_Id]==0)||(time_retain[Current_Space_Id]==1)||(min_time_insec==0))
		{
			if(glSystem_cfg.schedule_display_messages[0].font_size == FONT_SIZE_SMALL)
				printSharp16x24(glSystem_cfg.schedule_display_messages[sch_message_index_1 - 1].display_string, 88, 1, 0); //this is to fix LNGSIT-1735 //vinay
			else
				printSharp16x24("  INSERT CARD OR COINS  ", 88, 1, 0);   //110 to 88
		}
		else printSharp16x24("Min Purchase not Reached", 88, 1, 0);
		////printSharp16x24("  INSERT CARD OR COINS  ", 110, 1, 0);
		//printSharp16x24("   INSERT COINS ONLY    ", 110, 1, 0);
		//printSharp16x24("PRESS C TO CHANGE SPACE", 160, 1, 0);
		//Debug_Output2(0,"456:: %d, %d",Current_Space_Id, bay_number_to_display);
		if(glSystem_cfg.schedule_display_messages[1].font_size == FONT_SIZE_SMALL)
			printSharp16x24(glSystem_cfg.schedule_display_messages[sch_message_index_2 - 1].display_string, 120, 1, 0);
		if(glSystem_cfg.schedule_display_messages[2].font_size == FONT_SIZE_SMALL)
			printSharp16x24(glSystem_cfg.schedule_display_messages[sch_message_index_3 - 1].display_string, 152, 1, 0);
		if(glSystem_cfg.schedule_display_messages[3].font_size == FONT_SIZE_SMALL)
			printSharp16x24(glSystem_cfg.schedule_display_messages[sch_message_index_4 - 1].display_string, 184, 1, 0);

		Debug_Output2( 0, "ParkingClock Left: %02d:%02d\r\n",hours_to_display,minutes_to_display);
	}
}


void DSM_Screen1()
{
	Screen_TimeOut_RTC =  RTC_epoch_now();
	//Debug_Output1(0,"Screen_TimeOut_RTC DSM_Scrn1:%ld", Screen_TimeOut_RTC);
	//Debug_TextOut( 0, "DSM_Screen1" );
	Clear_Screen();
	//printSharp16x24("USE BUTTON TO SELECT BAY", 2, 1, 0);
	printSharp32x48("  USE ARROW ",15,0,0);//printSharp32x48(" THIS METER ",35,0,0);
	printSharp32x48(" BUTTONS TO ", 70,0,0);//printSharp32x48("   COVERS   ", 80,0,0);
	printSharp32x48("SELECT SPACE ", 125,0,0);//printSharp32x48(" TWO SPACES  ", 125,0,0);
	//printSharp16x24("PICK SPACE BEFORE PAYING", 184, 1, 0);
	//printSharp16x24("SELECT SPACE FOR RATE/HRS", 184, 0, 0);
	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
		printSharp16x24("  ~LEFT          RIGHT}  ", 184, 0, 0);
	else
		printSharp16x24("  |LEFT          RIGHT{  ", 184, 0, 0);
	//printSharp32x48("[LEFT RIGHT/", 145, 0, 0);
	Current_Space_Id = 0xFF;
	//Debug_Output1(0,"cardread DSM_scr1:%ld", cardread);
}



//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

