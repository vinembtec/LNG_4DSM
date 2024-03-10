//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_events.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_events_api
//! @{
//
//*****************************************************************************

#include "LibG2_events.h"

extern GPRSSystem           	glSystem_cfg;
extern uint8_t					Queue_failure_count_l, glComm_failure_count, Batch_clear_continuous_fail;
extern uint16_t			    	last_minimum_vltg;	//03-06-2013: voltage level in low battery event
extern volatile uint32_t		parking_time_left_on_meter[MSM_MAX_PARKING_BAYS] ;
extern uint8_t              	AES_KEY[AES_KEY_SIZE];//14-05-2013:DPLIBB-481
extern uint8_t 					last_SC_serial_no[6];//it is not declared anywhere in the code but still they are using it with extern //vinay
extern uint16_t 				SCT_refund_amount;//it is not declared anywhere in the code but still they are using it with extern //vinay
extern GPRS_AlarmsRequest   	glAlarm_req;
extern FlashFieActvnTblEntry   	flash_file_actvn_tbl[ FILE_TYPE_TOTAL_FILES ]; //24-05-12
extern uint8_t         			DLST_FLAG_CHECK;
extern uint8_t 					flash_data_read_buffer[ FLASH_DATA_READ_BUF_SIZE ];
extern GPRS_Queue 				Queue[MAX_QUEUE_COUNT];
/*extern */uint8_t 					glDLST_FLAGS[10]; //it was declared in rtc.c and it was not used there. So declaring here itself//vinay
extern uint8_t					Process_Flash_Queue, LORA_PWR;
extern uint32_t 				last_pktsent_time;
extern uint16_t 				Current_Space_Id;
extern uint8_t 					waiting_for_batch_response, Waiting_4GPRS_Resp;
//extern uint16_t 				baystatus_Space_Id;
extern uint8_t 					Current_bayStatus;
extern uint8_t 					sensor_tnx_flag;
//extern uint16_t					max_sens, min_sens;
//extern uint8_t					ratio_1, ratio_2;
extern uint32_t					Tasks_Priority_Register;
extern uint16_t					GPRS_comms_attempt_count, Original_packet_count;
extern uint8_t					ZERO_OUT_ENABLE_AT_EXPIRY[MSM_MAX_PARKING_BAYS],ZERO_OUT[MSM_MAX_PARKING_BAYS];
extern uint8_t					Coin_cal_index, add_2_seconds;
extern uint16_t					Coin_cal_max, Coin_cal_min;

//#define 						Max_Retrycount			3

Maintenance_Evt      			Maintenance_Log;
//EventCache                		glEvent_cache;
EventCache                		glEvent_cache[MSM_MAX_PARKING_BAYS];
uint32_t 						olt_RSAD        = AD_olt_r,
								/*olt_ack_RSAD    = AD_ACK_r,//not used in this program //vinay
								coin_RSAD       = AD_COIN_r,//not used in this program //vinay
								alarm_RSAD      = AD_ALARM_r,//not used in this program //vinay
								bay_status_RSAD = AD_BAYSTATUS_r,//not used in this program //vinay
								SCT_batch_RSAD  = AD_SMART_CARD_r,//25-06-12:SCT Batch//not used in this program //vinay*/
								EVENTS_BATCH_WSAD = AD_EVENTS_BATCH_RW;
								//PAMBayStatus_batch_WSAD = AD_PAMBayStatus_w,
								//PAMBayStatus_batch_RSAD = AD_PAMBayStatus_r;

uint32_t 						Current_Timestamp=0,OLT_Timestamp=0,Timestamp_diff=0;

uint8_t                 		OLTAck_Q_count    = 0,
                        		Coin_Q_count      = 0,
								BayStatus_Q_count = 0,
								OLT_Q_count       = 0,
								Alarms_Q_count    = 0,
								SCT_Q_count		  = 0,
								/*Audit_Q_count     = 0,*/
								PAMBayStatus_Q_count=0,
								EVENTS_Q_COUNT	  = 0,
								queue_pending     = true,
								EVENTS_Q_FULL     = false;//,//not used in this program //vinay
								/*CLR_BATCH_ON_QFULL = false;*///not used in this program //vinay
/*uint8_t                 		OLT1_overwrite      = 0,
                        		Alarms_overwrite    = 0,
								OLTAck_overwrite    = 0,
								BayStatus_overwrite = 0,
								CT_overwrite        = 0;*/
uint8_t 						queue_events        = false;
uint16_t						Total_Events_count  = 0;//pp
QUEUE                   		comm_queue;
//uint8_t 						Next_Batch_task=0;//not used in this program //vinay
//uint8_t 						Next_Batch_Erase=0;//not used in this program //vinay
//uint8_t 						Batch_Retrycount=0;//not used in this program //vinay
uint8_t                       	*flag_check_addr  = 0;
uint32_t 						batch_start_address = 0;
uint16_t 						Batch_Index_Sent = 0;
//uint8_t  						queue_pending_1     = false;//not used in this program //vinay
uint32_t 						last_pktsent_time_1 = 0;
uint16_t                    	Present_value = 0;
//uint16_t                    	Coin_Event_Max = 0, Coin_Event_Min = 0;
//uint8_t                    		Coin_Event_Rat1 = 0, Coin_Event_Rat2 = 0;
uint8_t							processwait = 0; // to check the process wait msg and move the screen to DSM_Screen1 //Vinay

//extern const uint8_t 			fontGIF5[488];
//extern uint8_t					Screen_Clear; //not used in this program //vinay

extern uint8_t 					cardread;
extern uint32_t 				coindrop_TimeOut_RTC;
extern uint32_t 				KeyPress_TimeOut_RTC ;
/************************************************************************
*Local Constants
************************************************************************/
// NONE

/************************************************************************
*Local Variables
************************************************************************/
static uint32_t olt_WSAD        = AD_olt_w,
                olt_ack_WSAD    = AD_ACK_w,
                coin_WSAD       = AD_COIN_w,
                alarm_WSAD      = AD_ALARM_w,
                SCT_batch_WSAD  = AD_SMART_CARD_r,
				bay_status_WSAD = AD_BAYSTATUS_r; //25-06-12:SCT Batch

