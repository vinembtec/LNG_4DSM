//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_communication.c
//
//****************************************************************************
//*****************************************************************************
//
//!
//! \addtogroup LibG2_communication_api
//! @{
//
//*****************************************************************************
#include "LibG2_communication.h"

/*
***********************************************************************
*Imported Functions
***********************************************************************
*/

/*
***********************************************************************
*Imported Global Variables
***********************************************************************
*/
extern volatile uint8_t		SLEEP_ENABLE;
extern volatile uint8_t 	Duty_Cycle_Mode;
extern uint8_t              DO_IP_SYNC;                 //2103: PBC
extern uint8_t              glIn_diag_mode;
extern uint8_t              rx_ans[ GPRS_GEN_BUF_SZ_SMALL ];
//extern uint8_t              glTemp_buf[GPRS_GEN_BUF_SZ_SMALL];//not used in this program //vinay
extern uint8_t              T2_Faw_flag, T1_Faw_flag;    //1908
//extern uint8_t              FIRST_COMM;//not used in this program //vinay
//extern uint16_t             glCoin_Index;               //1609//not used in this program //vinay
extern GPRS_FD_File         glAfm_response_p[FFACT_TOTAL_FILES+1];
extern RTC_C_Calendar 		rtc_c_calender;
extern Modem                sys_mdm_p;                  //05-01-12
extern uint16_t 			AES_KEY_ID;
extern uint8_t              AES_KEY[AES_KEY_SIZE];//14-05-2013:DPLIBB-481
extern uint32_t             received_crc, dfg_crc; //,mc_code_RSAD;
//extern uint8_t				CLR_BATCH_ON_QFULL;//not used in this program //vinay
extern Chunk_DNLD_Status    chunk_DNLD_status;
extern uint8_t  			File_count;
extern uint16_t				Total_Events_count;
extern uint16_t				Downloaded_chunks;
extern uint8_t 		        REINITIATE_FD;
//extern uint16_t		    	BSU_Batch_time_elapsed;//not used in this program //vinay
extern uint16_t             glMdmUart_bytes_recvd;
extern uint8_t              glMdmUart_recv_buf[ GPRS_GEN_BUF_SZ_VLARGE ];
extern uint32_t 			Sync_status;//,MB_RTC;//not used in this program //vinay
//extern uint8_t            	IP_done;//not used in this program //vinay
extern COINDATA			    lCoindata[9];
extern uint8_t				Coin_Per_Udp_Packet;
extern uint16_t             lCoinCount;
extern const uint8_t  		PCI_VERSION, MPB_VERSION , sub_VERSION, branch_version;
//extern uint8_t 				do_General_Sync;//not used in this program //vinay
extern uint16_t             DFG_filled_resp_len;
extern uint32_t 			Tasks_Priority_Register;
extern GPRSSystem      		glSystem_cfg;
extern uint8_t         		MDM_AWAKE;
extern EventCache           glEvent_cache[MSM_MAX_PARKING_BAYS];
extern uint16_t 			batt_vltg[MAX_DIAG_READINGS]; //, solar_voltage[MAX_DIAG_READINGS];
extern int8_t 				tempC[MAX_DIAG_READINGS];
extern GPRS_OLTResponse     glOLT_resp;
extern uint8_t				Process_Flash_Queue;//, sensor_tnx_flag;//not used in this program //vinay
extern uint16_t 			Current_Space_Id;

extern MBInfo      			glMB_info;
extern uint16_t 			BufferSize;			// RF buffer size
extern uint8_t 				Buffer[];
extern tRadioDriver 		*Radio;
extern uint8_t              *flag_check_addr;
extern uint32_t 			batch_start_address, glTotal_SCT_TransAmount, total_coin_values;
extern uint16_t 			Batch_Index_Sent, glTotal_SCT_TransCount;
extern uint8_t 				/*queue_pending_1,*//* in_prepay_parking[MSM_MAX_PARKING_BAYS],*/queue_pending;//not used in this program //vinay
//extern QUEUE                comm_queue;

//extern uint8_t				batt_threshold_level[];

//extern uint16_t             Present_value;
extern uint8_t				Meter_Reset;

extern uint8_t              OLTAck_Q_count,
	                       	Coin_Q_count,
							BayStatus_Q_count,
							OLT_Q_count,
							Alarms_Q_count,
							SCT_Q_count,
							/*Audit_Q_count,*/
							EVENTS_Q_COUNT,
							PAMBayStatus_Q_count;//not used in this program //vinay
extern double 				RxPacketRssiValue;
extern tLoRaSettings 		LoRaSettings, LoRaSettings1;
extern uint8_t 				LORA_PWR;
extern uint8_t				ELIGIBLE_FOR_ZERO_OUT;
extern uint8_t				glSNSRdiag_string[68];
extern uint16_t			    occupancygl;
extern int16_t 				glSNSR_TMP;
extern uint32_t				last_payment_try_time[MSM_MAX_PARKING_BAYS];
extern REFUND_UPDATE 		last_refund_update;
extern uint8_t				CFG_file_sizearr[4];
extern uint32_t 			SC_Serial_Num;
extern uint8_t				LCD_BKLT_ON;
extern Maintenance_Evt      Maintenance_Log;
extern GPRS_BayStatusRequest  			glBay_req;
extern FlashFieActvnTblEntry   			flash_file_actvn_tbl[ FILE_TYPE_TOTAL_FILES ];
//extern uint16_t            	Coin_Event_Max, Coin_Event_Min;
//extern uint8_t              Coin_Event_Rat1, Coin_Event_Rat2;
extern uint8_t 				Connected_Card_reader, HARDWARE_REVISION;
//extern uint8_t              RFLRState;
extern Coin_min_max_diag	Failed_Coins_diag[MAX_FAILED_COINS_DIAG];
extern uint8_t				failed_coin_counter, glLast_reported_space_status[MSM_MAX_PARKING_BAYS], ZERO_OUT_ENABLE_AT_EXPIRY[MSM_MAX_PARKING_BAYS], ZERO_OUT[MSM_MAX_PARKING_BAYS];
extern uint32_t				MDM_Total_On_Time,Total_LCD_Backlight_On_Time,Total_LORA_PWR_On_Time,Total_LORA_PWR_On_Time_DC;
//extern uint32_t				min_col_counter_read, accumulated_value_to_report;
uint32_t				min_col_counter_read1=0, accumulated_value_to_report1=0;
extern uint16_t				coin_type0;
extern uint16_t				coin_type1;
extern uint16_t				coin_type2;
extern uint16_t				coin_type3;
extern uint16_t				coin_type_invalid;
extern uint16_t				coin_type[20];
extern uint16_t				temp_coin_type[20];
extern uint8_t   			REV4_LED_CONFIG; //to know hardware details in backend //vinay
extern uint8_t				Front_Bi_Color;
extern volatile uint8_t 	current_rate_index;
//extern volatile uint32_t	parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];//in seconds ////not suing in this program //vinay
extern uint8_t				cc_approved; // When the network issuse is there we are getting approve twice and motorist is getting double parking clock// to fix it //vinay
/*
***********************************************************************
*Exported Global Variables
***********************************************************************
*/
volatile uint8_t 			Asynchronous_FD = FALSE;
volatile uint8_t		    wait_counter = 0;
volatile uint8_t			BSU_Chunk_received = FALSE;

uint8_t                     glDo_rtc_update;//, glFlash_rtc_update;
uint8_t              		glClose_socket = FALSE;
uint8_t              		Serial_no_length;           //31-01-12
//uint16_t			        glHB_interval_time_elapsed; //2703: Power mgmt//not used in this program //vinay
//uint8_t              		TELIT_SHUTDOWN = TRUE;//not used in this program //vinay
uint8_t						packet_start;
uint8_t         			glComm_failure_count = 0;                 //2508
uint8_t         			glComm_response[GPRS_GEN_BUF_SZ_SMALL];
uint8_t         			glDFG_response[GPRS_GEN_BUF_SZ_VLARGE]; //1810: changed from LARGE to VLARGE
uint8_t         			olt_sz;
//uint8_t						first_file = FALSE;//not used in this program //vinay
uint8_t         			/*DELAYED_BSU_BATCHED = FALSE,*/CFG_ERASE=FALSE;//not used in this program //vinay
uint8_t         			last_downloaded_chunks = 0;
uint8_t         			Special_Action_Trigger = TRUE;
GPRS_General_Sync_Request	gprs_general_sync_request;
GPRS_General_Sync_Response  gprs_general_sync_response;
QUEUE               		comm_queue;
//uint8_t             		lbay_status_count,Diag_Update;//not used in this program //vinay
uint32_t 					last_pktsent_time = 0, last_LoRa_pktsent_time = 0;
uint8_t						Packet_Index = 0, LoRa_Packet_Index = 0, Waiting_4GPRS_Resp = FALSE;
uint8_t						dfs_done_ready_to_activate = FALSE;
uint8_t 					/*Queue_SendReceive = false,*/In_Queue_Count = 0, LoRa_Queue_Count = 0;//not used in this program //vinay
uint8_t         			request_events[MAX_PACKET_LENGTH];
uint8_t						LoRa_tx_queue[MAX_LORA_PACKET_LENGTH];
uint16_t              		glTotal_5_cents = 0,
                            glTotal_10_cents = 0,
                            glTotal_25_cents = 0,
                            glTotal_100_cents = 0,
                            glTotal_0_cents = 0;//,
                           /* unknown_count = 0;*///not used anywhere in the code program //vinay
uint16_t					GPRS_comms_attempt_count = 0,
							GPRS_comms_success_count = 0,
							LoRa_comms_attempt_count = 0,
							LoRa_comms_success_count = 0,
							Original_packet_count = 0,
							Modem_On_Count = 0,
							bkp_original = 0,
							bkp_gprs_attempt = 0,
							bkp_gprs_success = 0,
							bkp_lora_attempt = 0,
							bkp_lora_success = 0,
							bkp_mdm_on = 0,
							/*bkp_total_invalid_coin = 0,
							bkp_total_valid_coin = 0,
							bkp_total_coin = 0,
							bkp_5cents = 0,
							bkp_10cents = 0,
							bkp_25cents = 0,
							bkp_100cents = 0,*/
							bkp_Events_count = 0;//not used in this program //vinay

uint16_t             		glTotal_Valid_Coin_Count = 0, glTotal_Coin_Count = 0;

uint8_t						Queue_failure_count_l = 0, Batch_clear_continuous_fail = 0;;
uint8_t              		PBC_expt_rcvd[4];
uint8_t              		PBC_amnt_rcvd[2];

uint8_t  					temp_max=0;
//uint16_t     				loCurrentTemp_d;
//uint16_t 					loCurrentTemp_f;
GPRS_Queue 					Queue[MAX_QUEUE_COUNT];
LORA_Queue 					LoRa_Queue[MAX_QUEUE_COUNT];
uint8_t 					do_FD = FALSE;
uint32_t					Last_Comms_Success_RTCTimestamp = 0;
//uint32_t					Last_Sensor_Success_RTCTimestamp = 0;//not used in this program //vinay
uint8_t 					Last_Comms_Success_Flg = FALSE;
uint8_t 					waiting_for_batch_response=false;
bool 						SEND_SEQ_NUM_CHK_FLAG = FALSE;
//uint16_t					SEQ_NUM_VALUE;//not used in this program //vinay
uint16_t					received_area_num;
uint16_t					received_cust_id;
uint16_t					received_meter_id;
uint16_t					received_curr_id = 0;
uint16_t					LNG_Meter_ID = 0;
//GPRS_PamBayStatusUpdateRequest  PAM_Bay_status_data[MAX_PAM_BSU_QUEUE];
GPRS_AlarmsRequest   		glAlarm_req;
//uint8_t 					PAM_Baystatus_update_count = 0;
//uint8_t						do_PAM_Baystatus_update = FALSE;
//uint8_t 					WPM_BAY_STATUS_COUNT=0,FILTERED_BAY_STATUS_COUNT=0;//not used in this program //vinay
uint8_t 					Current_bayStatus = 0, Current_ZO_eligibility;
uint8_t						DFG_Chunk_received = FALSE;
uint8_t						LQI_diagnostic[MAX_DIAG_READINGS] = { 0 }, LQI_counter;
//uint8_t 					factory_test_mode = 1;//not used in this program //vinay
uint8_t						Waiting_4LORA_Resp = FALSE;
//uint8_t 					PBC_Arry[10] = {0};//not used in this program //vinay
uint32_t					Last_processed_time[Max_Sensor] = { 0 };
uint32_t					current_extracted_time[Max_Sensor] = { 0 };
uint32_t					Current_extracted_time_PBC[Max_Sensor] = { 0 };
uint32_t					Last_processed_time_PBC[Max_Sensor] = { 0 };
uint32_t					Current_extracted_time_PBC_for_other_MID[Max_Sensor] = { 0 };
uint32_t					Last_processed_time_PBC_for_other_MID[Max_Sensor] = { 0 };
uint8_t						PBC_other_MID_tx[Size_Ten];
uint8_t						PBC_to_Other_Meter = FALSE;
uint8_t 					Seq_num = 255;


uint32_t  					bkp_MDM_Total_On_Time,bkp_Total_LORA_PWR_On_Time,bkp_Total_LCD_Backlight_On_Time,bkp_Total_LORA_PWR_On_Time_DC;
//extern uint32_t 			Last_Cash_Payment_RTC;//not used in this program //vinay
extern volatile uint8_t 	current_rate_index;
uint8_t						Start_Hour_Coin_Diag = 0;
uint8_t						Start_Minute_Coin_Diag = 0;
uint8_t						Start_Day_Coin_Diag = 0;
uint16_t					Start_Month_Coin_Diag = 0;

//To copy default/config apn -- 18022022
extern uint8_t             default_apn[GPRS_GEN_BUF_SZ_TINY];
extern uint8_t             default_UDP_server_ip[ GPRS_IPADD_BUF_SZ ];
extern uint16_t            default_UDP_server_port;

extern bool                t_mobile_selected_apn_retry_flag;
/*
***********************************************************************
*Local Variables
***********************************************************************
*/
static uint8_t      		request_olt[GPRS_GEN_BUF_SZ_SMALL],
                    		request_events_cache[GPRS_GEN_BUF_SZ_SMALL];
static uint8_t      		request_small[GPRS_GEN_BUF_SZ_SMALL];

/**************************************************************************/
//! Frame Request Header for UDP packet
//! \param uint8_t Communication type
//! \param uint8_t Flag
//! \param uint8_t * pointer to the communication buffer
//! \param uint16_t size the communication buffer
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_fill_request_hdr( uint8_t type, uint8_t flag, uint8_t * buf_p, uint16_t buf_sz, uint8_t MSM_transaction )
{
	uint8_t *        start_p = buf_p;

	if((Special_Action_Trigger == TRUE) && (type != GPRS_BATCHED_OLT_PCHSD_TIME))
	{
		if(type != LORA_NODE_RTC)
		{
			flag |= BIT_4;
			Special_Action_Trigger = FALSE;
		}
	}
	*((uint8_t*)buf_p)      = flag;
	buf_p                  += sizeof( uint8_t );

    *((uint8_t*)buf_p)      = type;
    buf_p                  += sizeof( uint8_t );

    *((uint16_t*)buf_p)     = glSystem_cfg.area_num;
    buf_p                  += sizeof( uint16_t );

    *((uint16_t*)buf_p)     = glSystem_cfg.cust_id;
    buf_p                  += sizeof( uint16_t );

    if(MSM_transaction == TRUE)
    	*((uint16_t*)buf_p)     = (glSystem_cfg.meter_id + Current_Space_Id);
    else
    	*((uint16_t*)buf_p)     = glSystem_cfg.meter_id;
    buf_p                  += sizeof( uint16_t );

    return (uint8_t) (buf_p - start_p);
}

/**************************************************************************/
//! Get Request header parameters from gcf config structure
//! \param uint8_t * pointer to the communication buffer
//! \param uint16_t size the communication buffer
//! \param GPRS_GeneralRequestHeader structure
//! \return void
/**************************************************************************/
void gprs_get_request_hdr( uint8_t * buf_p, uint16_t buf_sz, GPRS_GeneralRequestHeader * grq_hdr  )
{

	grq_hdr->ch.flag      = *((uint8_t*)buf_p);
    buf_p                += sizeof( uint8_t );

    grq_hdr->ch.type      = *((uint8_t*)buf_p);
    buf_p                += sizeof( uint8_t );

    grq_hdr->area_number  = *((uint16_t*)buf_p);
    buf_p                += sizeof( uint16_t );

    grq_hdr->customer_id = *((uint16_t*)buf_p);
    buf_p               += sizeof( uint16_t );

    grq_hdr->meter_id    = *((uint16_t*)buf_p);
    buf_p               += sizeof( uint16_t );
}

/**************************************************************************/
//! Frame response header for received response from server
//! \param uint8_t * pointer to the communication buffer
//! \param uint16_t size the communication buffer
//! \param GPRS_GeneralRequestHeader structure
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_fill_response_hdr( uint8_t * buf_p, uint16_t buf_sz, GPRS_GeneralResponseHeader * resp_hdr_p )
{
    uint8_t        * start_p = buf_p;

    resp_hdr_p->ch.flag                      = GPRS_GET_UINT8( buf_p );
    buf_p                                   += sizeof(resp_hdr_p->ch.flag);

    if(((resp_hdr_p->ch.flag) & BIT_4) == BIT_4)//request general sync to handle special action required
    {
    	if((Asynchronous_FD == FALSE) && (do_FD == FALSE)) //Avoid during downloads
    	{
    		Debug_TextOut(0, "Force General Sync");
    		Tasks_Priority_Register |= GENERAL_SYNC_TASK;
    	}
    	else
    	{
    		//Debug_TextOut(2, "Force General Sync received, but not effective with this packet");
    	}
    }

    resp_hdr_p->ch.type                      = GPRS_GET_UINT8( buf_p );
    buf_p                                   += sizeof( resp_hdr_p->ch.type );

    resp_hdr_p->server_resp.txn_resp_code    = GPRS_GET_UINT16( buf_p );
    buf_p                                   += sizeof( resp_hdr_p->server_resp.txn_resp_code );

    resp_hdr_p->server_resp.ripnet_resp_code = GPRS_GET_UINT16( buf_p );
    buf_p                                   += sizeof( resp_hdr_p->server_resp.ripnet_resp_code );

    resp_hdr_p->content_length               = GPRS_GET_UINT32( buf_p );
    buf_p                                   += sizeof( resp_hdr_p->content_length );

    return (uint8_t)(buf_p - start_p);
}

#if	0
/**************************************************************************/
/*  Name        : do_request_once                                         */
/*  Parameters  : uint16_t recv_byts, uint8_t *buf_p                      */
/*  Returns     : static int                                              */
/*  Function    : Do communication - socket open,send,receive             */
/*------------------------------------------------------------------------*/
static uint8_t resp_hdr_verify( uint16_t recv_byts, uint8_t *buf_p )
{
    uint16_t        cont_len;

    if ( recv_byts >= UDP_MIN_RESP_SIZE )
    {
        buf_p += sizeof( uint8_t ) + sizeof( uint8_t ) + sizeof( uint16_t ) + sizeof( uint16_t );

        cont_len = GPRS_GET_UINT32( buf_p );

        if ( recv_byts >= UDP_MIN_RESP_SIZE + cont_len )
        {
            return TRUE;
        }
    }

    return FALSE;
}
#endif

/**************************************************************************/
//! Do communication with server - socket open,send
//! \param uint8_t * pointer to the communication buffer
//! \param uint16_t size of data to send
//! \param uint8_t not_processing_batch - flag to identify online or offline communication
//! \param uint8_t this_is_in_fd - flag to identify file download or other communication packet
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t do_request_internal( uint8_t *request_p, uint16_t request_sz, uint8_t not_processing_batch, uint8_t this_is_in_fd)
{
	uint8_t        result = GPRS_COMMERR_NONE, i;
	uint8_t        communication_type = request_p[1];
	uint16_t 	   wait=0;

	/*if((communication_type!=GPRS_COMM_lOB_DFG) && (communication_type!=GPRS_COMM_lOB_DFS))
	{
		telit_wakeup_sleep(FALSE); //Remove later
	}*/

	//if(((communication_type!=GPRS_COMM_lOB_DFG) && (communication_type!=GPRS_COMM_lOB_DFS)) || (LAST_LOB_DFS == TRUE))
	if(this_is_in_fd == false)	//LNGSIT-745,785,696
	{
		telit_wakeup_sleep(FALSE); //Flow Changed, socket open is done outside for all packets except DFG, DFS
		if(not_processing_batch == true)
		{
			if(communication_type == GPRS_COMM_lOB_General_Sync)
			{
				Debug_TextOut(0,"Send General Sync");
			}
			else if(communication_type == GPRS_COMM_OLT_AES)
			{
				Debug_TextOut(0,"Send OLT");
			}
			else if(communication_type == GPRS_COMM_CT)
			{
			    Flash_RTC_ParkingClock_Storage(true, false); // to avoid sudden reset of meter and erasing the parking clock durango issue //vinay
				Debug_TextOut(0,"Send CT 55");
			}
			else if(communication_type == GPRS_COMM_CT_PROFILE)
			{
			    Flash_RTC_ParkingClock_Storage(true, false); // to avoid sudden reset of meter and erasing the parking clock durango issue //vinay
				Debug_TextOut(0,"Send CT 125");
			}
			else if(communication_type == GPRS_COMM_lOB_DFS)
			{
				Debug_TextOut(0,"Send DFS");
			}
			else if(communication_type == GPRS_COMM_BINARY_EVENTS)
			{
				if((request_p[9] | (request_p[10]<<8)) == UDP_EVTTYP_DIAG_INFO)
				{
					Debug_TextOut(0,"Send Diag Evt");
				}
				else if ((request_p[9] | (request_p[10]<<8)) == UDP_EVTTYP_ALARM)
				{
					Debug_TextOut(0,"Send Alarms");
				}
				else
					Debug_TextOut(0,"Send Evts");
			}
			else if(communication_type == GPRS_COMM_LOB_BayStatus_diagnostics)
			{
				Debug_TextOut(0,"Send BSU");
			}
		}
		else
		{
			//Debug_Output1(0,"size:%d",request_sz);
			//Debug_Output6(0,":%d %d %d %d %d %d",request_p[0],request_p[1],request_p[2],request_p[3],request_p[4],request_p[5]);
		}
	}
	else
	{
		telit_wakeup_sleep(FALSE);
		DelayMs(1000);
		result = telit_sock_open( communication_type );
		DelayMs(1000);
		if (( result != SOCERR_NONE ) && ( result != SOCERR_BUSY ))
		{
			result = GPRS_COMMERR_SOCK_OPEN_ERR;
			diag_text_Screen( "SocOpn Fail", FALSE, FALSE);
			//Debug_TextOut(0,"SocOpn Fail");
		}
		else
		{
			result = GPRS_COMMERR_NONE;
			//diag_text_Screen( "SocOpnd", FALSE );
		}

	}

	for ( i = 0;  i < TELIT_DRV_SOC_SND_n_RCV_RETRIES;  ++i )//OTA VT
	{

#if 0
        Debug_Output1(0,"request_sz=%d",(request_sz));

        int count;
        uint8_t value[10];
        for (count = 0; count < request_sz; count++)
        {
            liberty_sprintf((char*)value, "%x ", request_p[count]);
            Debug_Display(0, (char*)value);
        }
        Debug_Display(0, "\r\n");
#endif
		if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && ((glSystem_cfg.Enable_GW_Comms_Coin_Only == TRUE) || (glSystem_cfg.Coin_Only_Temporary_GW_Enable == TRUE)))//3oct18
		{
			result = Transactions_via_LoRa(request_p, request_sz);
		}
		else
		{
			result = telit_sock_send ( request_p, request_sz );
		}

		if ( result != SOCERR_NONE )
		{
			glComm_failure_count++;
			diag_text_Screen( "Snd Fail", FALSE, FALSE );
			//Debug_TextOut(0,"Snd Fail");
			result = GPRS_COMMERR_SOCK_SEND_ERR;
		}
		else
		{
			result = GPRS_COMMERR_NONE;

			//if(((communication_type!=GPRS_COMM_lOB_DFG) && (communication_type!=GPRS_COMM_lOB_DFS)) || (LAST_LOB_DFS == TRUE))
			if(this_is_in_fd == false)	//LNGSIT-745,785,696
			{
				return result;
			}
			else
			{
				if((gprs_general_sync_response.total_FD_chunks - Downloaded_chunks) > 2)
				{
					last_downloaded_chunks = Downloaded_chunks;
					wait_counter = 0;
					packet_start = FALSE;
					for(wait=0; wait<300; wait++)
					{
						watchdog_pat();
						DelayMs(1000);

						if(wait_counter >= 10)//changed wait time to retry DFS from 20 seconds to 10 seconds
							break;
						//Commented below ineffective code
/*						else if(wait_counter == 200)
						{
							memset(glMdmUart_recv_buf,0,GPRS_GEN_BUF_SZ_VLARGE);
							glMdmUart_bytes_recvd = 0;

							telit_wakeup_sleep(TRUE);
							UCA0IE |= UCRXIE;
						}*/
						if((gprs_general_sync_response.total_FD_chunks - Downloaded_chunks) < 2)
						{
							watchdog_pat();
							DelayMs(4000);
							break;
						}
						wait_counter++;
					}
				}
				else
				{
					//Debug_TextOut(0,"came here");
					watchdog_pat();
					DelayMs(4000);
					watchdog_pat();
					DelayMs(4000);
				}

				if(Downloaded_chunks != last_downloaded_chunks)
					return GPRS_COMMERR_NONE;
				else
					return GPRS_COMMERR_SOCK_RECV_ERR;
			}
		}
	}
	return result;
}

