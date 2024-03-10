//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_otap.c
//
//****************************************************************************

#include "LibG2_otap.h"

extern volatile uint8_t 				current_rate_index;
extern volatile uint8_t					SLEEP_ENABLE;
extern FlashFieActvnTblEntry   			flash_file_actvn_tbl[ FILE_TYPE_TOTAL_FILES ];
extern uint32_t 						Sync_status;
extern uint32_t							config_RSAD, config_WSAD;
extern uint8_t 							DLST_FLAG_CHECK;
extern uint8_t 							glDLST_FLAGS[10];
//extern uint8_t         					glDLST_month, glDLST_day, glDLST_hour;//not used in this program //vinay
//extern uint16_t        					glDLST_year;//not used in this program //vinay
//extern uint16_t 						min_calculated_amount, max_calculated_amount;	//01-06-12//not used in this program //vinay
extern GPRSSystem  						glSystem_cfg;
extern uint8_t 							flash_data_read_buffer[ FLASH_DATA_READ_BUF_SIZE ];
extern uint8_t 							Debug_Verbose_Level, activate_mpb_binary;
//extern uint32_t  						Max_parking_time;
//extern uint8_t                     		Flash_Operation;//not using in this program //vinay
extern uint8_t     						rx_ans[ GPRS_GEN_BUF_SZ_SMALL ];
//extern uint16_t 						Current_Space_Id; //not used in this program //vinay
extern RTC_C_Calendar 					rtc_c_calender;
extern uint8_t							Meter_Reset;
//extern RTC_ParkingClk					RTC_ParkingTime;//not used in this program //vinay
//extern uint8_t							CFG_file_sizearr[4];//not used in this program //vinay
//extern uint8_t 							Active_LED;//not used in this program //vinay
extern uint8_t							LED_ON, LED_selected_in_config;
extern uint8_t 							LORA_PWR, activate_mpb_binary_from_dk;
extern uint32_t 						Tasks_Priority_Register;
extern uint8_t							Self_calibrated_after_last_cfg;

extern volatile uint8_t					No_Modem_Coin_Only;

uint8_t 								//binary_waiting_for_activation=false,
										activate_mpb_config=false,
										activate_mpb_binary=false;
//uint8_t									CC_ALLOW_IN_OOO = TRUE;
uint8_t 								config_update    = false;
extern uint8_t							Coin_Per_Udp_Packet;
extern volatile uint8_t					HARDWARE_REVISION, REV4_LED_CONFIG, Front_Bi_Color, No_Modem_Coin_Only;

//To copy default/config apn -- 18022022
uint8_t             default_apn[GPRS_GEN_BUF_SZ_TINY];
uint8_t             default_UDP_server_ip[ GPRS_IPADD_BUF_SZ ];
uint16_t            default_UDP_server_port;
extern uint8_t      APN_Assigned;
void activate_a_flash_file( FlashFieActvnTblEntry *actvn_entry_p);