static uint8_t queue_process_message[40] = { 0 };
extern uint32_t						Last_CC_Payment_RTC;
/************************************************************************
*Local Functions
************************************************************************/
// NONE
/**************************************************************************/
//! Initialise communication request queue
//! \param void
//! \return void
/**************************************************************************/
void initialise_queue() //12-07-2013:error handling change for batch transactions
{
	//Debug_TextOut(0,"initialise_queue start");
	uint8_t fcount,Queue_cnt;
	memset( comm_queue.batch_request,        0, sizeof(comm_queue.batch_request));
	memset( comm_queue.OLT_batch_flag,       0, sizeof(comm_queue.OLT_batch_flag) );
	memset( comm_queue.OLTAck_batch_flag,    0, sizeof(comm_queue.OLTAck_batch_flag) );
	memset( comm_queue.BayStatus_batch_flag, 0, sizeof(comm_queue.BayStatus_batch_flag) );
	memset( comm_queue.Alarms_batch_flag,    0, sizeof(comm_queue.Alarms_batch_flag) );
	memset( comm_queue.coin_batch_flag,      0, sizeof(comm_queue.coin_batch_flag) );
	memset( comm_queue.SCT_batch_flag,      0, sizeof(comm_queue.SCT_batch_flag) );
	memset(comm_queue.EVENTS_batch_flag, 0, sizeof(comm_queue.EVENTS_batch_flag));//11-03-2014:hardware sensor code merge
	memset(comm_queue.PAMBayStatus_batch_flag, 0, sizeof(comm_queue.PAMBayStatus_batch_flag));  //pambay status
	memset(glDLST_FLAGS,0,sizeof(glDLST_FLAGS));
	// Flash_Batch_Flag_Write();//only for testing when some junk is there in external flash
	Flash_Batch_Flag_Read();

	if((OLT_Q_count>=1) && (OLT_Q_count<=Q_Big))
	{
		comm_queue.OLT_batch_erase=false;
	}
	else
	{
		comm_queue.OLT_batch_erase=true;
		OLT_Q_count=0;
		memset( comm_queue.OLT_batch_flag, 0, sizeof(comm_queue.OLT_batch_flag) );
	}

	if((OLTAck_Q_count >=1) && (OLTAck_Q_count<=Q_Big))
	{
		comm_queue.OLTAck_batch_erase=false;
	}
	else
	{
		comm_queue.OLTAck_batch_erase=true;
		OLTAck_Q_count=0;
		memset( comm_queue.OLTAck_batch_flag, 0, sizeof(comm_queue.OLTAck_batch_flag) );
	}

	if((BayStatus_Q_count >=1) && (BayStatus_Q_count<=Q_Big))
	{
		comm_queue.BayStatus_batch_erase=false;
	}
	else
	{
		comm_queue.BayStatus_batch_erase=true;
		BayStatus_Q_count=0;
		memset( comm_queue.BayStatus_batch_flag, 0, sizeof(comm_queue.BayStatus_batch_flag) );
	}

	if((Alarms_Q_count >=1) && (Alarms_Q_count<=Q_Big))
	{
		comm_queue.Alarms_batch_erase=false;
	}
	else
	{
		comm_queue.Alarms_batch_erase=true;
		Alarms_Q_count=0;
		memset( comm_queue.Alarms_batch_flag, 0, sizeof(comm_queue.Alarms_batch_flag) );
	}

	if((Coin_Q_count >=1) && (Coin_Q_count<=Q_Big))
	{
		comm_queue.coin_batch_erase=false;
	}
	else
	{
		comm_queue.coin_batch_erase=true;
		Coin_Q_count=0;
		memset( comm_queue.coin_batch_flag, 0, sizeof(comm_queue.coin_batch_flag) );
	}
	//25-06-12:SCT Batch
	if((SCT_Q_count>=1) && (SCT_Q_count<=Q_Big))
	{
		comm_queue.SCT_batch_erase=false;
	}
	else
	{
		comm_queue.SCT_batch_erase=true;
		SCT_Q_count=0;
		memset( comm_queue.SCT_batch_flag, 0, sizeof(comm_queue.SCT_batch_flag) );
	}
	//17-03-2014:hardware sensor code merge
	if((EVENTS_Q_COUNT>=1) && (EVENTS_Q_COUNT<=Q_Big))
	{
		comm_queue.EVENTS_batch_erase=false;
	}
	else
	{
		comm_queue.EVENTS_batch_erase=true;
		EVENTS_Q_COUNT=0;
		memset( comm_queue.EVENTS_batch_flag, 0, sizeof(comm_queue.EVENTS_batch_flag) );
	}

	if((PAMBayStatus_Q_count>=1) && (PAMBayStatus_Q_count<=Q_Big))
	{
		comm_queue.PAMBayStatus_batch_erase=false;
	}
	else
	{
		comm_queue.PAMBayStatus_batch_erase=true;
		PAMBayStatus_Q_count = 0;
		memset( comm_queue.PAMBayStatus_batch_flag, 0, sizeof(comm_queue.PAMBayStatus_batch_flag));
	}

	//17-03-2014:hardware sensor code merge

	for( fcount = 0; fcount < FILE_TYPE_TOTAL_FILES; fcount ++ )
	{
		if((flash_file_actvn_tbl[fcount].status>FF_161_RETRY)&&(flash_file_actvn_tbl[fcount].status!=0xff))//03-10-12:handle 161 retry case
		{
			flash_file_actvn_tbl[fcount].status=FF_161_COMPLETED;
		}
		else if(flash_file_actvn_tbl[fcount].status==0xff)
		{
			flash_file_actvn_tbl[fcount].status=FF_UNKNOWN;
		}
	}
	//25-06-12:SCT Batch
	if(glDLST_FLAGS[0]==0xff)//17-05-12:DLST
	{
		memset(glDLST_FLAGS,0,sizeof(glDLST_FLAGS));
	}
	Flash_Batch_Flag_Write();

	DLST_FLAG_CHECK = 0;    //17-05-12:DLST

	if( (comm_queue.coin_batch_erase      == true) &&
			(comm_queue.Alarms_batch_erase    == true) &&
			(comm_queue.BayStatus_batch_erase == true) &&
			(comm_queue.OLTAck_batch_erase    == true) &&
			(comm_queue.OLT_batch_erase       == true) &&
			(comm_queue.PAMBayStatus_batch_erase == true) &&
			(comm_queue.SCT_batch_erase       == true) &&    //21-11-11 //09-11-12
			(comm_queue.EVENTS_batch_erase	  == true)) //17-03-2014:hardware sensor code merge
	{
		queue_pending=false;
	}

	//Initialize RAM Queue
	for(Queue_cnt = 0; Queue_cnt < MAX_QUEUE_COUNT;Queue_cnt++)
	{
		memset(Queue[Queue_cnt].Packet,0,MAX_PACKET_LENGTH);
		Queue[Queue_cnt].Request_size = 0;
		Queue[Queue_cnt].Retrycount = 0;
	}
	// Debug_TextOut(0,"initialise_queue end");
	return;
}

