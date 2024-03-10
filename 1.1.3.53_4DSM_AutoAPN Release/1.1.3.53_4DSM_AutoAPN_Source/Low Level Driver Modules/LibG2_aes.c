//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_aes.c
//
//****************************************************************************

#include "LibG2_aes.h"

/* Key and Key ID are located in module aes.c */
uint16_t 							AES_KEY_ID = 1; //0;
uint8_t 							AES_KEY[AES_KEY_SIZE] = { 0x00, 0x01, 0x02, 0x03,
																0x04, 0x05, 0x06, 0x07,
																0x08, 0x09, 0x0a, 0x0b,
																0x0c, 0x0d, 0x0e, 0x0f };

void Init_128BIT_AES_Set_EncryptMode(void)
{
	/* Set AES module to encrypt mode */
	AESACTL0 &= ~AESOP_0;

	/* Set AES key length to 128 bits */
	AESACTL0 = AESACTL0 & ((AESKL_0)) | AESKL__128BIT;

	/* Load a cipher key to module */
	MAP_AES256_setCipherKey(AES256_BASE, AES_KEY, AES256_KEYLENGTH_128BIT);
}

void Init_128BIT_AES_Set_DecryptMode(void)
{
	/* Set AES module to decrypt mode */
	AESACTL0 |= AESOP_1;

	/* Set AES key length to 128 bits */
	AESACTL0 = AESACTL0 & ((AESKL_0)) | AESKL__128BIT;

	/* Load a Decipher key to module */
	MAP_AES256_setDecipherKey(AES256_BASE, AES_KEY, AES256_KEYLENGTH_128BIT);
}

/**************************************************************************/
/*  Name        : multiple_block_aes_encrypt                              */
/*  Parameters  : uint8_t *,uint16_t,uint8_t *            */
/*  Returns     : void                                                    */
/*  Function    : encrypt multiple blocks of data                         */
/*------------------------------------------------------------------------*/
void multiple_block_aes_encrypt(uint8_t *state,uint16_t block_size,uint8_t *key)
{
	uint8_t n=0,k;
	n=block_size/AES_BLOCK_SIZE;
	for(k=0;k<n;k++)
	{
		//aes_encrypt(state,AES_KEY);
		Init_128BIT_AES_Set_EncryptMode();
		MAP_AES256_encryptData(AES256_BASE, state, state);
		state +=(sizeof(uint8_t)*AES_BLOCK_SIZE);
	}
}
/**************************************************************************/
/*  Name        : multiple_block_aes_decrypt                              */
/*  Parameters  : uint8_t *,uint16_t,uint8_t *            */
/*  Returns     : void                                                    */
/*  Function    : decrypt multiple blocks of data                         */
/*------------------------------------------------------------------------*/
void multiple_block_aes_decrypt(uint8_t *state,uint16_t block_size,uint8_t *key)
{
	uint8_t n=0,k;
	n=block_size/AES_BLOCK_SIZE;
	for(k=0;k<n;k++)
	{
		//aes_decrypt(state,AES_KEY);
		Init_128BIT_AES_Set_DecryptMode();
		MAP_AES256_decryptData(AES256_BASE, state, state);
		state +=(sizeof(uint8_t)*AES_BLOCK_SIZE);
	}
}

/* Check the key CRC */
uint8_t Validate_AES_key(uint8_t *receivedKey, uint8_t *receivedCRC, uint8_t keyLength)
{
	uint8_t rc = 0;
	uint32_t lineCRC = 0;
	int i;
	for (i=3;i>=0;i--)
	{
		lineCRC <<= 8;
		lineCRC += receivedCRC[i];
	}

	uint32_t messageCRC = calc_crc32(0, receivedKey, keyLength);

	rc = (messageCRC == lineCRC);
	return rc;
}

/* Access key related fields in the receive buffer, check the CRC then save received key */
uint8_t processAesKey(uint8_t *pBuff, uint16_t bufferLength)
{
	int rc = 0;
	uint16_t key_length=(pBuff[AES_KEY_LENGTH_OFFSET])|(pBuff[AES_KEY_LENGTH_OFFSET + 1]<<8);
	if (Validate_AES_key(&(pBuff[AES_KEY_OFFSET]), &(pBuff[AES_KEY_CRC_OFFSET]), key_length))
	{
		memcpy(&AES_KEY[0], &pBuff[AES_KEY_OFFSET], key_length);
		AES_KEY_ID = (pBuff[AES_KEY_ID_OFFSET + 1] << 8) | (pBuff[AES_KEY_ID_OFFSET]);
	}
	else
	{
		rc = -1;
	}
	return rc;
}

bool isAESKeyExchangeRequired()
{
	return (AES_KEY_ID == 0);
}

