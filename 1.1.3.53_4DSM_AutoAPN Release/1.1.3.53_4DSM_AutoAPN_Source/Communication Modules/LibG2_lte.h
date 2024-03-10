/*
 * LibG2_lte.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef COMMUNICATION_MODULES_LIBG2_LTE_H_
#define COMMUNICATION_MODULES_LIBG2_LTE_H_

#include "../Main Module/LibG2_main.h"

#define 	ENABLE_SMS_MODE
//#define 	DEFAULT_SERVER_IP  "64.132.70.171"
#define 	DEFAULT_SERVER_IP  "172.25.41.43"
#define 	SMS_LIMIT               50

#define 	MAX_REGN_CHECK_COUNT				120
#define 	TELIT_DRV_SOC_OPN_CMD_RETRIES       2
//#define 	TELIT_DRV_SOC_CLS_CMD_RETRIES       2 //not used //vinay
#define 	TELIT_DRV_SOC_SND_n_RCV_RETRIES     1
#define 	MAX_SOCKOPN_FAIL_COUNT_4_MDM_INIT   2
#define     GPRS_GEN_BUF_SZ_TINY    			32 //not used defined again in com.c//vinay //if commented in both place it giving error
#define     GPRS_IPADD_BUF_SZ        			20 //not used defined again in com.c//vinay //if commented in both place it giving error
#define     GPRS_GEN_BUF_SZ_VSMALL    			64
#define     GPRS_GEN_BUF_SZ_SMALL    			128
#define     GPRS_GEN_BUF_SZ_MED        			256
//#define     GPRS_GEN_BUF_SZ_LARGE    			512  //it is defined in again in com.c //vinay
#define     GPRS_GEN_BUF_SZ_VLARGE   			1300
//#define     GPRS_GEN_BUF_SZ_GIANT    			(GPRS_GEN_BUF_SZ_VLARGE + GPRS_GEN_BUF_SZ_LARGE )	//not used //vinay
#define     DEFAULT_TIME_OUT         			5
#define     DEFAULT_RETRIES          			2
//#define     MODEM_BAUD_RATE            			0 //not used //vinay
//#define     MODEM_UNSOL_MSG_MAX_LEN  			512 //not used //vinay
//#define     RTC_LENGTH				 			32//not used //vinay
#define 	MODEM_BUFFER_S_SIZE  				30
#define		MAX_OPERATORS_ALLOWED				5

#define		PRIMARY_OPERATOR					1
#define		SECONDARY_OPERATOR					2

typedef enum
{
    MDM_ERR_NONE = 0,
    MDM_ERR_CMD_FAILED,
    MDM_ERR_SEND_DATA_FAILED,
//    MDM_ERR_OPEN_IP_FAILED, //not used //vinay
//    MDM_ERR_MISC_FAILURE, //not used //vinay
//    MDM_ERR_UNKNOWN_BAUD, //not used //vinay
    MDM_ERR_INIT_FAILED=6,
//    MDM_ERR_UART_SEND_FAILED, //not used //vinay
//    MDM_ERR_UART_RECV_FAILED //not used //vinay
} MdmErrCodes;

//typedef enum  //not used //vinay
//{
//	MPB_LIBERTY_1_0G,
//	MPB_LIBERTY_1_0C,
//	MPB_LIBERTY_1_5
//}hardwareVersion_t;

typedef struct _s_modem
{
    uint16_t    cmd_sent;
    uint16_t    signal_strength;
    uint8_t     sw_rev[MODEM_BUFFER_S_SIZE];
    uint8_t     IMEI[MODEM_BUFFER_S_SIZE+10];
    uint8_t     IMSI[MODEM_BUFFER_S_SIZE+10];
    uint8_t     id[MODEM_BUFFER_S_SIZE];
    uint8_t     type[MODEM_BUFFER_S_SIZE];
    uint8_t     local_ip_add[MODEM_BUFFER_S_SIZE];
    uint16_t    local_ip_length;        //2103:PBC
    uint16_t    local_PORT;             //2103:PBC
    uint8_t     ICCID[MODEM_BUFFER_S_SIZE+20];
    uint8_t     COPS[MODEM_BUFFER_S_SIZE];
} Modem;

typedef struct _s_operator
{
    uint8_t     Oper_Name[MODEM_BUFFER_S_SIZE+12];
    uint8_t     Oper_ID[MODEM_BUFFER_S_SIZE+12];
} Operator;

uint8_t mdm_init();
void mdm_init_once_per_bootup();//03-06-2013
uint8_t mdm_send_AT_command( const char * cmd_text_ptr,
                             uint8_t *    resp_buf_ptr,
                             uint16_t     resp_buf_sz,
                             uint8_t      max_retries,
                             uint16_t     timeout_ms );

uint8_t mdm_find_response( void * response_buf, const char * string_to_find );
uint8_t mdm_get_signal_strength();
uint8_t mdm_enable_unsolicited_reg_results();
uint8_t mdm_nw_reg_check();

uint8_t mdm_fetch_IMEI( uint8_t * rx_ans, uint16_t buf_sz );
uint8_t mdm_fetch_IMSI( uint8_t * rx_ans, uint16_t buf_sz  );

void telit_power_off();
void telit_power_on();
void telit_wakeup_sleep(uint8_t mdm_sleep_enable);
uint8_t telit_reinit();
uint8_t telit_init();
//uint8_t telit_self_test(); //not used //vinay
uint8_t telit_set_pdpcontext( uint8_t * apn );
uint8_t telit_sock_open( uint8_t communication_type );
uint8_t telit_sock_send( uint8_t * data_p, uint16_t data_size );
//uint8_t telit_sock_receive( uint8_t *buf, uint16_t buf_len, uint16_t * recvd, uint8_t timeout,
//                            uint8_t (*call_bk)( uint16_t rcv_byts, uint8_t *glMdmUart_recv_buf ),
//                            uint8_t sock_clos_state_at_start ); //not used //vinay
uint8_t telit_sock_close();
void TELIT_POWER_OFF();
void TELIT_POWER_ON();
void TELIT_POWER_ENABLE();
void TELIT_POWER_DISABLE();
void reinit_telit_Modem();
uint8_t mdm_fetch_SW_revision( uint8_t * rx_ans, uint16_t buf_sz );
uint8_t mdm_fetch_type( uint8_t * rx_ans, uint16_t buf_sz );
uint8_t mdm_send_data( uint8_t * data_p, uint16_t data_sz, uint16_t timeout );
uint8_t mdm_get_local_ip_port();
uint8_t telit_sock_open_states();
void Modem_Recovery_Loop();
void Init_SMS_Mode();
uint8_t Send_SMS_request();
void Read_Validate_SMS();
void Extract_SMSCFG_Details();
void Read_Mdm_diag_data();
uint8_t mdm_fetch_ICCID( uint8_t * rx_ans, uint16_t buf_sz  );
uint8_t nw_operator_name_id_check();
void Special_Test_AT_Commands(uint8_t * cmdstr, uint16_t cmdlen);
uint8_t mdm_get_local_ip_port_for_FD_OTA();
uint8_t mdm_fetch_SPN();
#endif /* COMMUNICATION_MODULES_LIBG2_LTE_H_ */