/**************************************************************************/
//! Queue up failed Communication requests which are eligible for batching
//! in external flash
//! \param uint8_t pointer to the data packet to be batched
//! \param uint16_t size of the data packet to be batched
//! \return void
/**************************************************************************/
void Queue_Request(uint8_t * queued_req_p, uint16_t queued_req_sz)//1810
{
	uint32_t loCurrentRTCTime = RTC_epoch_now();
	uint8_t queue_type = queued_req_p[1];    //DECIDE THE TYPE OF REQUEST
	//uint8_t i=0;

	/*if((queue_type == 2) || (queue_type == 57))
	{
		return;
	}*/
	if((cardread == 1)|| 
			(loCurrentRTCTime < (Last_CC_Payment_RTC + 30)) ||
			((loCurrentRTCTime < (coindrop_TimeOut_RTC + 30))&&(coindrop_TimeOut_RTC < (loCurrentRTCTime + 30)))||
				((loCurrentRTCTime < (KeyPress_TimeOut_RTC + 30)) && (KeyPress_TimeOut_RTC < loCurrentRTCTime + 30)))	//this is avoid please wait msg during CC payment //vinay)
	{
		//Debug_Output1(0,"Events Back cardread :%ld", cardread);
		Debug_TextOut(0,"Processing Please Wait.....");
	}
	else
	{
		//Debug_Output1(0,"Events Front + Back cardread :%ld", cardread);
		Debug_TextOut(0,"Processing Please Wait.....");
		Clear_Screen();
		printSharp32x48(" PROCESSING ",25,0,0);//printSharp32x48(" THIS METER ",35,0,0);
		printSharp32x48("   PLEASE    ", 80,0,0);//printSharp32x48("   COVERS   ", 80,0,0);
		printSharp32x48("    WAIT.....     ", 135,0,0);//printSharp32x48(" TWO SPACES  ", 125,0,0);
		processwait = 1;
	}

	Flash_Batch_Flag_Read();

	switch(queue_type)
	{
	case GPRS_COMM_OLT_AES:
		OLT_Q_count=comm_queue.OLT_batch_flag[QUEUE_COUNT_INDEX];
		if(OLT_Q_count == 0xFF)
		{
			Debug_TextOut(0,"Flash Corruption due to overwrite");
			OLT_Q_count = 0;	//Force Erase OLT section now
		}
		else if(OLT_Q_count >= (Q_Big-1))
		{
			Debug_TextOut(0,"OLT Q FULL");
			break;
		}
		else if(OLT_Q_count == 0)
			comm_queue.OLT_batch_erase = true;

		if( comm_queue.OLT_batch_flag[OLT_Q_count] == 0 )
		{
			olt_WSAD=(uint32_t)(AD_olt_w)+(uint32_t)(QUEUE_PACKET_SIZE*OLT_Q_count);//3011
			if((sizeof(comm_queue.batch_request)>=queued_req_sz)&&((uint32_t)(olt_WSAD+QUEUE_PACKET_SIZE)<=(uint32_t)(AD_olt_w+(FLASH_SECTOR_SIZE*2))))//3011
			{
				Debug_Output1( 0, "Q OLT @ %d", OLT_Q_count );

				queued_req_p[1] = GPRS_BATCHED_OLT_PCHSD_TIME/*GPRS_BATCHED_OLT*/; //Batched OLT identification for server    //2102:txn roll-up

				memset( comm_queue.batch_request, 0, sizeof(comm_queue.batch_request) );
				comm_queue.batch_request[0]= (uint8_t) queued_req_sz;
				memcpy(comm_queue.batch_request+1, queued_req_p, queued_req_sz);///1710
				comm_queue.OLT_batch_flag[OLT_Q_count] = 1;
				olt_WSAD = Flash_Write(comm_queue.batch_request,QUEUE_PACKET_SIZE,olt_WSAD, comm_queue.OLT_batch_erase);
				comm_queue.OLT_batch_erase=false;
				queue_pending = TRUE;
				OLT_Q_count++;
				comm_queue.OLT_batch_flag[QUEUE_COUNT_INDEX]=OLT_Q_count;
				Flash_Batch_Flag_Write();
				//push_event_to_cache( UDP_EVTTYP_OLT_BATCHED );//DPLIBB-601: Removing additional events to reduce events loss issue
			}
		}
		break;

	case GPRS_COMM_BayStatusUpdate:
	case GPRS_COMM_BayStatus_diagnostics:	//15-10-2013: sensor diag
	case GPRS_COMM_LOB_BayStatus_diagnostics: //LOB:30-12-2015
#if	1
		BayStatus_Q_count=comm_queue.BayStatus_batch_flag[QUEUE_COUNT_INDEX];
		if(BayStatus_Q_count == 0xFF)
		{
			Debug_TextOut(0,"Flash Corruption due to overwrite");
			BayStatus_Q_count = 0;	//Force Erase BSU section now
		}
		else if(BayStatus_Q_count >= (Q_Big-1))
		{
			Debug_TextOut(0,"BS Q FULL");
			break;
		}
		else if(BayStatus_Q_count == 0)
			comm_queue.BayStatus_batch_erase = true;

		if(comm_queue.BayStatus_batch_flag[BayStatus_Q_count]==0)
		{
			bay_status_WSAD=(uint32_t)(AD_BAYSTATUS_w)+(uint32_t)(QUEUE_PACKET_SIZE*BayStatus_Q_count);//3011

			if( (sizeof(comm_queue.batch_request) >= queued_req_sz) &&
					((uint32_t)((uint32_t)(bay_status_WSAD)+(uint32_t) (QUEUE_PACKET_SIZE)) <= (uint32_t)((uint32_t)(AD_BAYSTATUS_w)+ (uint32_t)((FLASH_SECTOR_SIZE*2))) ))
			{
				//Debug_Output1( 1, "Q BS @ %d", BayStatus_Q_count );
				memset( comm_queue.batch_request, 0, sizeof(comm_queue.batch_request) );
				comm_queue.batch_request[0] = (uint8_t) queued_req_sz;    //2211
				memcpy(comm_queue.batch_request+1, queued_req_p, queued_req_sz);    //2211
				comm_queue.BayStatus_batch_flag[BayStatus_Q_count] = 1;
				bay_status_WSAD = Flash_Write(comm_queue.batch_request,QUEUE_PACKET_SIZE,bay_status_WSAD, comm_queue.BayStatus_batch_erase);
				comm_queue.BayStatus_batch_erase=false;
				queue_pending = TRUE;
				//Debug_Output1( 0, "Queuing BayStatus Queue no %d ", BayStatus_Q_count );
				BayStatus_Q_count++;
				comm_queue.BayStatus_batch_flag[QUEUE_COUNT_INDEX]=BayStatus_Q_count;
				Flash_Batch_Flag_Write();
				//push_event_to_cache( UDP_EVTTYP_BAYSTATUS_BATCHED );//DPLIBB-601: Removing additional events to reduce events loss issue
			}
		}
#endif
		break;

	case GPRS_COMM_CT:
		Coin_Q_count=comm_queue.coin_batch_flag[QUEUE_COUNT_INDEX];
		if(Coin_Q_count == 0xFF)
		{
			Debug_TextOut(0,"Flash Corruption due to overwrite");
			Coin_Q_count = 0;	//Force Erase Coin section now
		}
		else if(Coin_Q_count >= (Q_Big-1))
		{
			Debug_TextOut(0,"CT Q FULL");
			break;
		}
		else if(Coin_Q_count == 0)
			comm_queue.coin_batch_erase = true;

		if( comm_queue.coin_batch_flag[Coin_Q_count] == 0 )
		{
			coin_WSAD=(uint32_t)(AD_COIN_w)+(uint32_t)(QUEUE_PACKET_SIZE*Coin_Q_count);//3011
			if((sizeof(comm_queue.batch_request)>=queued_req_sz)&&((uint32_t)(coin_WSAD+QUEUE_PACKET_SIZE)<=(uint32_t)(AD_COIN_w+(FLASH_SECTOR_SIZE*2))))
			{
				Debug_Output1( 0, "Q CT @ %d", Coin_Q_count );
				memset( comm_queue.batch_request, 0, sizeof(comm_queue.batch_request) );
				comm_queue.batch_request[0] = (uint8_t) queued_req_sz;    //2211
				memcpy(comm_queue.batch_request+1, queued_req_p, queued_req_sz);    //2211
				coin_WSAD = Flash_Write(comm_queue.batch_request,QUEUE_PACKET_SIZE,coin_WSAD,comm_queue.coin_batch_erase);
				comm_queue.coin_batch_erase=false;
				comm_queue.coin_batch_flag[Coin_Q_count]= 1;
				queue_pending = TRUE;
				Coin_Q_count++;
				comm_queue.coin_batch_flag[QUEUE_COUNT_INDEX] =Coin_Q_count;
				Flash_Batch_Flag_Write();
				//push_event_to_cache( UDP_EVTTYP_COIN_BATCHED );//DPLIBB-601: Removing additional events to reduce events loss issue
			}
		}
		break;

	case GPRS_COMM_CT_PROFILE:
		Coin_Q_count=comm_queue.coin_batch_flag[QUEUE_COUNT_INDEX];
		if(Coin_Q_count == 0xFF)
		{
			Debug_TextOut(0,"Flash Corruption due to overwrite");
			Coin_Q_count = 0;	//Force Erase Coin section now
		}
		else if(Coin_Q_count >= (Q_Big-1))
		{
			Debug_TextOut(0,"CT Q FULL");
			break;
		}
		else if(Coin_Q_count == 0)
			comm_queue.coin_batch_erase = true;

		if( comm_queue.coin_batch_flag[Coin_Q_count] == 0 )
		{
			coin_WSAD=(uint32_t)(AD_COIN_w)+(uint32_t)(QUEUE_PACKET_SIZE*Coin_Q_count);//3011
			if((sizeof(comm_queue.batch_request)>=queued_req_sz)&&((uint32_t)(coin_WSAD+QUEUE_PACKET_SIZE)<=(uint32_t)(AD_COIN_w+(FLASH_SECTOR_SIZE*2))))
			{
				Debug_Output1( 0, "Q CT @ %d", Coin_Q_count );
				memset( comm_queue.batch_request, 0, sizeof(comm_queue.batch_request) );
				comm_queue.batch_request[0] = (uint8_t) queued_req_sz;    //2211
				memcpy(comm_queue.batch_request+1, queued_req_p, queued_req_sz);    //2211
				coin_WSAD = Flash_Write(comm_queue.batch_request,QUEUE_PACKET_SIZE,coin_WSAD,comm_queue.coin_batch_erase);
				comm_queue.coin_batch_erase=false;
				comm_queue.coin_batch_flag[Coin_Q_count]= 1;
				queue_pending = TRUE;
				Coin_Q_count++;
				comm_queue.coin_batch_flag[QUEUE_COUNT_INDEX] =Coin_Q_count;
				Flash_Batch_Flag_Write();
				//push_event_to_cache( UDP_EVTTYP_COIN_BATCHED );//DPLIBB-601: Removing additional events to reduce events loss issue
			}
		}
		break;

	case GPRS_COMM_Evt:
		OLTAck_Q_count=comm_queue.OLTAck_batch_flag[QUEUE_COUNT_INDEX];
		if(OLTAck_Q_count == 0xFF)
		{
			Debug_TextOut(0,"Flash Corruption due to overwrite");
			OLTAck_Q_count = 0;	//Force Erase OLTACK section now
		}
		else if(OLTAck_Q_count >= (Q_Big-1))
		{
			Debug_TextOut(0,"OLT_ACK Q FULL");
			break;
		}
		else if(OLTAck_Q_count == 0)
			comm_queue.OLTAck_batch_erase = true;
		if(comm_queue.OLTAck_batch_flag[OLTAck_Q_count]==0)
		{
			olt_ack_WSAD=(uint32_t)(AD_ACK_w)+(uint32_t)(QUEUE_PACKET_SIZE*OLTAck_Q_count);//3011
			if((sizeof(comm_queue.batch_request)>=queued_req_sz)&&((uint32_t)(olt_ack_WSAD+QUEUE_PACKET_SIZE)<=(uint32_t)(AD_ACK_w+(FLASH_SECTOR_SIZE*2))))
			{
				Debug_Output1( 0, "Q OLT_ACK @ %d", OLTAck_Q_count  );
				memset( comm_queue.batch_request, 0, sizeof(comm_queue.batch_request) );
				comm_queue.batch_request[0] = (uint8_t) queued_req_sz;    //2211
				memcpy(comm_queue.batch_request+1, queued_req_p, queued_req_sz);    //2211
				olt_ack_WSAD = Flash_Write(comm_queue.batch_request,QUEUE_PACKET_SIZE,olt_ack_WSAD, comm_queue.OLTAck_batch_erase);
				comm_queue.OLTAck_batch_erase=false;
				comm_queue.OLTAck_batch_flag[OLTAck_Q_count] = 1;
				queue_pending = TRUE;
				OLTAck_Q_count++;
				comm_queue.OLTAck_batch_flag[QUEUE_COUNT_INDEX]=OLTAck_Q_count;
				Flash_Batch_Flag_Write();
				//push_event_to_cache( UDP_EVTTYP_OLTACK_BATCHED );//DPLIBB-601: Removing additional events to reduce events loss issue
			}
		}
		break;

	case GPRS_COMM_BINARY_EVENTS:
		if((queued_req_p[9]== 0x5C)&&(glAlarm_req.alarm_id!=0))    //2109//3012
		{
			Alarms_Q_count=comm_queue.Alarms_batch_flag[QUEUE_COUNT_INDEX];
			if(Alarms_Q_count == 0xFF)
			{
				Debug_TextOut(0,"Flash Corruption due to overwrite");
				Alarms_Q_count = 0;	//Force Erase Alarms section now
			}
			else if(Alarms_Q_count >= (Q_Big-1))
			{
				Debug_TextOut(0,"Alarms Q FULL");
				break;
			}
			else if(Alarms_Q_count == 0)
				comm_queue.Alarms_batch_erase = true;

			if(comm_queue.Alarms_batch_flag[Alarms_Q_count]==0)
			{
				alarm_WSAD=(uint32_t)(AD_ALARM_w)+(uint32_t)(QUEUE_PACKET_SIZE*Alarms_Q_count);//3011
				if((sizeof(comm_queue.batch_request)>=queued_req_sz)&&((uint32_t)(alarm_WSAD+QUEUE_PACKET_SIZE)<=(uint32_t)(AD_ALARM_w+(FLASH_SECTOR_SIZE*2))))//2109
				{
					Debug_Output1( 0, "Q Alarms @ %d", Alarms_Q_count  );
					memset( comm_queue.batch_request, 0, sizeof(comm_queue.batch_request) );
					comm_queue.batch_request[0] = (uint8_t) queued_req_sz;    //2211
					memcpy(comm_queue.batch_request+1, queued_req_p, queued_req_sz);    //2211
					alarm_WSAD = Flash_Write(comm_queue.batch_request,QUEUE_PACKET_SIZE,alarm_WSAD, comm_queue.Alarms_batch_erase);
					comm_queue.Alarms_batch_erase=false;
					comm_queue.Alarms_batch_flag[Alarms_Q_count]= 1;
					queue_pending = TRUE;
					Alarms_Q_count++;
					comm_queue.Alarms_batch_flag[QUEUE_COUNT_INDEX]=Alarms_Q_count;
					Flash_Batch_Flag_Write();
					//push_event_to_cache( UDP_EVTTYP_ALARM_BATCHED );//DPLIBB-601: Removing additional events to reduce events loss issue
				}
			}
			break;
		}    //2109
		//25-06-12:SCT Batch
		else if(queued_req_p[9]== UDP_EVTTYP_SCT_PAYMENT)
		{
			SCT_Q_count=comm_queue.SCT_batch_flag[QUEUE_COUNT_INDEX];
			if(SCT_Q_count == 0xFF)
			{
				Debug_TextOut(0,"Flash Corruption due to overwrite");
				SCT_Q_count = 0;	//Force Erase SCT section now
			}
			else if(SCT_Q_count >= (Q_Big-1))
			{
				Debug_TextOut(0,"SCT Q FULL");
				break;
			}
			else if(SCT_Q_count == 0)
				comm_queue.SCT_batch_erase = true;
			if(comm_queue.SCT_batch_flag[SCT_Q_count]==0)
			{
				SCT_batch_WSAD=(uint32_t)(AD_SMART_CARD_w)+(uint32_t)(QUEUE_PACKET_SIZE*SCT_Q_count);//3011
				if((sizeof(comm_queue.batch_request)>=queued_req_sz)&&((uint32_t)(SCT_batch_WSAD+QUEUE_PACKET_SIZE)<=(uint32_t)(AD_SMART_CARD_w+(FLASH_SECTOR_SIZE*2))))//2109
				{
					Debug_Output1( 0, "Q SCT @ %d", SCT_Q_count  );
					memset( comm_queue.batch_request, 0, sizeof(comm_queue.batch_request) );
					comm_queue.batch_request[0] = (uint8_t) queued_req_sz;    //2211
					memcpy(comm_queue.batch_request+1, queued_req_p, queued_req_sz);    //2211
					SCT_batch_WSAD = Flash_Write(comm_queue.batch_request,QUEUE_PACKET_SIZE,SCT_batch_WSAD, comm_queue.SCT_batch_erase);
					comm_queue.SCT_batch_erase=false;
					comm_queue.SCT_batch_flag[SCT_Q_count]= 1;
					queue_pending = TRUE;
					SCT_Q_count++;
					comm_queue.SCT_batch_flag[QUEUE_COUNT_INDEX]=SCT_Q_count;
					Flash_Batch_Flag_Write();
					//push_event_to_cache(UDP_EVTTYP_SCT_BATCHED);//DPLIBB-601: Removing additional events to reduce events loss issue

				}
			}
			break;
		}
		//25-06-12:SCT Batch
		else    //2109
		{
			break;
		}
#if 0
		//03-09-2013:DPLIBB-575:batch PAMBayStatus
	case GPRS_COMM_PAMBayStatusUpdate:

		PAMBayStatus_Q_count = comm_queue.PAMBayStatus_batch_flag[QUEUE_COUNT_INDEX];
		if(PAMBayStatus_Q_count == 0xFF)
		{
			Debug_TextOut(0,"Flash Corruption due to overwrite");
			PAMBayStatus_Q_count = 0;	//Force Erase PAM BS section now
		}
		else if(PAMBayStatus_Q_count >= (Q_Big-1))
		{
			Debug_TextOut(0,"PAMBayStatus Q FULL");
			break;
		}
		else if(PAMBayStatus_Q_count == 0)
			comm_queue.PAMBayStatus_batch_erase = true;
		if( comm_queue.PAMBayStatus_batch_flag[PAMBayStatus_Q_count] == 0 )
		{
			PAMBayStatus_batch_WSAD=(uint32_t)(AD_PAMBayStatus_w)+(uint32_t)(QUEUE_PACKET_SIZE*PAMBayStatus_Q_count);//LNGSIT-362,363; SPI Error and invalid batch seen only for PAM BS
			if((sizeof(comm_queue.batch_request)>=queued_req_sz)&&((uint32_t)(PAMBayStatus_batch_WSAD+QUEUE_PACKET_SIZE)<=(uint32_t)(AD_PAMBayStatus_w+(FLASH_SECTOR_SIZE*2))))
			{
				Debug_Output1( 1, "Q PAMBayStatus @ %d", PAMBayStatus_Q_count );
				memset( comm_queue.batch_request, 0, sizeof(comm_queue.batch_request) );
				comm_queue.batch_request[0] = (uint8_t) queued_req_sz;    //2211
				memcpy(comm_queue.batch_request+1, queued_req_p, queued_req_sz);    //2211
				PAMBayStatus_batch_WSAD = Flash_Write(comm_queue.batch_request,QUEUE_PACKET_SIZE,PAMBayStatus_batch_WSAD,comm_queue.PAMBayStatus_batch_erase);
				comm_queue.PAMBayStatus_batch_erase=false;
				comm_queue.PAMBayStatus_batch_flag[PAMBayStatus_Q_count]= 1;
				queue_pending = TRUE;
				PAMBayStatus_Q_count++;
				comm_queue.PAMBayStatus_batch_flag[QUEUE_COUNT_INDEX] =PAMBayStatus_Q_count;
				Flash_Batch_Flag_Write();
				//push_event_to_cache(UDP_EVTTYP_PAMBAYSTATUS_BATCHED);//DPLIBB-601: Removing additional events to reduce events loss issue
			}
		}
		break;
		//03-09-2013:DPLIBB-575:batch PAMBayStatus
#endif
	default:
		break;
	}
	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)	//Revert back the last blank line
	{
		diag_text_Screen(" ", true, true);
	}
	else	//Revert back the RTC line
	{
		diag_text_Screen(" ", true, true);
		Update_RTC_on_Display();
	}
	if(processwait == 1)
	{
		DSM_Screen1();//Idle_Screen();  //this is causing long hold of processing please wait msg. //so uncmmted //vinay
		processwait = 0;
	}
}

