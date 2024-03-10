//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_keypad.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_keypad_api
//! @{
//
//*****************************************************************************
#include "LibG2_keypad.h"

extern uint8_t					ccard_dataread_flag, card_removed_flag, check_card_read,Tech_menu,smart_card_mode;
//extern uint16_t 				gl_CC_amount_selected;
extern GPRSSystem   			glSystem_cfg;
extern uint16_t      			glCurrent_amount;//, min_calculated_amount, max_calculated_amount; //,pre_existing_time_in_mins;//not used in this program //vinay
extern uint8_t 					T2_Faw_flag, T1_Faw_flag;
extern uint32_t 				Tasks_Priority_Register;
extern volatile uint8_t 		Max_Amount_In_current_rate;
extern uint32_t 				OLT_start_time, CC_TimeOut_RTCTimestamp;
extern uint8_t      			glIn_diag_mode;
extern uint16_t					Current_Space_Id;
extern uint8_t					Meter_Full_Flg[MSM_MAX_PARKING_BAYS], Quick_meter_status_request, do_not_allow_cc_auth;
extern Tech_Menu_Disp   		tech_menu_disp;
extern uint32_t					last_payment_try_time[MSM_MAX_PARKING_BAYS];
extern uint32_t  				Refund_PreviousSN;
extern uint32_t  				Refund_PresentSN;
extern uint8_t 					Connected_Card_reader, glLast_reported_space_status[MSM_MAX_PARKING_BAYS], ZERO_OUT_ENABLE_AT_EXPIRY[MSM_MAX_PARKING_BAYS], ZERO_OUT[MSM_MAX_PARKING_BAYS];
extern uint32_t 				calc_refund_amount;
extern uint32_t  				Refund_earned_mins;
extern uint32_t  				Refund_timestamp;


uint8_t 						key_press = FALSE;
//char             				special_keys[6], special_key_count = 0, cancel_key_count = 0;
uint32_t 						KeyPress_TimeOut_RTC = 0;
//uint8_t  						Quick_meter_status_request = FALSE, Tech_key_seq = FALSE;
extern uint8_t					RefundOK;