/**************************************************************************/
//! Adds Communication Request to RAM Queue
//! \param uint8_t * pointer to the communication buffer
//! \param uint16_t size of data to send
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_do_request( uint8_t *request_p, uint16_t request_sz )
{
	uint8_t        result = GPRS_COMMERR_INTERNAL_PROCESS;
	uint8_t		   Que_count;
	//uint16_t 	   i = 0;
	//uint8_t 	   packet_retry =0;

	if ( glSystem_cfg.meter_id == 0 || glSystem_cfg.cust_id == 0 || glSystem_cfg.area_num == 0 )
	{
		Debug_TextOut( 0, "No COMMs possible" );
		return GPRS_COMMERR_INTERNAL_PROCESS;
	}

	/*Debug_Output6(0,"RTC_Request_GW: %02X %02X %02X %02X %02X %02X", request_p[0], request_p[1],
			request_p[2], request_p[3], request_p[4], request_p[5]);
	Debug_Output6(0,"RTC_Request_GW: %02X %02X %02X %02X %02X %02X", request_p[6], request_p[7],
				request_p[8], request_p[9], request_p[10], request_p[11]);*/

	//16Jan2018: Transactions and PUSH_ACK are to be tried via Dongle first, only on failure use GPRS
	if((((request_p[1] == GPRS_COMM_CT) || (request_p[1] == GPRS_COMM_CT_PROFILE) || (request_p[9] == 163) || (request_p[1] == GPRS_COMM_PAMPushAck) || (request_p[1] == GPRS_COMM_Evt)
			||(request_p[1] == GPRS_COMM_LOB_BayStatus_diagnostics)||(request_p[1] == GPRS_COMM_PAMBayStatusUpdate)) && (glSystem_cfg.transactions_via_GPRS == FALSE))	//Use GPRS modem only if specified in config
		|| ((request_p[1] == GPRS_COMM_OLT_AES) && (glSystem_cfg.OLT_via_GPRS == FALSE)))	//Separate Config for OLT
	{
		for(Que_count=0;Que_count<MAX_QUEUE_COUNT;Que_count++)
		{
			if((LoRa_Queue[Que_count].Packet[1] == 0) && (request_sz <= MAX_LORA_PACKET_LENGTH) && (LoRa_Queue[Que_count].Retrycount == 0))
			{
				LoRa_Queue[Que_count].Request_size = request_sz;
				memcpy(LoRa_Queue[Que_count].Packet,request_p,request_sz);
				LoRa_Queue_Count++;
				Original_packet_count++;
				result = GPRS_COMMERR_NONE;
				Debug_Output2( 0, "Added Req_Type: %d, LORA_Q: %d", LoRa_Queue[Que_count].Packet[1], Que_count);
				break;
			}
		}
		if(result != GPRS_COMMERR_NONE)
		{
			Debug_Output2( 0, "LORA Failed, pkt:%d, size:%d", request_p[1], request_sz);
		}
		//return result;
	}
	else if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && ((glSystem_cfg.Enable_GW_Comms_Coin_Only == TRUE) || (glSystem_cfg.Coin_Only_Temporary_GW_Enable == TRUE))//3oct18
			&& ((((request_p[9] | (request_p[10]<<8)) != UDP_EVTTYP_DIAG_INFO) && (request_p[1] == GPRS_COMM_BINARY_EVENTS))
			|| ((request_p[5] == LORA_NODE_RTC) || (request_p[1] == GPRS_COMM_CT)||(request_p[1] == GPRS_COMM_CT_PROFILE)||(request_p[1] == GPRS_SSM_AUDITINFO))))
	{
		//Debug_Output6(0,"new lora: %d,%d,%d,%d,%d,%d", glSystem_cfg.Disable_GPRS_Comms_Coin_Only, glSystem_cfg.Enable_GW_Comms_Coin_Only, (request_p[9] | (request_p[10]<<8)), request_p[1], request_p[5], request_p[4]);
		for(Que_count=0;Que_count<MAX_QUEUE_COUNT;Que_count++)
		{
			if((LoRa_Queue[Que_count].Packet[1] == 0) && (request_sz <= MAX_LORA_PACKET_LENGTH) && (LoRa_Queue[Que_count].Retrycount == 0))
			{
				LoRa_Queue[Que_count].Request_size = request_sz;
				memcpy(LoRa_Queue[Que_count].Packet,request_p,request_sz);
				LoRa_Queue_Count++;
				Original_packet_count++;
				result = GPRS_COMMERR_NONE;
				if(request_p[5] == LORA_NODE_RTC)
					Debug_Output2( 0, "Added Req_Type: %d, LORA_Q: %d", LoRa_Queue[Que_count].Packet[5], Que_count);
				else
					Debug_Output2( 0, "Added Req_Type: %d, LORA_Q: %d", LoRa_Queue[Que_count].Packet[1], Que_count);
				break;
			}
		}
		if(result != GPRS_COMMERR_NONE)
		{
			Debug_Output2( 0, "LORA Failed, pkt:%d, size:%d", request_p[5], request_sz);
		}
	}
	else if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (request_p[1] == FILE_UPDATE_TASK)//3oct18
			|| (request_p[1] == PAM_UPDATE_TASK) || (request_p[1] == FILE_DOWNLOAD_TASK))
	{
		return GPRS_COMMERR_INTERNAL_PROCESS; //Do nothing
	}
	else
	{
		if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == FALSE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))//3oct18
		{
			for(Que_count=0;Que_count<MAX_QUEUE_COUNT;Que_count++)
			{
				if((Queue[Que_count].Packet[1] == 0) && (request_sz <= MAX_PACKET_LENGTH) && (Queue[Que_count].Retrycount == 0))
				{
					Queue[Que_count].Request_size = request_sz;
					memcpy(Queue[Que_count].Packet,request_p,request_sz);
					In_Queue_Count++;
					Original_packet_count++;
					result = GPRS_COMMERR_NONE;
					Debug_Output2( 0, "Added Req_Type: %d, RAM_Q: %d", Queue[Que_count].Packet[1], Que_count);
					break;
				}
			}
			if(result != GPRS_COMMERR_NONE)
			{
				Debug_Output2( 0, "RAM Failed, pkt:%d, size:%d", request_p[1], request_sz);
			}
			Tasks_Priority_Register |= PROCESS_RAM_QUEUE_TASK;
			//return result;
		//	Debug_TextOut( 0, "checkiing");

		}
	}
	return result;
}

/**************************************************************************/
//! Generates the OLT reconciliation packet and adds to communication queue
//! \param pointer to the OLT reconciliation request structure
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_do_OLTACK(GPRS_OLTACKRequest * request_p)
{
	uint8_t     result = GPRS_COMMERR_BUSY;
    uint8_t                     * req_p = request_olt;
    uint16_t                    req_sz;

    //diag_text_Screen( "SND OLT_ACK", FALSE );

    req_p += gprs_fill_request_hdr( GPRS_COMM_Evt, GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof( request_olt ), true );

    memcpy( req_p, request_p->receipt_num, sizeof( request_p->receipt_num ) );
    req_p                  += sizeof( request_p->receipt_num );

    *((uint8_t*)req_p)      = request_p->olt_success;
    req_p                  += sizeof( request_p->olt_success );

    *((uint8_t*)req_p)      = request_p->paid_until_ts[0];
    req_p                  += sizeof(uint8_t );

    *((uint8_t*)req_p)      = request_p->paid_until_ts[1];
    req_p                  += sizeof(uint8_t );

    *((uint8_t*)req_p)      = request_p->paid_until_ts[2];
    req_p                  += sizeof(uint8_t );

    *((uint8_t*)req_p)      = request_p->paid_until_ts[3];
    req_p                  += sizeof(uint8_t );

    *((uint8_t*)req_p)      = (uint8_t) (request_p->bay_no);
    req_p                  += sizeof(uint8_t );

    *((uint8_t*)req_p)      = (request_p->bay_no>>8);
    req_p                  += sizeof(uint8_t );

    //2102:txn roll-up
    *((uint8_t*)req_p)      = (uint8_t) (request_p->earned_time);
    req_p                  += sizeof(uint8_t );

    *((uint8_t*)req_p)      = (uint8_t) (request_p->earned_time>>8);
    req_p                  += sizeof(uint8_t );
    //2102:txn roll-up

    req_sz = (uint16_t) (req_p - request_olt);

    result = gprs_do_request( request_olt, req_sz );

    return result;
}

/**************************************************************************/
//! Generates the OLT packet and adds to communication queue
//! \param pointer to the OLT request structure
//! \param pointer to the OLT response structure
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_do_OLT( GPRS_OLTRequest * request_p, GPRS_OLTResponse * response_p)
{
    uint16_t                    i, j, len;
    uint8_t                   * req_p_temp;
    uint8_t                   * req_p        = request_olt;
    uint8_t                     result       = GPRS_COMMERR_BUSY;

    memset( response_p->acq_ref, 0, sizeof( response_p->acq_ref ) );

    memset( request_olt, 0, sizeof(request_olt) );

    response_p->acq_ref_length   = 0;
    response_p->card_balance     = 0;
    response_p->auth_code        = GPRS_OLT_GWAY_RESP_Waiting;

    req_p += gprs_fill_request_hdr( GPRS_COMM_OLT_AES, GPRS_REQ_ENCRYPT_RESP_UNENCRYPT, req_p, sizeof( request_olt ), true );    //2102:txn roll-up, changed type from 21 to 41

    *((uint16_t*)req_p)     = AES_KEY_ID;
       req_p                  += sizeof( AES_KEY_ID );

    req_p_temp              =  req_p;
    req_p                  += sizeof(uint16_t);

    *((uint32_t*)req_p)     = request_p->earned_mins;   //2102: txn roll-up
    req_p                  += sizeof( request_p->earned_mins );          //2102: txn roll-up

    *((uint32_t*)req_p)     = request_p->time_stamp;
    req_p                  += sizeof( request_p->time_stamp );

    *((uint8_t*)req_p)      = request_p->multiple_event_counter;
    req_p                  += sizeof( request_p->multiple_event_counter );

    *((uint8_t*)req_p)      = request_p->card_type;
    req_p                  += sizeof( request_p->card_type );

    *((uint16_t*)req_p)     = request_p->amount;
    req_p                  += sizeof( request_p->amount );

    memcpy( req_p, request_p->ref_id, sizeof( request_p->ref_id ) );
    req_p                  += sizeof( request_p->ref_id );

    *((uint32_t*)req_p)     = request_p->paid_until;
    req_p                  += sizeof( request_p->paid_until );

    *((uint16_t*)req_p)     = request_p->bay_no;
    req_p                  += sizeof( request_p->bay_no );

    if(T2_Faw_flag==1)//1908
    {
        for (i=0; i<sizeof(request_p->track2)-2; i++)
        {
            if (request_p->track2[i]== 'F' )
            {
                request_p->track2[i+1]= 'F';
                for(j=1;j<(sizeof(request_p->track2)-i);j++)
                {
                    request_p->track2[i+1+j] = '\0'; //Null terminate but keep the LRC
                }
                break;
            }
        }

        memcpy( req_p, request_p->track2, sizeof( request_p->track2 ) );
        req_p += sizeof( request_p->track2 );
    }
    //else if((T1_Faw_flag==1) && (glSystem_cfg.CC_TRACK1_ENABLE == TRUE))//1908
    else if(T1_Faw_flag==1)//1908
    {
        for (i=0; i<sizeof(request_p->track1)-2; i++)
        {
            if (request_p->track1[i]== '?' )
            {
                for(j=1;j<(sizeof(request_p->track1)-i);j++)
                {
                    request_p->track1[i+1+j] = '\0'; //Null terminate but keep the LRC
                }
                break;
            }
        }
        memcpy( req_p, request_p->track1, sizeof( request_p->track1 ) );
        req_p += sizeof( request_p->track1 );
    }

    //diag_text_Screen( "SND OLT\n", FALSE );

    len = (uint16_t) ( ((uint8_t *)req_p - (uint8_t *)req_p_temp) );

    *((uint16_t*)req_p_temp)     = len - sizeof(uint16_t);

    multiple_block_aes_encrypt((uint8_t*)(req_p_temp + sizeof(uint16_t)),len-sizeof(uint16_t), AES_KEY);//14-05-2013:DPLIBB-481

    olt_sz = (uint8_t) (req_p - request_olt);    //2211

    //if(glSystem_cfg.Force_CC_batch==FALSE)   //02-01-2014:DPLIBB-609
    {
    	result = gprs_do_request( request_olt, olt_sz );
    }
    /*else   //02-01-2014:DPLIBB-609
    {
    	if( request_olt[1] == GPRS_COMM_OLT_AES)//2102: txn roll-up
    	{
    		if(sizeof(comm_queue.batch_request)>=olt_sz)
    		{
    			memset( comm_queue.batch_request, 0, sizeof(comm_queue.batch_request) );
    			memcpy( comm_queue.batch_request, request_olt, olt_sz );
    		}
    	}
    }*/
    return result;
}

/**************************************************************************/
//! Generates the Coin transaction packet and adds to communication queue
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_do_CT(uint8_t spaceID)
{
    uint8_t                        * req_p         = request_events, i;
    uint8_t                        result          = GPRS_COMMERR_BUSY;

	if((((glSystem_cfg.allowed_trans_types>>13) & 0x01) == 1) || (((glSystem_cfg.allowed_trans_types>>14) & 0x01) == 1))
		Coin_Per_Udp_Packet = COINS_PER_UDP_PACKET_CT125;
	else
		Coin_Per_Udp_Packet = COINS_PER_UDP_PACKET_CT55;

	memset( request_events, 0, sizeof(request_events) );

//    req_p += gprs_fill_request_hdr( GPRS_COMM_CT, GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof( request_events ), true );

//============================Header Part======================================================
    *((uint8_t*)req_p)      = GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT;
    req_p                  += sizeof( uint8_t );
	
	if(Coin_Per_Udp_Packet==COINS_PER_UDP_PACKET_CT55)
	{
    *((uint8_t*)req_p)      = GPRS_COMM_CT;
    req_p                  += sizeof( uint8_t );
	}
	else
	{
    *((uint8_t*)req_p)      = GPRS_COMM_CT_PROFILE;
    req_p                  += sizeof( uint8_t );		
	}
	
    *((uint16_t*)req_p)     = glSystem_cfg.area_num;
    req_p                  += sizeof( uint16_t );

    *((uint16_t*)req_p)     = glSystem_cfg.cust_id;
    req_p                  += sizeof( uint16_t );

   	*((uint16_t*)req_p)     = (glSystem_cfg.meter_id + spaceID);
   	req_p                  += sizeof( uint16_t );
//===========================================================================================

    *((uint16_t*)req_p)     = DEFAULT_BAY_NUMBER;
    req_p                  += sizeof( uint16_t );

    *((uint8_t*)req_p)      = Tx_Cash;
    req_p                  += sizeof( uint8_t );

    if(lCoinCount > Coin_Per_Udp_Packet)	//LNGSIT-852, 853: Max 9 coins per packet IN CT55 : Max 6 coins per packet IN CT125, OK to loose some coins as it is an exception. We were loosing the complete packet before
    {
    	lCoinCount = Coin_Per_Udp_Packet;
    }

    *((uint8_t*)req_p)      = (uint8_t)lCoinCount;
    req_p                  += sizeof( uint8_t );

    for(i=0; i<lCoinCount; i++)
    {
        *((uint8_t*)req_p)     = lCoindata[i].coin_index;
        req_p                  += sizeof( uint8_t );

        *((uint8_t*)req_p)     = lCoindata[i].coin_index>>8;
        req_p                  += sizeof( uint8_t );

        *((uint8_t*)req_p)     = lCoindata[i].cashtranstime;
        req_p                  += sizeof( uint8_t );

        *((uint8_t*)req_p)     = lCoindata[i].cashtranstime>>8;
        req_p                  += sizeof( uint8_t );

        *((uint8_t*)req_p)     = lCoindata[i].cashtranstime>>16;
        req_p                  += sizeof( uint8_t );

        *((uint8_t*)req_p)     = lCoindata[i].cashtranstime>>24;
        req_p                  += sizeof( uint8_t );

        *((uint8_t*)req_p)     = lCoindata[i].coinval;
        req_p                  += sizeof( uint8_t );

        *((uint8_t*)req_p)     = lCoindata[i].coinval>>8;
        req_p                  += sizeof( uint8_t );

        *((uint8_t*)req_p)     = lCoindata[i].cumulative_parktime_hr;
        req_p                  += sizeof( uint8_t );

        *((uint8_t*)req_p)     = lCoindata[i].cumulative_parktime_min;
        req_p                  += sizeof( uint8_t );

        *((uint8_t*)req_p)     = lCoindata[i].earned_coin_hr;
        req_p                  += sizeof( uint8_t );

        *((uint8_t*)req_p)     = lCoindata[i].earned_coin_min;
        req_p                  += sizeof( uint8_t );
     ////////////////////////////////////////////////////////////////////// to add min max values of the coins
      if((((glSystem_cfg.allowed_trans_types>>13) & 0x01) == 1) || (((glSystem_cfg.allowed_trans_types>>14) & 0x01) == 1))
      //if(Coin_Per_Udp_Packet==COINS_PER_UDP_PACKET_CT125)
        {
		   *((uint8_t*)req_p)     = lCoindata[i].max;
			req_p                  += sizeof( uint8_t );

			*((uint8_t*)req_p)     = lCoindata[i].max>>8;
			req_p                  += sizeof( uint8_t );

			*((uint8_t*)req_p)     = lCoindata[i].min;
			req_p                  += sizeof( uint8_t );

			*((uint8_t*)req_p)     = lCoindata[i].min>>8;
			req_p                  += sizeof( uint8_t );

			*((uint8_t*)req_p)     = lCoindata[i].rat1;
			req_p                  += sizeof( uint8_t );

			*((uint8_t*)req_p)     = lCoindata[i].rat2;
			req_p                  += sizeof( uint8_t );

			//Debug_Output6( 0, "max:%ld, min:%ld, rat1:%d, rat2:%d", lCoindata[i].max, lCoindata[i].min, lCoindata[i].rat1, lCoindata[i].rat2, 0, 0);

        }
   }
    //diag_text_Screen( "SND CT", FALSE );
   result = gprs_do_request( request_events, (uint16_t) (req_p - request_events) );

    return result;
}

#if 0
void gprs_do_BayStatus(GPRS_BayStatusRequest * request_p )
{
	uint16_t                    req_sz;
	uint8_t                     * req_p = request_events;

	Debug_TextOut( 0, "Batch BSU" );

	req_p += gprs_fill_request_hdr( GPRS_COMM_LOB_BayStatus_diagnostics, GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof( request_events ) );//15-10-2013: sensor diag

	*((uint16_t*)req_p)     = 1;
	req_p                  += sizeof( uint16_t);

	*((uint16_t*)req_p)     = 1;
	req_p                  += sizeof(uint16_t);

	*((uint32_t*)req_p)     = request_p->expiry_time;
	req_p                  += sizeof(uint32_t);

	*((uint16_t*)req_p)     = request_p->purchased_duration;
	req_p                  += sizeof(uint16_t);

	*((uint8_t*)req_p)     = request_p->txn_type;
		req_p                  += sizeof(request_p->txn_type);

	*((uint8_t*)req_p)     = 0;//request_p->topup;
		req_p                  += sizeof(uint8_t);

	*((uint8_t*)req_p)     = 0;//request_p->amount_cents;
		req_p                  += sizeof(uint8_t);

	*((uint8_t*)req_p)     = 0;//request_p->amount_cents;
	req_p                  += sizeof(uint8_t);

	*((uint8_t*)req_p) 	= (request_p->PAMtxn_time )& 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (request_p->PAMtxn_time>>8) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (request_p->PAMtxn_time>>16) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (request_p->PAMtxn_time>>24) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (request_p->time_stamp )& 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (request_p->time_stamp>>8) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (request_p->time_stamp>>16) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (request_p->time_stamp>>24) & 0xFF;
	req_p += sizeof( uint8_t );

	//15-10-2013: sensor diag
	*((uint8_t*)req_p) 	= glSystem_cfg.cust_id & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (glSystem_cfg.cust_id>>8) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= glSystem_cfg.area_num & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (glSystem_cfg.area_num>>8) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= glSystem_cfg.meter_id & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (glSystem_cfg.meter_id>>8) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p)    = (request_p->occupancy) & 0xFF;
	req_p                += sizeof( uint8_t );

	*((uint8_t*)req_p) = SENSOR_DIAG_LENGTH & 0xFF;	//BS diagnostics data length
	req_p += 1;

	*((uint8_t*)req_p) = (SENSOR_DIAG_LENGTH>>8) & 0xFF;	//BS diagnostics data length
	req_p += 1;
	//15-10-2013: sensor diag

	*((uint8_t*)req_p)  = (request_p->SNSR_TMP) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p)  = (request_p->SNSR_TMP>>8) & 0xFF;
	req_p += sizeof( uint8_t );

	memcpy(req_p, request_p->diag_string, 68);
	req_p += 68;

	req_sz = (uint16_t)(req_p - request_events);

	Queue_Request( request_events, req_sz);
}
#endif

/**************************************************************************/
//! Generates the Alarm packet and adds to communication queue
//! \param pointer to the Alarm request structure
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_do_Alarms(GPRS_AlarmsRequest * request_alm)
{
    uint16_t                    req_sz;
    uint8_t                        * req_p = request_events;
    uint8_t                         result = GPRS_COMMERR_BUSY;

    memset( request_events, 0, sizeof(request_events) );

    request_alm->noofalarms=1;
    req_p += gprs_fill_request_hdr( GPRS_COMM_BINARY_EVENTS, GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof( request_events ), false );

    *((uint8_t*)req_p)     = request_alm->noofalarms;
    req_p                 += sizeof( request_alm->noofalarms );

    *((uint8_t*)req_p)     = (request_alm->alarm_type) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = ((request_alm->alarm_type)>>8) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = (request_alm->alarm_ts) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = ((request_alm->alarm_ts)>>8) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = ((request_alm->alarm_ts)>>16) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = ((request_alm->alarm_ts)>>24) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = (request_alm->alarm_size) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = ((request_alm->alarm_size)>>8) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = request_alm->alarm_state;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = ((request_alm->alarm_id)>>8) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = (request_alm->alarm_id) & 0xFF;
    req_p                 += sizeof( uint8_t );

    req_sz = (uint16_t)(req_p - request_events);

    //Debug_Output6( 0, "SND Alarm %d,%d,%d", request_alm->alarm_type, request_alm->alarm_id, request_alm->alarm_state, 0, 0, 0 );

    result = gprs_do_request( request_events, req_sz );

    return result;
}

