/*
 * LibG2_communication.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef COMMUNICATION_MODULES_LIBG2_COMMUNICATION_H_
#define COMMUNICATION_MODULES_LIBG2_COMMUNICATION_H_

#include "../Main Module/LibG2_main.h"

#define UDP_MIN_RESP_SIZE                                 10
#define MAX_PACKET_LENGTH								  768 //512//256//128
#define MAX_LORA_PACKET_LENGTH							  250
#define MAX_QUEUE_COUNT									  25//50
#define QUEUE_RETRY_COUNT								  3
#define GPRS_TIMEOUT									  10 	//was 15 seconds to declare timeout
#define LORA_RESP_TIMEOUT								  10	//was 5 seconds to declare timeout

#define Size_Ten										  10
#define MC_PBC_TO_LIBRTY_LENGTH  						  6

//*********Alarm messages from MPB board to server************// 0308

//#define UDP_ALMTYP_Meter_Tampering_detected             802 //not used //vinay
//#define UDP_ALMTYP_NOVRAM_Error                         803 //not used //vinay
//#define UDP_ALMTYP_Coil_Sensor_Error                    804 //not used //vinay
#define UDP_ALMTYP_Wakeup_CoilJam_Error                 805
//#define UDP_ALMTYP_Recognition_Coiljam_error            806 //not used //vinay
#define UDP_ALMTYP_CARD_JAM               				807
#define UDP_ALMTYP_OpticalJam_Error						808
//#define UDP_ALMTYP_SensorFault							809 //not used //vinay
#define MB_ALARM_STATE_RAISED                           0xBA
#define MB_ALARM_STATE_CLEARED                          0x5C

//*********Alarm messages from MPB board to server************// 0308

/* 18Aug11.Veda: new events : START */
//#define    UDP_EVTTYP_METER_RESET                          800 //not used //vinay
#define    UDP_EVTTYP_PARKING_TIME_EXPIRED                 801

#define    UDP_EVTTYP_SPECIAL_FREE_TIME		               817
#define    UDP_EVTTYP_ANTI_FEED_START		               818
#define    UDP_EVTTYP_ANTI_FEED_CLEAR		               819
#define    UDP_EVTTYP_CARD_SWITCH_PRESS                    820
#define    UDP_EVTTYP_CARD_REMOVE_TIMEOUT                  821
#define    UDP_EVTTYP_CARD_READ_FAIL                       822
//#define    UDP_EVTTYP_MPB2MB_TIMEOUT                       823 //not used //vinay
//#define    UDP_EVTTYP_MB2MPB_TIMEOUT                       824 //not used //vinay
//#define    UDP_EVTTYP_KEYPRESS_TIMEOUT                     825 //not used //vinay
//#define    UDP_EVTTYP_COIN_BATCH_OVRWRT                    826 //not used //vinay
//#define    UDP_EVTTYP_OLT_BATCH_OVRWRT                     827 //not used //vinay
//#define    UDP_EVTTYP_OLTACK_BATCH_OVRWRT                  828 //not used //vinay
//#define    UDP_EVTTYP_ALARM_BATCH_OVRWRT                   829 //not used //vinay
//#define    UDP_EVTTYP_BAYSTATUS_BATCH_OVRWRT               830 //not used commented in other code//vinay
#define    UDP_EVTTYP_CARD_READ_SUCCESS                    831

#define    UDP_EVTTYP_CARD_NOT_SUPPORTED                   832
#define    UDP_EVTTYP_EXPIRED_CARD_SWIPE                   833

#define    UDP_EVTTYP_COIN_BATCH_EMPTY                     834        // need not store in cache, on the fly we will get
#define    UDP_EVTTYP_OLT_BATCH_EMPTY                      835        // need not store in cache, on the fly we will get
#define    UDP_EVTTYP_OLTACK_BATCH_EMPTY                   836        // need not store in cache, on the fly we will get
#define    UDP_EVTTYP_ALARM_BATCH_EMPTY                    837        // need not store in cache, on the fly we will get
//#define    UDP_EVTTYP_OLT_BATCHED                          838 //not used commented in other code//vinay
//#define    UDP_EVTTYP_COIN_BATCHED                         839 //not used commented in other code//vinay
//#define    UDP_EVTTYP_OLTACK_BATCHED                       840 //not used commented in other code//vinay
#define    UDP_EVTTYP_COIN_CALIBRATION_VALUES 			   841// Used to send missing coin calibration values
#define	   UDP_EVTTYP_SC_CARD_READ_SUCCESS				   842// Used to send SC card read success
//#define    UDP_EVTTYP_ALARM_BATCHED                        843 //not used commented in other code//vinay
//#define    UDP_EVTTYP_BAYSTATUS_BATCHED                    844 //not used commented in other code//vinay
#define    UDP_EVTTYP_BAYSTATUS_BATCH_EMPTY                845
//#define    UDP_EVTTYP_SCT_BATCHED						   846 //not used commented in other code//vinay
#define    UDP_EVTTYP_SCT_BATCH_EMPTY                      847
#define    UDP_EVTTYP_LOW_BATT_LEVEL1                      848
#define    UDP_EVTTYP_LOW_BATT_LEVEL2                      849
#define    UDP_EVTTYP_MPB_RESET                            850
#define    UDP_EVTTYP_EVENTS_BATCHED					   851//??
#define    UDP_EVTTYP_EVENTS_BATCH_EMPTY                   852//??

#define UDP_EVTTYP_BINARY_FILE_COPIED_TO_DATAKEY           854
#define UDP_EVTTYP_ALARMS_CLEARED                          855
//#define UDP_EVTTYP_COINCARDTEST_MENU ACCESS                856 //not used //vinay
#define UDP_EVTTYP_CLOCK_MANUALLY_ADJUSTED                 857

#define UDP_EVTTYP_CONFIG_FILE_COPIED_FROM_DATAKEY         860
#define UDP_EVTTYP_BINARY_FILE_COPIED_FROM_DATAKEY         861

