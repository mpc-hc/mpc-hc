/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_parser.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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
#include <libdirac_common/frame.h>
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
    const SeqParams& sparams = parser->GetSeqParams();

    seq_params->width = sparams.Xl();
    seq_params->height = sparams.Yl();

    ///TODO: how do we sync definition of Chroma in dirac_parser.cpp 
    // with Chroma is common.h
    seq_params->chroma = (dirac_chroma_t)sparams.CFormat();
    switch(seq_params->chroma)
    {
    case format411:
            seq_params->chroma_width = seq_params->width/4;
            seq_params->chroma_height = seq_params->height;
        break;

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

    // NOTE: frame rate will be replaced by a struct holding numerator
    //       and denominator values.
    seq_params->frame_rate.numerator = sparams.FrameRate();
    seq_params->frame_rate.denominator = 1;
    seq_params->interlace = sparams.Interlace() ? 1 : 0;
    seq_params->topfieldfirst = sparams.TopFieldFirst() ? 1 : 0;
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
    ValueType tempv;

    for (int j=0 ; j<yl ;++j)
    {
        for (int i=0 ; i<xl ; ++i)
        {                
            tempv=pic_data[j][i]+2;
            tempv>>=2;
            buf[j*xl+i]=(unsigned char) tempv;                
        }//i

    }//j
}

static void set_frame_data (const  DiracParser * const parser, dirac_decoder_t *decoder)
{
    TEST (parser != NULL);
    TEST (decoder != NULL);
    TEST (decoder->fbuf != NULL);
    TEST (decoder->state == STATE_PICTURE_AVAIL);

    const Frame& my_frame = parser->GetNextFrame();

    set_component (my_frame.Ydata(), Y_COMP, decoder);
    if (decoder->seq_params.chroma != Yonly)
    {
        set_component (my_frame.Udata(), U_COMP, decoder);
        set_component (my_frame.Vdata(), V_COMP, decoder);
    }

    return;
}

static void set_frame_params (const FrameParams& my_frame_params,  dirac_decoder_t *decoder)
{
    TEST (decoder != NULL);
    dirac_frameparams_t *frame_params = &decoder->frame_params;

    TEST (decoder->state == STATE_PICTURE_AVAIL ||
          decoder->state == STATE_PICTURE_START);

    frame_params->ftype = (dirac_frame_type_t)my_frame_params.FSort();
    frame_params->fnum = my_frame_params.FrameNum();

    return;
}

extern DllExport dirac_decoder_state_t dirac_parse (dirac_decoder_t *decoder)
{
    TEST (decoder != NULL);
    TEST (decoder->parser != NULL);
    DiracParser *parser = static_cast<DiracParser *>(decoder->parser);

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
