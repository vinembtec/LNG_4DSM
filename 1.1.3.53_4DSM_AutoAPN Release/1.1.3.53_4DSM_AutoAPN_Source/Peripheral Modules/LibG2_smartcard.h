/*
 * LibG2_smartcard.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef PERIPHERAL_MODULES_LIBG2_SMARTCARD_H_
#define PERIPHERAL_MODULES_LIBG2_SMARTCARD_H_

#include "../Main Module/LibG2_main.h"

#define MAX_SCT_RX_BYTES			64 //50//40//30//6
#define SC_RW_TIMING				53
#define SC_BUFFER_LENGTH			5

//#define ATRSTRING_LENGTH 		4  //not used //vinay
#define SMARTCARD_CMD_LENGTH 	4
#define RATIFICATION_COUNTER_MASK 0xE0000000

#define CARD_RESET 				1
#define CARD_POWER 				2
#define CARD_AUTHENTIC 			4
#define CARD_UNLOCKED 			8
#define CARD_BALANCE 			16
//#define CARD_RESET_RETRY 		32  //not used //vinay

#define SMARTCARD_SUCCESS 		1
#define SMARTCARD_IO_ERROR 		2
#define SMARTCARD_IN_BLACKLIST 	3
#define SMARTCARD_INVALID 		4
#define SMARTCARD_ZERO_BALANCE 	5

#define ACCESS_CARD_ID_SIZE		5
#define GEMCLUB_BUFFER_SIZE 	16

// array index of mapped address when stored in CardMap
#define DUNCAN_INDEX_MANUF   	0
#define DUNCAN_INDEX_NUMBER   	1
#define DUNCAN_INDEX_REGION   	2
#define DUNCAN_INDEX_MODE     	3
//#define DUNCAN_INDEX_ACCESS   	4  //not used //vinay
#define DUNCAN_INDEX_CTC1     	5
#define DUNCAN_INDEX_BAL1A1   	6
#define DUNCAN_INDEX_BAL1A2   	7
#define DUNCAN_INDEX_A1MAXCRED  8
#define DUNCAN_INDEX_CSC1     	9
#define DUNCAN_INDEX_TMP1     	10
#define DUNCAN_INDEX_TMP2     	11
#define DUNCAN_INDEX_TMP3     	12

#define DUNCAN_MAP_MANUF      	0x0
#define DUNCAN_MAP_NUMBER     	0x1
#define DUNCAN_MAP_REGION     	0x2
#define DUNCAN_MAP_MODE      	0x4
//#define DUNCAN_MAP_ACCESS     	0x5  //not used //vinay
#define DUNCAN_MAP_CTC1       	0x8
#define DUNCAN_MAP_BAL1A1     	0xC
#define DUNCAN_MAP_BAL1A2     	0xE
#define DUNCAN_MAP_A1MAXCRED  	0x10
#define DUNCAN_MAP_CSC1       	0x39

#define GEM_ID_LEN    			4
#define CARD_ID_SIZE    		6
#define SYNCARD_CARD_ID_LENGTH  CARD_ID_SIZE
#define MIN_REFUND_AMT_MULTIPLIER	5

typedef struct
{
	uint32_t Manufacturer : 32; // address 0
	uint32_t CardSerial   : 32; // address 1
	uint32_t RegionCode   : 32; // address 2
	uint32_t Mode         : 1;  // address 4
	uint32_t Random1      : 31;
} DuncanMap;

typedef union
{
	int8_t CCPrefix[8];
	uint32_t SCNumber;
} HotlistNumber;

enum en_smart_card_state
{
	GEMCLUBM_CARD_NO_FOUND = 0,
	GEMCLUBM_CARD_SC_INVALID,
	GEMCLUBM_CARD_BLACKLIST,
	GEMCLUBM_CARD_DEDUCT_SUCCESS,
	GEMCLUBM_CARD_DEDUCT_FAILED,
	GEMCLUBM_CARD_DEDUCT_NO_BALANCE,
	GEMCLUBM_CARD_NULL_DEDUCT_AMOUNT_REQUIRED,
    GEMCLUBM_CARD_BALANCE_SUCCESS
};
typedef enum en_smart_card_state TSmart_card_state;

#pragma pack (1)

typedef struct
{
	uint16_t CardFlags;
	uint32_t balance;
	uint32_t deduction;
	uint32_t tokens;
	uint32_t fraction;
	uint32_t tokenvalue;
	uint8_t CardID[8];	// may be less than fully occupied
	uint32_t CardMap[GEMCLUB_BUFFER_SIZE];
} AsyncCardDB;

#pragma pack  (1)
struct st_smart_card_object
{
    TSmart_card_state smart_card_state;					// set by various to control smartcard state machine
    uint8_t detected;							    // managed by crcard_mdk to indicate detection (ATR +)
    uint8_t insert_flag;							// managed by smartcard to indicate presence
    uint8_t end_smart_card; 						// managed by drivers to indicate end of operation
	uint32_t  card_type; 							// set by smartcard from config
	uint8_t access_card_id[ACCESS_CARD_ID_SIZE]; 	// contains retrieved ID
	uint8_t card_id[CARD_ID_SIZE]; 				// unknown
	uint8_t id_checked;  	 						// used by driver to indicate authentication state
	uint32_t balance; 								// used by driver to store balance
	uint32_t encode_amount; 						// unknown
	uint32_t deduct_amount;							// used by driver to store deducted amount
	uint32_t credit_amount;							// used by driver to store amount to be refunded
};
typedef struct st_smart_card_object 	TSmart_card;

typedef struct
{
	uint8_t refund_cardid[6];
	uint16_t refund_amount;
} REFUND_UPDATE;

typedef struct
{
	uint16_t RecordID;
	uint16_t CustomerID;
	uint16_t MeterID;
	uint32_t CurrentAuditTimeStamp;
	uint32_t LastAuditTimeStamp;
	uint16_t NoOfValidCoins;
	uint16_t NoOfInvalidCoins;
	uint16_t NoOf100CentCoins;
	uint16_t NoOf25CentCoins;
	uint16_t NoOf10CentCoins;
	uint16_t NoOf5CentCoins;
	uint32_t TotalCoinAmount;
	uint16_t NoOfSCCardCount;
	uint32_t TotalSCCardAmount;
	//uint32_t CardDataCRC;
	//uint16_t Reserved[2]; // Nebraska customer wants battery voltage to display and record it to audit card when audit card is used so using 2bytes for it out of Reserved[2]//vinay
	uint16_t Battery_Voltage;
	uint16_t Reserved[1];
} SCOSTA_AUDIT_DATA;

#define SCOSTA_MAX_RECORDS			250

/**************************************************************************/
/*	Defines  						                                      */
/**************************************************************************/
#define SMARTCARD_CLA_TYPE1 	0x80
//#define SMARTCARD_CLA_TYPE2 	0  //not used //vinay
#define SMARTCARD_INS_READ 		0xBE
#define SMARTCARD_INS_VERIFY 	0x20
#define SMARTCARD_INS_UPDATE 	0xDE
#define SMARTCARD_STATUS_OK 	0x90

