/*
 * LibG2_events.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef COMMUNICATION_MODULES_LIBG2_EVENTS_H_
#define COMMUNICATION_MODULES_LIBG2_EVENTS_H_

#include "../Main Module/LibG2_main.h"

#define file_activation_failed  		0
#define file_activation_success 		1
#define TECH_SL_NO_LEN 					3

//#define DEFAULT_SENS_DEPTH 				20 //not used //vinay
//#define Max_sense_distance  			20 //not used //vinay
#define SENSOR_DIAG_LENGTH				70
#define OLT_DELTE_TIMESTAMP 			604800L//(7*24*60*60)L	//PCI PA-DSS requirement

enum EvtCache_Error_Codes
{
    EVT_CACHE_ERR_NONE = 0,
//    EVT_CACHE_ERR_FULL, //not used //vinay
//    EVT_CACHE_ERR_TIMEOUT, //not used //vinay
    EVT_CACHE_ERR_OVRWRT=3,
//    EVT_CACHE_ERR_INTERNAL, //not used //vinay
//    EVT_CACHE_ERR_EMPTY, //not used //vinay
//    EVT_CACHE_ERR_BUSY, //not used //vinay
    EVT_CACHE_ERR_SYNC_TIMEOUT=7,
    EVT_CACHE_ERR_CACHE_DISABLED
};

typedef struct _mntc_Log
{
    uint8_t Prob_Code;
    uint8_t Sol_Code;
   // uint8_t  TECH_CARD_SL_NO[TECH_SL_NO_LEN];
    uint32_t  TECH_CARD_SL_NO;//27-06-12
}Maintenance_Evt;

uint16_t batch_events_in_flash(uint8_t * queued_req_p, uint16_t queued_req_sz);
uint8_t init_event_cache(uint8_t reset_event_id);
void initialise_queue();
void Queue_Request(uint8_t * queued_req_p, uint16_t queued_req_sz);
void process_queue();
uint8_t push_event_to_cache( uint16_t id );
void check_valid_batched_OLTs();
uint16_t batch_events_in_flash(uint8_t * queued_req_p, uint16_t queued_req_sz);
void New_process_queue();

#endif /* COMMUNICATION_MODULES_LIBG2_EVENTS_H_ */