extern uint8_t					No_Modem_Coin_Only; //for coin only meter check
extern uint8_t					cardread;
extern volatile uint8_t 		current_rate_index;
extern uint8_t					cc_approved; // When the network issuse is there we are getting approve twice and motorist is getting double parking clock// to fix it //vinay
extern uint8_t					Modem_Off_initiated; //to turn off modem for cc payment  //vinay
extern uint8_t 					modem_init_to_process_cc_transaction;
/**************************************************************************/
//! Identifies the pressed key and returns the detected key
//! \param void
//! \return uint8_t key
//! - \b KEY_UNKNOWN
//! - \b KEY_OK
//! - \b KEY_CANCEL
//! - \b KEY_UP
//! - \b KEY_DOWN
//! - \b KEY_LEFT
//! - \b KEY_RIGHT
/**************************************************************************/
uint8_t discriminate_which_key()
{
	volatile static uint32_t status;

	Debounce();
	status = GPIO_getEnabledInterruptStatus(KEYPAD_PORT);
	GPIO_clearInterruptFlag(KEYPAD_PORT, status);
	if((No_Modem_Coin_Only == true)||(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))
	{
		//Vinay
		if(status & KEYPAD_RT_PIN)
		{
			GPIO_clearInterruptFlag(KEYPAD_PORT, status);
			Debounce();
			KeyPress_TimeOut_RTC =  RTC_epoch_now();
			LCD_Backlight(TRUE);
			//Debug_TextOut( 0, "RT RT RT" );
			return (KEY_UP);
		}
		//Vinay
		if(status & KEYPAD_CAN_PIN)
		{
			GPIO_clearInterruptFlag(KEYPAD_PORT, status);
			Debounce();
			KeyPress_TimeOut_RTC =  RTC_epoch_now();
			LCD_Backlight(TRUE);
			//Debug_TextOut( 0, "CAN CAN CAN" );
			return(KEY_OK);
		}
		//Vinay
		if(status & KEYPAD_LT_PIN)
		{
			GPIO_clearInterruptFlag(KEYPAD_PORT, status);
			Debounce();
			KeyPress_TimeOut_RTC =  RTC_epoch_now();
			LCD_Backlight(TRUE);
			//Debug_TextOut( 0, "LT LT LT" );
			return(KEY_DOWN);
		}
	//Vinay
		if(status & KEYPAD_OKK_PIN)
		{
			GPIO_clearInterruptFlag(KEYPAD_PORT, status);
			Debounce();
			KeyPress_TimeOut_RTC =  RTC_epoch_now();
			LCD_Backlight(TRUE);
			//Debug_TextOut( 0, "OKK OKK OKK" );
			return(KEY_CANCEL);
		}

	}
	else
	{
		if(status & KEYPAD_UP_PIN)
		{
			GPIO_clearInterruptFlag(KEYPAD_PORT, status);
			Debounce();
			KeyPress_TimeOut_RTC =  RTC_epoch_now();
			LCD_Backlight(TRUE);
			//Debug_TextOut( 0, "UP UP UP" );
			return (KEY_UP);
		}

		if(status & KEYPAD_OK_PIN)
		{
			GPIO_clearInterruptFlag(KEYPAD_PORT, status);
			Debounce();
			KeyPress_TimeOut_RTC =  RTC_epoch_now();
			LCD_Backlight(TRUE);
			//Debug_TextOut( 0, "OK OK OK" );
			return(KEY_OK);
		}

		if(status & KEYPAD_DOWN_PIN)
		{
			GPIO_clearInterruptFlag(KEYPAD_PORT, status);
			Debounce();
			KeyPress_TimeOut_RTC =  RTC_epoch_now();
			LCD_Backlight(TRUE);
			//Debug_TextOut( 0, "DOWN DOWN DOWN" );
			return(KEY_DOWN);
		}

		if(status & KEYPAD_CANCEL_PIN)
		{
			GPIO_clearInterruptFlag(KEYPAD_PORT, status);
			Debounce();
			KeyPress_TimeOut_RTC =  RTC_epoch_now();
			LCD_Backlight(TRUE);
			//Debug_TextOut( 0, "CANCEL CANCEL CANCEL" );
			return(KEY_CANCEL);
		}
	}
	Debounce();
	GPIO_clearInterruptFlag(KEYPAD_PORT, status);
	return KEY_UNKNOWN;
}

void Debounce(void)
{
	uint32_t i,k;

	for(i=0;i<6;i++)  //delay is added for debounce issue
	{
		for(k=0;k<=10000;k++);
	}
}

