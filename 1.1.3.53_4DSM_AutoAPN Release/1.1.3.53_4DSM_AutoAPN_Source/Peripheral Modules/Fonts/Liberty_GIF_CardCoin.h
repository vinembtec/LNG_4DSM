/*
 * Liberty_GIF_CardCoin.h
 *
 *  Created on: Oct 21, 2016
 *      Author: admin
 */

#ifndef PERIPHERAL_MODULES_FONTS_LIBERTY_GIF_CARDCOIN_H_
#define PERIPHERAL_MODULES_FONTS_LIBERTY_GIF_CARDCOIN_H_

//fontgraph6:
// full card & coin graphic
// fontcharbytes16_graph6: = 0x288    // bytes per char
// fontbyteswide_graph6: = 0x9       // bytes required for width of each char
// fontheight_graph6: = 0x48          // pixels in height
//const uint8_t fontGIF6[1944] =
//{
//		//CARD																			//GAP															//COIN
//		0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x00,0x80,0xFF,0x07,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x00,0x1C,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x00,0xF0,0xC0,0x39,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x00,0x3E,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x00,0x1E,0x00,0x7F,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x00,0x07,0x00,0x88,0x01,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x80,0xFF,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x80,0x01,0x00,0xF0,0x01,0x00,0x00,0x00,
//		0x00,0x00,0x00,0xC0,0xFF,0x01,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0xC0,0x00,0x00,0x20,0x06,0x00,0x00,0x00,
//		0x00,0x00,0x00,0xE0,0xF7,0x03,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x60,0x00,0x00,0xC0,0x07,0x00,0x00,0x00,
//		0x00,0x00,0x00,0xF0,0xEB,0x07,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x30,0x00,0x00,0x80,0x08,0x00,0x00,0x00,
//		0x00,0x00,0x00,0xF8,0xD5,0x03,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x18,0x00,0x0E,0x00,0x1F,0x00,0x00,0x00,
//		0x00,0x00,0x00,0xFC,0xAA,0x01,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x0C,0x00,0x1F,0x00,0x12,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x7E,0x15,0x01,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x06,0x00,0x1F,0x00,0x3E,0x00,0x00,0x00,
//		0x00,0x00,0x00,0xFF,0x0A,0x06,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x02,0x00,0x1F,0x00,0x64,0x00,0x00,0x00,
//		0x00,0x00,0x80,0xFF,0x05,0x08,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x03,0x00,0x1F,0x00,0x78,0x00,0x00,0x00,
//		0x00,0x00,0xC0,0xFF,0x03,0x30,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x01,0x00,0x1F,0x00,0x88,0x00,0x00,0x00,
//		0x00,0x00,0xE0,0xFF,0x07,0x40,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x00,0x00,0x1F,0x00,0xF0,0x01,0x00,0x00,
//		0x00,0x00,0xF0,0xFF,0x1F,0x80,0x01,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x00,0x00,0x1F,0x00,0x10,0x01,0x00,0x00,
//		0x00,0x00,0xF8,0xFD,0x3F,0x00,0x02,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x7F,0x00,0x00,0x3F,0x00,0xE0,0x01,0x00,0x00,
//		0x00,0x00,0xFC,0xFA,0x7F,0x00,0x0C,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x7F,0x00,0xC0,0xFF,0x00,0x20,0x02,0x00,0x00,
//		0x00,0x00,0x7E,0xF5,0xFF,0x00,0x10,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x7F,0x00,0xE0,0xFF,0x01,0xE0,0x03,0x00,0x00,
//		0x00,0x00,0xBF,0xCA,0xFF,0x03,0x60,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x3F,0x00,0xF0,0xFF,0x03,0x40,0x06,0x00,0x00,
//		0x00,0x80,0x5F,0x85,0xFF,0x07,0x80,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x3F,0x00,0xF8,0xFF,0x07,0xC0,0x07,0x00,0x00,
//		0x00,0xC0,0xAF,0x02,0xFF,0x0F,0x00,0x01,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x3F,0x00,0xFC,0xFF,0x0F,0x40,0x04,0x00,0x00,
//		0x00,0xE0,0x57,0x01,0xFE,0x3F,0x00,0x06,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x3F,0x00,0xFC,0xDF,0x0F,0x80,0x07,0x00,0x00,
//		0x00,0xF0,0xAB,0x00,0xFC,0x7F,0x00,0x08,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x3F,0x00,0x7E,0x9F,0x1F,0x80,0x08,0x00,0x00,
//		0x00,0xF8,0x55,0x00,0xF8,0xFF,0x00,0x30,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x1F,0x00,0x7E,0x1F,0x1F,0x80,0x0F,0x00,0x00,
//		0x00,0xFC,0x2A,0x00,0xF0,0xFF,0x01,0x40,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x1F,0x00,0x3E,0x1F,0x3F,0x80,0x08,0x00,0x00,
//		0x00,0x7E,0x15,0x00,0xE0,0xFF,0x07,0x80,0x01,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x1F,0x00,0x3F,0x1F,0x3E,0x80,0x0F,0x00,0x00,
//		0x00,0xBF,0x0A,0x00,0xC0,0xFF,0x0F,0x00,0x02,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x1F,0x00,0x1F,0x1F,0x3E,0x00,0x19,0x00,0x00,
//		0x80,0x5F,0x05,0x00,0x80,0xFF,0x1F,0x00,0x0C,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x1F,0x00,0x1F,0x1F,0x00,0x00,0x1F,0x00,0x00,
//		0xC0,0xAF,0x02,0x00,0x00,0xFF,0x7F,0x00,0x10,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x0F,0x00,0x1F,0x1F,0x00,0x00,0x11,0x00,0x00,
//		0xE0,0x57,0x01,0x00,0x00,0xFE,0xFF,0x00,0x60,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x0F,0x80,0x0F,0x1F,0x00,0x00,0x1F,0x00,0x00,
//		0xF0,0xAB,0x00,0x00,0x00,0xFC,0xFF,0x01,0x80,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x0F,0x80,0x0F,0x1F,0x00,0x00,0x11,0x00,0x00,
//		0xF8,0x55,0x00,0x00,0x00,0xF8,0xFF,0x03,0x80,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x0F,0x80,0x0F,0x1F,0x00,0x00,0x1F,0x00,0x00,
//		0xFC,0x2A,0x00,0x00,0x00,0xF0,0xFF,0x0F,0xC0,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x0F,0x80,0x0F,0x1F,0x00,0x00,0x11,0x00,0x00,
//		0x7E,0x15,0x00,0x00,0x00,0xE0,0xFF,0x1F,0xE0,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x0F,0x80,0x0F,0x1F,0x00,0x00,0x1F,0x00,0x00,
//		0xBF,0x0A,0x00,0x00,0x00,0xC0,0xFF,0x3F,0x70,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x0F,0x80,0x0F,0x1F,0x00,0x00,0x11,0x00,0x00,
//		0x7E,0x05,0x00,0x00,0x00,0x80,0xFF,0xFF,0x38,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x0F,0x80,0x0F,0x1F,0x00,0x00,0x1F,0x00,0x00,
//		0xFC,0x02,0x00,0x00,0x00,0x00,0xFF,0xFF,0x1D,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x1F,0x80,0x0F,0x1F,0x00,0x00,0x11,0x00,0x00,
//		0xF8,0x01,0x00,0x00,0x00,0x00,0xFE,0xFF,0x0F,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x1F,0x80,0x0F,0x1F,0x00,0x00,0x1F,0x00,0x00,
//		0xF0,0x03,0x00,0x00,0x00,0x00,0xFC,0xFF,0x07,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x1F,0x80,0x1F,0x1F,0x00,0x00,0x11,0x00,0x00,
//		0xE0,0x02,0x00,0x00,0x00,0x00,0xF8,0xFF,0x03,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x1F,0x00,0x1F,0x1F,0x3E,0x00,0x1F,0x00,0x00,
//		0x40,0x04,0x00,0x00,0x00,0x00,0xE0,0xFF,0x01,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x3F,0x00,0x1F,0x1F,0x3E,0x80,0x08,0x00,0x00,
//		0x00,0x08,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x3F,0x00,0x3F,0x1F,0x3E,0x80,0x0F,0x00,0x00,
//		0x00,0x10,0x00,0x00,0x00,0x00,0x80,0x7F,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x3F,0x00,0x3E,0x1F,0x3F,0x80,0x08,0x00,0x00,
//		0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x3F,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x3F,0x00,0x3E,0x1F,0x1F,0x80,0x0F,0x00,0x00,
//		0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x1E,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x3F,0x00,0x7E,0x9F,0x1F,0x80,0x0C,0x00,0x00,
//		0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x0E,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x7F,0x00,0xFC,0xDF,0x0F,0xC0,0x07,0x00,0x00,
//		0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x07,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x7F,0x00,0xFC,0xFF,0x0F,0x40,0x04,0x00,0x00,
//		0x00,0x80,0x00,0x00,0x00,0x00,0x80,0x03,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x7F,0x00,0xF8,0xFF,0x07,0xC0,0x07,0x00,0x00,
//		0x00,0x00,0x01,0x00,0x00,0x00,0xC0,0x01,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x7F,0x00,0xF0,0xFF,0x03,0x20,0x02,0x00,0x00,
//		0x00,0x00,0x02,0x00,0x00,0x00,0xE0,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x7F,0x00,0xE0,0xFF,0x01,0xE0,0x03,0x00,0x00,
//		0x00,0x00,0x04,0x00,0x00,0x00,0x70,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x00,0xC0,0xFF,0x00,0x20,0x01,0x00,0x00,
//		0x00,0x00,0x04,0x00,0x00,0x00,0x38,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x00,0x00,0x3F,0x00,0xF0,0x01,0x00,0x00,
//		0x00,0x00,0x08,0x00,0x00,0x00,0x1C,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x01,0x00,0x1F,0x00,0x90,0x00,0x00,0x00,
//		0x00,0x00,0x10,0x00,0x00,0x00,0x0E,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x01,0x00,0x1F,0x00,0xF8,0x00,0x00,0x00,
//		0x00,0x00,0x20,0x00,0x00,0x00,0x07,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x03,0x00,0x1F,0x00,0x48,0x00,0x00,0x00,
//		0x00,0x00,0x20,0x00,0x00,0x80,0x03,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x02,0x00,0x1F,0x00,0x7C,0x00,0x00,0x00,
//		0x00,0x00,0x40,0x00,0x00,0xC0,0x01,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x06,0x00,0x0E,0x00,0x24,0x00,0x00,0x00,
//		0x00,0x00,0x80,0x00,0x00,0xE0,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x0C,0x00,0x00,0x00,0x3E,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x01,0x00,0x70,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x18,0x00,0x00,0x00,0x11,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x01,0x00,0x38,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x30,0x00,0x00,0x80,0x0F,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x02,0x00,0x1C,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x60,0x00,0x00,0x40,0x04,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x04,0x00,0x0E,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0xC0,0x00,0x00,0xE0,0x03,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x08,0x00,0x07,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x80,0x01,0x00,0x30,0x01,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x08,0x80,0x03,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x00,0x07,0x00,0xEC,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x10,0xC0,0x01,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x00,0x1C,0x00,0x63,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x20,0xE0,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x00,0xF0,0xE1,0x1F,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x40,0x70,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0xFF,0x00,0x80,0xFF,0x01,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x40,0x38,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x80,0x1C,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x80,0x0F,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
//
//};


#endif /* PERIPHERAL_MODULES_FONTS_LIBERTY_GIF_CARDCOIN_H_ */