/*
 * 18Aug11.Veda: Event Cache related functions
 *
 */

/**************************************************************************/
//! Initialise events request queue
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t init_event_cache(uint8_t reset_event_id)
{
	uint8_t result = EVT_CACHE_ERR_CACHE_DISABLED;


	uint8_t                i, index1;
	EventCacheEntry        * ent_p;

	result = EVT_CACHE_ERR_NONE;
	for(index1 = 0; index1<MSM_MAX_PARKING_BAYS; index1++)
	{
		if(reset_event_id != 0xFF)
			index1 = reset_event_id;
		for ( i = 0, ent_p = glEvent_cache[index1].cache;  i < MAX_EVENT_CACHE_SIZE;  ++i, ++ent_p )
		{
			ent_p->flags.byte = 0;
			ent_p->id = 0;
			ent_p->ts = 0;
		}

		glEvent_cache[index1].next_idx   = 0;
		glEvent_cache[index1].flags.byte = 0;
		glEvent_cache[index1].flags.bits.free=true;

		//glEvent_cache.flags.bits.ovrflw = true;
		for ( i=0;  i < MAX_EVENT_CACHE_SIZE;  ++i)//2208
		{
			glEvent_cache[index1].cache[i].flags.bits.dirty = true;
		}
		if(reset_event_id != 0xFF)
			break;
	}
	return result;
}

