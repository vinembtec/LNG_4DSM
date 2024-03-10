//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_lte.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_lte_api
//! @{
//
//*****************************************************************************

#include "LibG2_lte.h"

#define Enable_Reduce_CC_Processing_Delay   //If you want to minimize/reduce CC_Processing_Delay while doing card transaction, enable this

/*
***********************************************************************
*Imported Global Variables
***********************************************************************
*/
extern uint8_t          		glClose_socket;    //3009
extern uint8_t          		Debug_Verbose_Level;
extern uint8_t          		MDM_AWAKE;
extern GPRSSystem       		glSystem_cfg;
extern uint8_t         			glComm_failure_count;
extern uint8_t 					Last_Comms_Success_Flg;
extern uint8_t 					Initial_boot;
extern uint8_t					Meter_Reset, CC_Auth_Entered, do_CC_OLT;
extern uint8_t					Modem_Off_initiated;
extern uint8_t					Modem_On_Count, Parking_Clock_Refresh[MSM_MAX_PARKING_BAYS];
extern uint16_t 				Current_Space_Id;

/*
***********************************************************************
*Exported Global Variables
***********************************************************************
*/
uint8_t                			mdm_comm_status = 0;
uint8_t                 		DO_IP_SYNC      = false;     //2103: PBC
uint8_t         				MDM_AWAKE = FALSE,IP_done = FALSE;
//uint8_t							mdm_init_retries =2; //this is used only once in the program and called anywhere, so placing it inside the fuction//vinay
uint8_t							FIRST_COMM;
uint8_t    						cmd_send[ GPRS_GEN_BUF_SZ_VSMALL ];
uint8_t             			glTemp_buf[GPRS_GEN_BUF_SZ_SMALL];
uint8_t             			rx_ans[ GPRS_GEN_BUF_SZ_MED ];
Modem               			sys_mdm_p;
uint8_t         				glIn_diag_mode = FALSE;     //diag sequence
uint16_t			            glMdmUart_bytes_recvd = 0;
uint8_t             			glMdmUart_recv_buf[ GPRS_GEN_BUF_SZ_VLARGE ];
uint8_t							Regn_Check_Count = 0;//, gReInit_Mdm = 0;//this is used only once in the program and called anywhere, so placing it inside the fuction//vinay
uint8_t							Send_SMS_ACK = FALSE;
uint8_t							COMMS_DOWN = FALSE, MDM_Type_LE910NA1 = FALSE, MDM_Type_ME910C1WW = FALSE;
uint32_t						COMMS_DOWN_timestamp = 0;
uint32_t						MDM_On_Time = 0,MDM_Off_Time = 0,MDM_Total_On_Time = 0;

Operator						nw_operator_name_id[MAX_OPERATORS_ALLOWED];

uint8_t 						modem_init_to_process_cc_transaction = 0; //@r just added for testing
uint8_t							Modem_Test_Flag = FALSE;
/*
***********************************************************************
*Local Constants
***********************************************************************
*/
static const char * 			cmd_context_check    = "AT#SGACT?\r";
static const char * 			cmd_context_act      = "AT#SGACT=1,1\r";
//static const char * 			cmd_context_act_cdma      = "AT#SGACT=1,1,9342768265,9342768265\r";
//static const char * 			cmd_context_act_cdma      = "AT#SGACT=1,1\r";
static const char * 			cmd_autocontext_act  = "AT#SGACTCFG=1,3\r";
static const char * 			cmd_soc_cfg_evt      = "AT#SCFG=1,1,1050,0,600,10\r";    //0710
static const char * 			cmd_autocontxt_act_ext = "AT#SCFGEXT2=1,1\r";
//static const char * 			cmd_soc_cfg_evt      = "AT#SCFG=1,1,1000,0,600,10\r";    //0710
//static const char *				cmd_soc_cfg_ext		 = "AT#SCFGEXT=1,2,0,0,0,0\r";
//static const char *			cmd_show_ipaddr		 = "AT#CGPADDR=1\r";

static uint8_t      			glTemp_buf2[GPRS_GEN_BUF_SZ_VSMALL];
static uint8_t 					Final_mdm_init_attempt = false;
static uint8_t 					mdm_init_for_recovery = 0;
static uint8_t 					mdm_recovery_state = 0;
static uint8_t 					mdm_on_loop_count = 0;
static uint8_t					mdm_off_loop_count = 0;
static uint8_t					last_network_tried = SECONDARY_OPERATOR;
uint8_t 						FD_BackGround_Occuring_Flag = 0;
/*
//static const char * 			cmd_vodafone_network_selection		= "AT+COPS=4,0,\"Vodafone IN\",2\r";
static const char * 			cmd_vodafone_network_selection		= "AT+COPS=4,2,40486,2\r";
//static const char * 			cmd_airtel_network_selection    	= "AT+COPS=4,0,\"IND airtel\",2\r";
static const char * 			cmd_airtel_network_selection    	= "AT+COPS=4,2,40445,2\r";
//static const char * 			cmd_tmob_network_selection      	= "AT+COPS=4,0,T-Mobile,2\r";
static const char * 			cmd_tmob_network_selection      	= "AT+COPS=4,2,310260,2\r";
//static const char * 			cmd_att_network_selection       	= "AT+COPS=4,0,AT&T,2\r";
static const char * 			cmd_att_network_selection       	= "AT+COPS=4,2,310410,2\r";
*/
////extern uint8_t 				CC_Transaction_Retry_Request_Interval;

//Auto APN new settings -- 18022022 -- vivek
uint8_t APN_Assigned = 0;
//To copy default/config apn -- 18022022
extern uint8_t             default_apn[GPRS_GEN_BUF_SZ_TINY];
extern uint8_t             default_UDP_server_ip[ GPRS_IPADD_BUF_SZ ];
extern uint16_t            default_UDP_server_port;
uint8_t                    ICCID_First_6_Digit[7] = {"/0"};
bool                       APN_Assigned_ignore_steps = FALSE;
bool                       t_mobile_apn_switch_flag = FALSE;
bool                       first_time_ws46_set_flag = TRUE;
uint8_t                    ws46_selected_value = 0;
uint8_t                    ws46_selected_value_temp = 0;