#define UDP_EVTTYP_CONFIG_FILE_COPIED_TO_DATAKEY           863
#define UDP_EVTTYP_SMART_CARD_LOW_BALANCE				   864
#define UDP_EVTTYP_TECH_MENU_ACCESS                        865    //0404
#define UDP_EVTTYP_COLLECTION_EVT                          866    //0404
#define UDP_EVTTYP_MAX_TIME_CREDITED                       867    //0404
#define UDP_EVTTYP_CLEAR_SPACE                             868    //0404
#define UDP_EVTTYP_LOW_BATT_POWER_SAVE                     869    //0404
#define UDP_EVTTYP_MAINTENANCE_LOG                         870    //0404
#define UDP_EVTTYP_VEHICLE_IN		                       871
#define UDP_EVTTYP_VEHICLE_OUT							   872
#define UDP_EVTTYP_SC_CARD_READ_FAIL					   873// Used to send SC card read fail
#define UDP_EVTTYP_ZEROOUT_ON_VACANT					   874
#define UDP_EVTTYP_ZEROOUT_ON_OCCUPIED					   875
#define UDP_EVTTYP_DLST_FORWARD							   876
#define UDP_EVTTYP_DLST_BACKWARD	                       877
//#define UDP_EVTTYP_PAMBAYSTATUS_BATCHED				   	   878 //not used commented in other code//vinay
#define UDP_EVTTYP_PAMBAYSTATUS_BATCH_EMPTY                879
//#define UDP_EVTTYP_MSP_TO_MC_FAIL						   880//?? //not used //vinay
//#define UDP_EVTTYP_MC_TO_MSP_FAIL						   881//?? //not used //vinay
//#define UDP_EVTTYP_MC_COMM_DOWN							   882//??
//#define UDP_EVTTYP_MC_RESET 							   883//?? //not used //vinay
//#define UDP_EVTTYP_MC_LOW_BATT1 						   884//?? //not used //vinay
//#define UDP_EVTTYP_MC_BATT_CRITICAL 					   885//?? //not used //vinay
//#define UDP_EVTTYP_MC_FD_SUCCESS 					       886//?? //not used //vinay
//#define UDP_EVTTYP_MC_FD_FAILURE 					       887//?? //not used //vinay
#define UDP_EVTTYP_DIAG_EXCEPTION						   888 //06-04-2015
#define UDP_EVTTYP_FREETIME_ON_ENTRY					   889
//#define UDP_EVTTYP_BS_Q_FULL							   890 //not used //vinay
//#define UDP_EVTTYP_ZERO_TIME_PAID                          891 //not used //vinay
#define UDP_EVTTYP_SMARTCARD_REFUND                        892
#define UDP_EVTTYP_ENTERING_FREE_PARKING				   893
#define UDP_EVTTYP_ENTERING_PREPAY_PARKING				   894
#define UDP_EVTTYP_ENTERING_PAID_PARKING				   895
#define UDP_EVTTYP_ENTERING_NO_PARKING				   	   896
#define UDP_EVTTYP_PAY_BY_CELL_ACK				   	   	   899
//12-03-2014:hardware sensor code merge

#define UDP_EVTTYP_DIAG_INFO                               900

#define UDP_EVTTYP_COIN_M1_ERROR_OCCURED				   920
#define UDP_EVTTYP_COIN_M2_ERROR_OCCURED				   921
#define UDP_EVTTYP_COIN_MIN_PROFILE_SECOND_CHECK		   922
#define UDP_EVTTYP_COIN_BASE_VALUE_FROM_LAST			   923

#define UDP_EVTTYP_ALARM		                       	   92

#define FILE_ACTIVATION_FAILURE							   160
#define FILE_ACTIVATION_SUCCESS							   161
#define UDP_EVTTYP_SCT_PAYMENT                      	   163

/* 18Aug11.Veda: new events : END */

/* 18Aug11.Veda: Diagnostics info types : START */
//#define UDP_DIAGEVT_MB_CODE_REV                        1	//not used //vinay
#define UDP_DIAGEVT_MPB_VER                            2
#define UDP_DIAGEVT_MDM_TYPE                           3
#define UDP_DIAGEVT_MDM_FW_VER                         4
#define UDP_DIAGEVT_MDM_IMEI                           5
#define UDP_DIAGEVT_MDM_IMSI                           6
#define UDP_DIAGEVT_TEMP_MIN                           7
#define UDP_DIAGEVT_TEMP_MAX                           8
//#define UDP_DIAGEVT_SOLAR1_VOLT                        9	//not used //vinay
//#define UDP_DIAGEVT_SOLAR2_VOLT                        10	//not used //vinay
//#define UDP_DIAGEVT_DCELL_VOLT                         11	//not used //vinay
#define UDP_DIAGEVT_RCHRGBL_BATT_CURR_VOLT             12
#define UDP_DIAGEVT_SIM_APN                            13
#define UDP_DIAGEVT_MPB_CFG_VER                        14
#define UDP_DIAGEVT_SIG_STRENGTH                       15
#define UDP_DIAGEVT_COIN_COUNT                         16
//#define UDP_DIAGEVT_COIN_AMOUNT                        17	//not used //vinay
//#define UDP_DIAGEVT_CARD_COUNT                         18	//not used //vinay
//#define UDP_DIAGEVT_CARD_AMOUNT                        19	//not used //vinay
#define UDP_DIAGEVT_UDP_IP	                           20
#define UDP_DIAGEVT_UDP_PORT	                       21
//#define UDP_DIAGEVT_SMART_COUNT                        22	//not used //vinay
//#define UDP_DIAGEVT_SMART_AMOUNT                       23	//not used //vinay
//21-03-2013: coulomb counter driver merge
#define UDP_DIAGEVT_CARD_READER_TYPE	               24
#define UDP_DIAGEVT_EVENTS_COUNT	                   25
#define UDP_DIAGEVT_COL_CNTR_CURRENT_VALUE             26
#define UDP_DIAGEVT_COL_CNTR_ACC_CURRENT_VALUE         27
//21-03-2013: coulomb counter driver merge

//02-04-2014:DPLIBB-629
//#define UDP_DIAGEVT_MBCCF_FILE_NAME					   28//??
//#define UDP_DIAGEVT_MBRPG_FILE_NAME					   29//??
//#define UDP_DIAGEVT_MBPGM_FILE_NAME					   30//??
//#define UDP_DIAGEVT_SENSOR_FILE_NAME				   31//??