//SCOSTA Command List
#define SCOSTA_CLASS_TYPE			0X00
//#define SCOSTA_CREATE_FILE       	0xE0  //not used //vinay
//#define SCOSTA_DELETE_FILE       	0xE4  //not used //vinay
#define SCOSTA_SELECT_FILE       	0xA4
#define SCOSTA_ACTIVATE_FILE       	0x44
//#define SCOSTA_WRITE_RECORD       	0xD2  //not used //vinay
#define SCOSTA_READ_RECORD       	0xB2
#define SCOSTA_APPEND_RECORD       	0xE2


AsyncCardDB acdb;

TSmart_card smart_card;
SCOSTA_AUDIT_DATA Scosta_Audit_Data;

void Smart_Card_Reset();
uint8_t smartcard_write(uint8_t*command,uint8_t len);
void Identify_Smart_Card_Type();
void Process_smart_card_payment();
uint32_t smartcard_encrypt(uint32_t *iDataVal, uint8_t *pKey);

/**************************************************************************/
/*	Application data prototypes                                           */
/**************************************************************************/
uint8_t DeductAmountDuncanAsyncCard(void);
uint8_t AuthenticateDuncanAsyncCard(void);
void balance_gemclub_memo_card(void);
uint8_t RetrieveBalanceDuncanAsyncCard(void);
//uint8_t CheckManufacturerList(uint32_t dwManufacturer);
uint8_t ReadAsyncCardWord(uint8_t address, uint32_t *pWord);
uint8_t VerifyAsyncCardWord(uint8_t address, uint32_t Word);
uint8_t UpdateAsyncCardWord(uint8_t address, uint32_t Word, uint8_t fRetry);
//uint8_t ResetAsyncCard(void);  //not used //vinay
void deduct_gemclub_memo_card(uint32_t deduct_amount);
void SYNCard_SaveDeductRefundValue(uint32_t amount);
void end_read_card(void);
uint32_t L_Auth(uint32_t *pdwData, uint8_t *pKey);
//uint16_t ConvertKey(uint8_t *keyin, uint8_t *keyout);  //not used //vinay
void GEMCardSetCardID(uint8_t *id_ptr);
void SYNCardReadCardID(uint8_t *id_ptr);
uint32_t SYNCardReadDeductRedundValue(void);
void Refund_CardSN(void);
uint8_t Refund_CampareSN(void);
void Refund_SNcheck_timeleft(void);
uint8_t RefundAmountDuncanAsyncCard(uint32_t Addamount);
void refund_gemclub_memo_card (uint32_t calc_refund_amount);
uint8_t SCOSTA_ReadAsyncCardWord(uint8_t command, uint8_t address, uint32_t *pWord);
uint8_t SCOSTA_WriteAsyncCardWord(uint8_t command, uint8_t address, uint32_t Word, uint8_t fRetry);
uint8_t SCOSTA_smartcard_write(uint8_t*command, uint8_t len);
/**************************************************************************/
/*	Data avalible to other functions                                      */
/**************************************************************************/

#endif /* PERIPHERAL_MODULES_LIBG2_SMARTCARD_H_ */
