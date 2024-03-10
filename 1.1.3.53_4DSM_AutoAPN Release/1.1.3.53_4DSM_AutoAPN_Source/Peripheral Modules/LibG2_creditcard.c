//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_creditcard.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_creditcard_api
//! @{
//
//*****************************************************************************
#include "LibG2_creditcard.h"

#define Enable_Reduce_CC_Processing_Delay //If you want to minimize/reduce CC_Processing_Delay while doing card transaction, enable this


extern GPRSSystem       			glSystem_cfg;
extern uint8_t 						/*MDM_AWAKE,*/smart_card_mode,scosta_smartcard_mode;//not used in this program //vinay
extern TTrackString     			fTrack2String, fTrack1String, fTrack3String;
extern uint8_t 						/*AES_KEY[AES_KEY_SIZE], */CC_Auth_Entered;//not used in this program //vinay
//extern uint16_t 					AES_KEY_ID;//not used in this program //vinay
extern volatile uint32_t			parking_time_left_on_meter[MSM_MAX_PARKING_BAYS], negetive_parking_time_left_on_meter[MSM_MAX_PARKING_BAYS];
extern volatile uint8_t 			current_rate_index,Max_Amount_In_current_rate;
extern uint8_t 						*TData;
extern uint8_t						RData[500];
extern uint16_t 					Eof,RXIndex;
extern uint8_t 						RespIndex;
extern uint8_t 						Parking_Clock_Refresh[MSM_MAX_PARKING_BAYS], grace_time_trigger[MSM_MAX_PARKING_BAYS];
extern uint8_t                      do_CC_OLT, Tech_menu, Current_bayStatus;
//extern Tech_Menu_Disp   			tech_menu_disp;//not used in this program //vinay
extern uint8_t     					rx_ans[ GPRS_GEN_BUF_SZ_SMALL ];

extern uint16_t 					Current_Space_Id;
//extern GPRS_AlarmsRequest   		glAlarm_req;//not used in this program //vinay
extern GPRS_PamBayStatusUpdateRequest  PAM_Bay_status_data[MAX_PAM_BSU_QUEUE];
extern uint8_t 						PAM_Baystatus_update_count;
extern uint8_t						Meter_Full_Flg[MSM_MAX_PARKING_BAYS], meter_out_of_service, do_not_allow_cc_auth;
extern Maintenance_Evt      		Maintenance_Log;
extern uint8_t              		PBC_expt_rcvd[4];
extern uint8_t              		PBC_amnt_rcvd[2];
extern uint16_t						pbc_amount_recvd;
extern uint32_t						pbc_expiry_time_recvd;
extern uint32_t						last_payment_try_time[MSM_MAX_PARKING_BAYS];
extern uint8_t 						SC_bytes;
extern uint8_t 						RXData[MAX_SCT_RX_BYTES];
extern uint32_t  					Refund_PreviousSN;
extern uint32_t  					Refund_PresentSN;
extern uint32_t  					Refund_timestamp;
extern uint32_t  					Refund_earned_mins, santa_cruz_previous_paid_time;
extern uint8_t	 					RefundOK, in_prepay_parking[MSM_MAX_PARKING_BAYS], glLast_reported_space_status[MSM_MAX_PARKING_BAYS], ZERO_OUT_ENABLE_AT_EXPIRY[MSM_MAX_PARKING_BAYS], ZERO_OUT[MSM_MAX_PARKING_BAYS];
extern uint8_t						this_is_a_CC;
//uint8_t								SMS_Print;

uint8_t 							T2_Faw_flag = 0, T1_Faw_flag = 0;
uint8_t         					glCreditcard_frame[40],
                					glCreditcard_frame1[80],
									/*glCard_entered_flag = FALSE,*/
									glCCexpiry_date[2],
									/*glSCexpiry_date[3],*/
									ccard_dataread_flag = FALSE,
									card_removed_flag   = FALSE,
									check_card_read     = FALSE;
//uint8_t         					olt_sz;//not used in this program //vinay
uint8_t 							Connected_Card_reader = 0;
uint16_t  							gl_CC_amount_selected = 0;
uint16_t         					glCurrent_amount = 0;//, min_calculated_amount = 0;//,
									//max_calculated_amount = 0;//, pre_existing_time_in_mins = 0;//not used in this program //vinay
uint32_t 							OLT_start_time=0,OLT_end_time=0, olt_time_taken=0;
uint32_t 							CC_TimeOut_RTCTimestamp = 0;
uint8_t								mag_technician_card = FALSE, mag_collection_card = FALSE;
uint8_t								IDTECH_AUDIT_CARD = FALSE, Credit_Card_processed_screen = FALSE;

//Credit_Card_validate//1909

const char 							CFG_VISA_PREF[]       = "4";
const char 							CFG_VISA_PREF1[]      = "400000000000";
const char 							CFG_VISA_PREF2[]      = "429000999999";    /* +1 = AAA Range 429001 ~429099*/

const char 							CFG_VISA_PREF3[]      = "429100000000";
const char 							CFG_VISA_PREF4[]      = "438099999999";    /* +1 = AAA Range 438100 ~438270**/

const char 							CFG_VISA_PREF5[]      = "438271000000";
const char 							CFG_VISA_PREF6[]      = "499999999999";

//MEMORY_OPTIMIZATION
//const char 						CFG_MASTCARD_PREF_X1[] = "5018000000";    /* St george debit cards */
//const char 						CFG_MASTCARD_PREF_X2[] = "5039000000";    /* credit union */

const char 							CFG_MASTCARD_PREF_1[] = "5000000000";
const char 							CFG_MASTCARD_PREF_2[] = "5331899999";
const char 							CFG_MASTCARD_PREF_3[] = "5332000000";
const char 							CFG_MASTCARD_PREF_4[] = "5599999999";

const char 							CFG_AMEX_PREF_1[]     = "34";
const char 							CFG_AMEX_PREF_2[]     = "37";

const char 							CFG_DINERS_PREF_1[]   = "300";
const char 							CFG_DINERS_PREF_2[]   = "305";
const char 							CFG_DINERS_PREF_3[]   = "36";
const char 							CFG_DINERS_PREF_4[]   = "38";
const char 							CFG_DINERS_PREF_5[]   = "39";

const char 							CFG_ENROUTE_PREF_1[]  = "2014";
const char 							CFG_ENROUTE_PREF_2[]  = "2149";

const char 							CFG_DISCOVER_PREF[]   = "6011";

const char 							CFG_JCB_PREF_1[]      = "3";
const char 							CFG_JCB_PREF_2[]      = "2131";
const char 							CFG_JCB_PREF_3[]      = "1800";

const char 							CFG_REINO_TEST_CARD[] = "6008";

const char 							CFG_MAG_TECH_CARD[] = "031700000000";
const char 							CFG_MAG_COLL_CARD[] = "031600000000";
//Credit_Card_validate//1909

uint16_t        					fBitCntp,fBitCnt1,fBitCnt2,fBitCnt3;
uint16_t        					fOneCntp,fOneCnt1,fOneCnt2,fOneCnt3;
uint16_t        					fTruncBitCntp,fTruncBitCnt1,fTruncBitCnt2,fTruncBitCnt3;
uint8_t		       					fBitsPerByteT2,fBitsPerByteT1,fBitsPerByteT3;
uint16_t        					fLRCOnestrack1[8],fLRCOnestrack2[8],fLRCOnestrack3[8];
uint8_t			    				fCharMask;
uint8_t       						fStartSentinelChar,fStartSentinelCharT3;
uint8_t       						fEndSentinelChar;
uint16_t        					fParityError1,fParityError2,fParityError3;
uint16_t           					fReadingCard;
uint16_t        					fNotReadCardCnt;

TTrackData              			fTrack1Data,fTrack2Data;
TTrackString            			fTrack1String,fTrack2String; //1908
GPRS_OLTRequest                 	glOLT_req;
GPRS_OLTResponse                	glOLT_resp;
GPRS_OLTACKRequest              	glOLTACK_req;
uint32_t 							CardJam_TimeOut_RTC = 0;
uint8_t 							CardJam_Flag = 0;

//static uint8_t  					temp_arr[ GPRS_GEN_BUF_SZ_LargeQ ];
//---------------------------------IDTECH-----------------------------------
uint8_t 							RespIndex;
uint8_t 							CardData[25];
uint8_t 							CardDataEncT2[25];

//uint8_t 							card_valid=false;  // defined outside to use it in rates.c vinay for max time in credit card payment in split rates //it is not used anywhere //vinay
uint8_t								cardread=0; //to avoid display processing please wait msg in card payment//vinay
//Added for CC Processing Delay
extern uint8_t modem_init_to_process_cc_transaction;//@r just added for testing
////uint8_t CC_Transaction_Retry_Request_Interval;//@r just added for testing
////extern uint8_t request_internal_retried_once; //@r just added for testing
uint8_t								cc_approved=0; // When the network issuse is there we are getting approve twice and motorist is getting double parking clock// to fix it //vinay
extern uint8_t						Modem_Off_initiated; //to turn off modem for cc payment  //vinay
/**************************************************************************/
//! Initialize variables For New Card Read
//! \param void
//! \return void
/**************************************************************************/
void InitForNewRead()
{
	fBitCntp = 0;
	fBitCnt1 = 0;
	fBitCnt2 = 0;
	fBitCnt3 = 0;
	fOneCntp = 0;
	fOneCnt1 = 0;
	fOneCnt2 = 0;
	fOneCnt3 = 0;

	fTruncBitCntp = 0;
	fTruncBitCnt1 = 0;
	fTruncBitCnt2 = 0;
	fTruncBitCnt3 = 0;
	fParityError1 = 0;
	fParityError2 = 0;
	fParityError3 = 0;
}

/**************************************************************************/
//! Converts Track1 Data to String Format
//! \param void
//! \return void
/**************************************************************************/
void Track1DataToString()
{
	uint16_t loBitNo = 0;
	uint16_t loBitArrayNdx;
	uint16_t loByteNdx = 0;
	uint16_t loOneCnt = 0;
	uint8_t  loThisChar = 0;
	uint16_t loFoundStart = 0;
	uint16_t loFoundEnd = 0;

	memset( fLRCOnestrack1, 0, sizeof(fLRCOnestrack1)); // 2012-05-08 ajw
	fCharMask=Track13Charmask;
	fStartSentinelChar=StartSentinelChar13;
	fEndSentinelChar=EndSentinelChar13;

	// convert track bits to bytes
	fParityError1 = 0;
	for (loBitArrayNdx = 0; loBitArrayNdx < (uint16_t)fBitCnt1; loBitArrayNdx++)
	{

		loOneCnt += fTrack1Data[ loBitArrayNdx ];
		fLRCOnestrack1[ loBitNo ] += fTrack1Data[ loBitArrayNdx ];
		loThisChar  |= fTrack1Data[ loBitArrayNdx ] << loBitNo++;

		if (loBitNo != fBitsPerByteT1) continue;

		loBitNo = 0;

		loThisChar &= fCharMask;

		// have a full character, do we have a start sentinel yet?
		if (!loFoundStart)
		{
			loFoundStart = loThisChar == fStartSentinelChar;
		}

		if (!loFoundStart)
		{
			// reset the calculated LRC, this char doesn't count
			memset(fLRCOnestrack1, 0, sizeof(fLRCOnestrack1) );
			loOneCnt = 0;
			continue;
		}

		// is this a parity error? (should be an odd number of ones)
		if ((loOneCnt % 2) == 0)
		{
			fParityError1++;
		}

		// if we have already found the end, then this is the LRC, don't add it
		if (loFoundEnd)
		{
			break;
		}

		loFoundEnd = loThisChar == fEndSentinelChar;

		if (fBitsPerByteT1 == 5)
		{     // track 2 needs ascii "0" added to value
			loThisChar |= 0x30;
		}
		else
		{     // tracks 1 & 3 need ascii " " added to value
			loThisChar += 0x20;
		}
		fTrack1String[ loByteNdx ] = loThisChar;
		loThisChar = 0; // clear out for next time through

		if (++loByteNdx >= ( MAX_TRK_BYTES-1))
		{
			break;
		}
	}

	//terminate the string
	fTrack1String[ loByteNdx ] = 0;

	if(Validate_CivicSmart_Card_with_track1())
	{
		if(mag_technician_card == TRUE)
		{
			Tech_menu=true;
			smart_card_mode = FALSE;
			Debug_TextOut( 0, "LNG is in tech mode" );
			push_event_to_cache( UDP_EVTTYP_TECH_MENU_ACCESS );//09-04-12
			TechMenu_Display_Main_Menu();
		}
		else if(mag_collection_card == TRUE)
		{
			smart_card_mode = FALSE;
			Tech_menu = FALSE;
			push_event_to_cache( UDP_EVTTYP_COLLECTION_EVT );	//Audit event raised; server will take care of collection report
			AUDIT_Screen();
			DelayMs(3000);
		}
	}
}