extern uint32_t                min_col_counter_read1,accumulated_value_to_report1;
/**************************************************************************/
//! Part of Telit Modem Power ON cycle, this function presses the modem on
//! switch till the required amount of time and then releases it
//! \param void
//! \return void
/**************************************************************************/
void TELIT_POWER_ON()
{
	uint8_t i = 0;
	MDM_Tel_ON();
	for(i = 0; i < 8; i++)
	{
		if(Parking_Clock_Refresh[Current_Space_Id] == PARKING_CLOCK_RESET_MODE) //This is added to refresh clock for new payments
		{
			//Debug_TextOut(0,"Parking Clock");
			Update_Parking_Clock_Screen();
			Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_STOP_MODE;
		}
		if((i>=5)&&(mdm_send_AT_command("AT\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*10) == FALSE))
		{
			break;
		}
		DelayMs(1000);
	}
	//DelayMs(5000);
	MDM_Tel_OFF();
	mdm_on_loop_count = 0;
	mdm_off_loop_count = 0;
}

/**************************************************************************/
//! Part of Telit Modem Power OFF cycle, this function presses the modem on
//! switch till the required amount of time and then releases it
//! \param void
//! \return void
/**************************************************************************/
void TELIT_POWER_OFF()
{
	uint8_t i = 0;
	if(Modem_Off_initiated < 2)	//either false(not a stepwise modem off) or first step of modem stepwise off
	{
		MDM_Tel_ON();

		if(Modem_Off_initiated == 0)
		{
			for(i = 0; i < 4; i++)
			{
				if(Parking_Clock_Refresh[Current_Space_Id] == PARKING_CLOCK_RESET_MODE) //This is added to refresh clock for new payments
				{
					//Debug_TextOut(0,"Parking Clock");
					Update_Parking_Clock_Screen();
					Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_STOP_MODE;
				}
				DelayMs(1000);
			}
			//DelayMs(4000);
			MDM_Tel_OFF();
		}
	}
	else if(Modem_Off_initiated == 2)	//stepwise modem off time up
	{
		MDM_Tel_OFF();
		Modem_Off_initiated = 0;
	}
	mdm_on_loop_count = 0;
	mdm_off_loop_count = 0;
}

/**************************************************************************/
//! Part of Telit Modem Power ON cycle, this function turns ON the power
//! regulator in Comms board
//! \param void
//! \return void
/**************************************************************************/
void TELIT_POWER_ENABLE()
{
	MDM_REG_CNTRL_set();
}

/**************************************************************************/
//! Part of Telit Modem Power OFF cycle, this function turns OFF the power
//! regulator in Comms board
//! \param void
//! \return void
/**************************************************************************/
void TELIT_POWER_DISABLE()
{
	MDM_REG_CNTRL_clr();
}

/**************************************************************************/
//! Powers Off Telit Modem
//! \param void
//! \return void
/**************************************************************************/
void telit_power_off()
{
	telit_wakeup_sleep(TRUE);

	TELIT_POWER_OFF();
	if(Modem_Off_initiated == 0)
	{
		TELIT_POWER_DISABLE();
		MDM_RST_clr();
		GPIO_setAsInputPinWithPullUpResistor(MDM_PORT, MDM_RXD_PIN);
		disable_telit_RI_interrupt();
		Debug_TextOut( 0, "PWR Off MDM1" );
		MDM_Off_Time = RTC_epoch_now();
		if(MDM_On_Time == 0)
		{
			MDM_On_Time = MDM_Off_Time;
		}

		MDM_Total_On_Time +=(uint32_t)((uint32_t)(MDM_Off_Time)-(uint32_t)(MDM_On_Time));
		//Debug_Output1(2,"Total_Modem_on_time:%d",MDM_Total_On_Time);
		MDM_On_Time = 0;

		mdm_comm_status = 0;
		MDM_AWAKE = FALSE;
		Last_Comms_Success_Flg = FALSE;
	}
}

/**************************************************************************/
//! Powers On Telit Modem
//! \param void
//! \return void
/**************************************************************************/
void telit_power_on()
{
	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
	{
		return;
	}
	else
	{
		if(Modem_Off_initiated != 0)
		{
			Debug_TextOut(0,"Modem off initiated; Cannot interrupt");
			return;
		}
		GPIO_setAsPeripheralModuleFunctionInputPin(MDM_PORT, MDM_RXD_PIN, GPIO_PRIMARY_MODULE_FUNCTION);	//MDM_RXD
		MDM_RST_clr();
		telit_wakeup_sleep(FALSE);

		TELIT_POWER_ENABLE();
		DelayMs(300);
		TELIT_POWER_ON();
		DelayMs(400);
		Debug_TextOut( 0, "PWR On MDM1" );
		MDM_On_Time = RTC_epoch_now();
		Modem_On_Count++;
		MDM_AWAKE = TRUE;
		Regn_Check_Count =0;
	}
}

/**************************************************************************/
//! Brings the modem out of sleep mode or puts the modem to sleep mode by
//! toggling the DTR pin based on the parameter passed
//! \param uint8_t mdm_sleep_enable
//! 	- \b true - puts to sleep
//!     - \b false - wakes up from sleep
//! \return void
/**************************************************************************/
void telit_wakeup_sleep(uint8_t mdm_sleep_enable)
{
	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
	{
		return;
	}
	else
	{
		if(mdm_sleep_enable == TRUE)
		{
			MDM_DTR_clr();
			//UART_disableModule(EUSCI_A0_BASE);
			//Debug_TextOut( 0, "MDM Slept" );
		}
		else
		{
			MDM_DTR_set();
			//UART_enableModule(EUSCI_A0_BASE);
			//Debug_TextOut( 0, "MDM Woke Up" );
		}
	}
}

/**************************************************************************/
//! This function is used to do a complete reinitialization of modem
//! including Controller board's UART reinit, Telit Modem power recycle
//! \param void
//! \return void
/**************************************************************************/
void reinit_telit_Modem()
{
	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
	{
		return;
	}
	else
	{
		init_Telit_GPIO();

		telit_power_off();
		DelayMs(800);
		telit_power_on();
	}
}

/**************************************************************************/
//! This function is used to do a complete reinitialization of modem
//! including Controller board's UART reinit, Telit Modem power recycle,
//! AT commands initialization
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t telit_reinit()
{
	uint8_t        result = MDM_ERR_NONE;

	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
	{
		return result;
	}
	else
	{
		reinit_telit_Modem();

		//if(Get_HW_Version() != MPB_LIBERTY_1_0C)	//TODO: Avoid calling this for CDMA Modem
		result = mdm_init();

		return result;
	}
}

/**************************************************************************/
//! AT commands initialization, these are the boot up commands needed only
//! once per meter boot up, not called in every modem power recycle subsequently
//! \param void
//! \return void
/**************************************************************************/
void mdm_init_once_per_bootup()
{
	//int16_t        result = MDM_ERR_NONE;
	//Success or failure of these commands are not important, so ignore result check
	uint8_t i = 0;
	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
	{
		return;
	}
	else
	{
	    Debug_Output1(0, "mdm_init_once_per_bootup", 0);
		for(i = 0; i <= 20; i++)
		{
			////Debug_TextOut( 0, "mdm_init_once_per_bootup retry loop" );
			if(mdm_send_AT_command("AT\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*10) == FALSE)
			{
				break;
			}
			watchdog_pat();
			DelayMs(1000);
		}
if(APN_Assigned_ignore_steps == FALSE)
	{
		DelayMs(2000); //ADDED BY VT
		mdm_send_AT_command("AT\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT);

		mdm_send_AT_command("AT\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT);

		mdm_send_AT_command("AT#AUTOBND=2\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

		mdm_send_AT_command("ATE0&K0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

		mdm_send_AT_command("AT+CMEE=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

		DelayMs(100);
        ////mdm_send_AT_command("AT+CFUN=4\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
        DelayMs(100);

        DelayMs(100);
        mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

        DelayMs(100);
        mdm_send_AT_command("AT+CFUN=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

        DelayMs(100);
        mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

        DelayMs(100);
        mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

        DelayMs(1000);

        mdm_send_AT_command("AT+WS46?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

        mdm_send_AT_command("AT#CFLO=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

		mdm_send_AT_command("ATS0=0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

		mdm_send_AT_command("AT#SKIPESC=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

		mdm_send_AT_command("ATS12=20\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

		mdm_send_AT_command("AT#E2SLRI=1000\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

		mdm_send_AT_command("AT#PSMRI=1000\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

    }
		mdm_send_AT_command("AT+CEMODE=2\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100); //3g Sunset ATT //vinay

		/*if(mdm_find_response(glSystem_cfg.apn, "sierra" ) == TRUE)
	{
		Debug_TextOut(0,"Latch to AT&T first");
		mdm_send_AT_command(cmd_att_network_selection, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*300);
	}
	else
	{
		Debug_TextOut(0,"Automatic Network Selection");
	}*/

		watchdog_pat();
		mdm_send_AT_command("AT+WS46?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

        mdm_send_AT_command("AT+WS46=?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

#if 0
        //NO NEED TO CHANGE TO 2G, MODEM AUTOMATICALLY CHANGE IT IN TO 2G FALL BACK -- VT 28-03-2022
        //if(mdm_find_response(glSystem_cfg.apn, "www" ) == TRUE)
                mdm_send_AT_command("AT+WS46=12\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
/*        else //ADDED NEWLY TO GET COMMS FOR 4GMOBILE -- VT
            {

            }*/
#endif

#if 1
        mdm_send_AT_command("AT+WS46?\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
        Debug_Output1(0,"AT+WS46? response is: %s", (uint32_t)rx_ans);

        memset(sys_mdm_p.type,0,sizeof(sys_mdm_p.type));
        mdm_fetch_type(sys_mdm_p.type, sizeof(sys_mdm_p.type));
        Debug_Output1(0,"MDM Type: %s", (uint32_t)sys_mdm_p.type);

        mdm_send_AT_command("AT+COPS?\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
        Debug_Output1(0,"AT+COPS? response is: %s", (uint32_t)rx_ans);
        if((mdm_find_response((uint8_t*)rx_ans, "\",0" )==true) && (first_time_ws46_set_flag == FALSE))
        {
            mdm_send_AT_command("AT+WS46=12\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
        }
        else
        {
            if(mdm_find_response((uint8_t*)sys_mdm_p.type, "ME910C1-WW" )==true)
                {
                    mdm_send_AT_command("AT+WS46=30\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                    ws46_selected_value = 2;
                }
            else
                {
                    mdm_send_AT_command("AT+WS46?\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
                    Debug_Output1(0,"AT+WS46? response is: %s", (uint32_t)rx_ans);
                    ////if((mdm_find_response((uint8_t*)rx_ans, "12" )==true)||(mdm_find_response((uint8_t*)rx_ans, "+WS46: 12" )==true))
                        {
                            mdm_send_AT_command("AT+WS46=30\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                            mdm_send_AT_command("AT+WS46=31\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                        }
                    ws46_selected_value = 3;
                }
        }
        if(first_time_ws46_set_flag == TRUE)
        {
            if(mdm_find_response((uint8_t*)sys_mdm_p.type, "ME910C1-WW" )==true)
                {
                	watchdog_pat();
                    mdm_send_AT_command("AT+WS46=28\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
                    ws46_selected_value = 1; //this is used to save ws46, if we didn't disconnect/connect it wont update
                    ws46_selected_value_temp = 1;

                    mdm_send_AT_command("AT+CFUN=4\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
                    DelayMs(100);

                    DelayMs(100);
                    mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                    DelayMs(100);
                    mdm_send_AT_command("AT+CFUN=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                    DelayMs(100);
                    mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                    DelayMs(100);
                    mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                    mdm_send_AT_command("AT+COPS?\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
                    Debug_Output1(0,"AT+COPS? response is: %s", (uint32_t)rx_ans);
                    watchdog_pat();
                }
            first_time_ws46_set_flag = FALSE;
        }

        if(ws46_selected_value != ws46_selected_value_temp)
        {
            mdm_send_AT_command("AT+CFUN=4\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
            DelayMs(100);

            DelayMs(100);
            mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

            DelayMs(100);
            mdm_send_AT_command("AT+CFUN=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

            DelayMs(100);
            mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

            DelayMs(100);
            mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

            ws46_selected_value_temp = ws46_selected_value;
        }
#endif
		mdm_send_AT_command("AT+WS46?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

		mdm_send_AT_command("AT#WS46?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

		mdm_send_AT_command("AT#WS46=0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

		mdm_send_AT_command("AT#WS46?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

#if	0
		mdm_send_AT_command("AT+CEDRXS?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
		//at#cedrxs=1,4,"0101","1111"
		//PTW=20,48, eDRXcycle=81,92

		//at#cedrxs=1,1,"0101","0011"
		//PTW=5 sec, eDRXcycle=81,92
		mdm_send_AT_command("AT+CEDRXS=1,1,0101,0011\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
		mdm_send_AT_command("AT+CEDRXS=?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
#endif	
	}
}


/**************************************************************************/
//! Modem initialization, it checks SIM presence, signal strength, GSM and
//! GPRS network registration status
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t mdm_init()
{
	uint8_t        result = MDM_ERR_NONE, i;
	////Debug_TextOut(0, "mdm_init"); //@r just added for testing
	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
	{
		return result;
	}
	else
	{
        //28-05-2013: mdm_init redesigned to avoid unnecessary commands from every modem re-initialization sequence
		for (i=0; i<5; i++)
		{
			result =  mdm_send_AT_command("AT+CPIN?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);

			if ( result != MDM_ERR_NONE || (mdm_find_response(rx_ans, "+CPIN: READY") == FALSE) )
			{
				result = MDM_ERR_CMD_FAILED;
			}
			else
			{
				diag_text_Screen("SIM READY", FALSE, FALSE);
				break;
			}
			//watchdog_pat();
		}

		if (result != MDM_ERR_NONE)
		{
			diag_text_Screen("SIM FAIL", FALSE, FALSE);
			return MDM_ERR_INIT_FAILED;
		}

		sys_mdm_p.signal_strength = mdm_get_signal_strength();

		if ( glIn_diag_mode == TRUE )
		{
			liberty_sprintf( (char*)glTemp_buf,"Sig Q=%d", sys_mdm_p.signal_strength );
			diag_text_Screen(glTemp_buf, FALSE, FALSE);
		}

		if (mdm_enable_unsolicited_reg_results() != MDM_ERR_NONE)
		{
			return MDM_ERR_INIT_FAILED;
		}

		if (mdm_send_AT_command("AT+CGREG=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100) != MDM_ERR_NONE)
		{
			return MDM_ERR_INIT_FAILED;
		}

		if (mdm_send_AT_command("AT+CGREG=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100) != MDM_ERR_NONE)
		{
			return MDM_ERR_INIT_FAILED;
		}

		mdm_nw_reg_check();    // check if registered to GSM and GPRS

		mdm_fetch_SPN();  //This is to check Apn automatically -- 21/02/2022 -- VT
		Debug_TextOut( 0, "MDM Init DONE" );

		return result;
	}
}

/**************************************************************************/
//! Modem initialization function to be accessed by other modules, it
//! does all modem initializations internally including apn assignment
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t telit_init()
{
	uint8_t        result = MDM_ERR_NONE;
	uint8_t i;
	uint8_t		   mdm_init_retries =2;//it was declared outside and used only within this function, so placed it here //vinay


	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
	{
		return result;
	}
	else
	{
		if(mdm_comm_status < 1)
		{
			//Debug_TextOut(2, "telit_init-01");
			for( i = 0; i <mdm_init_retries; i++ )
			{
				//Debug_TextOut(2, "telit_init-02");
				if(FIRST_COMM == TRUE)//03-06-2013
				{
					FIRST_COMM = FALSE;    //0510
					mdm_init_once_per_bootup();
					//Debug_TextOut(2, "telit_init-03");
#ifdef ENABLE_SMS_MODE
					Init_SMS_Mode();
#endif
				}
				//if(Get_HW_Version() != MPB_LIBERTY_1_0C)	//TODO: Avoid calling this for CDMA Modem
//				if ((CC_Auth_Entered == FALSE) && (do_CC_OLT == FALSE))	//Avoid Calling this for OLT
				{
					result = mdm_init();
					//Debug_TextOut(2, "telit_init-04");
				}

				if(result == MDM_ERR_NONE)
				{
#ifdef Enable_Reduce_CC_Processing_Delay //this is added to just during telit init try to open socket, just added to test CC Processing Delay
					//@r start
					if(modem_init_to_process_cc_transaction)
					{
						modem_init_to_process_cc_transaction = 0;
						Debug_TextOut( 0, "during init open socket\r\n" ); //@r just added for testing
						result = mdm_send_AT_command(cmd_context_act, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
						DelayMs(100);

						sys_mdm_p.signal_strength = mdm_get_signal_strength();
						DelayMs(100);

						sprintf ((char*) cmd_send, "AT#SD=1,1,%d,\"%s\",%d,%d,0,0,0\r", glSystem_cfg.UDP_server_port, (char*)glSystem_cfg.UDP_server_ip, 0 , 8181);//LOB:22/12/2015
						result = mdm_send_AT_command( (char*) cmd_send, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT * 1200);

						if( result != MDM_ERR_NONE )
						{
							glComm_failure_count++;    //increment failure count
							mdm_comm_status = 3; // try to activate context again
							Debug_TextOut( 0, "soc open fail" );
							return GPRS_COMMERR_SOCK_OPEN_ERR;
//							if(glComm_failure_count > glSystem_cfg.Queue_failure_count)	//Error Recovery for continuous modem failures
//							{
//								Modem_Recovery_Loop();
//							}
						}
						else
						{
							if((mdm_find_response((uint8_t*)rx_ans, "CONNECT" ) == true)||
									(mdm_find_response((uint8_t*)rx_ans, "CONN" ) == true)||
									(mdm_find_response((uint8_t*)rx_ans, "NN" ) == true))//// VT
							{
								DelayMs(100);
								Debug_TextOut( 0, "CONNECTED WITH SERVER\r\n" );
							}
							else
							{
								DelayMs(2000);
							}
							//diag_text_Screen( "SocOpnd", FALSE );
							DO_IP_SYNC = TRUE;
							mdm_comm_status = 5;    // socket opened
							//Debug_TextOut( 0, "soc open success during telit init\r\n" ); //@r just added for testing
							return SOCERR_NONE;
						}
					}
					//@r end
#endif
					break;
				}
			}

			if(result != MDM_ERR_NONE)    // Failed 3 times to initialise, so reset the modem and try again
			{
				result = telit_reinit( );
				//Debug_TextOut(2, "telit_init-05");
			}
		}

		if( result == MDM_ERR_NONE )
		{
			/*  P2IFG = 0x00;    //2103: PBC  // TODO
        P2IE |= 0x00;    //2103: PBC
        P2IES |=0x04;    //2103: PBC
        P2IE |= 0x04;    //2103: PBC
			 */

			//diag_text_Screen("N/W Regd", FALSE);


			if(mdm_comm_status < 2)
			{
				mdm_comm_status = 1;
			}

			if( ( result == MDM_ERR_NONE ) && (mdm_comm_status >= 1) )
			{
				//Debug_TextOut(2, "telit_init-06");
				if(mdm_comm_status < 3)
				{
					mdm_comm_status = 2;

					result = telit_set_pdpcontext(glSystem_cfg.apn);

					if( result == MDM_ERR_NONE )
					{
						mdm_comm_status = 4;
						////  send_diag_text_to_MB( glIn_diag_mode, 1, "PDPC Actd" );
						//Debug_TextOut(2, "telit_init-07");
					}
					else
					{
						////   send_diag_text_to_MB( glIn_diag_mode, 1, "PDPC Act Fail" );
					}
				}
			}
			//Debug_TextOut(2, "telit_init-08");
		}

		return result;
	}
}


/**************************************************************************/
//! Sets and Opens a PDP Context for the modem, this is where apn assignment
//! is done
//! \param uint8_t * pointer to apn(Access Point Name)
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t telit_set_pdpcontext( uint8_t * apn )
{
	uint8_t   result, k,i;
	//watchdog_pat();
	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
	{
		return MDM_ERR_NONE;
	}
	else
	{
		//This is to check which Firmware image selected, change accordingly based on apn used
		result = mdm_send_AT_command("AT#FWSWITCH?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);

		if (result == MDM_ERR_NONE)
		{
			if((mdm_find_response(rx_ans, "#FWSWITCH: 2") == TRUE) && (mdm_find_response(glSystem_cfg.apn, "vzwinternet") == TRUE))
			{
				//Debug_TextOut(2, "FWSWITCH Changing to 1" );
				mdm_send_AT_command("AT#FWSWITCH=1,1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
				for(k=0; k<12; k++)
				{
					watchdog_pat();
					DelayMs(5000);
				}
			}
			else if((mdm_find_response(rx_ans, "#FWSWITCH: 2") == TRUE) && (mdm_find_response(glSystem_cfg.apn, "m2m.com.attz") == TRUE))
			{
				//Debug_TextOut(2, "FWSWITCH Changing to 0" );
				mdm_send_AT_command("AT#FWSWITCH=0,1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
				for(k=0; k<12; k++)
				{
					watchdog_pat();
					DelayMs(5000);
				}
			}
			else if((mdm_find_response(rx_ans, "#FWSWITCH: 2") == FALSE) && ((mdm_find_response(glSystem_cfg.apn, "www") == TRUE)
					|| (mdm_find_response(glSystem_cfg.apn, "m2m.t-mobile.com") == TRUE) || (mdm_find_response(glSystem_cfg.apn, "internet.sierrawireless.com") == TRUE)
					|| (mdm_find_response(glSystem_cfg.apn, "METERSPRO02.DUNCAN-USA.COM") == TRUE)))
			{
				//Debug_TextOut(2, "FWSWITCH Changing to 2" );
				mdm_send_AT_command("AT#FWSWITCH=2,1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
				for(k=0; k<12; k++)
				{
					watchdog_pat();
					DelayMs(5000);
				}
			}
		}
		else
		{
			//Debug_TextOut(2, "No Response for AT#FWSWITCH?-02" );
		}

        /***********************TO SAVE APN ,IF GOT CHANGED*****************************/
        mdm_send_AT_command("AT#BND?\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
        Debug_Output1(0,"AT#BND? response is: %s", (uint32_t)rx_ans);

        result = mdm_send_AT_command("AT+CGDCONT?\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*100); //timeout was 10 before now inc to 100 for ME910 to work in US
        Debug_Output1(0,"AT+CGDCONT? response is: %s", (uint32_t)rx_ans);

        liberty_sprintf( (char*)cmd_send, "1,\"%s\",\"%s\"", "IP", (char*)apn );
        if ( result == MDM_ERR_NONE)
        {
            if(mdm_find_response((uint8_t*)rx_ans, (char*)cmd_send  ) == true)  //// VT
            {
                 Debug_Output1(0,"APN ALREADY SET IN MODEM",0);
            }
            else
            {
                Debug_Output1(0,"APN GOING TO SET IN MODEM",0);

                mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                DelayMs(100);

                mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                DelayMs(100);

                mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                DelayMs(100);

                liberty_sprintf( (char*)cmd_send, "AT+CGDCONT=1,\"%s\",\"%s\"\r", "IP", (char*)apn );

                result = mdm_send_AT_command( (char*) cmd_send, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100 );

                mdm_send_AT_command("AT+CFUN=4\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                DelayMs(100);

                mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                DelayMs(100);

                mdm_send_AT_command("AT+CFUN=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                DelayMs(100);

                mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                DelayMs(100);

                mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                DelayMs(100);

                mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                DelayMs(100);

                result = mdm_send_AT_command("AT+CGDCONT?\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*100); //timeout was 10 before now inc to 100 for ME910 to work in US
                Debug_Output1(0,"AT+CGDCONT? NEW SET response is: %s", (uint32_t)rx_ans);
            }
        }
        //////////////////////////////////
        APN_Assigned_ignore_steps = TRUE;

        for(i=0;i<20;i++)
        {
            mdm_send_AT_command("AT+COPS?\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
            Debug_Output1(0,"AT+COPS? response is: %s", (uint32_t)rx_ans);

            if(mdm_find_response((uint8_t*)rx_ans, "+COPS: 0,0" )==true)
            {
                break;
            }
            DelayMs(500);
            watchdog_pat();
        }

        mdm_init_once_per_bootup();

        APN_Assigned_ignore_steps = FALSE;
        //////////////////////////////////
        /*****************************************************/
		result = mdm_send_AT_command( (char*) cmd_send, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20 );

		result = mdm_send_AT_command(cmd_soc_cfg_evt, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT);

		result = mdm_send_AT_command(cmd_autocontxt_act_ext, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT);

		result = mdm_send_AT_command("AT#SCFGEXT2=1,1\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT);
		DelayMs(500);
		//watchdog_pat();
		result = mdm_send_AT_command(cmd_autocontext_act, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
		DelayMs(500);

		//No needed Now --VT
		////result = mdm_send_AT_command(cmd_context_act, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
		//watchdog_pat();
		DelayMs(500);

		// mdm_send_AT_command( cmd_show_ipaddr, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);    // 0806

		//if(result == MDM_ERR_NONE)
		{
			mdm_send_AT_command( "AT&W0\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);    // 0806

			mdm_send_AT_command( "AT&P0\r", rx_ans,    sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);    //0806

			mdm_send_AT_command("ATE0&K0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);

			mdm_comm_status = 3;
		}
		return result;
	}
}
#if 0
/**************************************************************************/
/*  Name        : telit_gprs_detach                                       */
/*  Parameters  : void                                                    */
/*  Returns     : int                                                     */
/*  Function    : Detach gprs and shut down modem with a single command   */
/*------------------------------------------------------------------------*/
int16_t telit_gprs_detach()
{
	int16_t    result, i;

	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
	{
		return MDM_ERR_NONE;
	}
	else
	{
		for ( i = result = 0;  i < 1;  ++i )
		{
			result = mdm_send_AT_command( "AT#SHDN\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20 );

			if ( result == MDM_ERR_NONE )
			{
				break;
			}
		}

		diag_text_Screen( "MDM Shut", FALSE, FALSE );

		return result;
	}
}
#endif

/**************************************************************************/
//! Does GPRS registration, sets socket configuration and Opens a Socket
//! Connection with Server, it uses the Server IP, Port from meter's
//! configuration file directly
//! \param uint8_t communication type (needed to act differently for OLT packets)
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t telit_sock_open( /*Socket * sock_p,*/ uint8_t communication_type )
{
	int16_t        	fail_count = -1;
	uint16_t		j;
	uint8_t        	ret        = SOCERR_NONE;
	uint8_t        	result     = MDM_ERR_NONE;
	uint8_t        	i;

	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
	{
		return ret;
	}
	else
	{
		if(MDM_AWAKE == FALSE)
		{
			telit_power_on();
		}
		else
		{
			telit_wakeup_sleep(FALSE);
		}
		if(mdm_comm_status < 5)    // if socket is not opened already//26-07-12
		{
			memset( (char*) cmd_send, 0, sizeof(cmd_send) );
            DelayMs(100);

			if(mdm_send_AT_command("AT\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*10) == TRUE)
			{
				mdm_send_data( (uint8_t*)"+++", 3, 40 );
				mdm_send_AT_command( "AT\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200) ;
				mdm_send_AT_command( "AT#SH=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200) ;
			}
            result = mdm_send_AT_command("AT&P0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
            result = mdm_send_AT_command("ATE0&K0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
            result = mdm_send_AT_command("AT+CFUN=5\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);//1506    ///JBENDOR-2012.03.19
			result = mdm_send_AT_command("AT#SGACT=1,0\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);//25-09-12
			result = mdm_send_AT_command("AT+CGATT=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*500);
			result = mdm_send_AT_command("AT#SGACT=1,1\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);//25-09-12
			DelayMs(2000);
			//watchdog_pat();

			if(Parking_Clock_Refresh[Current_Space_Id] == PARKING_CLOCK_RESET_MODE) //This is added to refresh clock for new payments
			{
				//Debug_TextOut(0,"Parking Clock");
				Update_Parking_Clock_Screen();
				Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_STOP_MODE;
			}

			diag_text_Screen( "Chk Auto N/W Actvn", FALSE, FALSE );

			for ( j=0; j < (45*1000); j++ )	//increased from 20 to 45, Sierra SIM takes 3 minutes to register
			{
				DelayMs(2);
				if((j%1000)==0)
				{
					if( mdm_send_AT_command(cmd_context_check, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES,(DEFAULT_TIME_OUT*200)) == MDM_ERR_NONE )
					{
						if ( mdm_find_response(rx_ans, "#SGACT: 1,1" ) == TRUE )
						{
						    memcpy(sys_mdm_p.COPS,0,30);
						    mdm_send_AT_command("AT+COPS?\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
						    memcpy(sys_mdm_p.COPS,rx_ans,30);
							mdm_comm_status = 4;
							break;
						}
					}
					mdm_send_AT_command("AT#SGACT=1,1\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);//15-03-21 ADDED FOR SOCKET ACTIVATION RETRY -- VT
					mdm_comm_status = 0;
					Last_Comms_Success_Flg = FALSE;
					//watchdog_pat();

					if(Parking_Clock_Refresh[Current_Space_Id] == PARKING_CLOCK_RESET_MODE) //This is added to refresh clock for new payments
					{
						//Debug_TextOut(0,"Parking Clock");
						Update_Parking_Clock_Screen();
						Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_STOP_MODE;
					}
				}

				if((j%6000)==0)
                {
						if(mdm_send_AT_command("AT\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*10) == TRUE)
						{
							mdm_send_data( (uint8_t*)"+++", 3, 40 );
							mdm_send_AT_command( "AT\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200) ;
							mdm_send_AT_command( "AT#SH=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200) ;
						}
						result = mdm_send_AT_command("AT&P0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);

						result = mdm_send_AT_command("ATE0&K0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);

                		mdm_send_AT_command("AT#SGACT=1,0\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);

                		liberty_sprintf( (char*)cmd_send, "AT+CGDCONT=1,\"%s\",\"%s\"\r", "IP", (char*)glSystem_cfg.apn );

                		result = mdm_send_AT_command( (char*) cmd_send, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20 );

                		result = mdm_send_AT_command(cmd_soc_cfg_evt, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT);

                		result = mdm_send_AT_command(cmd_autocontxt_act_ext, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT);

                		result = mdm_send_AT_command("AT#SCFGEXT2=1,1\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT);
                		DelayMs(500);
                		//watchdog_pat();
                		result = mdm_send_AT_command(cmd_autocontext_act, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
                		DelayMs(500);
                		result = mdm_send_AT_command(cmd_context_act, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                		//watchdog_pat();
                		DelayMs(500);
                }

			}

			if((mdm_comm_status != 4) && (communication_type != GPRS_COMM_OLT_AES))
			{
				diag_text_Screen( "Auto N/W Regn Fail", Meter_Reset, FALSE );
				nw_operator_name_id_check();//try back up network now
				Read_Mdm_diag_data();
				if ( fail_count  < 0 || fail_count > MAX_SOCKOPN_FAIL_COUNT_4_MDM_INIT )
				{
					//LOB_Meter_details();
					//Display_Meter_Info();
					//LOB_try_alt_server();
					//result = telit_init();
					fail_count = 0;
				}
			}
			else if(mdm_comm_status == 4)
			{
				Read_Mdm_diag_data();
				diag_text_Screen( "N/W Regn Success", Meter_Reset, FALSE );
			}

            sys_mdm_p.signal_strength = mdm_get_signal_strength();
            DelayMs(100);

			sprintf ((char*) cmd_send, "AT#SD=1,1,%d,\"%s\",%d,%d,0,0,0\r", glSystem_cfg.UDP_server_port, (char*)glSystem_cfg.UDP_server_ip, 0 , 8181);//LOB:22/12/2015

			for ( i = 0; i < TELIT_DRV_SOC_OPN_CMD_RETRIES; ++ i )
			{
				result = mdm_send_AT_command( (char*) cmd_send, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT * 1200);

				if ( result == MDM_ERR_NONE )
				{

					if((mdm_find_response((uint8_t*)rx_ans, "CONNECT" ) == true)||
							(mdm_find_response((uint8_t*)rx_ans, "CONN" ) == true)||
							(mdm_find_response((uint8_t*)rx_ans, "NN" ) == true))//// VT
					{
						DelayMs(100);
						Debug_TextOut( 0, "CONNECTED WITH SERVER\r\n" );
					}
					else
					{
						DelayMs(2000);
					}

					break;
				}

				//Debug_Output2( 5, "SO Retry #%d SQ %d", i, mdm_get_signal_strength() );

				DelayMs(500);
			}

			if( result != MDM_ERR_NONE )
			{
				mdm_comm_status = 3; // try to activate context again

				Debug_TextOut( 0, "soc open fail" );

				ret = GPRS_COMMERR_SOCK_OPEN_ERR;
			}
			else
			{
				DO_IP_SYNC = TRUE;
				mdm_comm_status = 5;    // socket opened
				ret = SOCERR_NONE;
			}
		}
		else    // restore the socket if it is already open
		{
			ret = SOCERR_BUSY;
		}
		if ( ret == SOCERR_NONE )
			fail_count = 0;
		else
			fail_count ++;
		//Debug_Output1( 0, "mdm_comm_status=%d", mdm_comm_status );
		return ret;
	}
}

/**************************************************************************/
//! Closes a Socket Connection with Server if specified, otherwise just puts
//! the modem to sleep, Uses the global variable glClose_socket inside the function
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t telit_sock_close()
{
	uint8_t            result = SOCERR_NONE;//, i;
	uint8_t            close_timeout = 0;//, close_retries = 3;

	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
	{
		return result;
	}
	else
	{
		//sock_set_state(sock_p, SOC_STATE_CLOSING );
		//P2IE=0;//08-01-13
		//Debug_Output1( 0, "mdm_comm_status=%d", mdm_comm_status );
		if(glClose_socket == TRUE)    //3009:test
		{
			telit_wakeup_sleep(FALSE);

			glClose_socket = FALSE;    //3009

			if(mdm_comm_status > 4)
			{
				//for ( i = 0; i < close_retries; i ++ )
				{
					DelayMs(2000);
					//watchdog_pat();

					close_timeout = 40;    //0510: changed from 20 to 40

					result = mdm_send_data( (uint8_t*)"+++", 3, close_timeout );    //2209
					//Debug_TextOut( 0, (const char*) glMdmUart_recv_buf );

					if(result == MDM_ERR_NONE)    //0510
					{        //0510
						mdm_comm_status = 4;    //0510

						//if(communication_type != GPRS_COMM_OLT_XXTEA)
						{
							result = mdm_send_AT_command( "AT#SH=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200) ;

							if ( result == MDM_ERR_NONE )
							{
								//Debug_TextOut( 0, "soc closed" );
								// mdm_comm_status = 4;    //0510
								// result = mdm_send_AT_command( cmd_soc_cfg_evt, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT) ;//0710
								//break;
							}
							else
							{
								Debug_TextOut( 0, "soc close fail" );
							}
						}
					}    //0510
				}

				if(Parking_Clock_Refresh[Current_Space_Id] == PARKING_CLOCK_RESET_MODE) //This is added to refresh clock for new payments
				{
					//Debug_TextOut(0,"Parking Clock");
					Update_Parking_Clock_Screen();
					Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_STOP_MODE;
				}

				//Debug_Output1( 5, "+++ resp %d",glMdmUart_bytes_recvd );

				if (/*( i == 3 ) &&*/ (result != MDM_ERR_NONE))
				{
					Debug_TextOut( 0, "Reset CLS" );
					reinit_telit_Modem();// changed as just initing the UART did not work properly - 1jul11
				}
			}
		}
		else
		{
			telit_wakeup_sleep(TRUE);
		}
		return result;
	}
}

/**************************************************************************/
//! Used to send packets to server via modem
//! \param pointer to the data packet to be sent
//! \param uint16_t size of the data packet to be sent
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t telit_sock_send( uint8_t * data_p, uint16_t data_size )
{
	uint8_t ret = SOCERR_NONE;
//	Debug_TextOut( 0, "telit_sock_send ..." ); //@r just added for testing
//	uint8_t i = 0;
//	if(CC_Transaction_Retry_Request_Interval)  //@r complete if statement just added for testing
//	{
//		Debug_Output1(0,"telit_sock_send (data_size) : %d",data_size);
//		for(i=0;i<data_size;i++)
//		{
//			Debug_Output2(0,"telit_sock_send data_p[%d] : %x",i, *data_p++);
//		}
//		for(i = 0; i<data_size; i++)
//		{
//			*data_p--;
//		}
//	}

	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
	{
		return ret;
	}
	else
	{
		//    sock_set_state( sock_p, SOC_STATE_SENDING );    //0710

		ret = mdm_send_data( data_p, data_size, 0 );

		if ( ret != MDM_ERR_NONE )
			ret = SOCERR_SEND_SOME;
		else
			ret = SOCERR_NONE;

		//    sock_set_state( sock_p, SOC_STATE_SENT );    //0710

		return ret;
	}
}
#if 0
/**************************************************************************/
//! Used to Receive packets from server via modem
//! \param uint8_t * pointer to the data packet to be sent
//! \param uint16_t size of the data packet to be sent
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t telit_sock_receive( uint8_t *buf,
                            uint16_t buf_len,
                            uint16_t * recvd,
                            uint8_t timeout,
                            uint8_t (*call_bk)( uint16_t rcv_byts, uint8_t *glMdmUart_recv_buf ),
                            uint8_t sock_clos_state_at_start )
{
	uint16_t       i=0;//, min_V=9999;//, lcl_dcell_vltg=0;
	//float 		   MSP_Vbat=0;

	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
	{
		return SOCERR_NONE;
	}
	else
	{
		memset( buf, 0, buf_len );

		for( i = 0; i < (timeout*100); i++ )    // wait till started to receive response from server
		{
			DelayMs(1);
			if ( call_bk( glMdmUart_bytes_recvd, glMdmUart_recv_buf ) == true )    // break when start getting bytes
			{
				break;
			}
		}

		if ( glMdmUart_bytes_recvd > 0 )
			memcpy( buf, glMdmUart_recv_buf, glMdmUart_bytes_recvd );

		* recvd = glMdmUart_bytes_recvd;

		if ( i >= (timeout*100) )
		{
			* recvd = 0;    //0908: avoid writing previous bytes_recvd value
			return SOCERR_RECV_TIMEOUT;
		}

		return SOCERR_NONE;
	}
}
#endif
/**************************************************************************/
//! Send and Execute AT commands to modem
//! \param char * pointer to the AT command string to be sent
//! \param uint8_t * pointer to the buffer where response will be given
//! \param uint16_t size of the buffer where response will be given
//! \param uint8_t maximum number of attempts AT command will be tried in case of failure response
//! \param uint16_t timeout milliseconds to wait for each attempt of AT command
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t mdm_send_AT_command( const char * cmd_text_ptr,
                             uint8_t *    resp_buf_ptr,
                             uint16_t     resp_buf_sz,
                             uint8_t      max_retries,
                             uint16_t     timeout_ms )
{
	uint16_t           cmd_lng = (int16_t) strlen(cmd_text_ptr);
	uint8_t            result = MDM_ERR_NONE;
	uint16_t             i, j;
	uint8_t 		  bytes_compare=0;

	memset ( resp_buf_ptr, 0, resp_buf_sz );

	if ((mdm_comm_status == 5)&&(FD_BackGround_Occuring_Flag==FALSE))
	{
		return MDM_ERR_CMD_FAILED;    //0510
	}

	/*This is newly added to inform MSP as AT command to status check not a server packet */
	Special_Test_AT_Commands((uint8_t*) cmd_text_ptr,  cmd_lng);

	DelayMs(10);

	sys_mdm_p.cmd_sent = true;

	if ( Debug_Verbose_Level > 4 ) //if you comment it then we can see response from AT cmds //Vinay
	{
		if ( glMdmUart_bytes_recvd > 0 )
		{
			glMdmUart_recv_buf[glMdmUart_bytes_recvd] = 0;
			liberty_sprintf( (char*) glTemp_buf2, "%s", glMdmUart_recv_buf );
			Debug_TextOut( 0, (const char*) glTemp_buf2 );
		}

		liberty_sprintf( (char*) glTemp_buf2, "%s", cmd_text_ptr );
		Debug_TextOut( 0, (const char*) glTemp_buf2 );
	}

	glMdmUart_bytes_recvd = 0;
	memset( glMdmUart_recv_buf, 0, sizeof( glMdmUart_recv_buf ) );

	for ( i = 0;  i < max_retries;  ++i )
	{
		result = Modem_out_UART( (uint8_t*) cmd_text_ptr,  cmd_lng );

		for ( j = 0;  j < timeout_ms;  ++j )    // wait till started to receive response from server
		{
			if((j % 2000) == 0)
			{
				if(mdm_find_response((uint8_t*)cmd_text_ptr, "AT+COPS=?" ) == true)
				{
					if(Parking_Clock_Refresh[Current_Space_Id] == PARKING_CLOCK_RESET_MODE) //This is added to refresh clock for new payments
					{
						//Debug_TextOut(0,"Parking Clock");
						Update_Parking_Clock_Screen();
						Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_STOP_MODE;
					}
				}
			}

			DelayMs(1);
			if(mdm_find_response((uint8_t*)cmd_text_ptr, "AT#SPN" ) == true)
			{
				bytes_compare=10;
			}
			else
			{
				bytes_compare=0;
			}

			if ( glMdmUart_bytes_recvd > bytes_compare )
			{
				DelayMs(20);
				i = max_retries;
				if(mdm_find_response((uint8_t*)cmd_text_ptr, "AT+CGDCONT" ) == true)//18-11-2013:DPLIBB-589
				{
					DelayMs(100);
				}
				break;
			}
		}
	}

	for ( i = 0; i < glMdmUart_bytes_recvd; i++ )
	{
		glMdmUart_recv_buf[i] = glMdmUart_recv_buf[i] & 0x7F;
	}

	if ( glMdmUart_bytes_recvd <= 0 )
	{
		result = MDM_ERR_CMD_FAILED;
		//Debug_TextOut( 0, "AT FAIL" );
	}
	else
	{
		//Debug_TextOut( 0, "AT OK" );
		if (Debug_Verbose_Level > 4)  //if you comment it then we can see response from AT cmds //Vinay
		{
			memcpy( glTemp_buf2, glMdmUart_recv_buf, glMdmUart_bytes_recvd );
			glTemp_buf2[glMdmUart_bytes_recvd] = '\0';
			Debug_TextOut( 0, (const char*)glTemp_buf2 );
		}

		if (mdm_find_response (glMdmUart_recv_buf, "ERROR" ) == TRUE )
			result = MDM_ERR_CMD_FAILED;
		else if ( resp_buf_ptr != NULL )
			memcpy ( resp_buf_ptr, glMdmUart_recv_buf, resp_buf_sz );
	}

	sys_mdm_p.cmd_sent = false;

	Modem_Test_Flag = FALSE;

	return result;
}

/**************************************************************************/
//! Send data to server
//! \param char * pointer to the data packet to be sent
//! \param uint16_t size of the data packet to be sent
//! \param uint16_t timeout to wait for before declaring a success or failure
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t mdm_send_data( uint8_t * data_p, uint16_t data_sz, uint16_t timeout )
{
	uint8_t        result = MDM_ERR_NONE;
	uint8_t     i;

	Special_Test_AT_Commands((uint8_t*) data_p,  data_sz);
//	if(CC_Transaction_Retry_Request_Interval)  //@r complete if statement just added for testing
//	{
//		Debug_Output1(0,"mdm_send_data (data_sz) : %d",data_sz);
//		for(i=0;i<data_sz;i++)
//		{
//			Debug_Output2(0,"data_p[%d] : %x",i, *data_p++);
//		}
//		for(i = 0; i<data_sz; i++)
//		{
//			*data_p--;
//		}
//	}

	if((mdm_comm_status!=5)||((data_p[0]==0x41)&&(data_p[1]==0x54)))
	{
		//for(i=0; i<data_sz; i++)
		//	Debug_Output1(0, "data:%x", data_p[i]);
		Modem_Test_Flag = FALSE;
		return MDM_ERR_SEND_DATA_FAILED;
	}
	else
	{
		memset( glMdmUart_recv_buf, 0, sizeof( glMdmUart_recv_buf ) );
		memset( rx_ans, 0, sizeof( rx_ans ) );
		glMdmUart_bytes_recvd = 0;

		DelayMs(500);    //1710:shruthi

		result = Modem_out_UART( (uint8_t*) data_p, data_sz );

		if ( result != MDM_ERR_NONE)
		{
			Modem_Test_Flag = FALSE;
			return MDM_ERR_SEND_DATA_FAILED;
		}

		//sprintf( (char*)glTemp_buf,"Bytes Xmit %d", data_sz );
		//send_diag_text_to_MB( glIn_diag_mode, 3, (const char*) glTemp_buf );

		if (data_sz == 3)    //0509: it is an escape sequence
		{
			for ( i = 0; i < timeout; i++ )    // wait till started to receive response from server
			{
				if ( glMdmUart_bytes_recvd > 0 )    // break when start getting bytes
				{
					break;
				}
				DelayMs(100);
			}

			if ( glMdmUart_bytes_recvd == 0 )
			{
				Modem_Test_Flag = FALSE;
				return MDM_ERR_SEND_DATA_FAILED;
			}
		}

		Modem_Test_Flag = FALSE;
		return MDM_ERR_NONE;
	}

}

/**************************************************************************/
//! Reads the IMEI number of the Modem
//! \param uint8_t * pointer to the response buffer
//! \param uint16_t size of the response buffer
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t mdm_fetch_IMEI( uint8_t * rx_ans, uint16_t buf_sz )
{
	uint8_t     result, i;
	uint8_t *   rx_ans_loc = rx_ans + 2;

	memset( rx_ans, 0, buf_sz);

	result = mdm_send_AT_command("AT+CGSN\r", rx_ans, buf_sz-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);

	if ( result == MDM_ERR_NONE)
	{
		if(mdm_find_response((uint8_t*)rx_ans, "AT+CGSN" ) == true)
		{
			rx_ans_loc += 8;
		}

		for(i=0; i<buf_sz; i++)
		{
			if((rx_ans_loc[i] == 'O') && (rx_ans_loc[i+1] == 'K'))
			{
				break;
			}
		}

		memset( glTemp_buf, 0, sizeof( glTemp_buf));
		//memcpy( glTemp_buf, rx_ans_loc, 15 );
		//Debug_Output1(0,"IMEI i Value(Debug): %d", i);
		if(i > 6)
			memcpy( glTemp_buf, rx_ans_loc, (i-4));
		liberty_sprintf( (char*)rx_ans,"%s",(char*)glTemp_buf);
		//Debug_Output1(0,"IMEI: %s", (uint32_t)rx_ans);
	}
	return result;
}

/**************************************************************************/
//! Reads the IMSI number of the Modem
//! \param uint8_t * pointer to the response buffer
//! \param uint16_t size of the response buffer
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t mdm_fetch_IMSI( uint8_t * rx_ans, uint16_t buf_sz  )
{
	uint8_t     result, i;
	uint8_t * rx_ans_loc = rx_ans + 2;

	memset( rx_ans, 0, buf_sz );

	result = mdm_send_AT_command("AT+CIMI\r", rx_ans, buf_sz-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
	if ( result == MDM_ERR_NONE)
	{
		if(mdm_find_response((uint8_t*)rx_ans, "AT+CIMI" ) == true)
		{
			rx_ans_loc += 8;
		}

		for(i=0; i<buf_sz; i++)
		{
			if((rx_ans_loc[i] == 'O') && (rx_ans_loc[i+1] == 'K'))
			{
				break;
			}
		}

		memset( glTemp_buf, 0, sizeof( glTemp_buf));
		//memcpy( glTemp_buf, rx_ans_loc, 15);
		//Debug_Output1(0,"IMSI i Value(Debug): %d", i);
		if(i > 6)
			memcpy( glTemp_buf, rx_ans_loc, (i-4));
		liberty_sprintf( (char*)rx_ans,"%s",(char*)glTemp_buf);
		//Debug_Output1(0,"IMSI: %s", (uint32_t)rx_ans); //LNGSIT-876
		//memcpy(sys_mdm_p.IMSI,rx_ans,MODEM_BUFFER_S_SIZE);//11-06-12:need this to be copied to global structure
	}
	return result;
}

/**************************************************************************/
//! Reads the Modem Firmware Revision
//! \param uint8_t * pointer to the response buffer
//! \param uint16_t size of the response buffer
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t mdm_fetch_SW_revision( uint8_t * rx_ans, uint16_t buf_sz )
{
	uint8_t *       rx_ans_loc = rx_ans + 2;

	uint8_t         i, result = mdm_send_AT_command("AT+CGMR\r", rx_ans, buf_sz-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20 );

	if ( result == MDM_ERR_NONE)
	{
		if(mdm_find_response((uint8_t*)rx_ans, "AT+CGMR" ) == true)
		{
			rx_ans_loc += 8;
		}

		for(i=0; i<buf_sz; i++)
		{
			if((rx_ans_loc[i] == 'O') && (rx_ans_loc[i+1] == 'K'))
			{
				break;
			}
		}

		memset( glTemp_buf, 0, sizeof( glTemp_buf) );
		//memcpy( glTemp_buf, rx_ans_loc, 15);
		//Debug_Output1(0,"SW i Value(Debug): %d", i);
		if(i > 6)
			memcpy( glTemp_buf, rx_ans_loc, (i-4));

		//sprintf((char*)rx_ans,"MDM f/w V %s",glTemp_buf);
		//send_diag_text_to_MB( glIn_diag_mode, 1, (const char*)rx_ans );

		liberty_sprintf((char*)rx_ans,"%s",glTemp_buf);
		//Debug_Output1(0,"MDM FW Ver: %s", (uint32_t)rx_ans);
	}

	return result;
}

/**************************************************************************/
//! Reads the Modem's Model ID
//! \param uint8_t * pointer to the response buffer
//! \param uint16_t size of the response buffer
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t mdm_fetch_type( uint8_t * rx_ans, uint16_t buf_sz )
{
	uint8_t *       rx_ans_loc = rx_ans + 2;
	uint8_t            result, i;

	result = mdm_send_AT_command("AT+CGMM\r", rx_ans, buf_sz-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20 );

	if ( result == MDM_ERR_NONE)
	{
		if(mdm_find_response((uint8_t*)rx_ans, "AT+CGMM" ) == true)
		{
			rx_ans_loc += 8;
		}

		for(i=0; i<buf_sz; i++)
		{
			if((rx_ans_loc[i] == 'O') && (rx_ans_loc[i+1] == 'K'))
			{
				break;
			}
		}

		memset( glTemp_buf, 0, sizeof( glTemp_buf) );
		//memcpy( glTemp_buf, rx_ans_loc, 10);
		if(i > 6)
			memcpy( glTemp_buf, rx_ans_loc, (i-4));

		//sprintf( (char*)rx_ans,"MDM Model %s", glTemp_buf );
		//send_diag_text_to_MB( glIn_diag_mode, 1, (const char*) rx_ans );

		liberty_sprintf( (char*)rx_ans,"%s", glTemp_buf );
		//Debug_Output1(0,"MDM Type: %s", (uint32_t)rx_ans);
	}
	return result;
}

/**************************************************************************/
//! Reads the Operator name for the SIM and prints on log
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
#if 0
uint8_t mdm_fetch_SPN()
{
//	uint8_t *       rx_ans_loc = rx_ans + 2;
	uint8_t            result=0;//, i;
	return result;
#if 0
	result = mdm_send_AT_command("AT#SPN\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000); //timeout was 10 before now inc to 100 for ME910 to work in US
	DelayMs(1000);
	if ( result == MDM_ERR_NONE)
	{
		if(mdm_find_response((uint8_t*)rx_ans, "AT#SPN" ) == true)	//// VT
		{
			if(mdm_find_response((uint8_t*)rx_ans, "#SPN:" )==true)	rx_ans_loc += 13;
			else rx_ans_loc += 6;
		}

		for(i=0; i<30; i++)
		{
			if((rx_ans_loc[i] == 'O') && (rx_ans_loc[i+1] == 'K'))
			{
				break;
			}
		}

		memset( glTemp_buf, 0, sizeof( glTemp_buf) );

		//if(mdm_find_response(glSystem_cfg.apn, "sierra" ) == TRUE) //LNGSIT-1037
			//memcpy( glTemp_buf, rx_ans_loc, 15);
		//else
		if(i > 6)
			memcpy( glTemp_buf, rx_ans_loc, (i-4));

		memset(rx_ans, 0, sizeof(rx_ans));
		liberty_sprintf( (char*)rx_ans,"%s", glTemp_buf );
		Debug_Output1(0,"Operator: %s", (uint32_t)rx_ans);
	}
	else
	{
		DelayMs(1000);
	}
	return result;
#endif
}
#endif

uint8_t mdm_fetch_SPN() //Currently this function is used for Auto APN -21/02/2022 -- VT
{
    uint8_t i = 0;
    uint8_t APN_Assigned_got_Reset = 0;
    /*If GPRS Response Received flag true i.e. APN,IP and Port are valid, no need to retry for that*/
    /* If Change to Alternate IP flag set, then avoid Auto DNS */
    if(APN_Assigned == 0)
    {
        APN_Assigned_got_Reset = 1;
        //copy back temp apn to default
        //Added to copy apn settings from config
        memcpy(glSystem_cfg.apn,default_apn,strlen(default_apn)+1);
        memcpy(glSystem_cfg.UDP_server_ip,default_UDP_server_ip,strlen(default_UDP_server_ip)+1);
        glSystem_cfg.UDP_server_port = default_UDP_server_port;

        /*Check Module Connected to which network and assign apn*/
        /*Sending command to receive connected operator and as per operator, assign APN.*/
        mdm_send_AT_command("AT#BND?\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
        Debug_Output1(0,"AT#BND? response: %s", (uint32_t)rx_ans);

        for(i=0;i<5;i++)
        {
            mdm_send_AT_command("AT+COPS?\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
            Debug_Output1(0,"AT+COPS? response: %s", (uint32_t)rx_ans);

            if(mdm_find_response((uint8_t*)rx_ans, "+COPS:" )==true)
            {
                if((mdm_find_response((uint8_t*)rx_ans, "Vodafone IN" )==true)||(mdm_find_response((uint8_t*)rx_ans, "Vi India" )==true))
                {
                    Debug_Output1(0,"Connected to Vodafone IN",0);
                    memcpy(glSystem_cfg.apn, "www", sizeof("www")); //Vodafone
                    memcpy(glSystem_cfg.UDP_server_ip, "64.132.70.171", sizeof("64.132.70.171"));
                    glSystem_cfg.UDP_server_port = 8787;
                    Debug_Output6(0,"Assigning Default APN: %s , IP %s and PORT %d",(uint32_t)glSystem_cfg.apn,(uint32_t)glSystem_cfg.UDP_server_ip, (uint32_t)glSystem_cfg.UDP_server_port,0,0,0);
                    APN_Assigned = 1;
                    break;
                }
                else if((mdm_find_response((uint8_t*)rx_ans, "Jio 4G" )==true))  //IND-JIO
                {
                    Debug_Output1(0,"Connected to Jio 4G",0);
                    memcpy(glSystem_cfg.apn, "jionet", sizeof("jionet")); //Vodafone
                    memcpy(glSystem_cfg.UDP_server_ip, "64.132.70.171", sizeof("64.132.70.171"));
                    glSystem_cfg.UDP_server_port = 8787;
                    Debug_Output6(0,"Assigning Default APN: %s , IP %s and PORT %d",(uint32_t)glSystem_cfg.apn,(uint32_t)glSystem_cfg.UDP_server_ip, (uint32_t)glSystem_cfg.UDP_server_port,0,0,0);
                    APN_Assigned = 1;
                    break;
                }
                else if((mdm_find_response((uint8_t*)rx_ans, "airtel" )==true))
                {
                    Debug_Output1(0,"Operator is airtel", 0);
                    memcpy(glSystem_cfg.apn, "airtelgprs.com", sizeof("airtelgprs.com"));
                    memcpy(glSystem_cfg.UDP_server_ip, "64.132.70.171", sizeof("64.132.70.171"));
                    glSystem_cfg.UDP_server_port = 8787;
                    Debug_Output6(0,"Assigning Default APN: %s , IP %s and PORT %d",(uint32_t)glSystem_cfg.apn,(uint32_t)glSystem_cfg.UDP_server_ip, (uint32_t)glSystem_cfg.UDP_server_port,0,0,0);
                    APN_Assigned = 1;
                    break;
                }
                else if((mdm_find_response((uint8_t*)rx_ans, "Sierra Wireless" )==true)
                        ||(mdm_find_response((uint8_t*)rx_ans, "Sierra" )==true)
                        ||(mdm_find_response((uint8_t*)rx_ans, "sierra" )==true))
                {
                    Debug_Output1(0,"Connected to Sierra Wireless",0);
                    memcpy(glSystem_cfg.apn, "internet.sierrawireless.com", sizeof("internet.sierrawireless.com"));
                    memcpy(glSystem_cfg.UDP_server_ip, "64.132.70.171", sizeof("64.132.70.171"));
                    glSystem_cfg.UDP_server_port = 8787;
                    Debug_Output6(0,"Assigning Default APN: %s , IP %s and PORT %d",(uint32_t)glSystem_cfg.apn,(uint32_t)glSystem_cfg.UDP_server_ip, (uint32_t)glSystem_cfg.UDP_server_port,0,0,0);
                    APN_Assigned = 5;  //for Sierra assign Alternate UDP IP
                    break;
                }
                else if(mdm_find_response((uint8_t*)rx_ans, "AT&T" )==true)
                {
                    Debug_Output1(0,"Connected to AT&T",0);
                    memcpy(glSystem_cfg.apn, "METERSPRO02.DUNCAN-USA.COM", sizeof("METERSPRO02.DUNCAN-USA.COM"));
                    memcpy(glSystem_cfg.UDP_server_ip, "172.25.41.43", sizeof("172.25.41.43"));
                    glSystem_cfg.UDP_server_port = 7878;
                    Debug_Output6(0,"Assigning Default APN: %s , IP %s and PORT %d",(uint32_t)glSystem_cfg.apn,(uint32_t)glSystem_cfg.UDP_server_ip, (uint32_t)glSystem_cfg.UDP_server_port,0,0,0);
                    APN_Assigned = 2;
                    break;
                }
                else if((mdm_find_response((uint8_t*)rx_ans, "T-Mobile" )==true)
                        ||(mdm_find_response((uint8_t*)rx_ans, "t-Mobile" )==true))
                {
                    if(t_mobile_apn_switch_flag)
                    {
                        Debug_Output1(0,"Connected to T-Mobile",0); //t-mobile main UDP
                        memcpy(glSystem_cfg.apn, "m2m.t-mobile.com", sizeof("m2m.t-mobile.com"));
                        memcpy(glSystem_cfg.UDP_server_ip, "172.25.41.43", sizeof("172.25.41.43"));
                        glSystem_cfg.UDP_server_port = 7878;
                        Debug_Output6(0,"Assigning Default APN: %s , IP %s and PORT %d",(uint32_t)glSystem_cfg.apn,(uint32_t)glSystem_cfg.UDP_server_ip, (uint32_t)glSystem_cfg.UDP_server_port,0,0,0);
                        APN_Assigned = 3;
                    }
                    else
                    {
                        Debug_Output1(0,"Connected to Kore",0); //t-mobile Alternate UDP
                        memcpy(glSystem_cfg.apn, "c1.korem2m.com", sizeof("c1.korem2m.com"));
                        memcpy(glSystem_cfg.UDP_server_ip, "64.132.70.171", sizeof("64.132.70.171"));
                        glSystem_cfg.UDP_server_port = 8787;
                        Debug_Output6(0,"Assigning Default APN: %s , IP %s and PORT %d",(uint32_t)glSystem_cfg.apn,(uint32_t)glSystem_cfg.UDP_server_ip, (uint32_t)glSystem_cfg.UDP_server_port,0,0,0);
                        APN_Assigned = 4;
                    }
                    break;
                }
                else if(mdm_find_response((uint8_t*)rx_ans, "Verizone" )==true)
                {
                    Debug_Output1(0,"Connected to Verizone",0);
                    memcpy(glSystem_cfg.apn, "vzwinternet", sizeof("vzwinternet"));
                    memcpy(glSystem_cfg.UDP_server_ip, "64.132.70.171", sizeof("64.132.70.171"));
                    glSystem_cfg.UDP_server_port = 8787;
                    Debug_Output6(0,"Assigning Default APN: %s , IP %s and PORT %d",(uint32_t)glSystem_cfg.apn,(uint32_t)glSystem_cfg.UDP_server_ip, (uint32_t)glSystem_cfg.UDP_server_port,0,0,0);
                    APN_Assigned = 6;
                    break;
                }
                else if(mdm_find_response((uint8_t*)rx_ans, "+COPS: 0,0" )==true)
                {
                    Debug_Output1(0,"Connected to Unknown Network",0);
                    break;
                }
            }
            DelayMs(1000);
        }

        //if(APN_Assigned == 0) //AT+COPS Fails //NEED TO RECHECK FOR ICCID IN SOME CASE SIERRA NETWORK WILL MAKE ISSUE -- VT
        {
            APN_Assigned_got_Reset = 1;
            if(APN_Assigned >= 1) Debug_Output1(0,"+COPS: rcvd Successful, checking from ICCID", 0);
            else Debug_Output1(0,"+COPS: not received , checking from ICCID", 0);
            for(i=0;i<3;i++)
            {
                //if(!ICCID_First_6_Digit_Copied)
                {
                    Debug_Output1(0,"Get ICCID of SIM Card", 0);
                    mdm_fetch_ICCID(sys_mdm_p.ICCID, sizeof(sys_mdm_p.ICCID));
                    if(strlen((char *)sys_mdm_p.ICCID) >= 5)
                    {
                        Debug_Output1(0,"ICCID: %s", (uint32_t)sys_mdm_p.ICCID);
                        memcpy(ICCID_First_6_Digit, sys_mdm_p.ICCID + 6, 7 );
                        Debug_Output1(0,"ICCID_First_6_Digit: %s", (uint32_t)ICCID_First_6_Digit);
                        //ICCID_First_6_Digit_Copied = 1;
                    }
                }

                Debug_Output1(0,"APN not assigned, Unable to get +COPS response", 0);
                Debug_Output1(0,"Check For ICCIDs and try to assign APN, IP, Port", 0);
                if(strstr((const char*)sys_mdm_p.ICCID, (const char*)"899102"))
                {
                    Debug_Output1(0,"Indian N/W SIM BASED ON ICCID", 0);
                    memcpy(glSystem_cfg.apn, "www", sizeof("www")); //Vodafone
                    memcpy(glSystem_cfg.UDP_server_ip, "64.132.70.171", sizeof("64.132.70.171"));
                    glSystem_cfg.UDP_server_port = 8787;
                    Debug_Output6(0,"Assigning Default APN: %s , IP %s and PORT %d",(uint32_t)glSystem_cfg.apn,(uint32_t)glSystem_cfg.UDP_server_ip, (uint32_t)glSystem_cfg.UDP_server_port,0,0,0);
                    APN_Assigned = 1;
                    break;
                    //Assigned_IP_Using_ICCID = 1;
                    //Check_For_Default_APN_DNS = 0;  //Make this 0, When will receive +COPS response, AUTO DNS should work.
                }
                else if((strstr((const char*)sys_mdm_p.ICCID, (const char*)"890103")) ||
                        (strstr((const char*)sys_mdm_p.ICCID, (const char*)"890141")))
                {
                    Debug_Output1(0,"AT&T SIM BASED ON ICCID", 0);
                    memcpy(glSystem_cfg.apn, "METERSPRO02.DUNCAN-USA.COM", sizeof("METERSPRO02.DUNCAN-USA.COM"));
                    memcpy(glSystem_cfg.UDP_server_ip, "172.25.41.43", sizeof("172.25.41.43"));
                    glSystem_cfg.UDP_server_port = 7878;
                    Debug_Output6(0,"Assigning Default APN: %s , IP %s and PORT %d",(uint32_t)glSystem_cfg.apn,(uint32_t)glSystem_cfg.UDP_server_ip, (uint32_t)glSystem_cfg.UDP_server_port,0,0,0);
                    APN_Assigned = 2;
                    break;
                    //Assigned_IP_Using_ICCID = 1;
                    //Check_For_Default_APN_DNS = 0;  //Make this 0, When will receive +COPS response, AUTO DNS should work.
                }
                else if(strstr((const char*)sys_mdm_p.ICCID, (const char*)"890126"))
                {
                    if(t_mobile_apn_switch_flag)
                    {
                        Debug_Output1(0,"T-Mobile SIM BASED ON ICCID",0); //t-mobile main UDP
                        memcpy(glSystem_cfg.apn, "m2m.t-mobile.com", sizeof("m2m.t-mobile.com"));
                        memcpy(glSystem_cfg.UDP_server_ip, "172.25.41.43", sizeof("172.25.41.43"));
                        glSystem_cfg.UDP_server_port = 7878;
                        Debug_Output6(0,"Assigning Default APN: %s , IP %s and PORT %d",(uint32_t)glSystem_cfg.apn,(uint32_t)glSystem_cfg.UDP_server_ip, (uint32_t)glSystem_cfg.UDP_server_port,0,0,0);
                        APN_Assigned = 3;
                    }
                    else
                    {
                        Debug_Output1(0,"Kore SIM BASED ON ICCID",0); //t-mobile Alternate UDP
                        memcpy(glSystem_cfg.apn, "c1.korem2m.com", sizeof("c1.korem2m.com"));
                        memcpy(glSystem_cfg.UDP_server_ip, "64.132.70.171", sizeof("64.132.70.171"));
                        glSystem_cfg.UDP_server_port = 8787;
                        Debug_Output6(0,"Assigning Default APN: %s , IP %s and PORT %d",(uint32_t)glSystem_cfg.apn,(uint32_t)glSystem_cfg.UDP_server_ip, (uint32_t)glSystem_cfg.UDP_server_port,0,0,0);
                        APN_Assigned = 4;
                    }
                    break;
                    //Assigned_IP_Using_ICCID = 1;
                    //Check_For_Default_APN_DNS = 0;  //Make this 0, When will receive +COPS response, AUTO DNS should work.
                }
                else if(strstr((const char*)sys_mdm_p.ICCID, (const char*)"893324"))
                {
                    Debug_Output1(0,"Sierra Wireless SIM BASED ON ICCID", 0);
                    memcpy(glSystem_cfg.apn, "internet.sierrawireless.com", sizeof("internet.sierrawireless.com"));
                    memcpy(glSystem_cfg.UDP_server_ip, "64.132.70.171", sizeof("64.132.70.171"));
                    glSystem_cfg.UDP_server_port = 8787;
                    Debug_Output6(0,"Assigning Default APN: %s , IP %s and PORT %d",(uint32_t)glSystem_cfg.apn,(uint32_t)glSystem_cfg.UDP_server_ip, (uint32_t)glSystem_cfg.UDP_server_port,0,0,0);
                    APN_Assigned = 5;
                    break;
                    //Assigned_IP_Using_ICCID = 1;
                    //Check_For_Default_APN_DNS = 0; //Make this 0, When will receive +COPS response, AUTO DNS should work.
                }
                else if(strstr((const char*)sys_mdm_p.ICCID, (const char*)"891480"))
                {
                    Debug_Output1(0,"Verizone SIM BASED ON ICCID", 0);
                    memcpy(glSystem_cfg.apn, "vzwinternet", sizeof("vzwinternet"));
                    memcpy(glSystem_cfg.UDP_server_ip, "64.132.70.171", sizeof("64.132.70.171"));
                    glSystem_cfg.UDP_server_port = 8787;
                    Debug_Output6(0,"Assigning Default APN: %s , IP %s and PORT %d",(uint32_t)glSystem_cfg.apn,(uint32_t)glSystem_cfg.UDP_server_ip, (uint32_t)glSystem_cfg.UDP_server_port,0,0,0);
                    APN_Assigned = 6;
                    break;
                    //Assigned_IP_Using_ICCID = 1;
                    //Check_For_Default_APN_DNS = 0; //Make this 0, When will receive +COPS response, AUTO DNS should work.
                }
                DelayMs(1000);
             }
        }

        Debug_Output1(0,"Selected APN: %s", glSystem_cfg.apn);
        Debug_Output1(0,"Selected UDP_IP: %s", glSystem_cfg.UDP_server_ip);
        Debug_Output1(0,"Selected UDP_PORT: %d",  glSystem_cfg.UDP_server_port);
    }
    else //Just adding log message, need to remove after testing
    {
        Debug_Output1(0,"Selected APN: %s", glSystem_cfg.apn);
        Debug_Output1(0,"Selected UDP_IP: %s", glSystem_cfg.UDP_server_ip);
        Debug_Output1(0,"Selected UDP_PORT: %d",  glSystem_cfg.UDP_server_port);
        Debug_Output1(0,"GPRS_Response is already Rcvd, Skipping Auto APN, DNS loop ", 0);
    }

    liberty_sprintf((char *)rx_ans,"APN: %s", glSystem_cfg.apn);
    diag_text_Screen( rx_ans, TRUE, FALSE );

    APN_Assigned_ignore_steps = TRUE; //This is load WS46 properly other wise in IND testing it may take more time -- VT -- 21/02/2022

    if((APN_Assigned >= 1)&&(APN_Assigned_got_Reset >= 1))
    {
        liberty_sprintf( (char*)cmd_send, "AT+CGDCONT=1,\"%s\",\"%s\"\r", "IP", (char*)glSystem_cfg.apn );
        mdm_send_AT_command( (char*) cmd_send, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20 );

        mdm_send_AT_command("AT+CFUN=4\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
        DelayMs(100);

        mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
        DelayMs(100);

        mdm_send_AT_command("AT+CFUN=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
        DelayMs(100);

        mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
        DelayMs(100);

        mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
        DelayMs(100);

        mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
        DelayMs(100);
    }

    for(i=0;i<20;i++)
    {
        mdm_send_AT_command("AT+COPS?\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
        Debug_Output1(0,"AT+COPS? response is: %s", (uint32_t)rx_ans);

        if(mdm_find_response((uint8_t*)rx_ans, "+COPS: 0,0" )==true)
        {
            break;
        }
        DelayMs(500);
        watchdog_pat();
    }

    mdm_init_once_per_bootup();

    APN_Assigned_ignore_steps = FALSE;

    return APN_Assigned;
}

/**************************************************************************/
//! Reads the ICCID of the SIM
//! \param uint8_t * pointer to the response buffer
//! \param uint16_t size of the response buffer
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t mdm_fetch_ICCID( uint8_t * rx_ans, uint16_t buf_sz  )
{
	uint8_t     result, i;
	uint8_t * rx_ans_loc = rx_ans + 2;

	memset( rx_ans, 0, buf_sz );

	result = mdm_send_AT_command("AT+CCID\r", rx_ans, buf_sz-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
	if ( result == MDM_ERR_NONE)
	{
		if(mdm_find_response((uint8_t*)rx_ans, "AT+CCID" ) == true)	//// VT
		{
			if(mdm_find_response((uint8_t*)rx_ans, "+CCID:" ) == true)	rx_ans_loc += 15;
			else rx_ans_loc += 7;
		}

		for(i=0; i<buf_sz; i++)
		{
			if((rx_ans_loc[i] == 'O') && (rx_ans_loc[i+1] == 'K'))
			{
				break;
			}
		}

		memset( glTemp_buf, 0, sizeof( glTemp_buf) );
		//memcpy( glTemp_buf, rx_ans_loc, 19);
		if(i > 6)
			memcpy( glTemp_buf, rx_ans_loc, (i-4));
		liberty_sprintf( (char*)rx_ans,"%s",(char*)glTemp_buf );
        Debug_Output1(0,"AT+CCID = response is: %s", (uint32_t)rx_ans);
	}
	return result;
}

/**************************************************************************/
//! String Comparison to identify errors in modem response
//! \param void * pointer to string1
//! \param const char * pointer to string2
//! \return uint8_t result
//! - \b MATCH
//! - \b MISMATCH
/**************************************************************************/
uint8_t mdm_find_response( void * response_buf, const char * string_to_find )
{
	return ( strstr( (char*) response_buf, string_to_find) == NULL ) ? FALSE : TRUE ;
}

/**************************************************************************/
//! Extract CSQ value from received signal strength
//! \param void * pointer to CSQ response string
//! \return int16_t signal strength
/**************************************************************************/
static int16_t mdm_extract_csq_value(uint8_t * rx_ans)
{
	uint8_t     * str;
	uint8_t        substr[3];
	uint8_t        csq_value = 0;

	str = (uint8_t*)strstr( (char*)rx_ans,": " );
	str = str + 2;
	strncpy ((char*)substr,(const char*)str,2);
	substr[2]= '\0';
	csq_value = (int16_t) atoi((char*)substr);

	Debug_Output1(0,"CSQ:%d", csq_value);
	return csq_value;
}

/**************************************************************************/
//! Find network signal strength(CSQ value) using AT command
//! \param void
//! \return uint8_t signal strength
/**************************************************************************/
uint8_t mdm_get_signal_strength()
{
	uint8_t             i;
	uint8_t temp_sig_strength = 0;

	for ( i=0; i < 10;  ++i )
	{
		if (mdm_send_AT_command("AT+CSQ\r\n", rx_ans, GPRS_GEN_BUF_SZ_VSMALL, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200) == MDM_ERR_NONE)
		{
			if ( (mdm_find_response(rx_ans, "+CSQ: 99,99" ) == FALSE) && (mdm_find_response(rx_ans, "+CSQ: 0,0" ) == FALSE) )
			{
				break;
			}
			DelayMs(1000);
		}
		//watchdog_pat();
	}

	temp_sig_strength = mdm_extract_csq_value(rx_ans);
	if(temp_sig_strength == 31)	// for HE910 modem, a double confirmation is needed if CSQ=31
	{
		DelayMs(100);
		if (mdm_send_AT_command("AT+CSQ\r\n", rx_ans, GPRS_GEN_BUF_SZ_VSMALL, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200) == MDM_ERR_NONE)
		{
			temp_sig_strength = mdm_extract_csq_value(rx_ans);
		}
	}
	return temp_sig_strength;
}

/**************************************************************************/
//! Enable unsolicited response parameters using AT command
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t  mdm_enable_unsolicited_reg_results()
{
	if (mdm_send_AT_command("AT+CREG=1\r\n", NULL, 0, DEFAULT_RETRIES, DEFAULT_TIME_OUT * 20) != MDM_ERR_NONE)
	{
		return MDM_ERR_CMD_FAILED;
	}

	if (mdm_send_AT_command("AT+CGEREP=2,0\r\n", NULL, 0, DEFAULT_RETRIES, DEFAULT_TIME_OUT * 20) != MDM_ERR_NONE)
	{
		return MDM_ERR_CMD_FAILED;
	}

	return MDM_ERR_NONE;
}

/**************************************************************************/
//! Check GSM and GPRS registration status
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t mdm_nw_reg_check()
{
	uint8_t     i;
	//uint8_t     * rx_ans_loc;

	for ( i = 0;  i < 5;  ++i )
	{
		if (mdm_send_AT_command("AT+CGREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20) == MDM_ERR_NONE)
		{
			if (mdm_find_response(rx_ans, "+CGREG: 1,1" ) == TRUE )
			{
				break;
			}
			DelayMs(100);
		}
	}

	//rx_ans_loc = rx_ans + 13;

	//memset( glTemp_buf, 0, sizeof( glTemp_buf) );
	//memcpy( glTemp_buf, rx_ans_loc, 11);
	//liberty_sprintf( (char*)rx_ans,"%s",(char*)glTemp_buf );
	//send_diag_text_to_MB( glIn_diag_mode, 3, (const char*)rx_ans );

	if (mdm_send_AT_command("AT+CREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20) == MDM_ERR_NONE)
	{
		//rx_ans_loc = rx_ans + 12;

		//memset( glTemp_buf, 0, sizeof( glTemp_buf) );
		//memcpy( glTemp_buf, rx_ans_loc, 10 );
		//liberty_sprintf( (char*)rx_ans,"%s",(char*)glTemp_buf );
		//send_diag_text_to_MB( glIn_diag_mode, 3, (const char*)rx_ans );
	}

	nw_operator_name_id_check();

	return 0;
}

/**************************************************************************/
//! Read Operator name and then do Manual or Automatic Network Operator selection
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t nw_operator_name_id_check()
{
	uint8_t     result;
	//uint8_t * rx_ans_loc = rx_ans + 2;
	uint16_t i = 0, j = 0, next_char_to_scan = 0, operators_cnt = 0;

	if(mdm_find_response(glSystem_cfg.apn, "sierra" ) == TRUE)
	{
		Debug_TextOut(0,"Manual Operator Selection");
		//mdm_send_AT_command(cmd_att_network_selection, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*300);
	}
	else
	{
		Debug_TextOut(0,"Automatic Network Selection");
		return 0;
	}

	if(Meter_Reset == TRUE)	//Do network search only for the first time
	{
		//mdm_send_AT_command("AT+COPS=2\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
		mdm_send_AT_command("AT+COPS?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);

		memset( rx_ans, 0, sizeof(rx_ans) );

		result = mdm_send_AT_command("AT+COPS=?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*12000);
		if (result == MDM_ERR_NONE)
		{
			if (mdm_find_response(rx_ans, "+COPS:" ) == TRUE )
			{
				for(operators_cnt = 0; operators_cnt < MAX_OPERATORS_ALLOWED; operators_cnt++)
				{
					memset(nw_operator_name_id[operators_cnt].Oper_Name, 0, sizeof(nw_operator_name_id[operators_cnt].Oper_Name));
					memset(nw_operator_name_id[operators_cnt].Oper_ID, 0, sizeof(nw_operator_name_id[operators_cnt].Oper_ID));
					//Fill One Operator Name
					for(i=next_char_to_scan; i<sizeof(rx_ans); i++)
					{
						if(rx_ans[i] == 0x22)
						{
							for(j=0; j<sizeof(nw_operator_name_id[operators_cnt].Oper_Name); j++)
							{
								if((rx_ans[i+j+1] == 0x22) && (j>0))
								{
									next_char_to_scan = i+j+2;
									break;
								}
								nw_operator_name_id[operators_cnt].Oper_Name[j] = rx_ans[i+j+1];
							}
							break;
						}
					}
					//Fill One Operator ID
					for(i=next_char_to_scan; i<sizeof(rx_ans); i++)
					{
						if(rx_ans[i] == 0x22)
						{
							for(j=0; j<sizeof(nw_operator_name_id[operators_cnt].Oper_ID); j++)
							{
								if((rx_ans[i+j+1] == 0x22) && (j>0))
								{
									next_char_to_scan = i+j+2;
									break;
								}
								nw_operator_name_id[operators_cnt].Oper_ID[j] = rx_ans[i+j+1];
							}
							break;
						}
					}
					Debug_Output6(0,"COPS%d Name: %s, ID: %s", (operators_cnt+1), (uint32_t)nw_operator_name_id[operators_cnt].Oper_Name, (uint32_t)nw_operator_name_id[operators_cnt].Oper_ID,0,0,0);
				}
			}
			else
			{
				Debug_TextOut(0,"COPS did not get reponse-01");
			}
		}
		else
		{
			Debug_TextOut(0,"COPS did not get reponse-02");
		}
	}

	if(last_network_tried != PRIMARY_OPERATOR)
	{
		last_network_tried = PRIMARY_OPERATOR;
		for(operators_cnt = 0; operators_cnt < MAX_OPERATORS_ALLOWED; operators_cnt++)
		{
			if (mdm_find_response(nw_operator_name_id[operators_cnt].Oper_Name, (char *)glSystem_cfg.primary_operator) == TRUE )
			{
				//cmd_network_selection = "AT+COPS=4,2,40486,2\r";
				liberty_sprintf((char*) cmd_send, "AT+COPS=4,2,\"%s\",2\r", (char*)nw_operator_name_id[operators_cnt].Oper_ID);
				Debug_Output1(0, "cmd: %s", (uint32_t)cmd_send);
				mdm_send_AT_command((char*)cmd_send, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*300);
				break;
			}
		}
	}
	else
	{
		last_network_tried = SECONDARY_OPERATOR;
		for(operators_cnt = 0; operators_cnt < MAX_OPERATORS_ALLOWED; operators_cnt++)
		{
			if (mdm_find_response(nw_operator_name_id[operators_cnt].Oper_Name, (char *)glSystem_cfg.secondary_operator) == TRUE )
			{
				//cmd_network_selection = "AT+COPS=4,2,40486,2\r";
				liberty_sprintf((char*) cmd_send, "AT+COPS=4,2,\"%s\",2\r", (char*)nw_operator_name_id[operators_cnt].Oper_ID);
				Debug_Output1(0, "cmd: %s", (uint32_t)cmd_send);
				mdm_send_AT_command((char*)cmd_send, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*300);
				break;
			}
		}
	}

	return 0;
}

/**************************************************************************/
//! Read Local IP and Port assigned to the modem
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t mdm_get_local_ip_port()
{
	//    uint8_t * rx_ans_loc;
	uint8_t   i,ip_char, IP_address_start = 0;
	uint8_t   result=0;

	IP_done = FALSE;
	telit_wakeup_sleep(FALSE);

	if(mdm_comm_status == 5)
	{
		DelayMs(1000);
		result = mdm_send_data( (uint8_t*) "+++", 3, 40 );    //2209
		if(result!=MDM_ERR_NONE)
		    {
		        FD_BackGround_Occuring_Flag = TRUE;
		        DelayMs(1000);
	            result = mdm_send_AT_command( "AT#SS=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10 );
	            DelayMs(1000);
	            FD_BackGround_Occuring_Flag = FALSE;
		    }
		DelayMs(100);
		//    }
		if(result==MDM_ERR_NONE)//19-07-12
		{
			//Debug_TextOut(2,"Soc Closed");
			mdm_comm_status = 4;
			result = mdm_send_AT_command( "AT#SS=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10 );

			if(result == MDM_ERR_NONE)
			{
				//Debug_TextOut(2,"IP read");
				/*if(Get_HW_Version() == MPB_LIBERTY_1_0C)	//TODO: Avoid calling this for CDMA Modem
					IP_address_start = 19;
				else*/
					IP_address_start = 11; //20;
				memset(sys_mdm_p.local_ip_add,0,sizeof(sys_mdm_p.local_ip_add));//16-05-12
				for(i=0;i<17;i++)
				{
					ip_char = rx_ans[i+IP_address_start];
					if(ip_char == ',')
						break;
					sys_mdm_p.local_ip_add[i] = ip_char;
				}
				sys_mdm_p.local_ip_length = (uint8_t)i;
				sys_mdm_p.local_PORT = (uint16_t) atoi( (const char*) &rx_ans[sys_mdm_p.local_ip_length+(IP_address_start+1)] );

				/*for(i=0;i<sys_mdm_p.local_ip_length;i++)
    				Debug_Output1( 0, "IP: %d", sys_mdm_p.local_ip_add[i]);
    			Debug_Output1( 0, "PORT: %d", sys_mdm_p.local_PORT);*/

				result = mdm_send_AT_command( "AT#SO=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT* 1200 );

				if(result == MDM_ERR_NONE)
				{
					//Debug_TextOut(2,"Soc restored");
					if((mdm_find_response((uint8_t*)rx_ans, "CONNECT" ) == true)||
							(mdm_find_response((uint8_t*)rx_ans, "CONN" ) == true)||
							(mdm_find_response((uint8_t*)rx_ans, "NN" ) == true))//// VT
					{
						DelayMs(10);
						Debug_TextOut( 0, "CONNECTION WITH SERVER RESTORED\r\n" );
					}
					else
					{
						/////DelayMs(2000);
					}

					mdm_comm_status = 5;
				}
				else
				{
					//Debug_TextOut(2,"Soc restore fail");
				}

				IP_done = TRUE;
			}
		}
		else
		{
			Debug_TextOut(0,"Soc Not Opened +++ ");
		}
	}
	else
	{
		Debug_TextOut(0,"Soc Not Opened");
	}
	//telit_wakeup_sleep(TRUE);
	return result;
}


uint8_t mdm_get_local_ip_port_for_FD_OTA()
{
	uint8_t   result=0;

	FD_BackGround_Occuring_Flag = TRUE;
	telit_wakeup_sleep(FALSE);

	if(mdm_comm_status == 5)
	{
		result = mdm_send_data( (uint8_t*) "+++", 3, 40 );    //2209
		if(result!=MDM_ERR_NONE)
		{
		    DelayMs(1000);
	        result = mdm_send_data( (uint8_t*) "+++", 3, 40 );    //2209
		}
		mdm_send_AT_command( "AT#SS\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100 );
		mdm_send_AT_command( "AT#SH=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200) ;
		if(result!=MDM_ERR_NONE) result = mdm_send_AT_command( "AT#SS\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100 );
		watchdog_pat();

		if(result==MDM_ERR_NONE)//19-07-12
		{
			result = mdm_send_AT_command( "AT#SGACT=1,0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
			watchdog_pat();
			result = mdm_send_AT_command( "AT#SD=1,0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
			watchdog_pat();
			result = mdm_send_AT_command( "AT#SGACT=1,1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
			watchdog_pat();

            sys_mdm_p.signal_strength = mdm_get_signal_strength();
            DelayMs(100);

			sprintf ((char*) cmd_send, "AT#SD=1,1,%d,\"%s\",%d,%d,0,0,0\r", glSystem_cfg.UDP_server_port, (char*)glSystem_cfg.UDP_server_ip, 0 , 8181);//LOB:22/12/2015
			result = mdm_send_AT_command( (char*) cmd_send, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT * 1200);
		}
		else
		{
			Debug_TextOut(0,"Soc Not Opened 2 +++ ");
			result = mdm_send_AT_command( "AT#SGACT=1,0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
			watchdog_pat();
			result = mdm_send_AT_command( "AT#SD=1,0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
			watchdog_pat();
			result = mdm_send_AT_command( "AT#SGACT=1,1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
			watchdog_pat();

            sys_mdm_p.signal_strength = mdm_get_signal_strength();
            DelayMs(100);

			sprintf ((char*) cmd_send, "AT#SD=1,1,%d,\"%s\",%d,%d,0,0,0\r", glSystem_cfg.UDP_server_port, (char*)glSystem_cfg.UDP_server_ip, 0 , 8181);//LOB:22/12/2015
			result = mdm_send_AT_command( (char*) cmd_send, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT * 1200);
		}
	}
	else
	{
		Debug_TextOut(0,"Soc Not Opened");
	}
	FD_BackGround_Occuring_Flag = FALSE;
	return result;
}
/**************************************************************************/
//! Does Modem Power ON, Initialization, GPRS registration, sets socket
//! configuration and Opens a Socket Connection with Server, it uses the Server
//! IP, Port from meter's configuration file directly. All this in smaller
//! smaller states so this can be used in run time in task sharing basis and
//! meter is not blocked on modem processing causing other tasks to be delayed
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t telit_sock_open_states()
{
	uint8_t        ret        = SOCERR_NONE;
	uint8_t        result     = MDM_ERR_NONE;
	uint8_t i = 0;
	uint8_t modem_powered_on = 0;
//	Debug_TextOut(0,"telit_sock_open_states....."); //@r just added for testing
	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
	{
		return ret;
	}
	else
	{
		if(mdm_comm_status < 5)    // if socket is not opened already
		{
#ifdef Enable_Reduce_CC_Processing_Delay
			if(modem_init_to_process_cc_transaction && (!modem_powered_on))  //Check only when Credit Card is processing, for other events avoid this
			{
				////Debug_TextOut( 0, "recheck is modem power on" ); //@r uncommented for testing only
				if(mdm_send_AT_command("AT\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*10) == FALSE)
				{
					if(mdm_find_response((uint8_t*)rx_ans, "OK" ) == true)	//// VT
					{
						////Debug_TextOut( 0, "Response from Modem received" ); //@r uncommented for testing only
						modem_powered_on = 1;
					}
				}
			}
#endif
			if(MDM_AWAKE == FALSE)
			{

				if(!modem_init_to_process_cc_transaction || (!modem_powered_on)) //@r //if during cc transaction already powered on in credit_card_insert_isr, avoid this.
				{
					if(mdm_on_loop_count == 0) //if modem not powered on yet, then again retry once. Because, at the time of mdm_shut if we insert card, at that time this is needed.
					{
						GPIO_setAsPeripheralModuleFunctionInputPin(MDM_PORT, MDM_RXD_PIN, GPIO_PRIMARY_MODULE_FUNCTION);	//MDM_RXD
						MDM_RST_clr();
						telit_wakeup_sleep(FALSE);

						TELIT_POWER_ENABLE();
						DelayMs(300);
						MDM_Tel_ON();
					}
				}
				DelayMs(250);
				mdm_on_loop_count++;
			//	Debug_Output1( 0, "PWR On MDM loop : mdm_on_loop_count : %d", mdm_on_loop_count ); //@r added for testing only
			//	if(mdm_on_loop_count >= 22)
				if((mdm_on_loop_count >= 22) || (modem_powered_on == 1)) //@r added for testing only
				{
					for(i = 0; i <= 20; i++)
					{
						if(mdm_send_AT_command("AT\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*10) == FALSE)
						{
							break;
						}
						DelayMs(1000);
					}
					mdm_on_loop_count = 0;
					MDM_Tel_OFF();
					DelayMs(400);
					Debug_TextOut( 0, "PWR On MDM" );
					MDM_On_Time = RTC_epoch_now();
					Modem_On_Count++;
					MDM_AWAKE = TRUE;
					Regn_Check_Count =0;

					///////////////////////////////////////// Newly Added to Attach  to network in fast -- VT
					DelayMs(100);
			        ////mdm_send_AT_command("AT+CFUN=4\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

			        DelayMs(100);
			        mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

			        DelayMs(100);
			        ////mdm_send_AT_command("AT+CFUN=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

			        DelayMs(100);
			        mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

			        DelayMs(100);
			        mdm_send_AT_command("AT+CEREG?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

			        /////////////////////////////////////////

					if(!modem_init_to_process_cc_transaction) //@r
					{
						if(FIRST_COMM == TRUE) DelayMs(3000); //delay Added to get correct comms -- VT
						else DelayMs(5500); //delay Added to get correct comms -- VT
					}
					else //@r added to reduce cc process delay
					{
						DelayMs(1000);
					}

                    mdm_fetch_SPN(); //This is to check Apn automatically -- 21/02/2022 -- VT

				//	DelayMs(30000); //@r just added for testing //If enabled this delay, can see First Idle Screen, then Approved while processing cc transaction.
					if (((MDM_Type_LE910NA1 == TRUE)||(MDM_Type_ME910C1WW == TRUE)) && ((CC_Auth_Entered == TRUE) || (do_CC_OLT == TRUE)))	//Only for OLT, we'll do whole modem init here
					{
						result = telit_init();
#ifdef Enable_Reduce_CC_Processing_Delay
						if(result == SOCERR_NONE)  //@r loop just added for testing
						{
							////Debug_TextOut( 0, "Socket is opened ... do not process next commands" );
							return SOCERR_NONE;
						}
#endif
					}
				}
				else
					return SOCERR_IP_STK_OPEN;
			}
			else
			{
				if(Regn_Check_Count == 0)	//Do this only for the first time
					telit_wakeup_sleep(FALSE);
			}
			memset( (char*) cmd_send, 0, sizeof(cmd_send) );
			if(Regn_Check_Count == 0)	//Print this only for first time
			{
				mdm_recovery_state = 0;
				diag_text_Screen( "Chk Auto N/W Actvn", FALSE, FALSE );

				if(mdm_send_AT_command("AT\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*10) == TRUE)
				{
					mdm_send_data( (uint8_t*)"+++", 3, 40 );
					mdm_send_AT_command( "AT\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200) ;
					mdm_send_AT_command( "AT#SH=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200) ;
				}
				result = mdm_send_AT_command("AT&P0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
				result = mdm_send_AT_command("ATE0&K0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
				result = mdm_send_AT_command("AT+CFUN=5\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);//1506    ///JBENDOR-2012.03.19
				result = mdm_send_AT_command("AT#SGACT=1,0\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
				result = mdm_send_AT_command("AT+CGATT=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*500);
				result = mdm_send_AT_command("AT#SGACT=1,1\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);//25-09-12
				DelayMs(1000);
			}

#ifdef Enable_Reduce_CC_Processing_Delay //This is for ME910, try to open socket as soon as possible after modem init done. Need to test and confirm this statement.
					//@r start
					if(modem_init_to_process_cc_transaction)
					{
						modem_init_to_process_cc_transaction = 0;
						//Debug_TextOut( 0, "during Auto N/W Actvn2 try to open socket\r\n" ); //@r just added for testing
						result = mdm_send_AT_command(cmd_context_act, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
						DelayMs(100);

                        sys_mdm_p.signal_strength = mdm_get_signal_strength();
                        DelayMs(100);

						sprintf ((char*) cmd_send, "AT#SD=1,1,%d,\"%s\",%d,%d,0,0,0\r", glSystem_cfg.UDP_server_port, (char*)glSystem_cfg.UDP_server_ip, 0 , 8181);//LOB:22/12/2015
						result = mdm_send_AT_command( (char*) cmd_send, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT * 1200);

						if( result != MDM_ERR_NONE )
						{
							glComm_failure_count++;    //increment failure count
							mdm_comm_status = 3; // try to activate context again
							Debug_TextOut( 0, "soc open fail" );
							return GPRS_COMMERR_SOCK_OPEN_ERR;
//							if(glComm_failure_count > glSystem_cfg.Queue_failure_count)	//Error Recovery for continuous modem failures
//							{
//								Modem_Recovery_Loop();
//							}
						}
						else
						{

							if((mdm_find_response((uint8_t*)rx_ans, "CONNECT" ) == true)||
									(mdm_find_response((uint8_t*)rx_ans, "CONN" ) == true)||
									(mdm_find_response((uint8_t*)rx_ans, "NN" ) == true))//// VT
							{
								DelayMs(100);
								Debug_TextOut( 0, "CONNECTED WITH SERVER\r\n" );
							}
							else
							{
								DelayMs(2000);
							}

							//diag_text_Screen( "SocOpnd", FALSE );
							DO_IP_SYNC = TRUE;
							mdm_comm_status = 5;    // socket opened
							//Debug_TextOut( 0, "soc open success during Auto N/W Actvn2\r\n" ); //@r just added for testing
							return SOCERR_NONE;
						}
					}
					//@r end
#endif

			if(Regn_Check_Count < MAX_REGN_CHECK_COUNT)
			{
                if(Regn_Check_Count==8)
                {
						if(mdm_send_AT_command("AT\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*10) == TRUE)
						{
							mdm_send_data( (uint8_t*)"+++", 3, 40 );
							mdm_send_AT_command( "AT\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200) ;
							mdm_send_AT_command( "AT#SH=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200) ;
						}
                		result = mdm_send_AT_command("AT&P0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);

                		result = mdm_send_AT_command("ATE0&K0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);

                		mdm_send_AT_command("AT#SGACT=1,0\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);

                		liberty_sprintf( (char*)cmd_send, "AT+CGDCONT=1,\"%s\",\"%s\"\r", "IP", (char*)glSystem_cfg.apn );

                		result = mdm_send_AT_command( (char*) cmd_send, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20 );

                		result = mdm_send_AT_command(cmd_soc_cfg_evt, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT);

                		result = mdm_send_AT_command(cmd_autocontxt_act_ext, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT);

                		result = mdm_send_AT_command("AT#SCFGEXT2=1,1\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT);
                		DelayMs(500);
                		//watchdog_pat();
                		result = mdm_send_AT_command(cmd_autocontext_act, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
                		DelayMs(500);
                		result = mdm_send_AT_command(cmd_context_act, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                		//watchdog_pat();
                		DelayMs(500);
                }

				//watchdog_pat();
				mdm_send_AT_command("AT#SGACT=1,1\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);//15-03-21 ADDED FOR SOCKET ACTIVATION RETRY -- VT
				Regn_Check_Count++;
				DelayMs(250);//Check for registration once in approximately 1 second; Don't reduce this delay, we are forced to maintain at least one second before reusing this command
				if( mdm_send_AT_command(cmd_context_check, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES,(DEFAULT_TIME_OUT*200)) == MDM_ERR_NONE )
				{
					if ( mdm_find_response(rx_ans, "#SGACT: 1,1" ) == TRUE )
					{
					    memcpy(sys_mdm_p.COPS,0,30);
                        mdm_send_AT_command("AT+COPS?\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
                        memcpy(sys_mdm_p.COPS,rx_ans,30);
						Regn_Check_Count = 0;
						mdm_comm_status = 4;	//Proceed for next steps only when it succeeds
						//diag_text_Screen( "N/W Regn OK", FALSE );
					}
					else	//waiting for registration, continue to check
					{
						mdm_comm_status = 0;
						Last_Comms_Success_Flg = FALSE;
						return SOCERR_IP_STK_OPEN;
					}
				}
				else	//no response for the command, then need to break and retry quicker
				{
					Regn_Check_Count += 8;	//this will solve the too long wait when modem fails
					mdm_comm_status = 0;
					Last_Comms_Success_Flg = FALSE;
					return SOCERR_IP_STK_OPEN;
				}
			}

			//The following recovery will execute only if the modem did not register to network even after maximum wait
			if(mdm_comm_status != 4)
			{
				if(mdm_recovery_state == 0)
				{
					diag_text_Screen( "Auto N/W Regn Fail", FALSE, FALSE );
					mdm_init_once_per_bootup();
					nw_operator_name_id_check();//try backup network
					mdm_recovery_state = 1;
					mdm_init_for_recovery = 0;
					goto exit_sock_open;
				}
				else if(mdm_recovery_state == 1)
				{
					mdm_init_for_recovery++;
					DelayMs(250);
					result =  mdm_send_AT_command("AT+CPIN?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
					if ( result != MDM_ERR_NONE || (mdm_find_response(rx_ans, "+CPIN: READY") == FALSE) )
					{
						result = MDM_ERR_CMD_FAILED;
					}
					else
					{
						diag_text_Screen("SIM READY", FALSE, FALSE);
						mdm_enable_unsolicited_reg_results();
						mdm_send_AT_command("AT+CGREG=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
						mdm_send_AT_command("AT+CGREG=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
						mdm_nw_reg_check();    // check if registered to GSM and GPRS
						result = MDM_ERR_NONE;	// Set result to no error so that it continues with next steps
					}

					if(result == MDM_ERR_NONE)	//If it succeeds jump to recovery state 3
					{
						mdm_recovery_state = 3;
						mdm_fetch_SPN(); //This is to check Apn automatically -- 21/02/2022 -- VT
						Debug_TextOut( 0, "MDM Init DONE" );
						//Read_Mdm_diag_data();
						//sys_mdm_p.signal_strength = mdm_get_signal_strength();
					}
					else if(mdm_init_for_recovery >= 15)//Try 15 times if it fails, otherwise jump to recovery state 2
					{
						mdm_recovery_state = 2;
						if (result != MDM_ERR_NONE)
							diag_text_Screen("SIM FAIL", FALSE, FALSE);
						//Read_Mdm_diag_data();
						//sys_mdm_p.signal_strength = mdm_get_signal_strength();
					}
					else
						mdm_recovery_state = 1;
					//If it is not the final attempt, keep following the loop
					//Else let it go through the main failure count checks
					if(Final_mdm_init_attempt == false)
					{
						goto exit_sock_open;
					}
				}
				else if(mdm_recovery_state == 2)
				{
					//Try mdm_init() once again after resetting the modem,
					//do not reset the mdm_init_for_recovery variable to avoid infinite loop here
					if(MDM_AWAKE == TRUE)
					{
						if(mdm_off_loop_count == 0)
						{
							init_Telit_GPIO();
							telit_wakeup_sleep(TRUE);
							MDM_Tel_ON();
						}

						DelayMs(250);
						mdm_off_loop_count++;
						//Debug_TextOut( 0, "PWR Off MDM loop" );
						if(mdm_off_loop_count >= 14)
						{
							mdm_off_loop_count = 0;
							mdm_on_loop_count = 0;
							MDM_Tel_OFF();
							TELIT_POWER_DISABLE();
							MDM_RST_clr();
							GPIO_setAsInputPinWithPullUpResistor(MDM_PORT, MDM_RXD_PIN);
							Debug_TextOut( 0, "PWR Off MDM" );

							MDM_Off_Time = RTC_epoch_now();
							if(MDM_On_Time == 0)
							{
								MDM_On_Time = MDM_Off_Time;
							}
							MDM_Total_On_Time +=(uint32_t)((uint32_t)(MDM_Off_Time)-(uint32_t)(MDM_On_Time));
							//Debug_Output1(2,"Total_Modem_on_time:%d",MDM_Total_On_Time);
							MDM_On_Time = 0;
							mdm_comm_status = 0;
							MDM_AWAKE = FALSE;
							Last_Comms_Success_Flg = FALSE;

							mdm_recovery_state = 1;
							Final_mdm_init_attempt = true;
							return SOCERR_IP_STK_OPEN;
						}
						else
							return SOCERR_IP_STK_OPEN;
					}
					DelayMs(800);
				}
				else if(mdm_recovery_state == 3)
				{
					//diag_text_Screen("N/W Regd", FALSE);
					if(mdm_comm_status < 2)
						mdm_comm_status = 1;
					if(mdm_comm_status < 3)
					{
						mdm_comm_status = 2;
						result = telit_set_pdpcontext(glSystem_cfg.apn);
						if( result == MDM_ERR_NONE )
							mdm_comm_status = 4;
					}
				}
			}

			if(mdm_comm_status == 4)    //keeping a common registration success msg; don't use else for the previous condition
			{
				diag_text_Screen( "N/W Regn OK", FALSE , FALSE);
				Read_Mdm_diag_data();	//AS-672: Refresh these parameters, some of them were going blank causing the issue
				result = mdm_send_AT_command(cmd_soc_cfg_evt, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT);

				result = mdm_send_AT_command(cmd_autocontxt_act_ext, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT);

				result = mdm_send_AT_command("AT#SCFGEXT2=1,1\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT);
				DelayMs(100);
				result = mdm_send_AT_command(cmd_autocontext_act, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
				DelayMs(100);
				result = mdm_send_AT_command(cmd_context_act, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
				DelayMs(100);
			}

            sys_mdm_p.signal_strength = mdm_get_signal_strength();
            DelayMs(100);

			sprintf ((char*) cmd_send, "AT#SD=1,1,%d,\"%s\",%d,%d,0,0,0\r", glSystem_cfg.UDP_server_port, (char*)glSystem_cfg.UDP_server_ip, 0 , 8181);//LOB:22/12/2015
			result = mdm_send_AT_command( (char*) cmd_send, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT * 1200);

			if( result != MDM_ERR_NONE )
			{
				glComm_failure_count++;    //increment failure count
				mdm_comm_status = 3; // try to activate context again
				Debug_TextOut( 0, "soc open fail" );
				ret = GPRS_COMMERR_SOCK_OPEN_ERR;
				if(glComm_failure_count > glSystem_cfg.Queue_failure_count)	//Error Recovery for continuous modem failures
				{
					Modem_Recovery_Loop();
				}
			}
			else
			{
				if((mdm_find_response((uint8_t*)rx_ans, "CONNECT" ) == true)||
						(mdm_find_response((uint8_t*)rx_ans, "CONN" ) == true)||
						(mdm_find_response((uint8_t*)rx_ans, "NN" ) == true))//// VT
				{
					DelayMs(100);
					Debug_TextOut( 0, "CONNECTED WITH SERVER\r\n" );
				}
				else
				{
					DelayMs(2000);
				}
				//diag_text_Screen( "SocOpnd", FALSE );
				DO_IP_SYNC = TRUE;
				mdm_comm_status = 5;    // socket opened
				ret = SOCERR_NONE;
				//Debug_TextOut( 0, "soc open success\r\n" ); //@r just added for testing
			}
		}
		else    // restore the socket if it is already open
		{
			//diag_text_Screen( "SocOpnd", FALSE );
			//Debug_TextOut( 0, "soc is already opened \r\n" ); //@r just added for testing
			Regn_Check_Count = 0;
			ret = SOCERR_NONE;
		}

		exit_sock_open:
		return ret;
	}
}

/**************************************************************************/
//! Used in run time of meter to recover the GPRS modem from continuous
//! communication failures
//! \param void
//! \return void
/**************************************************************************/
void Modem_Recovery_Loop()
{
	uint8_t gReInit_Mdm = 0; //it was used only here so declared it as local variable //vinay
	glComm_failure_count = 0;
	//reinit_telit_Modem();
	init_Telit_GPIO();

	telit_power_off();
	mdm_recovery_state = 2;
	mdm_off_loop_count = 0;
	mdm_on_loop_count = 0;

	gReInit_Mdm++;
	//Debug_TextOut(0, "gReInit_Mdm++");
	if(gReInit_Mdm >= 2)
	{
		//Debug_TextOut(0, "gReInit_Mdm>2");
		if((COMMS_DOWN == FALSE) && (Initial_boot == TRUE))	//very important now since we are adding a timeout
		{
			Debug_TextOut(0, "COMMS_DOWN = TRUE");
			COMMS_DOWN = TRUE;
			COMMS_DOWN_timestamp = RTC_epoch_now();
		}
		gReInit_Mdm = 0;
	}
}

/**************************************************************************/
//! Reads all modem related diagnostic like IMEI, IMSI etc. into the
//! sys_mdm_p structure variable
//! \param void
//! \return void
/**************************************************************************/
void Read_Mdm_diag_data()
{
	uint8_t read_mdm_diag = 0;

	sys_mdm_p.signal_strength = 0;
	sys_mdm_p.signal_strength = mdm_get_signal_strength();

	for(read_mdm_diag=0; read_mdm_diag<2; read_mdm_diag++)
	{
		memset(sys_mdm_p.type,0,sizeof(sys_mdm_p.type));
		mdm_fetch_type(sys_mdm_p.type, sizeof(sys_mdm_p.type));
		if(strlen((char *)sys_mdm_p.type) >= 5)
		{
			Debug_Output1(0,"MDM Type: %s", (uint32_t)sys_mdm_p.type);
			if(strcmp((const char *)"LE910-NA1", (const char *)sys_mdm_p.type) == 0)
			{
				MDM_Type_LE910NA1 = TRUE;
				Debug_TextOut(0, "It's LE910-NA1");
			}
			else
			{
				MDM_Type_LE910NA1 = FALSE;
				//Debug_TextOut(2, "It's NOT a LE910-NA1");
			}
            if(strcmp((const char *)"ME910C1-WW", (const char *)sys_mdm_p.type) == 0)
            {
                MDM_Type_ME910C1WW = TRUE;
                Debug_TextOut(0, "It's ME910C1-WW MODEM");
            }
            else
            {
                MDM_Type_ME910C1WW = FALSE;
                //Debug_TextOut(0, "It's NOT a ME910C1-WW");
            }
			break;
		}
	}

    for(read_mdm_diag=0; read_mdm_diag<2; read_mdm_diag++)
    {
        memset(sys_mdm_p.type,0,sizeof(sys_mdm_p.type));
        mdm_fetch_type(sys_mdm_p.type, sizeof(sys_mdm_p.type));
        if(strlen((char *)sys_mdm_p.type) >= 5)
        {
            Debug_Output1(0,"MDM Type: %s", (uint32_t)sys_mdm_p.type);
            if(strcmp((const char *)"ME910C1-WW", (const char *)sys_mdm_p.type) == 0)
            {
                MDM_Type_ME910C1WW = TRUE;
                Debug_TextOut(2, "It's ME910C1-WW");
            }
            else
            {
                MDM_Type_ME910C1WW = FALSE;
                //Debug_TextOut(2, "It's NOT a ME910C1-WW");
            }
            break;
        }
    }

	for(read_mdm_diag=0; read_mdm_diag<2; read_mdm_diag++)
	{
		memset(sys_mdm_p.sw_rev,0,sizeof(sys_mdm_p.sw_rev));
		mdm_fetch_SW_revision(sys_mdm_p.sw_rev, sizeof(sys_mdm_p.sw_rev));
		if(strlen((char *)sys_mdm_p.sw_rev) >= 5)
		{
			Debug_Output1(0,"MDM FW Ver: %s", (uint32_t)sys_mdm_p.sw_rev);
			break;
		}
	}

	for(read_mdm_diag=0; read_mdm_diag<2; read_mdm_diag++)
	{
		memset(sys_mdm_p.IMEI,0,sizeof(sys_mdm_p.IMEI));
		mdm_fetch_IMEI(sys_mdm_p.IMEI, sizeof(sys_mdm_p.IMEI));
		if(strlen((char *)sys_mdm_p.IMEI) >= 5)
		{
			Debug_Output1(0,"IMEI: %s", (uint32_t)sys_mdm_p.IMEI);
			break;
		}
	}

	for(read_mdm_diag=0; read_mdm_diag<2; read_mdm_diag++)
	{
		memset(sys_mdm_p.IMSI,0,sizeof(sys_mdm_p.IMSI));//14-02-12
		mdm_fetch_IMSI(sys_mdm_p.IMSI, sizeof(sys_mdm_p.IMSI));//19-04-12
		if(strlen((char *)sys_mdm_p.IMSI) >= 5)
		{
			Debug_Output1(0,"IMSI: %s", (uint32_t)sys_mdm_p.IMSI);
			break;
		}
	}

	for(read_mdm_diag=0; read_mdm_diag<2; read_mdm_diag++)
	{
		memset(sys_mdm_p.ICCID,0,sizeof(sys_mdm_p.ICCID));
		mdm_fetch_ICCID(sys_mdm_p.ICCID, sizeof(sys_mdm_p.ICCID));
		if(strlen((char *)sys_mdm_p.ICCID) >= 5)
		{
			Debug_Output1(0,"ICCID: %s", (uint32_t)sys_mdm_p.ICCID);
			break;
		}
	}

	memset(rx_ans, 0, sizeof(rx_ans));
	mdm_fetch_SPN();
}

#ifdef ENABLE_SMS_MODE
/**************************************************************************/
//! Initializes the modem with all SMS related AT commands to support SMS sending
//! \param void
//! \return void
/**************************************************************************/
void Init_SMS_Mode()
{
	mdm_send_AT_command("AT+CSCS=GSM\r\n",rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
	mdm_send_AT_command("AT#SMSMODE=1\r\n",rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);

	mdm_send_AT_command("AT+CSMS=0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);

	mdm_send_AT_command("AT+CMGF=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
	mdm_send_AT_command("AT+CSMP=17,167,0,0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);

	mdm_send_AT_command("AT+CSCA?\r\n",rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);

	mdm_send_AT_command("AT+CPMS=SM,SM,SM\r\n",rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);

	mdm_send_AT_command("AT+CNMI=3,1\r\n",rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
	mdm_send_AT_command("AT+CSAS=0\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);

	mdm_send_AT_command("AT#E2SMSRI=1000\r\n",rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
}

/**************************************************************************/
//! Send SMS requesting for proper configuration
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t Send_SMS_request()
{
	uint8_t i=0,length=0,checksum=0,temp_buf[32]={0};//temp_buf1[22]={0};
	uint32_t device_ID = 0;
	uint8_t  result = MDM_ERR_NONE;

	if(mdm_comm_status >= 5)
	{
		glClose_socket = TRUE;
		telit_sock_close();
		watchdog_pat();
	}

	watchdog_pat();

	if(Parking_Clock_Refresh[Current_Space_Id] == PARKING_CLOCK_RESET_MODE) //This is added to refresh clock for new payments
	{
		//Debug_TextOut(0,"Parking Clock");
		Update_Parking_Clock_Screen();
		Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_STOP_MODE;
	}

	//FRAME REQUEST
	uint8_t check = 26;
	// memset(temp_buf1,0,sizeof(temp_buf1));
	//sprintf((char*)temp_buf1,"AT+CMGS=%s,145\r", DESTINATION_NUMBER);

	device_ID = (uint32_t)(glSystem_cfg.cust_id) |(uint32_t)(glSystem_cfg.meter_id<<16);
	sprintf((char*)temp_buf, "%lu",device_ID );

	length = strlen((char*)temp_buf);

	checksum += 0x2c;//FS
	for(i=0;i<length;i++)
	{
		checksum += temp_buf[i];
	}
	checksum += 0x2c;//FS
	length += 2;
	memset(temp_buf,0,sizeof(temp_buf));

	if(Send_SMS_ACK != TRUE)//SMS REQUEST
	{
		sprintf((char*)temp_buf, "CFG,%d,%d,%lu,END\r\0", checksum,length,device_ID );
		//diag_text_Screen( temp_buf, TRUE );
	}
	else//SMS ACK
	{
		sprintf((char*)temp_buf, "SMS ACK\r\0");
	}

	//FRAME REQUEST

	for(i=0;i<2;i++)//destination number2
	{
		watchdog_pat();
		result = mdm_send_AT_command("AT+CMGS=+917022953693,145\r",rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
		DelayMs(1000);
		result = mdm_send_AT_command((char*)temp_buf,rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
		Modem_out_UART( &check,  1 );
		DelayMs(1000);
		if(result == MDM_ERR_NONE)
		{
			Initial_boot = FALSE;
			break;
		}
		watchdog_pat();
	}

	if(Parking_Clock_Refresh[Current_Space_Id] == PARKING_CLOCK_RESET_MODE) //This is added to refresh clock for new payments
	{
		//Debug_TextOut(0,"Parking Clock");
		Update_Parking_Clock_Screen();
		Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_STOP_MODE;
	}

	for(i=0;i<2;i++)//destination number3
	{
		watchdog_pat();
		result = mdm_send_AT_command("AT+CMGS=+919686205233,145\r",rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
		DelayMs(1000);
		result = mdm_send_AT_command((char*)temp_buf,rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
		Modem_out_UART( &check,  1 );
		DelayMs(1000);
		if(result == MDM_ERR_NONE)
		{
			Initial_boot = FALSE;
			break;
		}
		watchdog_pat();
	}

	if(Parking_Clock_Refresh[Current_Space_Id] == PARKING_CLOCK_RESET_MODE) //This is added to refresh clock for new payments
	{
		//Debug_TextOut(0,"Parking Clock");
		Update_Parking_Clock_Screen();
		Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_STOP_MODE;
	}

	watchdog_pat();
	DelayMs(1000);
	return result;
}

/**************************************************************************/
//! Extract required configuration parameters from the received SMS
//! \param void
//! \return void
/**************************************************************************/
void Extract_SMSCFG_Details()
{
	uint8_t index = 0, local_buf[50] = {0},buf=0,sms_length=0,chksum=0,length=0;
	uint8_t index1 = 0,lchksum=0,FS_Count=0,ch=0,Format_verified = FALSE;
	uint32_t device_ID = 0,temp=0;
	//Debug_TextOut(0,"Extract_SMSCFG_Details\r\n");
	if ((mdm_find_response((char*) rx_ans, (char*) "CFG") == true) && (mdm_find_response((char*) rx_ans, (char*) "END") == true))
	{
		Debug_TextOut(0,"Extract_SMSCFG_Details-01\r\n");
		FS_Count = 0;
		length = strlen((char*)rx_ans);
		for(index=0;index<length;index++)
		{
			ch = rx_ans[index];
			if(((rx_ans[index]==0x43) && (rx_ans[index+1]==0x46) && (rx_ans[index+2]==0x47)) || (Format_verified == TRUE))//Identify SF
			{
				Format_verified = TRUE;
				switch (ch)
				{
				case ',':
					FS_Count++;
					if(FS_Count == 2)//CHECK SUM
					{
						chksum = atoi((char*)local_buf);
					}
					else if(FS_Count == 3)//LENGTH
					{
						sms_length = atoi((char*)local_buf);
						for(index1=0;index1<sms_length;index1++)
						{
							lchksum += rx_ans[index+index1];
						}
						if((lchksum != chksum) || (sms_length > 140))
						{
							//Debug_TextOut(0,"SMS:Invalid checksum\r\n");
							index = 254;
						}
						else
						{
							//Debug_TextOut(0,"SMS:Valid Checksum\r\n");
						}

					}
					else if(FS_Count == 4)//DEVICE ID
					{
						device_ID = atoi((char*)local_buf);
						temp = (uint32_t)(glSystem_cfg.cust_id) |(uint32_t)(glSystem_cfg.meter_id<<16);
						//memcpy(&temp,glSystem_cfg.meter_id,4);
						if(device_ID != temp)
						{
							//Debug_TextOut(0,"SMS:Invalid device ID\r\n");
							index = 254;
						}
						else
						{
							//Debug_TextOut(0,"SMS:Valid Device ID\r\n");
						}
					}
					else if(FS_Count == 5)//UPDATE FLAG
					{
						//future purpose
					}
					else if(FS_Count == 6)//NEW IDEVICE D
					{
						//future purpose
					}
					else if(FS_Count == 7)//APN
					{
						memset(glSystem_cfg.apn,0,sizeof(glSystem_cfg.apn));
						memcpy(glSystem_cfg.apn,local_buf,strlen((char*)local_buf));
					}
					else if(FS_Count == 8)//IP
					{
						//Sub_Net_validation;
						if ((mdm_find_response((char*) local_buf, (char*) "172.25") != 0) || (mdm_find_response((char*) local_buf, (char*) "106.51") != 0)
								|| (mdm_find_response((char*) local_buf, (char*) "64.132") != 0))
						{
							//memset(GW_Confing_ptr.cfg.server_IP,0,sizeof(GW_Confing_ptr.cfg.server_IP));
							//memcpy(GW_Confing_ptr.cfg.server_IP,local_buf,strlen((char*)local_buf));
							memset(glSystem_cfg.UDP_server_ip,0,sizeof(glSystem_cfg.UDP_server_ip));
							memcpy(glSystem_cfg.UDP_server_ip,local_buf,strlen((char*)local_buf));
							Send_SMS_ACK = TRUE;
						}
						else//assign default IP
						{
							memset(glSystem_cfg.UDP_server_ip,0,sizeof(glSystem_cfg.UDP_server_ip));
							//memcpy(GW_Confing_ptr.cfg.server_IP,DEFAULT_SERVER_IP,strlen(DEFAULT_SERVER_IP));
							strcpy( (char*)glSystem_cfg.UDP_server_ip,DEFAULT_SERVER_IP);
						}
					}
					else if(FS_Count == 9)//PORT
					{
						glSystem_cfg.UDP_server_port = atoi((char*)local_buf);
						Debug_Output1(0,"UDP Port: %d",glSystem_cfg.UDP_server_port);
					}

					memset(local_buf,0,sizeof(local_buf));
					buf = 0;
					//FS_Count++;
					break;
				default :
					break;
				}
				if((FS_Count > 0) && (ch!=','))
					local_buf[buf++] = rx_ans[index];
				if(buf >= 50)
					buf = 0;
			}
		}
	}
	else
	{
		Debug_TextOut(0,"END: Extract_SMSCFG_Details\r\n");
	}
}

/**************************************************************************/
//! Read the SMS received in modem and validate for the expected format
//! \param void
//! \return void
/**************************************************************************/
void Read_Validate_SMS()
{
	uint8_t index = 0,count,result = MDM_ERR_NONE,total_SMS=0;
	uint8_t temp_buf1[16] = {0};
	watchdog_pat();
	Send_SMS_ACK = FALSE;
	Debug_TextOut(0,"Read_Validate_SMS\r\n");

	if(Parking_Clock_Refresh[Current_Space_Id] == PARKING_CLOCK_RESET_MODE) //This is added to refresh clock for new payments
	{
		//Debug_TextOut(0,"Parking Clock");
		Update_Parking_Clock_Screen();
		Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_STOP_MODE;
	}

	if(mdm_comm_status >= 5)
	{
		glClose_socket = TRUE;
		telit_sock_close();
		DelayMs(2000);
		watchdog_pat();
	}
	for(index=0;index<3;index++)
	{
		result = mdm_send_AT_command("AT+CPMS=SM,SM,SM\r\n",rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);//INDEX WISE
		DelayMs(1000);

		if(result == MDM_ERR_NONE)
		{
			if (mdm_find_response((char*) rx_ans, (char*) "+CPMS") == true)
			{
				memcpy(temp_buf1,&rx_ans[9],2);
				total_SMS = atoi((char*)temp_buf1);
			}
			break;
		}
	}

	if(Parking_Clock_Refresh[Current_Space_Id] == PARKING_CLOCK_RESET_MODE) //This is added to refresh clock for new payments
	{
		//Debug_TextOut(0,"Parking Clock");
		Update_Parking_Clock_Screen();
		Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_STOP_MODE;
	}

	if(total_SMS > SMS_LIMIT)
		total_SMS = 0;
	Debug_Output1(0,"total_SMS:%d\r\n",total_SMS);
	if(total_SMS > 0)
	{
		for(count=1;count<=total_SMS;count++)
		{
			for(index=0;index<3;index++)
			{
				memset(temp_buf1,0,sizeof(temp_buf1));
				sprintf((char*)temp_buf1,"AT+CMGR=%d\r\n", count);
				result = mdm_send_AT_command((char*)temp_buf1,rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
				//result = Modem_Cmd_send("AT+CMGR=1\r\n",strlen("AT+CMGR=1\r\n"), 200);//INDEX WISE
				DelayMs(1000);

				if(result == MDM_ERR_NONE)
				{
					if (mdm_find_response((char*) rx_ans, (char*) "+CMGR") == true)
					{
						Extract_SMSCFG_Details();
					}
					break;
				}

			}
			if(Send_SMS_ACK == TRUE)
				break;
			watchdog_pat();
		}
		if(Send_SMS_ACK == TRUE)
		{
			Debug_TextOut(0,"Send_SMS_ACK\r\n");

			disable_telit_RI_interrupt();

			Send_SMS_request();
			Send_SMS_ACK = FALSE;

			//DELETE READ & EXTRACTED SMS MSG's
			for(index=0;index<3;index++)
			{
				result=mdm_send_AT_command("AT+CMGD=1,4\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*200);
				DelayMs(1000);
				if(result == MDM_ERR_NONE)
					break;
			}

			//Re-Init Telit Modem by power off/on
			telit_power_off();
		}
	}
	watchdog_pat();
	if(Parking_Clock_Refresh[Current_Space_Id] == PARKING_CLOCK_RESET_MODE) //This is added to refresh clock for new payments
	{
		//Debug_TextOut(0,"Parking Clock");
		Update_Parking_Clock_Screen();
		Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_STOP_MODE;
	}
}
#endif

/**************************************************************************/
//! These are AT response from Modem, No need to cross check with standard packet
//! \param void
//! \return void
/**************************************************************************/
void Special_Test_AT_Commands(uint8_t * cmdstr, uint16_t cmdlen)
{
    if((mdm_find_response((uint8_t*)cmdstr, "AT#SS" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT#SPN" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT#SGACT") == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT#SI") == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT+CGDCONT") == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT+COPS") == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT#MONI") == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT+IPR") == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT#RFSTS") == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT+SERVINFO" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT#CEDRXS") == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT+CEDRXS") == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT+CEDRXRDP") == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT+WS46") == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT#SD" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "+++" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT#SH=1" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT#FWSWITCH" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT+CFUN" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT&P0" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT&W0" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT+CCID" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT+CGMM" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT+CGMR" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT+CGSN" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;
    if((mdm_find_response((uint8_t*)cmdstr, "AT+CIMI" ) == true)&&(!Modem_Test_Flag)) Modem_Test_Flag = TRUE;

    if(mdm_find_response((uint8_t*)cmdstr, "AT#SSEND" ) == true) Modem_Test_Flag = FALSE;

    //if(Modem_Test_Flag) //Debug_Output1(0,"Modem_Test_Flag: %d",Modem_Test_Flag);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************



