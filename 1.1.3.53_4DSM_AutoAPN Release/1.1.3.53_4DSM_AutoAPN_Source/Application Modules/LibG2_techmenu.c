//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_techmenu.c
//
//****************************************************************************
//*****************************************************************************
//
//!
//! \addtogroup LibG2_techmenu_api
//! @{
//
//*****************************************************************************

#include "LibG2_techmenu.h"

/*
***********************************************************************
*Imported Global Variables
***********************************************************************
*/
extern uint32_t                         	config_WSAD, mpb_code_WSAD;
extern uint8_t 								flash_data_read_buffer[ FLASH_DATA_READ_BUF_SIZE ];
extern uint32_t                         	config_RSAD, mpb_code_RSAD;
extern uint8_t                          	file_sizearr[4],file_CRC[4];
extern Maintenance_Evt                  	Maintenance_Log;
extern GPRSSystem                       	glSystem_cfg;
extern uint8_t                          	OLT_Q_count;//,Coin_Q_count;//not used in this program //vinay
extern GPRS_AlarmsRequest               	glAlarm_req;
extern Modem                            	sys_mdm_p;
extern uint8_t                          	glIn_diag_mode;
extern uint8_t                          	mdm_comm_status;
extern uint8_t                          	DLST_FLAG_CHECK;
extern FlashFieActvnTblEntry   				flash_file_actvn_tbl[ FILE_TYPE_TOTAL_FILES ];
extern uint8_t         						fd_start;
extern uint8_t 								/*do_Events_Update,*/do_Alarms_Update,do_General_Sync,activate_mpb_config;//not used in this program //vinay
extern const uint8_t  						PCI_VERSION, MPB_VERSION , sub_VERSION, branch_version;
extern RTC_C_Calendar 						rtc_c_calender;
extern uint32_t 							KeyPress_TimeOut_RTC;
extern uint32_t								max_time_in_current_rate;
extern volatile uint32_t					parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];
volatile uint32_t							negetive_parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];
extern volatile uint8_t 					current_rate_index;
extern uint32_t 							Tasks_Priority_Register;
extern uint16_t 							Current_Space_Id;
extern MBInfo      							glMB_info;
extern uint8_t 								DK_data_read_buffer[ DK_DATA_READ_BUF_SIZE ]; //16-03-12
extern uint8_t     							Tech_key_seq;
//extern GPRS_PamBayStatusUpdateRequest  		PAM_Bay_status_data[MAX_PAM_BSU_QUEUE];//not used in this program //vinay
//extern uint8_t 								PAM_Baystatus_update_count; //not using in this program //vinay
extern uint32_t								Last_Comms_Success_RTCTimestamp;
extern uint8_t								Screen_Clear;
extern volatile uint16_t 					minutes_left_in_current_schedule;
extern uint32_t  							Refund_timestamp;
extern uint32_t  							Refund_earned_mins;
extern uint8_t								Connected_Card_reader, HARDWARE_REVISION;
extern uint8_t								ANTI_FEED_in_effect[MSM_MAX_PARKING_BAYS];
//extern tLoRaSettings 						LoRaSettings, LoRaSettings1;//not using in this program //vinay

uint32_t								    DK_File_CRC,Flash_File_CRC;
//uint32_t									CRC_2_COMPARE[5]; //not using in this program //vinay
//uint32_t 	   								DK_DataRead_CRC;//not using in this program //vinay
extern volatile uint8_t   					REV4_LED_CONFIG; //to know hardware details in techmenu //vinay
extern uint8_t								Front_Bi_Color;
/*
***********************************************************************
*Exported Global Variables
***********************************************************************
*/
uint32_t 									CONFIG_WSAD_DK   = AD_MPB_CONFIG_DK,
         	 	 	 	 	 	 	 	 	MPB_CODE_WSAD_DK = AD_MPB_BINARY_DK;

uint32_t 									CONFIG_RSAD_DK   = AD_MPB_CONFIG_DK,
         	 	 	 	 	 	 	 	 	MPB_CODE_RSAD_DK = AD_MPB_BINARY_DK;
static uint32_t  							file_size_flash=0;

uint8_t										gl_Commission_Card=false;
uint8_t                 					meter_out_of_service=false;
//uint8_t                          			copy_batch_data=false;    //23-04-12// this is used in keypad.c but it is commented //vinay
uint8_t                          			Tech_menu=false;
//uint8_t                          			glRTC_set=FALSE; //,GET_RTC_FROM_MB;//not used in this program //vinay
uint8_t  									advanced_feature           = false,
         	 	 	 	 	 	 	 	 	advanced_feature_count     = 1,
											Display_meter_info_2ndpage = false,
											Display_position           = 0,
											maintenance_scrn_count     = 1,
											maintenance_screen         = false,
											/*comms_test_screen          = false,*/
											Adjust_clock_key_count     = 1,
											Adjust_clock               = false,
											copy_files_screen_count    = 0,
											copy_files_screen          = false,
											exit_filemgr               = false,
											clear_alarm_check          = false,
											reset_timeout_count        = false,
											Static_display             = false,//23-05-12
											tech_mode_timeout          = 0,
											copy_lid_details           = false,
											activate_mpb_binary_from_dk= false;

Tech_Menu_Disp   							tech_menu_disp;
uint8_t 									activate_pgm_rpg_ccf=false;//25-05-12

uint8_t										Tech_modem_onoff_mode=false;
uint8_t 									save_maintenance_log=false,file_manager_screen	=0;
uint8_t 									comm_int_scrn=false,
											save_comm_interval_changes=false,
											do_RTC_EVT_AFM_HB=0,
											comm_int_scrn_count=1;
uint8_t 									do_not_copy_files=false,
        									copy_all_files=false,
											copy_only_config=false,
											copy_only_mpb_firmware=false,
											DK2Mech_copy_option_count=1,
											Mech2DK_copy_option_count=1,
											DK_MECH_files_copied_count=0;
uint8_t   		 							main_menu              = false,
                 	 	 	 	 	 	 	Tech_menu_info         = false,
											coms_info              = false,
											clr_alarm              = false,
											exit_mainpg			   = false;//vinay
uint8_t   									rtc_day    = 0,  rtc_month  = 0,
                 	 	 	 	 	 	 	rtc_year   = 0,  rtc_hour   = 0,
											rtc_min    = 0,  evnt_day   = 0;

//uint8_t 									CRC_Append_flg = 0;//not used in this program //vinay

extern uint8_t 								Current_bayStatus;
extern uint8_t								in_prepay_parking[MSM_MAX_PARKING_BAYS];
extern uint8_t								sensor_tnx_flag;
extern uint8_t		 						Running_Space_ID[MSM_MAX_PARKING_BAYS];
extern uint8_t                          	grace_time_trigger[MSM_MAX_PARKING_BAYS];

extern volatile uint8_t						time_retain[MSM_MAX_PARKING_BAYS]; //vinay // to retain parking clock to become zero trenton
extern uint32_t 							parking_time_left_on_meter_prev[MSM_MAX_PARKING_BAYS]; //to round off //vinay
extern uint8_t								cardread;
extern uint16_t             				glTotal_Coin_Count;

/*
***********************************************************************
*Local Constants
***********************************************************************
*/

/*
***********************************************************************
*Local Variables
***********************************************************************
*/
static uint8_t   							evnt_month = 0,  evnt_year  = 0,
                 	 	 	 	 	 	 	evnt_hour  = 0,  evnt_min   = 0;

static uint16_t  							Adj_clk_curr_mdy_year=0;

static uint8_t   							Adj_clk_curr_mdy_dow=0,
                 	 	 	 	 	 	 	Adj_clk_curr_mdy_min=0,
											Adj_clk_curr_mdy_hour=0,
											Adj_clk_curr_mdy_day=0,
											Adj_clk_curr_mdy_month=0;
static uint16_t   							Save_Clk_Changes = false;
static uint8_t   							local_value=0;
/*
***********************************************************************
*Local Functions
***********************************************************************
*/
static void    tech_clear_alarm();
 void    tech_meter_info_first_page();
 void    tech_meter_info_second_page();
 void    Technician_Menu_Information();
static void    tech_software_information();
//static void    tech_power_information();
static void    tech_communication_information();
//static void    tech_mechanism_files();
static void    tech_adjust_clock();
static void    tech_set_RTC_in_TechMode();
//static void    tech_display_maintenance_menu();
static uint8_t Copy_from_flash_to_datakey(uint8_t type, uint32_t size,uint8_t erase_flag);  // retrns 0 on failure, else returns 1//25-05-12
static uint8_t Copy_from_datakey_to_flash(uint8_t type, uint32_t size);  // retrns 0 on failure, else returns 1

/**************************************************************************/
//! Displays technician menu main screen
//! \param void
//! \return void
/**************************************************************************/
void TechMenu_Display_Main_Menu()
{
	KeyPress_TimeOut_RTC =  RTC_epoch_now();
	main_menu =  true;

	Clear_Screen();

	tech_menu_disp.row    = 0;
	tech_menu_disp.column = 0;
	tech_menu_disp.font   = font_1_invert_text;

	sprintf((char*)tech_menu_disp.text, "%s", " 1. TECH MENU INFO	      ");
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row += 32;//23-05-12
	tech_menu_disp.column = 0;//5;
	tech_menu_disp.font =font_1;
	sprintf((char*)tech_menu_disp.text,"%s", Main_Tech_menu2);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row += 32;//23-05-12
	sprintf((char*)tech_menu_disp.text,"%s", Main_Tech_menu3);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row+=32;//23-05-12
	sprintf((char*)tech_menu_disp.text,"%s", Main_Tech_menu4);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row+=32;//23-05-12
	sprintf((char*)tech_menu_disp.text,"%s", Main_Tech_menu8);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row+=32;//23-05-12
	sprintf((char*)tech_menu_disp.text,"%s", Main_Tech_menu9);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row+=32;//14.08.2019 vinay
	sprintf((char*)tech_menu_disp.text,"%s", Main_Tech_menu10);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	Tech_menu_info         = false;
	coms_info              = false;
	clr_alarm              = false;
	advanced_feature       = false;
	clear_alarm_check      = false;
	exit_mainpg			   = false;//vinay
}

/**************************************************************************/
//! Displays mech and data key files screen in technician menu
//! \param void
//! \return void
/**************************************************************************/
void tech_mechanism_and_datakey_files()
{
	if(gl_Commission_Card == true)
		KeyPress_TimeOut_RTC =  RTC_epoch_now();

	Clear_Screen();

	tech_menu_disp.row    = 0;
	tech_menu_disp.column = 0;//5;
	tech_menu_disp.font   = font_1_invert_text;
	sprintf((char*)tech_menu_disp.text, "DATA KEY FILE MANAGER    ");
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 32;
	tech_menu_disp.column = 0;//10;
	tech_menu_disp.font   = font_1;
	sprintf((char*)tech_menu_disp.text, "[COPY FILES]");
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 32;
	tech_menu_disp.column = 0;//5;
	/*if(gl_Commission_Card==true)
	{
		tech_menu_disp.font   = font_1;
	}
	else*/
	{
		tech_menu_disp.font   = font_1_invert_text;
	}
	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_1);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 32;
	/*if(gl_Commission_Card==true)
	{
		tech_menu_disp.font   = font_1_invert_text;
		sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_21);
	}
	else*/
	{
		tech_menu_disp.font   = font_1;
		sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_2);
	}
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 32;
	tech_menu_disp.font   = font_1;
	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_4);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	copy_files_screen = true;

	/*if(gl_Commission_Card==true)
	{
		copy_files_screen_count=2;
		copy_all_files=true;
		DK2Mech_copy_option_count=2;
	}
	else*/
	{
		copy_files_screen_count=1;
		copy_all_files=false;
		DK2Mech_copy_option_count=1;
	}
	do_not_copy_files=false;
	copy_only_config=false;
	copy_only_mpb_firmware=false;
	Tech_menu_info = false;

	if(gl_Commission_Card == true)
	{
		advanced_feature  = false;
		clear_alarm_check = false;
		coms_info         = false;
	}
}

/**************************************************************************/
//! Displays mechanism files screen in technician menu
//! \param void
//! \return void
/**************************************************************************/
void tech_mechanism_files()
{
	MPB_CODE_RSAD_DK = AD_MPB_BINARY_DK;
	CONFIG_RSAD_DK = AD_MPB_CONFIG_DK;

	Clear_Screen();

	tech_menu_disp.row   = 0;
	tech_menu_disp.column= 0;//5;
	tech_menu_disp.font  = font_1_invert_text;
	sprintf((char*)tech_menu_disp.text, "DATA KEY FILE MANAGER    ");
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 22;
	tech_menu_disp.column = 0;//10;
	tech_menu_disp.font  = font_1;
	sprintf((char*)tech_menu_disp.text, "[MECHANISM FILES]");
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 22;
	tech_menu_disp.column =0;
	sprintf((char*)tech_menu_disp.text, "LNG BIN VER:%d.%d.%d.%d_4DSM", PCI_VERSION,MPB_VERSION,sub_VERSION,branch_version);	//LNGSIT-818
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 22;
	sprintf((char*)tech_menu_disp.text, "LNG CFG VER:%d.%d.%d.%d", glSystem_cfg.config_version, glSystem_cfg.config_main_version, glSystem_cfg.config_sub_version, glSystem_cfg.config_branch_version);	//LNGSIT-818
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 44;
	tech_menu_disp.column = 0;//10;
	tech_menu_disp.font   = font_1;
	sprintf((char*)tech_menu_disp.text, "[DATAKEY FILES]");
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	MPB_CODE_RSAD_DK = DataKey_Read(MPB_CODE_RSAD_DK,3);
	init_display_SPI_module_GPIO();

	tech_menu_disp.row   += 22;
	tech_menu_disp.column = 0;
	sprintf((char*)tech_menu_disp.text, "LNG BIN VER:%d.%d.%d.%d", PCI_VERSION,DK_data_read_buffer[0],DK_data_read_buffer[1],DK_data_read_buffer[2]);	//LNGSIT-818
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	CONFIG_RSAD_DK = DataKey_Read(CONFIG_RSAD_DK,12);
	init_display_SPI_module_GPIO();

	tech_menu_disp.font   = font_1;
	tech_menu_disp.row   += 22;
	tech_menu_disp.column = 0;
	sprintf((char*)tech_menu_disp.text, "LNG CFG VER:%d.%d.%d.%d", DK_data_read_buffer[8] ,DK_data_read_buffer[9],DK_data_read_buffer[10],DK_data_read_buffer[11]);	//LNGSIT-818
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.font   = font_1;
	tech_menu_disp.row   += 44;
	tech_menu_disp.column = 0;//5;
 	sprintf((char*)tech_menu_disp.text, "PRESS C FOR NEXT SCREEN  ");
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	if(gl_Commission_Card == true) //LNGSIT-1239
	{
		copy_files_screen_count=2;
		copy_files_screen=false;
		file_manager_screen=1;//24-08-12
	}

	advanced_feature  = false;
	clear_alarm_check = false;
	coms_info         = false;
	Tech_menu_info    = false;
}

/**************************************************************************/
//! Send clear alarm command to server in technician menu
//! \param void
//! \return void
/**************************************************************************/
static void tech_clear_alarm()
{
    int16_t result;

    Clear_Screen();

    Static_display          = true;
    glAlarm_req.alarm_type  = 92;
    glAlarm_req.alarm_size  = 3;
    glAlarm_req.alarm_ts    = RTC_epoch_now();
    glAlarm_req.alarm_state = 0x5C;    //MB_ALARM_STATE_CLEARED
    glAlarm_req.alarm_id    = 0 ;

    tech_menu_disp.row    = 0;
    tech_menu_disp.column = 0;//5;
    tech_menu_disp.font   = font_1_invert_text;
    sprintf((char*)tech_menu_disp.text, "SENDING CLR ALARM CMD    ");
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);


    glIn_diag_mode=true;
    result = gprs_do_Alarms( &glAlarm_req);
    glIn_diag_mode=false;

    if(result==GPRS_COMMERR_NONE)
    {
    	do_Alarms_Update = false;
        //Debug_TextOut( 0, "Clear Alarm CMD Sucessful" );
        tech_menu_disp.row    = 0;
        tech_menu_disp.column = 0;//5;
        tech_menu_disp.font   = font_1_invert_text;
        sprintf((char*)tech_menu_disp.text, "CLR ALARM CMD OK         ");
        printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        push_event_to_cache( UDP_EVTTYP_ALARMS_CLEARED );//12-04-12
    }
    else
    {
        //Debug_TextOut( 0, "Clear Alarm CMD failed" );
        tech_menu_disp.row    = 0;
        tech_menu_disp.column = 0;//5;
        tech_menu_disp.font   = font_1_invert_text;
        sprintf((char*)tech_menu_disp.text, "CLR ALARM CMD FAILED     ");
        printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
    }
    Check_Coin_Jam_Alarm(CLEAR_WAKEUP_COIL_ALARM);

    DelayMs(1000);
    tech_meter_info_first_page();

    Static_display      = false;
    reset_timeout_count = false;
}

