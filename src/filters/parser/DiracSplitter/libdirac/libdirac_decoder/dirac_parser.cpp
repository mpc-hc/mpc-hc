/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_parser.cpp,v 1.22 2008/06/19 10:33:24 tjdwave Exp $ $Name:  $
*
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License
* Version 1.1 (the "License"); you may not use this file except in compliance
* with the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
* the specific language governing rights and limitations under the License.
*
* The Original Code is BBC Research and Development code.
*
* The Initial Developer of the Original Code is the British Broadcasting
* Corporation.
* Portions created by the Initial Developer are Copyright (C) 2004.
* All Rights Reserved.
*
* Contributor(s): Anuradha Suraparaju (Original Author)
*                 Andrew Kennedy
*
* Alternatively, the contents of this file may be used under the terms of
* the GNU General Public License Version 2 (the "GPL"), or the GNU Lesser
* Public License Version 2.1 (the "LGPL"), in which case the provisions of
* the GPL or the LGPL are applicable instead of those above. If you wish to
* allow use of your version of this file only under the terms of the either
* the GPL or LGPL and not to allow others to use your version of this file
* under the MPL, indicate your decision by deleting the provisions above
* and replace them with the notice and other provisions required by the GPL
* or LGPL. If you do not delete the provisions above, a recipient may use
* your version of this file under the terms of any one of the MPL, the GPL
* or the LGPL.
* ***** END LICENSE BLOCK ***** */

#include <cstring>
#include <libdirac_common/dirac_assertions.h>
#include <libdirac_decoder/dirac_cppparser.h>
#include <libdirac_decoder/dirac_parser.h>
#include <libdirac_common/dirac_exception.h>
#include <libdirac_common/picture.h>
#if defined (HAVE_MMX)
#include <mmintrin.h>

#endif
using namespace dirac;