#pragma CODE_SECTION(start_reprogram,".FLASHCODE")
//------------------------------------------------------------------------------
// This portion of the code is  stored in Flash at 0x0003F000
//-------------------------------------------------------------------------------
/*************************************************************************************************/
/*  Name        : start_reprogram                                                                */
/*  Parameters  : void                                                                           */
/*  Returns     : void                                                                           */
/*  Function    : Read MSP binary from on board flash and self program MSP.                      */
/*-----------------------------------------------------------------------------------------------*/
void start_reprogram()//(unsigned long file_size)//01-03-12
{
	uint8_t reprog_result = 0;
	uint8_t i=0;
	uint16_t j=0,k=0,l=0;
	eUSCI_SPI_MasterConfig Flash_int;
	uint8_t *Main_write_add =(uint8_t *) 0x0;
	uint32_t ext_flash_startAdd = AD_MSP_CODE_w;
	uint8_t buf_count=0, Data_buff=0;

	MAP_UART_enableModule(EUSCI_A1_BASE);

	MAP_UART_transmitData(EUSCI_A1_BASE,'A');
	MAP_UART_transmitData(EUSCI_A1_BASE,'A');

	P7DIR |= 0x10;
	P7OUT &= ~0x10;
	for(k=0;k<1000;k++);
	P7OUT |= 0x10;

	P6DIR &= ~0x38;
	P6SEL0 |= 0x38;	//Init flash SPI pins
	P6SEL1 &= ~0x38;

	//Init flash, display, DK CS pins
	P6DIR |= 0x04;
	P7DIR |= 0x80;
	P7DIR |= 0x08;

	P6OUT |= 0x04;
	P7OUT |= 0x80;	//disable datakey
	P7OUT &= ~0x08;	//disable display
	P6OUT |=0x40;

	Flash_int.clockPhase = EUSCI_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
	Flash_int.clockPolarity = EUSCI_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
	Flash_int.clockSourceFrequency= 12000000;
	Flash_int.desiredSpiClock= 1000000;
	Flash_int.msbFirst = EUSCI_SPI_MSB_FIRST;
	Flash_int.selectClockSource= EUSCI_SPI_CLOCKSOURCE_SMCLK;
	Flash_int.spiMode = EUSCI_SPI_3PIN;// EUSCI_SPI_4PIN_UCxSTE_ACTIVE_LOW;//
	MAP_SPI_initMaster(EUSCI_B1_BASE,&Flash_int);

	MAP_SPI_enableModule(EUSCI_B1_BASE);
	P6OUT &= ~0x04;	//enable flash
	P7OUT |= 0x80;	//disable datakey
	P7OUT &= ~0x08;	//disable display

	//Send Read cont command to external flash
	EUSCI_B_CMSIS(EUSCI_B1_BASE)->TXBUF =  0x03;
	while((MAP_SPI_isBusy(EUSCI_B1_BASE))&&(buf_count<10))
		buf_count++;
	buf_count=0;
	EUSCI_B_CMSIS(EUSCI_B1_BASE)->TXBUF =  (uint8_t) (ext_flash_startAdd >> 16);
	while((MAP_SPI_isBusy(EUSCI_B1_BASE))&&(buf_count<10))
		buf_count++;
	buf_count=0;
	EUSCI_B_CMSIS(EUSCI_B1_BASE)->TXBUF =  (uint8_t) (ext_flash_startAdd >> 8);
	while((MAP_SPI_isBusy(EUSCI_B1_BASE))&&(buf_count<10))
		buf_count++;
	buf_count=0;
	EUSCI_B_CMSIS(EUSCI_B1_BASE)->TXBUF =  (uint8_t) (ext_flash_startAdd);
	while((MAP_SPI_isBusy(EUSCI_B1_BASE))&&(buf_count<10))
		buf_count++;
	MAP_UART_transmitData(EUSCI_A1_BASE,'B');
	MAP_UART_transmitData(EUSCI_A1_BASE,'B');

	for(i=0;i<63;i++)
	{
		if(i<32)
			MAP_FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK0,((1<<i)&0xFFFFFFFF));
		else
			MAP_FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1,((1<<(i-32))&0xFFFFFFFF));

		MAP_UART_transmitData(EUSCI_A1_BASE,(48+i));

		reprog_result = MAP_FlashCtl_eraseSector((uint32_t)Main_write_add);
		for(k=0;k<5000;k++);
		if(reprog_result == false)
		{
			MAP_UART_transmitData(EUSCI_A1_BASE,'E');
			MAP_UART_transmitData(EUSCI_A1_BASE,'E');
			break;
		}
		MAP_UART_transmitData(EUSCI_A1_BASE,'x');
		for(k=0;k<1000;k++);

		P7DIR |= 0x10;
		P7OUT &= ~0x10;
		for(k=0;k<1000;k++);
		P7OUT |= 0x10;
		MAP_UART_transmitData(EUSCI_A1_BASE,'y');
		for(j=0;j<0x1000;j++)
		{
			buf_count=0;
			EUSCI_B_CMSIS(EUSCI_B1_BASE)->TXBUF =  0x00;//dummy transmit to receive the previous byte from external flash
			while((MAP_SPI_isBusy(EUSCI_B1_BASE))&&(buf_count<10))
				buf_count++;

			buf_count=0;
			Data_buff=EUSCI_B_CMSIS(EUSCI_B1_BASE)->RXBUF;
			while((MAP_SPI_isBusy(EUSCI_B1_BASE))&&(buf_count<10))
				buf_count++;
			if(j==1)
			{
				MAP_UART_transmitData(EUSCI_A1_BASE,Data_buff);
				MAP_UART_transmitData(EUSCI_A1_BASE,' ');
			}
			//Write in MSP432 flash
			reprog_result = MAP_FlashCtl_programMemory(&Data_buff,(void*)(Main_write_add++),1);
			if(reprog_result == false)
			{
				MAP_UART_transmitData(EUSCI_A1_BASE,'F');
				MAP_UART_transmitData(EUSCI_A1_BASE,'F');
				break;
			}
		}
		MAP_UART_transmitData(EUSCI_A1_BASE,'z');
		if(i<32)
			MAP_FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK0,((1<<i)&0xFFFFFFFF));
		else
			MAP_FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK1,((1<<(i-32))&0xFFFFFFFF));
	}

	MAP_GPIO_setOutputHighOnPin(FLASH_SPI_IOPORT, FLASH_SPI_CS_PIN);
	MAP_SPI_disableModule(EUSCI_B1_BASE);          // disable device

	P7OUT |= 0x80;	//disable datakey
	P7DIR |= 0x10;
	P7OUT &= ~0x10;
	for(k=0;k<1000;k++);
	P7OUT |= 0x10;

	//MAP_ResetCtl_initiateHardReset(); // Srini Change


	WDTCTL = WDTPW+WDTCNTCL+WDTSSEL1+WDTIS_7;
	for(l=0;l<60000;l++)
	{
		for(k=0;k<1000;k++);
	}
	return;
}