#define UDP_DIAGEVT_FAILED_COINS_DIAG				   37
#define UDP_DIAGEVT_LORA_FREQUENIES					   38
#define UDP_DIAGEVT_COMMS_QUALITY					   39
#define UDP_DIAGEVT_SIM_ICCID                          40
#define UDP_DIAGEVT_GPRS_MDM_ON_DURATION			   41
#define UDP_DIAGEVT_LAST_12_LORA_LQI			   	   42

//21-02-2022 for DEFAULT APN ENTRY
#define UDP_DEFAULT_APN                                43
#define UDP_DEFAULT_UDP_IP                             44
#define UDP_DEFAULT_UDP_PORT                           45
//02-04-2014:DPLIBB-629

#define MAX_EVENT_CACHE_SIZE            7//25   //17-03-2014:hardware sensor code merge
#define EVT_CACHE_SYNC_TIMEOUT          20

//12-04-12
//Card types
#define Payment_Card                                   0
//#define Time_Wipe_Card                                 5	//not used //vinay
#define Technician_Card                                6
//#define Collections_Card                               7	//not used //vinay
#define Commission_Card								   8	//06-08-2013:DPLIBB-551
//12-04-12
#define BSU_EXTRACT_INDEX								14
#define BSU_ZO_INDEX									28
#define BSU_TS_INDEX									15
#define BSU_SENSOR_PARAMS_INDEX							27
#define SENSOR_DIAG_DATA_LENGTH 						68
#define PBC_EXPIRY_TIME_LENGTH  						4
#define PBC_AMOUNT_LENGTH  								2
/* 18Aug11.Veda: Diagnostics info types : END */

//Diagnostics_data_types    //1908

#define DIAG_DATATYP_STRING                             1
//#define DIAG_DATATYP_FLOAT                              2	//not used //vinay
//#define DIAG_DATATYP_INTEGER                            3	//not used //vinay

//Diagnostics_data_types    //1908

#define Q_Big                                             65//32    //1810   //actually it was 64 changed to 65.pp
//#define Q_Small                                         1     //1810

#define GPRS_GEN_BUF_SZ_LargeQ                            128        // chaged from 112 as we get card data > 112 and need to add 8 bytes for hdr, other data etc.
//#define GPRS_GEN_BUF_SZ_SmallQ                            12 // not used here //vinay
//#define GPRS_GEN_BUF_SZ_MEDQ                              25        //0709: -ve time for reconcile batch // not used here //vinay

//#define FD_CHUNK_SIZE									  900	//not used //vinay

#define NUM_QUEUE_BATCH_FLAGS   65
#define QUEUE_COUNT_INDEX		64
#define QUEUE_PACKET_SIZE       128

#define     GPRS_GEN_BUF_SZ_TINY    32 //it is defined in lte.c //vinay
#define     GPRS_IPADD_BUF_SZ        20  //GPRS_GEN_BUF_SZ_TINY//3012 //itis not used here but defined in lte and used in config //vinay
//#define     GPRS_GEN_BUF_SZ_VSMALL    64 //itis not used here but defined in lte and used in config //vinay
#define     GPRS_GEN_BUF_SZ_SMALL    128
//#define     GPRS_GEN_BUF_SZ_MED        256  //it is not used here but defined in lte and filedownload and used in config //vinay
//#define     GPRS_GEN_BUF_SZ_LARGE    512 //it is defined in again //vinay
//#define     GPRS_GEN_BUF_SZ_VLARGE   1300 //it is defined in again //vinay
//#define     GPRS_GEN_BUF_SZ_GIANT    (GPRS_GEN_BUF_SZ_VLARGE + GPRS_GEN_BUF_SZ_LARGE )	//not used //vinay
//#define     DEFAULT_TIME_OUT         5//not used, it is again defined in lte.c and used there so commenting it here //vinay
//#define     DEFAULT_RETRIES          2 //not used, it is again defined in lte.c and used there so commenting it here //vinay
//#define     MODEM_BAUD_RATE            0//not used //vinay
//#define     MODEM_UNSOL_MSG_MAX_LEN  512 //not used //vinay
//#define     RTC_LENGTH				 32//not used //vinay

//1306 RESPONSE SIZES: TO BE CHANGED LATER
//#define        RTC_RESPONSE_SIZE        42 //not used //vinay
//#define        OLT_RESPONSE_SIZE        31 //not used //vinay
//#define        HB_RESPONSE_SIZE        10 //not used //vinay
//#define        ALARM_RESPONSE_SIZE        10 //not used //vinay
//#define        RECONCILE_RESPONSE_SIZE    10 //not used //vinay
//#define        BAYSTATUS_RESPONSE_SIZE    10//not used //vinay
//#define        CT_RESPONSE_SIZE        10 //not used //vinay
//#define        RETRIES                    2 //not used //vinay
//#define        OLT_RETRY                1//not used //vinay
//1306 RESPONSE SIZES: TO BE CHANGED LATER

#define GPRS_GET_UINT8(buf_p)   (buf_p[0])
#define GPRS_GET_UINT32(buf_p)  ((((uint32_t)buf_p[3]) << 24) + (((uint32_t)buf_p[2]) << 16) + (((uint32_t)buf_p[1]) << 8) + buf_p[0])
//#define GPRS_GET_UINT64(buf_p)  ((((uint64_t)buf_p[7]) << 64) + (((uint64_t)buf_p[6]) << 56) + (((uint64_t)buf_p[5]) << 48) + (((uint64_t)buf_p[4]) << 32) + (((uint64_t)buf_p[3]) << 24) + (((uint64_t)buf_p[2]) << 16) + (((uint64_t)buf_p[1]) << 8) + buf_p[0]) //not used //vinay
#define GPRS_GET_UINT16(buf_p)  ((((uint16_t)buf_p[1]) << 8) + buf_p[0])

#ifndef max
    #define    min(a,b)        ((a) < (b) ? (a) : (b) )
    #define    max(a,b)        ((a) > (b) ? (b) : (a) )
#endif

#define DEFAULT_BAY_NUMBER						1