/**************************************************************************/
/*  Name        : push_event_to_cache                                     */
/*  Parameters  : uint16_t id                                             */
/*  Returns     : uint8_t                                                 */
/*  Function    : Push events to cache, send events from cache            */
/*------------------------------------------------------------------------*/
uint8_t push_event_to_cache( uint16_t id )
{
	uint8_t result = EVT_CACHE_ERR_CACHE_DISABLED;


	uint8_t                	time_elapsed = 0;
	uint8_t                	idx = 0;
	uint16_t				event_buff_count = 0;
	uint32_t            	ts = RTC_epoch_now();
	EventCacheEntry    		* ent_p;

	if(sensor_tnx_flag == TRUE)
		event_buff_count = Current_bayStatus;
	else
		event_buff_count = 0; //Always to go with UID connected //Current_Space_Id;

	result = EVT_CACHE_ERR_NONE;

	/** Sync code for multi thread **/
	while ( (glEvent_cache[event_buff_count].flags.bits.free == false) && (++time_elapsed < EVT_CACHE_SYNC_TIMEOUT) )
	{
		DelayMs(100);
	}

	if(glEvent_cache[event_buff_count].flags.bits.free == true)
	{
		//Debug_Output1(2, "Current_bayStatus = %d" , event_buff_count);

		glEvent_cache[event_buff_count].flags.bits.free = false;
		idx = glEvent_cache[event_buff_count].next_idx;

		//Debug_Output1(2,"glEvent_cache[event_buff_count].flags.bits.free = %d",glEvent_cache[event_buff_count].flags.bits.free);

		if(Total_Events_count >= 0xFFFF)
		{
			Total_Events_count = 0;
		}
		Total_Events_count ++; //07/10/2015:LOB
		//Debug_Output1(0, "Total_Events_count = %ld" , Total_Events_count);
		//        while ( 1 )
		{
			glEvent_cache[event_buff_count].flags.bits.ovrflw = false;

			ent_p = glEvent_cache[event_buff_count].cache + idx;

			idx ++;
			//Debug_Output2(0, "idx = %d, %d" , idx, event_buff_count);
		/*	if ( idx == MAX_EVENT_CACHE_SIZE )//17-03-2014:hardware sensor code merge
			{
				//gprs_do_send_event_cache(0xff,0);
				idx = 0;
				queue_events = true;
			}*/

			if ( idx == glEvent_cache[event_buff_count].next_idx )
			{
				glEvent_cache[event_buff_count].flags.bits.ovrflw = true;
				result = EVT_CACHE_ERR_OVRWRT;
			}
		}

		glEvent_cache[event_buff_count].next_idx  = idx;
		ent_p->flags.bits.dirty = false;
		ent_p->id               = id;
		//ent_p->ts               = ts;
		Debug_Output1(0, "Event ID = %ld" , id); //to check event counts //vinay

		if(add_2_seconds > 0)	//Server was filtering the same eventcodes with same timestamp //Sudhansu instructions to vinay for Sreekanth Requirement
		{
			ent_p->ts               = ts + add_2_seconds;
		}
		else
		{
			ent_p->ts               = ts;
		}

		if ( glEvent_cache[event_buff_count].next_idx == MAX_EVENT_CACHE_SIZE )//17-03-2014:hardware sensor code merge
		{
			//gprs_do_send_event_cache(0xff,0);
			glEvent_cache[event_buff_count].next_idx = 0;
			idx = 0;
			queue_events = true;
		}

		// 0404: maintenance event
		if((id == UDP_EVTTYP_TECH_MENU_ACCESS) || (id == UDP_EVTTYP_COLLECTION_EVT))
		{
			// memcpy(ent_p->tech_card_sl_no, Maintenance_Log.TECH_CARD_SL_NO, TECH_SL_NO_LEN);
			ent_p->tech_card_sl_no=Maintenance_Log.TECH_CARD_SL_NO;//27-06-12
		}
		else if(id == UDP_EVTTYP_MAINTENANCE_LOG)
		{
			ent_p->PROBLEM_CODE = Maintenance_Log.Prob_Code;
			ent_p->SOLUTION_CODE = Maintenance_Log.Sol_Code;
			// memcpy(ent_p->tech_card_sl_no, Maintenance_Log.TECH_CARD_SL_NO, TECH_SL_NO_LEN);
			ent_p->tech_card_sl_no=Maintenance_Log.TECH_CARD_SL_NO;//27-06-12
		}
		// 0404: maintenance event
		//03-06-2013: voltage level in low battery event
		else if((id == UDP_EVTTYP_LOW_BATT_LEVEL1) || (id == UDP_EVTTYP_LOW_BATT_LEVEL2) || (id == UDP_EVTTYP_LOW_BATT_POWER_SAVE))
		{
			ent_p->tech_card_sl_no=(uint16_t)last_minimum_vltg;//same variable reused to store payload of low battery event
		}
		//03-06-2013: voltage level in low battery event
		//06-04-2015
		else if((id == UDP_EVTTYP_VEHICLE_IN)||(id == UDP_EVTTYP_VEHICLE_OUT)||(id == UDP_EVTTYP_ZEROOUT_ON_OCCUPIED)
				||(id == UDP_EVTTYP_ZEROOUT_ON_VACANT)||(id == UDP_EVTTYP_FREETIME_ON_ENTRY)||(id == UDP_EVTTYP_DIAG_EXCEPTION))//25-06-2013:DPLIBB-514
		{
			ent_p->tech_card_sl_no= (uint32_t)parking_time_left_on_meter[Current_Space_Id];
		}
		else if (id == UDP_EVTTYP_COIN_CALIBRATION_VALUES) //Used to send coin auto/ self calibration values
		{
			ent_p->PROBLEM_CODE = Coin_cal_index;	//Used as Coin Index
			ent_p->tech_card_sl_no = (Coin_cal_max<<16) | Coin_cal_min; //Used to report max and min
		}
		else if(id == UDP_EVTTYP_PARKING_TIME_EXPIRED)
		{
			ZERO_OUT_ENABLE_AT_EXPIRY[Current_bayStatus] = FALSE;
			ZERO_OUT[Current_bayStatus] = TRUE;
		}
/*		else if(id == UDP_EVTTYP_SMARTCARD_REFUND)
		{
			memcpy(last_refund_update.refund_cardid, last_SC_serial_no, sizeof(last_refund_update.refund_cardid));
			last_refund_update.refund_amount = SCT_refund_amount;
		}*/
		//06-04-2015
		glEvent_cache[event_buff_count].flags.bits.free = true;

	}
	else
	{
		result = EVT_CACHE_ERR_SYNC_TIMEOUT;
	}

	if((queue_events == true) || (sensor_tnx_flag == TRUE))
	{
		Debug_TextOut(2,"gprs_do_send_event_cache");
		queue_events = false;
		gprs_do_send_event_cache(0xff, event_buff_count);
	}

	return result;
}