/**************************************************************************/
//! Converts Track2 Data to String Format
//! \param void
//! \return void
/**************************************************************************/
void Track2DataToString()
{
	uint16_t loBitNo = 0;
	uint16_t loBitArrayNdx=0;
	uint16_t loByteNdx = 0;
	uint16_t loOneCnt = 0;
	uint8_t  loThisChar = 0;
	uint16_t loFoundStart = 0;
	uint16_t loFoundEnd = 0;

	memset( fLRCOnestrack2, 0, sizeof(fLRCOnestrack2));

	fBitsPerByteT2        = 5;
	fCharMask            = Track2CharMask;
	fStartSentinelChar    = StartSentinelChar2;
	fEndSentinelChar    = EndSentinelChar2;

	// convert track bits to bytes
	fParityError2 = 0;

	for (loBitArrayNdx = 0; loBitArrayNdx < (uint16_t) fBitCnt2; loBitArrayNdx++)
	{
		loOneCnt += fTrack2Data[ loBitArrayNdx ];
		fLRCOnestrack2[ loBitNo ] += fTrack2Data[ loBitArrayNdx ];
		loThisChar  |= fTrack2Data[ loBitArrayNdx ] << loBitNo++;

		if (loBitNo != fBitsPerByteT2) continue;

		loBitNo = 0;

		loThisChar &= fCharMask;

		// have a full character, do we have a start sentinel yet?
		if (!loFoundStart) loFoundStart = loThisChar == fStartSentinelChar;

		if (!loFoundStart)
		{
			// reset the calculated LRC, this char doesn't count
			memset(fLRCOnestrack2, 0, sizeof(fLRCOnestrack2) );
			loOneCnt = 0;
			continue;
		}

		// is this a parity error? (should be an odd number of ones)
		if ((loOneCnt % 2) == 0) fParityError2++;

		// if we have already found the end, then this is the LRC, don't add it
		if (loFoundEnd) break;

		loFoundEnd = loThisChar == fEndSentinelChar;

		if (fBitsPerByteT2 == 5)
		{     // track 2 needs ascii "0" added to value
			loThisChar |= 0x30;
		}
		else
		{     // tracks 1 & 3 need ascii " " added to value
			loThisChar += 0x20;
		}

		fTrack2String[ loByteNdx ] = loThisChar;
		loThisChar = 0; // clear out for next time through

		if (++loByteNdx >= ( MAX_TRK_BYTES-1)) break;
	}

	//terminate the string
	fTrack2String[ loByteNdx ] = 0;

	if(Validate_CivicSmart_Card_with_track2())
	{
		if(mag_technician_card == TRUE)
		{
			Tech_menu=true;
			smart_card_mode = FALSE;
			Debug_TextOut( 0, "LNG is in tech mode" );
			push_event_to_cache( UDP_EVTTYP_TECH_MENU_ACCESS );//09-04-12
			TechMenu_Display_Main_Menu();
		}
		else if(mag_collection_card == TRUE)
		{
			smart_card_mode = FALSE;
			Tech_menu = FALSE;
			push_event_to_cache( UDP_EVTTYP_COLLECTION_EVT );	//Audit event raised; server will take care of collection report
			AUDIT_Screen();
			DelayMs(3000);
		}
	}
}

/**************************************************************************/
//! Converts Track3 Data to String Format
//! \param void
//! \return void
/**************************************************************************/
void Track3DataToString()
{
#ifdef _no_capes_
	short loBitNo = 0;
	short loBitArrayNdx;
	short loByteNdx = 0;
	short loOneCnt = 0;
	char loThisChar = 0;
	short int loFoundStart = 0;
	short int loFoundEnd = 0;
	memset( fLRCOnestrack3, 0, sizeof(fLRCOnestrack3));
	fCharMask=Track13Charmask;
	fEndSentinelChar=EndSentinelChar13;

	// convert track bits to bytes
	fParityError3 = 0;
	for (loBitArrayNdx = 0; loBitArrayNdx < (uint16_t) fBitCnt3; loBitArrayNdx++)
	{
		loOneCnt += fTrack3Data[ loBitArrayNdx ];
		fLRCOnestrack3[ loBitNo ] += fTrack3Data[ loBitArrayNdx ];
		loThisChar  |= fTrack3Data[ loBitArrayNdx ] << loBitNo++;

		if (loBitNo != fBitsPerByteT3) continue;
		loBitNo = 0;

		loThisChar &= fCharMask;


		// have a full character, do we have a start sentinel yet?
		if (!loFoundStart) loFoundStart = loThisChar == fStartSentinelCharT3;

		if (!loFoundStart)
		{
			// reset the calculated LRC, this char doesn't count
			memset(fLRCOnestrack3, 0, sizeof(fLRCOnestrack3) );
			loOneCnt = 0;
			continue;
		}

		// is this a parity error? (should be an odd number of ones)
		if ((loOneCnt % 2) == 0) fParityError3++;

		// if we have already found the end, then this is the LRC, don't add it
		if (loFoundEnd) break;

		loFoundEnd = loThisChar == fEndSentinelChar;

		if (fBitsPerByteT3 == 5)
		{     // track 2 needs ascii "0" added to value
			loThisChar |= 0x30;
		}
		else
		{     // tracks 1 & 3 need ascii " " added to value
			loThisChar += 0x20;
		}
		fTrack3String[ loByteNdx ] = loThisChar;
		loThisChar = 0; // clear out for next time through

		if (++loByteNdx >= ( MAX_TRK_BYTES-1)) break;

	}

	//terminate the string
	fTrack3String[ loByteNdx ] = 0;
#endif
}

/**************************************************************************/
//! Checks to see we got sufficient bytes to proceed
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint16_t SufficientData()
{
	// at least 10 bits & mixed 0's and ones
	return (((fBitCnt2 >= 10) && (fOneCnt2) && (fBitCnt2 != fOneCnt2 ))&&((fBitCnt1 >= 10) && (fOneCnt1) && (fBitCnt1 != fOneCnt1 )));//13-04-12//in both the cases it is copied to track1
}

/*
*
* Reads in one track's worth of data and places it track buffer
* If iKeepData is false, data is not stored. In this mode,
* one can empty the chip of its data.
*/
/**************************************************************************/
//! Reads magnetic stripe track preamble data, Reads in one track's worth of
//! data and places it track buffer.
//! \param iBitsToRead - Number of bits to read
//! \param iKeepData - If iKeepData is false, data is not stored
//! \return void
/**************************************************************************/
void ReadMagStripeTrackp( uint16_t iBitsToRead, uint16_t iKeepData )
{
	uint16_t loGPLR;
	uint16_t loData;
	uint16_t loBitNdx;

	for (loBitNdx = 0; loBitNdx < (uint16_t) iBitsToRead; loBitNdx++)
	{
		// read this bit from the magstripe IC
		// Step 7: strobe low to begin pulling data from ASIC, must wait at least 170ns
		STROBE_LOW();
		DelayUs(4);// have to be careful, timer resolution is ~300ns, here waiting ~1us
		// read the level register
		loGPLR = GPIO_getInputPinValue(MAGTEK_SDA_PORT, MAGTEK_SDA_PIN);

		// Step 8: strobe high to end strobe and wait at least 40ns.
		STROBE_HIGH();
		// have to be careful, timer resolution is ~300ns, here waiting ~1us
		DelayUs(1);

		// wait for data to go high
		//if ( !WaitForMagStripeDataLevel( COUNTER, true ) )
		{
		}

		if (!iKeepData) continue; // no need to store data

		// make sure we don't go over - MAX_TRK_BITS is the ARRAY size, not the track size
		if (fBitCntp >= MAX_TRK_BITS)
		{
			fTruncBitCntp++;
			continue; // no room for it, but keep reading to drain IC
		}

		//  Remember that data is inverted, and it was masked during the read
		loData = ( loGPLR == 0 );

		//2012-04-04 ajw - remove unused declaration
		// store this bit and count it
		//fTrackPreambledata[ fBitCntp++ ] = (char) loData;                           ///JBENDOR-2012.03.19
		fBitCntp++;
		//2012-04-04 ajw - end

		// track how many ones
		fOneCntp += loData;
	}
}

/**************************************************************************/
//! Reads magnetic stripe track1 data, Reads in one track's worth of data
//! and places it track buffer.
//! \param iBitsToRead - Number of bits to read
//! \param iKeepData - If iKeepData is false, data is not stored
//! \return void
/**************************************************************************/
void ReadMagStripeTrack1( uint16_t iBitsToRead, uint16_t iKeepData )
{
	uint16_t loGPLR;
	uint16_t loData;
	uint16_t loBitNdx;

	for (loBitNdx = 0; loBitNdx < (uint16_t) iBitsToRead; loBitNdx++)
	{
		// read this bit from the magstripe IC
		// Step 7: strobe low to begin pulling data from ASIC, must wait at least 170ns
		STROBE_LOW();
		// have to be careful, timer resolution is ~300ns, here waiting ~1us
		DelayUs(4);
		// read the level register
		loGPLR = GPIO_getInputPinValue(MAGTEK_SDA_PORT, MAGTEK_SDA_PIN);

		// Step 8: strobe high to end strobe and wait at least 40ns.

		STROBE_HIGH();
		// have to be careful, timer resolution is ~300ns, here waiting ~1us
		DelayUs(1);

		// wait for data to go high
		//  if ( !WaitForMagStripeDataLevel( COUNTER, true ) )
		{
		}

		if (!iKeepData) continue; // no need to store data

		// make sure we don't go over - MAX_TRK_BITS is the ARRAY size, not the track size
		if (fBitCnt1 >= MAX_TRK_BITS)
		{
			fTruncBitCnt1++;
			continue; // no room for it, but keep reading to drain IC
		}

		//  Remember that data is inverted, and it was masked during the read
		loData = ( loGPLR == 0 );
		// store this bit and count it

		fTrack1Data[ fBitCnt1++ ] = (char) loData;
		// track how many ones
		fOneCnt1 += loData;
	}
}

/**************************************************************************/
//! Reads magnetic stripe track2 data, Reads in one track's worth of data
//! and places it track buffer.
//! \param iBitsToRead - Number of bits to read
//! \param iKeepData - If iKeepData is false, data is not stored
//! \return void
/**************************************************************************/
void ReadMagStripeTrack2( uint16_t iBitsToRead, uint16_t iKeepData )
{
	uint16_t loGPLR;
	uint16_t loData;
	uint16_t loBitNdx;

	for (loBitNdx = 0; loBitNdx < (uint16_t) iBitsToRead; loBitNdx++)
	{
		// read this bit from the magstripe IC
		// Step 7: strobe low to begin pulling data from ASIC, must wait at least 170ns
		STROBE_LOW();
		// have to be careful, timer resolution is ~300ns, here waiting ~1us
		DelayUs(4);
		// read the level register
		loGPLR = GPIO_getInputPinValue(MAGTEK_SDA_PORT, MAGTEK_SDA_PIN);

		// Step 8: strobe high to end strobe and wait at least 40ns.

		STROBE_HIGH();
		// have to be careful, timer resolution is ~300ns, here waiting ~1us
		DelayUs(1);

		// wait for data to go high
		// if ( !WaitForMagStripeDataLevel( COUNTER, true ) )
		{
		}

		if (!iKeepData) continue; // no need to store data

		// make sure we don't go over - MAX_TRK_BITS is the ARRAY size, not the track size
		if (fBitCnt2 >= MAX_TRK_BITS)
		{
			fTruncBitCnt2++;
			continue; // no room for it, but keep reading to drain IC
		}

		//  Remember that data is inverted, and it was masked during the read
		loData = ( loGPLR == 0 );
		// store this bit and count it
		fTrack2Data[ fBitCnt2++ ] = (char) loData;
		// track how many ones
		fOneCnt2 += loData;
	}
}

/**************************************************************************/
//! Reads magnetic stripe track3 data, Reads in one track's worth of data
//! and places it track buffer.
//! \param iBitsToRead - Number of bits to read
//! \param iKeepData - If iKeepData is false, data is not stored
//! \return void
/**************************************************************************/
void ReadMagStripeTrack3( uint16_t iBitsToRead, uint16_t iKeepData )
{
	uint16_t loGPLR;
	uint16_t loData;
	uint16_t loBitNdx;

	for (loBitNdx = 0; loBitNdx < (uint16_t) iBitsToRead; loBitNdx++)
	{
		// read this bit from the magstripe IC
		// Step 7: strobe low to begin pulling data from ASIC, must wait at least 170ns
		STROBE_LOW();
		// have to be careful, timer resolution is ~300ns, here waiting ~1us
		DelayUs(4);
		// read the level register
		loGPLR = GPIO_getInputPinValue(MAGTEK_SDA_PORT, MAGTEK_SDA_PIN);

		// Step 8: strobe high to end strobe and wait at least 40ns.
		STROBE_HIGH();
		// have to be careful, timer resolution is ~300ns, here waiting ~1us
		DelayUs(1);

		// wait for data to go high
		// if ( !WaitForMagStripeDataLevel( COUNTER, true ) )
		{
		}

		if (!iKeepData) continue; // no need to store data

		// make sure we don't go over - MAX_TRK_BITS is the ARRAY size, not the track size
		if (fBitCnt3 >= MAX_TRK_BITS)
		{
			fTruncBitCnt3++;
			continue; // no room for it, but keep reading to drain IC
		}

		loData = ( loGPLR == 0 );

		//2012-04-04 ajw - remove unused declaration
		// store this bit and count it
		//fTrack3Data[ fBitCnt3++ ] = (char) loData;
		fBitCnt3++;
		//2012-04-04 ajw end

		// track how many ones
		fOneCnt3 += loData;
	}
}

/*
* Sets up hardware in preparation for read.
*/
/**************************************************************************/
//! Sets up hardware in preparation for read.Initialize buffers for a new read.
//! \param void
//! \return void
/**************************************************************************/
void PrepareForRead()
{
	memset( fTrack1Data, 0, sizeof(TTrackData) );
	memset( fTrack2Data, 0xFF, sizeof(TTrackData) );

	//set memory to read trackdata as string
	memset(fTrack1String, 0, sizeof(TTrackString));
	memset(fTrack2String, 0, sizeof(TTrackString));

	fReadingCard = 1;
}