//19-08-2013:transaction types
#define Tx_Unknown								0
#define Tx_Credit_Card							1
#define Tx_Smart_Card							2
#define Tx_Cash									3
//#define Tx_Technician_Credit					4//not used //vinay
#define Tx_Pay_By_Phone							5
//#define Tx_Cash_Key								6//not used //vinay
//#define Tx_Refund_Reset							7//not used //vinay
#define Tx_Sensor_Reset							8
#define Tx_Grace_Period 						10
//#define Tx_Refund_by_Ripnet						57//not used //vinay

//19-08-2013:transaction types

#define MAX_PAM_BSU_QUEUE                   	5

//#define ONE_COIN_DATA_LENGTH					12
#define	MAX_COINS_TO_UDP						40 //20

//#define     GPRS_GEN_BUF_SZ_TINY    			32 //it is defined in lte.c //vinay
//#define     GPRS_IPADD_BUF_SZ        			20  //GPRS_GEN_BUF_SZ_TINY//3012//itis not used here but defined in lte and used in config //vinay
//#define     GPRS_GEN_BUF_SZ_VSMALL    			64 //it is not used here but defined in lte and used in config //vinay
//#define     GPRS_GEN_BUF_SZ_SMALL    			128//it is defined again //vinay
//#define     GPRS_GEN_BUF_SZ_MED        			256  //it is not used here but defined in lte and used in config //vinay
#define     GPRS_GEN_BUF_SZ_LARGE    			512
#define     GPRS_GEN_BUF_SZ_VLARGE   			1300
//#define     GPRS_GEN_BUF_SZ_GIANT    			(GPRS_GEN_BUF_SZ_VLARGE + GPRS_GEN_BUF_SZ_LARGE )	//not used //vinay
//#define     DEFAULT_TIME_OUT         			5 //not used, it is again defined in lte.c and used there so commenting it here //vinay
//#define     DEFAULT_RETRIES          			2	//not used, it is again defined in lte.c and used there so commenting it here //vinay
//#define     MODEM_BAUD_RATE            			0 //not used //vinay
//#define     MODEM_UNSOL_MSG_MAX_LEN  			512//not used //vinay
#define     RTC_LENGTH				 			32

//1306 RESPONSE SIZES: TO BE CHANGED LATER
//#define        RTC_RESPONSE_SIZE        42 //not used //vinay
//#define        OLT_RESPONSE_SIZE        31 //not used //vinay
//#define        HB_RESPONSE_SIZE        10 //not used //vinay
//#define        ALARM_RESPONSE_SIZE        10 //not used //vinay
//#define        RECONCILE_RESPONSE_SIZE    10 //not used //vinay
//#define        BAYSTATUS_RESPONSE_SIZE    10 //not used //vinay
//#define        CT_RESPONSE_SIZE        10 //not used //vinay
#define        RETRIES                    2
//#define        OLT_RETRY                1//not used //vinay


#define 		Max_Sensor				20
/*typedef struct lng_coin_only_audit
{
    uint32_t    lSerNum;
    uint32_t    lLocCode;
    uint16_t    CKCode;
    uint16_t    CityCode;
    uint16_t    ClkTrimr;
    uint16_t    CurErr;
    uint16_t    LogErr;
    uint32_t    lValScale;
    uint32_t    lNoAud;
    uint32_t    AudAmt;           // Currency//16-07-12:changed double to long
    uint32_t    CashlessAudit;    // Currency//16-07-12:changed double to long
    uint32_t    Batt;             // Currency//16-07-12:changed double to long
    uint16_t    Coins[20];
    uint16_t    RejCoin;
    uint16_t    CK;
    uint16_t    ExtDev;
    uint32_t    EDAudit;
} AuditInfo;
*/

/*typedef enum
{
    SOC_TYP_TCP = 0,
    SOC_TYP_UDP = 1,
    SOC_TYP_UDP_LISTEN
} SocketType;*/

//typedef enum //not used //vinay
//{
//    SOC_EVT_OPENED,
//    SOC_EVT_CLOSED,
//    SOC_SENT_DATA,
//    SOC_EVT_RECVD,
//    SOC_EVT_ERROR,
//    SOC_EVT_TIMEOUT
//} SocketEvent;

//typedef enum //not used //vinay
//{
//    SOC_STATE_IDLE,
//    SOC_STATE_OPENING,
//    SOC_STATE_OPENED,
//    SOC_STATE_CLOSING,
//    SOC_STATE_CLOSED,
//    SOC_STATE_SENDING,
//    SOC_STATE_SENT,
//    SOC_STATE_RECEIVING,
//    SOC_STATE_RECEIVED
//} SocketState;

typedef enum
{
    SOCERR_NONE,
    SOCERR_BUSY,
    //SOCERR_OPEN, //not used //vinay
    //SOCERR_CLOSE, //not used //vinay
    //SOCERR_SEND_NONE, //not used //vinay
    SOCERR_SEND_SOME=5,
    //SOCERR_SEND_ALL,//not used //vinay
    //SOCERR_SEND_TIMEOUT, //not used //vinay
    //SOCERR_RECVD_NONE, //not used //vinay
    //SOCERR_RECVD_SOME, //not used //vinay
    //SOCERR_RECVD_BUF_FULL, //not used //vinay
    SOCERR_RECV_TIMEOUT=11,
    //SOCERR_NOT_BOUND, //not used //vinay
    SOCERR_IP_STK_OPEN=13,
    //SOCERR_IP_STK_CLOSE //not used //vinay
} SocketError;

enum GPRSStatus {
    GPRS_SUCCESS,
//    GPRS_MDM_FUNC_FAILED, //not used //vinay
//    GPRS_COMM_FAILED, //not used //vinay
//    GPRS_FI_XML_CONSTRUCT_FAILURE, //not used //vinay
//    GPRS_EVENT_XML_CONSTRUCT_FAILURE, //not used //vinay
//    GPRS_INVALID_EVENT_BUF, //not used //vinay
//    GPRS_MEM_AVAIL_ERR, //not used //vinay
//    GPRS_NO_EVENT_BUF_CONSUMED, //not used //vinay
//    GPRS_PARTIAL_EVENT_BUF_CONSUMED, //not used //vinay
//    GPRS_ALL_EVENT_BUF_CONSUMED, //not used //vinay
//    GPRS_GET_FILE_INFO_FAILURE, //not used //vinay
//    GPRS_FD_FI_FAILED, //not used //vinay
//    GPRS_FD_AFM_FAILED, //not used //vinay
//    GPRS_FD_DFG_FAILED, //not used //vinay
//    GPRS_FD_DFS_FAILED, //not used //vinay
//    GPRS_MODEM_FAILURE, //not used //vinay
//    GPRS_SOCKET_FAILURE, //not used //vinay
//    GPRS_MISC_FAILURE, //not used //vinay
//    GPRS_FAILURE //not used //vinay
    };

