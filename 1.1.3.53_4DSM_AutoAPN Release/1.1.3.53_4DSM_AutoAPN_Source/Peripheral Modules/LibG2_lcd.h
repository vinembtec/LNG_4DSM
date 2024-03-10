/*
 * LibG2_lcd.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef COMMON_MODULES_LIBG2_LCD_H_
#define COMMON_MODULES_LIBG2_LCD_H_

#include "../Main Module/LibG2_main.h"

#define MLCD_WR 										0x01					// MLCD write line command
#define MLCD_CM 										0x04					// MLCD clear memory command
#define MLCD_SM 										0x00					// MLCD static mode command
#define MLCD_VCOM 										0x02					// MLCD VCOM bit

#define PIXELS_X 										400					// display is 400x240
#define PIXELS_Y 										240					// display is 400x240
#define MAXBYTESINROW 									50

#define DISP_INVERT 									1					// INVERT text
#define DISP_WIDE 										2						// double-width text
#define DISP_HIGH 										4						// double-height text

//#define DISP_HALF_SCREEN								3  //not used //vinay

#define DISP_INVERT_LEFT								5					// INVERT only left side
#define DISP_INVERT_RIGHT								6					// Invert only Right side

#define MIN_AMOUNT_DISPLAY								1
#define MAX_AMOUNT_DISPLAY								2

#define FONT_SIZE_SMALL									1
//#define FONT_SIZE_MEDIUM								2  //not used //vinay
#define MAX_CHAR_ON_LCD									30 //26

#define LEFT_BAY										0
#define RIGHT_BAY										1

void lcd_vcom_pat();
void SPIWriteLine(uint8_t line);
void SPIWriteRLELine(uint8_t line,uint8_t size );
void SPIWriteRLELine_mod(uint8_t line,uint8_t size,uint8_t column );

void printSharp16x24(const uint8_t* text, uint8_t line, uint8_t column, uint8_t options);
void printSharp32x48(const uint8_t* text, uint8_t line, uint8_t column,uint8_t options);
void printSharp32x48HfScr(const uint8_t* text, uint8_t line, uint8_t column,uint8_t options);
//void printSharp48x72(const uint8_t* text, uint8_t line, uint8_t column, uint8_t options); //vinay code opt
//void printSharpnoparking(const uint8_t *byte, uint8_t row, uint8_t column, uint8_t bytes4row, uint16_t numofrows);  //not used //vinay
//void printSharp_CardCoin(const uint8_t *byte, uint8_t row, uint8_t column, uint8_t bytes4row, uint16_t numofrows);  //not used //vinay
void printSharp_Card(const uint8_t *byte, uint8_t row, uint8_t column, uint8_t bytes4row, uint16_t numofrows);

//APPLICATION
//void InsertCard_Screen();  //not used //vinay
//void InsertCardCoin_Screen();  //not used //vinay
void Noparking_Screen();
void Freeparking_Screen();
uint8_t CardReadFail_Screen();
void ShowAmount_Screen(uint16_t lamount_selected, uint8_t min_max_display,uint8_t screen_refresh);
void Authorizing_Screen();
void ReadCard_Screen();
void MeterFull_Screen();
void AntiFeed_Screen();
//void InsertCoin_Screen();  //not used //vinay
//void InsertOnlyCard_Screen();  //not used //vinay
void CC_Payment_Approved_Screen();
void CC_Payment_Declined_Screen();
void Remove_Card_Screen();
void Clear_Screen();
void diag_text_Screen(uint8_t * text_to_lcd, uint8_t force_on_lcd, uint8_t quick_text);
void Coin_Calibration_Screen(uint16_t coin_value_to_show, uint8_t coin_value_index, uint8_t total_coins_to_show, uint8_t currently_calib_index);

void Update_RTC_on_Display();
void Update_Parking_Clock_Screen();
void Idle_Screen();
void AUDIT_Screen();
void Meter_Out_Of_Serice_Screen();
//void Meter_In_Serice_Screen();  //not used //vinay
void LCD_Backlight(uint8_t backlight_on);
void SC_Card_Invalid_Screen();
void SC_Card_Low_Fund_Screen();
void SC_Payment_Approved_Screen(uint8_t refund_done);
void Check_LCD_Backlight_Power_Save_Hours(uint8_t ON_HOUR,uint8_t OFF_HOUR);
void RefundShowAmount_Screen(uint16_t lamount_selected, uint32_t min_max_display,uint8_t screen_refresh);

void Card_Not_Supported_Screen();
void SC_Payment_Disabled_Screen();
void CC_Card_Expired_Screen();
void AUDIT_In_Progress_Screen();
void AUDIT_Card_Full_Screen();
void AUDIT_Append_Failed_Screen();
//void LCD_Test(void); //not used //vinay
void DSM_Screen1();
void DSM_Screen2();
void DSM_Screen3();
void DSM_Screen4();
void DSM_Screen5();
void DSM_Screen6();
void DSM_Screen7();

void RemoveDatakey_Screen();    //vinay for coin only meter to copy from datakey

#endif /* COMMON_MODULES_LIBG2_LCD_H_ */