/**************************************************************************/
//! Read last RTC update time from RTC module and assign to globals.
//! \param void
//! \return void
/******************************************************************************/
void TechMenu_Get_RTC_update_time()
{
    rtc_day   = RTCDAY;
    rtc_month = RTCMON;
    rtc_year  = (uint8_t) (RTCYEAR - REFERENCE_YR);
    rtc_hour  = RTCHOUR;
    rtc_min   = RTCMIN;
}

/**************************************************************************/
//! Read last Event update time from RTC module and assign to globals.
//! \param void
//! \return void
/**************************************************************************/
void TechMenu_Get_Event_Update_Time()
{
    evnt_day   = RTCDAY;
    evnt_month = RTCMON;
    evnt_year  = (uint8_t) (RTCYEAR - REFERENCE_YR);
    evnt_hour  = RTCHOUR;
    evnt_min   = RTCMIN;
}

/**************************************************************************/
//! Displays Tech menu info screen.
//! \param void
//! \return void
/**************************************************************************/
void Technician_Menu_Information()
{
	Clear_Screen();

    tech_menu_disp.row    = 0;
    tech_menu_disp.column = 0;
    tech_menu_disp.font   = font_1_invert_text;
    sprintf((char*)tech_menu_disp.text, "%s", " 1. TECH MENU INFO	        ");
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    // below display is not required in 4 keypad meters and to save some memory // vinay
   // tech_menu_disp.row   += 32;//to be updated
    tech_menu_disp.column = 0;//5;
    tech_menu_disp.font   = font_1;
   // sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu1_4);
   // printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 32;
    sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu1_1);
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 32;
    sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu1_2);
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 32;
    sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu1_3);
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    main_menu      = false;
    Tech_menu_info = true;
}

#if	0
/*******************************************************************************************************************/
/*  Name        : tech_power_information                                                                           */
/*  Parameters  : void					                                                                           */
/*  Returns     : void                                                                                             */
/*  Function    : Read(Vbat,Vback_up,Vsolar1,Vsolar2) voltage and Send commands to MB to display power info screen.*/
/*-----------------------------------------------------------------------------------------------------------------*/
static void tech_power_information()
{
	Clear_Screen();

	uint16_t   MSP_Vbat = 0;

    MSP_Vbat = Rechargeable_battery_read();

    tech_menu_disp.row   += 22;
    tech_menu_disp.column = 0;//5;
    tech_menu_disp.font   = font_1;
    sprintf((char*)tech_menu_disp.text, "BATT       : %d.%02dvolts", (MSP_Vbat/100),(MSP_Vbat%100));
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    MSP_Vbat = Solar_voltage_read();

    tech_menu_disp.row   += 22;
    tech_menu_disp.column = 0;//5;
    sprintf((char*)tech_menu_disp.text, "SOLAR      : %d.%02dvolts", (MSP_Vbat/100),(MSP_Vbat%100));
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
}
#endif

/**************************************************************************/
//! Displays Meter info screen1.
//! \param void
//! \return void
/**************************************************************************/
void tech_meter_info_first_page()
{
	uint8_t l,m=0;

	Clear_Screen();

	tech_menu_disp.row    = 0;
	tech_menu_disp.column = 0;
	tech_menu_disp.font   = font_1_invert_text;
	sprintf((char*)tech_menu_disp.text, "%s", " 2. METER INFORMATION	    ");
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 22;
	tech_menu_disp.column = 0;//5;
	tech_menu_disp.font   = font_1;
	sprintf((char*)tech_menu_disp.text, "MECH SN    : %s", glMB_info.serial_no_asci);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 22;
	for(l=0;l<20;l++)
	{
		if(glMB_info.locid[l]==0xff)
			m++;
	}
	if(m!=20)
	{
		sprintf((char*)tech_menu_disp.text, "LID        : %s", glMB_info.locid);
	}
	else
	{
		sprintf((char*)tech_menu_disp.text, "LID        : %s", "\0");
	}
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 22;
	sprintf((char*)tech_menu_disp.text, "CITY/CUSTID: %d/%u", glMB_info.city_code,glSystem_cfg.cust_id);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 22;
	sprintf((char*)tech_menu_disp.text, "MID/AID    : %u/%u", glSystem_cfg.meter_id,glSystem_cfg.area_num);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
	Flash_Batch_Flag_Read();

	tech_menu_disp.row   += 22;
	//sprintf((char*)tech_menu_disp.text, "COIN/CARD  : %d/%d", Coin_Q_count,OLT_Q_count);
	sprintf((char*)tech_menu_disp.text, "COIN/CARD  : %d/%d", glTotal_Coin_Count,OLT_Q_count);
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 22;
	//LNGSIT-2053 //they wanted to see hardware config in tech menu //vinay
	if(HARDWARE_REVISION == LNG_REV3)
	{
		if(Connected_Card_reader == GEM_CLUB_READER)
		{
			if(REV4_LED_CONFIG == true)
				sprintf((char*)tech_menu_disp.text, "HW CFG     : R3MB+R4C+GC ");
			else
				sprintf((char*)tech_menu_disp.text, "HW CFG     : R3MB+R3C+GC ");
		}
		else
		{
			if(REV4_LED_CONFIG == true)
				sprintf((char*)tech_menu_disp.text, "HW CFG     : R3MB+R4C+ID ");
			else
				sprintf((char*)tech_menu_disp.text, "HW CFG     : R3MB+R3C+ID ");
		}
	}
	else
	{
		if(Connected_Card_reader == GEM_CLUB_READER)
		{
			if(REV4_LED_CONFIG == true)
				if(Front_Bi_Color == true)
					sprintf((char*)tech_menu_disp.text, "HW CFG  : R4MB+R4C+GC+FBi ");
				else
					sprintf((char*)tech_menu_disp.text, "HW CFG  : R4MB+R4C+GC+FUi ");
			else
				sprintf((char*)tech_menu_disp.text, "HW CFG     : R4MB+R3C+GC ");
		}
		else
		{
			if(REV4_LED_CONFIG == true)
				if(Front_Bi_Color == true)
					sprintf((char*)tech_menu_disp.text, "HW CFG  : R4MB+R4C+ID+FBi ");
				else
					sprintf((char*)tech_menu_disp.text, "HW CFG  : R4MB+R4C+ID+FUi ");
			else
				sprintf((char*)tech_menu_disp.text, "HW CFG     : R4MB+R3C+ID ");
		}
	}
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 44;
	tech_menu_disp.column = 0;//5;
	tech_menu_disp.font   = font_1_invert_text;
	sprintf((char*)tech_menu_disp.text, "PRESS OK FOR NEXT PAGE    ");
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	Display_meter_info_2ndpage = true;
	main_menu                  = false;
}

/**************************************************************************/
//! Displays Meter info screen2.
//! \param void
//! \return void
/**************************************************************************/
void tech_meter_info_second_page()
{
	uint16_t   MSP_Vbat = 0;
    uint8_t reg_stat = (mdm_comm_status > 3) ? 1 : 0;//11-04-12

    Clear_Screen();

	tech_menu_disp.row    = 0;
	tech_menu_disp.column = 0;
	tech_menu_disp.font   = font_1_invert_text;
	sprintf((char*)tech_menu_disp.text, "%s", " 2. METER INFORMATION	    ");
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    //tech_power_information();
    MSP_Vbat = Rechargeable_battery_read();

    tech_menu_disp.row   += 22;
    tech_menu_disp.column = 0;//5;
    tech_menu_disp.font   = font_1;
    sprintf((char*)tech_menu_disp.text, "BATT       : %d.%02dvolts", (MSP_Vbat/100),(MSP_Vbat%100));
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    /*MSP_Vbat = Solar_voltage_read();

    tech_menu_disp.row   += 22;
    tech_menu_disp.column = 0;//5;
    sprintf((char*)tech_menu_disp.text, "SOLAR      : %d.%02dvolts", (MSP_Vbat/100),(MSP_Vbat%100));
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);*/

    tech_menu_disp.row   += 22;
    sprintf((char*)tech_menu_disp.text,"SIG        : %d\0", sys_mdm_p.signal_strength);//11-04-12
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 22;
    sprintf((char*)tech_menu_disp.text,"REG        : %d\0", reg_stat);//11-04-12
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    if(glAlarm_req.alarm_state == MB_ALARM_STATE_RAISED)
    {
        tech_menu_disp.row   += 22;
        sprintf((char*)tech_menu_disp.text,"ALARM CODE : %d\0", glAlarm_req.alarm_id);
        printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
    }
    else
    {
        glAlarm_req.alarm_id  = 0;
        tech_menu_disp.row   += 22;
        sprintf((char*)tech_menu_disp.text,"ALARM CODE : %d\0", glAlarm_req.alarm_id);
        printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
    }

    clear_alarm_check = true;

    tech_menu_disp.row   = 22*7;
    tech_menu_disp.font   = font_1_invert_text;
    sprintf((char*)tech_menu_disp.text,"%s", Main_Tech_menu5);
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    Display_meter_info_2ndpage = false;
}

/**************************************************************************/
//! Displays Software information screen.
//! \param void
//! \return void
/**************************************************************************/
static void tech_software_information()
{
	Clear_Screen();

	tech_menu_disp.row    = 0;
    tech_menu_disp.column = 0;
    tech_menu_disp.font   = font_1_invert_text;
    sprintf((char*)tech_menu_disp.text, "%s", " 4. SOFTWARE INFORMATION   ");
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 44;
    tech_menu_disp.column = 0;
    tech_menu_disp.font   = font_1;
    sprintf((char*)tech_menu_disp.text, "LNG BIN VER:%d.%d.%d.%d_4DSM", PCI_VERSION,MPB_VERSION,sub_VERSION,branch_version);	//LNGSIT-818
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 22;
    tech_menu_disp.column = 0;
    tech_menu_disp.font   = font_1;
    sprintf((char*)tech_menu_disp.text, "LNG CFG VER:%d.%d.%d.%d", glSystem_cfg.config_version,glSystem_cfg.config_main_version,glSystem_cfg.config_sub_version,glSystem_cfg.config_branch_version);	//LNGSIT-818
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    advanced_feature    =  true;
    main_menu           = false;

    tech_menu_disp.row   = 22*7;
    tech_menu_disp.column= 0;//5;
    tech_menu_disp.font   = font_1_invert_text;
    sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_8);
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
}

/**************************************************************************/
//! Displays Communication information screen.
//! \param void
//! \return void
/**************************************************************************/
static void tech_communication_information()
{
	Clear_Screen();
	uint32_t lRFFrequency = 0;
	uint8_t apn_length;

    tech_menu_disp.row    = 0;
    tech_menu_disp.column = 0;
    tech_menu_disp.font   = font_1_invert_text;
    sprintf((char*)tech_menu_disp.text, "%s", " 3. COMMS INFORMATION      ");
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 32;
    tech_menu_disp.column = 0;//5;
    tech_menu_disp.font   = font_1;
    sprintf((char*)tech_menu_disp.text, "IMSI    : %s",sys_mdm_p.IMSI);//01-06-12:broken part with merging
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 22;
    sprintf((char*)tech_menu_disp.text, "SERVRIP : %s", glSystem_cfg.UDP_server_ip);//11-06-12:Only Truncated string, cannot display full string
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 22;
    sprintf((char*)tech_menu_disp.text, "SERVRPRT: %d", glSystem_cfg.UDP_server_port);
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 22;
    sprintf((char*)tech_menu_disp.text, "MTRIP   : %s",sys_mdm_p.local_ip_add);//01-06-12:broken part with merging//11-06-12:Only Truncated string, cannot display full string
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
	//this below length calculator is used check whether the length of apn name is long enough to accommodate the display or not.
    apn_length = strlen((const char*)glSystem_cfg.apn); //LNGSIT-1028
    if(apn_length > 15)//it can accommodate only 15 chars if its more ignore it, else screen will be corrupted vinay
    {
    	apn_length = 16;
    	glSystem_cfg.apn[apn_length-1] = 0;	//Null Terminate
    }

    tech_menu_disp.row   += 22;
    sprintf((char*)tech_menu_disp.text, "APN     : %s", glSystem_cfg.apn);
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    lRFFrequency = IDBasedFreqCalculation(glSystem_cfg.meter_id ); // Freq Calculation

    tech_menu_disp.row   += 22;
    sprintf((char*)tech_menu_disp.text, "LoRa    : %d(G), %d(S)", glSystem_cfg.Active_Channel, (uint32_t)lRFFrequency/1000000L);
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 22;
    sprintf((char*)tech_menu_disp.text,"LASTRTC %02d/%02d/%02d %02d:%02d\0", rtc_month,rtc_day,rtc_year,rtc_hour,rtc_min);//27-11-12
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 22;
    sprintf((char*)tech_menu_disp.text,"LASTEVT %02d/%02d/%02d %02d:%02d\0", evnt_month,evnt_day,evnt_year,evnt_hour,evnt_min);//27-11-12
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);



    main_menu  = false;
    coms_info  = true;
}

/**************************************************************************/
//! Displays Communication Intervals selection screen.
//! \param void
//! \return void
/**************************************************************************/
void tech_comm_intervals()
{
	Clear_Screen();

	tech_menu_disp.row    = 0;
	tech_menu_disp.column = 0;//10;
	tech_menu_disp.font   = font_1_invert_text;
	sprintf((char*)tech_menu_disp.text, "    COMM INTERVALS       ");
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 32;
	tech_menu_disp.column = 0;
	tech_menu_disp.font   = font_1_invert_text;
	sprintf((char*)tech_menu_disp.text, "TRIGGER GENSYNC?NO      " );
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.font   = font_1;
	tech_menu_disp.row   += 32;
	sprintf((char*)tech_menu_disp.text, "TRIGGER EVENT?NO" );
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	tech_menu_disp.row   += 32;
	sprintf((char*)tech_menu_disp.text, "SAVE CHANGES?NO" );
	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

	do_RTC_EVT_AFM_HB=0;
	comm_int_scrn=true;
	save_comm_interval_changes=false;
	comm_int_scrn_count=1;
	advanced_feature=false;
}