/**************************************************************************/
//! Finish reading credit card
//! \param void
//! \return void
/**************************************************************************/
void FinishRead()
{
	fReadingCard = 0;

	// convert bit stream to bytes.
	Track2DataToString();
	//Track2DataToString1();//commented to reduce software delay//3005:shruthi


	//2012-04-04 ajw - not reading driver licenses (someday?)

	//if (memcmp(&fTrack2->fTrack2String[1], SixBitTrack13CA_ID, 6 ) == 0)
	if (memcmp(&fTrack2String[1], SixBitTrack13CA_ID, 6 ) == 0)
	{ // 6 bit Track13 CA license
		//fTrack1->fBitsPerByte = 6;
		//fTrack3->fBitsPerByte = 6;
		fBitsPerByteT1 = 6;
		fBitsPerByteT3 = 6;
		// CA 6 bit has a different start sentinel
		fStartSentinelCharT3 = StartSentinelCharTrack3Only_CA6bit;
	}
	else
		//2012-04-04 ajw end
	{ // 7 bit Track13 license
		fStartSentinelCharT3 =StartSentinelChar13;
		fBitsPerByteT1 = 7;
		fBitsPerByteT3 = 7;
	}

	Track1DataToString();
	//Track1DataToString1();//commented to reduce software delay//3005:shruthi
	Track3DataToString();
	//Track3DataToString1();//commented to reduce software delay//3005:shruthi
}

/**************************************************************************/
//! Record credit card track data
//! \param void
//! \return void
/**************************************************************************/
void RecordTrackData()
{
	uint16_t i;

	// if we're already busy, don't start again!
	if (!fReadingCard)
	{
		fNotReadCardCnt++;
		return;
	}

	// read in the preamble data
	ReadMagStripeTrackp( MAGSTRIPE_HARDWARE_PREAMBLE_BITS, true );
	// read in three tracks, saving the data

	ReadMagStripeTrack1( MAGSTRIPE_HARDWARE_TRACK_BITS, true);
	ReadMagStripeTrack2( MAGSTRIPE_HARDWARE_TRACK_BITS, true);

	//  account for mounting position of hardware
#ifdef READ_TRACK1_DATA_FROM_TRACK3
	// read track 3 data into track 1
	fBitCnt1 = 0;
	fOneCnt1 = 0;
	fTruncBitCnt1 = 0;
	fParityError1 = 0;
	memset( fTrack1Data, 0, sizeof(TTrackData) );
	ReadMagStripeTrack1( MAGSTRIPE_HARDWARE_TRACK_BITS, true);
#else
	// read track 3 data and discard
	ReadMagStripeTrack3( MAGSTRIPE_HARDWARE_TRACK_BITS, true);
#endif


	// what is the chips purpose of presenting the data repeatedly?
	// should we re-read and compare as a checksum?

	// the data is presented forward, then backward
	// it is presented a total of four times,
	// so we need to loop and read tracks to drain the IC
	// 21 times total
	// for (int loDrainNdx = 0; loDrainNdx < 21; loDrainNdx++)
	for (i = 0; i < 21; i++)
	{

		ReadMagStripeTrackp( MAGSTRIPE_HARDWARE_TRACK_BITS, false );
	}

	// and just ONE MORE read (strobe) to clear out the IC buffer
	ReadMagStripeTrackp( 1, false );
}

/**************************************************************************/
//! Pull credit card track data from mag
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint16_t PullDataFromMagCard()
{
	InitForNewRead();
	// reset the track buffers
	PrepareForRead();

	// pull it in
	RecordTrackData();

	// finish up
	FinishRead();

	STROBE_LOW();

	return true;
}

/**************************************************************************/
//! Get credit card data from IC
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint16_t GetMagDataFromIC()
{
	uint16_t	elapsed_time = 0;
	// we've recieved the first edge detect that data is available on
	// the magstripe
#define MAX_SETUP_ATTEMPTS 10

	/*
    1)  STROBE_MAGSTRP is an output that is normally held high.
    2)  DATA_MAGSTRP is an input that is normally HIGH.
    3)  Upon completion of a swipe, DATA_MAGSTRP will be driven low.
        That triggers an edge detect interrupt that the main
        driver code has been wait for before calling this rountine
    4)  Pulse STROBE_MAGSTRP two times
        a.       1st time: low 500ns, high 500ns
	 */

	// loop till we get the right response or give up
	uint16_t loAttempts;

	for (loAttempts = 0; loAttempts < MAX_SETUP_ATTEMPTS; loAttempts++ )
	{
		// pulse low
		STROBE_LOW();
		DelayUs(4);
		// wait for data to go high
		/*if(loAttempts > 5)
        {
			if ( !WaitForMagStripeDataLevel( COUNTER, true ) )
			{
				// timeout!
				//continue;
				return PullDataFromMagCard();
			}
        }*/

		// pulse high
		// wait 500ns... er as close as we can
		STROBE_HIGH();
		DelayUs(4);
		// 2nd time: low 12.4us, high 250ns
		STROBE_LOW();
		DelayUs(4);
		// pulse high -
		STROBE_HIGH();
		// wait for data to go low
		/*       if ( !WaitForMagStripeDataLevel( COUNTER,true  ) )
        {
            // timeout!
            continue;
        }
		 */
		//Wait for data line to go low
		while ((1 == GPIO_getInputPinValue(MAGTEK_SDA_PORT, MAGTEK_SDA_PIN)) && (++elapsed_time < 4) )
		{
			DelayMs(1);
		}

		// set strobe high to finish flag clearing sequence, and wait 250ns
		STROBE_HIGH();
		DelayUs(4);

		// go get the data!!
		return PullDataFromMagCard();
	}

	// 5 tries, no luck

	// cycle power to the mag head to reset it
	//PowerDownMagStripe
	//if(Connected_Card_reader == GEM_CLUB_READER)
		MAG_1_POWER_OFF();
	DelayMs(250);
	//PowerUpMagStripe
	MAG_1_POWER_ON();
	return false;
}

/**************************************************************************/
//! Validate credit card with track2 data
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t Validate_Credit_Card_with_track2()//1909
{
	uint8_t ccard_data_buf[16], card_serial_number_to_report[5];
	char *serial_no_index = 0;
	uint8_t ccard_data_len=0;
	uint8_t card_valid=false;  //already defined as global variable //vinay
	uint8_t i=0,j=0;
	
	//if(T1_Faw_flag==1)
	//return false;
	memset(ccard_data_buf,0,sizeof(ccard_data_buf));
	for(i=0;i<sizeof(fTrack2String);i++)
	{
		if(((fTrack2String[i]!=0x3D)&&(fTrack2String[i]!=0x3B))&&(j<sizeof(ccard_data_buf)))
		{
			ccard_data_buf[j++]=fTrack2String[i];
			ccard_data_len++;
		}
		else if(fTrack2String[i]==0x3D)
		{
			//credit_card_data_buf[i]=0xff;//end of credit card number
			break;//end of credit card number
		}
	}

	if ((strncmp((char*)ccard_data_buf,CFG_REINO_TEST_CARD,4)==0)&&(ccard_data_len == 16))
	{
		//Debug_TextOut( 0, "REINO TEST Card validated" );
		card_valid = true;
	}
	else if (strstr((char*)ccard_data_buf,CFG_MAG_TECH_CARD)!=0)//&&(ccard_data_len == 16))
	{
		serial_no_index = strstr((char*)ccard_data_buf,CFG_MAG_TECH_CARD);
		memcpy(card_serial_number_to_report, serial_no_index, 4);
		card_serial_number_to_report[4] = '\0';
		Maintenance_Log.TECH_CARD_SL_NO = atol((const char *)card_serial_number_to_report);
		//Debug_TextOut( 0, "Magtek TECH Card validated" );
		card_valid = true;
		mag_technician_card = TRUE;
	}
	else if (strstr((char*)ccard_data_buf,CFG_MAG_COLL_CARD)!=0)//&&(ccard_data_len == 16))
	{
		serial_no_index = strstr((char*)ccard_data_buf,CFG_MAG_COLL_CARD);
		memcpy(card_serial_number_to_report, serial_no_index, 4);
		card_serial_number_to_report[4] = '\0';
		Maintenance_Log.TECH_CARD_SL_NO = atol((const char *)card_serial_number_to_report);
		//Debug_TextOut( 0, "Magtek COLLECTION Card validated" );
		card_valid = true;
		mag_collection_card = TRUE;
	}
	else if  (( ((strncmp((char*)ccard_data_buf,CFG_VISA_PREF1,10)>=0)&&(strncmp((char*)ccard_data_buf,CFG_VISA_PREF2,10)<=0))
			||((strncmp((char*)ccard_data_buf,CFG_VISA_PREF3,10)>=0)&&(strncmp((char*)ccard_data_buf,CFG_VISA_PREF4,10)<=0))
			||((strncmp((char*)ccard_data_buf,CFG_VISA_PREF5,10)>=0)&&(strncmp((char*)ccard_data_buf,CFG_VISA_PREF6,10)<=0)))
			&&( (ccard_data_len == 16) || (ccard_data_len == 13)          )   )
	{
		//debug_CC_D("Validating VISA card");
		if(((glSystem_cfg.allowed_trans_types>>1) & 0x01) != 1)	//14-06-12
		{
			card_valid = false;
			//Debug_TextOut( 0, "VISA card not allowed" );
		}
		else if((ccard_data_len == 16)&&(ccard_data_buf[ccard_data_len-1]>0x39))
		{
			card_valid = false;
			//Debug_TextOut( 0, "VISA card length Invalid" );
		}
		else if((ccard_data_len == 13)&&(ccard_data_buf[ccard_data_len-1]<0x39)&&(ccard_data_buf[ccard_data_len-1]>=0x30))
		{
			card_valid = false;
			//Debug_TextOut( 0, "VISA card length Invalid" );
		}
		else
		{
			card_valid = true;
			//Debug_TextOut( 0, "VISA card validated" );
		}
	}
	else if  (( ((strncmp((char*)ccard_data_buf,CFG_MASTCARD_PREF_1,10)>=0)&&
			(strncmp((char*)ccard_data_buf,CFG_MASTCARD_PREF_2,10)<=0))
			||((strncmp((char*)ccard_data_buf,CFG_MASTCARD_PREF_3,10)>=0)&&
					(strncmp((char*)ccard_data_buf,CFG_MASTCARD_PREF_4,10)<=0)))
			&&(ccard_data_len == 16))
	{
		if(((glSystem_cfg.allowed_trans_types>>2) & 0x01) != 1)	//14-06-12
		{
			card_valid = false;
			//Debug_TextOut( 0, "MASTER card not allowed" );
		}
		else if(ccard_data_buf[ccard_data_len-1]>0x39)
		{
			card_valid = false;
			//Debug_TextOut( 0, "MASTER card length Invalid" );
		}
		else
		{
			card_valid = true;
			//Debug_TextOut( 0, "MASTER card validated" );
		}
	}
	// 2012-06-08 - ajw - re-enabled AMEX validation
	else if (((strncmp((char*)ccard_data_buf,CFG_AMEX_PREF_1,2)==0)	//2109
			||(strncmp((char*)ccard_data_buf,CFG_AMEX_PREF_2,2)==0))
			&&(ccard_data_len == AMEX_CARD_LENGTH))
	{
		if(((glSystem_cfg.allowed_trans_types>>3) & 0x01) != 1)	//14-06-12
		{
			card_valid = false;
			//Debug_TextOut( 0, "AMEX card not allowed" );
		}
		else
		{
			card_valid = true;
			//Debug_TextOut( 0, "AMEX card validated" );
		}

	}
	// 2012-06-08 - ajw - end

	else if ((((strncmp((char*)ccard_data_buf,CFG_DINERS_PREF_1,3)>=0)
			&&(strncmp((char*)ccard_data_buf,CFG_DINERS_PREF_2,3)<=0))
			||(strncmp((char*)ccard_data_buf,CFG_DINERS_PREF_3,2)==0)
			||(strncmp((char*)ccard_data_buf,CFG_DINERS_PREF_4,2)==0)
			||(strncmp((char*)ccard_data_buf,CFG_DINERS_PREF_5,2)==0))
			&&(ccard_data_len == DINNER_CARD_LENGTH))
	{
		if(((glSystem_cfg.allowed_trans_types>>4) & 0x01) != 1)	//14-06-12
		{
			card_valid = false;
			//Debug_TextOut( 0, "DINERS card not allowed" );
		}
		else
		{
			//Debug_TextOut( 0, "DINERS card validated" );
			card_valid = true;
		}

	}
	else if (((strncmp((char*)ccard_data_buf,CFG_ENROUTE_PREF_1,4)==0)
			||(strncmp((char*)ccard_data_buf,CFG_ENROUTE_PREF_2,4)==0))
			&&(ccard_data_len == ENROUTE_CARD_LENGTH))
	{
		if(((glSystem_cfg.allowed_trans_types>>5) & 0x01) != 1)	//14-06-12
		{
			card_valid = false;
			//Debug_TextOut( 0, "ENROUTE card not allowed" );
		}
		else
		{
			//Debug_TextOut( 0, "ENROUTE card validated" );
			card_valid = true;
		}


	}
	else if ((strncmp((char*)ccard_data_buf,CFG_DISCOVER_PREF,4)==0)
			&&(ccard_data_len == DISCOVER_CARD_LENGTH))
	{
		if(((glSystem_cfg.allowed_trans_types>>6) & 0x01) != 1)	//14-06-12
		{
			card_valid = false;
			//Debug_TextOut( 0, "DISCOVER card not allowed" );
		}
		else
		{
			//Debug_TextOut( 0, "DISCOVER card validated" );
			card_valid = true;
		}

	}
	else if ( ((strncmp((char*)ccard_data_buf,CFG_JCB_PREF_1,1)==0)&&(ccard_data_len == 20))
			||(((strncmp((char*)ccard_data_buf,CFG_JCB_PREF_2,4)==0)
					||(strncmp((char*)ccard_data_buf,CFG_JCB_PREF_3,4)==0))
					&&(ccard_data_len == JCB_CARD_LENGTH)) )
	{
		if(((glSystem_cfg.allowed_trans_types>>7) & 0x01) != 1)	//14-06-12
		{
			card_valid = false;
			//Debug_TextOut( 0, "JCB card not allowed" );
		}
		else
		{
			//Debug_TextOut( 0, "JCB card validated" );
			card_valid = true;
		}
	}
	else
	{
		//Debug_TextOut( 0, "NOT A CREDIT CARD " );
		card_valid = false;
	}

	return  card_valid;
}

