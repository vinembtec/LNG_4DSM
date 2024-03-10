//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_filedownload.c
//
//****************************************************************************

//*****************************************************************************
//
//!
//! \addtogroup LibG2_filedownload_api
//! @{
//
//*****************************************************************************
#include "LibG2_filedownload.h"

/*
***********************************************************************
*Imported Functions
***********************************************************************
*/

/*
***********************************************************************
*Local Functions
***********************************************************************
*/

/*
***********************************************************************
*Imported Global Variables
***********************************************************************
*/
extern uint8_t      				glDFG_response[GPRS_GEN_BUF_SZ_VLARGE], request_events[MAX_PACKET_LENGTH];
extern GPRSSystem   				glSystem_cfg;
extern uint8_t         				glTemp_buf[GPRS_GEN_BUF_SZ_SMALL];//24-05-12
extern FlashFieActvnTblEntry   		flash_file_actvn_tbl[ FILE_TYPE_TOTAL_FILES ];//24-05-12

extern uint8_t              		rx_ans[ GPRS_GEN_BUF_SZ_SMALL ];
extern volatile uint8_t         	Asynchronous_FD;
extern GPRS_General_Sync_Response   gprs_general_sync_response;
extern uint16_t                		glMdmUart_bytes_recvd;
extern uint8_t                 		glMdmUart_recv_buf[ GPRS_GEN_BUF_SZ_VLARGE ];
extern GPRS_General_Sync_Request	gprs_general_sync_request;
extern uint8_t 						flash_data_read_buffer[ FLASH_DATA_READ_BUF_SIZE ];
/*
***********************************************************************
*Exported Global Variables
***********************************************************************
*/
uint32_t     			        	config_WSAD, mpb_code_WSAD;
uint32_t     						config_RSAD, mpb_code_RSAD;
GPRS_FD_File    					glAfm_response_p[FFACT_TOTAL_FILES+1]; //16/09/2015:LOB
uint8_t         					file_sizearr[4],CFG_file_sizearr[4];//200312
uint8_t         					fd_start=false;    //19-03-12
uint32_t        					received_crc=0, dfg_crc=0/*,MC_Binary_File_ID=0*/; //25-01-13
uint8_t         					Chunk_FileType = 0;
uint32_t        					Chunk_FileOffset = 0;
uint16_t							Downloaded_chunks = 0;
uint8_t  							File_count = 0;
uint16_t        					DFG_filled_resp_len = 0;
Chunk_DNLD_Status 					chunk_DNLD_status;
uint8_t 							REINITIATE_FD = FALSE;
/*
***********************************************************************
*Local Constants
***********************************************************************
*/
static const char * 				EL_METERFILEINFO    = "MeterFileInformation";
static const char * 				EL_METER            = "Meter";
static const char * 				EL_FILEINFO         = "FileInfo";
static const char * 				ATT_FILE_ID         = "ID";
static const char * 				ATT_FILE_TYPE       = "Type";
static const char * 				ATT_FILE_BYTESIZE   = "ByteSize";
static const char * 				ATT_FILE_ALTCKSUM   = "AltChkSum";
static const char * 				ATT_FILE_ACTIVATETS = "ActivateTS";

/*
 ***********************************************************************
 *Local Variables
 ***********************************************************************
 */
static char 						attval_buf[GPRS_GEN_BUF_SZ_MED];
static char 						attnm_buf[GPRS_GEN_BUF_SZ_SMALL];
static uint16_t 					FD_HEADER_LENGTH, glChunk_size;//, no_of_chunks;
/**************************************************************************/
//! Used to extract the file activation timestamp from the timestamp received
//! in General Sync response from server
//! \param uint8_t * pointer to the timestamp buffer
//! \return uint32_t file_actvn_ts
/**************************************************************************/
//24-05-12
uint32_t gprs_extract_time( uint8_t *actvn_ts_buf )
{
	time_set_t     file_actvn_ts;
	uint16_t actvn_year;

	actvn_year = ((actvn_ts_buf[0]-0x30)*1000) + ((actvn_ts_buf[1]-0x30)*100) + ((actvn_ts_buf[2]-0x30)*10) + (actvn_ts_buf[3]-0x30);

	file_actvn_ts.year = (uint8_t)(actvn_year-2000);

	file_actvn_ts.month = ((actvn_ts_buf[5] - 0x30)*10) + (actvn_ts_buf[6] - 0x30);

	file_actvn_ts.date = ((actvn_ts_buf[8] - 0x30)*10) + (actvn_ts_buf[9] - 0x30);

	file_actvn_ts.hours = ((actvn_ts_buf[11] - 0x30)*10) + (actvn_ts_buf[12] - 0x30);

	file_actvn_ts.minutes = ((actvn_ts_buf[14] - 0x30)*10) + (actvn_ts_buf[15] - 0x30);

	file_actvn_ts.seconds = ((actvn_ts_buf[17] - 0x30)*10) + (actvn_ts_buf[18] - 0x30);

	// further bytes are to indicate timezone, so avoided.
	// example of activate timestamp: "2012-05-24T12:23:00+05:30"
	return RTC_from_year(&file_actvn_ts);

}
//24-05-12

