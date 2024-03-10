/*
 * LibG2_xmlparser.h
 *
 *  Created on: Jul 5, 2016
 *      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
 *     Project: Liberty Next Gen Single Space Meter
 */

#ifndef COMMUNICATION_MODULES_LIBG2_XMLPARSER_H_
#define COMMUNICATION_MODULES_LIBG2_XMLPARSER_H_

#include "../Main Module/LibG2_main.h"

#define NUM_XMLPARSE_MAX_ATTRS_ALLOWED         32

enum XMLPARSE_ERR
{
    XMLPARSE_ERR_None,
    XMLPARSE_ERR_Start_Inside_Start,
    XMLPARSE_ERR_Start_Inside_End,
    XMLPARSE_ERR_Callback_Returned_False,
    XMLPARSE_ERR_Improper_XML,
    XMLPARSE_ERR_Exceeds_Attribs
};

typedef struct xml_tag_attr
{
    int16_t        nm_st, nm_end, val_st, val_end;

} XMLAttrib;

typedef struct xml_tag
{
    int16_t        st, end, nm_end;

} XMLElement;

//typedef struct xml_cdata //not used //vinay
//{
//    int16_t        st, end;
//
//} XMLCData;

typedef uint16_t (*XMLParser_StartElement_Callback)(uint8_t * xml_p, XMLElement e, XMLAttrib * attribs, int16_t tot_attribs ) ;
typedef uint16_t (*XMLParser_EndElement_Callback)(uint8_t * xml_p, int16_t tag_start, int16_t tag_end ) ;
typedef uint16_t (*XMLParser_CDATA_Callback)(uint8_t * xml_p, int16_t start, int16_t end ) ;
typedef uint16_t (*XMLParser_Comment_Callback)(uint8_t * xml_p, int16_t start, int16_t end ) ;

int16_t parse_xml( uint8_t * xml_data_p,
                   uint16_t xml_sz,
                   XMLParser_StartElement_Callback start_handler,
                   XMLParser_EndElement_Callback end_handler,
                   XMLParser_CDATA_Callback cdata_handler,
                   XMLParser_Comment_Callback cmt_callback );


#endif /* COMMUNICATION_MODULES_LIBG2_XMLPARSER_H_ */