/**************************************************************************/
//! Validate credit card with track1 data
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t Validate_Credit_Card_with_track1()
{
	//if(T2_Faw_flag==1)
	//return false;
	uint8_t ccard_data_buf[16], card_serial_number_to_report[5];
	char *serial_no_index = 0;
	uint8_t ccard_data_len=0;
	uint8_t card_valid=false;
	uint8_t i=0,j=0;
	
	memset(ccard_data_buf,0,sizeof(ccard_data_buf));

	for(i=0;i<sizeof(fTrack1String);i++)
	{
		if((fTrack1String[i]!=0x5E)&&(fTrack1String[i]!=0x25)&&(fTrack1String[i]!=0x42)&&(j<sizeof(ccard_data_buf)))
		{
			ccard_data_buf[j++]=fTrack1String[i];
			ccard_data_len++;
		}
		else if(fTrack1String[i]==0x5E)
		{
			//credit_card_data_buf[i]=0xff;//end of credit card number
			break;//end of credit card number
		}
	}

	if ((strncmp((char*)ccard_data_buf,CFG_REINO_TEST_CARD,4)==0)&&(ccard_data_len == 16))
	{
		//Debug_TextOut( 0, "REINO TEST Card validated" );
		card_valid = true;
	}
	else if (strstr((char*)ccard_data_buf,CFG_MAG_TECH_CARD)!=0)//&&(ccard_data_len == 16))
	{
		serial_no_index = strstr((char*)ccard_data_buf,CFG_MAG_TECH_CARD);
		memcpy(card_serial_number_to_report, serial_no_index, 4);
		card_serial_number_to_report[4] = '\0';
		Maintenance_Log.TECH_CARD_SL_NO = atol((const char *)card_serial_number_to_report);
		//Debug_TextOut( 0, "Magtek TECH Card validated" );
		card_valid = true;
		mag_technician_card = TRUE;
	}
	else if (strstr((char*)ccard_data_buf,CFG_MAG_COLL_CARD)!=0)//&&(ccard_data_len == 16))
	{
		serial_no_index = strstr((char*)ccard_data_buf,CFG_MAG_COLL_CARD);
		memcpy(card_serial_number_to_report, serial_no_index, 4);
		card_serial_number_to_report[4] = '\0';
		Maintenance_Log.TECH_CARD_SL_NO = atol((const char *)card_serial_number_to_report);
		//Debug_TextOut( 0, "Magtek COLLECTION Card validated" );
		card_valid = true;
		mag_collection_card = TRUE;
	}
	else if  (( ((strncmp((char*)ccard_data_buf,CFG_VISA_PREF1,10)>=0)&&(strncmp((char*)ccard_data_buf,CFG_VISA_PREF2,10)<=0))
			||((strncmp((char*)ccard_data_buf,CFG_VISA_PREF3,10)>=0)&&(strncmp((char*)ccard_data_buf,CFG_VISA_PREF4,10)<=0))
			||((strncmp((char*)ccard_data_buf,CFG_VISA_PREF5,10)>=0)&&(strncmp((char*)ccard_data_buf,CFG_VISA_PREF6,10)<=0)))
			&&( (ccard_data_len == 16) || (ccard_data_len == 13)          )   )
	{
		//debug_CC_D("Validating VISA card");
		if(((glSystem_cfg.allowed_trans_types>>1) & 0x01) != 1)	//14-06-12
		{
			card_valid = false;
			//Debug_TextOut( 0, "VISA card not allowed" );
		}
		else if((ccard_data_len == 16)&&(ccard_data_buf[ccard_data_len-1]>0x39))
		{
			card_valid = false;
			//Debug_TextOut( 0, "VISA card length Invalid" );
		}
		else if((ccard_data_len == 13)&&(ccard_data_buf[ccard_data_len-1]<0x39)&&(ccard_data_buf[ccard_data_len-1]>=0x30))
		{
			card_valid = false;
			//Debug_TextOut( 0, "VISA card length Invalid" );
		}
		else
		{
			card_valid = true;
			//Debug_TextOut( 0, "VISA card validated" );
		}
	}
	else if  (( ((strncmp((char*)ccard_data_buf,CFG_MASTCARD_PREF_1,10)>=0)&&
			(strncmp((char*)ccard_data_buf,CFG_MASTCARD_PREF_2,10)<=0))
			||((strncmp((char*)ccard_data_buf,CFG_MASTCARD_PREF_3,10)>=0)&&
					(strncmp((char*)ccard_data_buf,CFG_MASTCARD_PREF_4,10)<=0)))
			&&(ccard_data_len == 16))
	{
		if(((glSystem_cfg.allowed_trans_types>>2) & 0x01) != 1)	//14-06-12
		{
			card_valid = false;
			//Debug_TextOut( 0, "MASTER card not allowed" );
		}
		else if(ccard_data_buf[ccard_data_len-1]>0x39)
		{
			card_valid = false;
			//Debug_TextOut( 0, "MASTER card length Invalid" );
		}
		else
		{
			card_valid = true;
			//Debug_TextOut( 0, "MASTER card validated" );
		}

	}
	else if (((strncmp((char*)ccard_data_buf,CFG_AMEX_PREF_1,2)==0)    //2109
			||(strncmp((char*)ccard_data_buf,CFG_AMEX_PREF_2,2)==0))
			&&(ccard_data_len == AMEX_CARD_LENGTH))
	{
		if(((glSystem_cfg.allowed_trans_types>>3) & 0x01) != 1)	//14-06-12
		{
			card_valid = false;
			//Debug_TextOut( 0, "AMEX card not allowed" );
		}
		else
		{
			//Debug_TextOut( 0, "AMERICAN_EXPRESS card validated" );
			card_valid = true;
		}
	}        //2109
	else if ((((strncmp((char*)ccard_data_buf,CFG_DINERS_PREF_1,3)>=0)
			&&(strncmp((char*)ccard_data_buf,CFG_DINERS_PREF_2,3)<=0))
			||(strncmp((char*)ccard_data_buf,CFG_DINERS_PREF_3,2)==0)
			||(strncmp((char*)ccard_data_buf,CFG_DINERS_PREF_4,2)==0)
			||(strncmp((char*)ccard_data_buf,CFG_DINERS_PREF_5,2)==0))
			&&(ccard_data_len == DINNER_CARD_LENGTH))
	{
		if(((glSystem_cfg.allowed_trans_types>>4) & 0x01) != 1)	//14-06-12
		{
			card_valid = false;
			//Debug_TextOut( 0, "DINERS card not allowed" );
		}
		else
		{
			//Debug_TextOut( 0, "DINERS card validated" );
			card_valid = true;
		}
	}
	else if (((strncmp((char*)ccard_data_buf,CFG_ENROUTE_PREF_1,4)==0)
			||(strncmp((char*)ccard_data_buf,CFG_ENROUTE_PREF_2,4)==0))
			&&(ccard_data_len == ENROUTE_CARD_LENGTH))
	{
		if(((glSystem_cfg.allowed_trans_types>>5) & 0x01) != 1)	//14-06-12
		{
			card_valid = false;
			//Debug_TextOut( 0, "ENROUTE card not allowed" );
		}
		else
		{
			//Debug_TextOut( 0, "ENROUTE card validated" );
			card_valid = true;
		}
	}
	else if ((strncmp((char*)ccard_data_buf,CFG_DISCOVER_PREF,4)==0)
			&&(ccard_data_len == DISCOVER_CARD_LENGTH))
	{
		if(((glSystem_cfg.allowed_trans_types>>6) & 0x01) != 1)	//14-06-12
		{
			card_valid = false;
			//Debug_TextOut( 0, "DISCOVER card not allowed" );
		}
		else
		{
			//Debug_TextOut( 0, "DISCOVER card validated" );
			card_valid = true;
		}
	}
	else if ( ((strncmp((char*)ccard_data_buf,CFG_JCB_PREF_1,1)==0)&&(ccard_data_len == 20))
			||(((strncmp((char*)ccard_data_buf,CFG_JCB_PREF_2,4)==0)
					||(strncmp((char*)ccard_data_buf,CFG_JCB_PREF_3,4)==0))
					&&(ccard_data_len == JCB_CARD_LENGTH)) )
	{
		if(((glSystem_cfg.allowed_trans_types>>7) & 0x01) != 1)	//14-06-12
		{
			card_valid = false;
			//Debug_TextOut( 0, "JCB card not allowed" );
		}
		else
		{
			//Debug_TextOut( 0, "JCB card validated" );
			card_valid = true;
		}
	}
	else
	{
		//Debug_TextOut( 0, "NOT A CREDIT CARD " );
		card_valid = false;
	}

	return  card_valid;
}
//14-06-12: added configurable card type support check - end