enum GPRSCommError {
    GPRS_COMMERR_NONE,
    GPRS_COMMERR_BUSY,
//    GPRS_COMMERR_MEM_ERR, //not used //vinay
    GPRS_COMMERR_SOCK_OPEN_ERR=3,
    GPRS_COMMERR_SOCK_SEND_ERR,
    GPRS_COMMERR_SOCK_RECV_ERR,
    GPRS_COMMERR_SOCK_CLOSE_ERR,
//    GPRS_COMMERR_TIMEOUT, //not used //vinay
//    GPRS_COMMERR_INVALID_REQ, //not used //vinay
    GPRS_COMMERR_INVALID_RESP_HDR=9,
//    GPRS_COMMERR_INVALID_RESP_BODY,//not used //vinay
 //   GPRS_COMMERR_INVAID_ALLOC_FOR_ARRAY, //not used //vinay
    GPRS_COMMERR_INTERNAL_PROCESS=12,
    GPRS_COMMERR_SERVER_ISSUE,
    GPRS_COMMERR_INTERNAL_PRCS_RESP,
//    GPRS_COMMERR_EVT_CACHE_EMPTY, //not used //vinay
//    GPRS_COMMERR_UNKNOWN //not used //vinay
    };

enum GPRSBufSizes
{
	GPRS_COMM_OLT_TRACK1_SIZE = 86,//11-06-2013:for AES encryption packet should be div by 16
	GPRS_COMM_OLT_TRACK2_SIZE = 54,//11-06-2013:for AES encryption packet should be div by 16
    GPRS_COMM_OLT_ACQ_REF_MAX_SZ = 50,
    GPRS_EVENT_PKG_SIZE    = 512+10  //aps
};

enum GPRSOLTGatewayRespCodes {
    GPRS_OLT_GWAY_RESP_Accepted = 0,
    GPRS_OLT_GWAY_RESP_Declined = 10,
    GPRS_OLT_GWAY_RESP_Invalid_card = 20,
    GPRS_OLT_GWAY_RESP_Bad_expiry_date = 30,
    GPRS_OLT_GWAY_RESP_Insufficient_funds = 40,
    GPRS_OLT_GWAY_RESP_Service_unavailable = 50,
    GPRS_OLT_GWAY_RESP_Waiting = 60,
    GPRS_OLT_GWAY_RESP_ServerError = 70,
    GPRS_OLT_GWAY_RESP_InvalidResp = 80,
    GPRS_OLT_GWAY_RESP_Timeout = 90
};

enum GPRSCommType
{
//        GPRS_COMM_UNKNOWN = 0, //not used //vinay
/*01*/    GPRS_COMM_OLT=1,
///*02*/    GPRS_COMM_GenHeartBeat, //not used //vinay
///*03*/    GPRS_COMM_PAMHearBeat, //not used //vinay
/*04*/    GPRS_COMM_PAMBayStatusUpdate=4,
///*05*/    GPRS_COMM_PAMBayStatusGet, //not used //vinay
///*06*/    GPRS_COMM_PAMClusterStatus, //not used //vinay
///*07*/    GPRS_COMM_PAMMeterStatus, //not used //vinay
///*08*/    GPRS_COMM_RTCTimeSetData, //not used //vinay
///*09*/    GPRS_COMM_AFM, //not used //vinay
///*10*/    GPRS_COMM_DFG, //not used //vinay
/*11*/    GPRS_COMM_DFS=11,
///*12*/    GPRS_COMM_FI, //not used //vinay
///*13*/    GPRS_COMM_EventUpload, //not used //vinay
/*14*/    /* left for PCI and will be used when we integrate PCI changes with this */
//        GPRS_COMM_IPSync = 15, //not used //vinay
        GPRS_COMM_PAMPushMsg=16,
        GPRS_COMM_PAMPushAck,
//        GPRS_COMM_ResetPushMsg = 19, //not used //vinay
//        GPRS_COMM_ResetPushAck = 20, //not used //vinay
//        GPRS_COMM_OLT_XXTEA=21, //not used //vinay
        //GPRS_COMM_MAX_REQs,
        GPRS_COMM_BINARY_EVENTS = 22,
        GPRS_COMM_OLT_AES = 43,
//        GPRS_MECH_SWAP=51, //not used //vinay
//        GPRS_SPARE_MECH=52, //not used //vinay
        GPRS_COMM_BayStatusUpdate = 54,
        GPRS_COMM_CT = 55,
        GPRS_COMM_Evt = 56,
//        GPRS_COMM_RTC_DOW=57, //not used //vinay
        GPRS_BATCHED_OLT=58,
        GPRS_BATCHED_OLT_PCHSD_TIME=69,
        GPRS_SSM_AUDITINFO=60,	//01-06-12: Audit registry type
        GPRS_COMM_Liberty_HeartBeat =62,  //01-06-12: new type for Liberty specific HB //not used //vinay
        GPRS_COMM_BayStatus_diagnostics = 64,	//15-10-2013: sensor diag
        GPRS_COMM_lOB_General_Sync = 120,
        GPRS_COMM_lOB_DFG = 121,
        GPRS_COMM_lOB_DFS = 122,
        GPRS_COMM_LOB_BayStatus_diagnostics=123,
		GPRS_COMM_CT_PROFILE = 125
};

enum GPRSCommReqRespEncrypt
{
    GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT = 0,
    GPRS_REQ_ENCRYPT_RESP_UNENCRYPT,
//    GPRS_REQ_UNENCRYPT_RESP_ENCRYPT, //not used //vinay
//    GPRS_REQ_ENCRYPT_RESP_ENCRYPT, //not used //vinay
    GPRS_REQ_REG_RESP_SPEACT=16
};

enum GPRSCommWays
{
    GPRS_COMM_UDP,
//    GPRS_COMM_TCP//not used //vinay

};