/**************************************************************************/
//! Read particular file from flash and write(copy) to data key.
//! \param uint8_t filetype
//! \param uint32_t filesize
//! \param uint8_t erase_flag
//! \return uint8_t
//! - \b Success
//! - \b Failure
/**************************************************************************/
static uint8_t Copy_from_flash_to_datakey(uint8_t fileType, uint32_t size,uint8_t erase_flag )
{
	uint16_t       i,  segment_size = FLASH_SEGMENT_SIZE;
	uint16_t       loop = (size > segment_size) ? (uint16_t) (size/segment_size)+1 : 1;
	uint32_t       offset = 0;
	uint32_t       bytes_left = 0;
	uint8_t 	   mpb_code_ver[3];

	Static_display=true;//11-04-12
	tech_mode_timeout=0;//09-04-12

	config_RSAD   	 = AD_MPB_config_r;
	mpb_code_RSAD	 = AD_MSP_CODE_r;
	CONFIG_WSAD_DK   = AD_MPB_CONFIG_DK;
	MPB_CODE_WSAD_DK = AD_MPB_BINARY_DK;
	//Debug_Output1(0,"loop=%d",loop);
	for(i=0; i<loop; i++)	//(i=loop; i<loop; i++)//12-06-12:?? broken part
	{
		//Debug_Output1(0,"i=%d",i);
		watchdog_pat();
		bytes_left = size-offset;

		if(bytes_left < segment_size)
		{
			segment_size = (uint16_t) bytes_left;
		}

		switch(fileType)
		{
		case TECH_MENU_MPB_CODE:
			if((uint32_t)(MPB_CODE_WSAD_DK+segment_size)<=(uint32_t)(0x60FFF))//25-05-12
			{
				//mpb_code_RSAD=Flash_Read(mpb_code_RSAD,segment_size);
				if(erase_flag==MULTIPLE_BLOCK_ERASE_FLAG)
				{
					//dummy write function call to erase one extra sector to accommodate the extra 11 bytes
					//here only the extra info related to binary version is written into the memory so any changes you want to write then use it.
					MPB_CODE_WSAD_DK = DataKey_Write(DK_data_read_buffer,0,AD_MPB_BINARY_PLUS_ONE_DK,SINGLE_SECTOR_ERASE);

					MPB_CODE_WSAD_DK = AD_MPB_BINARY_DK;
					mpb_code_ver[0]=MPB_VERSION;
					mpb_code_ver[1]=sub_VERSION;
					mpb_code_ver[2]=branch_version;
					MPB_CODE_WSAD_DK=DataKey_Write(mpb_code_ver,sizeof(mpb_code_ver),MPB_CODE_WSAD_DK,erase_flag);
					erase_flag=0;
					//05-04-12
					file_sizearr[0] = (uint8_t) (MPB_CODE_SIZE >> 24);//12-06-12
					file_sizearr[1] = (uint8_t) (MPB_CODE_SIZE >> 16);
					file_sizearr[2] = (uint8_t) (MPB_CODE_SIZE >> 8);
					file_sizearr[3] = (uint8_t) (MPB_CODE_SIZE);
					MPB_CODE_WSAD_DK=DataKey_Write(file_sizearr,sizeof(file_sizearr),MPB_CODE_WSAD_DK,erase_flag);
					//05-04-12

					MPB_CODE_WSAD_DK += sizeof(Flash_File_CRC);
					//4 bytes allocated to write the file CRC, it will be written after copying
					mpb_code_RSAD=Flash_Read(mpb_code_RSAD,segment_size);
					MPB_CODE_WSAD_DK=DataKey_Write(flash_data_read_buffer,segment_size,MPB_CODE_WSAD_DK,erase_flag);
					//Debug_Output2(2, "Flash:%08x, DK:%08x", mpb_code_RSAD, MPB_CODE_WSAD_DK);
				}
				else
				{
					mpb_code_RSAD=Flash_Read(mpb_code_RSAD,segment_size);
					MPB_CODE_WSAD_DK=DataKey_Write(flash_data_read_buffer,segment_size,MPB_CODE_WSAD_DK,erase_flag);
					//Debug_Output2(2, "Flash:%08x, DK:%08x", mpb_code_RSAD, MPB_CODE_WSAD_DK);
				}
				erase_flag=0;//25-05-12
			}
			break;

		case TECH_MENU_MPB_CONFIG:
			if((uint32_t)(CONFIG_WSAD_DK+segment_size)<=(uint32_t)(AD_MPB_CONFIG_DK+(FLASH_SECTOR_SIZE*2)))
			{
				config_RSAD=Flash_Read(config_RSAD,segment_size);
				if(i == 0)
				{
					Flash_File_CRC = (flash_data_read_buffer[7]<<24) | (flash_data_read_buffer[6]<<16) | (flash_data_read_buffer[5]<<8) | (flash_data_read_buffer[4]);
					//Debug_Output6(2, "Flash CRC:%02x,%02x,%02x,%02x", flash_data_read_buffer[4], flash_data_read_buffer[5], flash_data_read_buffer[6], flash_data_read_buffer[7], 0, 0);
				}
				CONFIG_WSAD_DK=DataKey_Write(flash_data_read_buffer,segment_size,CONFIG_WSAD_DK,erase_flag);
				//Debug_Output2(2, "Flash:%08x, DK:%08x", config_RSAD, CONFIG_WSAD_DK);
				erase_flag=0;
			}
			break;
		}
		offset +=segment_size;
		//this error handling is necessary
		//avoid copying corrupted files; if half copied, erase it; using this dummy write function to erase
		if((fileType==TECH_MENU_MPB_CONFIG)&&(CONFIG_WSAD_DK==0))
		{
			CONFIG_WSAD_DK = AD_MPB_CONFIG_DK;
			CONFIG_WSAD_DK = DataKey_Write(DK_data_read_buffer,0,CONFIG_WSAD_DK,true);
			return 0;
		}
		else if((fileType==TECH_MENU_MPB_CODE)&&(MPB_CODE_WSAD_DK==0))
		{
			MPB_CODE_WSAD_DK = AD_MPB_BINARY_DK;
			MPB_CODE_WSAD_DK=DataKey_Write(DK_data_read_buffer,0,MPB_CODE_WSAD_DK,MULTIPLE_BLOCK_ERASE_FLAG);
			//dummy write function call to erase one extra sector to accommodate the extra 11 bytes
			MPB_CODE_WSAD_DK = DataKey_Write(DK_data_read_buffer,0,AD_MPB_BINARY_PLUS_ONE_DK,SINGLE_SECTOR_ERASE);

			return 0;
		}
		//22-04-2013:avoid copying corrupted files
	}

	//CRC validation
	if(fileType==TECH_MENU_MPB_CONFIG)
	{
	    watchdog_pat();
		DK_File_CRC = Calculate_File_CRC_for_DK_copy(FILE_TYPE_MPB_CONFIG, DATAKEY_SPI);
		watchdog_pat();
        Flash_File_CRC = Calculate_File_CRC_for_DK_copy(FILE_TYPE_MPB_CONFIG, FLASH_SPI);
        watchdog_pat();
        Debug_Output2(0,"Flash CRC= %08lX, DK CRC= %08lX", Flash_File_CRC, DK_File_CRC);
		if(DK_File_CRC != Flash_File_CRC)
		{
			Debug_Output2(0,"CRC Mismatch: Flash CRC= %08lX, DK CRC= %08lX", Flash_File_CRC, DK_File_CRC);
			//Erase datakey config section to avoid using corrupt file
			CONFIG_WSAD_DK = AD_MPB_CONFIG_DK;
			CONFIG_WSAD_DK = DataKey_Write(DK_data_read_buffer,0,CONFIG_WSAD_DK,true);
			return 0;
		}
		else
		{
			//Debug_TextOut(2,"DK Flash CRC Match");
		}
	}
	else if(fileType==TECH_MENU_MPB_CODE)
	{
		watchdog_pat();
		DK_File_CRC = Calculate_File_CRC_for_DK_copy(FILE_TYPE_MPB_CODE, DATAKEY_SPI);
		watchdog_pat();
		Flash_File_CRC = Calculate_File_CRC_for_DK_copy(FILE_TYPE_MPB_CODE, FLASH_SPI);
		watchdog_pat();
		if(DK_File_CRC != Flash_File_CRC)
		{
			Debug_Output2(0,"CRC Mismatch: Flash= %08lX, DK= %08lX", Flash_File_CRC, DK_File_CRC);
			//Erase datakey Binary section to avoid using corrupt file
			MPB_CODE_WSAD_DK = AD_MPB_BINARY_DK;
			MPB_CODE_WSAD_DK=DataKey_Write(DK_data_read_buffer,0,MPB_CODE_WSAD_DK,MULTIPLE_BLOCK_ERASE_FLAG);
			//dummy write function call to erase one extra sector to accommodate the extra 11 bytes
			MPB_CODE_WSAD_DK = DataKey_Write(DK_data_read_buffer,0,AD_MPB_BINARY_PLUS_ONE_DK,SINGLE_SECTOR_ERASE);
			return 0;
		}
		else
		{
			//Write the calculated CRC in datakey
			MPB_CODE_WSAD_DK = AD_MPB_BINARY_DK + 7;
			file_sizearr[0] = (uint8_t) (Flash_File_CRC >> 24);//12-06-12
			file_sizearr[1] = (uint8_t) (Flash_File_CRC >> 16);
			file_sizearr[2] = (uint8_t) (Flash_File_CRC >> 8);
			file_sizearr[3] = (uint8_t) (Flash_File_CRC);
			MPB_CODE_WSAD_DK=DataKey_Write(file_sizearr,sizeof(file_sizearr),MPB_CODE_WSAD_DK,false);

			Debug_Output1(2,"DK Flash CRC Match=%08ld", DK_File_CRC);
		}
	}
	return 1;   // success
}

/**************************************************************************/
//! Read particular file from datakey and write(copy) to flash.
//! \param uint8_t filetype
//! \param uint32_t filesize
//! \return uint8_t
//! - \b Success
//! - \b Failure
/**************************************************************************/
static uint8_t Copy_from_datakey_to_flash(uint8_t fileType, uint32_t size)
{
	uint16_t       i,  segment_size = FLASH_SEGMENT_SIZE;
	uint16_t       loop = (size > segment_size) ? (uint16_t) (size/segment_size)+1 : 1;
	uint32_t       offset = 0;
	uint32_t       bytes_left = 0;
	uint8_t        config_erase_flag = true;
	uint8_t        mpb_code_erase_flag = MULTIPLE_BLOCK_ERASE_FLAG;

	Static_display=true;//11-04-12
	tech_mode_timeout=0;//09-04-12

	CONFIG_RSAD_DK   = AD_MPB_CONFIG_DK;
	MPB_CODE_RSAD_DK = AD_MPB_BINARY_DK+FLASH_FILE_SIZE_LEN+3+4;//25-05-12//exclude file size,CRC and version while writing to flash
	config_WSAD   = AD_MPB_config_w;
	mpb_code_WSAD = AD_MSP_CODE_w;
	//Debug_Output1(0,"loop=%d",loop);
	for(i=0; i<loop; i++)//(i=loop; i<loop; i++)//12-06-12:?? broken part
	{
		//Debug_Output1(0,"i=%d",i);
		watchdog_pat();
		bytes_left = size-offset;

		if(bytes_left < FLASH_SEGMENT_SIZE)
		{
			segment_size = (uint16_t) bytes_left;
		}

		switch(fileType)
		{
		case TECH_MENU_MPB_CODE:
			if((uint32_t) (mpb_code_WSAD+segment_size) <= (uint32_t)(0x05FFFF) )//25-05-12
			{
				MPB_CODE_RSAD_DK = DataKey_Read(MPB_CODE_RSAD_DK,segment_size);
				mpb_code_WSAD    = Flash_Write(DK_data_read_buffer,segment_size, mpb_code_WSAD, mpb_code_erase_flag);
				//Debug_Output2(2, "DK:%08x, Flash:%08x", MPB_CODE_RSAD_DK, mpb_code_WSAD);
				mpb_code_erase_flag = 0;//25-05-12
			}
			break;

		case TECH_MENU_MPB_CONFIG:
			if((uint32_t) (config_WSAD + segment_size) < (uint32_t)(AD_MPB_config_w + (FLASH_SECTOR_SIZE*2)) )//10-04-12
			{
				CONFIG_RSAD_DK = DataKey_Read(CONFIG_RSAD_DK,segment_size);
				if(i == 0)
				{
					DK_File_CRC = (DK_data_read_buffer[7]<<24) | (DK_data_read_buffer[6]<<16) | (DK_data_read_buffer[5]<<8) | (DK_data_read_buffer[4]);
					//Debug_Output6(2, "DK CRC:%02x,%02x,%02x,%02x", DK_data_read_buffer[4], DK_data_read_buffer[5], DK_data_read_buffer[6], DK_data_read_buffer[7], 0, 0);
				}
				config_WSAD    = Flash_Write(DK_data_read_buffer, segment_size,config_WSAD, config_erase_flag);
				//Debug_Output2(2, "DK:%08x, Flash:%08x", CONFIG_RSAD_DK, config_WSAD);
				config_erase_flag = 0;//25-05-12
			}
			break;
		}
		offset +=segment_size;//18-06-12
		//this error handling is necessary
		//avoid copying corrupted files; if half copied, erase it; using this dummy write function to erase
		if((fileType == TECH_MENU_MPB_CONFIG) && (config_WSAD==0))
		{
			config_erase_flag   = true;
			config_WSAD   = AD_MPB_config_w;
			Flash_Batch_Erase(config_WSAD, config_erase_flag, false);
			return 0;
		}
		else if((fileType == TECH_MENU_MPB_CODE) && (mpb_code_WSAD==0))
		{
			mpb_code_erase_flag = MULTIPLE_BLOCK_ERASE_FLAG;
			mpb_code_WSAD = AD_MSP_CODE_w;
			Flash_Batch_Erase(mpb_code_WSAD, mpb_code_erase_flag, false);
			return 0;
		}
		//22-04-2013:avoid copying corrupted files
	}
	//CRC validation
	if(fileType==TECH_MENU_MPB_CONFIG)
	{
        watchdog_pat();
        DK_File_CRC = Calculate_File_CRC_for_DK_copy(FILE_TYPE_MPB_CONFIG, DATAKEY_SPI);
        watchdog_pat();
		Flash_File_CRC = Calculate_File_CRC_for_DK_copy(FILE_TYPE_MPB_CONFIG, FLASH_SPI);
		watchdog_pat();
		if(DK_File_CRC != Flash_File_CRC)
		{
			Debug_Output2(0,"CRC Mismatch: Flash= %08lX, DK= %08lX", Flash_File_CRC, DK_File_CRC);
			//Erase Flash config write section to avoid using corrupt file
			config_erase_flag   = true;
			config_WSAD   = AD_MPB_config_w;
			Flash_Batch_Erase(config_WSAD, config_erase_flag, false);
			return 0;
		}
		else
		{
			//Debug_Output1(2,"DK Flash CRC Match=%08lX", Flash_File_CRC);
		}
	}
	else if(fileType==TECH_MENU_MPB_CODE)
	{
		watchdog_pat();
		DK_File_CRC = Calculate_File_CRC_for_DK_copy(FILE_TYPE_MPB_CODE, DATAKEY_SPI);
		watchdog_pat();
		Flash_File_CRC = Calculate_File_CRC_for_DK_copy(FILE_TYPE_MPB_CODE, FLASH_SPI);
		watchdog_pat();
		if(DK_File_CRC != Flash_File_CRC)
		{
			Debug_Output2(0,"CRC Mismatch: Flash= %08lX, DK= %08lX", Flash_File_CRC, DK_File_CRC);
			//Erase flash Binary section to avoid using corrupt file
			mpb_code_erase_flag = MULTIPLE_BLOCK_ERASE_FLAG;
			mpb_code_WSAD = AD_MSP_CODE_w;
			Flash_Batch_Erase(mpb_code_WSAD, mpb_code_erase_flag, false);
			return 0;
		}
		else
		{
			//Debug_Output1(2,"DK Flash CRC Match=%08lX", DK_File_CRC);
		}
	}

	return 1;   // success
}

/**************************************************************************/
//! Displays Adjust clock screen.
//! \param void
//! \return void
/**************************************************************************/
static void tech_adjust_clock()
{
    Adj_clk_curr_mdy_hour  = RTCHOUR;
    Adj_clk_curr_mdy_min   = RTCMIN;
    Adj_clk_curr_mdy_day   = RTCDAY;
    Adj_clk_curr_mdy_month = RTCMON ;
    Adj_clk_curr_mdy_year  = (uint8_t)(RTCYEAR - REFERENCE_YR);
    Adj_clk_curr_mdy_dow   = (uint8_t)RTCDOW;

    Clear_Screen();

    tech_menu_disp.row = 0; //commented to add dow
    tech_menu_disp.column = 0;//5;
    tech_menu_disp.font   = font_1_invert_text;
    sprintf((char*)tech_menu_disp.text, "MONTH?  %02d               ", Adj_clk_curr_mdy_month);
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 22;
    tech_menu_disp.font   = font_1;
    sprintf((char*)tech_menu_disp.text, "DAY?    %02d             ", Adj_clk_curr_mdy_day);
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 22;
    sprintf((char*)tech_menu_disp.text, "YEAR?   %02d             ", Adj_clk_curr_mdy_year);
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 22;
    sprintf((char*)tech_menu_disp.text, "DOW?    %02d             ", Adj_clk_curr_mdy_dow);
    tech_menu_disp.font  = font_1;
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 22;
    sprintf((char*)tech_menu_disp.text, "HOUR?   %02d             ", Adj_clk_curr_mdy_hour);
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    tech_menu_disp.row   += 22;
    sprintf((char*)tech_menu_disp.text, "MIN?    %02d             ", Adj_clk_curr_mdy_min);
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

//01-06-12:Broken part with merging
    tech_menu_disp.row   += 22;
    sprintf((char*)tech_menu_disp.text, "SAVE CHANGES? NO         ");
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//01-06-12
    tech_menu_disp.row   += 22;
    tech_menu_disp.font  = font_1;
    sprintf((char*)tech_menu_disp.text,"PREVW: %02d/%02d/%02d %02d:%02d\0", Adj_clk_curr_mdy_month,Adj_clk_curr_mdy_day,Adj_clk_curr_mdy_year,Adj_clk_curr_mdy_hour,Adj_clk_curr_mdy_min);
    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    copy_files_screen = false;
    advanced_feature  = false;
    clear_alarm_check = false;
    coms_info         = false;
    Tech_menu_info    = false;
    Adjust_clock      = true;
    Save_Clk_Changes  =false;
    Adjust_clock_key_count=1;
	main_menu                  = false;
}