/**************************************************************************/
//! Process credit card switch interrupt
//! \param void
//! \return void
/**************************************************************************/
void credit_card_insert_isr (void)
{
	uint16_t	elapsed_time = 0, elapsed_time1 = 0, elapsed_time2 = 0;
	uint8_t Read_Magtek_data = FALSE;

	//Debug_TextOut(0,"CC 1308 credit card ISR");
//#ifdef 	Enable_Reduce_CC_Processing_Delay
//	////CC_Transaction_Retry_Request_Interval = 1; //@r just added for testing
//	modem_init_to_process_cc_transaction = 1; //@r just added for testing
//	////request_internal_retried_once = 0;  //@r just added for testing
//	//@r start power on modem
//	GPIO_setAsPeripheralModuleFunctionInputPin(MDM_PORT, MDM_RXD_PIN, GPIO_PRIMARY_MODULE_FUNCTION);	//MDM_RXD
//	MDM_RST_clr();
//	telit_wakeup_sleep(FALSE);
//
//	TELIT_POWER_ENABLE();
//	DelayMs(300);
//	MDM_Tel_ON();
//	//@r end power on modem
//#endif

	smart_card_mode = FALSE;
	do_not_allow_cc_auth = FALSE;
	card_removed_flag = FALSE;
	check_card_read = FALSE;
	ccard_dataread_flag = FALSE;
	glCurrent_amount = 0;
	T2_Faw_flag = 0;
	T1_Faw_flag = 0;
	Meter_Full_Flg[Current_Space_Id] = FALSE;
	mag_technician_card = FALSE;
	mag_collection_card = FALSE;
	IDTECH_AUDIT_CARD = FALSE;
	Credit_Card_processed_screen = FALSE;

	if(1 == GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN))//added to fix re-reading card while removing
	{
		return;
	}//@ravi

	LCD_Backlight(TRUE);
	////push_event_to_cache( UDP_EVTTYP_CARD_SWITCH_PRESS );
	GPIO_setAsInputPinWithPullUpResistor(MAGTEK_SDA_PORT, MAGTEK_SDA_PIN);		//SDA Signal

	last_payment_try_time[Current_Space_Id] = RTC_epoch_now();
	//if(glLast_reported_space_status[Current_Space_Id] == 0)
	if((glSystem_cfg.vehicle_sensor_enable == TRUE)&&((glLast_reported_space_status[Current_Space_Id] == 0) && ((current_rate_index < 20) || (current_rate_index == PREPAY_PARKING))) )
	{
		ZERO_OUT_ENABLE_AT_EXPIRY[Current_Space_Id] = TRUE;
		ZERO_OUT[Current_Space_Id] = FALSE;
		//Debug_TextOut(0,"PA on Vac, Won't ZO till next 801");
		Debug_Output1(0,"PA on Vac, CC No ZO till next 801 for space [%d]",Current_Space_Id);
	}

	//if(meter_out_of_service == FALSE) //LNGSIT-591
	{
		if(Connected_Card_reader == GEM_CLUB_READER)
		{
			DelayMs(60);
			MAG_1_POWER_ON();
			Smart_Card_Reset();
			while ((smart_card_mode == false) && (++elapsed_time2 < CARD_TYPE_DETECTION_LIMIT_MS))//08-08-12:to handle invalid smart card
			{
				DelayMs(1);
			}
		}
		else
		{
			MAG_1_POWER_OFF();
			DelayMs(60);
			MAG_1_POWER_ON();
			IDTech_POWER_ON();

			if (GetAtr())
			{
				if(IDTECH_AUDIT_CARD == FALSE)
				{
					//LNGSIT-483
					if((current_rate_index == FREE_PARKING)||(current_rate_index == NO_PARKING))//Do not accept card for these modes
					{
						if(current_rate_index == NO_PARKING)
						{
							Noparking_Screen();
							//Debug_TextOut( 0, "PA-C:no_parking" );
						}
						else if(current_rate_index == FREE_PARKING)
						{
							Freeparking_Screen();
							//Debug_TextOut( 0, "PA-C:free_parking" );
						}
						//LNGSIT-827
						CardJam_TimeOut_RTC = RTC_epoch_now();
						CardJam_Flag = 1;
						card_removed_flag = FALSE;

						Turn_Off_card_IOs(Connected_Card_reader);
						return;
					}

					//ReadCard_Screen();
					StartTrnsaction();
				}
				else
				{
					ccard_dataread_flag = FALSE;
					card_removed_flag = FALSE;
					IDTECH_AUDIT_CARD = FALSE;
					Turn_Off_card_IOs(Connected_Card_reader);
					Idle_Screen();
					return;
				}
			}
			else
			{
				Read_Magtek_data = TRUE;
			}

			IDTech_POWER_OFF();
			//LNGSIT-483
			/*if((current_rate_index == FREE_PARKING)||(current_rate_index == NO_PARKING))//Do not accept card for these modes
			{
				if(current_rate_index == NO_PARKING)
				{
					Noparking_Screen();
					Debug_TextOut( 0, "PA-C:no_parking" );
				}
				else if(current_rate_index == FREE_PARKING)
				{
					Freeparking_Screen();
					Debug_TextOut( 0, "PA-C:free_parking" );
				}
				Turn_Off_card_IOs(Connected_Card_reader);
				return;
			}*/
		}
		if ( ((smart_card_mode == false) && (Connected_Card_reader == GEM_CLUB_READER)) || (Read_Magtek_data == TRUE))
		{
			/*if (1 != GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN))
			{
				Remove_Card_Screen();
			}*/

			if(Connected_Card_reader == GEM_CLUB_READER)
			{
				if(smart_card_mode == false)
				{
					cardread = 1; //to avoid processing please wait msg //vinay
					//Debug_TextOut(0,"CC1428CardRead=1");
					Remove_Card_Screen();
					//below if is not there in regular DSM
					if((Current_Space_Id != LEFT_BAY) && (Current_Space_Id != RIGHT_BAY))
					{
						//Debug_Output1(2,"Current_Space_Id_2 = %d",Current_Space_Id);
						DSM_Screen7();
						return;
					}
				}

				//Debug_Output6(0,"bytes:%d, %d, %d, %d, %d, %d", SC_bytes, RXData[0], RXData[1], RXData[2], RXData[3], RXData[4]);

				while ((0 == GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN)) && (++elapsed_time1 < 3000) ) //WAIT FOR CARD REMOVAL-5SECS
				{
					DelayMs(1);
				}
				//LNGSIT-483
				if((current_rate_index == FREE_PARKING)||(current_rate_index == NO_PARKING))//Do not accept card for these modes
				{
					if(current_rate_index == NO_PARKING)
					{
						Noparking_Screen();
						//Debug_TextOut( 0, "PA-C:no_parking" );
					}
					else if(current_rate_index == FREE_PARKING)
					{
						Freeparking_Screen();
						//Debug_TextOut( 0, "PA-C:free_parking" );
					}
					//LNGSIT-827
					CardJam_TimeOut_RTC = RTC_epoch_now();
					CardJam_Flag = 1;
					card_removed_flag = FALSE;

					Turn_Off_card_IOs(Connected_Card_reader);
					if(modem_init_to_process_cc_transaction)
					{
						Modem_Off_initiated = 2; //modem off for card if on //vinay
						telit_power_off(); //modem off for card if on //vinay
					}
					return;
				}
			}
			else
			{
				while ((1 == GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN)) && (++elapsed_time1 < 3000) ) //WAIT FOR CARD REMOVAL-5SECS
				{
					DelayMs(1);
				}
			}

			if ( elapsed_time1 >= 3000 )//if time out
			{
				Debug_TextOut( 0, "Card Remove timeout-01" );
				CardJam_TimeOut_RTC = RTC_epoch_now();
				CardJam_Flag = 1;
				card_removed_flag = FALSE;
				push_event_to_cache( UDP_EVTTYP_CARD_SWITCH_PRESS );
				push_event_to_cache( UDP_EVTTYP_CARD_REMOVE_TIMEOUT );
				//if(Connected_Card_reader == GEM_CLUB_READER)
				//MAG_1_POWER_OFF();
				Turn_Off_card_IOs(Connected_Card_reader);
				DelayMs(250);
                Idle_Screen(); //when card processing is taking too much long then without displaying decline or approve it will come to ideal screen so to infrom customer to reinsert card //vinay
                //card_process_timeout_screen(); //
				return;
			}
			else //CARD REMOVED
			{
				card_removed_flag = TRUE;
				Max_Amount_In_current_rate = FALSE;
			}

			//Wait for data line to go low
			while ((1 == GPIO_getInputPinValue(MAGTEK_SDA_PORT, MAGTEK_SDA_PIN)) && (++elapsed_time < 3000) )
			{
				DelayMs(1);
			}

			if ( elapsed_time >= 3000 )//if time out
			{
				Debug_TextOut( 0, "Card Read Fail" );
				card_removed_flag = FALSE;
				//CardReadFail_Screen();
				push_event_to_cache( UDP_EVTTYP_CARD_SWITCH_PRESS );
				push_event_to_cache( UDP_EVTTYP_CARD_READ_FAIL );
				//if(Connected_Card_reader == GEM_CLUB_READER)
				//MAG_1_POWER_OFF();
				Turn_Off_card_IOs(Connected_Card_reader);
				if(modem_init_to_process_cc_transaction)
				{
					Modem_Off_initiated = 2; //modem off for card if on //vinay
					telit_power_off(); //modem off for card if on //vinay
				}
				DelayMs(500);
				if(CardReadFail_Screen()) return;
				Idle_Screen();
				return;
			}
			if( GetMagDataFromIC() )
			{
				if( SufficientData())
				{
					//if(CredCardCheckFormat())
					if(( CredCardCheckFormat()) || (CredCardCheckFormat2()))
					{
						//if(( CredCardCheckFormat()) || (CredCardCheckFormat2()))
						{
							Debug_TextOut( 0, "Magstripe Card Read Success" );

	#ifdef 	Enable_Reduce_CC_Processing_Delay
							Debug_TextOut(0,"****Power ON Telit Modem****");
							////CC_Transaction_Retry_Request_Interval = 1; //@r just added for testing
							modem_init_to_process_cc_transaction = 1; //@r just added for testing
							////request_internal_retried_once = 0;  //@r just added for testing
							//@r start power on modem
							GPIO_setAsPeripheralModuleFunctionInputPin(MDM_PORT, MDM_RXD_PIN, GPIO_PRIMARY_MODULE_FUNCTION);	//MDM_RXD
							MDM_RST_clr();
							telit_wakeup_sleep(FALSE);

							TELIT_POWER_ENABLE();
							DelayMs(300);
							MDM_Tel_ON();
							//@r end power on modem
	#endif

							ccard_dataread_flag=TRUE;
							push_event_to_cache( UDP_EVTTYP_CARD_SWITCH_PRESS );
							push_event_to_cache( UDP_EVTTYP_CARD_READ_SUCCESS );
							//crd_read_ok = TRUE;// check read ok
						}
						//ccard_dataread_flag=TRUE;
						//push_event_to_cache( UDP_EVTTYP_CARD_SWITCH_PRESS );
						//push_event_to_cache( UDP_EVTTYP_CARD_READ_SUCCESS );
						//crd_read_ok = TRUE;// check read ok
					}
					/*else if((CredCardCheckFormat2()) && (glSystem_cfg.CC_TRACK1_ENABLE == TRUE)) //Track1
					{
						ccard_dataread_flag=TRUE;
						push_event_to_cache( UDP_EVTTYP_CARD_READ_SUCCESS );
						//crd_read_ok = TRUE;// check read ok
					}*/
					else
					{
						ccard_dataread_flag=FALSE;
						card_removed_flag = FALSE;
						////CardReadFail_Screen();
						Debug_TextOut( 0, "Card Read Fail 1" );
						push_event_to_cache( UDP_EVTTYP_CARD_SWITCH_PRESS );
						push_event_to_cache( UDP_EVTTYP_CARD_READ_FAIL );
						//if(Connected_Card_reader == GEM_CLUB_READER)
						//MAG_1_POWER_OFF();
						Turn_Off_card_IOs(Connected_Card_reader);
						DelayMs(500);
						if(CardReadFail_Screen()) return;
						Idle_Screen();
						return;
					}
				}
			}

			if((T2_Faw_flag==1) && (meter_out_of_service == FALSE)) //LNGSIT-591
			{
				if(Validate_Credit_Card_with_track2())
				{
					Debug_TextOut( 0, "Track2 OK" );
					if( check_card_read_ok(FALSE) )
					{
						//Start CC TimeOut Time
						CC_TimeOut_RTCTimestamp = RTC_epoch_now();
						check_card_read = TRUE;
						glCurrent_amount = glSystem_cfg.Card_default_amount;
						ShowAmount_Screen(glCurrent_amount, 0,TRUE);
					}
					else
					{
						CC_Card_Expired_Screen();
						push_event_to_cache( UDP_EVTTYP_EXPIRED_CARD_SWIPE );
						DelayMs(3000);
						Idle_Screen();
					}

				}
				else
				{
					Debug_TextOut( 0, "Card Not supported 2" );
					push_event_to_cache( UDP_EVTTYP_CARD_NOT_SUPPORTED );
					Card_Not_Supported_Screen();
					DelayMs(3000);
					Idle_Screen();
				}
			}
			//else if ((T1_Faw_flag==1) && (meter_out_of_service == FALSE) && (glSystem_cfg.CC_TRACK1_ENABLE == TRUE)) //LNGSIT-591
			else if ((T1_Faw_flag==1) && (meter_out_of_service == FALSE)) //LNGSIT-591
			{
				if(Validate_Credit_Card_with_track1())
				{
					Debug_TextOut( 0, "Track1 OK" );
					if( check_card_read_ok(FALSE) )
					{
						//Start CC TimeOut Time
						CC_TimeOut_RTCTimestamp = RTC_epoch_now();
						check_card_read = TRUE;
						glCurrent_amount = glSystem_cfg.Card_default_amount;
						ShowAmount_Screen(glCurrent_amount, 0,TRUE);
					}
					else
					{
						CC_Card_Expired_Screen();
						push_event_to_cache( UDP_EVTTYP_EXPIRED_CARD_SWIPE );
						DelayMs(3000);
						Idle_Screen();
					}
				}
				else
				{
					Debug_TextOut( 0, "Card Not supported 1" );
					push_event_to_cache( UDP_EVTTYP_CARD_NOT_SUPPORTED );
					Card_Not_Supported_Screen();
					DelayMs(3000);
					Idle_Screen();
				}
			}
			else
			{
				if((mag_technician_card == FALSE) && (mag_collection_card == FALSE) && (meter_out_of_service == FALSE)) //LNGSIT-591
				{
					Debug_TextOut( 0, "Card Read Fail 2" );
					////CardReadFail_Screen();
					push_event_to_cache( UDP_EVTTYP_CARD_READ_FAIL );
					check_card_read = FALSE;
					ccard_dataread_flag = FALSE;
					T2_Faw_flag = 0;
					T1_Faw_flag = 0;
					if(CardReadFail_Screen()) return;
				}
				else
				{
					mag_technician_card = FALSE;
					mag_collection_card = FALSE;
				}
			}
		}
		else if((smart_card_mode == TRUE) || (scosta_smartcard_mode == TRUE))
		{
			push_event_to_cache( UDP_EVTTYP_CARD_SWITCH_PRESS );
			if(glSystem_cfg.SC_refund_enable == TRUE)	//Allow Refund only if enabled in config
				Refund_SNcheck_timeleft();
			else
				RefundOK = FALSE;

			if (RefundOK == FALSE)
			{
				ReadCard_Screen();
				Debug_TextOut( 0, "Smart Card\r\n" );
				Identify_Smart_Card_Type();
			}
			Idle_Screen();
		}
	}
	Turn_Off_card_IOs(Connected_Card_reader);


}

/**************************************************************************/
//! Check card expiry
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t check_card_read_ok(uint8_t idtech_card_type)
{
	uint32_t    current_time = RTC_epoch_now(); //(uint32_t) get_RTC();
	uint32_t     expired_RTC_tm;
	time_set_t   exp_time_set;

	if(T2_Faw_flag==1)
	{
		if(idtech_card_type!=TRUE)
			get_CC_data();

		exp_time_set.year       = glCCexpiry_date[0];
		exp_time_set.month      = glCCexpiry_date[1] ;
		exp_time_set.date       = 1;
		exp_time_set.hours      = 0;
		exp_time_set.minutes    = 0;
		exp_time_set.seconds    = 1;

		if(exp_time_set.month > 12)
		{
			exp_time_set.year++;
			exp_time_set.month = 1;
		}

		expired_RTC_tm = RTC_from_year(&exp_time_set);

		if (expired_RTC_tm < current_time)
		{
			Debug_TextOut( 0, "CARD Expired" );
			if(modem_init_to_process_cc_transaction)
			{
				Modem_Off_initiated = 2; //modem off for card if on //vinay
				telit_power_off(); //modem off for card if on //vinay
			}
			return FALSE;
		}
		else
		{
			Debug_TextOut( 0, "Date valid" );
			return TRUE;
		}
	}
	//else if((T1_Faw_flag==1) && (glSystem_cfg.CC_TRACK1_ENABLE == TRUE))//24-02-12
	else if(T1_Faw_flag==1)//24-02-12
	{
		get_CC_data2();

		exp_time_set.year       = glCCexpiry_date[0];
		exp_time_set.month      = glCCexpiry_date[1] ;
		exp_time_set.date       = 1;
		exp_time_set.hours      = 0;
		exp_time_set.minutes    = 0;
		exp_time_set.seconds    = 1;

		if(exp_time_set.month > 12)
		{
			exp_time_set.year++;
			exp_time_set.month = 1;
		}

		expired_RTC_tm = RTC_from_year(&exp_time_set);

		if (expired_RTC_tm < current_time)
		{
			Debug_TextOut( 0, "CARD_EXPIRED" );
			return FALSE;
		}
		else
		{
			Debug_TextOut( 0, "Date valid" );
			return TRUE;
		}
	}
	return FALSE;
}