/**************************************************************************/
//! Used to extract the file information received in General Sync response
//! from server. These information is needed to start file downloads
//! \param pointer to the AFM structure where the output will be given
//! \param pointer to the file download xml received from server
//! \param pointer to the XML attributes structure
//! \param uint16_t total number of attributes to extract
//! \return void
/**************************************************************************/
static void handle_fileInfo_tag_attribs(GPRS_FD_File * afm_file_p,
		                                uint8_t      * xml_p,
                                        XMLAttrib    * attribs,
                                        int16_t        tot_attribs)
{
	int16_t a;
	int16_t len,len1;
	uint8_t * att_name_p;
	uint8_t att_nm_len, temp_array[16];
	uint8_t * att_val_p;
	uint8_t att_val_len;
	uint16_t max_val_lng = (uint16_t) (sizeof(attval_buf) - 2);
	uint16_t max_name_lng = (uint16_t) (sizeof(attnm_buf) - 2);

	for (a = 0; a < tot_attribs; a++)
	{
		att_name_p = xml_p + attribs[a].nm_st;
		att_nm_len = (uint8_t) (attribs[a].nm_end - attribs[a].nm_st + 1);
		att_val_p = xml_p + attribs[a].val_st;
		att_val_len = (uint8_t) (attribs[a].val_end - attribs[a].val_st + 1);

		len = min( att_nm_len, max_name_lng );
		strncpy(attnm_buf, (const char *) att_name_p, len);
		attnm_buf[len] = '\0';

		len = min( att_val_len, max_val_lng );
		strncpy(attval_buf, (const char *) att_val_p, len);
		attval_buf[len] = '\0';

		if (strncmp((const char *) att_name_p, ATT_FILE_ID, att_nm_len) == 0)
		{
			afm_file_p->id = atoi(attval_buf);
		}
		else if (strncmp((const char *) att_name_p, ATT_FILE_TYPE, att_nm_len) == 0)
		{
			afm_file_p->type = (uint8_t) atoi(attval_buf);
		}
		else if (strncmp((const char *) att_name_p, ATT_FILE_BYTESIZE, att_nm_len) == 0)
		{
			afm_file_p->size = atol(attval_buf);
		}
		/* else if (strncmp((const char *) att_name_p, ATT_FILE_CKSUM,	att_nm_len) == 0)
        {
			strncpy((char*) afm_file_p->chksum, (const char *) att_val_p, att_val_len);
		}*/ // not using sha1 checksum now
		else if (strncmp((const char *) att_name_p, ATT_FILE_ACTIVATETS, att_nm_len) == 0)
		{
			// convert att_val_p string to uint32
			strncpy( (char*) glTemp_buf, (char*)att_val_p, att_val_len );//24-05-12
			afm_file_p->actvn_ts = gprs_extract_time( glTemp_buf );//24-05-12
		}
		else if (strncmp((const char *) att_name_p, ATT_FILE_ALTCKSUM,	att_nm_len) == 0)
		{
			memset(temp_array,0,sizeof(temp_array));
			strncpy((char*) temp_array, (const char *) att_val_p, att_val_len);
			for(len1=0;len1<att_val_len;len1++)
			{
				if(temp_array[len1] < 0x40)
				{
					temp_array[len1] -= 0x30;
				}
				else
				{
					temp_array[len1] -= 0x37;
				}
			}

			for(len1=0;len1<att_val_len;len1++)
			{
				afm_file_p->chksum = afm_file_p->chksum<<4;
				afm_file_p->chksum |= temp_array[len1];
			}
			//afm_file_p->chksum = atol(attval_buf);
		}
	}

	return;
}