/**************************************************************************/
//! Check valid OLT transactions from batched OLTs, if any batched OLT is
//! older than 7 days, then delete it as per PCI PA-DSS
//! \param void
//! \return void
/**************************************************************************/
void check_valid_batched_OLTs()
{
	uint8_t olt_batch_size,*decrypt,i=0, something_changed = false;
	OLT_Timestamp=0;
	//uint32_t Timestamp_diff=0;
	olt_RSAD=AD_olt_r;

	Flash_Batch_Flag_Read();

	for(i=0; i<(Q_Big-1); i++)
	{
		if((comm_queue.OLT_batch_flag[i] == 1)&&((uint32_t)(olt_RSAD)<(uint32_t)(AD_olt_r+(FLASH_SECTOR_SIZE*2))))
		{
			olt_RSAD=(uint32_t)(AD_olt_r)+(uint32_t)(i*QUEUE_PACKET_SIZE);
			olt_RSAD=Flash_Read(olt_RSAD,QUEUE_PACKET_SIZE);
			olt_batch_size = flash_data_read_buffer[0];    //2211
			memcpy(comm_queue.batch_request,flash_data_read_buffer+1,olt_batch_size);    //2211
			decrypt=comm_queue.batch_request;
			if(comm_queue.batch_request[1]==GPRS_BATCHED_OLT_PCHSD_TIME)//AES
			{
				//todo:handle key id and key to be used for decryption
				decrypt += (sizeof(uint8_t)*10)+2;//skip header and request length byte
				multiple_block_aes_decrypt((uint8_t*)decrypt,(olt_batch_size-12), AES_KEY);//14-05-2013:DPLIBB-481
				memcpy(&OLT_Timestamp,(comm_queue.batch_request+16),sizeof(OLT_Timestamp));
				Current_Timestamp=RTC_epoch_now();
				if((uint32_t)(Current_Timestamp)>(uint32_t)(OLT_Timestamp))
				{
					Timestamp_diff=(uint32_t)((uint32_t)(Current_Timestamp)-(uint32_t)(OLT_Timestamp));
					if((uint32_t)(Timestamp_diff)>=(uint32_t)(OLT_DELTE_TIMESTAMP))	//PCI PA-DSS requirement
					{
						comm_queue.OLT_batch_flag[i]=0xCC;
						//corrupt this particular transaction in flash
						memset(comm_queue.batch_request,0,sizeof(comm_queue.batch_request));
						olt_WSAD=(uint32_t)(AD_olt_w)+(uint32_t)(QUEUE_PACKET_SIZE*i);
						Flash_Write( comm_queue.batch_request,sizeof(comm_queue.batch_request),olt_WSAD, false);
						Debug_TextOut(0, "DELETED too old OLT Batch");
						something_changed = true;
					}
				}
			}
			else
			{
				Debug_Output1(0, "DELETED Invalid OLT Batch @ %d", i);
				comm_queue.OLT_batch_flag[i] = 0;
			}
		}
	}
	if(something_changed == true)	//Overwrite flash only if anything changed
	{
		Flash_Batch_Flag_Write();
	}
	//Timestamp_diff=0;//avoid register optimization
}

