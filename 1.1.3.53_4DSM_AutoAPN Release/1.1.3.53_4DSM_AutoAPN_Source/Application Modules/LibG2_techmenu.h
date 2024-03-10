/*
 * LibG2_techmenu.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef APPLICATION_MODULES_LIBG2_TECHMENU_H_
#define APPLICATION_MODULES_LIBG2_TECHMENU_H_

#include "../Main Module/LibG2_main.h"

#define TECH_SL_NO_LEN        	   		3        // 0404: maintenance event
#define font_1      			   		0    // = 16 x 24 font (full character set)
#define font_1_invert_text 		   		1    // = 16 x 24 font (full character set)
//										"1234567890123456789012345"
#define Main_Tech_menu1            		" 1. TECH MENU INFO	        "
#define Main_Tech_menu2            		" 2. METER INFORMATION	    "
//#define Main_Tech_menu2          		"	 POWER INFORMATION    "
#define Main_Tech_menu3            		" 3. COMMS INFORMATION    "
#define Main_Tech_menu4            		" 4. SOFTWARE INFORMATION "
#define Main_Tech_menu5            		"    Clear Alarms?NO        "
#define Main_Tech_menu6            		"    Advanced?NO            "
//#define Main_Tech_menu8			   	"	 SENSOR INFORMATION   "
#define Main_Tech_menu8			   		" 5. ADJUST METER TIME    "
#define Main_Tech_menu9			   		" 6. COIN CALIBRATION     "
#define Main_Tech_menu10			   	" 7. EXIT TECH MENU       "
//26-11-12 :Tech_modem_onoff_mode
//#define Main_Tech_menu7			   	"	 MODEM ON/OFF         "
//#define Main_Tech_menu7_1		   		"	 Power On Modem?NO    "
//#define Main_Tech_menu7_11		   	"	 Power On Modem?YES   "
//#define Main_Tech_menu7_2		   		"	 Power Off Modem?NO	  "
//#define Main_Tech_menu7_21		   	"	 Power Off Modem?YES  "
//#define Main_Tech_menu7_3		   		"	 Modem On/Off Exit?NO "
//#define Main_Tech_menu7_31		   	"	 Modem On/Off Exit?YES"
//26-11-12 :Tech_modem_onoff_mode
#define Main_Tech_menu5_1          		"	 Clear Alarms?YES	      "
#define Main_Tech_menu6_1          		"Advanced?Clear BothSpace   "
//#define Main_Tech_menu6_1_1          	"Advanced?Clear SpaceLEFT   " //vinay code opt
//#define Main_Tech_menu6_1_2          	"Advanced?Clear SpaceRIGHT  " //vinay code opt
#define Main_Tech_menu6_2          		"Advanced?Credit BothSpace  "
//#define Main_Tech_menu6_2_1          	"Advanced?Credit SpaceLEFT  " //vinay code opt
//#define Main_Tech_menu6_2_2          	"Advanced?Credit SpaceRIGHT " //vinay code opt
//#define Main_Tech_menu6_3          	"	 Advanced?CoinCardTest  "
//#define Main_Tech_menu6_4          	"	 Advanced?Comms Test    "//23-04-12
//#define Main_Tech_menu6_5          		"	 Advanced?Mainten.Log   " //vinay code opt
#define Main_Tech_menu6_6          		"	 Advanced?Adjust Clock  "
#define Main_Tech_menu6_7          		"	 Advanced?DKeyFileMgr   "
#define Main_Tech_menu6_8          		"	 Advanced?Exit          "
#define Main_Tech_menu6_9          		"	 Advanced?COMM INT      "


//05-08-2013:DPLIBB-554
#define Main_Tech_menu6_10         		"	 Advanced?In Service    "
#define Main_Tech_menu6_11         		"	 Advanced?OutOfService  "
//05-08-2013:DPLIBB-554

#define Main_Tech_menu6_7_1        		"  MECH > DATAKEY? NO     "
//17-05-2013:DPLIBB-489
#define Main_Tech_menu6_7_11       		"  MECH > DATAKEY? ALL    "
#define Main_Tech_menu6_7_12       		"  MECH > DATAKEY? CFG    "
#define Main_Tech_menu6_7_13       		"  MECH > DATAKEY? LNG FMW"
//#define Main_Tech_menu6_7_14       	"  MECH > DATAKEY? MB FMW "
//17-05-2013:DPLIBB-489
#define Main_Tech_menu6_7_2        		"  DATAKEY > MECH? NO     "
//16-05-2013:DPLIBB-489
#define Main_Tech_menu6_7_21       		"  DATAKEY > MECH? ALL    "
#define Main_Tech_menu6_7_22       		"  DATAKEY > MECH? CFG    "
#define Main_Tech_menu6_7_23       		"  DATAKEY > MECH? LNG FMW"
//#define Main_Tech_menu6_7_24       	"  DATAKEY > MECH? MB FMW "
//#define Main_Tech_menu6_7_25       	"  DATAKEY > MECH? SNS FMW"
//16-05-2013:DPLIBB-489

//#define Main_Tech_menu6_7_3        	"  LID:MECH > DK?NO       "
#define Main_Tech_menu6_7_4        		"	 EXIT? NO 			  "
//#define Main_Tech_menu6_7_11     		"  MECH > DATAKEY? YES	  "//17-05-2013:DPLIBB-489
//#define Main_Tech_menu6_7_21     		"  DATAKEY > MECH? YES	  "//16-05-2013:DPLIBB-489
//#define Main_Tech_menu6_7_31       	"  LID:MECH > DK?YES"
#define Main_Tech_menu6_7_41       		"	 EXIT? YES			  "
//#define Main_Tech_menu6          		"	 Ping Server		  "
#define Main_Tech_menu1_1          		" 4KEY {/| = CHNG SETTING "//4KEY 1/2=CHNG SETTING
#define Main_Tech_menu1_2          		" OK  = APPLY SETTING	  "
#define Main_Tech_menu1_3          		" C   = NEXT SCRN/SETTING "
//#define Main_Tech_menu1_4				" 6KEY +/- = CHNG SETTING " // not needed in 4 keypad meters and to save some memory // vinay

#define main_menu_count            		7//6
#define Clock_parm_count           		7
//#define Sensor_Config_parm_count   		4
#define Advanced_Feature_Count	   		7//12 //8 //9 //05-08-2013:DPLIBB-554 //vinay for clear and credit space extra features//vinay code opt
#define Comm_Int_Scrn_Count		   		3

#define TECH_MENU_MPB_CODE         		1
#define TECH_MENU_MPB_CONFIG       		2
//#define TECH_MENU_MB_CODE          		2
//#define TECH_MENU_RPG              		3
//#define TECH_MENU_CCF              		4
//#define TECH_MENU_MPB_CONFIG       		5
//#define TECH_MENU_MC_PGM		   		6

#define Maintenance_scrn_Intfeat_count 	3
#define Copy_Files_Options				4

typedef struct _tech_menu_disp
{
	uint8_t text[30];
    uint8_t font;
    uint8_t row;
    uint8_t column;
} Tech_Menu_Disp;

//typedef struct _Coin_Test //it is not used in the code //vinay
//{
//    uint16_t coin1count;
//    uint16_t coin2count;
//    uint16_t coin3count;
//    uint16_t coin4count;
//    uint16_t coin5count;
//    uint8_t  card_switch;
//    uint8_t  magstripe;
//}Coin_Test;
//Coin_Test coincard_test;

// 0404: maintenance event
/*typedef struct _mntc_Log
{
    uint8_t Prob_Code;
    uint8_t Sol_Code;
   // uint8_t  TECH_CARD_SL_NO[TECH_SL_NO_LEN];
    uint32_t  TECH_CARD_SL_NO;//27-06-12
}Maintenance_Evt;*/
// 0404: maintenance event