/**************************************************************************/
//! Generates the Events update packet and adds to communication queue
//! \param add_diag_too - parameter to decide if a diagnostic update needs to be sent prior to the events reporting
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_do_send_event_cache( uint8_t add_diag_too, uint8_t event_buff_count2 )
{
	uint16_t                    req_sz, evts_batch_result;
	EventCacheEntry                * ent_p;
	uint8_t                        * tot_event_byte_pos_p, i;
	uint8_t                        evt_count      = 0;
	uint8_t                        * req_p        = request_events_cache;
	uint8_t                        result         = GPRS_COMMERR_NONE;
	uint8_t							indx 		  = event_buff_count2;

	//Debug_TextOut(0,"EVENTS INTERVAL");

	if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == FALSE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))//3oct18
	{
		if ( add_diag_too == true )//1908
		{
			gprs_send_diag_msg();
			if(Meter_Reset == TRUE)	//To avoid resending of boot up events
				return GPRS_COMMERR_NONE;
		}
		else
		{
			//for(indx = 0; indx < MSM_MAX_PARKING_BAYS; indx++)
			{

				memset( request_events_cache, 0, sizeof(request_events_cache) );

				//req_p += gprs_fill_request_hdr( GPRS_COMM_BINARY_EVENTS, GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof( request_events_cache ) );

				*((uint8_t*)req_p)      = 0;
				req_p                  += sizeof( uint8_t );

				*((uint8_t*)req_p)      = GPRS_COMM_BINARY_EVENTS;
				req_p                  += sizeof( uint8_t );

				*((uint16_t*)req_p)     = glSystem_cfg.area_num;
				req_p                  += sizeof( uint16_t );

				*((uint16_t*)req_p)     = glSystem_cfg.cust_id;
				req_p                  += sizeof( uint16_t );

				*((uint16_t*)req_p)     = (glSystem_cfg.meter_id + indx);
				req_p                  += sizeof( uint16_t );

				tot_event_byte_pos_p = (uint8_t*)req_p;
				req_p               += sizeof( *tot_event_byte_pos_p );

				* tot_event_byte_pos_p = 0;//2308

				for ( i = 0, ent_p = glEvent_cache[indx].cache; i < MAX_EVENT_CACHE_SIZE; i ++, ent_p ++ )
				{
					if ( ent_p->flags.bits.dirty == false )
					{
						evt_count ++;

						*((uint8_t*)req_p)     = (uint8_t) (ent_p->id & 0xFF);
						req_p                 += sizeof( uint8_t );

						*((uint8_t*)req_p)     = (uint8_t) (ent_p->id >> 8) & 0xFF;
						req_p                 += sizeof( uint8_t );

						*((uint8_t*)req_p)     = (ent_p->ts) & 0xFF;
						req_p                 += sizeof( uint8_t );

						*((uint8_t*)req_p)     = ((ent_p->ts)>>8) & 0xFF;
						req_p                 += sizeof( uint8_t );

						*((uint8_t*)req_p)     = ((ent_p->ts)>>16) & 0xFF;
						req_p                 += sizeof( uint8_t );

						*((uint8_t*)req_p)     = ((ent_p->ts)>>24) & 0xFF;
						req_p                 += sizeof( uint8_t );

						// 0404: maintenance event
						if((ent_p->id == UDP_EVTTYP_TECH_MENU_ACCESS) || (ent_p->id == UDP_EVTTYP_COLLECTION_EVT))
						{
							*((uint8_t*)req_p)     = (TECH_SL_NO_LEN+1) & 0xFF;
							req_p                 += sizeof( uint8_t );
							*((uint8_t*)req_p)     = ((TECH_SL_NO_LEN+1)>>8) & 0xFF;
							req_p                 += sizeof( uint8_t );

							*((uint8_t*)req_p)     = (Maintenance_Log.TECH_CARD_SL_NO>>24) & 0xFF;
							req_p 				  += sizeof( uint8_t );

							*((uint8_t*)req_p)     = (Maintenance_Log.TECH_CARD_SL_NO>>16) & 0xFF;
							req_p                 += sizeof( uint8_t );

							*((uint8_t*)req_p)     = ((Maintenance_Log.TECH_CARD_SL_NO)>>8) & 0xFF;
							req_p                 += sizeof( uint8_t );

							*((uint8_t*)req_p)     = (Maintenance_Log.TECH_CARD_SL_NO) & 0xFF;
							req_p += sizeof( uint8_t );
						}
						else if(ent_p->id == UDP_EVTTYP_MAINTENANCE_LOG)
						{
							*((uint8_t*)req_p)     = (TECH_SL_NO_LEN+3) & 0xFF;
							req_p                 += sizeof( uint8_t );
							*((uint8_t*)req_p)     = ((TECH_SL_NO_LEN+3)>>8) & 0xFF;
							req_p                 += sizeof( uint8_t );
							*((uint8_t*)req_p)     = ent_p->PROBLEM_CODE;
							req_p                 += sizeof( uint8_t );
							*((uint8_t*)req_p)     = ent_p->SOLUTION_CODE;
							req_p                 += sizeof( uint8_t );

							*((uint8_t*)req_p)     = (Maintenance_Log.TECH_CARD_SL_NO>>24) & 0xFF;
							req_p 				  += sizeof( uint8_t );
							*((uint8_t*)req_p)     = (Maintenance_Log.TECH_CARD_SL_NO>>16) & 0xFF;
							req_p                 += sizeof( uint8_t );

							*((uint8_t*)req_p)     = ((Maintenance_Log.TECH_CARD_SL_NO)>>8) & 0xFF;
							req_p                 += sizeof( uint8_t );

							*((uint8_t*)req_p)     = (Maintenance_Log.TECH_CARD_SL_NO) & 0xFF;
							req_p += sizeof( uint8_t );
						}
						//03-06-2013: voltage level in low battery event
						else if((ent_p->id == UDP_EVTTYP_LOW_BATT_LEVEL1) || (ent_p->id == UDP_EVTTYP_LOW_BATT_LEVEL2) || (ent_p->id == UDP_EVTTYP_LOW_BATT_POWER_SAVE))
						{
							memset(rx_ans, 0, sizeof(rx_ans));
							sprintf((char *)rx_ans, "%d.%02d", ((uint16_t)(ent_p->tech_card_sl_no)/100), ((uint16_t)(ent_p->tech_card_sl_no)%100) );

							*((uint8_t*)req_p)     = (strlen((const char*)rx_ans)) & 0xFF;
							req_p                 += sizeof( uint8_t );
							*((uint8_t*)req_p)     = (strlen((const char*)rx_ans)>>8) & 0xFF;
							req_p                 += sizeof( uint8_t );

							memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
							req_p                 += strlen((const char*)rx_ans);

						}
						//03-06-2013: voltage level in low battery event
						//25-06-2013:DPLIBB-514
						else if((ent_p->id == UDP_EVTTYP_VEHICLE_IN) || (ent_p->id == UDP_EVTTYP_VEHICLE_OUT) ||(ent_p->id == UDP_EVTTYP_DIAG_EXCEPTION)
								|| (ent_p->id == UDP_EVTTYP_ZEROOUT_ON_VACANT) || (ent_p->id == UDP_EVTTYP_ZEROOUT_ON_OCCUPIED) || (ent_p->id == UDP_EVTTYP_FREETIME_ON_ENTRY))//06-04-2015
						{
							*((uint8_t*)req_p)     = (sizeof( uint32_t )) & 0xFF;//06-04-2015
							req_p                 += sizeof( uint8_t );

							*((uint8_t*)req_p)     = (sizeof( uint32_t )>>8) & 0xFF;//06-04-2015
							req_p                 += sizeof( uint8_t );

							*((uint8_t*)req_p)     = (ent_p->tech_card_sl_no>>24) & 0xFF; //06-04-2015
							req_p                 += sizeof( uint8_t );//06-04-2015

							*((uint8_t*)req_p)     = (ent_p->tech_card_sl_no>>16) & 0xFF; //06-04-2015
							req_p                 += sizeof( uint8_t );//06-04-2015

							*((uint8_t*)req_p)     = ((ent_p->tech_card_sl_no)>>8) & 0xFF;	// parking time minutes
							req_p                 += sizeof( uint8_t );

							*((uint8_t*)req_p)     = (ent_p->tech_card_sl_no) & 0xFF; // parking time hours
							req_p                 += sizeof( uint8_t );

						}
						else if(ent_p->id == UDP_EVTTYP_SMARTCARD_REFUND)	//DPLIBFIVE-663: SC Refund Demo
						{
							*((uint8_t*)req_p)     = (8) & 0xFF;
							req_p                 += sizeof( uint8_t );

							*((uint8_t*)req_p)     = (8>>8) & 0xFF;
							req_p                 += sizeof( uint8_t );

							/*memcpy(req_p, last_refund_update.refund_cardid, sizeof(last_refund_update.refund_cardid));
            	req_p                 += sizeof(last_refund_update.refund_cardid);*/

							//Smart card serial number issue fix
							*((uint8_t*)req_p)      = ( uint8_t )((SC_Serial_Num)>>24);
							req_p                  += sizeof(uint8_t );

							*((uint8_t*)req_p)      = ( uint8_t )((SC_Serial_Num)>>16);
							req_p                  += sizeof(uint8_t );

							*((uint8_t*)req_p)      = ( uint8_t )((SC_Serial_Num)>>8);
							req_p                  += sizeof(uint8_t );

							*((uint8_t*)req_p)      = ( uint8_t )(SC_Serial_Num);
							req_p                  += sizeof(uint8_t );

							*((uint8_t*)req_p)      = 0;
							req_p                  += sizeof(uint8_t );

							*((uint8_t*)req_p)      = 0;
							req_p                  += sizeof(uint8_t );


							*((uint8_t*)req_p)     = ((last_refund_update.refund_amount)>>8) & 0xFF;
							req_p                 += sizeof( uint8_t );

							*((uint8_t*)req_p)     = (last_refund_update.refund_amount) & 0xFF;
							req_p                 += sizeof( uint8_t );
						}
						else if(ent_p->id == UDP_EVTTYP_COIN_CALIBRATION_VALUES)
						{
			            	*((uint8_t*)req_p)     = (5) & 0xFF;
			            	req_p                 += sizeof( uint8_t );

			            	*((uint8_t*)req_p)     = ((5)>>8) & 0xFF;
			            	req_p                 += sizeof( uint8_t );

			            	*((uint8_t*)req_p)     = ent_p->PROBLEM_CODE;
			            	req_p                 += sizeof( uint8_t );

			            	*((uint8_t*)req_p)     = (ent_p->tech_card_sl_no>>24) & 0xFF;
			            	req_p                 += sizeof( uint8_t );

			            	*((uint8_t*)req_p)     = (ent_p->tech_card_sl_no>>16) & 0xFF;
			            	req_p                 += sizeof( uint8_t );

			            	*((uint8_t*)req_p)     = ((ent_p->tech_card_sl_no)>>8) & 0xFF;
			            	req_p                 += sizeof( uint8_t );

			            	*((uint8_t*)req_p)     = (ent_p->tech_card_sl_no) & 0xFF;
			            	req_p                 += sizeof( uint8_t );
			            }
						//25-06-2013:DPLIBB-514
						else
						{
							*((uint8_t*)req_p)     = 0;
							req_p                 += sizeof( uint8_t );
							*((uint8_t*)req_p)     = 0;
							req_p                 += sizeof( uint8_t );
						}
						// 0404: maintenance event
					}
				}
			}

			//* evts_posted = evt_count;

			(* tot_event_byte_pos_p) += evt_count;    // for 900 which is diag-msg

			req_sz = (uint16_t) (req_p - request_events_cache);

			if(add_diag_too!=0xff)//17-03-2014:hardware sensor code merge //it was commented in DSM and VT removed it clear RAM Q //vinay
			{
				//Debug_Output1( 0, "Send Events #%d", evt_count );

				if(evt_count>=1)
				{
					result = gprs_do_request( request_events_cache,	req_sz );
				}

				return result;
			}
			else//17-03-2014:hardware sensor code merge
			{
				//write to flash
				if(evt_count >= 1)
				{
					//write to flash
					if(req_sz<=QUEUE_PACKET_SIZE)
					{
						evts_batch_result=batch_events_in_flash(request_events_cache,req_sz);
						if(evts_batch_result==UDP_EVTTYP_EVENTS_BATCHED)
						{
							init_event_cache(indx);
							//push_event_to_cache(UDP_EVTTYP_EVENTS_BATCHED);
							Debug_Output1(0,"Events_count:%d",evt_count);
						}
					}
					else
						Debug_TextOut( 0, "Evts batch exception" );
					//return result;
				}
			}
		}
	}
	return result;
}