/**************************************************************************/
//! Push batched events to flash, events reporting is always from batch
//! \param uint8_t pointer to the data packet to be batched
//! \param uint16_t size of the data packet to be batched
//! \return void
/**************************************************************************/
uint16_t batch_events_in_flash(uint8_t * queued_req_p, uint16_t queued_req_sz)//17-03-2014:hardware sensor code merge
{
	uint16_t result=false;
	uint32_t loCurrentRTCTime = RTC_epoch_now();

	if((cardread == 1)||
			(loCurrentRTCTime < (Last_CC_Payment_RTC + 30))||
			((loCurrentRTCTime < (coindrop_TimeOut_RTC + 30))&&(coindrop_TimeOut_RTC < (loCurrentRTCTime + 30)))||
				((loCurrentRTCTime < (KeyPress_TimeOut_RTC + 30)) && (KeyPress_TimeOut_RTC < loCurrentRTCTime + 30)))	//this is avoid please wait msg during CC payment //vinay)
	{
		//Debug_Output1(0,"Events Back1 cardread :%ld", cardread);
		Debug_TextOut(0,"Processing Please Wait.....");
	}
	else
	{
		//Debug_Output1(0,"Events Front1 cardread :%ld", cardread);
		Debug_TextOut(0,"Processing Please Wait.....");
		Clear_Screen();
		printSharp32x48(" PROCESSING ",25,0,0);//printSharp32x48(" THIS METER ",35,0,0);
		printSharp32x48("   PLEASE    ", 80,0,0);//printSharp32x48("   COVERS   ", 80,0,0);
		printSharp32x48("    WAIT.....     ", 135,0,0);//printSharp32x48(" TWO SPACES  ", 125,0,0);
		processwait = 1;
	}

	Flash_Batch_Flag_Read();
	EVENTS_Q_COUNT=comm_queue.EVENTS_batch_flag[QUEUE_COUNT_INDEX];

	if(EVENTS_Q_COUNT == 0xFF)
	{
		Debug_TextOut(0,"Flash Corruption due to overwrite");
		EVENTS_Q_COUNT = 0;	//Force Erase events section now
	}
	else if(EVENTS_Q_COUNT >= (Q_Big-1))
	{
		Debug_TextOut(0,"EVENTS Q FULL");
		EVENTS_Q_FULL = TRUE;//15/09/2015:LOB-1
		Process_Flash_Queue = TRUE; //LNGSIT-584
		Tasks_Priority_Register |= SEVERE_EVENTS_TASK;
		//push_event_to_cache( UDP_EVTTYP_BAYSTATUS_BATCH_OVRWRT );//DPLIBB-601: Removing additional events to reduce events loss issue
		return result;
	}
	else if(EVENTS_Q_COUNT == 0)
		comm_queue.EVENTS_batch_erase = true;

	EVENTS_Q_FULL = FALSE;
	if( comm_queue.EVENTS_batch_flag[EVENTS_Q_COUNT] == 0 )
	{
		EVENTS_BATCH_WSAD=(uint32_t)(AD_EVENTS_BATCH_RW+(EVENTS_Q_COUNT*QUEUE_PACKET_SIZE));
		if((sizeof(comm_queue.batch_request)>=queued_req_sz)&&((uint32_t)(EVENTS_BATCH_WSAD+QUEUE_PACKET_SIZE)<=(uint32_t)(AD_EVENTS_BATCH_RW+(FLASH_SECTOR_SIZE*2))))//3011
		{
			Debug_Output1( 1, "Q EVENTS @ %d", EVENTS_Q_COUNT );
			memset( comm_queue.batch_request, 0, sizeof(comm_queue.batch_request) );
			comm_queue.batch_request[0]= (uint8_t) queued_req_sz;
			memcpy(comm_queue.batch_request+1, queued_req_p, queued_req_sz);
			comm_queue.EVENTS_batch_flag[EVENTS_Q_COUNT] = 1;
			{
				EVENTS_BATCH_WSAD = Flash_Write(comm_queue.batch_request,QUEUE_PACKET_SIZE,EVENTS_BATCH_WSAD, comm_queue.EVENTS_batch_erase);
				comm_queue.EVENTS_batch_erase=false;
				queue_pending = TRUE;
				EVENTS_Q_COUNT++;
				Original_packet_count++;	//count this also as original event
				comm_queue.EVENTS_batch_flag[QUEUE_COUNT_INDEX]=EVENTS_Q_COUNT;
				Flash_Batch_Flag_Write();
				result=UDP_EVTTYP_EVENTS_BATCHED;
				//push_event_to_cache(UDP_EVTTYP_EVENTS_BATCHED);//DPLIBB-601: Removing additional events to reduce events loss issue
			}
		}
	}
	if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)	//Revert back the last blank line
	{
		diag_text_Screen(" ", true, true);
	}
	else	//Revert back the RTC line
	{
		diag_text_Screen(" ", true, true);
		Update_RTC_on_Display();
	}
	if(processwait == 1)
	{
		DSM_Screen1();//Idle_Screen();  //this is causing long hold of processing please wait msg. //so uncmmted //vinay
		processwait = 0;
	}
	return result;
}