/**************************************************************************/
//! Set MSP RTC with the modified clock values in adjust clock mode.
//! \param void
//! \return void
/**************************************************************************/
static void tech_set_RTC_in_TechMode()
{
	RTC_C_disableInterrupt(RTC_C_TIME_EVENT_INTERRUPT);
	rtc_c_calender.minutes = Adj_clk_curr_mdy_min;
	rtc_c_calender.hours   = Adj_clk_curr_mdy_hour;
	rtc_c_calender.dayOfWeek = Adj_clk_curr_mdy_dow;
	rtc_c_calender.dayOfmonth = Adj_clk_curr_mdy_day;
	rtc_c_calender.month = Adj_clk_curr_mdy_month;
	rtc_c_calender.year = Adj_clk_curr_mdy_year + REFERENCE_YR;
	RTC_C_holdClock();
	RTC_C_initCalendar(&rtc_c_calender,RTC_C_FORMAT_BINARY);
	RTC_C_setCalendarEvent(RTC_C_CALENDAREVENT_MINUTECHANGE);
	RTC_C_startClock();
	NVIC->ISER[0] = 1 << ((RTC_C_IRQn) & 31); //Enable RTC_C_IRQn interrupt in NVIC module
	RTC_C_enableInterrupt(RTC_C_TIME_EVENT_INTERRUPT);

	Last_Comms_Success_RTCTimestamp = RTC_epoch_now();
}

/**************************************************************************/
//! Process UP key in technician menu.
//! \param void
//! \return void
/**************************************************************************/
void TechMenu_ProcessKey_UP_In_TechMode()
{
    if(Static_display == true)//11-04-12
    {
        return;
    }

    if(clear_alarm_check==true)
    {
        tech_menu_disp.column= 0;//5;
        tech_menu_disp.row   = 22*7;//05-04-12
        tech_menu_disp.font  = font_1_invert_text;

        sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu5);
        printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        tech_menu_disp.row+=22;
        clr_alarm=false;
    }
    else if(advanced_feature==true)
    {
        if(advanced_feature_count>1)
            advanced_feature_count--;
        else
            advanced_feature_count=Advanced_Feature_Count;

        if(advanced_feature_count==1)
        {
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_8);
        }
        else if(advanced_feature_count==2)
        {
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_1); //SIT DSM clear space VINAY
        }
//        else if(advanced_feature_count==3)//vinay code opt
//        {
//        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_1_1); //SIT DSM clear space VINAY
//        }
//        else if(advanced_feature_count==4)
//        {
//        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_1_2); //SIT DSM clear space VINAY
//        }
        else if(advanced_feature_count==3)//(advanced_feature_count==5)
        {
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_2); //SIT DSM Credit space Vinay
        }
//        else if(advanced_feature_count==6) //vinay code opt
//         {
//         	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_2_1); //SIT DSM Credit space Vinay
//         }
//        else if(advanced_feature_count==7)//vinay code opt
//         {
//         	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_2_2); //SIT DSM Credit space Vinay
//         }
//        else if(advanced_feature_count==8)//vinay code opt
//        {
//        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_5);
//        }
        else if(advanced_feature_count==4)//(advanced_feature_count==9)//11-06-2013:DPLIBB-531
        {
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7);
        }
        else if(advanced_feature_count==5)//(advanced_feature_count==10)//05-08-2013:DPLIBB-554
        {
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_9);
        }
	    else if(advanced_feature_count==6)//(advanced_feature_count==11)//05-08-2013:DPLIBB-554
	    {
	    	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_10);
	    }
        else if(advanced_feature_count==7)//(advanced_feature_count==12)
        {
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_11);
        }
    	tech_menu_disp.column= 0;//5;
    	tech_menu_disp.row   = 22*7;
    	tech_menu_disp.font  = font_1_invert_text;
    	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
    }
    else if(copy_files_screen==true)
    {
    	if(copy_files_screen_count==1)
    	{
    		if(Mech2DK_copy_option_count>1)
    			Mech2DK_copy_option_count--;
    		else
    			Mech2DK_copy_option_count=4;
    		tech_menu_disp.column= 0;//5;
    		tech_menu_disp.row   = 32*(copy_files_screen_count+1);
    		tech_menu_disp.font  = font_1_invert_text;
    		if(Mech2DK_copy_option_count==1)
    		{
    			do_not_copy_files=true;
    			copy_all_files=false;
    			copy_only_config=false;
    			copy_only_mpb_firmware=false;
    			sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_1);
    		}
    		else if(Mech2DK_copy_option_count==2)
    		{
    			copy_all_files=true;
    			do_not_copy_files=false;
    			copy_only_config=false;
    			copy_only_mpb_firmware=false;
    			sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_11);

    		}
    		else if(Mech2DK_copy_option_count==3)
    		{
    			copy_only_config=true;
    			copy_all_files=false;
    			do_not_copy_files=false;
    			copy_only_mpb_firmware=false;
    			sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_12);
    		}
    		else if(Mech2DK_copy_option_count==4)
    		{
    			copy_only_mpb_firmware=true;
    			copy_only_config=false;
    			copy_all_files=false;
    			do_not_copy_files=false;
    			sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_13);
    		}
    		printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
    	}

    	if(copy_files_screen_count==2)
    	{
    		if(DK2Mech_copy_option_count>1)
    			DK2Mech_copy_option_count--;
    		else
    			DK2Mech_copy_option_count=4;
    		tech_menu_disp.column= 0;//5;
    		tech_menu_disp.row   = 32*(copy_files_screen_count+1);
    		tech_menu_disp.font  = font_1_invert_text;
    		if(DK2Mech_copy_option_count==1)
    		{
    			do_not_copy_files=true;
    			copy_all_files=false;
    			copy_only_config=false;
    			copy_only_mpb_firmware=false;
    			sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_2);
    		}
    		else if(DK2Mech_copy_option_count==2)
    		{
    			copy_all_files=true;
    			do_not_copy_files=false;
    			copy_only_config=false;
    			copy_only_mpb_firmware=false;
    			sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_21);

    		}
    		else if(DK2Mech_copy_option_count==3)
    		{
    			copy_only_config=true;
    			copy_all_files=false;
    			do_not_copy_files=false;
    			copy_only_mpb_firmware=false;
    			sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_22);
    		}
    		else if(DK2Mech_copy_option_count==4)
    		{
    			copy_only_mpb_firmware=true;
    			copy_only_config=false;
    			copy_all_files=false;
    			do_not_copy_files=false;
    			sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_23);
    		}
    		printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
    	}

    	if(copy_files_screen_count==3)//05-02-2014:DPLIBB-611
    	{
    		exit_filemgr=false;
    		tech_menu_disp.column= 0;//5;
    		tech_menu_disp.row   = 32*(copy_files_screen_count+1);
    		tech_menu_disp.font  = font_1_invert_text;
    		sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_4);
    		printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
    	}
    }
    else if(Adjust_clock==true)
    {
        if(Adjust_clock_key_count==1)
        {
            local_value=Adj_clk_curr_mdy_month;
            if(local_value<12)
                local_value++;
            else
                local_value=1;
            Adj_clk_curr_mdy_month=local_value;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "MONTH?  %02d               ", Adj_clk_curr_mdy_month);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            tech_menu_disp.row   = 22*Clock_parm_count;
            tech_menu_disp.font  = font_1;
            sprintf((char*)tech_menu_disp.text,"PREVW: %02d/%02d/%02d %02d:%02d\0", Adj_clk_curr_mdy_month,Adj_clk_curr_mdy_day,Adj_clk_curr_mdy_year,Adj_clk_curr_mdy_hour,Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==2)
        {
            local_value=Adj_clk_curr_mdy_day;
            if(local_value<31)//04-06-12
                local_value++;
            else
                local_value=1;
            Adj_clk_curr_mdy_day=local_value;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "DAY?    %02d               ", Adj_clk_curr_mdy_day);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            tech_menu_disp.row   = 22*Clock_parm_count;
            tech_menu_disp.font  = font_1;
            sprintf((char*)tech_menu_disp.text,"PREVW: %02d/%02d/%02d %02d:%02d\0", Adj_clk_curr_mdy_month,Adj_clk_curr_mdy_day,Adj_clk_curr_mdy_year,Adj_clk_curr_mdy_hour,Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==3)
        {
            local_value = (uint8_t) Adj_clk_curr_mdy_year;
            if(local_value<99)
                local_value++;
            else
                local_value=12;
            Adj_clk_curr_mdy_year=local_value;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "YEAR?   %02d                ", Adj_clk_curr_mdy_year);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            tech_menu_disp.row   = 22*Clock_parm_count;
            tech_menu_disp.font  = font_1;
            sprintf((char*)tech_menu_disp.text,"PREVW: %02d/%02d/%02d %02d:%02d\0", Adj_clk_curr_mdy_month,Adj_clk_curr_mdy_day,Adj_clk_curr_mdy_year,Adj_clk_curr_mdy_hour,Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==4)
        {
            local_value=Adj_clk_curr_mdy_dow;
            if(local_value<6)
                local_value++;
            else
                local_value=0;
            Adj_clk_curr_mdy_dow=local_value;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "DOW?    %02d               ", Adj_clk_curr_mdy_dow);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==5)
        {
            local_value=Adj_clk_curr_mdy_hour;
            if(local_value<23)
                local_value++;
            else
                local_value=0;
            Adj_clk_curr_mdy_hour=local_value;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "HOUR?   %02d               ", Adj_clk_curr_mdy_hour);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            tech_menu_disp.row   = 22*Clock_parm_count;
            tech_menu_disp.font  = font_1;
            sprintf((char*)tech_menu_disp.text,"PREVW: %02d/%02d/%02d %02d:%02d\0", Adj_clk_curr_mdy_month,Adj_clk_curr_mdy_day,Adj_clk_curr_mdy_year,Adj_clk_curr_mdy_hour,Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==6)
        {
            local_value=Adj_clk_curr_mdy_min;
            if(local_value<59)
                local_value++;
            else
                local_value=0;
            Adj_clk_curr_mdy_min=local_value;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "MIN?    %02d               ", Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

            tech_menu_disp.row   = 22*Clock_parm_count;
            tech_menu_disp.font  = font_1;
            sprintf((char*)tech_menu_disp.text,"PREVW: %02d/%02d/%02d %02d:%02d\0", Adj_clk_curr_mdy_month,Adj_clk_curr_mdy_day,Adj_clk_curr_mdy_year,Adj_clk_curr_mdy_hour,Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==7)
        {
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "SAVE CHANGES? YES          ");
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            tech_menu_disp.row   = 22*Clock_parm_count;
            tech_menu_disp.font  = font_1;
            sprintf((char*)tech_menu_disp.text,"PREVW: %02d/%02d/%02d %02d:%02d\0", Adj_clk_curr_mdy_month,Adj_clk_curr_mdy_day,Adj_clk_curr_mdy_year,Adj_clk_curr_mdy_hour,Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            Save_Clk_Changes=true;
        }
    }
//    else if(maintenance_screen==true)//09-04-12 //vinay code opt
//    {
//        tech_menu_disp.column= 0;//5;
//        tech_menu_disp.font  = font_1_invert_text;
//        if(maintenance_scrn_count==1)
//        {
//            local_value=Maintenance_Log.Prob_Code;
//            if(local_value<99)
//                local_value++;
//            else
//                local_value=0;
//            Maintenance_Log.Prob_Code=local_value;
//            tech_menu_disp.row   = 32*3;
//            sprintf((char*)tech_menu_disp.text, "PROBLEM CODE: %02d       ", Maintenance_Log.Prob_Code);
//            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//        }
//        else if(maintenance_scrn_count==2)
//        {
//            local_value=Maintenance_Log.Sol_Code;
//            if(local_value<99)
//                local_value++;
//            else
//                local_value=0;
//            Maintenance_Log.Sol_Code=local_value;
//            tech_menu_disp.row   = 32*4;
//            sprintf((char*)tech_menu_disp.text, "REPAIR CODE: %02d        ", Maintenance_Log.Sol_Code);
//            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//        }
//        else if(maintenance_scrn_count==3)
//        {
//            tech_menu_disp.row   = 32*5;
//            sprintf((char*)tech_menu_disp.text, "SAVE MAINTEN.LOG?YES     ");
//            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//            save_maintenance_log = true;
//        }
//    }
    //12-06-2013:DPLIBB-531
	else if(comm_int_scrn==true)
	{
		tech_menu_disp.column = 0;
		if(comm_int_scrn_count==1)
		{
			tech_menu_disp.row    = 32*comm_int_scrn_count;
			tech_menu_disp.font   = font_1_invert_text;
			sprintf((char*)tech_menu_disp.text, "TRIGGER GENSYNC?YES      " );
			printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
			do_RTC_EVT_AFM_HB=do_RTC_EVT_AFM_HB|0x01;
		}
		else if(comm_int_scrn_count==2)
		{
			tech_menu_disp.font   = font_1_invert_text;
			tech_menu_disp.row    = 32*comm_int_scrn_count;
			sprintf((char*)tech_menu_disp.text, "TRIGGER EVENT?YES        " );
			printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
			do_RTC_EVT_AFM_HB=do_RTC_EVT_AFM_HB|0x02;
		}
		else if(comm_int_scrn_count==3)
		{
			tech_menu_disp.font   = font_1_invert_text;
			tech_menu_disp.row   = 32*(comm_int_scrn_count);
			sprintf((char*)tech_menu_disp.text, "SAVE CHANGES?YES         " );
			printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
			save_comm_interval_changes=true;
		}
	}
    //12-06-2013:DPLIBB-531
    return;
}

/**************************************************************************/
//! Process DOWN key in technician menu.
//! \param void
//! \return void
/**************************************************************************/
void TechMenu_ProcessKey_DOWN_In_TechMode()
{
    if(Static_display==true)//11-04-12
    {
        return;
    }

    if(clear_alarm_check==true)
    {
        tech_menu_disp.column= 0;//5;
        tech_menu_disp.row   = 22*7;//05-04-12
        tech_menu_disp.font  = font_1_invert_text;
        sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu5_1);
        printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        tech_menu_disp.row+=22;
        clr_alarm=true;
    }
    else if(advanced_feature==true)
    {
        if(advanced_feature_count<Advanced_Feature_Count)
            advanced_feature_count++;
        else
            advanced_feature_count=1;

        if(advanced_feature_count==1)
        {
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_8);
        }
        else if(advanced_feature_count==2)
        {
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_1); //SIT DSM Clear space Vinay
        }
//        else if(advanced_feature_count==3) //vinay code opt
//        {
//        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_1_1); //SIT DSM Clear space Vinay
//        }
//        else if(advanced_feature_count==4) //vinay code opt
//        {
//        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_1_2); //SIT DSM Clear space Vinay
//        }
        else if(advanced_feature_count==3)//(advanced_feature_count==5)
        {
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_2); //SIT DSM Credit space Vinay
        }