/**************************************************************************/
//! Generates the Diagnostic update packet and adds to communication queue
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_send_diag_msg()
{
	uint16_t                    temp, current_index, i, j, k;
	uint16_t                    req_sz;
	uint32_t                    time_stamp;
	uint8_t                    *req_p_temp;                         //1908
	uint8_t                    *tot_event_byte_pos_p;
	uint8_t                    *req_p          = request_events;    //1908
	uint8_t                     data_pkt_count = 0;
	uint8_t                     result         = GPRS_COMMERR_BUSY;
	//uint8_t                     result1        = 0;
	int16_t                     len            = 0;
	uint32_t					lRFFrequency = 0;

	/*int Total_5_cents   = glTotal_5_cents ,
			Total_10_cents  = glTotal_10_cents,
			Total_25_cents  = glTotal_25_cents,
			Total_100_cents = glTotal_100_cents,
			Total_0_cents   = glTotal_0_cents,
			lunknown_count  = unknown_count,
			lpartial_packet = glPartialPacket,
			lCoin_count     = glTotal_Coin_Count;*/

	//Debug_TextOut( 0, "Send Diagnostics\r\n" );
	//MB_RTC = 0;
	//Diag_Update = TRUE;

	memset( request_events, 0, sizeof(request_events) );
	req_p += gprs_fill_request_hdr( GPRS_COMM_BINARY_EVENTS, GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof( request_events ), false );

	tot_event_byte_pos_p = (uint8_t*)req_p;
	req_p += sizeof( *tot_event_byte_pos_p );

	* tot_event_byte_pos_p = 1;
	//Below Modem Parameters read logic changed to give faster UI
	/*	//Power On Modem and force to command mode to read modem parameters
	if(MDM_AWAKE == FALSE)
		telit_power_on();

	glClose_socket = TRUE;

	if ( telit_sock_close() != SOCERR_NONE )
	{
		result = GPRS_COMMERR_SOCK_CLOSE_ERR;
		diag_text_Screen( "SocClos Fail", FALSE );
	}*/

	*((uint8_t*)req_p)     = UDP_EVTTYP_DIAG_INFO & 0xFF;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (UDP_EVTTYP_DIAG_INFO>>8) & 0xFF;
	req_p                 += sizeof( uint8_t );

	time_stamp = RTC_epoch_now();

	*((uint8_t*)req_p)     = time_stamp & 0xFF;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (time_stamp>>8) & 0xFF;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (time_stamp>>16) & 0xFF;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (time_stamp>>24) & 0xFF;
	req_p                 += sizeof( uint8_t );

	req_p_temp = req_p;
	req_p                 += sizeof( uint16_t );    //diag_message_length assigned at the end
	req_p                 += sizeof( uint8_t );     //data_packet count assigned at the end

	//mpb_rev_code
	memset(rx_ans,0,sizeof(rx_ans));
	len = (int16_t) sprintf( (char*)rx_ans,"%d.%d.%d.%d_%s", PCI_VERSION,MPB_VERSION,sub_VERSION,branch_version,"4DSM");

	*((uint8_t*)req_p)     = UDP_DIAGEVT_MPB_VER;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p, rx_ans, len);
	req_p += strlen((const char*)rx_ans);

	//(* tot_event_byte_pos_p) ++;
	data_pkt_count++;
	//mpb_rev_code

	//Modem type
	//memset(rx_ans,0,sizeof(rx_ans));
	//result1 = mdm_fetch_type(rx_ans, sizeof(rx_ans));
	//if(result1==MDM_ERR_NONE)
	memset(rx_ans,0,sizeof(rx_ans));
	memcpy(rx_ans, sys_mdm_p.type, sizeof(sys_mdm_p.type));
	if(sys_mdm_p.type != 0)
	{
		*((uint8_t*)req_p)     = UDP_DIAGEVT_MDM_TYPE;
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
		req_p                 += sizeof( uint8_t );

		memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
		req_p += strlen((const char*)rx_ans);

		//(* tot_event_byte_pos_p) ++;
		data_pkt_count++;
	}
	//Modem type

	//Modem f/w
	//memset(rx_ans,0,sizeof(rx_ans));
	//result1 = mdm_fetch_SW_revision(rx_ans, sizeof(rx_ans));
	//if(result1==MDM_ERR_NONE)
	memset(rx_ans,0,sizeof(rx_ans));
	memcpy(rx_ans, sys_mdm_p.sw_rev, sizeof(sys_mdm_p.sw_rev));
	if(sys_mdm_p.sw_rev != 0)
	{
		*((uint8_t*)req_p)     = UDP_DIAGEVT_MDM_FW_VER;
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
		req_p                 += sizeof( uint8_t );

		memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
		req_p += strlen((const char*)rx_ans);

		//(* tot_event_byte_pos_p) ++;
		data_pkt_count++;
	}
	//Modem f/w

	//IMEI
	//memset(rx_ans,0,sizeof(rx_ans));
	//result1=mdm_fetch_IMEI(rx_ans, sizeof(rx_ans));
	//if(result1==MDM_ERR_NONE)
	memset(rx_ans,0,sizeof(rx_ans));
	memcpy(rx_ans, sys_mdm_p.IMEI, sizeof(sys_mdm_p.IMEI));
	if(sys_mdm_p.IMEI != 0)
	{
		*((uint8_t*)req_p)     = UDP_DIAGEVT_MDM_IMEI;
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
		req_p                 += sizeof( uint8_t );

		memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
		req_p += strlen((const char*)rx_ans);

		//(* tot_event_byte_pos_p) ++;
		data_pkt_count++;
	}
	//IMEI

	//IMSI
	//memset(sys_mdm_p.IMSI,0,sizeof(sys_mdm_p.IMSI));//14-02-12
	//memset(rx_ans,0,sizeof(rx_ans));
	//result1 = mdm_fetch_IMSI(rx_ans, sizeof(rx_ans));//19-04-12
	//if(result1==MDM_ERR_NONE)
	memset(rx_ans,0,sizeof(rx_ans));
	memcpy(rx_ans, sys_mdm_p.IMSI, sizeof(sys_mdm_p.IMSI));
	if(sys_mdm_p.IMSI != 0)
	{
		*((uint8_t*)req_p)     = UDP_DIAGEVT_MDM_IMSI;
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
		req_p                 += sizeof( uint8_t );

		memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
		req_p += strlen((const char*)rx_ans);

		//(* tot_event_byte_pos_p) ++;
		data_pkt_count++;
	}
	//IMSI

	//sys_mdm_p.signal_strength=0;
	//sys_mdm_p.signal_strength=mdm_get_signal_strength();

	//MIN TEMP
	/*memset(rx_ans,0,sizeof(rx_ans));
	temp_min=tempC[0];	//Temperature is already sorted in ascending order while reading
	loCurrentTemp_d = (uint8_t) temp_min;
	loCurrentTemp_f = (uint16_t)((temp_min - loCurrentTemp_d) * 1000.0F);
	sprintf((char *)rx_ans, "%d.%02d", loCurrentTemp_d, loCurrentTemp_f );

	 *((uint8_t*)req_p)     = UDP_DIAGEVT_TEMP_MIN;
	req_p                 += sizeof( uint8_t );

	 *((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	 *((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	 *((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	req_p += strlen((const char*)rx_ans);

	//(* tot_event_byte_pos_p) ++;
	data_pkt_count++;*/
	//MIN TEMP
	//MAX TEMP
	memset(rx_ans,0,sizeof(rx_ans));
	temp_max=tempC[MAX_DIAG_READINGS-1];
	//loCurrentTemp_d = (uint8_t) temp_max;
	//loCurrentTemp_f = (uint16_t)((temp_max - loCurrentTemp_d) * 1000.0F);
	sprintf((char *)rx_ans, "%d", temp_max );

	*((uint8_t*)req_p)     = UDP_DIAGEVT_TEMP_MAX;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	req_p += strlen((const char*)rx_ans);

	//(* tot_event_byte_pos_p) ++;
	data_pkt_count++;
	//MAX TEMP

	//VBAT
	current_index = 0;
	k=0;
	//vbatt_vltg=Vbat();
	memset(rx_ans,0,sizeof(rx_ans));
	for(i=0; i<MAX_DIAG_READINGS; i++)//04-04-2013
	{
		if(current_index>0)
		{
			rx_ans[current_index] = ',';
			current_index++;
		}
		temp = batt_vltg[i];
		k = current_index+3;
		for(j=3;j>0;j--)
		{
			rx_ans[k]=(temp%10)+0x30;
			k--;
			if(k == current_index+1)
			{
				rx_ans[k] = '.';
				k--;
			}
			temp=temp/10;
		}
		current_index += 4;
	}

	*((uint8_t*)req_p)     = UDP_DIAGEVT_RCHRGBL_BATT_CURR_VOLT;//not sure about the type
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) current_index;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (current_index >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,current_index);
	req_p += current_index;

	data_pkt_count++;
	//VBAT

	//Signal Strength
	memset(rx_ans,0,sizeof(rx_ans));
	if(sys_mdm_p.signal_strength!=0)
	{
		*((uint8_t*)req_p)     = UDP_DIAGEVT_SIG_STRENGTH;
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
		req_p                 += sizeof( uint8_t );

		sprintf((char*)rx_ans,"%d", sys_mdm_p.signal_strength);
		*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
		req_p                 += sizeof( uint8_t );

		memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
		req_p += strlen((const char*)rx_ans);

		//(* tot_event_byte_pos_p) ++;
		data_pkt_count++;
	}
	//Signal Strength

	//Coin Counts
	memset(rx_ans,0,sizeof(rx_ans));
	*((uint8_t*)req_p)     = UDP_DIAGEVT_COIN_COUNT;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	sprintf((char*)rx_ans,"%d(V:%d,IV:%d) 5C= %d 10C= %d 25C= %d 1DLR= %d ST= %d/%d,%d:%d ET= %d/%d,%d:%d", glTotal_Coin_Count,
			glTotal_Valid_Coin_Count, glTotal_0_cents, glTotal_5_cents, glTotal_10_cents, glTotal_25_cents, glTotal_100_cents,
			Start_Day_Coin_Diag,Start_Month_Coin_Diag,Start_Hour_Coin_Diag,Start_Minute_Coin_Diag,
			RTC_C->DATE & RTC_C_DATE_DAY_MASK,(RTC_C->DATE & RTC_C_DATE_MON_MASK)>>RTC_C_DATE_MON_OFS,
			RTC_C->TIM1 & RTC_C_TIM1_HOUR_MASK,(RTC_C->TIM0 & RTC_C_TIM0_MIN_MASK)>>RTC_C_TIM0_MIN_OFS);

	*((uint8_t*)req_p)     = (uint8_t)(uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	req_p                += strlen((const char*)rx_ans);

	Start_Day_Coin_Diag = RTC_C->DATE & RTC_C_DATE_DAY_MASK;
	Start_Month_Coin_Diag = (RTC_C->DATE & RTC_C_DATE_MON_MASK)>>RTC_C_DATE_MON_OFS;
	Start_Hour_Coin_Diag = RTC_C->TIM1 & RTC_C_TIM1_HOUR_MASK;//>>VT<<
	Start_Minute_Coin_Diag = (RTC_C->TIM0 & RTC_C_TIM0_MIN_MASK)>>RTC_C_TIM0_MIN_OFS;//>>VT<<

	data_pkt_count++;
	Debug_Output6(0, "Coin Counts: Total:%d, Accepted:%d, Rejected:%d", glTotal_Coin_Count,
			glTotal_Valid_Coin_Count, glTotal_0_cents, 0, 0, 0);

//	bkp_total_invalid_coin = glTotal_0_cents;
//	bkp_total_valid_coin = glTotal_Valid_Coin_Count;
//	bkp_total_coin = glTotal_Coin_Count;
//	bkp_5cents = glTotal_5_cents;
//	bkp_10cents = glTotal_10_cents;
//	bkp_25cents = glTotal_25_cents;
//	bkp_100cents = glTotal_100_cents;
	glTotal_0_cents = 0;
	glTotal_Valid_Coin_Count = 0;
	glTotal_Coin_Count = 0;
	glTotal_5_cents = 0;
	glTotal_10_cents = 0;
	glTotal_25_cents = 0;
	glTotal_100_cents = 0;
	//Coin Counts

	//15-09-2015:Liberty Enhancements
	memset(rx_ans,0,sizeof(rx_ans));
	*((uint8_t*)req_p)     = UDP_DIAGEVT_SIM_APN;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	memcpy(rx_ans, glSystem_cfg.apn, sizeof(glSystem_cfg.apn));

	*((uint8_t*)req_p)     = (uint8_t)(uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	req_p                += strlen((const char*)rx_ans);

	data_pkt_count++;

	//07/10/2015:LOB
	memset(rx_ans,0,sizeof(rx_ans));
	*((uint8_t*)req_p)     = UDP_DIAGEVT_MPB_CFG_VER;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	sprintf((char*)rx_ans,"%d.%d.%d.%d_%lx", glSystem_cfg.config_version, glSystem_cfg.config_main_version, glSystem_cfg.config_sub_version, glSystem_cfg.config_branch_version,glSystem_cfg.config_file_CRC);
	*((uint8_t*)req_p)     = (uint8_t)(uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	req_p                += strlen((const char*)rx_ans);

	data_pkt_count++;
	//07/10/2015:LOB

	memset(rx_ans,0,sizeof(rx_ans));
	*((uint8_t*)req_p)     = UDP_DIAGEVT_UDP_IP;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	memcpy(rx_ans,glSystem_cfg.UDP_server_ip,sizeof(glSystem_cfg.UDP_server_ip));

	*((uint8_t*)req_p)     = (uint8_t)(uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	req_p                += strlen((const char*)rx_ans);

	data_pkt_count++;

	memset(rx_ans,0,sizeof(rx_ans));
	*((uint8_t*)req_p)     = UDP_DIAGEVT_UDP_PORT;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	sprintf((char*)rx_ans,"%d", glSystem_cfg.UDP_server_port);
	*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	req_p += strlen((const char*)rx_ans);

	data_pkt_count++;

	//Events Count
	memset(rx_ans,0,sizeof(rx_ans));
	*((uint8_t*)req_p)     = UDP_DIAGEVT_EVENTS_COUNT;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	bkp_Events_count = Total_Events_count;
	sprintf((char*)rx_ans,"%d", Total_Events_count);
	*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	req_p += strlen((const char*)rx_ans);

	//Debug_Output1(0,"Total Events Count: %d",Total_Events_count); //LNGSIT-1002

	data_pkt_count++;
	//Events Count

	//CNTR_CURRENT_VALUE------->26
	//Events Count
	memset(rx_ans,0,sizeof(rx_ans));
	*((uint8_t*)req_p)     = UDP_DIAGEVT_COL_CNTR_CURRENT_VALUE;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );
	//I think we are not using this COL_CNTR_CURRENT_VALUE so using it for wakeupcounts //vinay balu sir wants to retain this so reverted back
	//sprintf((char*)rx_ans,"%ld", sys_mdm_p.COPS);//sprintf((char*)rx_ans,"%ld", min_col_counter_read);//sprintf((char*)rx_ans,"%ld", wakecounts_to_check_update);
	memcpy(rx_ans,sys_mdm_p.COPS,sizeof(sys_mdm_p.COPS));
	*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	req_p += strlen((const char*)rx_ans);

	data_pkt_count++;

	//ACCUMULATED_CURRENT_VALUE----->27
	//Events Count
	memset(rx_ans,0,sizeof(rx_ans));
	*((uint8_t*)req_p)     = UDP_DIAGEVT_COL_CNTR_ACC_CURRENT_VALUE;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	sprintf((char*)rx_ans,"%ld", accumulated_value_to_report1);//sprintf((char*)rx_ans,"%ld", accumulated_value_to_report);
	*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	req_p += strlen((const char*)rx_ans);

	data_pkt_count++;



	//Card Reader Type
	//LNGSIT-291: Report if GEMCLUB or IDTECH reader is connected on meter: 1- GEMCLUB, 2- IDTECH
	memset(rx_ans,0,sizeof(rx_ans));
	*((uint8_t*)req_p)     = UDP_DIAGEVT_CARD_READER_TYPE;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	//modified below code to send the details of the hardware to backend //NVK
	if(HARDWARE_REVISION == LNG_REV3)
	{
		if(Connected_Card_reader == GEM_CLUB_READER)
		{
			if(REV4_LED_CONFIG == true)
				sprintf((char*)rx_ans, "R3MB+R4C+GC, CityCode: %d", glMB_info.city_code);
			else
				sprintf((char*)rx_ans, "R3MB+R3C+GC, CityCode: %d", glMB_info.city_code);
		}
		else
		{
			if(REV4_LED_CONFIG == true)
				sprintf((char*)rx_ans, "R3MB+R4C+ID, CityCode: %d", glMB_info.city_code);
			else
				sprintf((char*)rx_ans, "R3MB+R3C+ID, CityCode: %d", glMB_info.city_code);
			//sprintf((char*)rx_ans, "Rev3+IDTECH, City Code: %d", glMB_info.city_code);
		}

	}
	else
	{
		if(Connected_Card_reader == GEM_CLUB_READER)
		{
			if(REV4_LED_CONFIG == true)
				if(Front_Bi_Color == true)
					sprintf((char*)rx_ans, "R4MB+R4C+GC+FBi, CCode: %d", glMB_info.city_code);
				else
					sprintf((char*)rx_ans, "R4MB+R4C+GC+FUi, CCode: %d", glMB_info.city_code);
			else
				sprintf((char*)rx_ans, "R4MB+R3C+GC, CityCode: %d", glMB_info.city_code);
		}
		else
		{
			if(REV4_LED_CONFIG == true)
				if(Front_Bi_Color == true)
					sprintf((char*)rx_ans, "R4MB+R4C+ID+FBi, CCode: %d", glMB_info.city_code);
				else
					sprintf((char*)rx_ans, "R4MB+R4C+ID+FUi, CCode: %d", glMB_info.city_code);
			else
				sprintf((char*)rx_ans, "R4MB+R3C+ID, CityCode: %d", glMB_info.city_code);
			//sprintf((char*)rx_ans, "Rev4+IDTECH, City Code: %d", glMB_info.city_code);
		}
	}

	*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	req_p += strlen((const char*)rx_ans);

	data_pkt_count++;
	//Card Reader Type

	//Failed Coins Diagnostic
	current_index = 0;
	memset(rx_ans,0,sizeof(rx_ans));
	for(i=0; i<MAX_FAILED_COINS_DIAG; i++)//04-04-2013
	{
		if(current_index>0)
		{
			rx_ans[current_index] = ',';
			current_index++;
		}
		sprintf((char*)rx_ans+current_index,"%d %d %d %d", Failed_Coins_diag[i].Cmax_val, Failed_Coins_diag[i].Cmin_val,
				Failed_Coins_diag[i].Crat1_val, Failed_Coins_diag[i].Crat2_val);

		current_index = strlen((const char*)rx_ans);
	}

	*((uint8_t*)req_p)     = UDP_DIAGEVT_FAILED_COINS_DIAG;//not sure about the type
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) current_index;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (current_index >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,current_index);
	req_p += current_index;

	data_pkt_count++;
	//Failed Coins Diagnostic

	//LoRa Frequencies
	memset(rx_ans,0,sizeof(rx_ans));
	*((uint8_t*)req_p)     = UDP_DIAGEVT_LORA_FREQUENIES;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	lRFFrequency = IDBasedFreqCalculation(glSystem_cfg.meter_id ); // Freq Calculation

	sprintf((char*)rx_ans, "%d(G), %d(S)", glSystem_cfg.Active_Channel, (uint32_t)lRFFrequency/1000000L);
	*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	req_p += strlen((const char*)rx_ans);

	data_pkt_count++;
	//LoRa Frequencies

	//Communication quality diag
	memset(rx_ans,0,sizeof(rx_ans));
	*((uint8_t*)req_p)     = UDP_DIAGEVT_COMMS_QUALITY;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	//get a back up copy of counts to be used while resetting the counts
	bkp_original = Original_packet_count;
	bkp_gprs_attempt = GPRS_comms_attempt_count;
	bkp_gprs_success = GPRS_comms_success_count;
	bkp_lora_attempt = LoRa_comms_attempt_count;
	bkp_lora_success = LoRa_comms_success_count;
	bkp_mdm_on = Modem_On_Count;

	sprintf((char*)rx_ans, "%d,%d,%d,%d,%d,%d", Original_packet_count, GPRS_comms_attempt_count, GPRS_comms_success_count, LoRa_comms_attempt_count, LoRa_comms_success_count, Modem_On_Count);
	*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	req_p += strlen((const char*)rx_ans);

	data_pkt_count++;
	//Communication quality diag

	//SIM ICCID
	memset(rx_ans,0,sizeof(rx_ans));
	memcpy(rx_ans, sys_mdm_p.ICCID, sizeof(sys_mdm_p.ICCID));
	if(sys_mdm_p.ICCID != 0)
	{
		*((uint8_t*)req_p)     = UDP_DIAGEVT_SIM_ICCID;
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
		req_p                 += sizeof( uint8_t );

		*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
		req_p                 += sizeof( uint8_t );

		memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
		req_p += strlen((const char*)rx_ans);

		//(* tot_event_byte_pos_p) ++;
		data_pkt_count++;
	}

	bkp_MDM_Total_On_Time = MDM_Total_On_Time;
	bkp_Total_LCD_Backlight_On_Time = Total_LCD_Backlight_On_Time;
	bkp_Total_LORA_PWR_On_Time = Total_LORA_PWR_On_Time;
	bkp_Total_LORA_PWR_On_Time_DC = Total_LORA_PWR_On_Time_DC;

	//Power_on duration
	memset(rx_ans,0,sizeof(rx_ans));
	*((uint8_t*)req_p)     = UDP_DIAGEVT_GPRS_MDM_ON_DURATION;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	sprintf((char*)rx_ans,"MDM=%d,LCD=%d,LORA=%d,LORA_DC=%d", MDM_Total_On_Time,Total_LCD_Backlight_On_Time,Total_LORA_PWR_On_Time,Total_LORA_PWR_On_Time_DC);
	*((uint8_t*)req_p)     = (uint8_t) strlen((const char*)rx_ans);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	req_p += strlen((const char*)rx_ans);

	data_pkt_count++;

	//Last 12 LORA LQI's
	current_index = 0;
	memset(rx_ans, 0, sizeof(rx_ans));
	for(i = 0; i < MAX_DIAG_READINGS;  i++)
	{
		if(current_index > 0)
		{
			rx_ans[current_index] = ',';
			current_index++;
		}
		sprintf((char*)rx_ans+current_index, "%02d", LQI_diagnostic[i]); //LQI
		current_index = strlen((const char*)rx_ans);
	}

	*((uint8_t*)req_p)     = UDP_DIAGEVT_LAST_12_LORA_LQI;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) current_index;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (current_index >> 8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p, rx_ans, current_index);
	req_p += current_index;

	data_pkt_count++;

	/////////////////////AUTO APN
	    //15-09-2015:Liberty Enhancements
	    memset(rx_ans,0,sizeof(rx_ans));
	    *((uint8_t*)req_p)     = UDP_DEFAULT_APN;
	    req_p                 += sizeof( uint8_t );

	    *((uint8_t*)req_p)     = DIAG_DATATYP_STRING;
	    req_p                 += sizeof( uint8_t );

	    memcpy(rx_ans, default_apn, sizeof(default_apn));

	    *((uint8_t*)req_p)     = (uint8_t)(uint8_t) strlen((const char*)rx_ans);
	    req_p                 += sizeof( uint8_t );

	    *((uint8_t*)req_p)     = (uint8_t) (strlen((const char*)rx_ans) >> 8);
	    req_p                 += sizeof( uint8_t );

	    memcpy(req_p,rx_ans,strlen((const char*)rx_ans));
	    req_p                += strlen((const char*)rx_ans);

	    data_pkt_count++;
	/////////////////////////////


	//Last 12 LORA LQI's

	len = (int16_t) ((req_p - req_p_temp) - 2);

	*((uint8_t*)req_p_temp) = (uint8_t) len;        //diag_message_length assigned
	req_p_temp+=sizeof( uint8_t );

	*((uint8_t*)req_p_temp)     = len >> 8;         //diag_message_length assigned
	req_p_temp                 +=sizeof( uint8_t );

	*((uint8_t*)req_p_temp)     = data_pkt_count;   //diag_message_length assigned
	req_p_temp                 +=sizeof( uint8_t );

	req_sz = (uint16_t) (req_p - request_events);

	//diag_msgs_to_server = false;

	result = gprs_do_request( request_events, req_sz );

	return result;
}

/**************************************************************************/
//! Generates the file activation status update packet and adds to communication queue
//! \param uint32_t file_id - file id for which this activation status is being sent
//! \param uint8_t type - file type for which this activation status is being sent
//! \param uint8_t activation_status - file activation status
//! - \b File activated successfully
//! - \b File Activation Failed
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_file_actn_stat( uint32_t file_id, uint8_t type,uint8_t activation_status )//02-07-12
{
    uint16_t                    req_sz;
    uint32_t                    alarm_time;
    uint8_t                        * req_p        = request_events;
    uint8_t                        result         = GPRS_COMMERR_NONE;

    memset( request_events, 0, sizeof(request_events) );

    if(activation_status == file_activation_success)//02-07-12:file activation success
    {
    	Debug_TextOut( 0, "161 event" );
    }
    else //02-07-12:file activation failure
    {
    	Debug_TextOut( 0, "160 event" );
    }

    req_p += gprs_fill_request_hdr( GPRS_COMM_BINARY_EVENTS, GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof( request_events ), false );

    *((uint8_t*)req_p)     = 1;
    req_p                 += sizeof( uint8_t );

    if(activation_status==true)//02-07-12:file activation success
    {
    	*((uint8_t*)req_p)     = FILE_ACTIVATION_SUCCESS & 0xFF;
    	req_p                 += sizeof( uint8_t );

        *((uint8_t*)req_p)     = (FILE_ACTIVATION_SUCCESS>>8) & 0xFF;
        req_p                 += sizeof( uint8_t );
    }
    else//02-07-12:file activation failure
    {
    	*((uint8_t*)req_p)     = FILE_ACTIVATION_FAILURE & 0xFF;
    	req_p                 += sizeof( uint8_t );

        *((uint8_t*)req_p)     = (FILE_ACTIVATION_FAILURE>>8) & 0xFF;
        req_p                 += sizeof( uint8_t );
    }


    alarm_time = RTC_epoch_now(); ;

    *((uint8_t*)req_p)     = alarm_time & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = (alarm_time>>8) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = (alarm_time>>16) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = (alarm_time>>24) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = 5 & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = (5>>8) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = (file_id>>24) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = (file_id>>16) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = (file_id>>8) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = file_id & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = type & 0xFF;
    req_p                 += sizeof( uint8_t );

    req_sz = (uint16_t) (req_p - request_events);

    result = gprs_do_request( request_events, req_sz );

    return result;
}

/**************************************************************************/
//! Generates the Smart Card Transaction packet and adds to communication queue
//! \param pointer to Smart Card Transaction structure
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_do_SCT(GPRS_Smartcard_tranRequest *request_SCT)
{
    uint16_t                    	req_sz;
    uint8_t                        * req_p = request_events;
    uint8_t                        result  = GPRS_COMMERR_NONE;

    memset( request_events, 0, sizeof(request_events) );

    //Debug_TextOut( 0, "Send SCT" );

    req_p += gprs_fill_request_hdr( GPRS_COMM_BINARY_EVENTS, GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof( request_events ), true );

    *((uint8_t*)req_p)     = 1;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = UDP_EVTTYP_SCT_PAYMENT & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = (UDP_EVTTYP_SCT_PAYMENT>>8) & 0xFF;
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = ( uint8_t )(request_SCT->time_stamp);
    req_p                 += sizeof( uint8_t );
    *((uint8_t*)req_p)     = ( uint8_t )((request_SCT->time_stamp)>>8);
    req_p                 += sizeof( uint8_t );
    *((uint8_t*)req_p)     = ( uint8_t )((request_SCT->time_stamp)>>16);
    req_p                 += sizeof( uint8_t );
    *((uint8_t*)req_p)     = ( uint8_t )((request_SCT->time_stamp)>>24);
    req_p                 += sizeof( uint8_t );

    *((uint8_t*)req_p)     = 20;			//pay load size
    req_p                 += sizeof(  uint8_t );
    *((uint8_t*)req_p)     = 0;			//pay load size
    req_p                 += sizeof(  uint8_t );

    *((uint8_t*)req_p)     = ( uint8_t )((request_SCT->ref_id)>>8);
    req_p                 += sizeof(  uint8_t );
    *((uint8_t*)req_p)     = ( uint8_t )(request_SCT->ref_id);
    req_p                 += sizeof(  uint8_t );


    *((uint8_t*)req_p)     = ( uint8_t )((glSystem_cfg.bay_no)>>8);
    req_p                  += sizeof( uint8_t );
    *((uint8_t*)req_p)     = ( uint8_t )(glSystem_cfg.bay_no);
    req_p                  += sizeof( uint8_t );

    (request_SCT->earned_mins)=(request_SCT->earned_mins)*60;//18-06-12:server is expecting in seconds
    (request_SCT->paid_untill_mins)=(request_SCT->paid_untill_mins)*60;//06-07-12:SCT roll up

    *((uint8_t*)req_p)     =  ( uint8_t )((request_SCT->paid_untill_mins)>>24);
    req_p                  += sizeof(  uint8_t);
    *((uint8_t*)req_p)     =  ( uint8_t )((request_SCT->paid_untill_mins)>>16);
    req_p                  += sizeof(  uint8_t);

    *((uint8_t*)req_p)     =  ( uint8_t )((request_SCT->paid_untill_mins)>>8);
    req_p                  += sizeof(  uint8_t);

    *((uint8_t*)req_p)     =  ( uint8_t )(request_SCT->paid_untill_mins);
    req_p                  += sizeof(  uint8_t);

    *((uint8_t*)req_p)     = ( uint8_t )((request_SCT->amount)>>8);
    req_p                  += sizeof( uint8_t );
    *((uint8_t*)req_p)     = ( uint8_t )(request_SCT->amount);
    req_p                  += sizeof( uint8_t );

    //Smart card serial number issue fix
    *((uint8_t*)req_p)      = ( uint8_t )((SC_Serial_Num)>>24);
    req_p                  += sizeof(uint8_t );

    *((uint8_t*)req_p)      = ( uint8_t )((SC_Serial_Num)>>16);
    req_p                  += sizeof(uint8_t );

    *((uint8_t*)req_p)      = ( uint8_t )((SC_Serial_Num)>>8);
    req_p                  += sizeof(uint8_t );

    *((uint8_t*)req_p)      = ( uint8_t )(SC_Serial_Num);
    req_p                  += sizeof(uint8_t );

    *((uint8_t*)req_p)      = 0;
    req_p                  += sizeof(uint8_t );

    *((uint8_t*)req_p)      = 0;
    req_p                  += sizeof(uint8_t );


    *((uint8_t*)req_p)     =  ( uint8_t )((request_SCT->earned_mins)>>24);
       req_p                  += sizeof(  uint8_t);
       *((uint8_t*)req_p)     =  ( uint8_t )((request_SCT->earned_mins)>>16);
       req_p                  += sizeof(  uint8_t);

       *((uint8_t*)req_p)     =  ( uint8_t )((request_SCT->earned_mins)>>8);
       req_p                  += sizeof(  uint8_t);

       *((uint8_t*)req_p)     =  ( uint8_t )(request_SCT->earned_mins);
       req_p                  += sizeof(  uint8_t);

    req_sz = (uint16_t) (req_p - request_events);

    result = gprs_do_request( request_events, req_sz );

    return result;
}

/**************************************************************************/
//! Generates the PBC Push success notification packet and adds to communication queue
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
int16_t gprs_push_notify()
{
    uint16_t                    req_sz;
    uint8_t                     * req_p        = request_small;
    int16_t                     result         = GPRS_COMMERR_NONE;

    memset( request_small, 0, sizeof(request_small) );

    Debug_TextOut( 1, "Send PUSH ACK" );

   // req_p += gprs_fill_request_hdr( GPRS_COMM_PAMPushAck, GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof( request_small ), false );

	*((uint8_t*)req_p)      = 0;
	req_p                  += sizeof( uint8_t );

	*((uint8_t*)req_p)      = GPRS_COMM_PAMPushAck;
	req_p                  += sizeof( uint8_t );

	*((uint16_t*)req_p)     = glSystem_cfg.area_num;
	req_p                  += sizeof( uint16_t );

	*((uint16_t*)req_p)     = glSystem_cfg.cust_id;
	req_p                  += sizeof( uint16_t );

	*((uint16_t*)req_p)     = received_meter_id;
	req_p                  += sizeof( uint16_t );

    *((uint16_t*)req_p)    = 1;
    req_p                 += sizeof( uint16_t );

    memcpy(req_p, PBC_expt_rcvd, 4);
    req_p += 4;

    req_sz = (uint16_t) (req_p - request_small);

    result = gprs_do_request( request_small, req_sz );

    return result;
}

/**************************************************************************/
//! Validate AES Key received from server
//! \param pointer to the AES key buffer
//! \return uint8_t result
//! - \b VALID KEY
//! - \b INVALID KEY
/**************************************************************************/
uint8_t Validate_downloaded_AES_key(uint8_t *key)
{
	received_crc = ((0xffffffff & key[3+AES_KEY_SIZE]) << 24)
					| ((0xffffffff & key[2+AES_KEY_SIZE]) << 16)
					| ((0xffffffff & key[1+AES_KEY_SIZE]) << 8)
					| (0xffffffff  & key[0+AES_KEY_SIZE]);

	dfg_crc = calc_crc32(0, key, AES_KEY_SIZE);

	if (dfg_crc != received_crc)
	{
		//Debug_TextOut(0, "AES_KEY_CRC MISMATCH");
		return FALSE;
	}
	return TRUE;
}

