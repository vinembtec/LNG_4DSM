//*****************************************************************************
//*  Created on: Jul 5, 2016
//*      Author: EMQOS Embedded Engineering Pvt. Ltd. for CivicSmart, Inc
//*     Project: Liberty Next Gen Single Space Meter
// MSP432 LibG2_xmlparser.c
//
//****************************************************************************

#include "LibG2_xmlparser.h"

static XMLElement      elmt;
static XMLAttrib       attribs[ NUM_XMLPARSE_MAX_ATTRS_ALLOWED ];
static int16_t         attr_idx;

static int16_t  under_st_elmt , under_end_elmt ,
                under_att_nm, under_att_val,
                under_cdata, under_sngl_qts,
                under_dbl_qts, whtspc_found, under_cmt;

static int16_t  st_elmt_st_idx, st_tag_end_idx, end_elmt_st_idx,
                att_nm_st_idx, att_nm_end_idx, att_val_st_idx,
                cdata_st_idx, cmt_st_idx, cmt_end_idx;

static int16_t cleanup_attribs( uint8_t * xml_data_p, int16_t xml_sz  )
{
	XMLAttrib    * att;
	uint8_t        sch;
	uint8_t        ech;
	int16_t        a;

	for( a = 0; a < attr_idx; a ++ )
	{
		att = attribs + a;
		sch = xml_data_p[ att->val_st ];
		ech = xml_data_p[ att->val_end ];

		while ( att->val_st < xml_sz && ( sch == ' ' || sch == '\"' || sch == '\t' || sch == '\n' || sch == '\r' ) )
		{
			att->val_st ++;
			sch = xml_data_p[ att->val_st ];
		}

		while ( att->val_st < att->val_end && ( ech == ' ' || ech == '\"' || ech == '\t' || ech == '\n' || ech == '\r' ) )
		{
			att->val_end --;
			ech = xml_data_p[ att->val_end ];
		}

		sch = xml_data_p[ att->nm_st ];
		ech = xml_data_p[ att->nm_end ];

		while ( att->nm_st < xml_sz && ( sch == ' ' || sch == '\t' || sch == '\n' || sch == '\r' ) )
		{
			att->nm_st ++;
			sch = xml_data_p[ att->nm_st ];
		}

		while ( att->nm_st < att->nm_end && ( ech == ' ' || ech == '\t' || ech == '\n' || ech == '\r' ) )
		{
			att->nm_end --;
			ech = xml_data_p[ att->nm_end ];
		}
	}

	return 0;
}