#pragma pack(push)  // push current alignment to stack
#pragma pack(1)     // set alignment to 1 byte boundary
/*typedef struct
{
    uint8_t OLT1[GPRS_GEN_BUF_SZ_LargeQ];
    uint8_t request_size;
    uint8_t OLT1_flag;
} queue_OLT;

typedef struct
{
    uint8_t CT[GPRS_GEN_BUF_SZ_SMALL];
    uint8_t request_size;
    uint8_t CT_flag;
} CT_queue;

typedef struct
{
    uint8_t BayStatus[GPRS_GEN_BUF_SZ_SmallQ];
    uint8_t request_size;
    uint8_t BayStatus_flag;
} BayStatus_queue;

typedef struct
{
    uint8_t OLTAck[GPRS_GEN_BUF_SZ_MEDQ];
    uint8_t request_size;
    uint8_t OLTAck_flag;
} OLTAck_queue;

typedef struct
{
    uint8_t Alarms[GPRS_GEN_BUF_SZ_TINY];
    uint8_t request_size;
    uint8_t Alarms_flag;
} Alarms_queue;

typedef struct
{
    queue_OLT            OLT_Q[Q_Big];
    CT_queue            Coin_Q[Q_Small];
    BayStatus_queue        BayStatus_Q[Q_Small];
    OLTAck_queue        OLTAck_Q[Q_Small];
    Alarms_queue        Alarms_Q[Q_Small];
} QUEUE;*///1810




typedef struct//1810
{
    uint8_t batch_request[GPRS_GEN_BUF_SZ_LargeQ];
    uint8_t OLT_batch_flag[NUM_QUEUE_BATCH_FLAGS];
    uint8_t OLTAck_batch_flag[NUM_QUEUE_BATCH_FLAGS];
    uint8_t BayStatus_batch_flag[NUM_QUEUE_BATCH_FLAGS];
    uint8_t Alarms_batch_flag[NUM_QUEUE_BATCH_FLAGS];
    uint8_t coin_batch_flag[NUM_QUEUE_BATCH_FLAGS];
    uint8_t SCT_batch_flag[NUM_QUEUE_BATCH_FLAGS];//25-06-12:SCT Batch
    uint8_t EVENTS_batch_flag[NUM_QUEUE_BATCH_FLAGS];//17-03-2014:hardware sensor code merge
    uint8_t PAMBayStatus_batch_flag[NUM_QUEUE_BATCH_FLAGS];
    uint8_t OLT_batch_erase,
            OLTAck_batch_erase,
            BayStatus_batch_erase,
            Alarms_batch_erase,
            coin_batch_erase,
            SCT_batch_erase,//25-06-12:SCT Batch
    		EVENTS_batch_erase, //17-03-2014:hardware sensor code merge
    		PAMBayStatus_batch_erase;
} QUEUE;

typedef struct
{
    uint16_t        id;
    uint32_t        ts;
    uint8_t         PROBLEM_CODE;    // 0404: maintenance event
    uint8_t         SOLUTION_CODE;    // 0404: maintenance event
   // uint8_t       tech_card_sl_no[TECH_SL_NO_LEN];    // 0404: maintenance event
    uint32_t		tech_card_sl_no;//27-06-12
    union
    {
        uint8_t        byte;

        struct
        {
        	uint16_t        dirty : 1;
        	uint16_t        error : 1;
        } bits;
    } flags;
} EventCacheEntry;

typedef struct
{
    EventCacheEntry            cache[MAX_EVENT_CACHE_SIZE];
    int8_t                    next_idx;

    union
    {
        uint8_t        byte;
        struct
        {
        	uint16_t        ovrflw : 1;
        	uint16_t        undrflw : 1;
        	uint16_t        error : 1;
        	uint16_t        repush : 1;
        	uint16_t        free : 1;
        } bits;
    } flags;
} EventCache;

typedef struct
{
    uint16_t                        txn_resp_code;
    uint16_t                        ripnet_resp_code;
} GPRS_Server_Response;

typedef struct
{
    uint32_t                        earned_mins;    //2102:txn roll-up
    uint32_t                        time_stamp;
    uint8_t                            multiple_event_counter;
    uint8_t                            card_type;
    uint16_t                        amount;
    uint8_t                            ref_id[8];
    uint32_t                        paid_until;
    uint16_t                        bay_no;
   uint8_t                            track2[GPRS_COMM_OLT_TRACK2_SIZE];
   uint8_t                            track1[GPRS_COMM_OLT_TRACK1_SIZE];
} GPRS_OLTRequest;

typedef struct
{
    GPRS_Server_Response            server_resp;
    uint8_t                         auth_code;
    uint32_t                        card_balance;
    uint8_t                            acq_ref_length;
    uint8_t                            acq_ref[ GPRS_COMM_OLT_ACQ_REF_MAX_SZ];
    //uint8_t                            rcp_num[8];
} GPRS_OLTResponse;

typedef struct
{
    uint16_t                        bay_no;
    uint32_t                        time_stamp;
} GPRS_PamHeartbeatRequest;

typedef struct
{
    GPRS_Server_Response            server_resp;
}  GPRS_PamHeartbeatResponse;

typedef struct
{
    uint16_t coin_index;    //1509
    uint32_t cashtranstime;
    uint16_t coinval;
    uint8_t  cumulative_parktime_hr;
    uint8_t  cumulative_parktime_min;
    uint8_t  earned_coin_hr;        //2102:txn roll-up
    uint8_t  earned_coin_min;    //2102:txn roll-up
    uint8_t  space_id; //Left/Right Space for DSM/MSM
    uint16_t max;
    uint16_t min;
    uint8_t rat1;
    uint8_t rat2;
} COINDATA;
/*
typedef struct
{
    //uint16_t                        bay_no;
    //uint8_t                            txn_type;
    //uint8_t                            cdata_length;
    //uint8_t                            expirytime_hh[100];
    //uint8_t                            expirytime_mm[100];
    //uint16_t                        cash_amt[100];
    COINDATA        cdata[30];
} GPRS_CashtransactionRequest;*/
typedef struct
{
    uint32_t                        earned_mins;    //2102:txn roll-up
    uint32_t						paid_untill_mins;
    uint32_t                        time_stamp;
    uint16_t                        amount;
    uint16_t                        ref_id;
    uint8_t 						card_id[6];
} GPRS_Smartcard_tranRequest;	//25-05-12