/*void function_at_0x3F000()
{
	typedef void (*reprog)();

	reprog call_func = (reprog) 0x3F000;

	(*call_func)();

	return;
}*/

/***********************************************************************************************************/
/** Objective	: check the each file's FileHeader and set if need be start activation process for the file*/
/** Input		: None																					   */
/* Returns		: TRUE or FALSE (success or failure)													   */
/* Date			: 25 May 2012																			   */
/* Remarks		: should be called by ACTIVATION_CHECK periodic events									   */
/***********************************************************************************************************/
void check_n_activate_files_from_flash( void )
{
	uint16_t		fcount;
	//int16_t			comm_result;

	Flash_Batch_Flag_Read();
	//20-03-2013:modified to suit the MB files reset strategy
	//files_actvn_scheduled = 0;
	//activate_attempted_files_count = 0;
	//MB_files_sucess_count = 0;
	//Act_last_retry=false;
/*	for( fcount = 2; fcount < FILE_TYPE_TOTAL_FILES; fcount ++ )//25-03-2013:check only for mb files pending for activation
	{
		if ( flash_file_actvn_tbl[fcount].status == FF_DOWNLOADED || flash_file_actvn_tbl[fcount].status == FF_ACTIVATE_STARTED )
		{
			if ( RTC_epoch_now() >= flash_file_actvn_tbl[fcount].actvn_ts )//19-06-12
			{
				files_actvn_scheduled++;
			}
		}
	}*/
	//20-03-2013:modified to suit the MB files reset strategy
	for( fcount = 0; fcount < FILE_TYPE_TOTAL_FILES; fcount ++ )
	{
		if ( flash_file_actvn_tbl[fcount].status == FF_DOWNLOADED || flash_file_actvn_tbl[fcount].status == FF_ACTIVATE_STARTED )
		{
			if ( RTC_epoch_now() >= flash_file_actvn_tbl[fcount].actvn_ts )//19-06-12
			{
				flash_file_actvn_tbl[fcount].status = FF_ACTIVATE_STARTED;
				flash_file_actvn_tbl[fcount].actvn_try_count ++;

				Debug_Output6( 0, "F-ACTVN: Try #%d - Id=%d,Typ=%d", flash_file_actvn_tbl[fcount].actvn_try_count,
						flash_file_actvn_tbl[fcount].id, flash_file_actvn_tbl[fcount].type, 0, 0, 0 );

				if ( flash_file_actvn_tbl[fcount].actvn_try_count > FACTVN_MAX_TRIES )
				{
					flash_file_actvn_tbl[fcount].status = FF_STALE;
					flash_file_actvn_tbl[fcount].actvn_try_count = 0;
					Debug_TextOut( 0, "F-ACTVN: Tried, failed & gave up" );
					Flash_Batch_Flag_Write();
					gprs_file_actn_stat( flash_file_actvn_tbl[fcount].id, flash_file_actvn_tbl[fcount].type ,file_activation_failed);
				}

				if ( flash_file_actvn_tbl[fcount].status == FF_ACTIVATE_STARTED )
				{
					//activate_attempted_files_count++;//MB files activation reset strategy change
					//activate_a_flash_file( flash_file_actvn_tbl + fcount );
					if(flash_file_actvn_tbl[fcount].type == FILE_TYPE_MPB_CODE)
					{
						Debug_TextOut( 0, "F-ACTVN: LNG Binary TRUE" );
						activate_mpb_binary = true;	//It will be activated in idle state only
					}
					else if (flash_file_actvn_tbl[fcount].type == FILE_TYPE_MPB_CONFIG)
					{
						Debug_TextOut( 0, "F-ACTVN: LNG Config TRUE" );
						activate_mpb_config = true;	//It will be activated in idle state only
					}
					//check_pending_requests();
					Flash_Batch_Flag_Write();
				}
				if ( flash_file_actvn_tbl[fcount].status == FF_ACTIVATE_COMPLETED )
				{
					if(fcount==0)
					{
						Sync_status &= (0xFFFFF0);
						Sync_status |= (0x000001);
					}
					else if(fcount==1)
					{
						Sync_status &= (0xFFFF0F);
						Sync_status |= (0x000020);
					}

					gprs_file_actn_stat( flash_file_actvn_tbl[fcount].id, flash_file_actvn_tbl[fcount].type,file_activation_success );//02-07-12
				}
				Flash_Batch_Flag_Write();
			}
		}
		else if (( flash_file_actvn_tbl[fcount].status == FF_ACTIVATE_COMPLETED)||(flash_file_actvn_tbl[fcount].status == FF_161_RETRY))//01-10-12:do not copy files to data key for the 161 retry
		{
			if(fcount==0)
			{
				Sync_status &= (0xFFFFF0);
				Sync_status |= (0x000001);
			}
			else if(fcount==1)
			{
				Sync_status &= (0xFFFF0F);
				Sync_status |= (0x000020);
			}

			gprs_file_actn_stat( flash_file_actvn_tbl[fcount].id, flash_file_actvn_tbl[fcount].type,file_activation_success );//02-07-12

			Flash_Batch_Flag_Write();
		}
	}
}