#if 0
int16_t PAM_Baystatus_update(GPRS_PamBayStatusUpdateRequest *PAM_baystatus_update)
{
	uint16_t                    	req_sz;
	uint8_t                        * req_p        = request_events;
	int16_t                        result         = GPRS_COMMERR_NONE;

	memset( request_events, 0, sizeof(request_events) );

	Debug_TextOut( 0, "PAM_Baystatus_update" );

	//req_p += gprs_fill_request_hdr( GPRS_COMM_PAMBayStatusUpdate, GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof( request_events ), false );

	*((uint8_t*)req_p)      = 0;
	req_p                  += sizeof( uint8_t );

	*((uint8_t*)req_p)      = GPRS_COMM_PAMBayStatusUpdate;
	req_p                  += sizeof( uint8_t );

	*((uint16_t*)req_p)     = glSystem_cfg.area_num;
	req_p                  += sizeof( uint16_t );

	*((uint16_t*)req_p)     = glSystem_cfg.cust_id;
	req_p                  += sizeof( uint16_t );

	*((uint16_t*)req_p)     = received_meter_id;
	req_p                  += sizeof( uint16_t );

	PAM_baystatus_update->bay_no=1;

	*((uint8_t*)req_p)     = ( uint8_t )(PAM_baystatus_update->bay_no);
	req_p                  += sizeof( uint8_t );

	*((uint8_t*)req_p)     = ( uint8_t )((PAM_baystatus_update->bay_no)>>8);
	req_p                  += sizeof( uint8_t );

	*((uint8_t*)req_p)     =  ( uint8_t )(PAM_baystatus_update->expiry_time);
	req_p                  += sizeof(  uint8_t);

	*((uint8_t*)req_p)     =  ( uint8_t )((PAM_baystatus_update->expiry_time)>>8);
		req_p                  += sizeof(  uint8_t);

	*((uint8_t*)req_p)     =  ( uint8_t )((PAM_baystatus_update->expiry_time)>>16);
	req_p                  += sizeof(  uint8_t);

	*((uint8_t*)req_p)     =  ( uint8_t )((PAM_baystatus_update->expiry_time)>>24);
	req_p                  += sizeof(  uint8_t);

	*((uint8_t*)req_p)     =  ( uint8_t )(PAM_baystatus_update->purchased_duration);
	req_p                  += sizeof(  uint8_t);

	*((uint8_t*)req_p)     =  ( uint8_t )((PAM_baystatus_update->purchased_duration)>>8);
	req_p                  += sizeof(  uint8_t);

	*((uint8_t*)req_p)     =  ( uint8_t )(PAM_baystatus_update->txn_type);
	req_p                  += sizeof(  uint8_t);

	*((uint8_t*)req_p)     =  ( uint8_t )(PAM_baystatus_update->topup);
	req_p                  += sizeof(  uint8_t);

	*((uint8_t*)req_p)     =  ( uint8_t )(PAM_baystatus_update->amount_cents);
	req_p                  += sizeof(  uint8_t);

	*((uint8_t*)req_p)     =  ( uint8_t )((PAM_baystatus_update->amount_cents)>>8);
	req_p                  += sizeof(  uint8_t);

	*((uint8_t*)req_p)     =  ( uint8_t )(PAM_baystatus_update->txn_time);
	req_p                  += sizeof(  uint8_t);

	*((uint8_t*)req_p)     =  ( uint8_t )((PAM_baystatus_update->txn_time)>>8);
	req_p                  += sizeof(  uint8_t);

	*((uint8_t*)req_p)     =  ( uint8_t )((PAM_baystatus_update->txn_time)>>16);
	req_p                  += sizeof(  uint8_t);

	*((uint8_t*)req_p)     =  ( uint8_t )((PAM_baystatus_update->txn_time)>>24);
	req_p                  += sizeof(  uint8_t);

	req_sz = (uint16_t) (req_p - request_events);

	result = gprs_do_request( request_events, req_sz );

	return result;

#if	0
	result = gprs_do_request( GPRS_COMM_UDP,
			request_events, req_sz,
			glComm_response,
			sizeof(glComm_response)-2,
			& filled_resp_len,
			glSystem_cfg.gen_retries,
			glSystem_cfg.gen_timeout,
			glSystem_cfg.gen_retry_timeout,
			glSystem_cfg.gen_retry_timeout );

	if ( result == GPRS_COMMERR_NONE )
	{
		//2106
		if ( telit_sock_close( glSystem_cfg.gen_sock_close_retries, GPRS_COMM_PAMBayStatusUpdate) != SOCERR_NONE )
		{
			result = GPRS_COMMERR_SOCK_CLOSE_ERR;
			send_diag_text_to_MB( glIn_diag_mode, 1, "SocClos Fail" );
		}
		//2106

		if ( filled_resp_len >= glSystem_cfg.gen_resp_hdr_sz )
		{
			gen_resp_hdr_len = gprs_fill_response_hdr( recvd_resp_p, filled_resp_len, &recvd_resp_hdr );
			recvd_resp_p     += gen_resp_hdr_len;

			if ( recvd_resp_hdr.ch.type != GPRS_COMM_PAMBayStatusUpdate ||
					(recvd_resp_hdr.ch.flag != GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT &&
					recvd_resp_hdr.ch.flag != GPRS_REQ_REG_RESP_SPEACT) ||
					filled_resp_len - (uint16_t) gen_resp_hdr_len < (uint16_t) recvd_resp_hdr.content_length )
			{
				result = GPRS_COMMERR_INVALID_RESP_HDR;
			}
			else
			{
				//    response_alm->server_resp = recvd_resp_hdr.server_resp;

				Debug_Output6( 2, "COMM %d %d %d %d %d",
						recvd_resp_hdr.ch.flag,
						recvd_resp_hdr.ch.type,
						recvd_resp_hdr.server_resp.txn_resp_code,
						recvd_resp_hdr.server_resp.ripnet_resp_code,
						recvd_resp_hdr.content_length, 0 );

				if ( recvd_resp_hdr.server_resp.txn_resp_code == 200 || recvd_resp_hdr.server_resp.ripnet_resp_code == 200 )
					result = GPRS_COMMERR_NONE;
				else
					result = GPRS_COMMERR_SERVER_ISSUE;
			}
		}
		else
		{
			result = GPRS_COMMERR_INVALID_RESP_HDR;
		}
	}
	else
	{
		send_diag_text_to_MB( glIn_diag_mode, 1, "Snd PAM_Baystatus failed" );
	}

	err_exit:

	Debug_Output1( 0, "PAM_Baystatus ERR=%d", result );

	return result;
#endif
}
#endif

/**************************************************************************/
//! Generates Meter's General Synchronization request and adds to communication queue
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_do_get_General_Sync_request()
{
	uint8_t                       * req_p         = request_events, j;
	uint8_t                       result          = GPRS_COMMERR_BUSY;

	// catch hold of a semaphore/monitor here for avoiding re-entry

	//Debug_TextOut(0,"GEN SYNC INTERVAL");

	Fill_General_Sync_request_structure();

	Asynchronous_FD = FALSE;

	memset( request_events, 0, sizeof(request_events) );
	memset(glDFG_response, 0 , sizeof(glDFG_response));
	memset(&glAfm_response_p, 0, sizeof(glAfm_response_p));
	memset(&CFG_file_sizearr,0, sizeof(CFG_file_sizearr));

	Serial_no_length = (strlen((const char*)glMB_info.serial_no_asci)); //5;

	req_p += gprs_fill_request_hdr( GPRS_COMM_lOB_General_Sync, GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof( request_events ), false );

	*((uint8_t*)req_p)     = ( uint8_t )(strlen((const char*)glMB_info.locid));
	req_p                  += sizeof( uint8_t );

	*((uint8_t*)req_p)     = ( uint8_t )((strlen((const char*)glMB_info.locid))>>8);
	req_p                  += sizeof( uint8_t );

	memcpy(req_p,&glMB_info.locid,(strlen((const char*)glMB_info.locid)));
	req_p                  += ( uint8_t )(strlen((const char*)glMB_info.locid));

	*((uint8_t*)req_p)     = ( uint8_t )(Serial_no_length);
	req_p                  += sizeof( uint8_t );

	*((uint8_t*)req_p)     = ( uint8_t )(Serial_no_length>>8);
	req_p                  += sizeof( uint8_t );

	memcpy(req_p,glMB_info.serial_no_asci,Serial_no_length);
	req_p 				   += ( uint8_t )(Serial_no_length);

	*((uint8_t*)req_p)     = ( uint8_t )(gprs_general_sync_request.OLT_AES_Encryption);
	req_p                  += sizeof( uint8_t );

	if(DO_IP_SYNC == TRUE)
	{
		DO_IP_SYNC = FALSE;
		sys_mdm_p.local_ip_length=0;//30-05-12
		result = mdm_get_local_ip_port();
	}

	sys_mdm_p.local_PORT = 8181;//LOB:22/12/2015
	*((uint8_t*)req_p)    = ( uint8_t )(sys_mdm_p.local_PORT);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)    = ( uint8_t )(sys_mdm_p.local_PORT>>8);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)    = ( uint8_t )(sys_mdm_p.local_ip_length);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)    = ( uint8_t )(sys_mdm_p.local_ip_length>>8);
	req_p                 += sizeof( uint8_t );

	memcpy(req_p, sys_mdm_p.local_ip_add, sys_mdm_p.local_ip_length);
	req_p += sys_mdm_p.local_ip_length;

	*((uint8_t*)req_p)     = ( uint8_t )(gprs_general_sync_request.Total_Files);
	req_p                  += sizeof( uint8_t );

	for(j=0;j<FILE_TYPE_TOTAL_FILES;j++)
	{
		*((uint8_t*)req_p)     = ( uint8_t )(gprs_general_sync_request.file_data[j].File_Type);
		req_p                  += sizeof( uint8_t );
		memcpy(req_p,&gprs_general_sync_request.file_data[j].File_ID,4);
		req_p                  += 4;
		memcpy(req_p,gprs_general_sync_request.file_data[j].CHK_SUM,20);
		req_p                  += 20;
		*((uint8_t*)req_p)     = ( uint8_t )(gprs_general_sync_request.file_data[j].VER_LEN);
		req_p                  += sizeof( uint8_t );
		memcpy(req_p,gprs_general_sync_request.file_data[j].VER_NUM,gprs_general_sync_request.file_data[j].VER_LEN);
		req_p                  += gprs_general_sync_request.file_data[j].VER_LEN;
	}

	//diag_text_Screen( "General_Sync_request", FALSE );

	result = gprs_do_request( request_events, (uint16_t)(req_p - request_events) );

	return result;
}

/**************************************************************************/
//! Parse general sync response received from server
//! \param pointer to the response buffer
//! \return void
/**************************************************************************/
void parse_General_Sync_Response(uint8_t * response)
{
	uint16_t length1=0,length2=0/*,result*/;
	uint16_t length3 = 0, k = 0, l = 0;
	uint8_t  AES_KEY_CRC_LENGTH = 8;

	gprs_general_sync_response.meter_action = response[0];
	if(gprs_general_sync_response.meter_action != 0)
	{
		Special_Action_Trigger = TRUE;
	}

	gprs_general_sync_response.mech_swap_result = response[1];

	memset(gprs_general_sync_response.RTC_String,0,sizeof(gprs_general_sync_response.RTC_String));
	length1 = (uint8_t)(response[2] | (response[3] << 8));//RTC length
	if(length1 <= sizeof(gprs_general_sync_response.RTC_String))
	{
		memcpy(&gprs_general_sync_response.RTC_String,(response+4),length1);
		glDo_rtc_update = TRUE;
		//glFlash_rtc_update = TRUE;
	}

	length2 = (uint8_t)(response[4+length1] | (response[5+length1]<<8));//AES key length

	if(length2 == AES_KEY_SIZE)
	{
		if(Validate_downloaded_AES_key(&response[6+length1]))
		{
			memcpy(AES_KEY,(response+6+length1),length2);
			AES_KEY_ID = response[6+length1+length2+AES_KEY_CRC_LENGTH] |(response[7+length1+length2+AES_KEY_CRC_LENGTH]<<8);
		}
	}

	//skip 8,9 which is chunk size for now
	if(length2 != AES_KEY_SIZE)
		length2 = AES_KEY_SIZE;
	gprs_general_sync_response.total_FD_chunks = (uint16_t)(response[10+length1+length2+AES_KEY_CRC_LENGTH] | (response[11+length1+length2+AES_KEY_CRC_LENGTH]<<8));
	if(gprs_general_sync_response.total_FD_chunks > MAX_FD_CHUNKS)
	{
		gprs_general_sync_response.total_FD_chunks = 0;//avoid FD when duplicate files,	buffer corruption
	}
	length3 = (uint16_t)(response[12+length1+length2+AES_KEY_CRC_LENGTH] | (response[13+length1+length2+AES_KEY_CRC_LENGTH]<<8));
	if(length3 > GPRS_GEN_BUF_SZ_VLARGE)
		length3 = GPRS_GEN_BUF_SZ_VLARGE;

	//glDo_rtc_update = TRUE;
	//set_rtc_time();
	Handle_RTC_4mServer();
	Handle_Mechswap_Response();

	if(gprs_general_sync_response.total_FD_chunks > 0)
	{
		File_count = 0;
		//AFM XML parse,pass appropriate buffer index and length
		call_xml_parse( &response[14+length1+length2+AES_KEY_CRC_LENGTH], length3 );
		//TODO:RE-ARRANGE afm response in order,erase flash based on the file count
		//Re_arrange_AFM_response();
		chunk_DNLD_status.CFG = 0;
		chunk_DNLD_status.CCF = 0;
		chunk_DNLD_status.RPG = 0;
		memset(chunk_DNLD_status.PGM,0,sizeof(chunk_DNLD_status.PGM));
		memset(chunk_DNLD_status.SNSR,0,sizeof(chunk_DNLD_status.SNSR));
		memset(chunk_DNLD_status.BIN,0,sizeof(chunk_DNLD_status.BIN));
		memset(glDFG_response,0,sizeof(glDFG_response));
		//Asynchronous_FD = TRUE;
		Downloaded_chunks = 0;
		//if(sys_mdm_p.local_ip_add[0] > 0x30)
			Prepare_Flash_4FD();
		//Sync_status = 0x654321;//after parsing mark which one to download
			Sync_status = 0x21;//after parsing mark which one to download
		Get_Meter_Status(FALSE);
		if(REINITIATE_FD == TRUE)//WAIT FOR SERVER TO STOP PREVIOUS PUSH
			DelayMs(4000);
		REINITIATE_FD = FALSE;
		//chunk_count = 0;
		packet_start = FALSE;
		//if(IP_done == TRUE)
		//Trigger_Asynchronous_FD();
		Tasks_Priority_Register |= FILE_DOWNLOAD_TASK;
	}
	else if ((gprs_general_sync_response.meter_action > 0XF0) && (gprs_general_sync_response.meter_action < 0XFF)) //25/09/2015-LOB
	{
		//Debug_TextOut(0,"0XF1 - 0XFE - LNG Special Action Request");

		switch(gprs_general_sync_response.meter_action)
		{
		case 0XF1://reset MB and MPB
			//Debug_TextOut(0,"0XF1 - LNG Reset CMD");
			Flash_RTC_ParkingClock_Storage(true, false);	//RTC append in flash every minute
			telit_power_off();
			__disable_interrupt();
			WDTCTL = WDTPW+WDTCNTCL+WDTSSEL1+WDTIS_7;

			for(l=0;l<60000;l++)
			{
				for(k=0;k<1000;k++);
			}
			break;
		case 0XF2://reset LNG only
			//Debug_TextOut(0,"0XF2 - LNG Reset CMD");
			Flash_RTC_ParkingClock_Storage(true, false);	//RTC append in flash every minute
			telit_power_off();
			__disable_interrupt();
			WDTCTL = WDTPW+WDTCNTCL+WDTSSEL1+WDTIS_7;

			for(l=0;l<60000;l++)
			{
				for(k=0;k<1000;k++);
			}
			break;
		case 0XF3://reset sensor
			//FORCE_MC_RESET = TRUE;
			break;
		case 0XF4://meter inactive
			break;

		default :
			break;
		}
		gprs_general_sync_response.meter_action = 0;
	}
	Get_Meter_Status(FALSE);

}
//SHRUTHI:11/09/2015-LOB-1

//11/09/2015
/**************************************************************************/
//! Sends the File download trigger and then downloads files
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t Trigger_Asynchronous_FD()
{
    uint8_t                       * req_p         = request_events, dfs_retry=0;
    uint8_t                       result          = GPRS_COMMERR_BUSY, result1 = GPRS_COMMERR_BUSY ;

    memset( request_events, 0, sizeof(request_events) );//2212

    req_p += gprs_fill_request_hdr( GPRS_COMM_lOB_DFG, GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof( request_events ), false );

    diag_text_Screen( "Downloading File..", TRUE, FALSE );

    if(LCD_BKLT_ON == TRUE)	// need to save battery, no need to keep backlight on
		LCD_Backlight(FALSE);

    //LNGSIT-745,785,696
    Waiting_4GPRS_Resp = FALSE;
    waiting_for_batch_response = FALSE;
    dfs_done_ready_to_activate = FALSE;

    result = do_request_internal( request_events, (uint16_t)(req_p - request_events), true, true );

    if ( result == GPRS_COMMERR_NONE )
    {
    	for(dfs_retry=0; dfs_retry<MAX_DFS_RETRY; dfs_retry++)
    	{
    		if(Downloaded_chunks < gprs_general_sync_response.total_FD_chunks)
    			gprs_do_DFS_4all_files(FALSE);
    	}
    	if(Downloaded_chunks < gprs_general_sync_response.total_FD_chunks)
    		REINITIATE_FD = TRUE;
    		//gprs_do_get_General_Sync_request();
    }
    //Asynchronous_FD = FALSE;
    Debug_Output1( 0, "Trigger_FD ERR=%d", result );

	if(Downloaded_chunks == gprs_general_sync_response.total_FD_chunks)
	{
		//Asynchronous_FD = FALSE;
		validate_asynchronous_FD();
		for(dfs_retry=0; dfs_retry<2; dfs_retry++)
		{
			result1=gprs_do_DFS_4all_files(TRUE);
			if ( result1 == GPRS_COMMERR_NONE )
				break;
		}
		if(REINITIATE_FD!=TRUE)
			diag_text_Screen( "Download Complete", Meter_Reset, FALSE );
	}
	else
		diag_text_Screen( "Download Failed!", Meter_Reset, FALSE );

	if((REINITIATE_FD != TRUE) && (Downloaded_chunks == gprs_general_sync_response.total_FD_chunks))
	{
		check_n_activate_files_from_flash();
	}
	else if(gprs_general_sync_response.total_FD_chunks > 0)//DO GENERAL SYNC 3 TIMES EVEN WHEN IT DID NOT DOWNLOAD ANY CHUNK
	{
		REINITIATE_FD = TRUE;
	}

/*	if(CFG_ERASE == TRUE)
	{
		CFG_ERASE = FALSE;
		//Write_cfg4mRAM_2flash();
	}*/
	Asynchronous_FD = FALSE;
	return result;

}
//11/09/2015
#if 0
//LOB:re-arrange file details as per file activation table
void Re_arrange_AFM_response()
{
	uint8_t loop=0,FFACT_file_type=0;
	for(FFACT_file_type=0;FFACT_file_type<(FILE_TYPE_TOTAL_FILES+1);FFACT_file_type++)
	{
		for(loop=0;loop<File_count;loop++)
		{

			if((glAfm_response_p[loop].type !=(FFACT_file_type+FILE_TYPE_MPB_CONFIG)) && (glAfm_response_p[loop].type !=FILE_TYPE_MC_PGM))
				continue;
			else
			{
				memcpy(glDFG_response,&glAfm_response_p[FFACT_file_type],sizeof(glAfm_response_p[loop]));
				memcpy(&glAfm_response_p[FFACT_file_type],&glAfm_response_p[loop],sizeof(glAfm_response_p[loop]));
				memcpy(&glAfm_response_p[loop],&glAfm_response_p[FFACT_file_type],sizeof(glAfm_response_p[loop]));
				break;
			}
		}
	}
}
#endif
//LOB:re-arrange file details as per file activation table

/**************************************************************************/
//! Generates the LOB BayStatus packet and adds to communication queue
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_do_LOB_BayStatus()
{
	//uint16_t                    req_sz;
	uint8_t                     * req_p = request_events;
	uint8_t                 	result = GPRS_COMMERR_BUSY;
	//uint32_t                    time_stamp;

	//Debug_TextOut( 0, "BAY STATUS UPDATE" );

	//req_p += gprs_fill_request_hdr( GPRS_COMM_LOB_BayStatus_diagnostics, GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof( request_events ), false);//15-10-2013: sensor diag

	*((uint8_t*)req_p)      = 0;
	req_p                  += sizeof( uint8_t );

	*((uint8_t*)req_p)      = GPRS_COMM_LOB_BayStatus_diagnostics;
	req_p                  += sizeof( uint8_t );

	*((uint16_t*)req_p)     = glSystem_cfg.area_num;
	req_p                  += sizeof( uint16_t );

	*((uint16_t*)req_p)     = glSystem_cfg.cust_id;
	req_p                  += sizeof( uint16_t );

	*((uint16_t*)req_p)     = received_meter_id;
	req_p                  += sizeof( uint16_t );

	*((uint16_t*)req_p)     = 1;//request_p->bay_no;
	req_p                  += sizeof( uint16_t );

	*((uint16_t*)req_p)     = 1;//no.of BSU in packet
	req_p                  += sizeof( uint16_t );

	*((uint8_t*)req_p) 	= ( uint8_t )(glBay_req.expiry_time & 0xFF);
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= ( uint8_t )((glBay_req.expiry_time>>8) & 0xFF);
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= ( uint8_t )((glBay_req.expiry_time>>16) & 0xFF);
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	=( uint8_t ) ((glBay_req.expiry_time>>24) & 0xFF);
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p)     = ( uint8_t )(glBay_req.purchased_duration & 0XFF);
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p)     = ( uint8_t )((glBay_req.purchased_duration >> 8)& 0XFF);
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p)     = glBay_req.txn_type;
	req_p                  += sizeof(glBay_req.txn_type);

	*((uint8_t*)req_p)     = 0;//request_p->topup;
	req_p                  += sizeof(uint8_t);

	*((uint8_t*)req_p)     = 0;//request_p->amount_cents;
	req_p                  += sizeof(uint8_t);

	*((uint8_t*)req_p)     = 0;//request_p->amount_cents;
	req_p                  += sizeof(uint8_t);

	//time_stamp = RTC_epoch_now();

	//glBay_req.time_stamp = glBay_req.PAMtxn_time = time_stamp;

	*((uint8_t*)req_p) 	= (glBay_req.PAMtxn_time )& 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (glBay_req.PAMtxn_time>>8) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (glBay_req.PAMtxn_time>>16) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (glBay_req.PAMtxn_time>>24) & 0xFF;
	req_p += sizeof( uint8_t );



	*((uint8_t*)req_p) 	= (glBay_req.time_stamp )& 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (glBay_req.time_stamp>>8) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (glBay_req.time_stamp>>16) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (glBay_req.time_stamp>>24) & 0xFF;
	req_p += sizeof( uint8_t );

	//15-10-2013: sensor diag
	*((uint8_t*)req_p) 	= glSystem_cfg.cust_id & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (glSystem_cfg.cust_id>>8) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= glSystem_cfg.area_num & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (glSystem_cfg.area_num>>8) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= glSystem_cfg.meter_id & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p) 	= (glSystem_cfg.meter_id>>8) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p)    = (glBay_req.occupancy) & 0xFF;
	req_p                += sizeof( uint8_t );

	*((uint8_t*)req_p) = SENSOR_DIAG_LENGTH & 0xFF;	//BS diagnostics data length
	req_p += 1;

	*((uint8_t*)req_p) = (SENSOR_DIAG_LENGTH>>8) & 0xFF;	//BS diagnostics data length
	req_p += 1;
	//15-10-2013: sensor diag

	*((uint8_t*)req_p)  = (glBay_req.SNSR_TMP) & 0xFF;
	req_p += sizeof( uint8_t );

	*((uint8_t*)req_p)  = (glBay_req.SNSR_TMP>>8) & 0xFF;
	req_p += sizeof( uint8_t );

	memcpy(req_p, glBay_req.diag_string, 68);
	req_p += 68;

	//Debug_Output1( 0, "Bay Status = %d",glBay_req.occupancy );

	result = gprs_do_request( request_events, (uint16_t)(req_p - request_events) );
	//req_sz = (uint16_t)(req_p - request_events);
	//result = gprs_do_request( req_p, req_sz );

	//Debug_Output1( 0, "BSU ERR=%d", result );

	return result;
}