//        else if(advanced_feature_count==6) //vinay code opt
//        {
//        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_2_1); //SIT DSM Credit space Vinay
//        }
//        else if(advanced_feature_count==7) //vinay code opt
//        {
//        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_2_2); //SIT DSM Credit space Vinay
//        }
//        else if(advanced_feature_count==8) //vinay code opt
//        {
//        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_5);
//        }
        else if(advanced_feature_count==4)//(advanced_feature_count==9)//11-06-2013:DPLIBB-531
        {
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7);
        }
        else if(advanced_feature_count==5)//(advanced_feature_count==10)//05-08-2013:DPLIBB-554
        {
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_9);
        }
        else if(advanced_feature_count==6)//(advanced_feature_count==11)//05-08-2013:DPLIBB-554
        {
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_10);
        }
        else if(advanced_feature_count==7)//(advanced_feature_count==12)
        {
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_11);
        }
        tech_menu_disp.column= 0;//5;
        tech_menu_disp.row   = 22*7;
        tech_menu_disp.font  = font_1_invert_text;
        printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
    }
    else if(copy_files_screen==true)
    {
        if(copy_files_screen_count==1)
        {
        	if(Mech2DK_copy_option_count<Copy_Files_Options)
        		Mech2DK_copy_option_count++;
        	else
        		Mech2DK_copy_option_count=1;
        	tech_menu_disp.column= 0;//5;
        	tech_menu_disp.row   = 32*(copy_files_screen_count+1);
        	tech_menu_disp.font  = font_1_invert_text;
        	if(Mech2DK_copy_option_count==1)
        	{
        		do_not_copy_files=true;
        		copy_all_files=false;
        		copy_only_config=false;
        		copy_only_mpb_firmware=false;
        		sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_1);
        	}
        	else if(Mech2DK_copy_option_count==2)
        	{
        		copy_all_files=true;
        		do_not_copy_files=false;
        		copy_only_config=false;
        		copy_only_mpb_firmware=false;
        		sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_11);

        	}
        	else if(Mech2DK_copy_option_count==3)
        	{
        		copy_only_config=true;
        		copy_all_files=false;
        		do_not_copy_files=false;
        		copy_only_mpb_firmware=false;
        		sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_12);
        	}
        	else if(Mech2DK_copy_option_count==4)
        	{
        		copy_only_mpb_firmware=true;
        		copy_only_config=false;
        		copy_all_files=false;
        		do_not_copy_files=false;
        		sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_13);
        	}
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        if(copy_files_screen_count==2)
        {
        	if(DK2Mech_copy_option_count<Copy_Files_Options)
        		DK2Mech_copy_option_count++;
        	else
        		DK2Mech_copy_option_count=1;
        	tech_menu_disp.column= 0;//5;
        	tech_menu_disp.row   = 32*(copy_files_screen_count+1);
        	tech_menu_disp.font  = font_1_invert_text;
        	if(DK2Mech_copy_option_count==1)
        	{
        		do_not_copy_files=true;
        		copy_all_files=false;
        		copy_only_config=false;
        		copy_only_mpb_firmware=false;
        		sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_2);
        	}
        	else if(DK2Mech_copy_option_count==2)
        	{
        		copy_all_files=true;
        		do_not_copy_files=false;
        		copy_only_config=false;
        		copy_only_mpb_firmware=false;
        		sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_21);

        	}
        	else if(DK2Mech_copy_option_count==3)
        	{
        		copy_only_config=true;
        		copy_all_files=false;
        		do_not_copy_files=false;
        		copy_only_mpb_firmware=false;
        		sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_22);
        	}
        	else if(DK2Mech_copy_option_count==4)
        	{
        		copy_only_mpb_firmware=true;
        		copy_only_config=false;
        		copy_all_files=false;
        		do_not_copy_files=false;
        		sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_23);
        	}
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }

        if(copy_files_screen_count==3)
        {
            exit_filemgr=true;
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.row   = 32*(copy_files_screen_count+1);
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_41);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
    }
    else if(Adjust_clock==true)
    {
        if(Adjust_clock_key_count==1)
        {

            local_value=Adj_clk_curr_mdy_month;
            if(local_value>1)
                local_value--;
            else
                local_value=12;
            Adj_clk_curr_mdy_month=local_value;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "MONTH?  %02d               ", Adj_clk_curr_mdy_month);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            tech_menu_disp.row   = 22*Clock_parm_count;
            tech_menu_disp.font  = font_1;
            sprintf((char*)tech_menu_disp.text, "PREVW: %02d/%02d/%02d %02d:%02d\0", Adj_clk_curr_mdy_month,Adj_clk_curr_mdy_day,Adj_clk_curr_mdy_year,Adj_clk_curr_mdy_hour,Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==2)
        {
            local_value=Adj_clk_curr_mdy_day;
            if(local_value>1)
                local_value--;
            else
                local_value=31;//04-06-12
            Adj_clk_curr_mdy_day=local_value;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "DAY?    %02d               ", Adj_clk_curr_mdy_day);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            tech_menu_disp.row   = 22*Clock_parm_count;
            tech_menu_disp.font  = font_1;
            sprintf((char*)tech_menu_disp.text, "PREVW: %02d/%02d/%02d %02d:%02d\0", Adj_clk_curr_mdy_month,Adj_clk_curr_mdy_day,Adj_clk_curr_mdy_year,Adj_clk_curr_mdy_hour,Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==3)
        {
            local_value = (uint8_t) Adj_clk_curr_mdy_year;
            if(local_value>12)
                local_value--;
            else
                local_value=99;
            Adj_clk_curr_mdy_year=local_value;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "YEAR?   %02d		  	  ", Adj_clk_curr_mdy_year);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

            tech_menu_disp.row   = 22*Clock_parm_count;
            tech_menu_disp.font  = font_1;
            sprintf((char*)tech_menu_disp.text, "PREVW: %02d/%02d/%02d %02d:%02d\0", Adj_clk_curr_mdy_month,Adj_clk_curr_mdy_day,Adj_clk_curr_mdy_year,Adj_clk_curr_mdy_hour,Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==4)
        {
            local_value=Adj_clk_curr_mdy_dow;
            if(local_value>0)
                local_value--;
            else
                local_value=6;
            Adj_clk_curr_mdy_dow=local_value;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "DOW?    %02d		  	  ", Adj_clk_curr_mdy_dow);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==5)
        {
            local_value=Adj_clk_curr_mdy_hour;
            if(local_value>0)
                local_value--;
            else
                local_value=23;
            Adj_clk_curr_mdy_hour=local_value;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "HOUR?   %02d		  	  ", Adj_clk_curr_mdy_hour);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            tech_menu_disp.row   = 22*Clock_parm_count;
            tech_menu_disp.font  = font_1;
            sprintf((char*)tech_menu_disp.text, "PREVW: %02d/%02d/%02d %02d:%02d\0", Adj_clk_curr_mdy_month,Adj_clk_curr_mdy_day,Adj_clk_curr_mdy_year,Adj_clk_curr_mdy_hour,Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==6)
        {
            local_value=Adj_clk_curr_mdy_min;
            if(local_value>0)
                local_value--;
            else
                local_value=59;
            Adj_clk_curr_mdy_min=local_value;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "MIN?    %02d		  	  ", Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

            tech_menu_disp.row   = 22*Clock_parm_count;
            tech_menu_disp.font  = font_1;
            sprintf((char*)tech_menu_disp.text, "PREVW: %02d/%02d/%02d %02d:%02d\0", Adj_clk_curr_mdy_month,Adj_clk_curr_mdy_day,Adj_clk_curr_mdy_year,Adj_clk_curr_mdy_hour,Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==7)
        {
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "SAVE CHANGES? NO	  	  ");
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

            tech_menu_disp.row   = 22*Clock_parm_count;
            tech_menu_disp.font  = font_1;
            sprintf((char*)tech_menu_disp.text, "PREVW: %02d/%02d/%02d %02d:%02d\0", Adj_clk_curr_mdy_month,Adj_clk_curr_mdy_day,Adj_clk_curr_mdy_year,Adj_clk_curr_mdy_hour,Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            Save_Clk_Changes=false;
        }
    }
//    else if(maintenance_screen==true)//09-04-12 //vinay code opt
//    {
//        tech_menu_disp.column= 0;//5;
//        tech_menu_disp.font  = font_1_invert_text;
//        if(maintenance_scrn_count==1)
//        {
//            local_value=Maintenance_Log.Prob_Code;
//            if(local_value>0)
//                local_value--;
//            else
//                local_value=99;
//            Maintenance_Log.Prob_Code=local_value;
//            tech_menu_disp.row   = 32*3;
//            sprintf((char*)tech_menu_disp.text, "PROBLEM CODE: %02d		  ", Maintenance_Log.Prob_Code);
//            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//        }
//        else if(maintenance_scrn_count==2)
//        {
//            local_value=Maintenance_Log.Sol_Code;
//            if(local_value>0)
//                local_value--;
//            else
//                local_value=99;
//            Maintenance_Log.Sol_Code=local_value;
//            tech_menu_disp.row   = 32*4;
//            sprintf((char*)tech_menu_disp.text, "REPAIR CODE: %02d		  ", Maintenance_Log.Sol_Code);
//            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//        }
//        else if(maintenance_scrn_count==3)
//        {
//            tech_menu_disp.row   = 32*5;
//            sprintf((char*)tech_menu_disp.text, "SAVE MAINTEN.LOG?NO	  ");
//            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//            save_maintenance_log=false;
//        }
//    }
//12-06-2013:DPLIBB-531
    else if(comm_int_scrn==true)
    {
    	tech_menu_disp.column = 0;
    	if(comm_int_scrn_count==1)
    	{
    		tech_menu_disp.row    = 32*comm_int_scrn_count;
    		tech_menu_disp.font   = font_1_invert_text;
    		sprintf((char*)tech_menu_disp.text, "TRIGGER GEN SYNC?NO      " );
    		printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
    		do_RTC_EVT_AFM_HB=do_RTC_EVT_AFM_HB & ~(0x01);
    	}
    	else if(comm_int_scrn_count==2)
    	{
    		tech_menu_disp.font   = font_1_invert_text;
    		tech_menu_disp.row    = 32*comm_int_scrn_count;
    		sprintf((char*)tech_menu_disp.text, "TRIGGER EVENT?NO         " );
    		printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
    		do_RTC_EVT_AFM_HB=do_RTC_EVT_AFM_HB & ~(0x02);
    	}
    	else if(comm_int_scrn_count==3)
    	{
    		tech_menu_disp.font   = font_1_invert_text;
    		tech_menu_disp.row   = 32*(comm_int_scrn_count);
    		sprintf((char*)tech_menu_disp.text, "SAVE CHANGES?NO          " );
    		printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
    		save_comm_interval_changes=false;
    	}
    }
//12-06-2013:DPLIBB-531
    return;
}

/**************************************************************************/
//! Process OK key in technician menu.
//! \param void
//! \return void
/**************************************************************************/
void TechMenu_ProcessKey_OK_In_TechMode()
{
	uint8_t	i = 0;

    if(Static_display==true)//11-04-12
    {
        return;
    }

    if(main_menu==true)
    {
        if(Display_position==1)
        {
            Technician_Menu_Information();
        }
        else if(Display_position==2)
        {
            tech_meter_info_first_page();
        }
        else if(Display_position==3)
        {
            tech_communication_information();
        }
        else if (Display_position==4)
        {
            tech_software_information();
        }
        else if(Display_position==5)
        {
        	tech_adjust_clock();
        }
        else if(Display_position==6)
        {
        	initialise_tech_menu_param();//27-11-12
        	Debug_TextOut( 0, "LNG Exited Tech Mode 4" );
        	cardread = 0;
        	Tech_menu = false;
        	Tech_key_seq = FALSE;
        	Display_position=0; //to send the cursor to line one (tech info) always when entered to techmenu //vinay LNGSIT
        	Coin_Calibration_Mode(AUTO_CALIBRATION_MODE);
        }
//        else if(Display_position==7)//vinay
//        {
//        	//main_exit();
//        }
    }
    else if((clear_alarm_check==true)&&(clr_alarm==true))
    {
        main_menu=false;
        tech_clear_alarm();
        clr_alarm=false;
        clear_alarm_check=false;
    }
    else if((clear_alarm_check==true)&&(clr_alarm==false))
    {
        tech_menu_disp.column= 0;//5;
        tech_menu_disp.row   = 22*7;//16-05-12
        tech_menu_disp.font  = font_1_invert_text;
        sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu5);
        printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        tech_menu_disp.row+=22;
        clr_alarm=false;
    }
    else if(advanced_feature==true)
    {
        main_menu=false;

        if(advanced_feature_count==1)//11-04-12////05-08-2013:DPLIBB-554
        {
        	initialise_tech_menu_param();//27-11-12
        	Debug_TextOut( 0, "LNG Exited Tech Mode 3" );
        	cardread = 0;
        	Screen_Clear = true;
        	Tech_menu = false;
        	Display_position=0; //to send the cursor to line one (tech info) always when entered to techmenu //vinay LNGSIT
        	if(GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN) == 0)
        	{
        		uint8_t z;
        		if(Tech_key_seq == FALSE)
        		{
        			diag_text_Screen( "PLEASE REMOVE CARD      ", TRUE, FALSE );
        		}
        		Tech_key_seq = FALSE;
        		for(z=0;z<50;z++)
        		{
        			DelayMs(100);
        			if(GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN) != 0)
        				break;
        		}
        	}
        	Tech_key_seq = FALSE;
        	DSM_Screen1();//Idle_Screen();
        }
    	else if(advanced_feature_count==2)
    	{
            	Debug_TextOut( 0, "Clear Both Spaces" );
            	push_event_to_cache( UDP_EVTTYP_CLEAR_SPACE );
//            	if((parking_time_left_on_meter[Current_Space_Id] > 0) || (negetive_parking_time_left_on_meter[Current_Space_Id] > 0))	//LNGSIT-811
//            	{
//            		push_event_to_cache(UDP_EVTTYP_PARKING_TIME_EXPIRED);
//            	}

            	for(i=0; i<MSM_MAX_PARKING_BAYS; i++)
            	{
            		parking_time_left_on_meter[i] = 0;
            		time_retain[i] = 0;
            		parking_time_left_on_meter_prev[i] = 0;
            		negetive_parking_time_left_on_meter[i] = 0;
        			//ANTI_FEED_in_effect[0] = false;
        			//ANTI_FEED_in_effect[1] = false;

        			grace_time_trigger[i] = false;
					in_prepay_parking[i]  = FALSE;
					time_retain[i] = 0;
					Debug_Output1(0,"Time Expired for Space: %d", (i));
					Running_Space_ID[i] = FALSE; //when spaces expires within few secs apart then expired exents for some were missing so placed FALSE here to avoid missing //vinay
					sensor_tnx_flag = TRUE;
					Current_bayStatus = i;
					push_event_to_cache(UDP_EVTTYP_PARKING_TIME_EXPIRED);
					sensor_tnx_flag = FALSE;
					Flash_RTC_ParkingClock_Storage(true, false);
					ANTI_FEED_in_effect[i] = false;

            	}

            	Refund_earned_mins = 0;
            	Refund_timestamp = 0;
            	tech_software_information();
        }