void activate_a_flash_file( FlashFieActvnTblEntry *actvn_entry_p  )
{
	uint8_t 	result;//,i=0;    											//0904

	diag_text_Screen( "Activating File", Meter_Reset, FALSE );   //1612

	//    Flash_Batch_Flag_Read();										// 24May12.Veda - load activation-table

	switch( actvn_entry_p->type )
	{
	case FILE_TYPE_MPB_CODE://activating mpb code will reset meter

		if (actvn_entry_p->size ==(uint32_t)(MPB_CODE_SIZE) )							//09-03-12
		{
			//01-02-13:check for pending requests and send it to server before msp resets with binary activation
			//diag_text_Screen( "check for pending requests", FALSE );
/*			binary_waiting_for_activation=true;
			check_pending_requests();
			binary_waiting_for_activation=false;*/
			//01-02-13:check for pending requests and send it to server before msp resets with binary activation

			diag_text_Screen( "Activating LNG Binary", TRUE, FALSE );//12-06-12
			DelayMs(100);
			if(activate_mpb_binary_from_dk == false)	//LNGSIT-730
			{
				Flash_Batch_Flag_Read();//01-06-12
				actvn_entry_p->status=FF_ACTIVATE_COMPLETED;//01-06-12
				Flash_Batch_Flag_Write();
			}
			else
				activate_mpb_binary_from_dk = false;
			Flash_RTC_ParkingClock_Storage(true, false);	//RTC append in flash every minute
			Debug_TextOut( 0, "F-ACTVN: ReProg LNG Binary..." );
			//26-03-12

			//Turn Off all LEDs
			current_rate_index = 0;
			LED_ON = FALSE;
			init_LEDs_PWM();	//This is to switch off LEDs; this change will take care of the countdown issue in R3

			watchdog_pat();
			WDT_A_holdTimer();           // Stop watchdog timer
			WDT_A_clearTimer();
			__disable_interrupt();
			GPIO_clearInterruptFlag(GPIO_PORT_P1, 0xFF);
			GPIO_clearInterruptFlag(GPIO_PORT_P2, 0xFF);
			GPIO_clearInterruptFlag(GPIO_PORT_P3, 0xFF);
			GPIO_clearInterruptFlag(GPIO_PORT_P4, 0xFF);
			GPIO_clearInterruptFlag(GPIO_PORT_P5, 0xFF);
			GPIO_clearInterruptFlag(GPIO_PORT_P6, 0xFF);
			GPIO_clearInterruptFlag(GPIO_PORT_P7, 0xFF);
			RTC_C_clearInterruptFlag(RTC_C_TIME_EVENT_INTERRUPT);
			WDT_A_unregisterInterrupt();
			Timer_A_clearInterruptFlag(TIMER_A0_BASE);
			Timer_A_clearInterruptFlag(TIMER_A1_BASE);
			Timer_A_clearInterruptFlag(TIMER_A2_BASE);
			Timer_A_clearInterruptFlag(TIMER_A3_BASE);
			UART_clearInterruptFlag(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG | EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG | EUSCI_A_UART_STARTBIT_INTERRUPT_FLAG | EUSCI_A_UART_TRANSMIT_COMPLETE_INTERRUPT_FLAG);
			UART_clearInterruptFlag(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG | EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG | EUSCI_A_UART_STARTBIT_INTERRUPT_FLAG | EUSCI_A_UART_TRANSMIT_COMPLETE_INTERRUPT_FLAG);
			UART_clearInterruptFlag(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG | EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG | EUSCI_A_UART_STARTBIT_INTERRUPT_FLAG | EUSCI_A_UART_TRANSMIT_COMPLETE_INTERRUPT_FLAG);
			UART_clearInterruptFlag(EUSCI_A3_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG | EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG | EUSCI_A_UART_STARTBIT_INTERRUPT_FLAG | EUSCI_A_UART_TRANSMIT_COMPLETE_INTERRUPT_FLAG);
			GPIO_setOutputLowOnPin(DK_CNTRL_PORT, DK_CNTRL_PIN);			//Power to datakey

			if((No_Modem_Coin_Only == true) || ((glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE) && (glSystem_cfg.Enable_GW_Comms_Coin_Only == FALSE))) //COIN Only LNG with NO Comms
			{
remove:	 		if(GPIO_getInputPinValue(KEYDETECT_PORT, KEYDETECT_PIN) == 0)
				{
					watchdog_pat();
					RemoveDatakey_Screen();
					DelayMs(1000);
					diag_text_Screen( "PLEASE REMOVE DATAKEY", TRUE , FALSE );//12-06-12
					goto remove;
				}
				else
				{
					diag_text_Screen( ".REPROGRAMING WAIT....", TRUE , FALSE );
					start_reprogram();
				}
			}
			else
			{
				diag_text_Screen( ".REPROGRAMING WAIT....", TRUE , FALSE );
				start_reprogram();
			}
//this 'if' condition is to avoid reprogram continuously in coin only meter when datakey is connected.
//vinay for coin only meter to copy from datakey
		}
		else
		{
			diag_text_Screen( "F-ACTVN:Invalid LNG Bin", Meter_Reset, FALSE );
			// Debug_TextOut( 0, "F-ACTVN: Invalid LNG Bin" );
		}
		DelayMs(500);//21-02-12
		break;

	case FILE_TYPE_MPB_CONFIG://23-05-12
		diag_text_Screen( "Activating LNG Config", Meter_Reset, FALSE );//12-06-12
		if(((actvn_entry_p->size) <= sizeof(glSystem_cfg)) && ((actvn_entry_p->size)!=0)
				&& ((actvn_entry_p->size) > MIN_LNG_CONFIG_FILE_SIZE)) //LNGSIT-1014
		{
			result = activate_config_file( TRUE );
			if (result == TRUE)
			{
				Flash_Batch_Flag_Read();//01-06-12
				actvn_entry_p->status=FF_ACTIVATE_COMPLETED;//01-06-12
				Flash_Batch_Flag_Write();
				diag_text_Screen( "F-ACTVN:CFG Success", TRUE, FALSE );
			}
			else
			{
				diag_text_Screen( "F-ACTVN:CFG Failed", Meter_Reset, FALSE );
			}
		}
		else
		{
			diag_text_Screen( "F-ACTVN:Invalid CFG", Meter_Reset, FALSE );
		}
		break;

	default:
		break;
	}
	Tasks_Priority_Register |= FILE_UPDATE_TASK;	//To update 160 or 161 event
}

