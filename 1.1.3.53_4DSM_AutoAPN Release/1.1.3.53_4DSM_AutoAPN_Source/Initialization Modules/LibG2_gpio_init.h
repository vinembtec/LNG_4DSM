/*
 * LibG2_gpio_init.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef INITIALIZATION_MODULES_LIBG2_GPIO_INIT_H_
#define INITIALIZATION_MODULES_LIBG2_GPIO_INIT_H_

#include "../Main Module/LibG2_main.h"

#define MAGTEK_INTR_PORT								GPIO_PORT_P3
#define MAGTEK_INTR_PIN									GPIO_PIN1

#define MAGTEK_SDA_PORT									GPIO_PORT_P3
#define MAGTEK_SDA_PIN									GPIO_PIN2

#define MAGTEK_SCL_PORT									GPIO_PORT_P8
#define MAGTEK_SCL_PIN									GPIO_PIN0

#define MAGTEK_PWR_PORT									GPIO_PORT_P8
#define MAGTEK_PWR_PIN									GPIO_PIN4

#define GEMCLUB_CLK_PORT								GPIO_PORT_P4
#define GEMCLUB_CLK_PIN									GPIO_PIN3

#define GEMCLUB_RST_PORT								GPIO_PORT_P1
#define GEMCLUB_RST_PIN									GPIO_PIN1

#define GEMCLUB_IO_PORT									GPIO_PORT_P9
#define GEMCLUB_IO_PIN									GPIO_PIN6

#define PCLK_CN_PORT									GPIO_PORT_P9
#define PCLK_CN_PIN										GPIO_PIN0

#define KEYPAD_PORT										GPIO_PORT_P2
#define KEYPAD_UP_PIN									GPIO_PIN4 	//GPIO_PIN5		//RIGHT for new keypad vinay
#define KEYPAD_DOWN_PIN									GPIO_PIN1	//GPIO_PIN4		//LEFT for new keypad vinay
#define KEYPAD_OK_PIN									GPIO_PIN5	//GPIO_PIN7 	//Cancel for new keypad vinay
#define KEYPAD_CANCEL_PIN								GPIO_PIN7   //GPIO_PIN1   	//Ok for new keypad vinay
//#define KEYPAD_PORT										GPIO_PORT_P2
#define KEYPAD_RT_PIN									GPIO_PIN5		//RIGHT for new keypad vinay
#define KEYPAD_LT_PIN									GPIO_PIN4		//LEFT for new keypad vinay
#define KEYPAD_CAN_PIN									GPIO_PIN7 		//Cancel for new keypad vinay
#define KEYPAD_OKK_PIN									GPIO_PIN1   	//Ok for new keypad vinay

#define EXT_UID_IO_PORT									GPIO_PORT_P4
#define EXT_UID_IO_PIN									GPIO_PIN0

#define EXT_UID_IO_DATA_IN								P4IN
#define EXT_UID_IO_DATA_OUT								P4OUT
#define EXT_UID_IO_DATA_DIR								P4DIR
#define EXT_UID_IO_DATA_BIT								BIT_0

#define INT_UID_IO_DATA_IN								P3IN
#define INT_UID_IO_DATA_OUT								P3OUT
#define INT_UID_IO_DATA_DIR								P3DIR
#define INT_UID_IO_DATA_BIT								BIT_0

#define INT_UID_IO_PORT									GPIO_PORT_P3
#define INT_UID_IO_PIN									GPIO_PIN0

#define DISPLAY_SPI_IOPORT                         		GPIO_PORT_P6
#define DISPLAY_SPI_MOSI_PIN                            GPIO_PIN4
#define DISPLAY_SPI_MISO_PIN                            GPIO_PIN5
#define DISPLAY_SPI_SCK_PIN								GPIO_PIN3

#define DISPLAY_SPI_DATA                         		P6OUT

#define DISPLAY_SPI_CS_IOPORT  							GPIO_PORT_P7
#define DISPLAY_SPI_CS_PIN								GPIO_PIN3

#define DISPLAY_BKLT_PORT								GPIO_PORT_P6
#define DISPLAY_BKLT_PIN								GPIO_PIN7

#define FLASH_SPI_IOPORT                         		GPIO_PORT_P6
#define FLASH_SPI_MOSI_PIN                            	GPIO_PIN4
#define FLASH_SPI_MISO_PIN                            	GPIO_PIN5
#define FLASH_SPI_SCK_PIN								GPIO_PIN3
#define FLASH_SPI_CS_PIN								GPIO_PIN2

#define NLED_DRIVE1_PORT								GPIO_PORT_P10
#define NLED_DRIVE1_PIN									GPIO_PIN5

#define NLED_DRIVE2_PORT								GPIO_PORT_P4
#define NLED_DRIVE2_PIN									GPIO_PIN4

#define NLED_DRIVE3_PORT								GPIO_PORT_P5
#define NLED_DRIVE3_PIN									GPIO_PIN2

#define NLED_DRIVE3_PORT_R4								GPIO_PORT_P9
#define NLED_DRIVE3_PIN_R4								GPIO_PIN3

#define PCLK_PORT										GPIO_PORT_P4
#define PCLK_PIN										GPIO_PIN3

#define LED_RXD_PORT									GPIO_PORT_P4
#define LED_RXD_PIN										GPIO_PIN1

#define	CHIRP_PORT										GPIO_PORT_P6
#define	CHIRP_PIN										GPIO_PIN1

#define SNSON_PORT										GPIO_PORT_P9
#define SNSON_PIN										GPIO_PIN1

#define CLSNS_PORT										GPIO_PORT_P5
#define CLSNS_PIN										GPIO_PIN3

#define SNSRDY_PORT										GPIO_PORT_P6
#define SNSRDY_PIN										GPIO_PIN0

#define T_PORT											GPIO_PORT_P9
#define T_PIN											GPIO_PIN2

#define CNTR_PORT										GPIO_PORT_P2
#define CNTR_PIN										GPIO_PIN6

#define MDM_REG_CNTRL_PORT								GPIO_PORT_P1
#define MDM_REG_CNTRL_PIN								GPIO_PIN0

#define MDM_RST_PORT									GPIO_PORT_P8
#define MDM_RST_PIN										GPIO_PIN6

#define MDM_ON_PORT										GPIO_PORT_P10
#define MDM_ON_PIN										GPIO_PIN2

#define MDM_DTR_PORT									GPIO_PORT_P5
#define MDM_DTR_PIN										GPIO_PIN7

#define MDM_RING_PORT									GPIO_PORT_P3
#define MDM_RING_PIN									GPIO_PIN3

#define MDM_PORT										GPIO_PORT_P1
#define MDM_RXD_PIN										GPIO_PIN2
#define MDM_TXD_PIN										GPIO_PIN3

#define SENSOR_CNTRL1_PORT								GPIO_PORT_P10
#define SENSOR_CNTRL1_PIN								GPIO_PIN4

#define SENSOR_CNTRL2_PORT								GPIO_PORT_P7
#define SENSOR_CNTRL2_PIN								GPIO_PIN4

#define RECH_BATT_PORT									GPIO_PORT_P5
#define RECH_BATT_PIN									GPIO_PIN4

#define SOLAR_BATT_PORT									GPIO_PORT_P5
#define SOLAR_BATT_PIN									GPIO_PIN5

#define DEBUG_PORT										GPIO_PORT_P2
#define DEBUG_TX_PIN									GPIO_PIN3
#define DEBUG_RX_PIN									GPIO_PIN2

#define RF_CNTRL_PORT									GPIO_PORT_P7
#define RF_CNTRL_PIN									GPIO_PIN1

#define IO_EXP_CS_PORT									GPIO_PORT_P10
#define IO_EXP_CS_PIN									GPIO_PIN0

#define IDTECH_INTR_PORT								GPIO_PORT_P1
#define IDTECH_INTR_PIN									GPIO_PIN1

#define IDTECH_PWR_PORT									GPIO_PORT_P7
#define IDTECH_PWR_PIN									GPIO_PIN2

#define EXCESS_LED_PORT									GPIO_PORT_PJ
#define EXCESS_LED_PIN									GPIO_PIN5

#define EXCESS_LED_PORT_R4								GPIO_PORT_P5
#define EXCESS_LED_PIN_R4								GPIO_PIN1

#define PENALTY_LED_PORT								GPIO_PORT_P5
#define PENALTY_LED_PIN									GPIO_PIN6

#define RGLED_PORT										GPIO_PORT_P7
#define RGLED_PIN										GPIO_PIN5

#define COMMS_LED_HILO_PORT								GPIO_PORT_P8
#define COMMS_LED_HILO_PIN								GPIO_PIN5

#define DISP_LED_HILO_PORT								GPIO_PORT_P8
#define DISP_LED_HILO_PIN								GPIO_PIN3

#define LED_SWITCH_PORT									GPIO_PORT_P9
#define LED_SWITCH_PIN									GPIO_PIN0

#define DISP_LED_SWITCH_PORT							GPIO_PORT_P7
#define DISP_LED_SWITCH_PIN								GPIO_PIN0

#define BKLT_PORT										GPIO_PORT_P6
#define BKLT_PIN										GPIO_PIN7

#define RED_LED											1
#define GREEN_LED										2

#define RED_LED1										1
#define GREEN_LED1										2

#define RED_LED2									    3
#define GREEN_LED2										4

#define FRONT_RED_ONLY_ENF_LED							6
#define FRONT_GREEN_ONLY_ENF_LED						5
#define REAR_RED_ONLY_ENF_LED							4
#define REAR_GREEN_ONLY_ENF_LED							3
#define FRONT_AND_REAR_ENF_LED							2
#define REAR_ENF_LED									1
#define FRONT_ENF_LED									0

#define DataKey_SPI_IOPORT                           	GPIO_PORT_P7
#define DataKey_SPI_CS_PIN        						GPIO_PIN7

#define DK_CNTRL_PORT                           		GPIO_PORT_P6
#define DK_CNTRL_PIN        							GPIO_PIN6
#define COMMS_DETECT1_PORT                           	GPIO_PORT_P6
#define COMMS_DETECT1_PIN        						GPIO_PIN6

#define FRONT_BICOLOR_DETECT_PORT                       GPIO_PORT_P2
#define FRONT_BICOLOR_DETECT_PIN        				GPIO_PIN0

#define KEYDETECT_PORT         							GPIO_PORT_P4
#define KEYDETECT_PIN         							GPIO_PIN2

#define WD_DSP_PORT         							GPIO_PORT_P7
#define WD_DSP_PIN         								GPIO_PIN4

//LORA IOs
#define LORA_RESET_IOPORT                               GPIO_PORT_P10
#define LORA_RESET_PIN                                  GPIO_PIN1
#define LORA_HFANT_CNTL									GPIO_PIN3//GPIO_PIN4
#define LORA_LFANT_CNTL									GPIO_PIN3

#define LORA_LFANT_CNTL_PORT							GPIO_PORT_P9
#define LORA_LFANT_CNTL_PIN								GPIO_PIN4

#define LORA_HF_PORT									GPIO_PORT_P10//GPIO_PORT_PJ
/*!
 * SX1276 SPI NSS I/O definitions
 */
