/*
 * Liberty_GIF_FullCoin.,
 *
 *  Created on: Oct 14, 2016
 *      Aut,or: admin
 */

#ifndef PERIPHERAL_MODULES_FONTS_LIBERTY_GIF_FULLCOIN_H_
#define PERIPHERAL_MODULES_FONTS_LIBERTY_GIF_FULLCOIN_H_



//fontgraph4:
// full coin graphic
// fontcharbytes16_graph4: = 0x288    // bytes per char
// fontbyteswide_graph4: = 0x9       // bytes required for width of each char
// fontheight_graph4: = 0x48          // pixels in height

//const uint8_t fontGIF4[648] =
//{
//		0xFF,0x00,0x80,0xFF,0x07,0x00,0x00,0x00,
//		0x00,0xFF,0x00,0xF0,0xC0,0x39,0x00,0x00,
//		0x00,0x00,0xFF,0x00,0x1E,0x00,0x7F,0x00,
//		0x00,0x00,0x00,0xFF,0x00,0x07,0x00,0x88,
//		0x01,0x00,0x00,0x00,0xFF,0x80,0x01,0x00,
//		0xF0,0x01,0x00,0x00,0x00,0xFF,0xC0,0x00,
//		0x00,0x20,0x06,0x00,0x00,0x00,0xFF,0x60,
//		0x00,0x00,0xC0,0x07,0x00,0x00,0x00,0xFF,
//		0x30,0x00,0x00,0x80,0x08,0x00,0x00,0x00,
//		0xFF,0x18,0x00,0x0E,0x00,0x1F,0x00,0x00,
//		0x00,0xFF,0x0C,0x00,0x1F,0x00,0x12,0x00,
//		0x00,0x00,0xFF,0x06,0x00,0x1F,0x00,0x3E,
//		0x00,0x00,0x00,0xFF,0x02,0x00,0x1F,0x00,
//		0x64,0x00,0x00,0x00,0xFF,0x03,0x00,0x1F,
//		0x00,0x78,0x00,0x00,0x00,0xFF,0x01,0x00,
//		0x1F,0x00,0x88,0x00,0x00,0x00,0xFF,0x00,
//		0x00,0x1F,0x00,0xF0,0x01,0x00,0x00,0xFF,
//		0x00,0x00,0x1F,0x00,0x10,0x01,0x00,0x00,
//		0x7F,0x00,0x00,0x3F,0x00,0xE0,0x01,0x00,
//		0x00,0x7F,0x00,0xC0,0xFF,0x00,0x20,0x02,
//		0x00,0x00,0x7F,0x00,0xE0,0xFF,0x01,0xE0,
//		0x03,0x00,0x00,0x3F,0x00,0xF0,0xFF,0x03,
//		0x40,0x06,0x00,0x00,0x3F,0x00,0xF8,0xFF,
//		0x07,0xC0,0x07,0x00,0x00,0x3F,0x00,0xFC,
//		0xFF,0x0F,0x40,0x04,0x00,0x00,0x3F,0x00,
//		0xFC,0xDF,0x0F,0x80,0x07,0x00,0x00,0x3F,
//		0x00,0x7E,0x9F,0x1F,0x80,0x08,0x00,0x00,
//		0x1F,0x00,0x7E,0x1F,0x1F,0x80,0x0F,0x00,
//		0x00,0x1F,0x00,0x3E,0x1F,0x3F,0x80,0x08,
//		0x00,0x00,0x1F,0x00,0x3F,0x1F,0x3E,0x80,
//		0x0F,0x00,0x00,0x1F,0x00,0x1F,0x1F,0x3E,
//		0x00,0x19,0x00,0x00,0x1F,0x00,0x1F,0x1F,
//		0x00,0x00,0x1F,0x00,0x00,0x0F,0x00,0x1F,
//		0x1F,0x00,0x00,0x11,0x00,0x00,0x0F,0x80,
//		0x0F,0x1F,0x00,0x00,0x1F,0x00,0x00,0x0F,
//		0x80,0x0F,0x1F,0x00,0x00,0x11,0x00,0x00,
//		0x0F,0x80,0x0F,0x1F,0x00,0x00,0x1F,0x00,
//		0x00,0x0F,0x80,0x0F,0x1F,0x00,0x00,0x11,
//		0x00,0x00,0x0F,0x80,0x0F,0x1F,0x00,0x00,
//		0x1F,0x00,0x00,0x0F,0x80,0x0F,0x1F,0x00,
//		0x00,0x11,0x00,0x00,0x0F,0x80,0x0F,0x1F,
//		0x00,0x00,0x1F,0x00,0x00,0x1F,0x80,0x0F,
//		0x1F,0x00,0x00,0x11,0x00,0x00,0x1F,0x80,
//		0x0F,0x1F,0x00,0x00,0x1F,0x00,0x00,0x1F,
//		0x80,0x1F,0x1F,0x00,0x00,0x11,0x00,0x00,
//		0x1F,0x00,0x1F,0x1F,0x3E,0x00,0x1F,0x00,
//		0x00,0x3F,0x00,0x1F,0x1F,0x3E,0x80,0x08,
//		0x00,0x00,0x3F,0x00,0x3F,0x1F,0x3E,0x80,
//		0x0F,0x00,0x00,0x3F,0x00,0x3E,0x1F,0x3F,
//		0x80,0x08,0x00,0x00,0x3F,0x00,0x3E,0x1F,
//		0x1F,0x80,0x0F,0x00,0x00,0x3F,0x00,0x7E,
//		0x9F,0x1F,0x80,0x0C,0x00,0x00,0x7F,0x00,
//		0xFC,0xDF,0x0F,0xC0,0x07,0x00,0x00,0x7F,
//		0x00,0xFC,0xFF,0x0F,0x40,0x04,0x00,0x00,
//		0x7F,0x00,0xF8,0xFF,0x07,0xC0,0x07,0x00,
//		0x00,0x7F,0x00,0xF0,0xFF,0x03,0x20,0x02,
//		0x00,0x00,0x7F,0x00,0xE0,0xFF,0x01,0xE0,
//		0x03,0x00,0x00,0xFF,0x00,0xC0,0xFF,0x00,
//		0x20,0x01,0x00,0x00,0xFF,0x00,0x00,0x3F,
//		0x00,0xF0,0x01,0x00,0x00,0xFF,0x01,0x00,
//		0x1F,0x00,0x90,0x00,0x00,0x00,0xFF,0x01,
//		0x00,0x1F,0x00,0xF8,0x00,0x00,0x00,0xFF,
//		0x03,0x00,0x1F,0x00,0x48,0x00,0x00,0x00,
//		0xFF,0x02,0x00,0x1F,0x00,0x7C,0x00,0x00,
//		0x00,0xFF,0x06,0x00,0x0E,0x00,0x24,0x00,
//		0x00,0x00,0xFF,0x0C,0x00,0x00,0x00,0x3E,
//		0x00,0x00,0x00,0xFF,0x18,0x00,0x00,0x00,
//		0x11,0x00,0x00,0x00,0xFF,0x30,0x00,0x00,
//		0x80,0x0F,0x00,0x00,0x00,0xFF,0x60,0x00,
//		0x00,0x40,0x04,0x00,0x00,0x00,0xFF,0xC0,
//		0x00,0x00,0xE0,0x03,0x00,0x00,0x00,0xFF,
//		0x80,0x01,0x00,0x30,0x01,0x00,0x00,0x00,
//		0xFF,0x00,0x07,0x00,0xEC,0x00,0x00,0x00,
//		0x00,0xFF,0x00,0x1C,0x00,0x63,0x00,0x00,
//		0x00,0x00,0xFF,0x00,0xF0,0xE1,0x1F,0x00,
//		0x00,0x00,0x00,0xFF,0x00,0x80,0xFF,0x01,
//		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
//
//};

#endif /* PERIPHERAL_MODULES_FONTS_LIBERTY_GIF_FULLCOIN_H_ */
