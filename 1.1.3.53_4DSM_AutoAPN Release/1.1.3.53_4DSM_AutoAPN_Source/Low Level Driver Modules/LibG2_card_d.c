/********************************************************************/
/*                                                                  */
/*  Module           : xxtea.c                                      */
/*  Language used    : GNU C / H8                                   */
/*  Microprocessor   : H8S/2633H                                    */
/*                                                                  */
/*  Function         : Encrypt and Decrypt using XXTEA encryptions  */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/*  Revision History :  Author        Date            Reason        */
/*                      Santhosh/                                   */
/*                      Dhanu         Nov 2008        First Design  */
/*                        Veda        Apr 2011        fixed bug     */
/*                                                                  */
/* Veda: it was using 4 bytes (long) at a time to encrypt/decrypt.  */
/* But the loop was restricted to 5 iterations and so 5*4=20 bytes  */
/* of the given buffer only was encrypted/decrypted. Took care of   */
/* the entire length of the buffer                                  */
/*                                                                  */
/********************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "LibG2_card_d.h"

/*
***********************************************************************
*Imported functions
***********************************************************************
*/

/*
***********************************************************************
*Global Constants
***********************************************************************
*/
uint32_t const card_xxtea_key[NUM_XXTEA_KEYS] =
{
    0xAFACE0FF, 0xAFACE0FF, 0xAFACE0FF, 0xAFACE0FF,0xAFACE0FF
};    //2812

/*
***********************************************************************
*Local Defines
***********************************************************************
*/
#define XXTEA_DELTA   0x9E3779B9

/*
***********************************************************************
*Local Variables
***********************************************************************
*/
static int32_t     z, y, sum, tmp, mx;
static uint8_t  e;


/*----------------------------------------------------------------------*/
/*  Func Name   : mx_update                                             */
/*  Parameters  : char                                                  */
/*  Returns     : void                                                  */
/*  Function    : Update function                                       */
/*----------------------------------------------------------------------*/
static void mx_update (uint8_t p, const uint32_t * tea_key_ptr)
{
// #define XXTEA_DO_MX   ( (z >> 5  ^  y << 2)  +  (y >> 3  ^  z << 4) ^ (sum ^ y)  + (tea_key_ptr[ p & 3  ^ e] ^ z) )
//  mx = XXTEA_DO_MX;    //2812
    mx = (  (z >> 5  ^  y << 2)  +  (y >> 3  ^  z << 4) ^ (sum ^ y)  + (tea_key_ptr[ p & 3  ^ e] ^ z) );    //2812
}

/*----------------------------------------------------------------------*/
/*  Func Name   : mx_encode                                             */
/*  Parameters  : char                                                  */
/*  Returns     : void                                                  */
/*  Function    : Encode function                                       */
/*----------------------------------------------------------------------*/
static void mx_encode (uint8_t p, const uint32_t * tea_key_ptr)
{
    mx_update (p, tea_key_ptr);
    z = tmp + mx;
}

/*----------------------------------------------------------------------*/
/*  Func Name   : mx_decode                                             */
/*  Parameters  : char                                                  */
/*  Returns     : void                                                  */
/*  Function    : Decode function                                       */
/*----------------------------------------------------------------------*/
static void mx_decode (uint8_t p, const uint32_t * tea_key_ptr)
{
    mx_update (p, tea_key_ptr);
    y = tmp - mx;
}

/*----------------------------------------------------------------------*/
/*  Func Name   : xxtea_encode                                          */
/*  Parameters  : long *                                                */
/*  Returns     : void                                                  */
/*  Function    : Encryption                                            */
/*----------------------------------------------------------------------*/
void card_encode (uint32_t * buffer, uint16_t buf_len,  const uint32_t * tea_key_ptr )
{
    uint32_t    n = buf_len / sizeof( *buffer );
    uint32_t    q = (uint32_t) ( 6 + (52 / n) );
    uint32_t    i;

    y = sum = mx = 0;
    z = buffer[n - 1];

    while ( q-- > 0 )
    {
        sum += XXTEA_DELTA;

        e = (uint8_t)((sum >> 2) & 0x3);

        for ( i = 0;  i < n-1;  ++i )
        {
            y = buffer[i+1];

            tmp = buffer[i];

            mx_encode ((uint8_t)i, tea_key_ptr);

            buffer[i] = z;
        }

        y = buffer[0];

        tmp = buffer[i];

        mx_encode ( (uint8_t)i, tea_key_ptr );

        buffer[n-1] = z;
    }

    return;
}

/*----------------------------------------------------------------------*/
/*  Func Name   : card_decode                                          */
/*  Parameters  : long *                                                */
/*  Returns     : void                                                  */
/*  Function    : Decryption                                            */
/*----------------------------------------------------------------------*/
void card_decode (uint32_t * buffer, uint16_t buf_len,  const uint32_t * tea_key_ptr )
{
    uint32_t    n = buf_len / sizeof( *buffer );
    uint32_t    q = (uint32_t) ( 6 + (52 / n) );
    uint32_t    i;

    z = mx = 0;
    y = buffer[0];

    sum = XXTEA_DELTA * q;

    while ( sum != 0 )
    {
        e = (uint8_t)((sum >> 2) & 0x3);

        for( i = n-1;  i > 0;  --i )
        {
            z = buffer[i-1];

            tmp = buffer[i];

            mx_decode ((uint8_t)i,  tea_key_ptr);

            buffer[i] = y;
        }

        z = buffer[n-1];

        tmp = buffer[0];

        mx_decode (0,  tea_key_ptr);

        buffer[0] = y;

        sum -= XXTEA_DELTA;
    }

    return;
}