#define LORA_SPI_IOPORT                                 GPIO_PORT_P3
#define LORA_NSS_PIN                                    GPIO_PIN4
#define LORA_SCK_PIN									GPIO_PIN5
#define LORA_MOSI_PIN									GPIO_PIN6
#define LORA_MISO_PIN									GPIO_PIN7

#define LORA_RXTX_IOPORT								GPIO_PORT_P7
#define LORA_RXTX_PIN 									GPIO_PIN6

#define DIO0_IOPORT                                 	GPIO_PORT_P3
#define DIO0_PIN                                    	GPIO_PIN0

#define DIO1_IOPORT                                 	GPIO_PORT_P3
#define DIO1_PIN                                    	GPIO_PIN0


#define DIO2_IOPORT                                 	GPIO_PORT_P3
#define DIO2_PIN                                    	GPIO_PIN0

#define DIO3_IOPORT                                		GPIO_PORT_P3
#define DIO3_PIN                                    	GPIO_PIN0

#define DIO4_IOPORT                                 	GPIO_PORT_P3
#define DIO4_PIN                                    	GPIO_PIN0

#define DIO5_IOPORT                                 	GPIO_PORT_P3
#define DIO5_PIN                                    	GPIO_PIN0

void gpio_default_init();
void init_cointrack_GPIO();
void init_keypad_GPIO (void);
void init_mag_card_reader_GPIO (void);
void init_smartcard_GPIO(void);
void init_rchbattery_GPIO (void);
void init_solarbattery_GPIO (void);
void init_adc_module_GPIO (void);
void init_IO_exp_SPI_module_GPIO (void);
void init_IO_exp_module_GPIO (void);
void init_flash_SPI_module_GPIO (void);
void init_display_SPI_module_GPIO (void);
void init_DataKey_SPI_module_GPIO (void);
void init_debug_GPIO (void);
void init_Telit_GPIO (void);
void MAG_1_POWER_ON(void);
void MAG_1_POWER_OFF(void);
void MDM_DTR_set();
void MDM_DTR_clr();
void MDM_REG_CNTRL_set();
void MDM_REG_CNTRL_clr();
void MDM_Tel_ON();
void MDM_Tel_OFF();
void MDM_RST_set();
void MDM_RST_clr();
void Display_Init_SWSPI(void);
void init_IDTech_card_reader_GPIO (void);
void init_card_interrupt(uint8_t Card_Reader_Version);
void Turn_Off_card_IOs(uint8_t Card_Reader_Version);
void BKLT_OFF(void);
void BKLT_ON(void);
void IDTech_POWER_OFF(void);
void IDTech_POWER_ON(void);
void LORA_power_on();
void LORA_power_off();

void disable_telit_RI_interrupt();
void enable_telit_RI_interrupt();
void enable_optical_LED_interrupt();
void disable_optical_LED_interrupt();

#endif /* INITIALIZATION_MODULES_LIBG2_GPIO_INIT_H_ */