/**************************************************************************/
//! XML parser function used to extract the file information received in
//! General Sync response from server. These information is needed to start file downloads
//! \param pointer to the XML
//! \param xml element
//! \param pointer to the XML attributes structure
//! \param uint16_t total number of attributes to extract
//! \return void
/**************************************************************************/
bool AFM_XML_parser_start_element_callback(uint8_t    * xml_p,
                                           XMLElement   e,
                                           XMLAttrib  * attribs,
                                           int16_t tot_attribs)
{
	uint8_t * tag_name_p = xml_p + e.st;
	uint8_t tag_nm_len = (uint8_t) (e.nm_end - e.st + 1);

	if (strncmp((const char *) tag_name_p, EL_METERFILEINFO, tag_nm_len) == 0)
	{
		//handle_meterInfo_tag_attribs( xml_p, attribs, tot_attribs );
	}
	else if (strncmp((const char *) tag_name_p, EL_METER, tag_nm_len) == 0)
	{
		//handle_meter_tag_attribs( xml_p, attribs, tot_attribs );
	}
	else if (strncmp((const char *) tag_name_p, EL_FILEINFO, tag_nm_len) == 0)
	{
		//memset(&glAfm_response_p[File_count], 0, sizeof(glAfm_response_p[File_count]));

		handle_fileInfo_tag_attribs(&glAfm_response_p[File_count], xml_p, attribs, tot_attribs);
		File_count++;

		//        glAfm_response_p.valid = true;
		//        file_download_mgr.afm_file_idx ++;
	}

	return true;
}

/**************************************************************************/
//! XML parser internal function1
//! \param int16_t tag_start
//! \param int16_t tag_end
//! \return true
/**************************************************************************/
static bool AFM_XML_parser_end_element_callback(int16_t tag_start, int16_t tag_end)
{
	bool result = true;
	return result;
}

/**************************************************************************/
//! XML parser internal function2
//! \param int16_t start
//! \param int16_t end
//! \return true
/**************************************************************************/
static bool AFM_XML_parser_cdata_callback(int16_t start, int16_t end)
{
	bool result = true;
	return result;
}

/**************************************************************************/
//! XML parser internal function3
//! \param int16_t start
//! \param int16_t end
//! \return true
/**************************************************************************/
static bool AFM_XML_parser_commen_callback(int16_t start, int16_t end)
{
	bool result = true;
	return result;
}

/**************************************************************************/
//! Caller of the xml parser
//! \param pointer to the xml file
//! \param uint16_t xml file size
//! \return void
/**************************************************************************/
void call_xml_parse(uint8_t * xml_file, uint16_t xml_file_size)
{
	/*int16_t parse_result= */
	parse_xml(xml_file,
			xml_file_size,
			(XMLParser_StartElement_Callback) AFM_XML_parser_start_element_callback,
			(XMLParser_EndElement_Callback) AFM_XML_parser_end_element_callback,
			(XMLParser_CDATA_Callback) AFM_XML_parser_cdata_callback,
			(XMLParser_Comment_Callback) AFM_XML_parser_commen_callback);

	//    glAfm_response_p.size = 2522;
	//    glAfm_response_p.id = 8;
}

/**************************************************************************/
//! Used to write the downloaded file chunks to appropriate address in flash
//! \param void
//! \return int16_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
int16_t download_file()
{
	int16_t     result = GPRS_COMMERR_NONE;//,i;
	uint8_t     config_erase_flag   = 1;
	uint8_t     mpb_code_erase_flag = 2;

	config_WSAD   = AD_MPB_config_w+FLASH_FILE_SIZE_LEN+Chunk_FileOffset;
	mpb_code_WSAD = AD_MSP_CODE_w+Chunk_FileOffset;

	fd_start=true;

	switch(Chunk_FileType)
	{
	case FILE_TYPE_MPB_CONFIG://MPB_CONFIG://23-05-12

		if ((uint32_t)(config_WSAD + glChunk_size) < (uint32_t)(AD_MPB_config_w + (FLASH_SECTOR_SIZE * 2) ))
		{
			config_WSAD   = Flash_Write( (glDFG_response + FD_HEADER_LENGTH),glChunk_size,config_WSAD,0); //1810
		}
		break;

	case FILE_TYPE_MPB_CODE://MPB_CODE://23-05-12

		if ((uint32_t)(mpb_code_WSAD + glChunk_size) <= (uint32_t)(0x05FFFF))//25-05-12
		{
			mpb_code_WSAD = Flash_Write((glDFG_response+FD_HEADER_LENGTH),glChunk_size,mpb_code_WSAD,0);
		}
		break;

	default:
		//FFACT_TYPE=0XFF;//28-06-12
		break;
	}
	//22-04-2013:avoid copying corrupted files
	if((Chunk_FileType==FILE_TYPE_MPB_CONFIG)&&(config_WSAD==0))
	{
		{
			//Debug_TextOut(0, "FLASH-CFG SECTION E");
			config_WSAD = AD_MPB_config_w;
			Flash_Batch_Erase(config_WSAD, config_erase_flag, false);

		}
	}
	else if((Chunk_FileType==FILE_TYPE_MPB_CODE)&&(mpb_code_WSAD==0))
	{
		{
			//Debug_TextOut(0, "FLASH-MPB SECTION E");
			//fd_status = FDSTATUS_FAILED_DOWNLOAD;
			mpb_code_erase_flag = 2;
			mpb_code_WSAD = AD_MSP_CODE_w;
			Flash_Batch_Erase(mpb_code_WSAD, mpb_code_erase_flag, false);
			//break;
		}
	}


	fd_start=false;
	if(Downloaded_chunks >= gprs_general_sync_response.total_FD_chunks)
	{
		Asynchronous_FD = FALSE;
	}

	// MC_FD_COMPLETE=TRUE;//17-03-2014:hardware sensor code merge
	return result;
}