/**************************************************************************/
//! Check track2 data format
//! \param void
//! \return uint8_t result
//! - \b CCS_OK: If test passes.
//! - \b CCS_BAD_FORMAT: didn't find the format chars
/**************************************************************************/
uint8_t CredCardCheckFormat()//TRACK2 FORMAT//forward swipe
{
	uint8_t result = 1; //CCS_OK;
	int i;
	uint8_t *pccc = glCreditcard_frame;
	uint8_t ccc;
	int seq = 0;

	memset(glCreditcard_frame, 0, sizeof(glCreditcard_frame) );

	memcpy(glCreditcard_frame,fTrack2String,sizeof(glCreditcard_frame));

	for(i=sizeof(glCreditcard_frame);i>0;i--)
	{

		ccc = (*pccc & 0x0F);

		switch(seq)
		{
		case 0:
			if(ccc == 0xB)
			{
				//    CREDCARD_DEBUG("(SS)" );
				//ccd_count = 0;
				seq++;
			}
			break;

		case 1:
			if(ccc == 0xD)
			{
				//CREDCARD_DEBUG("(FS)" );
				seq++;
			}
			break;

		case 2:
			if(ccc == 0xF)
			{
				//CREDCARD_DEBUG("(ES)" );
				seq++;
			}
			break;
		}

		pccc++;
	}

	//CREDCARD_DEBUG("" );

	if(seq<3)
	{
		result = 0; //CCS_BAD_FORMAT;
		T2_Faw_flag=0;
		//REDCARD_DEBUG("CCS_BAD_FORMAT\r\n" );
	}
	else
	{
		T2_Faw_flag=1;

	}
	return result;
}

/**************************************************************************/
//! Check track1 data format
//! \param void
//! \return uint8_t result
//! - \b CCS_OK: If test passes.
//! - \b CCS_BAD_FORMAT: didn't find the format chars
/**************************************************************************/
uint8_t CredCardCheckFormat2()//TRACK1 FORMAT//forward swipe
{
	uint8_t     result = 1; //CCS_OK/*creditcard_frame1[80]*/;
	int             i;
	uint8_t     *pccc = glCreditcard_frame1;
	uint8_t     ccc;
	//uint8_t *pccd = credcard_data;
	//uint16_t lrc = 0;
	int                 seq = 0;
	//int ccd_count = 0;

	memset(glCreditcard_frame1, 0, sizeof(glCreditcard_frame1) );

	memcpy(glCreditcard_frame1,fTrack1String,sizeof(glCreditcard_frame1));

	for( i = sizeof(glCreditcard_frame1); i > 0; i-- )
	{
		//ccc = (*pccc & 0x0F);//24-07-12:Track 1 carries valid characters for name, so check for full byte
		ccc = *pccc;//24-07-12:Track 1 carries valid characters for name, so check for full byte
		switch(seq)
		{
		case 0:
			if((ccc == 0x25)||(ccc == 0xA5))//25 /in hex for %(SS)
			{
				//    CREDCARD_DEBUG("(SS)" );
				//ccd_count = 0;
				seq++;
			}
			break;

		case 1:
			if((ccc == 0x5E)||(ccc == 0xDE))//5E IN HEX FOR ^(FS)
			{
				//CREDCARD_DEBUG("(FS)" );
				seq++;
			}
			break;

		case 2:
			if((ccc == 0x5E)||(ccc == 0xDE))//5E IN HEX FOR ^(FS)
			{
				//CREDCARD_DEBUG("(FS)" );
				seq++;
			}
			break;

		case 3:
			if((ccc == 0x3F)||(ccc == 0xBF))//ES
			{
				//CREDCARD_DEBUG("(ES)" );
				seq++;
			}
			break;
		}

		pccc++;
	}

	//CREDCARD_DEBUG("" );

	if( seq < 4 )
	{
		result         = 0; //CCS_BAD_FORMAT;
		T1_Faw_flag = 0;
		//REDCARD_DEBUG("CCS_BAD_FORMAT\r\n" );
	}
	else
	{
		T1_Faw_flag = 1;
	}
	return result;
}

/**************************************************************************/
//! extract Expiry date from Track2 data
//! \param void
//! \return void
/**************************************************************************/
void get_CC_data( void )////forward  swipe//track2
{
	uint8_t        i = 0, k = 0;

	memset(glCCexpiry_date, 0, sizeof(glCCexpiry_date));

	for(i=0;i<sizeof(fTrack2String);i++)
	{
		if((fTrack2String[i]==0x3D))
		{
			for(k=0;k<2;k++)
			{
				glCCexpiry_date[k]=((fTrack2String[(i+1)+(2*k)] & 0x0F)*10)+((fTrack2String[(i+2)+(2*k)])&0x0F);
			}
		}
	}
}

/**************************************************************************/
//! extract Expiry date from Track1 data
//! \param void
//! \return void
/**************************************************************************/
void get_CC_data2( void )//forward swipe//track1
{
	uint16_t i = 0, r = 0, k = 0;

	memset( glCCexpiry_date, 0, sizeof(glCCexpiry_date) );

	for(i=0;i<sizeof(fTrack1String);i++)
	{
		if((fTrack1String[i]==0x5E))
		{
			r++;
		}

		if(r==2)
		{
			for(k=0;k<2;k++)
			{
				glCCexpiry_date[k]=((fTrack1String[(i+1)+(2*k)] & 0x0F)*10)+((fTrack1String[(i+2)+(2*k)])&0x0F);
			}
			break;
		}
	}

	return;
}

/**************************************************************************/
//! Pre process credit card payment
//! \param void
//! \return void
/**************************************************************************/
void pre_process_CC_payment( void )
{
	ccard_dataread_flag = FALSE;
	//do_OLT = TRUE;
	construct_OLT_request();
}

