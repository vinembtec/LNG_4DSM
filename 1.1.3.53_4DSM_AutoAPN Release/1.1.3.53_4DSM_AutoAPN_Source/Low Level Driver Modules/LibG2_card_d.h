/***************************************************************************/
/*  Module Names    : xxtea.h                                              */                                                    
/*  MCU             : Hitachi H8S/2633F                                    */
/*  Function        : definition of technician key                         */
/*  Revision        :                                                      */
/*    Author        : Santhosh Krishna                                     */ 
/*  Revision        :    Date                        Reason                */
/*                       Nov 2008                Initial Design            */
/***************************************************************************/

#ifndef _XXTEA_H_
#define _XXTEA_H_

#include <stdint.h>

#define NUM_XXTEA_KEYS (5)

extern uint32_t const card_xxtea_key[NUM_XXTEA_KEYS];

extern void card_encode (uint32_t * buffer, uint16_t buf_len,  const uint32_t * tea_key );
extern void card_decode (uint32_t * buffer, uint16_t buf_len,  const uint32_t * tea_key );

#endif   // _XXTEA_H_