#ifdef __cplusplus
extern "C" {
#endif

extern DllExport dirac_decoder_t *dirac_decoder_init(int verbose)
{
    dirac_decoder_t* decoder = new dirac_decoder_t;
    memset (decoder, 0, sizeof(dirac_decoder_t));

    bool verbosity = verbose > 0 ? true : false;
    DiracParser *parser = new DiracParser(verbosity);
    decoder->parser = static_cast<void *>(parser);

    decoder->fbuf = new dirac_framebuf_t;
    decoder->fbuf->id = NULL;
    decoder->fbuf->buf[0] = decoder->fbuf->buf[1] = decoder->fbuf->buf[2] = NULL;
    return decoder;
}

extern DllExport void dirac_decoder_close(dirac_decoder_t *decoder)
{
    TEST (decoder != NULL);
    TEST (decoder->parser != NULL);
    DiracParser *parser = static_cast<DiracParser *>(decoder->parser);

    delete parser;

    delete decoder->fbuf;

    delete decoder;

    decoder = NULL;
}

extern DllExport void dirac_buffer (dirac_decoder_t *decoder, unsigned char *start, unsigned char *end)
{
    TEST (decoder != NULL);
    TEST (decoder->parser != NULL);
    DiracParser *parser = static_cast<DiracParser *>(decoder->parser);

    parser->SetBuffer((char *)start, (char *)end);
}

static void set_sequence_params (const  DiracParser * const parser, dirac_decoder_t *decoder)
{
    TEST (parser != NULL);
    TEST (decoder != NULL);

    dirac_sourceparams_t *src_params = &decoder->src_params;
    dirac_parseparams_t *parse_params = &decoder->parse_params;
    const SourceParams& srcparams = parser->GetSourceParams();
    const ParseParams& pparams = parser->GetParseParams();

    parse_params->major_ver = pparams.MajorVersion();
    parse_params->minor_ver = pparams.MinorVersion();
    parse_params->profile = pparams.Profile();
    parse_params->level = pparams.Level();

    src_params->width = srcparams.Xl();
    src_params->height = srcparams.Yl();

    src_params->chroma = (dirac_chroma_t)srcparams.CFormat();
    src_params->chroma_width = srcparams.ChromaWidth();
    src_params->chroma_height = srcparams.ChromaHeight();

   // set the source parmeters
    src_params->source_sampling = srcparams.SourceSampling();
    src_params->topfieldfirst = srcparams.TopFieldFirst() ? 1 : 0;

    src_params->frame_rate.numerator = srcparams.FrameRate().m_num;
    src_params->frame_rate.denominator = srcparams.FrameRate().m_denom;

    src_params->pix_asr.numerator = srcparams.PixelAspectRatio().m_num;
    src_params->pix_asr.denominator = srcparams.PixelAspectRatio().m_denom;

    // clean area
    src_params->clean_area.width = srcparams.CleanWidth();
    src_params->clean_area.height = srcparams.CleanHeight();
    src_params->clean_area.left_offset = srcparams.LeftOffset();
    src_params->clean_area.top_offset = srcparams.TopOffset();

    // signal range
    src_params->signal_range.luma_offset = srcparams.LumaOffset();
    src_params->signal_range.luma_excursion = srcparams.LumaExcursion();
    src_params->signal_range.chroma_offset = srcparams.ChromaOffset();
    src_params->signal_range.chroma_excursion = srcparams.ChromaExcursion();

    // Colour specfication
    src_params->colour_spec.col_primary = srcparams.ColourPrimariesIndex();
    src_params->colour_spec.trans_func = srcparams.TransferFunctionIndex();
    switch(srcparams.ColourMatrixIndex())
    {
    case CM_SDTV:
        src_params->colour_spec.col_matrix.kr =  0.299f;
        src_params->colour_spec.col_matrix.kb =  0.114f;
        break;
    case CM_REVERSIBLE:
        src_params->colour_spec.col_matrix.kr =  0.25f;
        src_params->colour_spec.col_matrix.kb =  0.25f;
        break;
    case CM_HDTV_COMP_INTERNET:
    default:
        src_params->colour_spec.col_matrix.kr = 0.2126f;
        src_params->colour_spec.col_matrix.kb = 0.0722f;
        break;
    }
}

static void set_frame_component (const PicArray& pic_data,  const CompSort cs, dirac_decoder_t *decoder)
{
    TEST (decoder->fbuf != NULL);
    int xl, yl;

    unsigned char *buf;

    switch (cs)
    {
    case U_COMP:
        xl = decoder->src_params.chroma_width;
        yl = decoder->src_params.chroma_height;
        buf = decoder->fbuf->buf[1];
        break;
    case V_COMP:
        xl = decoder->src_params.chroma_width;
        yl = decoder->src_params.chroma_height;
        buf = decoder->fbuf->buf[2];
        break;

    case Y_COMP:
    default:
        xl = decoder->src_params.width;
        yl = decoder->src_params.height;
        buf = decoder->fbuf->buf[0];
        break;
    }

    TEST (buf != NULL);

#if defined HAVE_MMX
    int last_idx = (xl>>3)<<3;
    __m64 tmp = _mm_set_pi16(128, 128, 128, 128);
    for (int j=0 ; j<yl ;++j)
    {
        for (int i=0 ; i<last_idx ; i+=8 )
        {
            __m64 pic1 = *(__m64 *)&pic_data[j][i];
            pic1 = _mm_add_pi16 (pic1, tmp);
            __m64 pic2 = *(__m64 *)(&pic_data[j][i+4]);
            pic2 = _mm_add_pi16 (pic2, tmp);
            __m64 *tmp = (__m64 *)&buf[j*xl+i];
            *tmp = _mm_packs_pu16 (pic1, pic2);
        }//i
    }//j
    _mm_empty();

    // mop up remaining pixels
    for (int j=0 ; j<yl ;++j)
    {
        for (int i=last_idx ; i<xl ; i++ )
        {
            buf[j*xl+i]=(unsigned char) (pic_data[j][i]+128);
        }//i
    }//j
    return;
#else

    for (int j=0 ; j<yl ;++j)
    {
        for (int i=0 ; i<xl ; ++i)
        {
            buf[j*xl+i]=(unsigned char) (pic_data[j][i]+128);
        }//i
    }//j

#endif
}

static void set_field_component (const PicArray& pic_data,  const CompSort cs, dirac_decoder_t *decoder, unsigned int pic_num)
{
    TEST (decoder->fbuf != NULL);
    int xl, yl;

    unsigned char *buf;

    switch (cs)
    {
    case U_COMP:
        xl = decoder->src_params.chroma_width;
        yl = decoder->src_params.chroma_height;
        buf = decoder->fbuf->buf[1];
        break;
    case V_COMP:
        xl = decoder->src_params.chroma_width;
        yl = decoder->src_params.chroma_height;
        buf = decoder->fbuf->buf[2];
        break;

    case Y_COMP:
    default:
        xl = decoder->src_params.width;
        yl = decoder->src_params.height;
        buf = decoder->fbuf->buf[0];
        break;
    }

    TEST (buf != NULL);

    // Seek offset before writing field to store
    int start = 0;
    // Seek offset between writing fields to store
    int skip = 0;


    bool top_field = decoder->src_params.topfieldfirst ? (!(pic_num%2)) :
                    (pic_num%2);

    if (top_field) // i.e. top field
    {
        start = 0;
        skip = 2 * xl * sizeof(char);
    }
    else // else bottom field
    {
        start = xl;
        skip = 2 * xl * sizeof(char);
    }

    unsigned char *tempc = buf + start;

    int field_yl = yl>>1;
    int field_xl = xl;

    for (int j=0 ; j<field_yl ;++j)
    {
        for (int i=0 ; i<field_xl ; ++i)
        {
            tempc[i] = (unsigned char) (pic_data[j][i]+128);
        }//I
        tempc += skip;
    }
}

static void set_frame_data (const  DiracParser * const parser, dirac_decoder_t *decoder)
{
    TEST (parser != NULL);
    TEST (decoder != NULL);
    TEST (decoder->fbuf != NULL);
    TEST (decoder->state == STATE_PICTURE_AVAIL);

    const Picture* my_picture = parser->GetNextPicture();

    if (my_picture)
    {
        int pic_num = my_picture->GetPparams().PictureNum();

        if (!parser->GetDecoderParams().FieldCoding())
        {
            set_frame_component (my_picture->Data(Y_COMP), Y_COMP, decoder);
            set_frame_component (my_picture->Data(U_COMP), U_COMP, decoder);
            set_frame_component (my_picture->Data(V_COMP), V_COMP, decoder);
        }
        else
        {
            set_field_component (my_picture->Data(Y_COMP), Y_COMP, decoder, pic_num);
            set_field_component (my_picture->Data(U_COMP), U_COMP, decoder, pic_num);
            set_field_component (my_picture->Data(V_COMP), V_COMP, decoder, pic_num);
        }
    }
    return;
}

extern DllExport dirac_decoder_state_t dirac_parse (dirac_decoder_t *decoder)
{
    TEST (decoder != NULL);
    TEST (decoder->parser != NULL);
    DiracParser *parser = static_cast<DiracParser *>(decoder->parser);

    unsigned int pic_num;

    while(true)
    {
        try
        {
            decoder->state = parser->Parse();

            switch (decoder->state)
            {
            case STATE_BUFFER:
                return decoder->state;
                break;

            case STATE_SEQUENCE:
                set_sequence_params(parser, decoder);
                decoder->frame_avail = 0;
                return decoder->state;
                break;

            case STATE_PICTURE_AVAIL:
            {
                const Picture *my_picture = parser->GetNextPicture();
                if (my_picture)
                {
                    pic_num = parser->GetNextPicture()->GetPparams().PictureNum();
                    decoder->frame_num = pic_num;
                    set_frame_data (parser, decoder);

                    /* A full frame is only available if we're doing
                    * progressive coding or have decoded the second field.
                    * Will only return when a full frame is available
                    */
                    if (!parser->GetDecoderParams().FieldCoding() ||
                        pic_num%2)
                    {
                        /* Frame number currently available for display */
                        decoder->frame_num = pic_num;
                        if (parser->GetDecoderParams().FieldCoding())
                            decoder->frame_num = pic_num>>1;
                        decoder->frame_avail = 1;
                        return decoder->state;
                    }
                }
                break;
            }

            case STATE_INVALID:
                return decoder->state;
                break;

            case STATE_SEQUENCE_END:
                return decoder->state;
                break;

            default:
                break;
            }
        }//try
        catch (const DiracException& e)
        {
             return STATE_INVALID;
        }
    }

    return decoder->state;
}

extern DllExport void dirac_set_buf (dirac_decoder_t *decoder, unsigned char *buf[3], void *id)
{
    TEST (decoder != NULL);
    TEST (decoder->fbuf != NULL);

    decoder->fbuf->buf[0] = buf[0];
    decoder->fbuf->buf[1] = buf[1];
    decoder->fbuf->buf[2] = buf[2];
    decoder->fbuf->id  = id;
}

#ifdef __cplusplus
}
#endif