/**************************************************************************/
//! This is the Offline data/ batch processor, it reads batched packets from
//! flash, tries to send those to server and on success, clears those from flash
//! \param void
//! \return void
/**************************************************************************/
void New_process_queue()
{
	uint32_t i=0;
	uint16_t result;
	uint32_t /*Batch_Erase_flg=0,*/Device_addr=0;
	uint32_t loCurrentRTCTime = RTC_epoch_now();

	Waiting_4GPRS_Resp = FALSE;
	if(queue_pending == TRUE)
	{
		if((RTC_epoch_now() > (last_pktsent_time_1 + (GPRS_TIMEOUT))) || (waiting_for_batch_response == false))
		{
			//Flash_Batch_Flag_Read();//already being read inside the next function
			check_valid_batched_OLTs();	//PCI PA-DSS requirement, check if any too old OLTs are there in flash
			for(i = Present_value; i < (Q_Big*8); i++)
			{
				if(i < Q_Big)//<64
				{
					flag_check_addr = comm_queue.OLT_batch_flag;
					//Batch_Erase_flg= comm_queue.OLT_batch_erase;
					batch_start_address = AD_olt_r;
					liberty_sprintf( (char*)queue_process_message,"%s","Send OLT Batch");	// this message will be printed later only when there is something to process
				}
				else if((i >= Q_Big) && (i < (Q_Big*2)))//65 and 129 limit
				{
					flag_check_addr = comm_queue.coin_batch_flag;
					//Batch_Erase_flg= comm_queue.coin_batch_erase;
					batch_start_address = AD_COIN_r;
					liberty_sprintf( (char*)queue_process_message,"%s","Send Coin Batch");	// this message will be printed later only when there is something to process
				}
				else if((i >= Q_Big*2) && (i < (Q_Big*3)))//130 and 194 limit
				{
					flag_check_addr= comm_queue.OLTAck_batch_flag;
					//Batch_Erase_flg= comm_queue.OLTAck_batch_erase;
					batch_start_address =AD_ACK_r;
					liberty_sprintf( (char*)queue_process_message,"%s","Send OLTACK Batch");	// this message will be printed later only when there is something to process
				}
				else if((i >= (Q_Big*3)) && (i < (Q_Big*4)))//195 and 259 limit
				{
					flag_check_addr = comm_queue.SCT_batch_flag;
					//Batch_Erase_flg= comm_queue.SCT_batch_erase;
					batch_start_address = AD_SMART_CARD_r;
					liberty_sprintf( (char*)queue_process_message,"%s","Send SC Batch");	// this message will be printed later only when there is something to process
				}
				else if((i >= (Q_Big*4)) && (i<(Q_Big*5)))//260 and 324 limit
				{
					flag_check_addr = comm_queue.BayStatus_batch_flag;
					//Batch_Erase_flg= comm_queue.BayStatus_batch_erase;
					batch_start_address = AD_BAYSTATUS_r;
					liberty_sprintf( (char*)queue_process_message,"%s","Send BSU Batch");	// this message will be printed later only when there is something to process
				}
				else if((i >= (Q_Big*5)) && (i < (Q_Big*6)))//325 and 389 limit
				{
					flag_check_addr = comm_queue.Alarms_batch_flag;
					//Batch_Erase_flg= comm_queue.Alarms_batch_erase;
					batch_start_address = AD_ALARM_r;
					liberty_sprintf( (char*)queue_process_message,"%s","Send Alarms Batch");	// this message will be printed later only when there is something to process
				}
				else if((i >= (Q_Big*6)) && (i < (Q_Big*7)))//390 and 454 limit
				{
					flag_check_addr = comm_queue.EVENTS_batch_flag;
					//Batch_Erase_flg= comm_queue.EVENTS_batch_erase;
					batch_start_address = AD_EVENTS_BATCH_RW;
					liberty_sprintf( (char*)queue_process_message,"%s","Send Events Batch");	// this message will be printed later only when there is something to process
				}
				else if((i >= (Q_Big*7)) && (i<(Q_Big*8))) //455 and 520 limit
				{
					flag_check_addr = comm_queue.PAMBayStatus_batch_flag;
					//Batch_Erase_flg= comm_queue.EVENTS_batch_erase;
					batch_start_address = AD_PAMBayStatus_w;
					liberty_sprintf( (char*)queue_process_message,"%s","Send PAM BSU Batch");	// this message will be printed later only when there is something to process
				}

				if((i == 64) || (i == 129) || (i == 194) || (i == 259) || (i == 324) || (i == 389)
						|| (i == 454) || (i == 519)) //Any remaining flags
				{
					Present_value = i;

					if(i == ((Q_Big*8)-1))
					{
						//Debug_TextOut( 0, "Process_Flash_Queue=false");
						Present_value = 0;
						Process_Flash_Queue = FALSE;

					}
				}
				else
				{
					if((flag_check_addr[i%Q_Big] == 1))	//Some queue is there to process
					{
						if((Queue_failure_count_l >= glSystem_cfg.Queue_failure_count)/*&&(bootup_process_queue==true)*/)
						{
							Queue_failure_count_l = 0;
							glComm_failure_count++;
							Present_value = i+1;
							Debug_Output1( 0, "Batch Failed @ Q %d ", i%Q_Big);
							Batch_clear_continuous_fail++;
							if(Batch_clear_continuous_fail >= glSystem_cfg.Queue_failure_count)	//Too many failures, give up batch clearing
							{
								Batch_clear_continuous_fail = 0;
								Process_Flash_Queue = FALSE;
								Debug_TextOut( 0, "Giving Up Batch Clearing");
							}

							if((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == TRUE))//3oct18
							{
								if(LORA_PWR == TRUE)	//Turn Off LORA if it is On
								{
									LORA_PWR = FALSE;
									LORA_power_off();
								}
							}
							return;
						}

						Debug_TextOut( 0, (const char *)queue_process_message);

						Device_addr = (batch_start_address)+((i % Q_Big) * QUEUE_PACKET_SIZE);   //base address= base address + 128 bytes
						Device_addr =  Flash_Read(Device_addr,QUEUE_PACKET_SIZE);

						memcpy(comm_queue.batch_request,flash_data_read_buffer+1,flash_data_read_buffer[0]);    //2211

						if((flash_data_read_buffer[0] <= QUEUE_PACKET_SIZE) && (flash_data_read_buffer[0] != 0) && (flash_data_read_buffer[0] != 0xFF))
						{
							result = do_request_internal(comm_queue.batch_request,flash_data_read_buffer[0], false, false);
							if(result == GPRS_COMMERR_NONE)//MDM_ERR_NONE
							{
								GPRS_comms_attempt_count++;
								//if(Queue_failure_count_l == 0)
									//Original_packet_count++;	//batched packets are not original packets, creating misunderstanding in team
								last_pktsent_time_1  = RTC_epoch_now();
								Batch_Index_Sent = i%Q_Big;
								waiting_for_batch_response = true;
								Debug_Output2( 0, "SENT Batch @ Q %d, Attempt:%d ", i%Q_Big, (Queue_failure_count_l+1));
								Present_value = i;
								Queue_failure_count_l++;
								return;
							}
						}
						else
						{

							if((cardread == 1)||
									(loCurrentRTCTime < (Last_CC_Payment_RTC + 30))||
									((loCurrentRTCTime < (coindrop_TimeOut_RTC + 30))&&(coindrop_TimeOut_RTC < (loCurrentRTCTime + 30)))||
										((loCurrentRTCTime < (KeyPress_TimeOut_RTC + 30)) && (KeyPress_TimeOut_RTC < loCurrentRTCTime + 30)))	//this is avoid please wait msg during CC payment //vinay)
							{
								//Debug_Output1(0,"Events Back2 cardread :%ld", cardread);
								Debug_TextOut(0,"Processing Please Wait.....");
							}
							else
							{
								//Debug_Output1(0,"Events Front2 cardread :%ld", cardread);
								Debug_TextOut(0,"Processing Please Wait.....");
								Clear_Screen();
								printSharp32x48(" PROCESSING ",25,0,0);//printSharp32x48(" THIS METER ",35,0,0);
								printSharp32x48("   PLEASE    ", 80,0,0);//printSharp32x48("   COVERS   ", 80,0,0);
								printSharp32x48("    WAIT.....     ", 135,0,0);//printSharp32x48(" TWO SPACES  ", 125,0,0);
								processwait = 1;
							}

							flag_check_addr[i%Q_Big] = 0;
							Flash_Batch_Flag_Write();
							memset(comm_queue.batch_request,0,sizeof(comm_queue.batch_request));
							olt_WSAD=(uint32_t)(batch_start_address)+(uint32_t)(QUEUE_PACKET_SIZE*(i%Q_Big));
							Flash_Write( comm_queue.batch_request,sizeof(comm_queue.batch_request),olt_WSAD, false);
							Debug_Output1( 0, "DELETED INVALID Batch @ Q %d ", i%Q_Big);
							//Debug_Output1( 0, "Batch_Size=%d ", flash_data_read_buffer[0]);

							if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE)	//Revert back the last blank line
							{
								diag_text_Screen(" ", true, true);
							}
							else	//Revert back the RTC line
							{
								diag_text_Screen(" ", true, true);
								Update_RTC_on_Display();
							}
							if(processwait == 1)
							{
								DSM_Screen1();//Idle_Screen();  //this is causing long hold of processing please wait msg. //so uncmmted //vinay
								processwait = 0;
							}
						}
					}
				}
			}
			if((((coindrop_TimeOut_RTC > 0) && (loCurrentRTCTime > (coindrop_TimeOut_RTC + 30))) || (loCurrentRTCTime < coindrop_TimeOut_RTC))&&
					(((KeyPress_TimeOut_RTC > 0) && (loCurrentRTCTime > (KeyPress_TimeOut_RTC + 30))) || (loCurrentRTCTime < KeyPress_TimeOut_RTC)))
			{
				//Debug_Output1( 0, "coindrop_TimeOut_RTC: %d\r\n",coindrop_TimeOut_RTC);
				//Debug_Output1( 0, "KeyPress_TimeOut_RTC: %d\r\n",KeyPress_TimeOut_RTC);
				//Debug_Output1( 0, "loCurrentRTCTime: %d\r\n",loCurrentRTCTime);
				//Debug_TextOut( 0, "Why here" );
				coindrop_TimeOut_RTC = 0;
				KeyPress_TimeOut_RTC = 0;
				if(cardread == 0)
				DSM_Screen1();
			}
		}
	}
	else
		Process_Flash_Queue = FALSE;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