/**************************************************************************/
//! Main Communication Processor for the LORA packets added in communication Queue
//! Process all the requests in the RAM Queue one by one
//! \param void
//! \return void
/**************************************************************************/
void process_LORA_Queue()
{
	uint8_t Que_count = 0;//, start_Queue_scan_index = 0;
	uint8_t result = GPRS_COMMERR_NONE;
	//Debug_TextOut(0,"process_LORA_Queue");
	if(Waiting_4LORA_Resp == FALSE)
	{
		for(Que_count=0; Que_count<MAX_QUEUE_COUNT; Que_count++)
		{
			if((LoRa_Queue[Que_count].Packet[1] != 0) && (LoRa_Queue[Que_count].Request_size <= MAX_LORA_PACKET_LENGTH) && (LoRa_Queue[Que_count].Retrycount < QUEUE_RETRY_COUNT))
			{
				LoRa_Queue[Que_count].Retrycount++;

				glMdmUart_bytes_recvd = 0;
				result = Transactions_via_LoRa(LoRa_Queue[Que_count].Packet, LoRa_Queue[Que_count].Request_size);

				/*Debug_Output6(2,"RTC_Request_GW-02: %02X %02X %02X %02X %02X %02X", LoRa_Queue[Que_count].Packet[0], LoRa_Queue[Que_count].Packet[1],
						LoRa_Queue[Que_count].Packet[2], LoRa_Queue[Que_count].Packet[3], LoRa_Queue[Que_count].Packet[4], LoRa_Queue[Que_count].Packet[5]);*/

				if(result == GPRS_COMMERR_NONE)
				{
					LoRa_comms_attempt_count++;
					if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && ((glSystem_cfg.Enable_GW_Comms_Coin_Only == TRUE) || (glSystem_cfg.Coin_Only_Temporary_GW_Enable == TRUE))
						&& ((LoRa_Queue[Que_count].Packet[5] == LORA_NODE_RTC)))//3oct18
					{
						//Debug_Output2(2,"Sent Req_Type: %d, LORA Q:%d",LoRa_Queue[Que_count].Packet[5], Que_count);
					}
					else
					{
						//Debug_Output2(2,"Sent Req_Type: %d, LORA Q:%d",LoRa_Queue[Que_count].Packet[1], Que_count);
					}
					Waiting_4LORA_Resp = TRUE;
					last_LoRa_pktsent_time  = RTC_epoch_now();
					LoRa_Packet_Index = Que_count;
				}
				else if(LoRa_Queue[Que_count].Retrycount >= QUEUE_RETRY_COUNT)
				{
					if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && ((glSystem_cfg.Enable_GW_Comms_Coin_Only == TRUE) || (glSystem_cfg.Coin_Only_Temporary_GW_Enable == TRUE))
						&& ((LoRa_Queue[Que_count].Packet[5] == LORA_NODE_RTC)))//3oct18
					{
						Debug_Output2(0,"Failed Req_Type: %d, LORA Q: %d", LoRa_Queue[Que_count].Packet[5], Que_count);
					}
					else
					{
						Debug_Output2(0,"Failed Req_Type: %d, LORA Q: %d", LoRa_Queue[Que_count].Packet[1], Que_count);
					}
					if(LORA_PWR == TRUE)	//Turn Off LORA if it is On
					{
						LORA_PWR = FALSE;
						LORA_power_off();
					}
					Waiting_4LORA_Resp = FALSE;
					if(LoRa_Queue[Que_count].Packet[1] == GPRS_COMM_OLT_AES)
					{
						glOLT_resp.auth_code = GPRS_OLT_GWAY_RESP_Timeout;  //Batch OLT and show approved
						Handle_OLT_response();
					}
					Queue_Request(LoRa_Queue[Que_count].Packet, LoRa_Queue[Que_count].Request_size );
					memset(LoRa_Queue[Que_count].Packet,0,MAX_LORA_PACKET_LENGTH);
					LoRa_Queue[Que_count].Request_size = 0;
					LoRa_Queue[Que_count].Retrycount = 0;
					LoRa_Packet_Index = 0;
					if(LoRa_Queue_Count > 0)
						LoRa_Queue_Count--;

					if(queue_pending == TRUE)
						Process_Flash_Queue = TRUE;	//to be tested: call batch clear here since the live request is forced to batch with new design
				}
				break;

			}
		}
	}
	else
	{
		OnMaster();
		if(RTC_epoch_now() > (last_LoRa_pktsent_time + LORA_RESP_TIMEOUT))
		{
			if(LORA_PWR == TRUE)	//Turn Off LORA if it is On
			{
				LORA_PWR = FALSE;
				LORA_power_off();
			}
			Waiting_4LORA_Resp = FALSE;
			if(LoRa_Queue[LoRa_Packet_Index].Retrycount >= QUEUE_RETRY_COUNT)
			{
				if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && ((glSystem_cfg.Enable_GW_Comms_Coin_Only == TRUE) || (glSystem_cfg.Coin_Only_Temporary_GW_Enable == TRUE))
					&& ((LoRa_Queue[Que_count].Packet[5] == LORA_NODE_RTC)))//3oct18
				{
					Debug_Output2(0,"Failed Req_Type: %d, LORA Q: %d", LoRa_Queue[LoRa_Packet_Index].Packet[5], LoRa_Packet_Index);
				}
				else
				{
					Debug_Output2(0,"Failed Req_Type: %d, LORA Q: %d", LoRa_Queue[LoRa_Packet_Index].Packet[1], LoRa_Packet_Index);
				}
				if(LoRa_Queue[LoRa_Packet_Index].Packet[1] == GPRS_COMM_OLT_AES)
				{
					glOLT_resp.auth_code = GPRS_OLT_GWAY_RESP_Timeout;  //Batch OLT and show approved
					Handle_OLT_response();
				}
				Queue_Request(LoRa_Queue[LoRa_Packet_Index].Packet, LoRa_Queue[LoRa_Packet_Index].Request_size );
				memset(LoRa_Queue[LoRa_Packet_Index].Packet,0,MAX_LORA_PACKET_LENGTH);
				LoRa_Queue[LoRa_Packet_Index].Request_size = 0;
				LoRa_Queue[LoRa_Packet_Index].Retrycount = 0;
				LoRa_Packet_Index = 0;
				if(LoRa_Queue_Count > 0)
					LoRa_Queue_Count--;

				if(queue_pending == TRUE)
					Process_Flash_Queue = TRUE;	//to be tested: call batch clear here since the live request is forced to batch with new design

			}
		}
	}
}


/**************************************************************************/
//! Main Communication Processor for the GPRS packets added in communication Queue
//! Process all the requests in the RAM Queue one by one
//! \param void
//! \return void
/**************************************************************************/
void process_RAM_Queue()
{
	uint8_t Que_count = 0, start_Queue_scan_index = 0;
	uint8_t result = GPRS_COMMERR_NONE;
	waiting_for_batch_response = false;
	if(Waiting_4GPRS_Resp != TRUE)
	{
		for(Que_count=0; Que_count<MAX_QUEUE_COUNT; Que_count++)	//scan once to find if any high priority packet is in queue
		{
			if((Queue[Que_count].Packet[1] == GPRS_COMM_OLT_AES) && (Queue[Que_count].Request_size <= MAX_PACKET_LENGTH))
			{
				start_Queue_scan_index = Que_count;
				break;	//break as soon as an OLT is found in queue
			}
			else
			{
				start_Queue_scan_index = 0;
			}
		}

		for(Que_count=start_Queue_scan_index; Que_count<MAX_QUEUE_COUNT; Que_count++)
		{
			if((Queue[Que_count].Packet[1] != 0) && (Queue[Que_count].Request_size <= MAX_PACKET_LENGTH) && (Queue[Que_count].Retrycount < QUEUE_RETRY_COUNT))
			{
				Queue[Que_count].Retrycount++;
				glMdmUart_bytes_recvd = 0;

				if(sys_mdm_p.local_ip_length == 0) mdm_get_local_ip_port(); //-- NO NEED FOR ALL PACKETS, IF METER DIDN'T RECIEVED LOCAL IP THEN ONLY NEEDED

				result = do_request_internal( Queue[Que_count].Packet,Queue[Que_count].Request_size, true, false );
				if(result == GPRS_COMMERR_NONE)
				{
					GPRS_comms_attempt_count++;
					//Debug_Output2(2,"Sent Req_Type: %d, RAM Q:%d",Queue[Que_count].Packet[1], Que_count);
					Waiting_4GPRS_Resp = TRUE;
					last_pktsent_time  = RTC_epoch_now();
					Packet_Index = Que_count;
					Process_Flash_Queue = TRUE;	//Enable Batch Processing only when any other comms is successful  ppp
					//Present_value=0;
				}
				else if((Queue[Que_count].Retrycount >= QUEUE_RETRY_COUNT) || (Queue[Que_count].Packet[1] == GPRS_COMM_OLT_AES))
				{
					glComm_failure_count++;    //increment failure count
					Debug_Output2(0,"Failed Req_Type: %d, RAM Q:%d", Queue[Que_count].Packet[1], Que_count);
					if(Queue[Que_count].Packet[1] == GPRS_COMM_OLT_AES)
					{
						glOLT_resp.auth_code = GPRS_OLT_GWAY_RESP_Timeout;  //Batch OLT and show approved
						Handle_OLT_response();
					}
					if(Queue[Que_count].Packet[1] == GPRS_COMM_lOB_DFS)
					{
						//Debug_TextOut(0,"DFS FAILED");
						dfs_done_ready_to_activate = TRUE;
					}
					Queue_Request(Queue[Que_count].Packet,Queue[Que_count].Request_size );
					memset(Queue[Que_count].Packet,0,MAX_PACKET_LENGTH);
					Queue[Que_count].Request_size = 0;
					Queue[Que_count].Retrycount = 0;
					Packet_Index = 0;
					if(In_Queue_Count > 0)
						In_Queue_Count--;
				}
				//TODO: Enable below to save power
				//result = telit_sock_close();	//Packet sent, put the modem to sleep now
				break;
			}
		}
	}
	else
	{
		if(RTC_epoch_now() > (last_pktsent_time + GPRS_TIMEOUT))
		{
			glComm_failure_count++;    //increment failure count
			Waiting_4GPRS_Resp = FALSE;
			if(Packet_Index < MAX_QUEUE_COUNT)
			{
				Debug_Output2(0,"Failed Req_Type:%d  ERR Code = %d", Queue[Packet_Index].Packet[1], GPRS_COMMERR_SOCK_RECV_ERR);
				if(Queue[Packet_Index].Packet[1] == GPRS_COMM_OLT_AES)
				{
					glOLT_resp.auth_code = GPRS_OLT_GWAY_RESP_Timeout;   //Batch OLT and show approved
					Handle_OLT_response();
					Queue_Request(Queue[Packet_Index].Packet,Queue[Packet_Index].Request_size );
					memset(Queue[Packet_Index].Packet,0,MAX_PACKET_LENGTH);
					Queue[Packet_Index].Request_size = 0;
					Queue[Packet_Index].Retrycount = 0;
					if(In_Queue_Count > 0)
						In_Queue_Count--;
				}
				else if(Queue[Packet_Index].Retrycount >= QUEUE_RETRY_COUNT)
				{
					if(Queue[Packet_Index].Packet[1] == GPRS_COMM_lOB_DFS)
					{
						//Debug_TextOut(0,"DFS FAILED");
						dfs_done_ready_to_activate = TRUE;
					}
					Queue_Request(Queue[Packet_Index].Packet,Queue[Packet_Index].Request_size );
					memset(Queue[Packet_Index].Packet,0,MAX_PACKET_LENGTH);
					Queue[Packet_Index].Request_size = 0;
					Queue[Packet_Index].Retrycount = 0;
					if(In_Queue_Count > 0)
						In_Queue_Count--;

					if(Meter_Reset == TRUE)
					{
						Meter_Reset = FALSE;
						push_event_to_cache(UDP_EVTTYP_MPB_RESET);
						Tasks_Priority_Register |= SEVERE_EVENTS_TASK;
						Clear_all_active_alarms();
					}
				}
				Packet_Index = 0;
			}
		}
	}
}

/**************************************************************************/
//! Response handler for the GPRS communication packets
//! Handles the GPRS Response from server
//! \param void
//! \return void
/**************************************************************************/
void Handle_GPRS_Response()
{
	uint8_t                       * recvd_resp_p  = glDFG_response;
	uint8_t                       result          = GPRS_COMMERR_BUSY;
	uint8_t 					  file_index = 0;
	int16_t                       gen_resp_hdr_len;
	//uint32_t 					  Flash_del_add;        //TO copy the base address with the index value
	GPRS_GeneralResponseHeader    recvd_resp_hdr;
	uint8_t   					  j = 0, simply = 0;
	//result = telit_sock_close();	//Packet received, put the modem to sleep now
	//Debug_TextOut(0,"Rcvd Some");
	GPRS_comms_success_count++;
	if(waiting_for_batch_response == true)
	{
		if ( DFG_filled_resp_len >= glSystem_cfg.gen_resp_hdr_sz )
		{
			gen_resp_hdr_len = gprs_fill_response_hdr( recvd_resp_p, DFG_filled_resp_len, &recvd_resp_hdr );
			recvd_resp_p     += gen_resp_hdr_len;

			if ( recvd_resp_hdr.ch.type != comm_queue.batch_request[1] ||
					(recvd_resp_hdr.ch.flag != GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT &&
							recvd_resp_hdr.ch.flag != GPRS_REQ_REG_RESP_SPEACT) ||
							DFG_filled_resp_len - (uint16_t)gen_resp_hdr_len < (uint16_t)recvd_resp_hdr.content_length )
			{
				result = GPRS_COMMERR_INVALID_RESP_HDR;
			}
			else
			{
				/*Debug_Output6( 3, "COMM %d %d %d %d %d",
						recvd_resp_hdr.ch.flag,
						recvd_resp_hdr.ch.type,
						recvd_resp_hdr.server_resp.txn_resp_code,
						recvd_resp_hdr.server_resp.ripnet_resp_code,
						recvd_resp_hdr.content_length, 0 );*/

				if ( recvd_resp_hdr.server_resp.txn_resp_code != 200 /*|| recvd_resp_hdr.server_resp.ripnet_resp_code != 200*/ )//TODO:need to enable back if required
					result = GPRS_COMMERR_SERVER_ISSUE;
				else
					result = GPRS_COMMERR_NONE;//2006
			}
			if(result == GPRS_COMMERR_NONE)
			{
				glComm_failure_count = 0;
				t_mobile_selected_apn_retry_flag = TRUE;
				Debug_Output2( 0, "Processed Req_Type:%d  ERR Code = %d", comm_queue.batch_request[1],result );
				Debug_Output1(0,"Batch Clr: %d",Batch_Index_Sent);
				flag_check_addr[Batch_Index_Sent] = 0;
				Flash_Batch_Flag_Write();
				if(comm_queue.batch_request[1] == GPRS_COMM_BINARY_EVENTS) //LNGSIT-1023
				{
					TechMenu_Get_Event_Update_Time();
				}
				memset(comm_queue.batch_request,0,sizeof(comm_queue.batch_request));
				//Flash_del_add=(uint32_t)(batch_start_address)+(uint32_t)(QUEUE_PACKET_SIZE*Batch_Index_Sent);
				//Flash_Write( comm_queue.batch_request,sizeof(comm_queue.batch_request),Flash_del_add,false);
				Queue_failure_count_l=0;
				Batch_clear_continuous_fail = 0;
				waiting_for_batch_response = false;
			}
			//waiting_for_batch_response = false;

			for (j = 0; j < (Q_Big-1); j++)
			{
				if (flag_check_addr[j] == 1)    //This to check any flag pending.
					break;
			}

			if (j == (Q_Big-1))
			{
				//Debug_Output1(0,"Q_Big:%d",j);
				switch(batch_start_address)
				{
					case (AD_olt_r):
						OLT_Q_count=0;
						push_event_to_cache( UDP_EVTTYP_OLT_BATCH_EMPTY);
						comm_queue.OLT_batch_erase       = true;
						cc_approved = 0; //reset for next olt payment //vinay
						//Debug_TextOut( 0, "Com Batch empty");
						//Debug_Output1( 0, "Com cleared cc_approved = %d", cc_approved );
						memset(comm_queue.OLT_batch_flag, 0, sizeof(comm_queue.OLT_batch_flag));
						Debug_TextOut(0,"OLT Batch Empty");
					break;
					case (AD_COIN_r):
						Coin_Q_count=0;
						push_event_to_cache( UDP_EVTTYP_COIN_BATCH_EMPTY);
						comm_queue.coin_batch_erase       = true;
						memset(comm_queue.coin_batch_flag, 0, sizeof(comm_queue.coin_batch_flag));
						Debug_TextOut(0,"CT Batch Empty");
					break;
					case (AD_ACK_r):
						OLTAck_Q_count=0;
						push_event_to_cache( UDP_EVTTYP_OLTACK_BATCH_EMPTY);
						comm_queue.OLTAck_batch_erase       = true;
						memset(comm_queue.OLTAck_batch_flag, 0, sizeof(comm_queue.OLTAck_batch_flag));
						Debug_TextOut(0,"OLT-ACK Batch Empty");
					break;
					case (AD_ALARM_r):
						Alarms_Q_count=0;
						push_event_to_cache( UDP_EVTTYP_ALARM_BATCH_EMPTY);
						comm_queue.Alarms_batch_erase       = true;
						memset(comm_queue.Alarms_batch_flag, 0, sizeof(comm_queue.Alarms_batch_flag));
						Debug_TextOut(0,"Alarms Batch Empty");
					break;
					case (AD_BAYSTATUS_r):
						BayStatus_Q_count=0;
						push_event_to_cache( UDP_EVTTYP_BAYSTATUS_BATCH_EMPTY );
						comm_queue.BayStatus_batch_erase       = true;
						memset(comm_queue.BayStatus_batch_flag, 0, sizeof(comm_queue.BayStatus_batch_flag));
						Debug_TextOut(0,"BSU Batch Empty");
					break;
					case (AD_SMART_CARD_r):
						SCT_Q_count=0;
						push_event_to_cache( UDP_EVTTYP_SCT_BATCH_EMPTY);
						comm_queue.SCT_batch_erase       = true;
						memset(comm_queue.SCT_batch_flag, 0, sizeof(comm_queue.SCT_batch_flag));
						Debug_TextOut(0,"SCT Batch Empty");
					break;
					case (AD_EVENTS_BATCH_RW):
						EVENTS_Q_COUNT=0;
						push_event_to_cache( UDP_EVTTYP_EVENTS_BATCH_EMPTY );
						comm_queue.EVENTS_batch_erase       = true;
						memset(comm_queue.EVENTS_batch_flag, 0, sizeof(comm_queue.EVENTS_batch_flag));
						Debug_TextOut(0,"Events Batch Empty");
					break;
					case (AD_PAMBayStatus_w):
						PAMBayStatus_Q_count = 0;
						push_event_to_cache( UDP_EVTTYP_PAMBAYSTATUS_BATCH_EMPTY );
						comm_queue.PAMBayStatus_batch_erase       = true;
						memset(comm_queue.PAMBayStatus_batch_flag, 0, sizeof(comm_queue.PAMBayStatus_batch_flag));
						Debug_TextOut(0,"PAM-BS Batch Empty");
					break;
				}
				Flash_Batch_Flag_Write();
			}
		}
	}
	else
	{
		if ( DFG_filled_resp_len >= glSystem_cfg.gen_resp_hdr_sz )
		{
			gen_resp_hdr_len = gprs_fill_response_hdr( recvd_resp_p, DFG_filled_resp_len, &recvd_resp_hdr );
			recvd_resp_p     += gen_resp_hdr_len;

			if ( recvd_resp_hdr.ch.type != Queue[Packet_Index].Packet[1] ||
					(recvd_resp_hdr.ch.flag != GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT &&
							recvd_resp_hdr.ch.flag != GPRS_REQ_REG_RESP_SPEACT) ||
							DFG_filled_resp_len - (uint16_t)gen_resp_hdr_len < (uint16_t)recvd_resp_hdr.content_length )
			{
				result = GPRS_COMMERR_INVALID_RESP_HDR;
			}
			else
			{
				//response_p->server_resp = recvd_resp_hdr.server_resp;

			/*	Debug_Output6( 3, "COMM %d %d %d %d %d",
						recvd_resp_hdr.ch.flag,
						recvd_resp_hdr.ch.type,
						recvd_resp_hdr.server_resp.txn_resp_code,
						recvd_resp_hdr.server_resp.ripnet_resp_code,
						recvd_resp_hdr.content_length, 0 );*/

				if ( recvd_resp_hdr.server_resp.txn_resp_code != 200 /*|| recvd_resp_hdr.server_resp.ripnet_resp_code != 200*/ )//TODO:need to enable back if required
					result = GPRS_COMMERR_SERVER_ISSUE;
				else
					result = GPRS_COMMERR_NONE;//2006
			}
		}
		else
		{
			result = GPRS_COMMERR_INVALID_RESP_HDR;
		}
		if(Waiting_4GPRS_Resp == TRUE)
			Debug_Output2( 0, "Processed Req_Type:%d  ERR Code = %d", Queue[Packet_Index].Packet[1],result );

		if((result == GPRS_COMMERR_NONE) && ((Queue[Packet_Index].Packet[1]) == GPRS_COMM_lOB_General_Sync))
		{
			parse_General_Sync_Response(&glDFG_response[10]);//enable when server side changes supported
			Debug_TextOut(0,"General Sync OK");

			if(Meter_Reset == TRUE)
			{
				Meter_Reset = FALSE;
				push_event_to_cache(UDP_EVTTYP_MPB_RESET);
				Tasks_Priority_Register |= SEVERE_EVENTS_TASK;
				Clear_all_active_alarms();
			}
		}
		else if((result == GPRS_COMMERR_NONE) && (Queue[Packet_Index].Packet[1] == GPRS_COMM_OLT_AES))
		{
			glOLT_resp.auth_code = glDFG_response[14];
			Handle_OLT_response();
			Debug_TextOut(0,"OLT OK");
		}
		else if((result == GPRS_COMMERR_NONE) && (Queue[Packet_Index].Packet[1] == GPRS_COMM_CT))
		{
			Debug_TextOut(0,"CT 55 OK");
		}
		else if((result == GPRS_COMMERR_NONE) && (Queue[Packet_Index].Packet[1] == GPRS_COMM_CT_PROFILE))
		{
			Debug_TextOut(0,"CT 125 OK");
		}
		else if((result == GPRS_COMMERR_NONE) && (Queue[Packet_Index].Packet[1] == GPRS_COMM_lOB_DFS))
		{
			Debug_TextOut(0,"DFS OK");
			dfs_done_ready_to_activate = TRUE;
		}
		else if((result == GPRS_COMMERR_NONE) && (Queue[Packet_Index].Packet[1] == GPRS_COMM_BINARY_EVENTS))
		{
			//TechMenu_Get_Event_Update_Time();
			Last_Comms_Success_RTCTimestamp = RTC_epoch_now();
			Last_Comms_Success_Flg = TRUE;

			if((glDFG_response[10] == 0) || (glDFG_response[11] == 2)) //Diagnostics
			{
//				glTotal_0_cents -= bkp_total_invalid_coin;
//				glTotal_Valid_Coin_Count -= bkp_total_valid_coin;
//				glTotal_Coin_Count -= bkp_total_coin;
//				glTotal_5_cents -= bkp_5cents;
//				glTotal_10_cents -= bkp_10cents;
//				glTotal_25_cents -= bkp_25cents;
//				glTotal_100_cents -= bkp_100cents;
				glTotal_0_cents = 0;
				glTotal_Valid_Coin_Count = 0;
				glTotal_Coin_Count = 0;
				glTotal_5_cents = 0;
				glTotal_10_cents = 0;
				glTotal_25_cents = 0;
				glTotal_100_cents = 0;

				//keep the new counts and reduce the last reported counts
				Original_packet_count -= bkp_original;
				GPRS_comms_attempt_count -= bkp_gprs_attempt;
				GPRS_comms_success_count -= bkp_gprs_success;
				LoRa_comms_attempt_count -= bkp_lora_attempt;
				LoRa_comms_success_count -= bkp_lora_success;
				Modem_On_Count -= bkp_mdm_on;
				Total_Events_count -= bkp_Events_count;	//LNGSIT-703,617

				//san
				MDM_Total_On_Time -= bkp_MDM_Total_On_Time ;
				Total_LORA_PWR_On_Time -= bkp_Total_LORA_PWR_On_Time;
				Total_LCD_Backlight_On_Time -= bkp_Total_LCD_Backlight_On_Time;
				Total_LORA_PWR_On_Time_DC -= bkp_Total_LORA_PWR_On_Time_DC;


				for(simply=0; simply<MAX_FAILED_COINS_DIAG; simply++)
				{
					Failed_Coins_diag[simply].Cmax_val  = 0;
					Failed_Coins_diag[simply].Cmin_val  = 0;
					Failed_Coins_diag[simply].Crat1_val = 0;
					Failed_Coins_diag[simply].Crat2_val = 0;
				}
				failed_coin_counter = 0;

				Debug_TextOut(0,"Diag Evts OK");
			}
			else if((glDFG_response[11] | (glDFG_response[12]<<8)) == UDP_EVTTYP_ALARM)
			{
				TechMenu_Get_Event_Update_Time(); //LNGSIT-1023
				Debug_TextOut(0,"Alarms OK");
			}
			else if((glDFG_response[11] | (glDFG_response[12]<<8)) == FILE_ACTIVATION_SUCCESS)
			{
				TechMenu_Get_Event_Update_Time(); //LNGSIT-1023
				Flash_Batch_Flag_Read();
				for(file_index = 0; file_index < FFACT_TOTAL_FILES;  file_index++)
				{
					if(flash_file_actvn_tbl[file_index].status == FF_ACTIVATE_COMPLETED)
					{
						flash_file_actvn_tbl[file_index].status = FF_161_COMPLETED;
						flash_file_actvn_tbl[file_index].actvn_try_count = 0;
					}
				}
				Flash_Batch_Flag_Write();
				Debug_TextOut(0,"161 Event OK");
			}
			else if((glDFG_response[11] | (glDFG_response[12]<<8)) == FILE_ACTIVATION_FAILURE)
			{
				TechMenu_Get_Event_Update_Time(); //LNGSIT-1023
				Debug_TextOut(0,"160 Event OK");
			}
			else
			{
				TechMenu_Get_Event_Update_Time();
				init_event_cache(0xff);	//Live events were posted, clear them now, LNGSIT-405
				Debug_TextOut(0,"Evts OK");
			}
		}

		//Batch Eligible packets if wrong response from Server also
		if(result != GPRS_COMMERR_NONE)	//LNGSIT-743: Meter partially hanged after ERR=13 for OLT
		{
			if(Queue[Packet_Index].Packet[1] == GPRS_COMM_OLT_AES)
			{
				glOLT_resp.auth_code = GPRS_OLT_GWAY_RESP_Timeout;   //Batch OLT and show approved
				Handle_OLT_response();
				Queue_Request(Queue[Packet_Index].Packet,Queue[Packet_Index].Request_size );
			}
		}
		else //if(result == GPRS_COMMERR_NONE)	// || (result == GPRS_COMMERR_SERVER_ISSUE)) //Let invalid response failures also go through retries
		{
			glComm_failure_count = 0;
			t_mobile_selected_apn_retry_flag = TRUE;
			//Debug_Output1(0,"RAM Q Clr:%d",Packet_Index);
			memset(Queue[Packet_Index].Packet, 0 ,MAX_PACKET_LENGTH);
			Queue[Packet_Index].Request_size = 0;
			Queue[Packet_Index].Retrycount = 0;
			Packet_Index = 0;
			if(In_Queue_Count>0)
				In_Queue_Count--;
			Waiting_4GPRS_Resp = FALSE;
		}
	}
	//waiting_for_batch_response = false;
	//Waiting_4GPRS_Resp = FALSE;
	//Special_Action_Trigger = FALSE;
	disable_telit_RI_interrupt();
}