/**************************************************************************/
//! Keypress processor called from outer layer after identification of a key
//! \param void
//! \return void
/**************************************************************************/
void Process_keypress()
{
	//int16_t     result;
	//char        keys_local[6];//30-03-12;
	int16_t     max_amount_to_check, min_amount_to_check;	//12-06-12
	//LCD_Backlight(TRUE);

#if	0
	if((key_press != KEY_UNKNOWN) && (card_removed_flag==false)&&(smart_card_mode==false)&&(Tech_menu==false)/*&&(UI_for_dataextraction_key==false)*/)
	{
		if(special_key_count<5)
		{
			switch ( key_press )//up arrow
			{
			case KEY_UP:
				special_keys[special_key_count]=KEY_UP+0x30;
				special_key_count++;
				break;

			case KEY_DOWN://down arrow
				special_keys[special_key_count]=KEY_DOWN+0x30;
				special_key_count++;
				break;

			case KEY_CANCEL: //cancel key
				special_keys[special_key_count]=KEY_CANCEL+0x30;
				special_key_count++;
				break;

			case KEY_OK://key ok
				special_keys[special_key_count]=KEY_OK+0x30;
				special_key_count++;
				break;
			}
		}

		if(special_key_count>=5)
		{
			special_key_count=0;

			sprintf((char*)keys_local,"%s", special_keys);

			sprintf((char*)special_keys,"%s", SPECIAL_KEYS_ENTER_TECH_MENU);

			if( (strncmp(keys_local,special_keys,5)==0) && (Tech_menu==false) )
			{
				Tech_menu=true;
				Tech_key_seq = TRUE;
				Debug_TextOut( 0, "LNG is in tech mode" );
				push_event_to_cache( UDP_EVTTYP_TECH_MENU_ACCESS );
				TechMenu_Display_Main_Menu();
			}

			sprintf((char*)special_keys,"%s", SPECIAL_KEYS_METER_STATUS); //Quick Meter Status
			if((strncmp(keys_local,special_keys,5)==0) && (Tech_menu==false))
			{
				Quick_meter_status_request = TRUE;
				Debug_TextOut( 0, "Quick Meter Status" );
				Display_Meter_status();
				memset(special_keys,0,sizeof(special_keys));
			}

			sprintf((char*)special_keys,"%s", SPECIAL_KEYS_METER_AUDIT); //Meter Audit
			if((strncmp(keys_local,special_keys,5)==0) && (Tech_menu==false))
			{
				Debug_TextOut( 0, "Meter Audit Done" );
				push_event_to_cache( UDP_EVTTYP_COLLECTION_EVT );	//Audit event raised; server will take care of collection report
				tech_menu_disp.row = 184;//23-05-12
				tech_menu_disp.column = 10;
				tech_menu_disp.font = font_1;
				sprintf((char*)tech_menu_disp.text,"Meter Audit Done");
				printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
				memset(special_keys,0,sizeof(special_keys));
			}

			/*sprintf((char*)special_keys,"%s", SPECIAL_KEYS_DIAG_MODE);

			if( (strncmp(keys_local,special_keys,5)==0) && (glIn_diag_mode==false) )
			{
				glIn_diag_mode=true;
				Debug_TextOut( 0, "LNG is in Diag mode" );
				//set_ip_adress();//01-08-12:no need to set here ,just display what was already set .
				display_info_on_LCD();
				memset(special_keys,0,sizeof(special_keys));    // TODO --- why?
			}
			else if((strncmp(keys_local,special_keys,5)==0)&&(glIn_diag_mode==true))
			{
				glIn_diag_mode=false;
				Debug_TextOut( 0, "LNG Exited Diag mode" );
				memset(special_keys,0,sizeof(special_keys));    // TODO --- why?
			}*/
		}
		else
		{
			Idle_Screen();
		}
	}
	else
#endif

	//Key press will not be considered as payment attempt due to it is used to select space in DSM/MSM
/*	last_payment_try_time[Current_Space_Id] = RTC_epoch_now();
 	 //if(glLast_reported_space_status[Current_bayStatus] == 0)
	if((glLast_reported_space_status[Current_bayStatus] == 0) && ((current_rate_index < 20) || (current_rate_index == PREPAY_PARKING)))
	{
		ZERO_OUT_ENABLE_AT_EXPIRY[Current_bayStatus] = TRUE;
		ZERO_OUT[Current_bayStatus] = FALSE;
		Debug_TextOut(0,"PA on Vacant, Won't Zero Out till next 801 event");
	}*/

	//if((key_press != KEY_UNKNOWN) && (card_removed_flag==true)&&(smart_card_mode==false)/*&&(UI_for_dataextraction_key==false)*/)
	if((key_press != KEY_UNKNOWN) && (card_removed_flag==true)&&(smart_card_mode==false)&&(cardread==1)) // even if card read was failed and upon key press it was showing selection screen so added cardread //vinay
	{
		switch ( key_press )//up arrow
		{
		//01-06-12
		case KEY_UP:
			//Re-Fresh CC TimeOut Time
			CC_TimeOut_RTCTimestamp = RTC_epoch_now();

			if(Max_Amount_In_current_rate == FALSE)
				glCurrent_amount += glSystem_cfg.amt_incr;//increment current_amount
			//Debug_Output1(0,"amount:%d", glCurrent_amount);
			//MB PGM: 1105
			//if(glSystem_cfg.CC_MAX_FLAG==0)
				max_amount_to_check = glSystem_cfg.Card_max_amount;
			//else
				//max_amount_to_check = max_calculated_amount;

			if (( glCurrent_amount >= max_amount_to_check ) || (Max_Amount_In_current_rate == TRUE))
			{
				//Debug_Output2(0,"amount:%d, %d", glCurrent_amount, max_amount_to_check);
				if(Max_Amount_In_current_rate == FALSE)
					glCurrent_amount = max_amount_to_check;
				ShowAmount_Screen(glCurrent_amount, MAX_AMOUNT_DISPLAY,FALSE);
			}
			else
			{
				//Debug_Output2(0,"amount2:%d, %d", glCurrent_amount, max_amount_to_check);
				ShowAmount_Screen(glCurrent_amount, 0,FALSE);
			}
			break;

		case KEY_DOWN://down arrow
			Max_Amount_In_current_rate = FALSE;
			//Re-Fresh CC TimeOut Time
			CC_TimeOut_RTCTimestamp = RTC_epoch_now();

			glCurrent_amount -= glSystem_cfg.amt_incr; //decrement current_amount

			min_amount_to_check = glSystem_cfg.Card_min_amount;

			if ( glCurrent_amount <= min_amount_to_check )
			{
				glCurrent_amount = min_amount_to_check;
				ShowAmount_Screen(glCurrent_amount, MIN_AMOUNT_DISPLAY,FALSE);
			}
			else
			{
				ShowAmount_Screen(glCurrent_amount, 0,FALSE);
			}
			break;

		case KEY_OK: //ok key
			if(do_not_allow_cc_auth == FALSE)
				Max_Amount_In_current_rate = FALSE;
			//Debug_Output1( 0, "KP before cc_approved = %d", cc_approved );
			if(Meter_Full_Flg[Current_Space_Id] == FALSE)
			{
				if(do_not_allow_cc_auth == FALSE)
				{
					if(ccard_dataread_flag == TRUE)
					{
						Authorizing_Screen();

						if(check_card_read == TRUE)
						{
							cc_approved = 1; // When the network issuse is there we are getting approve twice and motorist is getting double parking clock// to fix it //vinay
							//Debug_Output1( 0, "KP cc_approved = %d", cc_approved );
							Debug_TextOut( 0, "OLT START\r\n" );
							OLT_start_time=RTC_epoch_now();
							check_card_read = FALSE;
							//card_removed_flag = false; //LNGSIT-753
							Tasks_Priority_Register |= CREDIT_CARD_OLT_TASK;
						}
					}
				}
			}
			else
			{
				ShowAmount_Screen(glCurrent_amount, 0,FALSE);
			}
			break;

		case KEY_CANCEL: //cancel key
			Max_Amount_In_current_rate = FALSE;
			glCurrent_amount = 0;
			ccard_dataread_flag = FALSE;
			card_removed_flag = FALSE;//to be ready for next swipe and wait for keypress
			check_card_read = false;
			do_not_allow_cc_auth = FALSE;
			T2_Faw_flag = 0;
			T1_Faw_flag = 0;
			if(modem_init_to_process_cc_transaction)
			{
				Modem_Off_initiated = 2; //modem off for card if on //vinay
				telit_power_off(); //modem off for card if on //vinay
			}
			Idle_Screen();
			break;
		}
	}
	else if((key_press != KEY_UNKNOWN)&&(smart_card_mode==true)&&(Tech_menu==false)&&(smart_card.detected==TRUE))
	{
		//Debug_TextOut( 0, "In SC Keypad\r\n" );
		switch ( key_press )//up arrow
		{
		//01-06-12
		case KEY_UP:
			if((RefundOK == 3) && (glSystem_cfg.SC_refund_enable == TRUE))	//LNGSIT-454
			{
				break;
			}
			else
			{
				//Re-Fresh CC TimeOut Time
				CC_TimeOut_RTCTimestamp = RTC_epoch_now();

				if(Max_Amount_In_current_rate == FALSE)
				{
					if(glSystem_cfg.SCT_amt_incr!=0)
					{
						glCurrent_amount += glSystem_cfg.SCT_amt_incr; //glSystem_cfg.amt_incr;//increment current_amount//01-08-2013:DPLIBB-569
					}
					else
					{
						glCurrent_amount += glSystem_cfg.amt_incr;//increment current_amount//01-08-2013:DPLIBB-569
					}
				}

				//if(glSystem_cfg.CC_MAX_FLAG==0)
				{
					if(glSystem_cfg.SCT_default_max_amount!=0)
					{
						max_amount_to_check = glSystem_cfg.SCT_default_max_amount; //glSystem_cfg.default_max_amount;//01-08-2013:DPLIBB-569
					}
					else
					{
						max_amount_to_check = glSystem_cfg.Card_max_amount;//01-08-2013:DPLIBB-569
					}
				}
/*				else
				{
					max_amount_to_check = max_calculated_amount;
				}*/

				if (( glCurrent_amount >= max_amount_to_check ) || (Max_Amount_In_current_rate == TRUE))
				{
					if(Max_Amount_In_current_rate == FALSE)
						glCurrent_amount = max_amount_to_check;
					ShowAmount_Screen(glCurrent_amount, MAX_AMOUNT_DISPLAY,FALSE);
					break;
				}

				// 2012-06-12 ajw - updated cmd - this is correct now - please stop reverting back
				ShowAmount_Screen(glCurrent_amount, 0,FALSE);
				break;
			}

		case KEY_DOWN://down arrow
			//Re-Fresh CC TimeOut Time
			if((RefundOK == 3) && (glSystem_cfg.SC_refund_enable == TRUE))//LNGSIT-454
			{
				break;
			}
			else
			{
				Max_Amount_In_current_rate = FALSE;
				CC_TimeOut_RTCTimestamp = RTC_epoch_now();

				if(glSystem_cfg.SCT_amt_incr!=0)
				{
					glCurrent_amount -= glSystem_cfg.SCT_amt_incr;//glSystem_cfg.amt_incr; //decrement current_amount//01-08-2013:DPLIBB-569
				}
				else
				{
					glCurrent_amount -= glSystem_cfg.amt_incr; //decrement current_amount//01-08-2013:DPLIBB-569
				}
				//if(glSystem_cfg.CC_MAX_FLAG==0)
				{
					if(glSystem_cfg.SCT_default_min_amount!=0)
					{
						min_amount_to_check = glSystem_cfg.SCT_default_min_amount;//glSystem_cfg.default_min_amount;//01-08-2013:DPLIBB-569
					}
					else
					{
						min_amount_to_check = glSystem_cfg.Card_min_amount;//01-08-2013:DPLIBB-569
					}
				}
/*				else
				{
					min_amount_to_check = min_calculated_amount;
				}*/
				if ( glCurrent_amount <= min_amount_to_check )
				{
					glCurrent_amount = min_amount_to_check;

					ShowAmount_Screen(glCurrent_amount, MIN_AMOUNT_DISPLAY,FALSE);
					break;
				}

				// 2012-06-12 ajw - updated cmd - this is correct now - please stop reverting back
				ShowAmount_Screen(glCurrent_amount, 0,FALSE);
				break;
			}
			//01-06-12

		case KEY_OK: //ok key
			// older smart cards don't work without "full" power - 2013-07-26 ajw
			Max_Amount_In_current_rate = FALSE;
			//Debug_Output1(0,"RefundOK Keypad=%d",RefundOK);
			if((RefundOK == 3) && (glSystem_cfg.SC_refund_enable == TRUE) && (Meter_Full_Flg[Current_Space_Id] == FALSE)) //LNGSIT-1176
			{
				RefundOK = 1;
				//Debug_Output1(0,"calc_refund_amount=%d",calc_refund_amount);
				refund_gemclub_memo_card(calc_refund_amount);
				switch (smart_card.smart_card_state)
				{
				case GEMCLUBM_CARD_BALANCE_SUCCESS:
					//Debug_TextOut( 0,"Card Balance Success\r\n");
					//debug_balance("Successful balance %ul", smart_card.balance);
					break;

				case GEMCLUBM_CARD_DEDUCT_SUCCESS:
					//Debug_TextOut( 0,"Card Refund Success\r\n");
					end_read_card();
					break;

				case GEMCLUBM_CARD_DEDUCT_NO_BALANCE:
				case GEMCLUBM_CARD_NULL_DEDUCT_AMOUNT_REQUIRED:

					end_read_card();                       /* Thats it no money card deduct */
					break;

				case GEMCLUBM_CARD_SC_INVALID:
					//Debug_TextOut( 0,"Card invalid\r\n");
					end_read_card();             /* Bad card deduct */
					break;

				case GEMCLUBM_CARD_DEDUCT_FAILED:
					//Debug_TextOut( 0,"Card Refund Failed \r\n");
					end_read_card();             /* Bad card deduct */
					break;

				case GEMCLUBM_CARD_BLACKLIST:
					end_read_card();             /* hotlisted card  end */
					break;

				default:
					/*
					 * This is set in deduct_gemclub_memo_card in case of any failure
					 * In which case call end_read_card and break out
					 */
					if (smart_card.end_smart_card == TRUE)
					{
						end_read_card();
						break;
					}
					break;
				}
			}
			else
			{
				if(Meter_Full_Flg[Current_Space_Id] == FALSE)
				{
					MAG_1_POWER_ON();
					Debug_TextOut( 0, "SC Payment in Progress\r\n" );
					Authorizing_Screen();

					deduct_gemclub_memo_card(glCurrent_amount);
					switch (smart_card.smart_card_state)
					{
					case GEMCLUBM_CARD_BALANCE_SUCCESS:
						//Debug_TextOut( 0,"Card Balance Success\r\n");
						//debug_balance("Successful balance %ul", smart_card.balance);
						break;

					case GEMCLUBM_CARD_DEDUCT_SUCCESS:
						//Debug_TextOut( 0,"Card Deduct Success\r\n");
						end_read_card();
						break;

					case GEMCLUBM_CARD_DEDUCT_NO_BALANCE:
					case GEMCLUBM_CARD_NULL_DEDUCT_AMOUNT_REQUIRED:

						end_read_card();                       /* Thats it no money card deduct */
						break;

					case GEMCLUBM_CARD_SC_INVALID:
						//Debug_TextOut( 0,"Card invalid\r\n");
						end_read_card();             /* Bad card deduct */
						break;

					case GEMCLUBM_CARD_DEDUCT_FAILED:
						//Debug_TextOut( 0,"Card Deduct Failed \r\n");
						end_read_card();             /* Bad card deduct */
						break;

					case GEMCLUBM_CARD_BLACKLIST:
						end_read_card();             /* hotlisted card  end */
						break;

					default:
						/*
						 * This is set in deduct_gemclub_memo_card in case of any failure
						 * In which case call end_read_card and break out
						 */
						if (smart_card.end_smart_card == TRUE)
						{
							end_read_card();
							break;
						}
						break;
					}
				}
				else
				{
					ShowAmount_Screen(glCurrent_amount, 0,FALSE);
				}
			}
			break;

		case KEY_CANCEL: //cancel key

			if((RefundOK == 3) && (glSystem_cfg.SC_refund_enable == TRUE))
			{
				Refund_PreviousSN = 0;
				//Refund_PresentSN = 0;	//LNGSIT-370
				Refund_timestamp = 0;
				Refund_earned_mins = 0;
				RefundOK = 0;
				Process_smart_card_payment();
				Turn_Off_card_IOs(Connected_Card_reader);
			}
			else
			{
				Max_Amount_In_current_rate = FALSE;
				smart_card_mode=false;
				glCurrent_amount = 0;
				ccard_dataread_flag = FALSE;
				card_removed_flag= FALSE; //to be ready for next swipe and wait for keypress
				check_card_read=false;
				Refund_PreviousSN = 0;
				Refund_PresentSN = 0;
				Refund_timestamp = 0;
				Refund_earned_mins = 0;
				RefundOK = 0;
				Turn_Off_card_IOs(Connected_Card_reader);
				Idle_Screen();  //vinay for new screen changes ardsley and generic
			}
			break;
		}
	}
	// 2012-02-12 ajw - end
	else if((key_press != KEY_UNKNOWN)&&(Tech_menu==true)&&(smart_card_mode==false)/*&&(UI_for_dataextraction_key==false)*/)
	{
		switch ( key_press )
		{
		case KEY_UP://up arrow
			TechMenu_ProcessKey_UP_In_TechMode();
			break;
		case KEY_DOWN://down arrow
			TechMenu_ProcessKey_DOWN_In_TechMode();
			break;
		case KEY_OK: //ok key
			TechMenu_ProcessKey_OK_In_TechMode();
			break;
		case KEY_CANCEL: //cancel key
			TechMenu_ProcessKey_CANCEL_In_TechMode();
			break;
		}
	}
#if	0
	else if((key_press != KEY_UNKNOWN) && (card_removed_flag==false)&&(smart_card_mode==false)&&(Tech_menu==false)&&(UI_for_dataextraction_key==true))
	{
		special_key_timeout=false;
		switch ( key_press )
		{
		case KEY_UP://up arrow//09-04-12
			tech_menu_disp.row    = 32*2;
			tech_menu_disp.column = 0;
			tech_menu_disp.font   = font_1;
			liberty_sprintf(tech_menu_disp.text, "Abort Data extraction" );
			TechMenu_Display_Item( &tech_menu_disp );

			tech_menu_disp.row    = 32;
			tech_menu_disp.font   = font_1|disp_invert_text;
			liberty_sprintf(tech_menu_disp.text, "Delete Data and Write" );
			TechMenu_Display_Item( &tech_menu_disp );

			tech_menu_disp.row+=32;
			copy_batch_data=true;
			break;

		case KEY_DOWN://down arrow
			tech_menu_disp.row    = 32;
			tech_menu_disp.column = 0;
			tech_menu_disp.font   = font_1;   //16-05-12
			liberty_sprintf(tech_menu_disp.text, "Delete Data and Write" );
			TechMenu_Display_Item( &tech_menu_disp );

			tech_menu_disp.row  += 32;
			tech_menu_disp.font  = font_1|disp_invert_text;
			liberty_sprintf(tech_menu_disp.text, "Abort Data extraction" );
			TechMenu_Display_Item( &tech_menu_disp );

			copy_batch_data=false;
			break;

		case KEY_OK: //ok key
			if(copy_batch_data==true)
			{
				send_diag_text_to_MB( TRUE, 0, "Sending Data to Datakey" );//05-06-12:need this msg on display

				result = Flash_copy_batch_data_to_datakey();

				if(result == FLASH_ERROR_NONE)
				{
					send_diag_text_to_MB( TRUE, 0, "Data extraction completed" );//05-06-12:need this msg on display
				}
				else
				{
					send_diag_text_to_MB( TRUE, 0, "Data extraction aborted" );//05-06-12:need this msg on display
				}
			}
			else
			{
				send_diag_text_to_MB( TRUE, 0, "Data extraction aborted" );//05-06-12:need this msg on display
			}
			UI_for_dataextraction_key=false;
			copy_batch_data=false;
			SendCommand_2_meter( MPB2MB_FXD_MSG_HDR_ID, MPB2MB_MSG_ID_mpbScr_Idle, glCurrent_amount);
			break;
		case KEY_CANCEL: //cancel key
			break;
		}
	}
#endif

	Quick_meter_status_request = FALSE;
	key_press = KEY_UNKNOWN;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