/**************************************************************************/
//! Used to validate the downloaded file chunks and write to appropriate address in flash
//! \param void
//! \return int16_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_do_dfg()
{
	GPRS_GeneralResponseHeader recvd_resp_hdr;
	int16_t gen_resp_hdr_len;
	uint8_t crc_start;
	uint8_t result = GPRS_COMMERR_BUSY;
	uint8_t * recvd_resp_p = glDFG_response;

	//if (chunk_rxd == TRUE)
	{
		//2106
		if (DFG_filled_resp_len >= glSystem_cfg.gen_resp_hdr_sz)
		{
			gen_resp_hdr_len = gprs_fill_response_hdr(recvd_resp_p,
					DFG_filled_resp_len, &recvd_resp_hdr);
			recvd_resp_p += gen_resp_hdr_len;

			if (recvd_resp_hdr.ch.type != (uint8_t) GPRS_COMM_lOB_DFG
					&& recvd_resp_hdr.ch.flag != (uint8_t) GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT
					&& DFG_filled_resp_len < (uint16_t) gen_resp_hdr_len + (uint16_t) recvd_resp_hdr.content_length)
			{
				result = GPRS_COMMERR_INVALID_RESP_HDR;
			}
			else if (recvd_resp_hdr.server_resp.txn_resp_code != 200
					|| recvd_resp_hdr.server_resp.ripnet_resp_code != 200)
			{
				result = GPRS_COMMERR_SERVER_ISSUE;
			}
			else
			{
				result = GPRS_COMMERR_NONE;
			}

/*			Debug_Output6(3, "COMM %d %d %d %d %d RESULT=%d",
					recvd_resp_hdr.ch.flag, recvd_resp_hdr.ch.type,
					recvd_resp_hdr.server_resp.txn_resp_code,
					recvd_resp_hdr.server_resp.ripnet_resp_code,
					recvd_resp_hdr.content_length, result);*/
		}
		else
		{
			result = GPRS_COMMERR_INVALID_RESP_HDR;
		}
		//16/09/2015:LOB
		if (result == GPRS_COMMERR_NONE) {
			crc_start = (uint8_t) (20 + glDFG_response[19]); //16/09/2015:5 additional bytes for offset and type

			received_crc = ((0xffffffff & glDFG_response[crc_start + 3]) << 24)
							| ((0xffffffff & glDFG_response[crc_start + 2]) << 16)
							| ((0xffffffff & glDFG_response[crc_start + 1]) << 8)
							| (0xffffffff & glDFG_response[crc_start + 0]);

			FD_HEADER_LENGTH = 30 + glDFG_response[19]; //16/09/2015:additional bytes for offset and type
			glChunk_size = (uint16_t)(glDFG_response[crc_start+8] | (glDFG_response[crc_start+9] << 8));

			if(glChunk_size > 1024)
				glChunk_size = 1024;
			dfg_crc = calculate_chksum(glDFG_response + FD_HEADER_LENGTH,glChunk_size);

			if (dfg_crc != received_crc) {
				Debug_TextOut(2, "DFG_CRC MISMATCH");
				result = GPRS_COMMERR_INTERNAL_PRCS_RESP;
			} else
			{
				Chunk_FileType = glDFG_response[10];
				Chunk_FileOffset = (uint32_t)((glDFG_response[15]&0xFFFFFFFF) | ((glDFG_response[16]&0xFFFFFFFF) << 8) | ((glDFG_response[17]&0xFFFFFFFF) << 16)| ((glDFG_response[18]&0x0FFFFFFFF) << 24));
				Update_DFG_status(Chunk_FileType,Chunk_FileOffset);
				Downloaded_chunks++;

				DFG_filled_resp_len = 0;
				if(Chunk_FileOffset == 0)
				{
					//Debug_Output1(0, "DFG Checking :%d",Downloaded_chunks);  //OTA //Vinay
					//Debug_Output1(0, "Downloaded_chunks :%d",Downloaded_chunks);  //OTA //Vinay
					//store_MBfiles_name();
				}
				download_file();//21/09/2015:LOB
				Debug_Output6(0,"Download: %x %x %x %x, %d, %d",glDFG_response[18],glDFG_response[17],glDFG_response[16],glDFG_response[15],Downloaded_chunks,Chunk_FileType); //OTA //Vinay
				//Debug_Output1(0, "DFG OK:%d",Downloaded_chunks);
			}

		}
		//16/09/2015:LOB
	}
	//chunk_rxd = FALSE;
	return result;
}