/**************************************************************************/
//! OLT request packet formation
//! \param void
//! \return void
/**************************************************************************/
void construct_OLT_request()
{
	uint8_t i,j=0,k=0;
	uint32_t temp_earned_time_in_seconds = 0, pre_existing_seconds = 0;
	uint32_t temp_parking_clock_for_display = 0;
	static const uint8_t hextable[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	////////////////Forcing to send track1 for testing///////////////////////////////////////////

	memset(glCreditcard_frame1,0,sizeof(glCreditcard_frame1)); //27-04-12
	memset(glCreditcard_frame,0,sizeof(glCreditcard_frame)); //27-04-12

	if(T2_Faw_flag==1)
	{
		memcpy( glCreditcard_frame, fTrack2String, 40 ); //forward
	}
	//else if((T1_Faw_flag==1) && (glSystem_cfg.CC_TRACK1_ENABLE == TRUE))
	else if(T1_Faw_flag==1)
	{
		for(i=0;i<80;i++)
		{
			if(fTrack1String[i]=='^'&&j==0)
			{
				j=1;
				continue;
			}
			if((fTrack1String[i]=='^')&&j==1)
			{
				glCreditcard_frame1[k++]=fTrack1String[i]; //27-04-12
			}
			else
			{
				glCreditcard_frame1[k++]=fTrack1String[i]; //27-04-12
			}
		}
	}

	if(T2_Faw_flag==1)
	{
		for(i=0;i<40;i++)
		{
			glCreditcard_frame[i] = hextable[glCreditcard_frame[i] & 0x0F];
		}
		pre_existing_seconds = (parking_time_left_on_meter[Current_Space_Id]);

		this_is_a_CC = true;
		temp_parking_clock_for_display = get_earned_seconds(glCurrent_amount, true, false);
		this_is_a_CC = false;
		temp_earned_time_in_seconds = temp_parking_clock_for_display - pre_existing_seconds;

		glOLT_req.earned_mins = temp_earned_time_in_seconds/60;

		if((((temp_earned_time_in_seconds) % 3600) % 60) > 0)	//This is to fix the wrong update of parking clock
		{
			glOLT_req.earned_mins += 1;
		}

		glOLT_req.time_stamp = RTC_epoch_now();
		glOLT_req.multiple_event_counter = 1;
		glOLT_req.card_type = 10;
		gl_CC_amount_selected = glCurrent_amount;
		glOLT_req.amount = glCurrent_amount+((uint16_t)(glCurrent_amount*glSystem_cfg.CC_percentage_Surcharge)/100)+glSystem_cfg.CC_fixed_Surcharge; //glCurrent_amount;//31-07-3013:DPLIBB-552
		create_ref_id();
		glOLT_req.paid_until = (glOLT_req.time_stamp + temp_parking_clock_for_display/*(glOLT_req.earned_mins*60)*/) ;
		glOLT_req.bay_no = glSystem_cfg.bay_no;
		memset(glOLT_req.track2,0,sizeof(glOLT_req.track2)); //11-06-2013:for AES encryption pad extra bytes with 0
		memcpy( glOLT_req.track2, glCreditcard_frame, 40 );
	}
	//else if((T1_Faw_flag==1) && (glSystem_cfg.CC_TRACK1_ENABLE == TRUE))
	else if(T1_Faw_flag==1)
	{
		pre_existing_seconds = (parking_time_left_on_meter[Current_Space_Id]);

		this_is_a_CC = true;
		temp_parking_clock_for_display = get_earned_seconds(glCurrent_amount, true, false);
		this_is_a_CC = false;
		temp_earned_time_in_seconds = temp_parking_clock_for_display - pre_existing_seconds;

		glOLT_req.earned_mins = temp_earned_time_in_seconds/60;

		if((((temp_earned_time_in_seconds) % 3600) % 60) > 0)	//This is to fix the wrong update of parking clock
		{
			glOLT_req.earned_mins += 1;
		}

		glOLT_req.time_stamp = RTC_epoch_now();
		glOLT_req.multiple_event_counter = 1;
		glOLT_req.card_type = 10;
		gl_CC_amount_selected = glCurrent_amount;
		glOLT_req.amount = glCurrent_amount+((uint16_t)(glCurrent_amount*glSystem_cfg.CC_percentage_Surcharge)/100)+glSystem_cfg.CC_fixed_Surcharge; //glCurrent_amount;//31-07-3013:DPLIBB-552
		create_ref_id();
		glOLT_req.paid_until = (glOLT_req.time_stamp + temp_parking_clock_for_display/*(glOLT_req.earned_mins*60)*/) ;
		glOLT_req.bay_no = glSystem_cfg.bay_no;

		memset(glOLT_req.track1,0,sizeof(glOLT_req.track1)); //11-06-2013:for AES encryption pad extra bytes with 0
		memcpy( glOLT_req.track1, glCreditcard_frame1, 80 ); //27-04-12
	}


	return;
}

/**************************************************************************/
//! Generate reference id for OLT from time stamp
//! \param void
//! \return void
/**************************************************************************/
void create_ref_id()
{
	uint32_t epochtime;
	uint8_t reference_id[8];

	memset(reference_id, 0, 8);

	epochtime = RTC_epoch_now();
	reference_id[0] =    epochtime & 0xFF;
	reference_id[1] = (epochtime >> 8) & 0x0F;
	reference_id[2] = (epochtime >> 16) & 0xFF;
	memcpy( glOLT_req.ref_id, reference_id, 8 );
}

/**************************************************************************/
//! Handle credit card payment response from server
//! \param void
//! \return void
/**************************************************************************/
void Handle_OLT_response()
{
	uint16_t result = 0;
//	uint32_t /*temp_tm,*/ //tmp_currentRTC = 0, temp_earned_mins;
	uint32_t temp_parking_clock_for_display = 0;
	//uint32_t temp_earned_time_in_seconds = 0, pre_existing_seconds = 0;
	uint8_t  credit_card_approved = FALSE;
	//uint16_t elapsed_time3 = 0;
	//Debug_TextOut(0,"Handle_OLT_response");
	switch ( glOLT_resp.auth_code )
	{
	case GPRS_OLT_GWAY_RESP_Accepted:
		credit_card_approved = TRUE;
		Debug_TextOut( 0, "OLT GWAY_Accepted" );
		break;
	case GPRS_OLT_GWAY_RESP_Declined:
		credit_card_approved = FALSE;
		Debug_TextOut( 0, "OLT GWAY_Declined" );
		break;
	case GPRS_OLT_GWAY_RESP_Invalid_card:
		credit_card_approved = FALSE;
		Debug_TextOut( 0, "OLT GWAY_Invalid_card" );
		break;
	case GPRS_OLT_GWAY_RESP_Bad_expiry_date:
		credit_card_approved = FALSE;
		Debug_TextOut( 0, "OLT GWAY_Bad_expiry_date" );
		break;
	case GPRS_OLT_GWAY_RESP_Insufficient_funds:
		credit_card_approved = FALSE;
		Debug_TextOut( 0, "OLT GWAY_Insufficient_funds" );
		break;
	case GPRS_OLT_GWAY_RESP_Service_unavailable:
		credit_card_approved = TRUE;
		Debug_TextOut( 0, "OLT GWAY_Service_unavailable" );
		break;
	case GPRS_OLT_GWAY_RESP_Timeout:	//This is used for batching and showing approved screen to user
		credit_card_approved = TRUE;
		Debug_TextOut( 0, "OLT Timed Out & Txn Batched");
		break;
	default :
		credit_card_approved = TRUE;
		Debug_Output1( 0, "OLT GWAY_Unknown Code %d", glOLT_resp.auth_code );
		break;
	}

	//if(credit_card_approved == TRUE)
	// When the network issuse is there we are getting approve twice and motorist is getting double parking clock// to fix it //vinay
	//Debug_Output1( 0, "CC cc_approved = %d", cc_approved );
	//Debug_Output1( 0, "CC credit_card_approved = %d", credit_card_approved );
	if((credit_card_approved == TRUE) && (cc_approved == 1))
	{
		if((grace_time_trigger[Current_Space_Id] == true) && (glSystem_cfg.grace_time_type != 0)) // For Grace time feature
		{
			parking_time_left_on_meter[Current_Space_Id] = 0;
			negetive_parking_time_left_on_meter[Current_Space_Id] = 0;
			grace_time_trigger[Current_Space_Id] = false;
		}

		LCD_Backlight(TRUE);
		//pre_existing_seconds = parking_time_left_on_meter[Current_Space_Id];
		this_is_a_CC = true;
		temp_parking_clock_for_display = get_earned_seconds(glCurrent_amount, true, true);
		this_is_a_CC = false;
		//temp_earned_time_in_seconds = temp_parking_clock_for_display - pre_existing_seconds;

		parking_time_left_on_meter[Current_Space_Id] = temp_parking_clock_for_display;
		
		if(glSystem_cfg.santa_cruz_spl_feature_enable == TRUE)
			santa_cruz_previous_paid_time += glOLT_req.earned_mins;

		cc_approved = 0; //reset for next payment //vinay
		//Debug_TextOut( 0, "CC Approved");
		//Debug_Output1( 0, "CC cleared cc_approved = %d", cc_approved );
		//Debug_Output6(0,"PRExt:%ld, TLeft:%ld, tmptm:%ld, tmpclk:%ld", pre_existing_seconds,parking_time_left_on_meter,temp_tm,temp_parking_clock_for_display,0,0);
		CC_Payment_Approved_Screen();

		DelayMs(3000);//TODO: Use the config parameter for the duration of approved/declined screen
		CC_Auth_Entered = FALSE;
		do_CC_OLT = FALSE;
		in_prepay_parking[Current_Space_Id] = FALSE;

		//Reset SmartCard Refund Data after successful CC payment
		Refund_PreviousSN = 0;
		Refund_PresentSN = 0;
		Refund_timestamp = 0;
		Refund_earned_mins = 0;
		RefundOK = 0;
	}
	else
	{
		CC_Payment_Declined_Screen();
		cc_approved = 0; //reset for next olt payment //vinay
		//Debug_TextOut( 0, "CC Declined");
		//Debug_Output1( 0, "CC cleared cc_approved = %d", cc_approved );
		do_CC_OLT = FALSE;
		CC_Auth_Entered = FALSE;
	}

#ifdef	Enable_Reduce_CC_Processing_Delay
	////CC_Transaction_Retry_Request_Interval = 0; //@r
	modem_init_to_process_cc_transaction = 0; //@r reseting already in fun telit_init, this is added temporary to avoid in ME910, Need to check LE910/ME910.
#endif

	//Take care of the 3 seconds spent to display the CC screens here by reducing the parking clock
/*	if(parking_time_left_on_meter[Current_Space_Id] >= 4)
		parking_time_left_on_meter[Current_Space_Id] -= 4;
	else
		parking_time_left_on_meter[Current_Space_Id] = 0;*/

	if(parking_time_left_on_meter[Current_Space_Id] > 0)
		Parking_Clock_Refresh[Current_Space_Id] = PARKING_CLOCK_RESET_MODE;	//Avoid spending time in parking clock display here, it will be done outside

	OLT_end_time=RTC_epoch_now();
	if((uint32_t)(OLT_end_time)>(uint32_t)(OLT_start_time))
	{
		olt_time_taken=(uint32_t)(OLT_end_time)-(uint32_t)(OLT_start_time);
		Debug_Output1(0,"ONLINE OLT ELAPSED TIME:%d",olt_time_taken);
		Debug_TextOut( 0, "OLT END\r\n" );
		CC_TimeOut_RTCTimestamp = RTC_epoch_now();
		Credit_Card_processed_screen = true;
		Flash_RTC_ParkingClock_Storage(true, false); // to avoid sudden reset of meter and erasing the parking clock durango issue //vinay
	}


	if(glOLT_resp.auth_code != GPRS_OLT_GWAY_RESP_Timeout)	//Send OLT ACK only if the OLT got a response from UDP
	{
		glOLTACK_req.earned_time = glOLT_req.earned_mins;//temp_earned_time_in_seconds/60;
		memcpy(glOLTACK_req.paid_until_ts, &glOLT_req.paid_until, sizeof(glOLTACK_req.paid_until_ts));
		/*glOLTACK_req.paid_until_ts[0] = (uint8_t) (temp_tm);
		glOLTACK_req.paid_until_ts[1] = (uint8_t) (temp_tm>>8);
		glOLTACK_req.paid_until_ts[2] = (uint8_t) (temp_tm>>16);
		glOLTACK_req.paid_until_ts[3] = (uint8_t) (temp_tm>>24);*/

		glOLTACK_req.olt_success = glOLT_resp.auth_code;
		glOLTACK_req.bay_no=glOLT_req.bay_no;
		memcpy(glOLTACK_req.receipt_num, glOLT_req.ref_id, sizeof(glOLTACK_req.receipt_num));

		result = gprs_do_OLTACK(&glOLTACK_req);
		if(result == GPRS_COMMERR_NONE)
		{
			;
		}
	}

/*	//PAM bay status update for cc
	if(PAM_Baystatus_update_count <= (MAX_PAM_BSU_QUEUE-1))
	{
		tmp_currentRTC = RTC_epoch_now();
		PAM_Bay_status_data[PAM_Baystatus_update_count].txn_time = tmp_currentRTC; //RTC_epoch_now();
		PAM_Bay_status_data[PAM_Baystatus_update_count].txn_type = Tx_Credit_Card;
		PAM_Bay_status_data[PAM_Baystatus_update_count].expiry_time = (tmp_currentRTC + parking_time_left_on_meter[Current_Space_Id]);
		PAM_Bay_status_data[PAM_Baystatus_update_count].amount_cents = glOLT_req.amount;
		PAM_Bay_status_data[PAM_Baystatus_update_count].purchased_duration = glOLT_req.earned_mins;
		if(pre_existing_seconds > 0)
		{
			PAM_Bay_status_data[PAM_Baystatus_update_count].topup = 1;
		}
		else
		{
			PAM_Bay_status_data[PAM_Baystatus_update_count].topup = 0;
		}

		PAM_Baystatus_update_count++;
	}*/
}

/**************************************************************************/
//! Start OLT
//! \param void
//! \return void
/**************************************************************************/
void initiate_OLT()
{
	uint16_t result; //, delay=0;

	memset( &glOLT_resp, 0, sizeof( glOLT_resp ));
	glOLT_resp.auth_code = GPRS_OLT_GWAY_RESP_Waiting;

	result = gprs_do_OLT( & glOLT_req, & glOLT_resp );
	if(result == GPRS_COMMERR_NONE)
	{
		;
	}
	else
	{
		//Debug_TextOut(0,"No Space for OLT in RAM");
		CC_Payment_Declined_Screen();
		do_CC_OLT = FALSE;
		CC_Auth_Entered = FALSE;
		smart_card_mode = FALSE;
	}
}

/**************************************************************************/
//! Validate CivicSmart card with track1 data
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t Validate_CivicSmart_Card_with_track1()
{
	//if(T2_Faw_flag==1)
	//return false;
	uint8_t ccard_data_buf[16], card_serial_number_to_report[5];
	char *serial_no_index = 0;
	uint8_t ccard_data_len=0;
	uint8_t card_valid=false;
	//uint8_t i=0,j=0;
	//uint16_t allowed_trans_types=(glSystem_cfg.allowed_trans_types<<8)|(glSystem_cfg.allowed_trans_types>>8);
	memset(ccard_data_buf,0,sizeof(ccard_data_buf));

	if ((strncmp((char*)ccard_data_buf,CFG_REINO_TEST_CARD,4)==0)&&(ccard_data_len == 16))
	{
		//Debug_TextOut( 0, "REINO TEST Card validated" );
		card_valid = true;
	}
	else if (strstr((char*)fTrack1String,CFG_MAG_TECH_CARD)!=0)//&&(ccard_data_len == 16))
	{
		serial_no_index = strstr((char*)fTrack1String,CFG_MAG_TECH_CARD);
		memcpy(card_serial_number_to_report, serial_no_index+12, 4);
		card_serial_number_to_report[4] = '\0';
		Maintenance_Log.TECH_CARD_SL_NO = atol((const char *)card_serial_number_to_report);
		Debug_TextOut( 0, "TECHNICIAN Card validated" );
		card_valid = true;
		mag_technician_card = TRUE;
	}
	else if (strstr((char*)fTrack1String,CFG_MAG_COLL_CARD)!=0)//&&(ccard_data_len == 16))
	{
		serial_no_index = strstr((char*)fTrack1String,CFG_MAG_COLL_CARD);
		memcpy(card_serial_number_to_report, serial_no_index+12, 4);
		card_serial_number_to_report[4] = '\0';
		Maintenance_Log.TECH_CARD_SL_NO = atol((const char *)card_serial_number_to_report);
		Debug_TextOut( 0, "COLLECTION Card validated" );
		card_valid = true;
		mag_collection_card = TRUE;
	}

	return  card_valid;
}

/**************************************************************************/
//! Validate CivicSmart card with track2 data
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t Validate_CivicSmart_Card_with_track2()//1909
{
	uint8_t ccard_data_buf[16], card_serial_number_to_report[5];
	char *serial_no_index = 0;
	uint8_t ccard_data_len=0;
	uint8_t card_valid=false;
	//uint8_t i=0,j=0;
	//uint16_t allowed_trans_types=(glSystem_cfg.allowed_trans_types<<8)|(glSystem_cfg.allowed_trans_types>>8);
	//if(T1_Faw_flag==1)
	//return false;
	memset(ccard_data_buf,0,sizeof(ccard_data_buf));

	if ((strncmp((char*)ccard_data_buf,CFG_REINO_TEST_CARD,4)==0)&&(ccard_data_len == 16))
	{
		//Debug_TextOut( 0, "REINO TEST Card validated" );
		card_valid = true;
	}
	else if (strstr((char*)fTrack2String,CFG_MAG_TECH_CARD)!=0)//&&(ccard_data_len == 16))
	{
		serial_no_index = strstr((char*)fTrack2String,CFG_MAG_TECH_CARD);
		memcpy(card_serial_number_to_report, serial_no_index+12, 4);
		card_serial_number_to_report[4] = '\0';
		Maintenance_Log.TECH_CARD_SL_NO = atol((const char *)card_serial_number_to_report);
		Debug_TextOut( 0, "TECHNICIAN Card validated" );
		card_valid = true;
		mag_technician_card = TRUE;
	}
	else if (strstr((char*)fTrack2String,CFG_MAG_COLL_CARD)!=0)//&&(ccard_data_len == 16))
	{
		serial_no_index = strstr((char*)fTrack2String,CFG_MAG_COLL_CARD);
		memcpy(card_serial_number_to_report, serial_no_index+12, 4);
		card_serial_number_to_report[4] = '\0';
		Maintenance_Log.TECH_CARD_SL_NO = atol((const char *)card_serial_number_to_report);
		Debug_TextOut( 0, "COLLECTION Card validated" );
		card_valid = true;
		mag_collection_card = TRUE;
	}

	return  card_valid;
}

/**************************************************************************/
//! Initiate EMV Transaction
//! \param void
//! \return void
/**************************************************************************/
void StartTrnsaction()
{
	RespIndex=0;
	uint16_t i = 0,j=0;;
	uint16_t len = 0;
	uint8_t Year=0,Month=0;
	IdtechTXFormat(StartTransaction);  //Start transaction command will give Six responses.
	DelayMs_2(4000);
	memset(CardData,0,sizeof(CardData));

	if(RespIndex == 6)
	{
		for (i = 0; i <= RData[1]; i++)
		{
			if(RData[i] == 0x57)//OX57  start of T2 data TLV Command.
				break;
		}

		i = i+1;  //This is due to break statment above because variable "i" will not get incremented.
		len = RData[i++];
		//Debug_Output1(0," LEN-%d", len);
		for(j = 0; j < len; j++)  //This for loop is to get T2 data.
		{
			CardData[j] = RData[i++];
			//Debug_Output1(0," RD-CardData %d", CardData[j]);
		}
		CardDataEncT2[0] = ';';  //start sentinal

		//Debug_Output1(0," CardDataEncT2 %h", CardDataEncT2[0]);

		i = 1;
		for(j = 0; j < len; j++)   //card data converted to ascii character
		{
			CardDataEncT2[i++] = (((CardData[j] & 0xf0 ) >> 4) + 48);
			CardDataEncT2[i++] = ((CardData[j] & 0x0f ) + 48 );
			/*if((CardData[j]&0xd0))
				CardDataEncT2[j+1]=(0xDF&CardData[j]);//Changing "d" to "D"
			CardDataEncT2[j+1]=CardData[j];*/
		}
		CardDataEncT2[i] = '?';  //End sentinal

		for (i = 0; i <= RData[1]; i++)
		{
			if(RData[i] == 0x5F)
				break;
		}

		for (i = i+1; i <= RData[1]; i++)
		{
			if(RData[i] == 0x24)
				break;
		}

		T2_Faw_flag = 1;
		Year = RData[i+2];
		Year = ((Year/16)*10) + (Year%16);
		//Debug_Output1(0,"Year %d", Year);
		Month=RData[i+3];
		Month = ((Month/16)*10) + (Month%16);
		//Debug_Output1(0,"Month %d", Month);

		for( i = 0; i < (len*2) + 2 ;i++)
		{
			fTrack2String[i] = CardDataEncT2[i];
			//Debug_Output1(0," ID-fTrack2String %d", fTrack2String[i]);
		}

		glCCexpiry_date[0] = Year;
		glCCexpiry_date[1] = Month;

		if( check_card_read_ok(TRUE) )
		{
			//Debug_TextOut( 0, "IDTech Card Read2" );
			//Start CC TimeOut Time
			CC_TimeOut_RTCTimestamp = RTC_epoch_now();
			//cc_timeout = TRUE;
			check_card_read = TRUE;
			ccard_dataread_flag = TRUE;
			card_removed_flag = TRUE;
			Max_Amount_In_current_rate = FALSE;
			smart_card_mode = false;
			glCurrent_amount = glSystem_cfg.Card_default_amount;
			ShowAmount_Screen(glCurrent_amount, 0,TRUE);
		}
		else
		{
			//Debug_TextOut( 0, "IDTech Card Read3" );
			CC_Card_Expired_Screen();
			push_event_to_cache( UDP_EVTTYP_EXPIRED_CARD_SWIPE );
			DelayMs(3000);
			Idle_Screen();
		}
		memset(RData,0,sizeof(RData));
		//AuthenticateTranscation();
	}
	else
	{
		T2_Faw_flag=0;
		ccard_dataread_flag=FALSE;
		card_removed_flag = FALSE;
		check_card_read = FALSE;
		////CardReadFail_Screen();
		Debug_TextOut( 0, "IDTech Card Read Fail" );
		push_event_to_cache( UDP_EVTTYP_CARD_SWITCH_PRESS );
		push_event_to_cache( UDP_EVTTYP_CARD_READ_FAIL );
		DelayMs(3000);
		if(CardReadFail_Screen()) return;
		Idle_Screen();
		return;
	}
}

/**************************************************************************/
//! IDTech TX data format
//! \param uint16_t length
//! \return void
/**************************************************************************/
void IdtechTXFormat(uint16_t len,...)
{
	va_list arguments;
	uint8_t data[500];
	uint8_t Eof;
	uint16_t CTC=0,LTC=0;
	uint8_t x=0;
	/* Initializing arg
	 * arguments to store all values after num */
	len =(len & 0xff)+((len>>8)& 0xff);
	va_start(arguments,len);
	data[0]= 0x02;
	data[1]= (len & 0xff);
	data[2]= ((len>>8) & 0xff);
	for (x = 0;x<len;x++)
	{
		data[x+3] = va_arg (arguments,uint8_t );
		LTC=(LTC^data[x+3]);    //Exor data
		CTC=(CTC+data[x+3]);	//Add all the data
	}
	x=len+3;
	data[x++] = (LTC & 0xff);
	data[x++] = (CTC & 0xff);
	data[x]	= 0x03;
	Eof=x;
	va_end (arguments );                  // Cleans up the list
	TxData(data,Eof);
}

/**************************************************************************/
//! IDTech TX data
//! \param pointer to the buffer to send
//! \param length of data to send
//! \return void
/**************************************************************************/
void TxData(uint8_t *data,uint16_t Len)
{
	uint8_t debug_uart_bytes_counter=0;
	TData=data;
	Eof=Len+1;
	RXIndex=0;//reset the receive buffer index;
	memset(RData,0,sizeof(RData));
	for(debug_uart_bytes_counter=0; debug_uart_bytes_counter<Eof; debug_uart_bytes_counter++)
	{
		UART_transmitData(EUSCI_A3_BASE, data[debug_uart_bytes_counter]);
	}
}

/**************************************************************************/
//! Authenticate EMV Transcation
//! \param void
//! \return void
/**************************************************************************/
void AuthenticateTranscation()
{
#if 0
	RespIndex=0;
	IdtechTXFormat( AuthenticateTransaction	);
	DelayMs(1500);
	if(RespIndex==4)
	{
		memset(RData,0,sizeof(RData));
		CompleteTranscation();
	}
#endif
}

/**************************************************************************/
//! Complete EMV Transcation
//! \param void
//! \return void
/**************************************************************************/
void CompleteTranscation()
{
#if 0
	RespIndex=0;
	IdtechTXFormat(CompleteTransaction);
	DelayMs(800);
	if(RespIndex==3)
	{
		memset(RData,0,sizeof(RData));
	}
#endif
}

/**************************************************************************/
//! Cancel EMV Transcation
//! \param void
//! \return void
/**************************************************************************/
void CancelTranscation()
{
	RespIndex=0;
	IdtechTXFormat(CancelTransaction);
	DelayMs(200);
	if(RespIndex==3)
	{
		memset(RData,0,sizeof(RData));
	}
}

/**************************************************************************/
//! Get ATR from Card
//! \param void
//! \return uint8_t IDTECH ATR check result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t  GetAtr()
{
	RespIndex=0;
	IdtechTXFormat(PowerOff);
	DelayMs_1(400);
	RespIndex=0;
	memset(RData, 0, sizeof(RData));
	RXIndex=0;
	ReadCard_Screen();
	IdtechTXFormat(PowerOn);
	DelayMs_1(400);
	if (0 == GPIO_getInputPinValue(MAGTEK_INTR_PORT, MAGTEK_INTR_PIN))
	{
		return 0;
	}

	if(RespIndex==1)
	{
		//Debug_Output6(0, "start:%x, %x, %x, %x, %x, %x", RData[4], RData[5], RData[6], RData[7], RData[8], RData[9]);
		//if((RData[4]==0x8b) && (RData[5]==0x10) && (RData[6]==0x8e) && (RData[7]==0xb0)) //MeterCash Card
		if((RData[4]==0x3b) && (RData[5]==0x02) && (RData[6]==0x53) && (RData[7]==0x01)) //GemClub Card
		{
			Debug_TextOut( 0, "Collection Card: Meter Audit Done");
			IDTECH_AUDIT_CARD = TRUE;
			smart_card_mode = FALSE;
			Tech_menu = FALSE;
			push_event_to_cache( UDP_EVTTYP_COLLECTION_EVT );	//Audit event raised; server will take care of collection report
			AUDIT_Screen();
			DelayMs(3000);
			RespIndex=0;
			memset(RData, 0, sizeof(RData));
			RXIndex=0;
			return 1;
		}
		//else if((RData[4]==0x3b) && (RData[5]==0x02) && (RData[6]==0x53) && (RData[7]==0x01)) //GemClub Card
		/*else if((RData[4]==0x8b) && (RData[5]==0x10) && (RData[6]==0x8e) && (RData[7]==0xb0)) //MeterCash Card
		{
			Debug_TextOut( 0, "Technician Card: Enter Tech Menu");
			Tech_menu=true;
			IDTECH_AUDIT_CARD = TRUE;
			smart_card_mode = FALSE;
			Debug_TextOut( 0, "LNG is in tech mode" );
			push_event_to_cache( UDP_EVTTYP_TECH_MENU_ACCESS );//09-04-12
			TechMenu_Display_Main_Menu();
			RespIndex=0;
			memset(RData, 0, sizeof(RData));
			RXIndex=0;
			return 1;
		}*/
		else if((RData[4]==0x3b) && (RData[3]==0x06)) //Chip Credit Card
		{
			//Debug_TextOut(0, "IDtech ATR Checked");
			//Debug_Output6(0, "start:%d, %d, %d, %d, %d, %d", RData[4], RData[5], RData[6], RData[7], RData[8], RData[9]);
			RespIndex=0;
			return 1;
		}
	}
	return 0;
}

/**************************************************************************/
//! Initiate EMV Transaction
//! \param void
//! \return uint8_t IDTECH Serial Number check result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t Start()
{
	MAG_1_POWER_ON();
	IDTech_POWER_ON();
	RespIndex=0;
	IdtechTXFormat(GetSerialNumber);
	DelayMs(2000);
	//if(Connected_Card_reader == GEM_CLUB_READER)
	//MAG_1_POWER_OFF();
	//IDTech_POWER_OFF();
	//Debug_Output6(0, "start:%d, %d, %d, %d, %d, %d", RData[0], RData[1], RData[2], RData[3], RespIndex, RXIndex);
	if(RespIndex==1)
	{
		//Debug_TextOut(0, "IDtech Serial Checked");
		return 1;
	}
	return 0;
}

/**************************************************************************/
//! Load default values to start EMV Transaction
//! \param void
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t LoadDefaultValue()
{
	RespIndex=0;
	IdtechTXFormat(DefaultAPID1);
	DelayMs(200);
	IdtechTXFormat(DefaultAPID2);
	DelayMs(200);
	IdtechTXFormat(DefaultAPID3);
	DelayMs(200);
	IdtechTXFormat(DefaultAPID4);
	DelayMs(200);
	IdtechTXFormat(DefaultAPID5);
	DelayMs(200);
	IdtechTXFormat(DefaultAPID6);
	DelayMs(200);
	if(RespIndex>=1)
	{
		RespIndex=0;
		IdtechTXFormat(SetTerminalData);
		DelayMs(200);
	}
	if(RespIndex==1)
	{
		RespIndex=0;
		IdtechTXFormat(SetCAPublicKey);

#if 0
		IdtechTXFormat(DefaultCAPublicKey1);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey2);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey4);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey5);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey6);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey7);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey8);
		DelayMs(300);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey10);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey11);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey12);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey13);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey14);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey15);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey16);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey17);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey18);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey19);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey20);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey21);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey22);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey23);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey24);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey25);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey26);
		DelayMs(200);
		/*if(RespIndex==1)
							RespIndex=0;*/
		IdtechTXFormat(DefaultCAPublicKey27);
		DelayMs(200);