int16_t parse_xml( uint8_t * xml_data_p, uint16_t xml_sz, XMLParser_StartElement_Callback start_handler,
                XMLParser_EndElement_Callback end_handler,  XMLParser_CDATA_Callback cdata_handler,
                XMLParser_Comment_Callback cmt_callback )
{
	uint8_t       ch;
	int16_t   stidx;
	XMLAttrib    * a;
	int16_t   prev_c;

	int16_t        c = 0;
	int16_t        error = XMLPARSE_ERR_None;

	under_st_elmt = false;
	under_end_elmt = false;
	under_att_nm = false;
	under_att_val = false;
	under_cdata = false;
	under_sngl_qts = false;
	under_dbl_qts = false;
	whtspc_found = false;
	under_cmt = false;
	st_elmt_st_idx = -1;
	st_tag_end_idx = -1;
	end_elmt_st_idx = -1;
	att_nm_st_idx = -1;
	att_nm_end_idx = -1;
	att_val_st_idx = -1;
	cdata_st_idx = -1;
	cmt_st_idx = -1;
	cmt_end_idx = -1;

	attr_idx = 0;
	memset( attribs, 0, sizeof( attribs ) );

	while ( c < xml_sz )
	{
		ch = xml_data_p[c];

		switch ( ch )
		{
		case '\"':
		if ( under_dbl_qts == true )
			under_dbl_qts = false;
		else
			under_dbl_qts = true;
		break;

		case '\'':
			if ( under_sngl_qts == true )
				under_sngl_qts = false;
			else
				under_sngl_qts = true;
			break;

		case '<':
			if ( under_sngl_qts == true || under_dbl_qts == true || under_cmt == true )
			{
				break;
			}
			if ( under_cdata == true )
			{
				stidx = cdata_st_idx;

				under_cdata     = false;

				cdata_st_idx = -1;

				if ( stidx != ~0 )
				{
					if ( cdata_handler( xml_data_p, stidx, c-1 ) == false )
					{
						error             = XMLPARSE_ERR_Callback_Returned_False;
						c                 = xml_sz;
						break;
					}
				}
				else
				{
					error     = XMLPARSE_ERR_Improper_XML;
					c         = xml_sz;
					break;
				}
			}

			if ( under_cmt == false      &&
					c < xml_sz - 3          &&
					xml_data_p[c+1] == '!'  &&
					xml_data_p[c+2] == '-'  &&
					xml_data_p[c+3] == '-' )
			{
				under_cmt             = true;
				c                     += 3;
				cmt_st_idx            = c;
				break;
			}

			if ( under_st_elmt == true )
			{
				error     = XMLPARSE_ERR_Start_Inside_Start;
				c         = xml_sz;
				break;
			}

			if ( under_end_elmt == true )
			{
				error     = XMLPARSE_ERR_Start_Inside_End;
				c         = xml_sz;
				break;
			}

			under_st_elmt       = true;
			st_elmt_st_idx      = c + 1;
			st_tag_end_idx      = -1;
			end_elmt_st_idx     = -1;
			break;

		case '-':
			if ( under_cmt == true      &&
					c < xml_sz - 2         &&
					xml_data_p[c+1] == '-' &&
					xml_data_p[c+2] == '>' )
			{
				under_cmt             = false;
				cmt_end_idx            = c;

				if ( cmt_callback( xml_data_p, cmt_st_idx, cmt_end_idx ) == false )
				{
					error     = XMLPARSE_ERR_Callback_Returned_False;
					c         = xml_sz;
				}
			}
			break;

		case '/':
			/*    //2410: test
            if ( under_sngl_qts == false && under_dbl_qts == false && under_cmt == false 
            && under_cdata == false && under_st_elmt == true )
            {
            if ( c < xml_sz - 1 && xml_data_p[c+1] == '>' )
            {
            elmt.st = st_elmt_st_idx;
            elmt.end = c-1;
            elmt.nm_end = st_tag_end_idx;

            cleanup_attribs( xml_data_p, xml_sz );
            if ( start_handler( xml_data_p, elmt, attribs, attr_idx ) == false )
            {
            error     = XMLPARSE_ERR_Callback_Returned_False;
            c         = xml_sz;
            break;
            }

            attr_idx = 0;
            memset( attribs, 0, sizeof(attribs) );

            under_st_elmt         = false;
            under_end_elmt         = false;
            under_att_nm        = false;
            under_att_val        = false;

            st_elmt_st_idx         = -1;
            st_tag_end_idx         = -1;
            end_elmt_st_idx     = -1;
            att_nm_end_idx        = -1;
            att_nm_st_idx        = -1;
            att_val_st_idx        = -1;

            if ( end_handler( xml_data_p, elmt.st, elmt.end  ) == false )
            {
            error     = XMLPARSE_ERR_Callback_Returned_False;
            c         = xml_sz;
            break;
            }

            c ++;
            break;
            }

            under_st_elmt         = false;
            under_end_elmt         = true;
            end_elmt_st_idx     = c + 1;
            }
			 */    //2410: test
			break;

		case '>':
			if ( under_sngl_qts == false &&
					under_dbl_qts == false   &&
					under_cmt == false       &&
					under_cdata == false  )
			{
				if ( under_st_elmt == true )
				{
					if ( under_att_val == true )
					{
						/* unprocessed attrib */

						if ( attr_idx >= NUM_XMLPARSE_MAX_ATTRS_ALLOWED )
						{
							error     = XMLPARSE_ERR_Exceeds_Attribs;
							c         = xml_sz;
							break;
						}

						a                = attribs + attr_idx;
						a->nm_st         = att_nm_st_idx;
						a->nm_end        = att_nm_end_idx;
						a->val_st        = att_val_st_idx;
						a->val_end       = c - 1;

						attr_idx ++;

						under_att_val     = false;
						att_val_st_idx     = -1;
					}

					elmt.st = st_elmt_st_idx;
					elmt.end = c-1;
					elmt.nm_end = st_tag_end_idx;

					cleanup_attribs( xml_data_p, xml_sz );
					if ( start_handler( xml_data_p, elmt, attribs, attr_idx ) == false )
					{
						error     = XMLPARSE_ERR_Callback_Returned_False;
						c         = xml_sz;
						break;
					}

					attr_idx = 0;
					memset( attribs, 0, sizeof(attribs) );

					under_st_elmt       = false;
					under_end_elmt      = false;
					under_att_nm        = false;
					under_att_val       = false;

					st_elmt_st_idx      = -1;
					st_tag_end_idx      = -1;
					end_elmt_st_idx     = -1;
					att_nm_end_idx      = -1;
					att_nm_st_idx       = -1;
					att_val_st_idx      = -1;
				}
				else if ( under_end_elmt == true )
				{
					if ( end_handler( xml_data_p, end_elmt_st_idx, c-1  ) == false )
					{
						error     = XMLPARSE_ERR_Callback_Returned_False;
						c         = xml_sz;
						break;
					}

					end_elmt_st_idx = -1;
					under_end_elmt = false;
				}
			}
			break;

		case '=':
			if ( under_sngl_qts == false &&
					under_dbl_qts == false  &&
					under_cmt == false      &&
					under_cdata == false    &&
					under_st_elmt == true )
			{
				att_nm_end_idx     = c - 1;
				att_val_st_idx     = c + 1;
				under_att_nm     = false;
				under_att_val    = true;
			}
			break;

		default:
		{
			prev_c = c;
			if ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' )
			{
				whtspc_found = true;

				while ( c < xml_sz-1 && (xml_data_p[c+1] == ' ') || xml_data_p[c+1] == '\t' || xml_data_p[c+1] == '\n' || xml_data_p[c+1] == '\r' )
				{
					c++;
				}
				ch = xml_data_p[c];
			}
			else
			{
				whtspc_found = false;
			}

			if ( whtspc_found == true )
			{
				if ( under_sngl_qts == false && under_dbl_qts == false && under_cmt == false )
				{
					if ( under_st_elmt == true )
					{
						if ( under_att_nm == true )
						{
							/* probably name without value ?? */
									;
						}
						else if ( under_att_val == true )
						{
							/* next attrib starts */
							if ( attr_idx >= NUM_XMLPARSE_MAX_ATTRS_ALLOWED )
							{
								error     = XMLPARSE_ERR_Exceeds_Attribs;
								c         = xml_sz;
								break;
							}

							a = attribs + attr_idx;

							a->nm_st         = att_nm_st_idx;
							a->nm_end        = att_nm_end_idx;
							a->val_st        = att_val_st_idx;
							a->val_end       = prev_c - 1;

							attr_idx ++;

							under_att_val    = false;
							att_val_st_idx   = -1;
							att_nm_st_idx    = c;
							att_nm_end_idx   = c;
						}
						else
						{
							if ( st_elmt_st_idx == -1 )
							{
								error     = XMLPARSE_ERR_Improper_XML;
								c         = xml_sz;
								break;
							}

							st_tag_end_idx   = prev_c - 1;
							att_nm_st_idx    = c;
							under_att_nm     = true;
						}
					}
				}
			}

			if ( under_sngl_qts == false &&
					under_dbl_qts == false  &&
					under_cmt == false      &&
					under_cdata == false    &&
					under_st_elmt == false  &&
					under_end_elmt == false )
			{
				under_cdata     = true;
				cdata_st_idx    = c;
			}
			break;
		}

		} /* switch */

		c++;
	}

	return error;
}