#if 0
uint8_t gprs_do_DFS(uint32_t file_id, uint32_t status) //2609:test
{
	uint8_t result = GPRS_COMMERR_BUSY;
	uint8_t * req_p = request_events;
	uint16_t req_sz;

	Debug_Output6(0, "SND DFS %d, %d", file_id, status, 0, 0, 0, 0);

	req_p += gprs_fill_request_hdr(GPRS_COMM_DFS,
			GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof(request_events), false);

	*((uint32_t*) req_p) = (uint32_t) file_id;
	req_p += sizeof(uint32_t);
	*((uint32_t*) req_p) = (uint32_t) status;
	req_p += sizeof(uint32_t);

	req_sz = (uint16_t) (req_p - request_events);

	result = gprs_do_request(request_events, req_sz );

	if (result == GPRS_COMMERR_NONE)
	{
/*		if (telit_sock_close()!= SOCERR_NONE)
		{
			result = GPRS_COMMERR_SOCK_CLOSE_ERR;
			diag_text_Screen("SocClos Fail", FALSE);
		}*/

		Debug_TextOut(0, "DFS OK");
	}
	else
		Debug_Output1(0, "DFS ERR= %d", result);

	return result;
}
#endif
//22/09/2015:LOB

/**************************************************************************/
//! Used to mark the successfully downloaded chunk index in the chunk_DNLD_status
//! structure which is used in DFS to ask for missing chunks
//! \param uint8_t file type
//! \param uint32_t Chunk_FileOffset
//! \return void
/**************************************************************************/
void Update_DFG_status(uint8_t Type,uint32_t Chunk_FileOffset)
{
	uint16_t chunk_numb;

	if((Chunk_FileOffset % 1024) !=0)
	{
		chunk_numb = (Chunk_FileOffset / 1024) + 1;
	}
	else
	{
		chunk_numb = (Chunk_FileOffset / 1024);
	}
	if(chunk_numb < MAX_FD_CHUNKS)//max for any kind of file
	{
		switch(Type)
		{
		case FILE_TYPE_MPB_CONFIG:
			chunk_DNLD_status.CFG |= (BIT0 << chunk_numb);
			break;
		case FILE_TYPE_MPB_CODE://02-04-2014:DPLIBB-629
			chunk_DNLD_status.BIN[chunk_numb/8] |= (BIT0 << (chunk_numb % 8));
			break;
		default:
			break;
		}
	}
}