//    	else if(advanced_feature_count==3) //vinay code opt
//    	{
//    		Debug_TextOut( 0, "Clear Space Left" );
//    		push_event_to_cache( UDP_EVTTYP_CLEAR_SPACE ); //868
//
////    		if((parking_time_left_on_meter[Current_Space_Id] > 0) || (negetive_parking_time_left_on_meter[Current_Space_Id] > 0))	//LNGSIT-811
////    		{
////    			push_event_to_cache(UDP_EVTTYP_PARKING_TIME_EXPIRED); //801
////    		}
////
////    		//for(i=0; i<MSM_MAX_PARKING_BAYS; i++)
////    		{
////    			ANTI_FEED_in_effect[0] = false;
////    			parking_time_left_on_meter[0] = 0;
////    			time_retain[0] = 0;
////    			parking_time_left_on_meter_prev[0] = 0;
////    			negetive_parking_time_left_on_meter[0] = 0;
////    		}
//    		//expiry events are going for only space left LNGDSM4K-112 & 111//vinay
//        	{
//        		parking_time_left_on_meter[0] = 0;
//        		time_retain[0] = 0;
//        		parking_time_left_on_meter_prev[0] = 0;
//        		negetive_parking_time_left_on_meter[0] = 0;
//     			grace_time_trigger[0] = false;
//				in_prepay_parking[0]  = FALSE;
//				time_retain[0] = 0;
//				Debug_TextOut( 0,"Time Expired for Space: 0");
//				Running_Space_ID[0] = FALSE; //when spaces expires within few secs apart then expired exents for some were missing so placed FALSE here to avoid missing //vinay
//				sensor_tnx_flag = TRUE;
//				Current_bayStatus = 0;
//				push_event_to_cache(UDP_EVTTYP_PARKING_TIME_EXPIRED);
//				sensor_tnx_flag = FALSE;
//				Flash_RTC_ParkingClock_Storage(true, false);
//				ANTI_FEED_in_effect[0] = false;
//
//        	}
//
//    		Refund_earned_mins = 0;
//    		Refund_timestamp = 0;
//    		tech_software_information();
//        }
//    	else if(advanced_feature_count==4) //vinay code opt
//    	{
//    		//LNGSIT-859
//    		Debug_TextOut( 0, "Clear Space Right" );
//    		push_event_to_cache( UDP_EVTTYP_CLEAR_SPACE ); //868
//    		//Tasks_Priority_Register |= SEVERE_EVENTS_TASK;
//
////    		if((parking_time_left_on_meter[Current_Space_Id] > 0) || (negetive_parking_time_left_on_meter[Current_Space_Id] > 0))	//LNGSIT-811
////    		{
////    			push_event_to_cache(UDP_EVTTYP_PARKING_TIME_EXPIRED); //801
////    		}
////
////    		//for(i=0; i<MSM_MAX_PARKING_BAYS; i++)
////    		{
////    			ANTI_FEED_in_effect[1] = false;
////    			parking_time_left_on_meter[1] = 0;
////    			time_retain[1] = 0;
////    			parking_time_left_on_meter_prev[1] = 0;
////    			negetive_parking_time_left_on_meter[1] = 0;
////    		}
//    		//expiry events are going for only space left LNGDSM4K-112 & 111//vinay
//        	{
//        		parking_time_left_on_meter[1] = 0;
//        		time_retain[1] = 0;
//        		parking_time_left_on_meter_prev[1] = 0;
//        		negetive_parking_time_left_on_meter[1] = 0;
//     			grace_time_trigger[1] = false;
//				in_prepay_parking[1]  = FALSE;
//				time_retain[1] = 0;
//				Debug_TextOut( 0,"Time Expired for Space: 1");
//				Running_Space_ID[1] = FALSE; //when spaces expires within few secs apart then expired exents for some were missing so placed FALSE here to avoid missing //vinay
//				sensor_tnx_flag = TRUE;
//				Current_bayStatus = 1;
//				push_event_to_cache(UDP_EVTTYP_PARKING_TIME_EXPIRED);
//				sensor_tnx_flag = FALSE;
//				Flash_RTC_ParkingClock_Storage(true, false);
//				ANTI_FEED_in_effect[1] = false;
//
//        	}
//
//    		Refund_earned_mins = 0;
//    		Refund_timestamp = 0;
//    		//Update_Parking_Clock_Screen();
//    		//DelayMs(500);	//LNGSIT-859
//    		tech_software_information();
//        }
    	else if(advanced_feature_count==3)//(advanced_feature_count==5)
    	{
    	Debug_TextOut( 0, "Credit Both Spaces" );
    		if(current_rate_index == NO_PARKING)
    		{
    			Noparking_Screen();
    			Debug_TextOut( 0, "PA-C:no_parking" );
    			return;
    		}
    		else if(current_rate_index == FREE_PARKING)
    		{
    			Freeparking_Screen();
    			Debug_TextOut( 0, "PA-C:free_parking" );
    			return;
    		}
    		else if(ANTI_FEED_in_effect[0] == true)	//Anti Feed feature enabled in config	//LNGSIT-932
    		{
    			AntiFeed_Screen();
    			Debug_TextOut(0,"Anti Feed, No parking time earned for space 0");
    			return;
    		}
    		else if(ANTI_FEED_in_effect[1] == true)	//Anti Feed feature enabled in config	//LNGSIT-932
    		{
    			AntiFeed_Screen();
    			Debug_TextOut(0,"Anti Feed, No parking time earned for space 1");
    			return;
    		}
    		//if((current_rate_index == FREE_PARKING)||(current_rate_index == NO_PARKING))//Do not accept payment for these modes
    		Debug_TextOut( 0, "Credit Time for Both spaces" );
    		for(i=0; i<MSM_MAX_PARKING_BAYS; i++)
    		{
    			get_earned_seconds(100, false, true);//dummy call to refresh the max time allowed variable
    			//if(((minutes_left_in_current_schedule*60) > max_time_in_current_rate) || (glSystem_cfg.bleed_into_next_schedule_enable == TRUE))
    			{
    				//parking_time_left_on_meter[i] = max_time_in_current_rate;
    				parking_time_left_on_meter[i] = (uint32_t)(minutes_left_in_current_schedule*60);// to provide only remaining time in that schedule //vinay
    			}
    			//else
    			//{
    			//	parking_time_left_on_meter[Current_Space_Id] = (uint32_t)(minutes_left_in_current_schedule*60);
    			//}
    			Update_Parking_Clock_Screen();
				sensor_tnx_flag = TRUE;
				Current_bayStatus = i;
    			push_event_to_cache( UDP_EVTTYP_MAX_TIME_CREDITED );//09-04-12

    		}
    		tech_software_information();
        }
//    	else if(advanced_feature_count==6) //vinay code opt
//    	{
//    	Debug_TextOut( 0, "Credit Space Left" );
//    	Current_Space_Id = 0;
//    		if(current_rate_index == NO_PARKING)
//    		{
//    			Noparking_Screen();
//    			Debug_TextOut( 0, "PA-C:no_parking" );
//    			return;
//    		}
//    		else if(current_rate_index == FREE_PARKING)
//    		{
//    			Freeparking_Screen();
//    			Debug_TextOut( 0, "PA-C:free_parking" );
//    			return;
//    		}
//    		else if(ANTI_FEED_in_effect[0] == true)	//Anti Feed feature enabled in config	//LNGSIT-932
//    		{
//    			AntiFeed_Screen();
//    			Debug_TextOut(0,"Anti Feed, No parking time earned for space 0");
//    			return;
//    		}
//    		//if((current_rate_index == FREE_PARKING)||(current_rate_index == NO_PARKING))//Do not accept payment for these modes
//    		{
//    			Debug_TextOut( 0, "Credit Time for Left Space" );
//    			get_earned_seconds(100, false, true);//dummy call to refresh the max time allowed variable
//    			//if(((minutes_left_in_current_schedule*60) > max_time_in_current_rate) || (glSystem_cfg.bleed_into_next_schedule_enable == TRUE))
//    			{
//    				//parking_time_left_on_meter[0] = max_time_in_current_rate;
//    				parking_time_left_on_meter[0] = (uint32_t)(minutes_left_in_current_schedule*60);// to provide only remaining time in that schedule //vinay
//    			}
//    			//else
//    			//{
//    			//	parking_time_left_on_meter[0] = (uint32_t)(minutes_left_in_current_schedule*60);
//    			//}
//    			Update_Parking_Clock_Screen();
//				sensor_tnx_flag = TRUE;
//				Current_bayStatus = 0;
//    			push_event_to_cache( UDP_EVTTYP_MAX_TIME_CREDITED );//09-04-12  //867
//
//    		}
//    		tech_software_information();
//       }
//
//       	else if(advanced_feature_count==7) //vinay code opt
//       	{
//       	Debug_TextOut( 0, "Credit Space Right" );
//       	Current_Space_Id = 0;
//       		if(current_rate_index == NO_PARKING)
//       		{
//       			Noparking_Screen();
//       			Debug_TextOut( 0, "PA-C:no_parking" );
//       			return;
//       		}
//       		else if(current_rate_index == FREE_PARKING)
//       		{
//       			Freeparking_Screen();
//       			Debug_TextOut( 0, "PA-C:free_parking" );
//       			return;
//       		}
//       		else if(ANTI_FEED_in_effect[1] == true)	//Anti Feed feature enabled in config	//LNGSIT-932
//       		{
//       			AntiFeed_Screen();
//       			Debug_TextOut(0,"Anti Feed, No parking time earned for space 1");
//       			return;
//       		}
//       		//if((current_rate_index == FREE_PARKING)||(current_rate_index == NO_PARKING))//Do not accept payment for these modes
//       		{
//       			Debug_TextOut( 0, "Credit Time for Right Space" );
//       			get_earned_seconds(100, false, true);//dummy call to refresh the max time allowed variable
//       			//if(((minutes_left_in_current_schedule*60) > max_time_in_current_rate) || (glSystem_cfg.bleed_into_next_schedule_enable == TRUE))
//       			{
//       				//parking_time_left_on_meter[1] = max_time_in_current_rate;
//       				parking_time_left_on_meter[1] = (uint32_t)(minutes_left_in_current_schedule*60); // to provide only remaining time in that schedule //vinay
//       			}
//       			//else
//       			//{
//       			//	parking_time_left_on_meter[1] = (uint32_t)(minutes_left_in_current_schedule*60);
//       			//}
//       			Update_Parking_Clock_Screen();
//				sensor_tnx_flag = TRUE;
//				Current_bayStatus = 1;
//       			push_event_to_cache( UDP_EVTTYP_MAX_TIME_CREDITED );//09-04-12  //867
//
//       		}
//       		tech_software_information();
//       }
//        else if(advanced_feature_count==8)
//        {
//            tech_display_maintenance_menu();//09-04-12
//        }
        else if(advanced_feature_count==4)//(advanced_feature_count==9)
        {
        	Clear_Screen();
            //FILE MANAGER
//#ifdef ALLOW_DATAKEY_COPY
        	if(GPIO_getInputPinValue(KEYDETECT_PORT, KEYDETECT_PIN) == 0)
        	{
        		copy_files_screen=false;
        		tech_mechanism_files();//24-08-12
        		file_manager_screen=1;//24-08-12
//#else
        	}
        	else
        	{
				advanced_feature  = false;
				copy_files_screen=false;

				tech_menu_disp.row   = 32;
				tech_menu_disp.column= 0;//5;
				tech_menu_disp.font  = font_1_invert_text;
				sprintf((char*)tech_menu_disp.text, "DATAKEY NOT CONNECTED    ");
				printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

				tech_menu_disp.row   += 64;
				tech_menu_disp.column= 0;//5;
				tech_menu_disp.font  = font_1;
				sprintf((char*)tech_menu_disp.text, "DATAKEY FILES COPY ");
				printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

				tech_menu_disp.row   += 32;
				tech_menu_disp.column= 0;//5;
				tech_menu_disp.font  = font_1;
				sprintf((char*)tech_menu_disp.text, "NOT ALLOWED ");
				printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
			}
//#endif
        }
        else if(advanced_feature_count==5)//(advanced_feature_count==10) //todo://11-06-2013:DPLIBB-531
        {
        	tech_comm_intervals();
        }

//05-08-2013:DPLIBB-554
        else if(advanced_feature_count==6)//(advanced_feature_count==11)
        {
        	Meter_In_Service();
        	initialise_tech_menu_param();
        	Debug_TextOut( 0, "LNG Exited Tech Mode 1" );
        	cardread = 0;
        	Display_position=0; //to send the cursor to line one (tech info) always when entered to techmenu //vinay LNGSIT
         	Screen_Clear = true;
        	Tech_menu = false;

        	if(GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN) == 0)
        	{
        		uint8_t z;
        		if(Tech_key_seq == FALSE)
        		{
        			diag_text_Screen( "  PLEASE REMOVE CARD     ", TRUE, FALSE );
        		}
        		Tech_key_seq = FALSE;
        		for(z=0;z<50;z++)
        		{
        			DelayMs(100);
        			if(GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN) != 0)
        				break;
        		}
        	}
        	Tech_key_seq = FALSE;
        	DSM_Screen1();//Idle_Screen();
        }
        else if(advanced_feature_count==7)//(advanced_feature_count==12)
        {
        	//init_LEDs_PWM();
        	Meter_Out_of_service();
        	initialise_tech_menu_param();//27-11-12
        	Debug_TextOut( 0, "LNG exited tech mode 2" );
        	cardread = 0;
        	Display_position=0; //to send the cursor to line one (tech info) always when entered to techmenu //vinay LNGSIT
        	Screen_Clear = true;
        	Tech_menu = false;
        	if(GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN) == 0)
        	{
        		uint8_t z;
        		if(Tech_key_seq == FALSE)
        		{
        			diag_text_Screen( "  PLEASE REMOVE CARD     ", TRUE, FALSE );
        		}
        		Tech_key_seq = FALSE;
        		for(z=0;z<50;z++)
        		{
        			DelayMs(100);
        			if(GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN) != 0)
        				break;
        		}
        	}
        	Tech_key_seq = FALSE;
        	DSM_Screen1();//Idle_Screen();
        }
//05-08-2013:DPLIBB-554
        advanced_feature_count=1;
    }
    else if(copy_files_screen==true)
    {
    	if(copy_files_screen_count==1)
    	{
    		//copy from mech to data key
    		if(fd_start==false)
    		{
    			if(do_not_copy_files==true)
    			{
    				;
    			}
    			else if(copy_all_files==true)
    			{
    				Copy_all_flash_files_to_datakey();
    				copy_all_files=false;
    			}
    			else if(copy_only_config==true)
    			{
    				Copy_only_MPB_config_2DK();
    				copy_only_config=false;
    			}
    			else if(copy_only_mpb_firmware==true)
    			{
    				Copy_only_MPB_firmware_2DK();
    				copy_only_mpb_firmware=false;
    			}
    		}
    		else
    		{
    			//Debug_TextOut( 0, "Try Later" );
    			DelayMs(1000);
    			tech_software_information();
    			reset_timeout_count=false;
    			copy_files_screen=false;
    			Static_display=false;
    		}
    	}

    	if(copy_files_screen_count==2)
    	{
    		//copy from data key to flash
    		if(fd_start==false)
    		{
    			if(do_not_copy_files == true)
    			{
    				;
    			}
    			else if(copy_all_files == true)
    			{
        			Copy_all_files_to_mechanism();
        			copy_all_files=false;
    			}
    			else if(copy_only_config == true)
    			{
    				Copy_only_MPB_config_2mech();
    				copy_only_config=false;
    			}
    			else if(copy_only_mpb_firmware == true)
    			{
        			Copy_only_MPB_firmware_2mech();
        			copy_only_mpb_firmware=false;
    			}
    		}
    		else
    		{
    			//Debug_TextOut( 0, "Try Later" );
    			DelayMs(1000);
    			tech_software_information();
    			reset_timeout_count=false;
    			copy_files_screen=false;
    			Static_display=false;
    		}
    	}
    	if((copy_files_screen_count==3)&&(exit_filemgr==true))
    	{
    		exit_filemgr=false;
    		copy_files_screen=false;
    		tech_software_information();
    		copy_files_screen_count=0;
    	}
    }
    else if(Adjust_clock==true)
    {
    	if(Adjust_clock_key_count==7)
    	{
    		if(Save_Clk_Changes==true)
    		{
    			push_event_to_cache( UDP_EVTTYP_CLOCK_MANUALLY_ADJUSTED );//12-04-12
    			tech_set_RTC_in_TechMode();
    			//Get the rate refreshed immediately here
    			Update_RTC_on_Display();
    			read_and_fill_current_schedule(&rtc_c_calender);

    			//18-05-12:DLST
    			if(glSystem_cfg.DLST_ON_OFF==1)
    			{
    				DLST_FLAG_CHECK=1;
    				Flash_Batch_Flag_Read();
    				DLST_FLAG_CHECK=0;
    			}
    			//18-05-12:DLST

    		}
    		Save_Clk_Changes=false;
    		Adjust_clock_key_count=1;
    		Adjust_clock=false;
    		//tech_software_information();
    		TechMenu_Display_Main_Menu();
    	}
    }
//	else if((maintenance_screen==true)&&(maintenance_scrn_count==3))//09-04-12 //vinay code opt
//	{
//		if(save_maintenance_log==true)
//		{
//			push_event_to_cache( UDP_EVTTYP_MAINTENANCE_LOG );
//			//Tasks_Priority_Register |= SEVERE_EVENTS_TASK;
//		}
//		Maintenance_Log.Prob_Code=0;
//		Maintenance_Log.Sol_Code=0;
//		maintenance_screen=false;
//		save_maintenance_log=false;
//		maintenance_scrn_count=1;
//		tech_software_information();
//	}
	else if(Display_meter_info_2ndpage==true)
	{
		tech_meter_info_second_page();
	}

    if((comm_int_scrn==true)&&(comm_int_scrn_count==3))
    {
    	if((save_comm_interval_changes==true)&&(do_RTC_EVT_AFM_HB>0))
    	{
    		if((do_RTC_EVT_AFM_HB&0x01)==0x01)
    		{
    			Tasks_Priority_Register |= GENERAL_SYNC_TASK;
    		}
    		if((do_RTC_EVT_AFM_HB&0x02)==0x02)
    		{
    			Tasks_Priority_Register |= EVENTS_UPDATE_TASK;
    		}
    	}
    	comm_int_scrn=false;
    	save_comm_interval_changes=false;
    	do_RTC_EVT_AFM_HB=0;
    	comm_int_scrn_count=1;
    	tech_software_information();
    }
    	Display_position=0;//1;  //to send the cursor to line one (tech info) always when entered to techmenu //vinay LNGSIT

    	return;
}