//void    TechMenu_Display_Item( Tech_Menu_Disp * ptr_tech_menu_item ); //not used in the code, all are commented //vinay
//void    TechMenu_Coin_Card_Test();
void    TechMenu_Get_RTC_update_time();
void    TechMenu_Get_Event_Update_Time();
void    TechMenu_ProcessKey_OK_In_TechMode();
void    TechMenu_ProcessKey_UP_In_TechMode();
void    TechMenu_ProcessKey_DOWN_In_TechMode();
void    TechMenu_ProcessKey_CANCEL_In_TechMode();
void    TechMenu_Display_Main_Menu();
void 	Copy_all_files_to_mechanism();
void 	Copy_all_flash_files_to_datakey();
void 	initialise_tech_menu_param();
//uint8_t check_for_valid_data(uint8_t *data); //not used in the code, all are commented //vinay

//16-05-2013:DPLIBB-489
uint8_t Copy_only_MPB_config_2mech();
uint8_t Copy_only_MPB_firmware_2mech();
//uint8_t Copy_only_MB_firmware_2mech(); //not used in the code
//16-05-2013:DPLIBB-489

//17-05-2013:DPLIBB-489
uint8_t Copy_only_MPB_config_2DK();
uint8_t Copy_only_MPB_firmware_2DK();
//uint8_t Copy_only_MB_firmware_2DK(); //not used in the code
//17-05-2013:DPLIBB-489

void    tech_mechanism_and_datakey_files();//
void    tech_mechanism_files();
//void    Copy_LID_details2DK();
//void 	Calculate_MCPGM_CRC(uint32_t File_size); //not used in the code //vinay
//uint8_t Copy_MCPGM_From_Datakey(); //not used in the code //vinay
void Meter_In_Service();
void Meter_Out_of_service();
void main_exit(); //vinay

#endif /* APPLICATION_MODULES_LIBG2_TECHMENU_H_ */