#endif

		/*if(RespIndex==1)
							RespIndex=0;*/
	}
	//Debug_Output6(0, "start:%d, %d, %d, %d, %d, %d", RData[0], RData[1], RData[2], RData[3], RespIndex, RXIndex);

	if(RespIndex==1)
	{
		//Debug_TextOut(0, "IDTECH Init Done");
		RespIndex=0;
		return 1;
	}
	return 0;
}

/**************************************************************************/
//! Parse the Pay By Cell payment received from Server, Calculate paid time
//! and add to parking clock
//! \param void
//! \return void
/**************************************************************************/
void Parse_PBC_Payment()
{
    //uint32_t temp_earned_time_seconds = 0, pre_existing_seconds = 0;
    uint32_t temp_parking_clock_display = 0;

	pbc_expiry_time_recvd = (((0xffffffff & PBC_expt_rcvd[3]) << 24)
							| ((0xffffffff & PBC_expt_rcvd[2]) << 16)
							| ((0xffffffff & PBC_expt_rcvd[1]) << 8)
							| (0xffffffff & PBC_expt_rcvd[0]));

	pbc_amount_recvd 	  = (((0xFF & PBC_amnt_rcvd[1]) << 8)
							| (0xFF & PBC_amnt_rcvd[0]));

	//pre_existing_seconds = (parking_time_left_on_meter[Current_bayStatus]);
	//if((parking_time_left_on_meter[Current_bayStatus]%60)>0)
		//pre_existing_minutes++;
	temp_parking_clock_display = get_earned_seconds(pbc_amount_recvd, true, true);
	//temp_earned_time_seconds = temp_parking_clock_display - pre_existing_seconds;

	parking_time_left_on_meter[Current_bayStatus] = temp_parking_clock_display;
	if(parking_time_left_on_meter[Current_bayStatus] > 0)
		Parking_Clock_Refresh[Current_bayStatus] = PARKING_CLOCK_RESET_MODE;

	in_prepay_parking[Current_bayStatus] = FALSE;

	LCD_Backlight(TRUE);
	//Add LCD message for PBC payment
	liberty_sprintf((char *)rx_ans,"%s", "  PAID BY CELL");
	diag_text_Screen( rx_ans, TRUE, FALSE );

/*	//PAM bay status update for PBC
	if(PAM_Baystatus_update_count <= (MAX_PAM_BSU_QUEUE-1))
	{
		PAM_Bay_status_data[PAM_Baystatus_update_count].txn_time = RTC_epoch_now();
		PAM_Bay_status_data[PAM_Baystatus_update_count].txn_type = Tx_Pay_By_Phone;
		PAM_Bay_status_data[PAM_Baystatus_update_count].expiry_time = pbc_expiry_time_recvd;
		PAM_Bay_status_data[PAM_Baystatus_update_count].amount_cents = pbc_amount_recvd;
		PAM_Bay_status_data[PAM_Baystatus_update_count].purchased_duration = temp_earned_time_seconds/60;
		PAM_Bay_status_data[PAM_Baystatus_update_count].topup = 0;
		PAM_Baystatus_update_count++;
	}*/
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

