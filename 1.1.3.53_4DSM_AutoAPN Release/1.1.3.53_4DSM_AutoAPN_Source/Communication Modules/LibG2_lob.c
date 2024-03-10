//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_lob.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_lob_api
//! @{
//
//*****************************************************************************
#include "LibG2_lob.h"

extern const uint8_t    				PCI_VERSION, MPB_VERSION, sub_VERSION, branch_version;
extern uint8_t  						glDo_rtc_update;//, glFlash_rtc_update;
extern GPRSSystem      					glSystem_cfg;
extern GPRS_General_Sync_Request		gprs_general_sync_request;
extern GPRS_General_Sync_Response       gprs_general_sync_response;
extern uint8_t          				rx_ans[ GPRS_GEN_BUF_SZ_SMALL ];
//extern uint8_t 			    			Sensor_VER[7];//not used anywhere in program //vinay
extern GPRS_FD_File    					glAfm_response_p[FFACT_TOTAL_FILES+1];
extern uint8_t   						rtc_day,  rtc_month, rtc_year,rtc_hour, rtc_min;
extern RTC_C_Calendar 					rtc_c_calender;
extern uint8_t                 			DLST_FLAG_CHECK;
extern Tech_Menu_Disp   				tech_menu_disp;
extern Modem               			    sys_mdm_p;
extern uint8_t                			mdm_comm_status;
extern MBInfo      						glMB_info;
extern uint32_t 						Last_Comms_Success_RTCTimestamp;
extern uint8_t 							Last_Comms_Success_Flg;


uint32_t 								Sync_status = 0;
uint8_t									/*gl_RTC_diag_msg,*/Initial_boot = TRUE;//not used in this program //vinay
FlashFieActvnTblEntry   				flash_file_actvn_tbl[ FILE_TYPE_TOTAL_FILES ];
uint8_t         						SENSOR_OUT_STATUS = FALSE;//, MB_COMMS_OUTAGE = FALSE;//not used in this program //vinay
uint8_t  								General_Sync_Status;
uint8_t									Meter_Reset = TRUE;
#if	0
void LOB_Meter_details()
{
	uint8_t l=0, m=0;

	uint8_t reg_stat = (mdm_comm_status > 3) ? 1 : 0;//11-04-12

	tech_menu_disp.row    = 0;
	tech_menu_disp.column = 0xff;
	tech_menu_disp.font   = font_1_invert_text;
	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu2);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.font   = font_1;
	tech_menu_disp.column = 0;
	tech_menu_disp.row   += 22;
	sprintf((char*)tech_menu_disp.text, "Mech SN:%s,LID:", glMB_info.serial_no_asci);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.font   = font_1;
	tech_menu_disp.column = 0;
	tech_menu_disp.row   += 22;
	for(l=0;l<20;l++)
	{
		if(glMB_info.locid[l]==0xff)
			m++;
	}
	if(m!=20)
	{
		sprintf((char*)tech_menu_disp.text, "%s", glMB_info.locid);
	}
	else
	{
		sprintf((char*)tech_menu_disp.text, "%s", "\0");
	}

	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 22;
	sprintf((char*)tech_menu_disp.text, "City/CustID:%d/%u", glMB_info.city_code,glSystem_cfg.cust_id);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 22;
	sprintf((char*)tech_menu_disp.text, "MID/AID:%u/%u", glSystem_cfg.meter_id,glSystem_cfg.area_num);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 22;
	sprintf((char*)tech_menu_disp.text, "SrvrIP:%s", glSystem_cfg.UDP_server_ip);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 22;
	sprintf((char*)tech_menu_disp.text,":%d,SIG:%d,REG:%d\0", glSystem_cfg.UDP_server_port, sys_mdm_p.signal_strength,reg_stat);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	/*tech_menu_disp.row   += 22;
	sprintf(tech_menu_disp.text, "ServerPort:%d", glSystem_cfg.UDP_server_port);
	TechMenu_Display_Item( &tech_menu_disp );*/

	tech_menu_disp.row   += 22;
	sprintf((char*)tech_menu_disp.text, "APN:%s", glSystem_cfg.apn);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	diag_text_Screen( "Trying alternate Srvr", TRUE );
}