typedef struct
{
    GPRS_Server_Response            server_resp;
    uint32_t                        expiry_time;
} GPRS_CashtransactionResponse;

typedef struct
{
    uint8_t                            receipt_num[8];
    //uint32_t                         paid_until_ts;
    uint8_t                            olt_success;
    uint8_t                         paid_until_ts[4];
    uint16_t                        bay_no;
    uint16_t                        earned_time;    //2102:txn roll-up
} GPRS_OLTACKRequest;
/*typedef struct
{
    GPRS_Server_Response            server_resp;
    uint32_t                        expiry_time;
} GPRS_OLTACKResponse;*/

typedef struct
{
	//uint16_t						bay_no;
	uint16_t						occupancy;	//1401: from 1 byte to 2
	int16_t							SNSR_TMP;	//1401
	uint8_t					diag_string[68];	//2202
	uint32_t						time_stamp;//14-03-2014:hardware sensor code merge
    uint32_t                        expiry_time;
    uint32_t                        PAMtxn_time;
    uint16_t                        purchased_duration;
    //uint16_t                        amount_cents;
    uint8_t                         txn_type;
    //uint8_t                         topup;
}GPRS_BayStatusRequest;

typedef struct
{
    GPRS_Server_Response            server_resp;
    uint32_t                        expiry_time;
}GPRS_BayStatusResponse;

typedef struct
{
    uint8_t                     noofalarms;
    uint16_t                    alarm_type;
    uint32_t                    alarm_ts;
    uint16_t                    alarm_size;
    uint8_t                     alarm_state;
    uint16_t                    alarm_id;
}GPRS_AlarmsRequest;

typedef struct
{
    GPRS_Server_Response            server_resp;
    uint32_t                        expiry_time;
}GPRS_AlarmsResponse;



typedef struct
{
    uint16_t                        bay_no;
    uint32_t                        expiry_time;
    uint16_t                        purchased_duration;
    uint8_t                         txn_type;
    uint8_t                         topup;
    uint16_t                        amount_cents;
    uint32_t                        txn_time;
} GPRS_PamBayStatusUpdateRequest;


typedef struct
{
    GPRS_Server_Response            server_resp;
    uint32_t                        expiry_time;
} GPRS_PamBayStatusUpdateResponse;

typedef GPRS_PamBayStatusUpdateResponse GPRS_PamBayStatusGetResponse;

typedef struct
{
    uint16_t                        bay_no;
    uint32_t                        time_stamp;
} GPRS_PamBayStatusGetRequest;

typedef struct
{
    uint16_t                        bay_no;
    uint32_t                        last_UP_timestamp;
} GPRS_PamClusterStatusRequest;

typedef struct
{
    uint16_t                        no;
    uint32_t                        expiry_ts;
} GPRS_BayInfo;

typedef struct
{
    uint16_t                        id;
    uint32_t                        imin;
    uint32_t                        up_timestamp;
    GPRS_BayInfo                    * bay_info_p;
    uint16_t                        bay_buf_sz;
    uint16_t                        total_bays;
} GPRS_BayMeterInfo;

typedef struct
{
    uint16_t                        no;
    uint16_t                        cust_id;
    GPRS_BayMeterInfo                * meter_info_p;
    uint16_t                        meter_buf_sz;
    uint16_t                        total_meters;
} GPRS_ClusterInfo;


typedef struct
{
    GPRS_Server_Response            server_resp;
     GPRS_ClusterInfo                rest;
} GPRS_PamClusterStatusResponse;

typedef struct
{
    uint16_t                        bay_no;
    uint32_t                        last_UP_timestamp;
} GPRS_PamMeterStatusRequest;

typedef struct
{
    GPRS_Server_Response            server_resp;
     GPRS_BayMeterInfo                    rest;
} GPRS_PamMeterStatusResponse;

typedef struct
{
    GPRS_Server_Response            server_resp;
     uint32_t                        time;
} GPRS_RTCResponse;

typedef struct
{
    uint8_t                            events_buf[ GPRS_EVENT_PKG_SIZE ];
    uint16_t                        start_offset;
    uint16_t                        filled_upto_idx;
    uint8_t                            is_first;
} GPRS_EventUpload_Request;

typedef struct
{
    GPRS_Server_Response            server_resp;
    uint16_t                        events_posted;
    uint16_t                        consumed_event_buf_idx;
} GPRS_EventUploadResponse;

typedef struct
{
    GPRS_Server_Response            server_resp;
} GPRS_GeneralHeartbeatResponse;


typedef struct _s_fd_file
{
	//uint8_t                  name[ GPRS_GEN_BUF_SZ_SMALL ];    //1910
   // uint8_t                 chksum[ GPRS_GEN_BUF_SZ_VSMALL ];
	uint32_t      			chksum;
    uint32_t                size;
    uint32_t                actvn_ts;	// seconds
    uint32_t                id;
    uint8_t                 type;        // add the enum list for possible types
} GPRS_FD_File;

typedef struct
{
	uint8_t  				File_Type;
	uint8_t  				VER_LEN;
	uint8_t  				CHK_SUM[20];
	uint8_t  				VER_NUM[20];
	uint32_t 				File_ID;
}File_data;

typedef struct
{
	uint8_t					OLT_AES_Encryption;
	uint8_t					Total_Files;
	File_data				file_data[2];
	uint32_t				MPB_CFG_CRC;
	uint32_t				MPB_BIN_CRC;
} GPRS_General_Sync_Request;

typedef struct
{
	 uint32_t						 content_length;
	 uint8_t						 meter_action;
	 uint8_t						 mech_swap_result;
	 uint8_t						 RTC_String[RTC_LENGTH];
	 uint16_t						 total_FD_chunks;
} GPRS_General_Sync_Response;

typedef struct
{
    uint8_t                            flag;
    uint8_t                            type;
} GPRS_CommonHeader;

enum LORAreqType
{
	ACK = 1,
	DATA,
	CMD
};

enum LORACommType
{
 LORA_NODE_RTC = 1,
// LORA_NODE_DFG, //not used //vinay
// LORA_NODE_WAKEUP, //not used //vinay
// LORA_NODE_EXWAKEUP, //not used //vinay
// LORA_NODE_AFM=62 //not used //vinay
};