/**************************************************************************/
//! Process CANCEL key in technician menu.
//! \param void
//! \return void
/**************************************************************************/
void TechMenu_ProcessKey_CANCEL_In_TechMode()
{
    if(Static_display==true)//11-04-12
    {
        return;
    }

    if(main_menu==true)
    {
        if((Display_position<main_menu_count)&&(Tech_modem_onoff_mode==false))
        {
            Display_position++;
        }
/*        else if((Display_position<(main_menu_count+1))&&(Tech_modem_onoff_mode==true))
        {
        	Display_position++;
        }*/
        else
        {
            Display_position=1;
        }

        if(Display_position==1)
        {
        	tech_menu_disp.row   = (32*(Display_position+5));//+4
        	tech_menu_disp.column= 0;//5;
        	tech_menu_disp.font  = font_1;
       		sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu10);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

        	tech_menu_disp.font  = font_1_invert_text;
        	tech_menu_disp.row   = (32*(Display_position-1));
        	tech_menu_disp.column= 0;
        	sprintf((char*)tech_menu_disp.text, "%s", " 1. TECH MENU INFO	      ");
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Display_position==2)
        {
            tech_menu_disp.font  = font_1;
            tech_menu_disp.row   = (32*(Display_position-2));
            tech_menu_disp.column= 0;
        	sprintf((char*)tech_menu_disp.text, "%s", " 1. TECH MENU INFO	      ");
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

            tech_menu_disp.row   = (32*(Display_position-1));
            tech_menu_disp.column= 0;//5;
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu2);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Display_position==3)
        {
            tech_menu_disp.font  = font_1;
            tech_menu_disp.row   = (32*(Display_position-2));
            tech_menu_disp.column= 0;//5;
            sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu2);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

            tech_menu_disp.row   = (32*(Display_position-1));
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu3);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Display_position==4)
        {
            tech_menu_disp.font  = font_1;
            tech_menu_disp.row   = (32*(Display_position-2));
            tech_menu_disp.column= 0;//5;
            sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu3);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

            tech_menu_disp.row   = (32*(Display_position-1));
            tech_menu_disp.font  = font_1_invert_text;
            sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu4);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Display_position==5)
        {
        	tech_menu_disp.font  = font_1;
        	tech_menu_disp.row   = (32*(Display_position-2));
        	tech_menu_disp.column= 0;//5;
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu4);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

        	tech_menu_disp.row   = (32*(Display_position-1));
        	tech_menu_disp.font  = font_1_invert_text;
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu8);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Display_position==6)
        {
        	tech_menu_disp.font  = font_1;
        	tech_menu_disp.row   = (32*(Display_position-2));
        	tech_menu_disp.column= 0;//5;
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu8);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

        	tech_menu_disp.row   = (32*(Display_position-1));
        	tech_menu_disp.font  = font_1_invert_text;
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu9);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Display_position==7) //vinay
        {
        	tech_menu_disp.font  = font_1;
        	tech_menu_disp.row   = (32*(Display_position-2));
        	tech_menu_disp.column= 0;//5;
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu9);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

        	tech_menu_disp.row   = (32*(Display_position-1));
        	tech_menu_disp.font  = font_1_invert_text;
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu10);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
		}
    }
    else if((advanced_feature==true)||(clear_alarm_check==true)||(Tech_menu_info==true)||(coms_info==true)||(Display_meter_info_2ndpage==true))//26-04-12
    {
        Display_meter_info_2ndpage=false;
        TechMenu_Display_Main_Menu();
        advanced_feature=false;
        clear_alarm_check=false;
        coms_info=false;
        Tech_menu_info=false;
    }
    else if((file_manager_screen==1)&&(copy_files_screen==false))
    {
    	file_manager_screen=0;
    	tech_mechanism_and_datakey_files();
    }
    else if(copy_files_screen==true)
    {
        if(copy_files_screen_count<3)	//LNGSIT-726: changed from 4 to 3
        {
            copy_files_screen_count++;
        }
        else
        {
        	copy_files_screen_count=0;
        	copy_files_screen=false;
        	if((copy_files_screen_count!=2) && (file_manager_screen!=1) && (copy_files_screen!=false)) //LNGSIT-1239
        	{
        		tech_mechanism_files();//24-08-12
        		file_manager_screen=1;//24-08-12
        	}
        }

        if(copy_files_screen_count==2)
        {
        	tech_menu_disp.row   = 32*copy_files_screen_count;
        	tech_menu_disp.font  = font_1;
        	tech_menu_disp.column= 0;//5;
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_1);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

        	tech_menu_disp.row   = 32*(copy_files_screen_count+1);
        	tech_menu_disp.font  = font_1_invert_text;
        	sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_2);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(copy_files_screen_count==3)
        {
            tech_menu_disp.row   = 32*copy_files_screen_count;
            tech_menu_disp.font  = font_1;
            sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_2);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

            tech_menu_disp.row   = 32*(copy_files_screen_count+1);
			tech_menu_disp.font  = font_1_invert_text;
			sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_4);
			printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        //LNGSIT-726: changed from 4 to 3
/*        else if(copy_files_screen_count==4)
        {
            tech_menu_disp.row   = 32*copy_files_screen_count;
            tech_menu_disp.font  = font_1;
            sprintf((char*)tech_menu_disp.text, "%s", Main_Tech_menu6_7_4);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }*/
    	do_not_copy_files=false;
    	copy_all_files=false;
    	copy_only_config=false;
    	copy_only_mpb_firmware=false;
    	DK2Mech_copy_option_count=1;
    	Mech2DK_copy_option_count=1;
    }
    else if(Adjust_clock==true)
    {
        if(Adjust_clock_key_count<Clock_parm_count)
        {
            Adjust_clock_key_count++;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-2);
        }
        else
        {
            Adjust_clock_key_count=1;
            tech_menu_disp.row   = 22*(Clock_parm_count-1);
        }
        tech_menu_disp.column= 0;//5;
        tech_menu_disp.font  = font_1;
        if(Adjust_clock_key_count==1)
        {
        	Save_Clk_Changes=false;
        	sprintf((char*)tech_menu_disp.text, "SAVE CHANGES? NO		     ");
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

            tech_menu_disp.font  = font_1_invert_text;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            sprintf((char*)tech_menu_disp.text, "MONTH?  %02d		  	  ", Adj_clk_curr_mdy_month);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==2)
        {
        	sprintf((char*)tech_menu_disp.text, "MONTH?  %02d			  ", Adj_clk_curr_mdy_month);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            tech_menu_disp.font  = font_1_invert_text;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            sprintf((char*)tech_menu_disp.text, "DAY?    %02d		  	  ", Adj_clk_curr_mdy_day);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==3)
        {
        	sprintf((char*)tech_menu_disp.text, "DAY?    %02d		  	  ", Adj_clk_curr_mdy_day);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            tech_menu_disp.font  = font_1_invert_text;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            sprintf((char*)tech_menu_disp.text, "YEAR?   %02d		  	  ", Adj_clk_curr_mdy_year);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==4)
        {
        	sprintf((char*)tech_menu_disp.text, "YEAR?   %02d			  ", Adj_clk_curr_mdy_year);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            tech_menu_disp.font  = font_1_invert_text;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            sprintf((char*)tech_menu_disp.text, "DOW?    %02d			  ", Adj_clk_curr_mdy_dow);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==5)
        {
        	sprintf((char*)tech_menu_disp.text, "DOW?    %02d			  ", Adj_clk_curr_mdy_dow);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            tech_menu_disp.font  = font_1_invert_text;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            sprintf((char*)tech_menu_disp.text, "HOUR?   %02d			  ", Adj_clk_curr_mdy_hour);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==6)
        {
        	sprintf((char*)tech_menu_disp.text, "HOUR?   %02d			  ", Adj_clk_curr_mdy_hour);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            tech_menu_disp.font  = font_1_invert_text;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            sprintf((char*)tech_menu_disp.text, "MIN?    %02d			   ", Adj_clk_curr_mdy_min);
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
        }
        else if(Adjust_clock_key_count==7)
        {
        	sprintf((char*)tech_menu_disp.text, "MIN?    %02d			   ", Adj_clk_curr_mdy_min);
        	printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            tech_menu_disp.font  = font_1_invert_text;
            tech_menu_disp.row   = 22*(Adjust_clock_key_count-1);
            sprintf((char*)tech_menu_disp.text, "SAVE CHANGES? NO		   ");
            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
            Save_Clk_Changes=false;
        }
    }
//    else if((maintenance_screen==true))//09-04-12 //vinay code opt
//    {
//        if(maintenance_scrn_count<Maintenance_scrn_Intfeat_count)
//            maintenance_scrn_count++;
//        else
//            maintenance_scrn_count=1;
//        tech_menu_disp.font  = font_1;
//        tech_menu_disp.column= 0;//5;
//        if(maintenance_scrn_count==1)
//        {
//            tech_menu_disp.row   = 32*(Maintenance_scrn_Intfeat_count+(maintenance_scrn_count+1));
//            sprintf((char*)tech_menu_disp.text, "SAVE MAINTEN.LOG?NO	   ");
//            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//
//            tech_menu_disp.font  = font_1_invert_text;
//            tech_menu_disp.row   = 32*Maintenance_scrn_Intfeat_count;
//            sprintf((char*)tech_menu_disp.text, "PROBLEM CODE: %02d		   ", Maintenance_Log.Prob_Code);
//            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//        }
//        else if(maintenance_scrn_count==2)
//        {
//            tech_menu_disp.row   = 32*Maintenance_scrn_Intfeat_count;
//            sprintf((char*)tech_menu_disp.text, "PROBLEM CODE: %02d		   ", Maintenance_Log.Prob_Code);
//            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//
//            tech_menu_disp.font  = font_1_invert_text;
//            tech_menu_disp.row   = 32*(Maintenance_scrn_Intfeat_count+(maintenance_scrn_count-1));
//            sprintf((char*)tech_menu_disp.text, "REPAIR CODE: %02d		   ", Maintenance_Log.Sol_Code);
//            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//        }
//        else if(maintenance_scrn_count==3)
//        {
//            tech_menu_disp.row   = 32*(Maintenance_scrn_Intfeat_count+(maintenance_scrn_count-2));
//            sprintf((char*)tech_menu_disp.text, "REPAIR CODE: %02d		   ", Maintenance_Log.Sol_Code);
//            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//
//            tech_menu_disp.font  = font_1_invert_text;
//            tech_menu_disp.row   = 32*(Maintenance_scrn_Intfeat_count+(maintenance_scrn_count-1));
//            sprintf((char*)tech_menu_disp.text, "SAVE MAINTEN.LOG?NO	   ");
//            printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//        }
//    }
    else if(comm_int_scrn==true)
    {
      if(comm_int_scrn_count<Comm_Int_Scrn_Count)
      {
    	comm_int_scrn_count++;
      }
      else
      {
    	  comm_int_scrn_count=1;
      }
	  	tech_menu_disp.column = 0;
      if(comm_int_scrn_count==1)
      {
    	  save_comm_interval_changes=false;
    	  tech_menu_disp.font   = font_1;
    	  tech_menu_disp.row   = 32*(Comm_Int_Scrn_Count);
    	  sprintf((char*)tech_menu_disp.text, "SAVE CHANGES?NO" );
    	  printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    	  tech_menu_disp.row    = 32*comm_int_scrn_count;
    	  tech_menu_disp.font   = font_1_invert_text;
    	  if((do_RTC_EVT_AFM_HB&0x01)==0x01)
    	  {
    		  sprintf((char*)tech_menu_disp.text, "TRIGGER GENSYNC?YES       " );
    	  }
    	  else
    	  {
    		  sprintf((char*)tech_menu_disp.text, "TRIGGER GENSYNC?NO        " );
    	  }
    	  printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
      }
      else if(comm_int_scrn_count==2)
      {
    	  tech_menu_disp.row    = 32*(comm_int_scrn_count-1);
    	  tech_menu_disp.font   = font_1;
    	  if((do_RTC_EVT_AFM_HB&0x01)==0x01)
    	  {
    		  sprintf((char*)tech_menu_disp.text, "TRIGGER GENSYNC?YES       " );
    	  }
    	  else
    	  {
    		  sprintf((char*)tech_menu_disp.text, "TRIGGER GENSYNC?NO        " );
    	  }
    	  printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    	  tech_menu_disp.font   = font_1_invert_text;
    	  tech_menu_disp.row    = 32*comm_int_scrn_count;
    	  if((do_RTC_EVT_AFM_HB&0x02)==0x02)
    	  {
    		  sprintf((char*)tech_menu_disp.text, "TRIGGER EVENT?YES         " );
    	  }
    	  else
    	  {
    		  sprintf((char*)tech_menu_disp.text, "TRIGGER EVENT?NO          " );
    	  }
    	  printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
      }
      else if(comm_int_scrn_count==3)
      {
    	  tech_menu_disp.font   = font_1;
    	  tech_menu_disp.row    = 32*(comm_int_scrn_count-1);
    	  if((do_RTC_EVT_AFM_HB&0x02)==0x02)
    	  {
    		  sprintf((char*)tech_menu_disp.text, "TRIGGER EVENT?YES        " );
    	  }
    	  else
    	  {
    		  sprintf((char*)tech_menu_disp.text, "TRIGGER EVENT?NO         " );
    	  }
    	  printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);

    	  tech_menu_disp.font   = font_1_invert_text;
    	  tech_menu_disp.row   = 32*(comm_int_scrn_count);
    	  save_comm_interval_changes=false;
		  sprintf((char*)tech_menu_disp.text, "SAVE CHANGES?NO          " );

		  printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
      }
    }
    else if(advanced_feature_count != 0)
    {
    	Display_position = 4;
    	tech_software_information();
    }
    return;
}

/**************************************************************************/
//! Display maintenance menu screen.
//! \param void
//! \return void
/**************************************************************************/
//static void tech_display_maintenance_menu() //vinay code opt
//{
//	Clear_Screen();
//
//    Maintenance_Log.Prob_Code=0;
//    Maintenance_Log.Sol_Code=0;
//    tech_menu_disp.row   = 0;
//    tech_menu_disp.column= 0;//10;
//    tech_menu_disp.font  = font_1_invert_text;
//    sprintf((char*)tech_menu_disp.text, "MAINTENANCE LOG		  ");
//    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//
//    tech_menu_disp.row   += 32;
//    tech_menu_disp.column= 0;//5;
//    tech_menu_disp.font  = font_1;
//    sprintf((char*)tech_menu_disp.text, "TECH CARD SN: %08ld", Maintenance_Log.TECH_CARD_SL_NO);//27-06-12
//    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//
//    tech_menu_disp.row   += 32;
//
//    if( glAlarm_req.alarm_state== MB_ALARM_STATE_RAISED)
//    {
//        tech_menu_disp.font  = font_1;
//        sprintf((char*)tech_menu_disp.text, "ALARM CODE: %d", glAlarm_req.alarm_id);
//        printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//    }
//    else
//    {
//        glAlarm_req.alarm_id=0;
//        tech_menu_disp.font  = font_1;
//        sprintf((char*)tech_menu_disp.text, "ALARM CODE: %d", glAlarm_req.alarm_id);
//        printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//    }
//
//    tech_menu_disp.row   += 32;
//    tech_menu_disp.font  = font_1_invert_text;
//    sprintf((char*)tech_menu_disp.text, "PROBLEM CODE: %d", Maintenance_Log.Prob_Code);
//    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//
//    tech_menu_disp.row   += 32;
//    tech_menu_disp.font  = font_1;
//    sprintf((char*)tech_menu_disp.text, "REPAIR CODE: %d", Maintenance_Log.Sol_Code);
//    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//
//    tech_menu_disp.row   += 32;
//    tech_menu_disp.font  = font_1;
//    sprintf((char*)tech_menu_disp.text, "SAVE MAINTEN.LOG?NO");
//    printSharp16x24(tech_menu_disp.text, tech_menu_disp.row, tech_menu_disp.column, tech_menu_disp.font);
//
//    copy_files_screen = false;
//    advanced_feature  = false;
//    clear_alarm_check = false;
//    coms_info         = false;
//    Tech_menu_info    = false;
//    maintenance_screen= true;
//}