/**************************************************************************/
//! This function reports the file download status to server, if there are
//! missing chunks, then requests for those and downloads those chunks
//! \param uint8_t flag to notify if it is the status to be sent at the end
//!	of download or it is a missing chunk request
//! \return uint8_t result
//! - \b Success
//! - \b Failure
/**************************************************************************/
uint8_t gprs_do_DFS_4all_files(uint8_t fd_complete) //2609:test
{
	uint8_t result = GPRS_COMMERR_BUSY,i=0/*,k=0*/,pkt_count=0;
	uint8_t * req_p = request_events;
	uint8_t *req_p_temp;
	uint16_t req_sz;

	if(fd_complete == FALSE)
		Debug_TextOut(0, "Resume FD");

	req_p += gprs_fill_request_hdr(GPRS_COMM_lOB_DFS,
			GPRS_REQ_UNENCRYPT_RESP_UNENCRYPT, req_p, sizeof(request_events), false );

	req_p_temp = req_p;

	*((uint8_t*) req_p) = 1;
	req_p += sizeof(uint8_t);

	for(i=0;i<(FILE_TYPE_TOTAL_FILES+1);i++)
	{
		if(glAfm_response_p[i].type == FILE_TYPE_MPB_CONFIG)
		{
			*((uint8_t*) req_p) = glAfm_response_p[i].type;
			req_p += sizeof(uint8_t);

			*((uint8_t*) req_p) = glAfm_response_p[i].id & 0xFF;
			req_p += sizeof(uint8_t);
			*((uint8_t*) req_p) = (glAfm_response_p[i].id >> 8) & 0xFF;
			req_p += sizeof(uint8_t);
			*((uint8_t*) req_p) = (glAfm_response_p[i].id>>16) & 0xFF;
			req_p += sizeof(uint8_t);
			*((uint8_t*) req_p) = (glAfm_response_p[i].id>>24) & 0xFF;
			req_p += sizeof(uint8_t);

			if(fd_complete == TRUE)
			{
				if(flash_file_actvn_tbl[FFACT_MPB_CONFIG].status == FF_DOWNLOADED)
				{
					*((uint8_t*) req_p) = FDSTATUS_OK;
					req_p += sizeof(uint8_t);
					*((uint8_t*) req_p) = 0;
					req_p += sizeof(uint8_t);
					*((uint8_t*) req_p) = 0;
					req_p += sizeof(uint8_t);
					*((uint8_t*) req_p) = 0;
					req_p += sizeof(uint8_t);
				}
				else
				{
					*((uint8_t*) req_p) = FDSTATUS_FAILED_DOWNLOAD;
					req_p += sizeof(uint8_t);
					*((uint8_t*) req_p) = 0;
					req_p += sizeof(uint8_t);
					*((uint8_t*) req_p) = 0;
					req_p += sizeof(uint8_t);
					*((uint8_t*) req_p) = 0;
					req_p += sizeof(uint8_t);
				}
				*((uint8_t*) req_p) = 0;
				req_p += sizeof(uint8_t);
				//break;
			}
			else
			{
				*((uint8_t*) req_p) = 0;
				req_p += sizeof(uint8_t);
				*((uint8_t*) req_p) = 0;
				req_p += sizeof(uint8_t);
				*((uint8_t*) req_p) = 0;
				req_p += sizeof(uint8_t);
				*((uint8_t*) req_p) = 0;
				req_p += sizeof(uint8_t);

				*((uint8_t*) req_p) = sizeof(chunk_DNLD_status.CFG);
				req_p += sizeof(uint8_t);

				*((uint8_t*) req_p) = chunk_DNLD_status.CFG;
				req_p += sizeof(uint8_t);
			}
			pkt_count++;

		}
		else if(glAfm_response_p[i].type == FILE_TYPE_MPB_CODE)
		{
			*((uint8_t*) req_p) = glAfm_response_p[i].type;
			req_p += sizeof(uint8_t);

			*((uint8_t*) req_p) = glAfm_response_p[i].id & 0xFF;
			req_p += sizeof(uint8_t);
			*((uint8_t*) req_p) = (glAfm_response_p[i].id >> 8) & 0xFF;
			req_p += sizeof(uint8_t);
			*((uint8_t*) req_p) = (glAfm_response_p[i].id>>16) & 0xFF;
			req_p += sizeof(uint8_t);
			*((uint8_t*) req_p) = (glAfm_response_p[i].id>>24) & 0xFF;
			req_p += sizeof(uint8_t);

			if(fd_complete == TRUE)
			{
				if(flash_file_actvn_tbl[FFACT_MPB_CODE].status == FF_DOWNLOADED)
				{
					*((uint8_t*) req_p) = FDSTATUS_OK;
					req_p += sizeof(uint8_t);
					*((uint8_t*) req_p) = 0;
					req_p += sizeof(uint8_t);
					*((uint8_t*) req_p) = 0;
					req_p += sizeof(uint8_t);
					*((uint8_t*) req_p) = 0;
					req_p += sizeof(uint8_t);
				}
				else
				{
					*((uint8_t*) req_p) = FDSTATUS_FAILED_DOWNLOAD;
					req_p += sizeof(uint8_t);
					*((uint8_t*) req_p) = 0;
					req_p += sizeof(uint8_t);
					*((uint8_t*) req_p) = 0;
					req_p += sizeof(uint8_t);
					*((uint8_t*) req_p) = 0;
					req_p += sizeof(uint8_t);
				}
				*((uint8_t*) req_p) = 0;
				req_p += sizeof(uint8_t);
				//break;
			}
			else
			{
				*((uint8_t*) req_p) = 0;
				req_p += sizeof(uint8_t);
				*((uint8_t*) req_p) = 0;
				req_p += sizeof(uint8_t);
				*((uint8_t*) req_p) = 0;
				req_p += sizeof(uint8_t);
				*((uint8_t*) req_p) = 0;
				req_p += sizeof(uint8_t);

				*((uint8_t*) req_p) = sizeof(chunk_DNLD_status.BIN);
				req_p += sizeof(uint8_t);

				memcpy(req_p,chunk_DNLD_status.BIN,sizeof(chunk_DNLD_status.BIN));
				req_p += (uint8_t)(sizeof(chunk_DNLD_status.BIN));
			}
			pkt_count++;
		}
	}

	*((uint8_t*)req_p_temp)     = pkt_count;   //diag_message_length assigned
	req_p_temp                 +=sizeof( uint8_t );

	memset(glDFG_response,0,GPRS_GEN_BUF_SZ_VLARGE);
	memset(glMdmUart_recv_buf,0,GPRS_GEN_BUF_SZ_VLARGE);
	glMdmUart_bytes_recvd = 0;

	req_sz = (uint16_t) (req_p - request_events);
	if(fd_complete == false)
	{
		result = do_request_internal(request_events, req_sz, true, true);

		if (result != GPRS_COMMERR_NONE)
		{
			Debug_Output1(0, "FD ERR= %d", result);
			DelayMs(100);

			mdm_get_local_ip_port_for_FD_OTA();
		}
	}
	else
	{
		result = gprs_do_request(request_events, req_sz );
	}

/*	if (result == GPRS_COMMERR_NONE)
	{
		if (telit_sock_close()!= SOCERR_NONE)
		{
			result = GPRS_COMMERR_SOCK_CLOSE_ERR;
			diag_text_Screen("SocClos Fail", FALSE);
		}

		Debug_TextOut(0, "DFS OK");
	}
	else
		Debug_Output1(0, "DFS ERR= %d", result);*/

	return result;
}
//22/09/2015:LOB