void LOB_try_alt_server()
{
	uint8_t LOB_recovery_try=0;
	int16_t result;
	//Socket                            * sock_p;

	for(LOB_recovery_try = 0; LOB_recovery_try < LOB_MAX_RECOVERY_TRY; LOB_recovery_try++)
	{
		if((LOB_recovery_try%2) == 0)
		{
			strcpy( (char*)glSystem_cfg.UDP_server_ip, US_FT_PRIVATE_SERVER_IP );
			glSystem_cfg.UDP_server_port = US_FT_PRIVATE_SERVER_PORT;
			strcpy( (char*)glSystem_cfg.apn, TMOBILE_PRIVATE_SIM_CARD_APN );
		}
		else
		{
			strcpy( (char*)glSystem_cfg.UDP_server_ip, US_FT_PRIVATE_SERVER_IP );
			glSystem_cfg.UDP_server_port = US_FT_PRIVATE_SERVER_PORT;
			strcpy( (char*)glSystem_cfg.apn, ATT_PRIVATE_SIM_CARD_APN );
		}

		result = telit_init();
		result = telit_sock_open(0);
		DelayMs(4000);
		if(result == MDM_ERR_NONE)
		{
			result = gprs_do_get_General_Sync_request();
			if(result == GPRS_COMMERR_NONE )
			{
				General_Sync_Status = General_Sync_Error_None;
			}
			else
			{
				General_Sync_Status = General_Sync_Recv_Error;
			}
			break;
		}
	}
	if(LOB_recovery_try >= LOB_MAX_RECOVERY_TRY)
		activate_config_file(false);
}
#endif

//11/09/2015
/**************************************************************************/
//! This function is used to prepare the General Sync Request structure
//! \param void
//! \return void
/**************************************************************************/
void Fill_General_Sync_request_structure()
{
	uint8_t loop = 0;
	glDo_rtc_update = FALSE; //11/09/2015
	//glFlash_rtc_update = FALSE;


	gprs_general_sync_request.OLT_AES_Encryption = 1;

	memset(gprs_general_sync_request.file_data, 0, sizeof(gprs_general_sync_request.file_data));

	gprs_general_sync_request.MPB_CFG_CRC = Calculate_File_CRC_4mflash(FILE_TYPE_MPB_CONFIG);

	gprs_general_sync_request.Total_Files = FILE_TYPE_TOTAL_FILES;

	for(loop=0;loop<FILE_TYPE_TOTAL_FILES;loop++)
	{
		gprs_general_sync_request.file_data[loop].File_Type = FILE_TYPE_MPB_CONFIG + loop;
		gprs_general_sync_request.file_data[loop].File_ID = flash_file_actvn_tbl[loop].id;//loop+1;

		if(loop == FFACT_MPB_CONFIG)
		{
			memcpy(gprs_general_sync_request.file_data[loop].CHK_SUM, &gprs_general_sync_request.MPB_CFG_CRC, sizeof(gprs_general_sync_request.MPB_CFG_CRC));

			memset(rx_ans, 0, sizeof(rx_ans));
			sprintf( (char*)rx_ans,"%d.%d.%d.%d", glSystem_cfg.config_version, glSystem_cfg.config_main_version, glSystem_cfg.config_sub_version, glSystem_cfg.config_branch_version );
			memcpy(gprs_general_sync_request.file_data[loop].VER_NUM, rx_ans,  strlen((const char*)rx_ans));

			gprs_general_sync_request.file_data[loop].VER_LEN= strlen((const char *)gprs_general_sync_request.file_data[loop].VER_NUM);
		}
		else if(loop == FFACT_MPB_CODE)
		{
			memcpy(gprs_general_sync_request.file_data[loop].CHK_SUM, &gprs_general_sync_request.MPB_BIN_CRC, sizeof(gprs_general_sync_request.MPB_BIN_CRC));

			memset(rx_ans, 0, sizeof(rx_ans));
			sprintf( (char*)rx_ans,"%d.%d.%d.%d_4DSM", PCI_VERSION,MPB_VERSION,sub_VERSION,branch_version );
			memcpy(gprs_general_sync_request.file_data[loop].VER_NUM, rx_ans,  strlen((const char*)rx_ans));

			gprs_general_sync_request.file_data[loop].VER_LEN= strlen((const char *)gprs_general_sync_request.file_data[loop].VER_NUM);
		}
	}
}
//11/09/2015
/**************************************************************************/
//! This function processes the RTC received in General Sync response from
//! server. It assigns the received RTC in MSP RTC module, updates RTC on
//! display, refreshes the rates, schedules according to current time
//! \param void
//! \return void
/**************************************************************************/
void Handle_RTC_4mServer()
{
	SetMSP_RTC(FALSE);

	if ( glDo_rtc_update == TRUE )
	{
		Initial_boot = FALSE;
		TechMenu_Get_RTC_update_time();
		glDo_rtc_update = FALSE;

		if(glSystem_cfg.DLST_ON_OFF==1)
		{
			DLST_FLAG_CHECK = 1;
			Flash_Batch_Flag_Read();
			DLST_FLAG_CHECK = 0;
		}
		LCD_Backlight(TRUE);
		Update_RTC_on_Display();
		read_and_fill_current_schedule(&rtc_c_calender);	//Refresh rate every time after updating RTC from server; just in case anything changed
		diag_text_Screen( "RTC Updated", Meter_Reset, FALSE );
		Flash_RTC_ParkingClock_Storage(true, false); //Now we are writing RTC and parking clock for every 10mins and if any one resets then we are lossing it, So to avoid that we are storing RTC to flash inintially only.//Vinay
		Last_Comms_Success_RTCTimestamp = RTC_epoch_now();
		Last_Comms_Success_Flg = TRUE;
	}
	else
	{
		diag_text_Screen( "RTC Update failed", Meter_Reset, FALSE );//05-06-12:need this msg on display
	}
}

