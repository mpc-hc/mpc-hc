/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_parser.cpp,v 1.8 2006/06/12 12:10:43 asuraparaju Exp $ $Name: Dirac_0_7_0 $
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
#include <libdirac_common/frame.h>
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

    dirac_seqparams_t *seq_params = &decoder->seq_params;
    dirac_sourceparams_t *src_params = &decoder->src_params;
    dirac_parseparams_t *parse_params = &decoder->parse_params;
    const SeqParams& sparams = parser->GetSeqParams();
    const SourceParams& srcparams = parser->GetSourceParams();
    const ParseParams& pparams = parser->GetParseParams();

    parse_params->au_pnum = pparams.AccessUnitPictureNumber();
    parse_params->major_ver = pparams.MajorVersion();
    parse_params->minor_ver = pparams.MinorVersion();
    parse_params->profile = pparams.Profile();
    parse_params->level = pparams.Level();

    seq_params->width = sparams.Xl();
    seq_params->height = sparams.Yl();

    seq_params->chroma = (dirac_chroma_t)sparams.CFormat();
    switch(seq_params->chroma)
    {
    case format420:
          seq_params->chroma_width = seq_params->width/2;
            seq_params->chroma_height = seq_params->height/2;
        break;

    case format422:
            seq_params->chroma_width = seq_params->width/2;
            seq_params->chroma_height = seq_params->height;
            break;
    default:
            seq_params->chroma_width = seq_params->width;
            seq_params->chroma_height = seq_params->height;
        break;

   }
   seq_params->video_depth = sparams.GetVideoDepth();

   // set the source parmeters
    src_params->interlace = srcparams.Interlace() ? 1 : 0;
    src_params->topfieldfirst = srcparams.TopFieldFirst() ? 1 : 0;
    src_params->seqfields = srcparams.SequentialFields() ? 1 : 0;

    src_params->frame_rate.numerator = srcparams.FrameRate().m_num;
    src_params->frame_rate.denominator = srcparams.FrameRate().m_denom;

    src_params->pix_asr.numerator = srcparams.AspectRatio().m_num;
    src_params->pix_asr.denominator = srcparams.AspectRatio().m_denom;

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
    case CM_DCINEMA:
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

static void set_component (const PicArray& pic_data,  const CompSort cs, dirac_decoder_t *decoder)
{
    TEST (decoder->fbuf != NULL);
    int xl, yl;

    unsigned char *buf;

    switch (cs)
    {
    case U_COMP:
        xl = decoder->seq_params.chroma_width;
        yl = decoder->seq_params.chroma_height;
           buf = decoder->fbuf->buf[1];
        break;
    case V_COMP:
        xl = decoder->seq_params.chroma_width;
        yl = decoder->seq_params.chroma_height;
           buf = decoder->fbuf->buf[2];
        break;

    case Y_COMP:
    default:
        xl = decoder->seq_params.width;
        yl = decoder->seq_params.height;
        buf = decoder->fbuf->buf[0];
        break;
    }

    TEST (buf != NULL);

#if defined HAVE_MMX
    int last_idx = (xl>>3)<<3;
    for (int j=0 ; j<yl ;++j)
    {
        for (int i=0 ; i<last_idx ; i+=8 )
        {
            __m64 pic1 = *(__m64 *)&pic_data[j][i];
            __m64 pic2 = *(__m64 *)(&pic_data[j][i+4]);
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
            buf[j*xl+i]=(unsigned char) pic_data[j][i];                
        }//i
    }//j
    return;
#else
    for (int j=0 ; j<yl ;++j)
    {
        for (int i=0 ; i<xl ; ++i)
        {                
            buf[j*xl+i]=(unsigned char) pic_data[j][i];                
        }//i

    }//j
#endif
}

static void set_frame_data (const  DiracParser * const parser, dirac_decoder_t *decoder)
{
    TEST (parser != NULL);
    TEST (decoder != NULL);
    TEST (decoder->fbuf != NULL);
    TEST (decoder->state == STATE_PICTURE_AVAIL);

    const Frame& my_frame = parser->GetNextFrame();

    set_component (my_frame.Ydata(), Y_COMP, decoder);
    set_component (my_frame.Udata(), U_COMP, decoder);
    set_component (my_frame.Vdata(), V_COMP, decoder);

    return;
}

static void set_frame_params (const FrameParams& my_frame_params,  dirac_decoder_t *decoder)
{
    TEST (decoder != NULL);
    dirac_frameparams_t *frame_params = &decoder->frame_params;

    TEST (decoder->state == STATE_PICTURE_AVAIL ||
          decoder->state == STATE_PICTURE_START);

    frame_params->ftype = my_frame_params.FSort().IsIntra() ? INTRA_FRAME : INTER_FRAME;
    frame_params->rtype = my_frame_params.FSort().IsRef() ? REFERENCE_FRAME : NON_REFERENCE_FRAME;
    frame_params->fnum = my_frame_params.FrameNum();

    return;
}

extern DllExport dirac_decoder_state_t dirac_parse (dirac_decoder_t *decoder)
{
    TEST (decoder != NULL);
    TEST (decoder->parser != NULL);
    DiracParser *parser = static_cast<DiracParser *>(decoder->parser);

    try 
    {
        decoder->state = parser->Parse();

        switch (decoder->state)
        {
        case STATE_BUFFER:
            break;

        case STATE_SEQUENCE:
            set_sequence_params(parser, decoder);
            decoder->frame_avail = 0;
            break;

        case STATE_PICTURE_START:
            /* frame params of the frame being decoded in coding order */
            set_frame_params (parser->GetNextFrameParams(), decoder);
            decoder->frame_avail = 0;
            break;

        case STATE_PICTURE_AVAIL:
            decoder->frame_avail = 1;
            /* frame params of the frame available for display */
            set_frame_params (parser->GetNextFrame().GetFparams(), decoder);
            set_frame_data (parser, decoder);
            break;

        case STATE_INVALID:
            break;

    default:
        break;
        }
    }//try 
     catch (const DiracException& e) {
         return STATE_INVALID;
     }            


    return decoder->state;
}

extern DllExport void dirac_skip (dirac_decoder_t *decoder, int skip)
{
    TEST (decoder != NULL);
    TEST (decoder->parser != NULL);
    DiracParser *parser = static_cast<DiracParser *>(decoder->parser);

    parser->SetSkip(skip > 0 ? true : false);
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