/**************************************************************************/
//! Converts the packets from LoRa Queue and converts them into Gateway
//! acceptable format and sends to gateway
//! \param pointer to the LoRa Queue to be processed
//! \param size of the LoRa Queue to be processed
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t Transactions_via_LoRa(uint8_t *request_p, uint16_t request_sz)
{
	//uint16_t		i;
	uint8_t			result = GPRS_COMMERR_NONE;

	if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && ((glSystem_cfg.Enable_GW_Comms_Coin_Only == TRUE) || (glSystem_cfg.Coin_Only_Temporary_GW_Enable == TRUE))//3oct18
			&& (request_p[5] == LORA_NODE_RTC))
	{
		memcpy(&LoRa_tx_queue[0], request_p, request_sz);
	}
	else
	{
		LoRa_tx_queue[0]     = 0XFE;
		LoRa_tx_queue[1]     = 0XFA;
		LoRa_tx_queue[2]     = 0;
		LoRa_tx_queue[3]     = 0;
		memcpy(&LoRa_tx_queue[4], request_p, request_sz);
	}

	if(LORA_PWR == TRUE)	//Turn Off LORA if it is On; This will help in changing LORA settings and calling OnMaster
	{
		LORA_PWR = FALSE;
		LORA_power_off();
	}

	memset(glDFG_response, 0, GPRS_GEN_BUF_SZ_LARGE);
	DFG_Chunk_received = FALSE;

	if(TRUE == SX1276LoRaCheckTXState())
	{
		if(LORA_PWR != TRUE)
		{
			LoRaSettings1.RFFrequency = (uint32_t)((uint32_t)glSystem_cfg.Active_Channel*(uint32_t)1000000L);	//primary communication channel
			//Debug_Output1(0,"RFFrequency:%d",((  LoRaSettings1.RFFrequency )/1000000));
			LoRaSettings1.RxPacketTimeout = LORA_RESP_TIMEOUT-1;	//20 seconds timeout
			LoRaSettings1.RxSingleOn = 0;
			LORA_PWR = TRUE;
			Init_LORA();
		}
		OnMaster();
		Radio->SetTxPacket( LoRa_tx_queue, (request_sz+4) );

		OnMaster();	//give one chance to send the packet here

		/*		for(i=0; i<5000; i++)
		{
			OnMaster();
			if(DFG_Chunk_received == TRUE)
			{
				//TODO: extract and copy the received buffer here for processing
				result = GPRS_COMMERR_NONE;
				break;
			}
		}*/
		//Debug_Output1(0,"Rxd_Resp_frm_GW: %d", Rxd_Resp_frm_GW);
	}
	else
	{
		result = GPRS_COMMERR_BUSY;
		Debug_TextOut(0, "LORA Tx pending\r\n");
	}
/*	SLEEP_ENABLE = FALSE;
	if(LORA_PWR == TRUE)	//Turn Off LORA if it is On; This will help in changing LORA settings and calling OnMaster
	{
		LORA_power_off();
		LORA_PWR = FALSE;
	}*/
	return result;
}

/**************************************************************************/
//! Generates the audit registry packet for Coin Only meters and sends to Gateway
//! \param void
//! \return void
/**************************************************************************/
void Send_Audit_Via_Temp_GW(void)
{
	uint16_t                    	req_sz;
	uint8_t                       	* req_p = request_events;
	uint32_t 						time_stamp;
	int8_t							i	=	0;

	memset(request_events, 0, sizeof(request_events));

	Debug_TextOut(0, "Send Audit Registry via GW");

	Flash_Batch_Flag_Read();

	req_p += gprs_fill_request_hdr(GPRS_SSM_AUDITINFO, GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof(request_events), false);

	time_stamp = RTC_epoch_now();

	*((uint8_t*)req_p)     = time_stamp & 0xFF;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (time_stamp>>8) & 0xFF;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (time_stamp>>16) & 0xFF;
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (time_stamp>>24) & 0xFF;
	req_p                 += sizeof( uint8_t );

	memcpy(req_p, &Maintenance_Log.TECH_CARD_SL_NO, (TECH_SL_NO_LEN+1)); //27-06-12
	req_p                 += (TECH_SL_NO_LEN+1);

	if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))
		{
			glTotal_Valid_Coin_Count = 0;
			for(i=0;i<20;i++)
				{
					glTotal_Valid_Coin_Count += (temp_coin_type[i] + coin_type[i]);
				}
			//Debug_Output1(0, "Comm glTotal_Valid_Coin_Count: %ld", glTotal_Valid_Coin_Count);
			if(total_coin_values > 0)
				{
					*((uint8_t*)req_p)     = (uint8_t) glTotal_Valid_Coin_Count;
					req_p                 += sizeof( uint8_t );

					*((uint8_t*)req_p)     = (uint8_t) (glTotal_Valid_Coin_Count>>8);
					req_p                 += sizeof( uint8_t );
				}
			else
				{
					*((uint8_t*)req_p)     = (uint8_t) (0);
					req_p                 += sizeof( uint8_t );

					*((uint8_t*)req_p)     = (uint8_t) (0);
					req_p                 += sizeof( uint8_t );
				}

		}
	else
		{
			*((uint8_t*)req_p)     = (uint8_t) glTotal_Valid_Coin_Count;
			req_p                 += sizeof( uint8_t );

			*((uint8_t*)req_p)     = (uint8_t) (glTotal_Valid_Coin_Count>>8);
			req_p                 += sizeof( uint8_t );
		}

	*((uint8_t*)req_p)     = (uint8_t) total_coin_values; //audit_info_p->AudAmt;
	req_p                 += sizeof( uint8_t );
	*((uint8_t*)req_p)     = (uint8_t) ((total_coin_values)>>8);
	req_p                 += sizeof( uint8_t );
	*((uint8_t*)req_p)     = (uint8_t) ((total_coin_values)>>16);
	req_p                 += sizeof( uint8_t );
	*((uint8_t*)req_p)     = (uint8_t) ((total_coin_values)>>24);
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) glTotal_SCT_TransCount; //audit_info_p->CK;	//SC/CK count to be filled
	req_p                 += sizeof( uint8_t );
	*((uint16_t*)req_p)     = (uint8_t) ((glTotal_SCT_TransCount)>>8);	//SC/CK count to be filled
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) glTotal_SCT_TransAmount; //(audit_info_p->CashlessAudit);	//SC/CK Debit Amount to be filled
	req_p                 += sizeof( uint8_t );
	*((uint8_t*)req_p)     = (uint8_t) ((glTotal_SCT_TransAmount)>>8);	//SC/CK Debit Amount to be filled
	req_p                 += sizeof( uint8_t );
	*((uint8_t*)req_p)     = (uint8_t) ((glTotal_SCT_TransAmount)>>16);	//SC/CK Debit Amount to be filled
	req_p                 += sizeof( uint8_t );
	*((uint8_t*)req_p)     = (uint8_t) ((glTotal_SCT_TransAmount)>>24);	//SC/CK Debit Amount to be filled
	req_p                 += sizeof( uint8_t );

	*((uint8_t*)req_p)     = (uint8_t) (0);	//CC count to be filled
	req_p                 += sizeof( uint8_t );
	*((uint8_t*)req_p)     = (uint8_t) (0);	//CC count to be filled
	req_p                 += sizeof( uint8_t );


	*((uint8_t*)req_p)     = (uint8_t) (0);	//CC amount to be filled
	req_p                 += sizeof( uint8_t );
	*((uint8_t*)req_p)     = (uint8_t) (0);	//CC amount to be filled
	req_p                 += sizeof( uint8_t );
	*((uint8_t*)req_p)     = (uint8_t) (0);	//CC amount to be filled
	req_p                 += sizeof( uint8_t );
	*((uint8_t*)req_p)     = (uint8_t) (0);	//CC amount to be filled
	req_p                 += sizeof( uint8_t );
	if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))
		{
			*((uint8_t*)req_p)     = (uint8_t) coin_type_invalid; //audit_info_p->RejCoin;
			req_p                 += sizeof( uint8_t );

			*((uint8_t*)req_p)     = (uint8_t) (coin_type_invalid>>8);
			req_p                 += sizeof( uint8_t );
		}
	else
		{
			*((uint8_t*)req_p)     = (uint8_t) glTotal_0_cents; //audit_info_p->RejCoin;
			req_p                 += sizeof( uint8_t );

			*((uint8_t*)req_p)     = (uint8_t) (glTotal_0_cents>>8);
			req_p                 += sizeof( uint8_t );
		}
	*((uint8_t*)req_p)     = MAX_COINS_SUPPORTED;
	req_p                 += sizeof( uint8_t );
	if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))
		{
			for(i=19;i>=0;i--)
			{
				*((uint8_t*)req_p)     = (uint8_t)(coin_type[i]);
				req_p                 += sizeof( uint8_t );
				*((uint8_t*)req_p)     = (uint8_t)((coin_type[i])>>8);
				req_p                 += sizeof( uint8_t );
			}

		}
	else
		{
			*((uint8_t*)req_p)     = (uint8_t)(coin_type0);
			req_p                 += sizeof( uint8_t );
			*((uint8_t*)req_p)     = (uint8_t)((coin_type0)>>8);
			req_p                 += sizeof( uint8_t );

			*((uint8_t*)req_p)     = (uint8_t)(coin_type1);
			req_p                 += sizeof( uint8_t );
			*((uint8_t*)req_p)     = (uint8_t)((coin_type1)>>8);
			req_p                 += sizeof( uint8_t );

			*((uint8_t*)req_p)     = (uint8_t)(coin_type2);
			req_p                 += sizeof( uint8_t );
			*((uint8_t*)req_p)     = (uint8_t)((coin_type2)>>8);
			req_p                 += sizeof( uint8_t );

			*((uint8_t*)req_p)     = (uint8_t)(coin_type3);
			req_p                 += sizeof( uint8_t );
			*((uint8_t*)req_p)     = (uint8_t)((coin_type3)>>8);
			req_p                 += sizeof( uint8_t );

			for(i=0; i<(MAX_COINS_SUPPORTED-4); i++)
			{
				*((uint8_t*)req_p)     = (uint8_t)(0);
				req_p                 += sizeof( uint8_t );
				*((uint8_t*)req_p)     = (uint8_t)(0);
				req_p                 += sizeof( uint8_t );
			}
		}

	req_sz = (uint16_t) (req_p - request_events);

	gprs_do_request( request_events, req_sz);
}