/**************************************************************************/
//! This function gives appropriate log after parsing the mech swap response
//! received in General Sync response from server.
//! \param void
//! \return void
/**************************************************************************/
void Handle_Mechswap_Response()
{
	if(gprs_general_sync_response.mech_swap_result == 0)
	{
		diag_text_Screen( "SN and LID matches", Meter_Reset, FALSE );
	}
	else if(gprs_general_sync_response.mech_swap_result == 1)
	{
		diag_text_Screen( "SN and LID updated", Meter_Reset, FALSE );
	}
	else if(gprs_general_sync_response.mech_swap_result == 99)
	{
		diag_text_Screen( "Server Error", Meter_Reset, FALSE );
	}
	else if(gprs_general_sync_response.mech_swap_result == 3)
	{
		diag_text_Screen( "Invalid LID Mtr frozen", Meter_Reset, FALSE );
#if 0
		//////freeze mpb///////
		meter_out_of_service=true;//05-08-2013:DPLIBB-554
		SendCommand_2_meter( MPB2MB_FXD_MSG_HDR_ID, MPB2MB_MSG_ID_MB_Outof_Service, 0);//10-01-12
		telit_power_off();
		__disable_interrupt();
		WDT_A_holdTimer();           // Stop watchdog timer
		Meter_regular_sleep();
		//////freeze mpb///////
#endif
	}
	else
	{
		diag_text_Screen( "Invalid Mechswap response", Meter_Reset, FALSE );
	}

	//start_timer_isr(); // enable timer isr
}
//14/09/2015

/**************************************************************************/
//! This function checks the file syncronization status and marks in Sync_status
//! variable which is used during Quick Meter Status display
//! \param void
//! \return void
/**************************************************************************/
void Get_Meter_Status(uint8_t FD_timeout)
{
	uint8_t loop = 0;

	if(gprs_general_sync_response.total_FD_chunks > 0)
	{

		if(FD_timeout != TRUE)
		{
			for(loop=0;loop<FILE_TYPE_TOTAL_FILES+1;loop++)
			{
				if(glAfm_response_p[loop].type == FILE_TYPE_MPB_CONFIG)
				{
					Sync_status = Sync_status & (0xFFFFF0);
					Sync_status |= (9 << 0);
				}
				else if(glAfm_response_p[loop].type == FILE_TYPE_MPB_CODE)
				{
					Sync_status = Sync_status & (0xFFFF0F);
					Sync_status |= (9 << 4);
				}
			}
		}
		else//time out
		{
			if(glAfm_response_p[loop].type == FILE_TYPE_MPB_CONFIG)
			{
				Sync_status = Sync_status & (0xFFFFF0);
			}
			else if(glAfm_response_p[loop].type == FILE_TYPE_MPB_CODE)
			{
				Sync_status = Sync_status & (0xFFFF0F);
			}
		}
	}
	else if((FD_timeout != TRUE))//nothing to download
	{
		//Sync_status = 0x654321;
		Sync_status = 0x21;
	}
}

/**************************************************************************/
//! This function shows Quick Meter Status on LCD, it shows important
//! diagnostic information in one line text on screen, the diagnostic include
//! File sync status, Last RTC sync time, Sensor Status, Battery Voltage
//! \param void
//! \return void
/**************************************************************************/
void Display_Meter_status()
{
	uint8_t  STATUS = (SENSOR_OUT_STATUS << 1);
	uint16_t   MSP_Vbat = 0;

	MSP_Vbat = Rechargeable_battery_read();
	tech_menu_disp.row = 184;//23-05-12
	tech_menu_disp.column = 3;
	tech_menu_disp.font = font_1;
	sprintf((char*)tech_menu_disp.text,"%02lx-%d-%02d%02d-%02d:%02d-%d-%d.%d",(uint32_t)(Sync_status),sys_mdm_p.signal_strength,rtc_month,rtc_day,rtc_hour,rtc_min,STATUS,(MSP_Vbat/100),(MSP_Vbat%100));
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
	Debug_Output1(0, "%s", (uint32_t)tech_menu_disp.text);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