/**************************************************************************/
//! This function is used to validate the downloaded file at the end of
//! downloading all chunks. It validates the received file CRC with the CRC
//! as sent by the Server in the General Sync response before the start of
//! the file download. Then marks the comparison result in the File activation table.
//! \param void
//! \return void
/**************************************************************************/
void validate_asynchronous_FD()
{
	uint8_t i=0;

	Flash_Batch_Flag_Read();

	for(i=0;i<(FILE_TYPE_TOTAL_FILES+1);i++)
	{
		if(glAfm_response_p[i].type == FILE_TYPE_MPB_CONFIG)
		{
			gprs_general_sync_request.MPB_CFG_CRC = Calculate_File_CRC_4mflash(FILE_TYPE_MPB_CONFIG);
			if(glAfm_response_p[i].chksum == gprs_general_sync_request.MPB_CFG_CRC)
			{
				flash_file_actvn_tbl[FFACT_MPB_CONFIG].type 			= glAfm_response_p[i].type;
				flash_file_actvn_tbl[FFACT_MPB_CONFIG].status 			= FF_DOWNLOADED;
				flash_file_actvn_tbl[FFACT_MPB_CONFIG].actvn_try_count  = 0;
				flash_file_actvn_tbl[FFACT_MPB_CONFIG].id 				= glAfm_response_p[i].id;
				flash_file_actvn_tbl[FFACT_MPB_CONFIG].actvn_ts 		= glAfm_response_p[i].actvn_ts;
				flash_file_actvn_tbl[FFACT_MPB_CONFIG].size 			= glAfm_response_p[i].size;
			}
			else
			{
				REINITIATE_FD = TRUE;
				flash_file_actvn_tbl[FFACT_MPB_CONFIG].status 			= FF_UNKNOWN;
				Debug_TextOut(0,"File CRC Mismatch");
			}
		}
		else if(glAfm_response_p[i].type == FILE_TYPE_MPB_CODE)
		{
			gprs_general_sync_request.MPB_BIN_CRC = Calculate_File_CRC_4mflash(FILE_TYPE_MPB_CODE);
			if(glAfm_response_p[i].chksum == gprs_general_sync_request.MPB_BIN_CRC)
			{
				flash_file_actvn_tbl[FFACT_MPB_CODE].type 			= glAfm_response_p[i].type;
				flash_file_actvn_tbl[FFACT_MPB_CODE].status 			= FF_DOWNLOADED;
				flash_file_actvn_tbl[FFACT_MPB_CODE].actvn_try_count  = 0;
				flash_file_actvn_tbl[FFACT_MPB_CODE].id 				= glAfm_response_p[i].id;
				flash_file_actvn_tbl[FFACT_MPB_CODE].actvn_ts 		= glAfm_response_p[i].actvn_ts;
				flash_file_actvn_tbl[FFACT_MPB_CODE].size 			= glAfm_response_p[i].size;
			}
			else
			{
				REINITIATE_FD = TRUE;
				flash_file_actvn_tbl[FFACT_MPB_CODE].status 			= FF_UNKNOWN;
				Debug_TextOut(0,"File CRC Mismatch");
			}
		}
	}
	Flash_Batch_Flag_Write();
}