/**************************************************************************/
//! decodes the received LoRa packets and processes those
//! \param uint16_t lora_resp_length - Length of LoRa packet received at meter
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t Process_Incoming_Sensor_Response(uint16_t lora_resp_length)
{
	uint8_t 					  result = GPRS_COMMERR_BUSY;//,error_code = 0;
	uint8_t                       * recvd_resp_p  = &glComm_response[12];
	GPRS_GeneralResponseHeader    recvd_resp_hdr;
	int16_t                       gen_resp_hdr_len;
	uint8_t 					  RECV_SEQ_NUM_CHK_FLAG = 0;
	uint8_t   					  j = 0;
	uint8_t 					  file_index = 0;
	uint16_t					  check_if_special_code_recvd = 0;
	uint8_t						  i = 0;

	Debug_Display(2,"Recvd response from GW");

	/*for(file_index=0; file_index<30; file_index++)
	{
		Debug_Output2(0, "GW Resp Packet[%d]:%02X",file_index,glComm_response[file_index]);
	}*/

	if((glComm_response[0] == 0xFE) && (glComm_response[1] == 0xFA))
	{
		received_area_num = (glComm_response[7]<<8) | glComm_response[6];
		received_cust_id = (glComm_response[9]<<8) | glComm_response[8];
		received_meter_id = (glComm_response[11]<<8) | glComm_response[10];

		if((glSystem_cfg.cust_id == received_cust_id) && ((glSystem_cfg.meter_id == received_meter_id) || ((glSystem_cfg.meter_id+(glSystem_cfg.MSM_max_spaces-1)) == received_meter_id)))
		{
			RECV_SEQ_NUM_CHK_FLAG = 1;	//for BSU, no need to validate area number
			Debug_Output6(0,"bytes:%d, C-A-M ID: %d-%d-%d", lora_resp_length, received_cust_id, received_area_num, received_meter_id, 0, 0);

			//if((glSystem_cfg.meter_id+(glSystem_cfg.MSM_max_spaces-1)) == received_meter_id)
			{
				LNG_Meter_ID = glSystem_cfg.meter_id;
				//Debug_Output1(2,"LNG_Meter_ID ID = %d",LNG_Meter_ID);

				if(received_meter_id >= LNG_Meter_ID)
				{
					Current_bayStatus = (received_meter_id - LNG_Meter_ID);
					if(Current_bayStatus <= 0)
						Current_bayStatus = 0;
				}
				//Debug_Output1( 2, "Current_bayStatus: %d ", Current_bayStatus);
			}
		}
		else  //different Id with same frequency
		{
			//Debug_Output6(2,"bytes:%d, different C-A-M ID: %d-%d-%d", lora_resp_length, received_cust_id, received_area_num, received_meter_id, 0, 0);

			if(Waiting_4LORA_Resp == FALSE)
			{
				SLEEP_ENABLE = FALSE;
				if(Duty_Cycle_Mode == FALSE)
				{
					Duty_Cycle_Mode = TRUE;
					/*Debug_Output1(0,"Seq_num = %X",Seq_num);*/
					/*Debug_Output1(0,"Cust ID = %d",received_cust_id);
					Debug_Output1(0,"Area ID = %d",received_area_num);
					Debug_Output1(0,"Meter ID = %d",received_meter_id);*/

					LNG_Meter_ID = glSystem_cfg.meter_id;
					//Debug_Output1(2,"LNG_Meter_ID ID = %d",LNG_Meter_ID);

					received_curr_id = (received_meter_id % 100);
					Debug_Output1(0,"Rxd_WPM_Id = %d",received_curr_id);

					Current_bayStatus = (received_meter_id - LNG_Meter_ID);
					if(Current_bayStatus <= 0)
						Current_bayStatus = 0;
					//Debug_Output1(2, "Current_bayStatus: %d ", Current_bayStatus);
				}
				BSU_Chunk_received = FALSE;
			}

			if((glComm_response[4] == ACK) && (glComm_response[5] == GPRS_COMM_PAMPushMsg))  // validating PBC for own or initiate to other dongle with preamble
			{
				/*Debug_TextOut(0,"Rxd PBC for differnt MID\r\n");
				Debug_Output1(0,"Cust ID = %d",received_cust_id);
				Debug_Output1(0,"Area ID = %d",received_area_num);
				Debug_Output1(0,"Meter ID = %d",received_meter_id);*/
				Debug_Output6(0,"Rxd PBC for differnt C-A-M: %d-%d-%d",received_cust_id, received_area_num, received_meter_id, 0, 0, 0);

				result = GPRS_COMMERR_NONE;

				if(LORA_PWR == TRUE)	//Turn Off LORA if it is On
				{
					LORA_PWR = FALSE;
					LORA_power_off();
				}
				memcpy(&Current_extracted_time_PBC_for_other_MID,&glComm_response[14],(PBC_EXPIRY_TIME_LENGTH));
				//Debug_Output1(0,"Current_extracted_time_PBC_for_other_MID = %X", Current_extracted_time_PBC_for_other_MID);
				if(Current_extracted_time_PBC_for_other_MID[Current_bayStatus] > Last_processed_time_PBC_for_other_MID[Current_bayStatus])
				{
					//Debug_Display(2,"GPRS_COMM_PAMPushMsg\r\n");
					PBC_TO_OTHER_METER();
					Last_processed_time_PBC_for_other_MID[Current_bayStatus] = Current_extracted_time_PBC_for_other_MID[Current_bayStatus];
				}
				else
				{
					Debug_Output1(0,"Same_PBC_Pckt_Rxd for Meter ID = %d",received_meter_id);
				}
			}
			else
			{
				//Debug_Display(2,"Else of GPRS_COMM_PAMPushMsg\r\n");
				//Debug_Output2(2,"ACK = %d, CommType = %d",glComm_response[4], glComm_response[5]);
			}

		}
		//Debug_Output2(2,"glComm_response[4] = %d, glComm_response[5] = %d",glComm_response[4], glComm_response[5]);
		if((glComm_response[4] == DATA) && (RECV_SEQ_NUM_CHK_FLAG == 1))
		{
			//Debug_Output1(2,"Rxd_Data_Type: %d",glComm_response[5]);
			switch(glComm_response[5])
			{
			case  LORA_NODE_RTC:
				result = GPRS_COMMERR_NONE;
				if((glSystem_cfg.Enable_GW_Comms_Coin_Only == TRUE) || (glSystem_cfg.Coin_Only_Temporary_GW_Enable == TRUE))
				{
					SetMSP_RTC(2);
					LCD_Backlight(TRUE);
					Update_RTC_on_Display();
					read_and_fill_current_schedule(&rtc_c_calender); //Refresh rate every time after updating RTC from server; just in case anything changed
					diag_text_Screen( "RTC Updated from GW", TRUE, FALSE );
					Last_Comms_Success_RTCTimestamp = RTC_epoch_now();
					Last_Comms_Success_Flg = TRUE;
					if(Meter_Reset == TRUE)
					{
						Meter_Reset = FALSE;
						push_event_to_cache(UDP_EVTTYP_MPB_RESET);
						Tasks_Priority_Register |= SEVERE_EVENTS_TASK;
						Clear_all_active_alarms();
					}

					if((glSystem_cfg.Coin_Only_Temporary_GW_Enable == TRUE) && (glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)
							&& (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))
					{
						Coin_Only_Audit_to_Flash(); //writing coin details which were not wrote into flash
						//Send Audit Registry data
						Send_Audit_Via_Temp_GW();
					}
				}

				if(result == GPRS_COMMERR_NONE)
				{
					LoRa_comms_success_count++;
					memset(LoRa_Queue[LoRa_Packet_Index].Packet, 0 ,MAX_LORA_PACKET_LENGTH);
					LoRa_Queue[LoRa_Packet_Index].Request_size = 0;
					LoRa_Queue[LoRa_Packet_Index].Retrycount = 0;
					LoRa_Packet_Index = 0;
					if(LoRa_Queue_Count>0)
						LoRa_Queue_Count--;
					//}//LNGSIT-883: this was creating modem staying on issue
					SLEEP_ENABLE = TRUE;
					if(Duty_Cycle_Mode == FALSE)
					{
						Duty_Cycle_Mode = TRUE;
						SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );
					}
					if(LORA_PWR == TRUE)	//Turn Off LORA if it is On
					{
						LORA_PWR = FALSE;
						LORA_power_off();
					}
					Waiting_4LORA_Resp = FALSE;
					//Debug_Output1(0,"Rxd_Resp_Data_Type: %d",glComm_response[5]);
				}
				break;
			case GPRS_COMM_PAMBayStatusUpdate:
			case GPRS_COMM_LOB_BayStatus_diagnostics:
			case GPRS_COMM_PAMPushAck:
			case GPRS_COMM_Evt:
			case GPRS_COMM_OLT_AES:
			case GPRS_COMM_CT:
			case GPRS_COMM_CT_PROFILE:
			case GPRS_COMM_BINARY_EVENTS:  // this is only for smartcard payment
			case GPRS_SSM_AUDITINFO:
				//TODO: Extract response by removing the LoRa header and then add processing including clear from LORA Q etc
				if ( lora_resp_length >= glSystem_cfg.gen_resp_hdr_sz )
				{
					gen_resp_hdr_len = gprs_fill_response_hdr( recvd_resp_p, lora_resp_length, &recvd_resp_hdr );
					recvd_resp_p     += gen_resp_hdr_len;

					if ( recvd_resp_hdr.server_resp.txn_resp_code != 200 )
						result = GPRS_COMMERR_SERVER_ISSUE;
					else
						result = GPRS_COMMERR_NONE;//2006

					if(waiting_for_batch_response != true)
						Debug_Output2( 0, "Processed Req_Type:%d  ERR Code = %d", LoRa_Queue[LoRa_Packet_Index].Packet[1],result );

					if((result == GPRS_COMMERR_NONE) && (LoRa_Queue[LoRa_Packet_Index].Packet[1] == GPRS_COMM_OLT_AES))
					{
						glOLT_resp.auth_code = glComm_response[26];
						Handle_OLT_response();
						Debug_TextOut(0,"OLT OK");
					}
					else if((result == GPRS_COMMERR_NONE) && (LoRa_Queue[LoRa_Packet_Index].Packet[1] == GPRS_COMM_CT))
					{
						Debug_TextOut(0,"CT 55 OK");
					}
					else if((result == GPRS_COMMERR_NONE) && (LoRa_Queue[LoRa_Packet_Index].Packet[1] == GPRS_COMM_CT_PROFILE))
					{
						Debug_TextOut(0,"CT 125 OK");
					}
					else if((result == GPRS_COMMERR_NONE) && (LoRa_Queue[LoRa_Packet_Index].Packet[1] == GPRS_COMM_BINARY_EVENTS)
							&& (LoRa_Queue[LoRa_Packet_Index].Packet[9] == 163))
					{
						Debug_TextOut(0,"SCT OK");
					}
					else if((result == GPRS_COMMERR_NONE) && (LoRa_Queue[LoRa_Packet_Index].Packet[1] == GPRS_SSM_AUDITINFO))
					{
						Debug_TextOut(0,"GW Audit Done");
						AUDIT_Screen();
						DelayMs(3000);
						diag_text_Screen( "Pls Clear the Jam", TRUE, FALSE );
						DelayMs(3000);
						Coin_WakeCount_adjustment(JAM_CLEAR_UPWARDS_ADJUSTMENT);

						//Clear Audit Data and go to Idle state
						//Reset parameters
						Flash_Batch_Flag_Read();
						coin_type3 = 0;
						coin_type2 = 0;
						coin_type1 = 0;
						coin_type0 = 0;
						coin_type_invalid = 0;
						total_coin_values = 0;
						glTotal_SCT_TransCount = 0;
						glTotal_SCT_TransAmount = 0;
						for(i=0;i<20;i++)
							{
								coin_type[i] = 0;
							}
						Flash_Batch_Flag_Write();
						Idle_Screen();
					}
					else if((result == GPRS_COMMERR_NONE) && (waiting_for_batch_response == true))
					{
						GPRS_comms_success_count++;
						glComm_failure_count = 0;
						t_mobile_selected_apn_retry_flag = TRUE;
						Debug_Output2( 0, "Processed Req_Type:%d  ERR Code = %d", comm_queue.batch_request[1],result );
						Debug_Output1(0,"Batch Clr: %d",Batch_Index_Sent);
						flag_check_addr[Batch_Index_Sent] = 0;
						Flash_Batch_Flag_Write();
						if(comm_queue.batch_request[1] == GPRS_COMM_BINARY_EVENTS) //LNGSIT-1023
						{
							TechMenu_Get_Event_Update_Time();
						}
						memset(comm_queue.batch_request,0,sizeof(comm_queue.batch_request));
						//Flash_del_add=(uint32_t)(batch_start_address)+(uint32_t)(QUEUE_PACKET_SIZE*Batch_Index_Sent);
						//Flash_Write( comm_queue.batch_request,sizeof(comm_queue.batch_request),Flash_del_add,false);
						Queue_failure_count_l=0;
						Batch_clear_continuous_fail = 0;
						waiting_for_batch_response = false;

						for (j = 0; j < (Q_Big-1); j++)
						{
							if (flag_check_addr[j] == 1)    //This to check any flag pending.
								break;
						}

						if (j == (Q_Big-1))
						{
							//Debug_Output1(0,"Q_Big:%d",j);
							switch(batch_start_address)
							{
							case (AD_olt_r):
								OLT_Q_count=0;
								push_event_to_cache( UDP_EVTTYP_OLT_BATCH_EMPTY);
								comm_queue.OLT_batch_erase       = true;
								cc_approved = 0; //reset for next olt payment //vinay
								//Debug_TextOut( 0, "Com Batch empty");
								//Debug_Output1( 0, "Com cleared cc_approved = %d", cc_approved );
								memset(comm_queue.OLT_batch_flag, 0, sizeof(comm_queue.OLT_batch_flag));
								Debug_TextOut(0,"OLT Batch Empty");
								break;
							case (AD_COIN_r):
								Coin_Q_count=0;
								push_event_to_cache( UDP_EVTTYP_COIN_BATCH_EMPTY);
								comm_queue.coin_batch_erase       = true;
								memset(comm_queue.coin_batch_flag, 0, sizeof(comm_queue.coin_batch_flag));
								Debug_TextOut(0,"CT Batch Empty");
								break;
							case (AD_ACK_r):
								OLTAck_Q_count=0;
								push_event_to_cache( UDP_EVTTYP_OLTACK_BATCH_EMPTY);
								comm_queue.OLTAck_batch_erase       = true;
								memset(comm_queue.OLTAck_batch_flag, 0, sizeof(comm_queue.OLTAck_batch_flag));
								Debug_TextOut(0,"OLT-ACK Batch Empty");
								break;
							case (AD_ALARM_r):
								Alarms_Q_count=0;
								push_event_to_cache( UDP_EVTTYP_ALARM_BATCH_EMPTY);
								comm_queue.Alarms_batch_erase       = true;
								memset(comm_queue.Alarms_batch_flag, 0, sizeof(comm_queue.Alarms_batch_flag));
								Debug_TextOut(0,"Alarms Batch Empty");
								break;
							case (AD_BAYSTATUS_r):
								BayStatus_Q_count=0;
								push_event_to_cache( UDP_EVTTYP_BAYSTATUS_BATCH_EMPTY );
								comm_queue.BayStatus_batch_erase       = true;
								memset(comm_queue.BayStatus_batch_flag, 0, sizeof(comm_queue.BayStatus_batch_flag));
								Debug_TextOut(0,"BSU Batch Empty");
								break;
							case (AD_SMART_CARD_r):
								SCT_Q_count=0;
								push_event_to_cache( UDP_EVTTYP_SCT_BATCH_EMPTY);
								comm_queue.SCT_batch_erase       = true;
								memset(comm_queue.SCT_batch_flag, 0, sizeof(comm_queue.SCT_batch_flag));
								Debug_TextOut(0,"SCT Batch Empty");
								break;
							case (AD_EVENTS_BATCH_RW):
								EVENTS_Q_COUNT=0;
								push_event_to_cache( UDP_EVTTYP_EVENTS_BATCH_EMPTY );
								comm_queue.EVENTS_batch_erase       = true;
								memset(comm_queue.EVENTS_batch_flag, 0, sizeof(comm_queue.EVENTS_batch_flag));
								Debug_TextOut(0,"Events Batch Empty");
								break;
							case (AD_PAMBayStatus_w):
								PAMBayStatus_Q_count = 0;
								push_event_to_cache( UDP_EVTTYP_PAMBAYSTATUS_BATCH_EMPTY );
								comm_queue.PAMBayStatus_batch_erase       = true;
								memset(comm_queue.PAMBayStatus_batch_flag, 0, sizeof(comm_queue.PAMBayStatus_batch_flag));
								Debug_TextOut(0,"PAM-BS Batch Empty");
								break;
							}
							Flash_Batch_Flag_Write();
						}
					}
					else if((result == GPRS_COMMERR_NONE) && (LoRa_Queue[LoRa_Packet_Index].Packet[1] == GPRS_COMM_BINARY_EVENTS))
					{
						//LNGSIT-1195
						if((glComm_response[23] | (glComm_response[24]<<8)) == UDP_EVTTYP_ALARM)
						{
							TechMenu_Get_Event_Update_Time(); //LNGSIT-1023
							Debug_TextOut(0,"Alarms OK");
						}
						else if((glComm_response[23] | (glComm_response[24]<<8)) == FILE_ACTIVATION_SUCCESS)
						{
							TechMenu_Get_Event_Update_Time(); //LNGSIT-1023
							Flash_Batch_Flag_Read();
							for(file_index = 0; file_index < FFACT_TOTAL_FILES;  file_index++)
							{
								if(flash_file_actvn_tbl[file_index].status == FF_ACTIVATE_COMPLETED)
								{
									flash_file_actvn_tbl[file_index].status = FF_161_COMPLETED;
									flash_file_actvn_tbl[file_index].actvn_try_count = 0;
								}
							}
							Flash_Batch_Flag_Write();
							Debug_TextOut(0,"161 Event OK");
						}
						else if((glComm_response[23] | (glComm_response[24]<<8)) == FILE_ACTIVATION_FAILURE)
						{
							TechMenu_Get_Event_Update_Time(); //LNGSIT-1023
							Debug_TextOut(0,"160 Event OK");
						}
						else
						{
							TechMenu_Get_Event_Update_Time();
							init_event_cache(0xff);	//Live events were posted, clear them now, LNGSIT-405
							Debug_TextOut(0,"Evts OK");
						}
					}
					else if((result == GPRS_COMMERR_NONE) && (LoRa_Queue[LoRa_Packet_Index].Packet[1] == GPRS_COMM_Evt))
					{
						Debug_TextOut(0,"OLTAck OK");
					}
					else if((result == GPRS_COMMERR_NONE) && (LoRa_Queue[LoRa_Packet_Index].Packet[1] == GPRS_COMM_PAMBayStatusUpdate))
					{
						Debug_TextOut(0,"PAM BS OK");
					}
					else if((result == GPRS_COMMERR_NONE) && (LoRa_Queue[LoRa_Packet_Index].Packet[1] == GPRS_COMM_LOB_BayStatus_diagnostics))
					{
						Debug_TextOut(0,"BSU OK");
					}
					else if((result == GPRS_COMMERR_NONE) && (LoRa_Queue[LoRa_Packet_Index].Packet[1] == GPRS_COMM_PAMPushAck))
					{
						Debug_TextOut(0,"PUSH ACK OK");
					}

					//Batch Eligible packets if wrong response from Server also
					if(result != GPRS_COMMERR_NONE)	//LNGSIT-743: Meter partially hanged after ERR=13 for OLT
					{
						if(LoRa_Queue[LoRa_Packet_Index].Packet[1] == GPRS_COMM_OLT_AES)
						{
							glOLT_resp.auth_code = GPRS_OLT_GWAY_RESP_Timeout;   //Batch OLT and show approved
							Handle_OLT_response();
							Queue_Request(LoRa_Queue[LoRa_Packet_Index].Packet,LoRa_Queue[LoRa_Packet_Index].Request_size );
						}
					}

					if(result == GPRS_COMMERR_NONE)	// || (result == GPRS_COMMERR_SERVER_ISSUE)) //Let invalid response failures also go through retries
					{
						LoRa_comms_success_count++;
						memset(LoRa_Queue[LoRa_Packet_Index].Packet, 0 ,MAX_LORA_PACKET_LENGTH);
						LoRa_Queue[LoRa_Packet_Index].Request_size = 0;
						LoRa_Queue[LoRa_Packet_Index].Retrycount = 0;
						LoRa_Packet_Index = 0;
						if(LoRa_Queue_Count>0)
							LoRa_Queue_Count--;
						//}//LNGSIT-883: this was creating modem staying on issue
						SLEEP_ENABLE = TRUE;
						if(Duty_Cycle_Mode == FALSE)
						{
							Duty_Cycle_Mode = TRUE;
							SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );
						}
						if(LORA_PWR == TRUE)	//Turn Off LORA if it is On
						{
							LORA_PWR = FALSE;
							LORA_power_off();
						}
						Waiting_4LORA_Resp = FALSE;
						result = GPRS_COMMERR_NONE;
						//Debug_Output1(0,"Rxd_Resp_Data_Type: %d",glComm_response[5]);
					}
					break;
				}
			default:
				break;
			}
		}
		else if((glComm_response[4] == ACK) && (RECV_SEQ_NUM_CHK_FLAG == 1))
		{
			switch(glComm_response[5])
			{
			case GPRS_COMM_PAMPushMsg:
				result = GPRS_COMMERR_NONE;

				//Debug_Display(0,"GPRS_COMM_PAMPushMsg\r\n");
				last_payment_try_time[Current_bayStatus] = RTC_epoch_now();
				//if(glLast_reported_space_status[Current_Space_Id] == 0)
				if((glSystem_cfg.vehicle_sensor_enable == TRUE)&&((glLast_reported_space_status[Current_bayStatus] == 0) && ((current_rate_index < 20) || (current_rate_index == PREPAY_PARKING))) )
				{
					ZERO_OUT_ENABLE_AT_EXPIRY[Current_bayStatus] = TRUE;
					ZERO_OUT[Current_bayStatus] = FALSE;
					//Debug_TextOut(0,"PA on Vac, Won't ZO till next 801");
					Debug_Output1(0,"PA on Vac, No ZO till next 801 for space [%d]",Current_bayStatus);
				}
				memcpy(&PBC_amnt_rcvd,&glComm_response[12],(PBC_AMOUNT_LENGTH));
				memcpy(&PBC_expt_rcvd,&glComm_response[14],(PBC_EXPIRY_TIME_LENGTH));

				check_if_special_code_recvd 	= (((0xFF & PBC_amnt_rcvd[1]) << 8)
										| (0xFF & PBC_amnt_rcvd[0]));
				if(check_if_special_code_recvd == 65002)
				{
					Tasks_Priority_Register |= GENERAL_SYNC_TASK;
					//force a general sync; designed specifically for Denver demo
				}
				else if(check_if_special_code_recvd == 65003)
				{
					Meter_Out_of_service();
					//make a meter inactive; designed specifically for Denver demo
				}
				else if(check_if_special_code_recvd == 65004)
				{
					Meter_In_Service();
					//make a meter active; designed specifically for Denver demo
				}
				else
				{
					//Debug_Output2(0,"PBC_AMOUNT: %02x,%02x",PBC_amnt_rcvd[0],PBC_amnt_rcvd[1]);
					//Debug_Output6(0,"PBC_EXPIRY_TIME: %02x,%02x,%02x,%02x",PBC_expt_rcvd[0],PBC_expt_rcvd[1],PBC_expt_rcvd[2],PBC_expt_rcvd[3],0,0);
					memcpy(&Current_extracted_time_PBC[Current_bayStatus],&glComm_response[14],(PBC_EXPIRY_TIME_LENGTH));
					//Debug_Output1(0,"Current_extracted_PBC = %X", Current_extracted_time_PBC);
					if(Current_extracted_time_PBC[Current_bayStatus] > Last_processed_time_PBC[Current_bayStatus])
					{
						Parse_PBC_Payment();
						gprs_push_notify();

						Last_processed_time_PBC[Current_bayStatus] = Current_extracted_time_PBC[Current_bayStatus];
						//Debug_Output1(0,"Last_processed_PBC = %X", Last_processed_time_PBC);
						push_event_to_cache(UDP_EVTTYP_PAY_BY_CELL_ACK);	//secondary confirmation of PBC
					}
					else
					{
						Debug_TextOut(2,"Same_PBC_Packet_Received");
					}
				}

				SLEEP_ENABLE = TRUE;
				if(Duty_Cycle_Mode == FALSE)
				{
					Duty_Cycle_Mode = TRUE;
					SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );
				}
				break;

			case GPRS_COMM_BayStatus_diagnostics:
				//Debug_TextOut(2,"BayStatus-01");
				if(glSystem_cfg.vehicle_sensor_enable == 1)
				{
					BSU_Chunk_received = TRUE;
					result = GPRS_COMMERR_NONE;

					memcpy(&current_extracted_time[Current_bayStatus], &glComm_response[BSU_TS_INDEX],4);

					if(current_extracted_time[Current_bayStatus] > Last_processed_time[Current_bayStatus])
					{
						//Debug_TextOut(2,"BayStatus-02");
						glSNSR_TMP = (glComm_response[BSU_SENSOR_PARAMS_INDEX+1]<<8) | glComm_response[BSU_SENSOR_PARAMS_INDEX];
						memcpy(glSNSRdiag_string,&glComm_response[BSU_SENSOR_PARAMS_INDEX+2],(SENSOR_DIAG_DATA_LENGTH));
						Current_bayStatus = glComm_response[BSU_EXTRACT_INDEX];

						Send_ACK2Node(glComm_response);
						OnSlave();
						Radio->SetTxPacket( Buffer, BufferSize );
						int i =0;
						for(i=0; i<3000; i++)
						{
							OnSlave();
						}

						occupancygl = Current_bayStatus;
						Current_ZO_eligibility = glComm_response[BSU_ZO_INDEX];
						ELIGIBLE_FOR_ZERO_OUT = Current_ZO_eligibility;
						//ELIGIBLE_FOR_ZERO_OUT = 0;
						Debug_Output2(0,"BS = %d, ZO = %d", Current_bayStatus, Current_ZO_eligibility);
						//poll_space_status();	//This was one reason of config crash since it raises events inside ISR, resolved with below routine now
						//do_Sensor_Update = TRUE;
						Last_processed_time[Current_bayStatus] = current_extracted_time[Current_bayStatus];
						Tasks_Priority_Register |= SENSOR_UPDATE_TASK;	//To save power, no need to update IN/ OUT events immediately, send only BSU via LoRa
					}
					else
					{
						//Debug_Output2(2,"Current_time =%ld, Last_time = %ld",current_extracted_time,Last_processed_time);
					}
					SLEEP_ENABLE = FALSE;
					if(Duty_Cycle_Mode == FALSE)
					{
						Duty_Cycle_Mode = TRUE;
					}
					BSU_Chunk_received = FALSE;
				}
				else
				{
					//Debug_TextOut( 0, "BSU received, but Sensor is not enabled in Config" );
				}
				break;

			default:
				break;
			}
		}
	}
	return result;
}

/**************************************************************************/
//! Response handler for the LORA communication packets
//! Handles the LORA packets received from sensor or gateway
//! \param void
//! \return void
/**************************************************************************/
void Handle_LORA_Response()
{
	uint16_t do_not_delete_this = sizeof(glComm_response);
	uint8_t		result = GPRS_COMMERR_NONE;
	Radio->GetRxPacket( glComm_response, ( uint16_t* )&do_not_delete_this);

	/*Debug_Output6(0,"%X %X %X %X %X %X",glComm_response[0],glComm_response[1],glComm_response[2],glComm_response[3],glComm_response[4],glComm_response[5]);
	Debug_Output6(0,"%X %X %X %X %X %X",glComm_response[6],glComm_response[7],glComm_response[8],glComm_response[9],glComm_response[10],glComm_response[11]);*/

    LQI_diagnostic[LQI_counter++] = RxPacketRssiValue + DEFAULT_LQI_ADDER;
    Debug_Output1(0,"LQI: %d",LQI_diagnostic[LQI_counter-1]);
    if(LQI_counter >= MAX_DIAG_READINGS)
    	LQI_counter = 0;

	result = Process_Incoming_Sensor_Response(do_not_delete_this);
	if((Waiting_4LORA_Resp == TRUE) && (result != GPRS_COMMERR_NONE))
	{
		Radio->StartRx( );
	}
	//else if(queue_pending == TRUE)
		//Process_Flash_Queue = TRUE;

	memset(glComm_response, 0, sizeof(glComm_response));
	//Debug_TextOut(2,"ready");
}

/**************************************************************************/
//! Frame the Acknowledgement packet for Sensor after receiving a BSU successfully
//! \param pointer to the header of the LoRa packet being framed
//! \return void
/**************************************************************************/
void Send_ACK2Node(uint8_t *header)
{
	memcpy(Buffer,header,12);
	Buffer[12] = 0xC8;
	Buffer[13] = 0;
	BufferSize = 14;
}

/**************************************************************************/
//! Frames the Pay By Cell packets to be sent to other meter which current meter received
//! \param void
//! \return void
/**************************************************************************/
void PBC_TO_OTHER_METER()
{
	uint16_t i = 0;
	uint32_t lRFFrequency = 0;

	memcpy(&PBC_other_MID_tx,&glComm_response[12],(MC_PBC_TO_LIBRTY_LENGTH));

	uint8_t * req_p = request_events;

	Debug_TextOut(0,"PBC_TO_OTHER_METER\r\n");

	PBC_to_Other_Meter = TRUE;
	//SEND_SEQ_NUM_CHK_FLAG = TRUE;

	if(Seq_num < 255)
		Seq_num++;
	else
		Seq_num	= 0;

	*((uint8_t*)req_p)     = 0XFE;
	req_p                  += sizeof( uint8_t );

	*((uint8_t*)req_p)     = 0XFA;
	req_p                  += sizeof( uint8_t );

	*((uint8_t*)req_p)     = 0;
	req_p                  += sizeof( uint8_t );

	*((uint8_t*)req_p)     = ( uint8_t )(Seq_num);
	req_p                  += sizeof( uint8_t );


	*((uint8_t*)req_p)      = ACK;
	req_p                  += sizeof( uint8_t );

	*((uint8_t*)req_p)      = GPRS_COMM_PAMPushMsg;
	req_p                  += sizeof( uint8_t );

	*((uint16_t*)req_p)     = received_area_num;
	req_p                  += sizeof( uint16_t );

	*((uint16_t*)req_p)     = received_cust_id;//1055
	req_p                  += sizeof( uint16_t );

	*((uint16_t*)req_p)     = received_meter_id;
	req_p                  += sizeof( uint16_t );

	memcpy(req_p, PBC_other_MID_tx, (MC_PBC_TO_LIBRTY_LENGTH));
		req_p += (MC_PBC_TO_LIBRTY_LENGTH);

	if(TRUE == SX1276LoRaCheckTXState())
	{
		if(PBC_to_Other_Meter == TRUE)  //Rxd PBC of other MID from GW, Tx the rxd PBC using Preamble and meterId freq.
		{
			if(LORA_PWR != TRUE)
			{
				lRFFrequency = IDBasedFreqCalculation(received_meter_id); // Freq Calculation
				SX1276LoRaSetRFFrequency(lRFFrequency);
				//Debug_Output1(0,"calc_freq_for_Rxd_MID = %d",calc_freq_for_Rxd_MID);
				LoRaSettings1.RxPacketTimeout = 1;	//20 seconds timeout
				LoRaSettings1.RxSingleOn = 1;
				LORA_PWR = TRUE;
				Init_LORA();
			}

			OnSlave();
			//Radio->SetTxPacket( Buffer, BufferSize );
			Radio->SetTxPacket(request_events, (uint16_t)(req_p - request_events));
			for(i=0; i<5000; i++)
			{
				OnSlave();
				if(SLEEP_ENABLE == TRUE)
					break;
			}
		}
	}

	PBC_to_Other_Meter = FALSE;
	SLEEP_ENABLE = FALSE;

	if(LORA_PWR == TRUE)	//Turn Off LORA if it is On
	{
		LORA_PWR = FALSE;
		LORA_power_off();
	}
}

/**************************************************************************/
//! Generate the RTC request packet to be sent to Gateway in Coin only meters
//! \param void
//! \return void
/**************************************************************************/
void get_RTC()
{
	uint8_t * req_p = request_events;

	SEND_SEQ_NUM_CHK_FLAG = TRUE;

	if(Seq_num < 255)
		Seq_num++;
	else
		Seq_num = 0;

	*((uint8_t*)req_p)     = 0XFE;
	req_p                  += sizeof( uint8_t );

	*((uint8_t*)req_p)     = 0XFA;
	req_p                  += sizeof( uint8_t );

	*((uint8_t*)req_p)     = 0;
	req_p                  += sizeof( uint8_t );

	*((uint8_t*)req_p)     = ( uint8_t )(Seq_num);
	req_p                  += sizeof( uint8_t );

	req_p += gprs_fill_request_hdr( LORA_NODE_RTC, DATA, req_p, sizeof( request_events ), false );

	gprs_do_request( request_events, (uint16_t)(req_p - request_events));
}

/**************************************************************************/
//! Generate the Communication packet to clear all active alarms from server
//! and adds it to communication queue
//! \param void
//! \return void
/**************************************************************************/
void Clear_all_active_alarms(void)
{
	glAlarm_req.alarm_type  = 92;
	glAlarm_req.alarm_size  = 3;
	glAlarm_req.alarm_ts    = RTC_epoch_now();
	glAlarm_req.alarm_state = MB_ALARM_STATE_CLEARED;
	glAlarm_req.alarm_id    = 0 ;

	gprs_do_Alarms(&glAlarm_req);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
