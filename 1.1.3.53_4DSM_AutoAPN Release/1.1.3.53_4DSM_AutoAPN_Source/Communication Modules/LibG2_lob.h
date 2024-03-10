/*
 * LibG2_lob.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef COMMUNICATION_MODULES_LIBG2_LOB_H_
#define COMMUNICATION_MODULES_LIBG2_LOB_H_

#include "../Main Module/LibG2_main.h"

#define LOB_MAX_RECOVERY_TRY				4
#define MAX_DFS_RETRY						40 //15
//#define MAX_GENSYNC_RETRY					3 //not used //vinay
#define MAX_FD_CHUNKS						260		//254 max for binary

// US FT private apn
//#define US_FT_PRIVATE_SERVER_IP      	"172.25.41.43" //LOB PEM direct IP //not used //vinay
//#define US_FT_PRIVATE_SERVER_PORT    	7878 //not used //vinay

// yellow private AT&T APN SIMs
//#define ATT_PRIVATE_SIM_CARD_APN        "METERSPRO02.DUNCAN-USA.COM" //not used //vinay
//#define TMOBILE_PRIVATE_SIM_CARD_APN 	"m2m.t-mobile.com" //not used //vinay

typedef enum
{
	General_Sync_Error_None = 0,
//	General_Sync_First_Attempt, //not used //vinay
//	General_Sync_Ntework_Reg_fail, //not used //vinay
	General_Sync_Recv_Error=3,
//	General_Sync_DFG_Error, //not used //vinay
//	General_Sync_Default_Server1, //not used //vinay
//	General_Sync_Default_Server2, //not used //vinay
//	General_Sync_Default_Server3, //not used //vinay
//	General_Sync_Default_Server4 //not used //vinay

}General_Sync_error;


//void LOB_Meter_details(); //not used //vinay
//void LOB_try_alt_server(); //not used //vinay
void Fill_General_Sync_request_structure();
void Handle_RTC_4mServer();
void Handle_Mechswap_Response();
//void check_health_of_sensor(uint8_t bootup_seq);//14-03-2014:hardware sensor code merge
void Get_Meter_Status(uint8_t FD_timeout);
void Display_Meter_status();



#endif /* COMMUNICATION_MODULES_LIBG2_LOB_H_ */