typedef struct
{
    uint8_t                            * p;
    uint16_t                        size;
    uint16_t                        used_upto_idx;
} GPRS_GrowingBuffer;

typedef struct
{
    GPRS_CommonHeader                ch;
    uint16_t                        area_number;
    uint16_t                        customer_id;
    uint16_t                        meter_id;
} GPRS_GeneralRequestHeader;

typedef struct
{
    GPRS_CommonHeader                ch;
    GPRS_Server_Response            server_resp;
    uint32_t                        content_length;
} GPRS_GeneralResponseHeader;

typedef struct
{
	uint8_t Packet[MAX_PACKET_LENGTH];
	uint8_t Retrycount;
	uint16_t Request_size;
}GPRS_Queue;

typedef struct
{
	uint8_t Packet[MAX_LORA_PACKET_LENGTH];
	uint8_t Retrycount;
	uint16_t Request_size;
}LORA_Queue;

#pragma pack(pop)   // restore original alignment from stack

uint8_t gprs_fill_request_hdr( uint8_t type, uint8_t flag, uint8_t * buf_p, uint16_t buf_sz, uint8_t MSM_transaction );
uint8_t gprs_fill_response_hdr( uint8_t * buf_p, uint16_t buf_sz, GPRS_GeneralResponseHeader * resp_hdr_p );
uint8_t gprs_do_request( uint8_t *request_p, uint16_t request_sz );
uint8_t do_request_internal( uint8_t *request_p, uint16_t request_sz, uint8_t not_processing_batch, uint8_t this_is_in_fd );

uint8_t gprs_do_OLTACK(GPRS_OLTACKRequest * request_p);

uint8_t gprs_do_OLT( GPRS_OLTRequest * request_p, GPRS_OLTResponse * response_p);

uint8_t gprs_do_CT(uint8_t spaceID);

uint8_t gprs_do_SCT();

uint8_t gprs_do_Alarms(GPRS_AlarmsRequest * request_alm);

//uint8_t gprs_do_pam_heartbeat( GPRS_PamHeartbeatRequest  * request_p,
//        GPRS_PamHeartbeatResponse * response_p );

//uint8_t gprs_do_pam_baystatus_update( GPRS_PamBayStatusUpdateRequest * request_p,
    //    GPRS_PamBayStatusUpdateResponse * response_p );

//uint8_t gprs_do_pam_baystatus_get( GPRS_PamBayStatusGetRequest * request_p,
//        GPRS_PamBayStatusUpdateResponse * response_p );

//uint8_t gprs_do_pam_cluster_status_get( GPRS_PamClusterStatusRequest * request_p,
//        GPRS_PamClusterStatusResponse * response_p );

//uint8_t gprs_do_pam_meterstatus_get( GPRS_PamMeterStatusRequest * request_p,
//        GPRS_PamMeterStatusResponse * response_p );

//uint8_t gprs_do_get_RIPNET_time( GPRS_GeneralHeartbeatResponse * response_p );

//uint8_t gprs_do_event_upload( GPRS_EventUpload_Request  * request_p,
//        GPRS_EventUploadResponse * response_p );
//void gprs_do_BayStatus(GPRS_BayStatusRequest * request_p );
//uint8_t gprs_do_LOB_BayStatus(GPRS_GeneralHeartbeatResponse * response_p );
uint8_t gprs_do_LOB_BayStatus();
//uint8_t gprs_do_general_heartbeat( GPRS_GeneralHeartbeatResponse * response_p );

//uint8_t gprs_Queue_SendReceive(uint8_t * queued_req_p, uint16_t queued_req_sz); //not used //vinay

//uint8_t gprs_do_general_heartbeat( /*GPRS_BayStatusRequest * request_p , */GPRS_GeneralHeartbeatResponse * response_p );

//uint8_t gprs_start_download( GPRS_Server_Response * response_p, int16_t * step_status_p ); // does the seq: FI->AFM->DFG*->DFS : wait call

//uint8_t gprs_get_modem_status( GPRS_ModemStatus * modem_status_p );

uint8_t gprs_do_dfg();    //04-06-12
uint8_t gprs_do_DFS(uint32_t file_id, uint32_t status);    //2609:test
uint8_t gprs_do_get_General_Sync_request();
void    parse_General_Sync_Response(uint8_t * response);
uint8_t Trigger_Asynchronous_FD();
void    Update_DFG_status(uint8_t Type,uint32_t Chunk_FileOffset);
uint8_t gprs_do_DFS_4all_files(uint8_t fd_complete);
//uint8_t init_event_cache();
uint8_t init_event_cache(uint8_t reset_event_id);
uint8_t push_event_to_cache( uint16_t id );
//uint8_t pop_event_from_cache( uint8_t * id, uint32_t * ts ); //not used //vinay
//uint8_t peek_event_cache( uint8_t * id, uint32_t * ts ); //not used //vinay
//uint8_t is_event_cache_full(); //not used //vinay
uint8_t gprs_do_send_event_cache( uint8_t add_diag_too, uint8_t event_buff_count2 );
uint8_t gprs_send_diag_msg();
uint8_t gprs_file_actn_stat( uint32_t file_id, uint8_t type,uint8_t activation_status );
uint8_t Transactions_via_LoRa(uint8_t *request_p, uint16_t request_sz);
void Handle_GPRS_Response();
void process_LORA_Queue();
void process_RAM_Queue();
int16_t PAM_Baystatus_update(GPRS_PamBayStatusUpdateRequest *PAM_baystatus_update);
uint8_t Process_Incoming_Sensor_Response(uint16_t lora_resp_length);
void Send_ACK2Node(uint8_t *header);
void Handle_LORA_Response();
int16_t gprs_push_notify();
uint8_t Transactions_via_LoRa(uint8_t *request_p, uint16_t request_sz);
void process_LORA_Queue();
void PBC_TO_OTHER_METER();
void get_RTC();
void Send_Audit_Via_Temp_GW(void);
void Clear_all_active_alarms(void);
uint8_t gprs_do_DFS_4all_files_OK(uint8_t fd_complete); //2609:test

#endif /* COMMUNICATION_MODULES_LIBG2_COMMUNICATION_H_ */