/**************************************************************************/
//! This function is used before start of file download after receiving a
//! General sync response. It prepares the external flash for file download by
//! erasing the required sectors in flash
//! \param void
//! \return void
/**************************************************************************/
void Prepare_Flash_4FD()
{
	int loop;
	uint32_t Erase_address;
	config_WSAD   = AD_MPB_config_w;
	mpb_code_WSAD = AD_MSP_CODE_w;
	//config_RSAD   = AD_MPB_config_r;
	mpb_code_RSAD = AD_MSP_CODE_r;

	uint8_t     config_erase_flag   = 1;
	uint8_t     mpb_code_erase_flag = 2;

	for(loop=0;loop<(FILE_TYPE_TOTAL_FILES+1);loop++)
	{
		switch( glAfm_response_p[loop].type)
		{

		case  FILE_TYPE_MPB_CONFIG:
			CFG_file_sizearr[0] = (uint8_t) (glAfm_response_p[loop].size >> 24);
			CFG_file_sizearr[1] = (uint8_t) (glAfm_response_p[loop].size >> 16);
			CFG_file_sizearr[2] = (uint8_t) (glAfm_response_p[loop].size >> 8);
			CFG_file_sizearr[3] = (uint8_t) (glAfm_response_p[loop].size);

			config_WSAD = Flash_Write( CFG_file_sizearr, FLASH_FILE_SIZE_LEN,
					config_WSAD,config_erase_flag);
			break;
		case  FILE_TYPE_MPB_CODE:
			Erase_address = AD_MSP_CODE_w;
			//Debug_TextOut(2,"Flash Erased FD");
			Flash_Batch_Erase( Erase_address, mpb_code_erase_flag, false );
			break;

		default:
			break;
		}
	}
}


/**************************************************************************/
//! This function is used to read the file from flash and calculate 32-bit CRC
//! \param file type
//! \return uint32_t calculated CRC
/**************************************************************************/
uint32_t Calculate_File_CRC_4mflash(uint8_t file_type)
{
	uint32_t CRC = 0XFFFFFFFF,File_Size = 0,offset = 0, bytes_left = 0,Start_address = 0;
	uint16_t i = 0,glChunk_size = 0,no_of_chunks;

	switch(file_type)
	{
	case FILE_TYPE_MPB_CONFIG:
		Start_address   = AD_MPB_config_w;
		Start_address   = Flash_Read(Start_address, 4);
		File_Size 	    = (uint32_t)(((flash_data_read_buffer[0]&0xffffffff)<<24) | ((flash_data_read_buffer[1]&0xffffffff)<<16) | ((flash_data_read_buffer[2]&0xffffffff)<<8) | ((flash_data_read_buffer[3])&0xffffffff));
		if(File_Size > sizeof( glSystem_cfg ))
		{
			File_Size = 0;
		}
		break;

	case FILE_TYPE_MPB_CODE:
		Start_address = AD_MSP_CODE_r;
		File_Size 			= MPB_CODE_SIZE;
		break;

	default:
		CRC = 0;
		break;
	}
	//TO CALCULATE CRC
	if((CRC!=0) &&(File_Size!=0xFFFFFFFF) && (File_Size!=0))
	{
		Debug_Output1(0,"File Size:%ld",File_Size);
		offset=0;
		CRC=0;
		glChunk_size  = sizeof(flash_data_read_buffer);
		if(File_Size> (sizeof(flash_data_read_buffer)))
			no_of_chunks = (uint16_t) (File_Size/glChunk_size)+1;
		else
			no_of_chunks = 1;
		for (i = 0; i < no_of_chunks; i++)
		{
			bytes_left = File_Size - offset;

			if(bytes_left <glChunk_size)
			{
				glChunk_size = (uint16_t) bytes_left;
			}
			Start_address=Flash_Read(Start_address, glChunk_size);
			CRC = calc_crc32(CRC, flash_data_read_buffer, glChunk_size);
			//Debug_Output6(0,"CRC%d:%x %x %x %x %d",i,((CRC>>24)& 0xFF),((CRC>>16)& 0xFF),((CRC>>8) & 0xFF),(CRC & 0xFF),0);
			offset += glChunk_size;
		}
		Debug_Output6(0,"File CRC:%x %x %x %x %d %d",((CRC>>24)& 0xFF),((CRC>>16)& 0xFF),((CRC>>8) & 0xFF),(CRC & 0xFF),0,0);
	}


	//Debug_Output6(0,"File CRC:%x %x %x %x %d %d",((CRC>>24)& 0xFF),((CRC>>16)& 0xFF),((CRC>>8) & 0xFF),(CRC & 0xFF),0,0);
	return CRC;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