/**************************************************************************/
//! Initialize all technician menu parameters.
//! \param void
//! \return void
/**************************************************************************/
void initialise_tech_menu_param()
{
	Tech_menu=false;
	gl_Commission_Card=false;//05-08-2013:DPLIBB-554

	reset_timeout_count=false;
	Static_display=false;//11-04-12
	tech_mode_timeout=0;
	advanced_feature=false;
	clear_alarm_check=false;
	copy_files_screen=false;
	Adjust_clock=false;
	exit_filemgr=false;
	copy_lid_details=false;
	save_maintenance_log=false;
	maintenance_screen=false;
	copy_files_screen_count=0;
	Adjust_clock_key_count=1;
	maintenance_scrn_count=1;
	Display_position=0;//1;  //to send the cursor to line one (tech info) always when entered to techmenu //vinay LNGSIT
	advanced_feature_count=1;
	Display_meter_info_2ndpage=false;
	file_manager_screen=0;
	do_not_copy_files=false;
	copy_all_files=false;
	copy_only_config=false;
	copy_only_mpb_firmware=false;
	DK2Mech_copy_option_count=1;
	Mech2DK_copy_option_count=1;
	comm_int_scrn=false;
}

#if 0
uint8_t check_for_valid_data(uint8_t *data)
{
	uint8_t i=0,l=0,m=0;
	for(i=0;i<20;i++)
	{
		if(data[i]==0xff)
			l++;
	}
	for(i=0;i<20;i++)
	{
		if(data[i]==0)
		{
			m++;
		}
	}
	if((l==20)||(m==20))
	{
		return false;
	}
	return true;
}
#endif

/**************************************************************************/
//! Copy all files from flash to data key.
//! \param void
//! \return void
/**************************************************************************/
void Copy_all_flash_files_to_datakey()
{
	uint8_t fcount;
	DK_MECH_files_copied_count=0;
	//if(copy_due_to_FD_activation==false)
	{
		Static_display=true;//25-05-12
		reset_timeout_count=true;
	}
	Debug_TextOut( 0, "Copying files from mech to data key" );
	//check for pending and activated files
	Flash_Batch_Flag_Read();
	for( fcount = 0; fcount < FILE_TYPE_TOTAL_FILES; fcount ++ )
	{
		if((flash_file_actvn_tbl[fcount].status!=FF_161_COMPLETED) && (flash_file_actvn_tbl[fcount].status!=FF_ACTIVATE_COMPLETED))
		{
			diag_text_Screen( "FILE(S) ACTVN PENDING    ", TRUE, FALSE );
			DelayMs(500);
			diag_text_Screen( "COPYING FILE FAILED      ", TRUE, FALSE );
			//if(copy_due_to_FD_activation==false)
			{
				tech_software_information();
				reset_timeout_count=false;
				copy_files_screen=false;
				Static_display=false;
			}
			//return;
		}
	}

	Copy_only_MPB_config_2DK();
	Copy_only_MPB_firmware_2DK();

	tech_software_information();//26-03-12
	reset_timeout_count=false;
	copy_files_screen=false;
	Static_display=false;//25-05-12
}

/**************************************************************************/
//! Copy all files from data key to flash.
//! \param void
//! \return void
/**************************************************************************/
void Copy_all_files_to_mechanism()
{
	DK_MECH_files_copied_count=0;

	Debug_TextOut( 0, "Copying files from data key to mech" );

	Copy_only_MPB_config_2mech();

	Copy_only_MPB_firmware_2mech();

	copy_files_screen=false;
	tech_software_information();
	reset_timeout_count=false;
	Static_display=false;//25-05-12
}

/**************************************************************************/
//! Copy only MPB config file from data key to flash.
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t Copy_only_MPB_config_2mech()
{
	uint8_t result;
	Static_display = true;//25-05-12
	reset_timeout_count = true;
	activate_mpb_config = false;

	diag_text_Screen( "COPY LNG CONFIG FILE     ", TRUE, FALSE );

	CONFIG_RSAD_DK=AD_MPB_CONFIG_DK;
	CONFIG_RSAD_DK=DataKey_Read(CONFIG_RSAD_DK,4);
	file_size_flash=(((DK_data_read_buffer[0]&0xffffffff)<<24)|((DK_data_read_buffer[1]&0xffffffff)<<16)|((DK_data_read_buffer[2]&0xffffffff)<<8)|(DK_data_read_buffer[3]&0xffffffff));
	Debug_Output1( 0, "DK CFG File Size: %08X", file_size_flash );
	if((file_size_flash<=sizeof(glSystem_cfg))&&(file_size_flash!=0))
	{
		result=Copy_from_datakey_to_flash(TECH_MENU_MPB_CONFIG, (file_size_flash+sizeof(file_size_flash)));//23-05-12
		if(result==0)
		{
			diag_text_Screen( "CFG COPY FROM DK FAILED  ", TRUE, FALSE );
			if(copy_all_files!=true)
			{
				tech_software_information();
				reset_timeout_count=false;
				Static_display=false;
				copy_files_screen=false;
			}
			activate_pgm_rpg_ccf=false;
			return 0;
		}
		else
		{
			diag_text_Screen( "CFG COPY FROM DK SUCCESS ", TRUE, FALSE );
			//activate_mpb_config = true;
			push_event_to_cache( UDP_EVTTYP_CONFIG_FILE_COPIED_FROM_DATAKEY );
			//check_pending_requests();
			//Tasks_Priority_Register |= SEVERE_EVENTS_TASK;     //LNGSIT-730,729
		}
		result = activate_config_file( true );//20-09-12
		Flash_Batch_Flag_Read();
		if(result==true)
		{
			flash_file_actvn_tbl[FFACT_MPB_CONFIG].status = FF_161_COMPLETED;
			flash_file_actvn_tbl[FFACT_MPB_CONFIG].actvn_try_count = 0;
		}
		else
		{
			flash_file_actvn_tbl[FFACT_MPB_CONFIG].status = FF_STALE;
			flash_file_actvn_tbl[FFACT_MPB_CONFIG].actvn_try_count = 0;
		}
		Flash_Batch_Flag_Write();
	}
	else
	{
		diag_text_Screen( "INVALID FILE             ", TRUE, FALSE );
		if(copy_all_files!=true)
		{
			tech_software_information();
			reset_timeout_count=false;
			copy_files_screen=false;
			Static_display=false;
		}
		return 0;
	}
	if(copy_all_files!=true)
	{
		copy_files_screen=false;
		tech_software_information();
		reset_timeout_count=false;
		Static_display=false;
	}
	else
	{
		DK_MECH_files_copied_count++;
	}
	return 1;
}

/**************************************************************************/
//! Copy only MPB Binary file from data key to flash.
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t Copy_only_MPB_firmware_2mech()
{
	uint8_t result;
	Static_display=true;//25-05-12
	reset_timeout_count=true;
	activate_mpb_binary_from_dk = false;//25-07-12

	MPB_CODE_RSAD_DK=AD_MPB_BINARY_DK;
	diag_text_Screen( "COPY LNG BIN FILE        ", TRUE, FALSE );

	MPB_CODE_RSAD_DK=DataKey_Read(MPB_CODE_RSAD_DK,7);
	file_size_flash=(((DK_data_read_buffer[3]&0xffffffff)<<24)|((DK_data_read_buffer[4]&0xffffffff)<<16)|((DK_data_read_buffer[5]&0xffffffff)<<8)|(DK_data_read_buffer[6]&0xffffffff));
	if(file_size_flash==MPB_CODE_SIZE)
	{
		Flash_Batch_Flag_Read();
		flash_file_actvn_tbl[FFACT_MPB_CODE].size=MPB_CODE_SIZE;
		Flash_Batch_Flag_Write();

		result=Copy_from_datakey_to_flash(TECH_MENU_MPB_CODE,file_size_flash);

		if(result==0)
		{
			diag_text_Screen( "BIN COPY FROM DK FAILED  ", TRUE, FALSE );
			if(copy_all_files!=true)
			{
				tech_software_information();
				reset_timeout_count=false;
				copy_files_screen=false;
				Static_display=false;
			}
			return 0;
		}
		else
		{
			diag_text_Screen( "BIN COPY FROM DK SUCCESS ", TRUE, FALSE );
			push_event_to_cache( UDP_EVTTYP_BINARY_FILE_COPIED_FROM_DATAKEY );

			Flash_Batch_Flag_Read();
			flash_file_actvn_tbl[FFACT_MPB_CODE].type = FILE_TYPE_MPB_CODE;
			flash_file_actvn_tbl[FFACT_MPB_CONFIG].status = FF_161_COMPLETED;	//LNGSIT-730
			flash_file_actvn_tbl[FFACT_MPB_CONFIG].actvn_try_count = 0;
			Flash_Batch_Flag_Write();

			activate_mpb_binary_from_dk = true;
			//check_pending_requests();
			//Tasks_Priority_Register |= SEVERE_EVENTS_TASK;     //LNGSIT-730,729
		}
	}
	else
	{
		diag_text_Screen( "INVALID FILE             ", TRUE , FALSE);
		if(copy_all_files!=true)
		{
			tech_software_information();
			reset_timeout_count=false;
			copy_files_screen=false;
			Static_display=false;
		}
		return 0;
	}

	if(copy_all_files!=true)
	{
		copy_files_screen=false;
		tech_software_information();
		reset_timeout_count=false;
		Static_display=false;//25-05-12
	}
	else
	{
		DK_MECH_files_copied_count++;
	}
	return 1;
}

/**************************************************************************/
//! Copy only MPB config file from flash to data key.
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t Copy_only_MPB_config_2DK()
{
	uint8_t result;
	diag_text_Screen( "COPY LNG CONFIG FILE     ", TRUE, FALSE );
	Flash_Batch_Flag_Read();
	if((flash_file_actvn_tbl[FFACT_MPB_CONFIG].status!=FF_161_COMPLETED) && (flash_file_actvn_tbl[FFACT_MPB_CONFIG].status!=FF_ACTIVATE_COMPLETED))
	{
		diag_text_Screen( "CONFIG ACTVN PENDING		", TRUE, FALSE );
		DelayMs(500);
		diag_text_Screen( "COPYING FILE FAILED      ", TRUE, FALSE );
		if(copy_all_files!=true)
		{
			tech_software_information();
			reset_timeout_count=false;
			copy_files_screen=false;
			Static_display=false;
		}
		//return 0;
	}

	config_RSAD=AD_MPB_config_r;
	config_RSAD=Flash_Read(config_RSAD,4);
	file_size_flash=(((flash_data_read_buffer[0]&0xffffffff)<<24)|((flash_data_read_buffer[1]&0xffffffff)<<16)|((flash_data_read_buffer[2]&0xffffffff)<<8)|(flash_data_read_buffer[3]&0xffffffff));
	if((file_size_flash <= sizeof( glSystem_cfg ))&&(file_size_flash!=0))
	{
		result=Copy_from_flash_to_datakey(TECH_MENU_MPB_CONFIG,(file_size_flash+sizeof(file_size_flash)), true);//23-05-12
		if(result==0)
		{
			diag_text_Screen( "CFG COPY TO DK FAILED    ", TRUE, FALSE );
			if(copy_all_files!=true)
			{
				tech_software_information();
				reset_timeout_count=false;
				copy_files_screen=false;
				Static_display=false;
			}
			return 0;
		}
		else
		{
			diag_text_Screen( "CFG COPY TO DK SUCCESS   ", TRUE, FALSE );
			push_event_to_cache( UDP_EVTTYP_CONFIG_FILE_COPIED_TO_DATAKEY );
		}
	}
	else
	{
		diag_text_Screen( "INVALID FILE             ", TRUE, FALSE );
		if(copy_all_files!=true)
		{
			tech_software_information();
			reset_timeout_count=false;
			copy_files_screen=false;
			Static_display=false;
		}
		return 0;
	}
	if(copy_all_files!=true)
	{
		copy_files_screen=false;
		tech_software_information();
		reset_timeout_count=false;
		Static_display=false;
	}
	else
	{
		DK_MECH_files_copied_count++;
	}
	return 1;
}

/**************************************************************************/
//! Copy only MPB Binary file from flash to data key.
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t Copy_only_MPB_firmware_2DK()
{
	uint8_t result;
	mpb_code_RSAD=AD_MSP_CODE_r;

		diag_text_Screen( "COPY LNG BINARY FILE     ", TRUE, FALSE );
		Flash_Batch_Flag_Read();
		if((flash_file_actvn_tbl[FFACT_MPB_CODE].status!=FF_161_COMPLETED) && (flash_file_actvn_tbl[FFACT_MPB_CODE].status!=FF_ACTIVATE_COMPLETED))
		{
			diag_text_Screen( "LNG BIN ACTVN PENDING    ", TRUE, FALSE );
			DelayMs(500);
			diag_text_Screen( "COPYING FILE FAILED      ", TRUE, FALSE );
			if(copy_all_files!=true)
			{
				tech_software_information();
				reset_timeout_count=false;
				copy_files_screen=false;
				Static_display=false;
			}
			//return 0;
		}

	Flash_Batch_Flag_Read();
	file_size_flash=flash_file_actvn_tbl[FFACT_MPB_CODE].size;

	if(file_size_flash==MPB_CODE_SIZE)
	{
			result=Copy_from_flash_to_datakey(TECH_MENU_MPB_CODE,file_size_flash,MULTIPLE_BLOCK_ERASE_FLAG);//05-04-12//23-05-12
			if(result==0)
			{
				diag_text_Screen( "BIN COPY TO DK FAILED    ", TRUE, FALSE );
				if(copy_all_files!=true)
				{
					tech_software_information();
					reset_timeout_count=false;
					copy_files_screen=false;
					Static_display=false;
				}
				return 0;
			}
			else
			{
				diag_text_Screen( "BIN COPY TO DK SUCCESS   ", TRUE, FALSE );
				push_event_to_cache( UDP_EVTTYP_BINARY_FILE_COPIED_TO_DATAKEY );
			}
	}
	else
	{
		diag_text_Screen( "INVALID FILE             ", TRUE, FALSE );
		if(copy_all_files!=true)
		{
			tech_software_information();
			reset_timeout_count=false;
			copy_files_screen=false;
			Static_display=false;
		}
		return 0;
	}
	if(copy_all_files!=true)
	{
		copy_files_screen=false;
		tech_software_information();
		reset_timeout_count=false;
		Static_display=false;
	}
	else
	{
		DK_MECH_files_copied_count++;
	}
	return 1;
}

/**************************************************************************/
//! Make meter Out Of Service.
//! \param void
//! \return void
/**************************************************************************/
void Meter_Out_of_service()
{
	meter_out_of_service = true;
	parking_time_left_on_meter[Current_Space_Id] = 0;
	Update_Parking_Clock_Screen();
	Meter_Out_Of_Serice_Screen();
}

/**************************************************************************/
//! Make meter In Service.
//! \param void
//! \return void
/**************************************************************************/
void Meter_In_Service()
{
	//RTC_C_enableInterrupt(RTC_C_TIME_EVENT_INTERRUPT);
	//start_coinwakeup_timer();
	//watchdog_init();
	parking_time_left_on_meter[Current_Space_Id] = 0;
	meter_out_of_service = false;
	Idle_Screen();	//Idle screen is the In service screen
}

/**************************************************************************/
//! Exits from technician mode, called from the main menu screen.
//! \param void
//! \return void
/**************************************************************************/
//void main_exit() //vinay
//{
//	initialise_tech_menu_param();//27-11-12
//	Debug_TextOut( 0, "LNG Exited Tech Mode" );
//	cardread = 0;
//	Display_position=0; //to send the cursor to line one (tech info) always when entered to techmenu //vinay LNGSIT
//	Tech_menu = false;
//
//	if(GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN) == 0)
//	{
//		uint8_t z;
//		if(Tech_key_seq == FALSE)
//		{
//			Clear_Screen();
//			diag_text_Screen("  PLEASE REMOVE CARD     ", TRUE , FALSE );
//		}
//		Tech_key_seq = FALSE;
//		for(z=0;z<50;z++)
//		{
//			DelayMs(100);
//			if(GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN) != 0)
//				break;
//		}
//	}
//	Tech_key_seq = FALSE;
//	//Debug_TextOut( 0, "TM 3515 DSM_Screen1" );
//	DSM_Screen1();//Idle_Screen();
//}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