uint8_t activate_config_file( uint8_t call_for_dnld )
{
	uint16_t        config_version_no = 0;//09-03-12
	uint8_t         cfg_loaded = false, tmp_erase_flg = 0, loopcnt = 0, i = 0;
	uint32_t        file_sz = 0;
	uint16_t		bytes_leftto_read = 0, bytes_read = 0;
	const uint32_t  factory_cfg_sz = sizeof( glSystem_cfg );

	if(call_for_dnld == TRUE)	//This is not a boot up config read, it is a config activation after download
	{
		config_WSAD = AD_MPB_config_r;
		config_RSAD = AD_MPB_config_w;

		loopcnt = ((FLASH_FILE_SIZE_LEN + factory_cfg_sz)/FLASH_DATA_READ_BUF_SIZE);
		if (((FLASH_FILE_SIZE_LEN + factory_cfg_sz)%FLASH_DATA_READ_BUF_SIZE) != 0)
			loopcnt += 1;

		for(i = 0; i < loopcnt; i++)
		{
			config_RSAD = Flash_Read(config_RSAD, FLASH_DATA_READ_BUF_SIZE);
			if(i == 0)
				tmp_erase_flg = 1;
			else
				tmp_erase_flg = 0;

			config_WSAD = Flash_Write(flash_data_read_buffer, FLASH_DATA_READ_BUF_SIZE, config_WSAD, tmp_erase_flg);
			DelayMs(50);
		}
	}
	config_RSAD = AD_MPB_config_r;            //09-03-12
	// read only the necessary header part of CFG file as we dont know yet, whether it is going to be
	// < factory_cfg_sz or >. Only after ver validates, we will look for sz check - 24May12.veda

	//config_RSAD = Flash_Read( config_RSAD, FLASH_FILE_SIZE_LEN + factory_cfg_sz );
	config_RSAD = Flash_Read( config_RSAD, (FLASH_FILE_SIZE_LEN + (sizeof(glSystem_cfg.config_file_CRC)*2)));

	file_sz = (((uint32_t)flash_data_read_buffer[0]) << 24)
                								+ (((uint32_t)flash_data_read_buffer[1]) << 16)
												+ (((uint32_t)flash_data_read_buffer[2]) << 8)
												+ ((uint32_t)flash_data_read_buffer[3]);

	config_version_no = ((flash_data_read_buffer[FLASH_FILE_SIZE_LEN+sizeof(glSystem_cfg.config_file_CRC)] * 1000)
			+ (flash_data_read_buffer[FLASH_FILE_SIZE_LEN+sizeof(glSystem_cfg.config_file_CRC)+1] * 100)
			+ (flash_data_read_buffer[FLASH_FILE_SIZE_LEN+sizeof(glSystem_cfg.config_file_CRC)+2] * 10)
			+ flash_data_read_buffer[FLASH_FILE_SIZE_LEN+sizeof(glSystem_cfg.config_file_CRC)+3]);

	Debug_Output1( 0, "Config File Size: %08X", file_sz );

	liberty_sprintf((char *)rx_ans,"LNG CFG Ver: %d.%d.%d.%d", (flash_data_read_buffer[FLASH_FILE_SIZE_LEN+sizeof(glSystem_cfg.config_file_CRC)]),
			(flash_data_read_buffer[FLASH_FILE_SIZE_LEN+sizeof(glSystem_cfg.config_file_CRC)+1]),
			(flash_data_read_buffer[FLASH_FILE_SIZE_LEN+sizeof(glSystem_cfg.config_file_CRC)+2]),
			(flash_data_read_buffer[FLASH_FILE_SIZE_LEN+sizeof(glSystem_cfg.config_file_CRC)+3]));
	diag_text_Screen( rx_ans, FALSE, FALSE );

	if ((config_version_no <= FACTORY_MPB_CONFIG_VERSION) && (config_version_no != 0))//09-03-12
	{
		config_RSAD = AD_MPB_config_r + FLASH_FILE_SIZE_LEN;

		if ( file_sz >= factory_cfg_sz )
		{
			bytes_read = 0;
			bytes_leftto_read = FLASH_DATA_READ_BUF_SIZE;

			while(bytes_read < factory_cfg_sz)
			{
				config_RSAD = Flash_Read(config_RSAD, bytes_leftto_read);
				memcpy(((uint8_t *)(&glSystem_cfg.config_file_CRC) + bytes_read), flash_data_read_buffer, bytes_leftto_read);
				bytes_read += bytes_leftto_read;
				bytes_leftto_read = factory_cfg_sz - bytes_read;
				if(bytes_leftto_read > FLASH_DATA_READ_BUF_SIZE)
					bytes_leftto_read = FLASH_DATA_READ_BUF_SIZE;
			}
			Debug_TextOut( 0, "LNG CFG: file_sz >= factory_sz" );
		}
		else
		{
			init_Config();
			bytes_read = 0;
			bytes_leftto_read = FLASH_DATA_READ_BUF_SIZE;

			while(bytes_read < factory_cfg_sz)
			{
				config_RSAD = Flash_Read(config_RSAD, bytes_leftto_read);
				memcpy(((uint8_t *)(&glSystem_cfg.config_file_CRC) + bytes_read), flash_data_read_buffer, bytes_leftto_read);
				bytes_read += bytes_leftto_read;
				bytes_leftto_read = factory_cfg_sz - bytes_read;
				if(bytes_leftto_read > FLASH_DATA_READ_BUF_SIZE)
					bytes_leftto_read = FLASH_DATA_READ_BUF_SIZE;
			}
			Debug_TextOut( 0, "LNG CFG: file_sz < factory_sz" );
		}

		config_update = true;
		cfg_loaded    = true;
		// glSystem_cfg.DLST_Action_byte=(glSystem_cfg.DLST_Action_byte<<8)|(glSystem_cfg.DLST_Action_byte>>8);//24-05-12
		// glSystem_cfg.allowed_trans_types=(glSystem_cfg.allowed_trans_types<<8)|(glSystem_cfg.allowed_trans_types>>8);
		diag_text_Screen( "LNG Config Updated", Meter_Reset, FALSE );//05-06-12:need this msg on display
		glSystem_cfg.MSM_max_spaces = 2; //vinay .... to avoid meter hang for SSM config LNGDSM4K-4
	}

	if ( cfg_loaded == false )
	{
		diag_text_Screen( "LNG Config Mismatch", Meter_Reset, FALSE );//05-06-12:need this msg on display

		if ( call_for_dnld == true )
		{
			return false;
		}

		init_Config();
	}
	if(glSystem_cfg.DLST_ON_OFF == 1)
	{
		Flash_Batch_Flag_Read();
		memset(glDLST_FLAGS,0,sizeof(glDLST_FLAGS));
		Flash_Batch_Flag_Write();
		DLST_FLAG_CHECK=1;
		Flash_Batch_Flag_Read();
		DLST_FLAG_CHECK=0;
	}
	else
	{
		Flash_Batch_Flag_Read();
	}

	Read_UIDKey();	//Read ID from external UID Key

	//glSystem_cfg.ON_OFF_RANDOM_INTERVAL -= (((uint32_t)(glSystem_cfg.meter_id % 10) * (uint32_t)glSystem_cfg.ON_OFF_RANDOM_INTERVAL)/10);
	//TELIT_SHUTDOWN = (glSystem_cfg.MODEM_POWER == 0) ? TRUE : FALSE;

	Debug_Verbose_Level = glSystem_cfg.debug_verbose_level;

	//Validate configuration parameters values
	Validate_Config_Parameters();

	//Following will make sure LoRa is reinitialized after any config change
	SLEEP_ENABLE = TRUE;
	LORA_PWR = FALSE;
	LORA_power_off();

	//LED selection to be refreshed after config activation
	LED_selected_in_config = glSystem_cfg.Enf_LEDs_selection;
	init_Enf_LED_GPIO();

	if(call_for_dnld == TRUE)	//This is not a boot up config read, it is a config activation after download
	{
		for(i=0; i<(MAX_COINS_SUPPORTED + MULTIPART_COINS_COUNT); i++)
		{
			//Check if self calibration for particular index is enabled
			if(((glSystem_cfg.Coin_self_calib_cfg>>i) & BIT_0) == BIT_0)
			{
				//Mark as Calibration Not done so that meter enters calibration with the next config download
				Flash_Batch_Flag_Read();
				Self_calibrated_after_last_cfg = FALSE;
				Flash_Batch_Flag_Write();
			}
		}

	}
	//Read Coin Calibration from flash and copy
	if(glSystem_cfg.erase_coin_calibration == true)
	{
		Flash_Coin_Calibration_Params(ERASE_CALIBRATION);
		glSystem_cfg.erase_coin_calibration = false;
	}
	else
		Flash_Coin_Calibration_Params(READ_CALIBRATION);

	//Get the rate refreshed here
	rtc_c_calender = RTC_C_getCalendarTime();
	read_and_fill_current_schedule(&rtc_c_calender);
//ADDED NEW TO SELECT COIN PACKET SIZE WITH RESEPECT COIN TYPE 	VT
	//glSystem_cfg.allowed_trans_types |= (1<<13);
	//glSystem_cfg.allowed_trans_types |= (1<<14);
	//Debug_Output1(0,"Coin_Per_Udp_Packet = %d",Coin_Per_Udp_Packet);
	if((((glSystem_cfg.allowed_trans_types>>13) & 0x01) == 1) || (((glSystem_cfg.allowed_trans_types>>14) & 0x01) == 1))
		Coin_Per_Udp_Packet = COINS_PER_UDP_PACKET_CT125;
	else
		Coin_Per_Udp_Packet = COINS_PER_UDP_PACKET_CT55;

    //Added to copy apn settings from config
    memcpy(default_apn , glSystem_cfg.apn,   strlen(glSystem_cfg.apn)+1);
    memcpy(default_UDP_server_ip , glSystem_cfg.UDP_server_ip, strlen(glSystem_cfg.UDP_server_ip)+1);
    default_UDP_server_port = glSystem_cfg.UDP_server_port;

    APN_Assigned = 0; //ONCE IF CONFIG UPDATE

    Debug_Output1(0,"Flash/default APN: %s", default_apn);
    Debug_Output1(0,"Flash/default UDP_IP: %s", default_UDP_server_ip);
    Debug_Output1(0,"Flash/default UDP_PORT: %d",  default_UDP_server_port);

	return true;
}

