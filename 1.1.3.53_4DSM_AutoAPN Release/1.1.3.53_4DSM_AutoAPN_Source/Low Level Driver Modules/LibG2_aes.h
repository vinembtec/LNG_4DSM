/*
 * LibG2_aes.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef LOW_LEVEL_DRIVER_MODULES_LIBG2_AES_H_
#define LOW_LEVEL_DRIVER_MODULES_LIBG2_AES_H_

#include "../Main Module/LibG2_main.h"

#define AES_BLOCK_SIZE 						16
#define AES_KEY_SIZE   						16
// foreward sbox
#define AES_KEY_ID_OFFSET     				68 /* Offset to key ID in buffer */
#define AES_KEY_CRC_OFFSET    				60 /* Offset to start of key CRC in the received buffer */
#define AES_KEY_LENGTH_OFFSET 				42 /* offset to length field for AES Key in buffer */
#define AES_KEY_OFFSET        				44 /* Offset to start of actual key in the received buffer */

#define CARD_OLT_TRACK1_SIZE 				(86) //for AES encryption packet should be div by 16 //not used //vinay
#define CARD_OLT_TRACK2_SIZE 				(54) //:for AES encryption packet should be div by 16 //not used //vinay
#define CCS_OK                              1           // Card read check format is OK
#define CCS_BAD_FORMAT                      0           // Card read check format is BAD

void multiple_block_aes_encrypt(uint8_t *state,uint16_t block_size,uint8_t *key);
void multiple_block_aes_decrypt(uint8_t *state,uint16_t block_size,uint8_t *key);

void Init_128BIT_AES_Set_EncryptMode(void);
void Init_128BIT_AES_Set_DecryptMode(void);

#endif /* LOW_LEVEL_DRIVER_MODULES_LIBG2_AES_H_ */
