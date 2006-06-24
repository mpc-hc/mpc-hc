/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_encoder.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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
#include <sstream>
#include <fstream>
#include <queue>
#include <libdirac_common/dirac_assertions.h>
#include <libdirac_common/common.h>
#include <libdirac_common/frame.h>
#include <libdirac_common/pic_io.h>
#include <libdirac_encoder/dirac_encoder.h>
#include <libdirac_encoder/seq_compress.h>
using namespace dirac;

template <class T >
void copy_2dArray (const TwoDArray<T> & in, T *out)
{
    for (int j=0 ; j<in.LengthY() ; ++j)
    {
        for (int i=0 ; i<in.LengthX() ; ++i)
        {
            // out[j*in.LengthX() + i] =  in[j][i];
            *out++ =  in[j][i];
        }// i
    }// j
}

void copy_2dArray (const TwoDArray<PredMode> & in, int *out)
{
    for (int j=0 ; j<in.LengthY() ; ++j)
    {
        for (int i=0 ; i<in.LengthX() ; ++i)
        {
            // out[j*in.LengthX() + i] =  in[j][i];
            *out++ =  in[j][i];
        }// i
    }// j
}

void copy_2dArray (const TwoDArray<bool> & in, int *out)
{
    for (int j=0 ; j<in.LengthY() ; ++j)
    {
        for (int i=0 ; i<in.LengthX() ; ++i)
        {
            // out[j*in.LengthX() + i] =  in[j][i];
            *out++ =  in[j][i];
        }// i
    }// j
}

void copy_mv ( const MvArray& mv, dirac_mv_t *dmv)
{
    for (int j=0 ; j<mv.LengthY() ; ++j)
    {
        for (int i=0 ; i<mv.LengthX() ; ++i)
        {
            //dmv[j*mv.LengthX() + i].x = mv[j][i].x;
            //dmv[j*mv.LengthX() + i].y = mv[j][i].y;
            (*dmv).x = mv[j][i].x;
            (*dmv).y = mv[j][i].y;
            dmv++;
        }// i
    }// j
}

void copy_mv_cost (const TwoDArray<MvCostData> &pc, dirac_mv_cost_t *dpc)
{
    for (int j=0 ; j<pc.LengthY() ; ++j)
    {
        for (int i=0 ; i<pc.LengthX() ; ++i)
        {
            //dpc[j*pc.LengthX() + i].SAD = pc[j][i].SAD;
            //dpc[j*pc.LengthX() + i].mvcost = pc[j][i].mvcost;
            (*dpc).SAD = pc[j][i].SAD;
            (*dpc).mvcost = pc[j][i].mvcost;
            dpc++;
        }// i
    }// j
}

/*
    Function that allocates the locally managed instrumentation data
*/
void alloc_instr_data(dirac_instr_t *instr)
{
    instr->mb_split_mode = new int [instr->mb_ylen*instr->mb_xlen];
    memset (instr->mb_split_mode, 0, sizeof(int)*instr->mb_ylen*instr->mb_xlen);

    instr->mb_common_mode = new int [instr->mb_ylen*instr->mb_xlen];
    memset (instr->mb_common_mode, 0, 
                    sizeof(int)*instr->mb_ylen*instr->mb_xlen);

    instr->mb_costs = new float [instr->mb_ylen*instr->mb_xlen];
    memset (instr->mb_costs, 0, sizeof(float)*instr->mb_ylen*instr->mb_xlen);

    instr->pred_mode = new int [instr->mv_ylen * instr->mv_xlen];
    memset (instr->pred_mode, 0, sizeof(int)*instr->mv_ylen*instr->mv_xlen);
        
    instr->intra_costs = new float [instr->mv_ylen * instr->mv_xlen];
    memset (instr->intra_costs, 0, sizeof(float)*instr->mv_ylen*instr->mv_xlen);
        
    instr->bipred_costs = new dirac_mv_cost_t [instr->mv_ylen * instr->mv_xlen];
    memset (instr->bipred_costs, 0, sizeof(dirac_mv_cost_t)*instr->mv_ylen*instr->mv_xlen);
        
    instr->dc_ycomp = new short [instr->mv_ylen * instr->mv_xlen];
    memset (instr->dc_ycomp, 0, sizeof(short)*instr->mv_ylen*instr->mv_xlen);
        
    instr->dc_ucomp = new short [instr->mv_ylen * instr->mv_xlen];
    memset (instr->dc_ucomp, 0, sizeof(short)*instr->mv_ylen*instr->mv_xlen);
        
    instr->dc_vcomp = new short [instr->mv_ylen * instr->mv_xlen];
    memset (instr->dc_vcomp, 0, sizeof(short)*instr->mv_ylen*instr->mv_xlen);
        
    for (int i = 0; i < 2; i++)
    {
        instr->mv[i] = new dirac_mv_t[instr->mv_ylen * instr->mv_xlen];
        memset (instr->mv[i], 0, 
            sizeof(dirac_mv_t)*instr->mv_ylen*instr->mv_xlen);
    }
        
    for (int i = 0; i < 2; i++)
    {
        instr->pred_costs[i] = new dirac_mv_cost_t[instr->mv_ylen * instr->mv_xlen];
        memset (instr->pred_costs[i], 0, 
            sizeof(dirac_mv_cost_t)*instr->mv_ylen*instr->mv_xlen);
    }
}

/*
    Function that frees the locally managed instrumentation data
*/
void dealloc_instr_data(dirac_instr_t *instr)
{
    if (instr->mb_split_mode)
        delete [] instr->mb_split_mode;

    if (instr->mb_common_mode)
        delete [] instr->mb_common_mode;

    if (instr->mb_costs)
        delete [] instr->mb_costs;

    if (instr->pred_mode)
        delete [] instr->pred_mode;

    if (instr->intra_costs)
        delete [] instr->intra_costs;

    if (instr->bipred_costs)
        delete [] instr->bipred_costs;

    if (instr->dc_ycomp)
        delete [] instr->dc_ycomp;

    if (instr->dc_ucomp)
        delete [] instr->dc_ucomp;

    if (instr->dc_vcomp)
        delete [] instr->dc_vcomp;

    for (int i = 0; i < 2; i++)
    {
        if (instr->mv[i])
            delete [] instr->mv[i];
    }
    for (int i = 0; i < 2; i++)
    {
        if (instr->pred_costs[i])
            delete [] instr->pred_costs[i];
    }
}

/*
    Wrapper class around the SequenceCompressor Class. This class is used 
    by the "C" encoder interface
*/
class DiracEncoder
{
public:
    // constructor
    DiracEncoder(const dirac_encoder_context_t *enc_ctx, bool verbose);
    // destructor
    ~DiracEncoder();

    // Load the next frame of uncompressed data into the SequenceCompressor
    bool LoadNextFrame(unsigned char *data, int size);
    // Compress the next frame of data
    int CompressNextFrame();

    // Set the encode frame in encoder to the encoded frame data
    int GetEncodedData(dirac_encoder_t *encoder);
    
    // Set the locally decoded frame data in encoder
    int GetDecodedData (dirac_encoder_t *encoder);

    // Set the instrumentation data in encoder
    void GetInstrumentationData (dirac_encoder_t *encoder);

    // Set the end of sequence infomration in encoder
    int GetSequenceEnd(dirac_encoder_t *encoder);

    // Set the buffer to hold the locally decoded frame
    void SetDecodeBuffer (unsigned char *buffer, int buffer_size);

    // Return the sequence parameters
    const SeqParams& GetSeqParams() const { return m_sparams; }

    // Return the encoder parameters
    const EncoderParams& GetEncParams() const { return m_encparams; }

private:

    // Set the sequence parameters
    void SetSequenceParams (const dirac_encoder_context_t *enc_ctx);
    
    // Set the encoder parameters
    void SetEncoderParams (const dirac_encoder_context_t *enc_ctx);

    // Get the frame statistics
    void GetFrameStats(dirac_encoder_t *encoder);

    // Get the seuqence statistics
    void GetSequenceStats(dirac_encoder_t *encoder);

private:
    // sequence compressor
    SequenceCompressor *m_comp;
    // sequence parameters
    SeqParams m_sparams;
    // encoder parameters
    EncoderParams m_encparams;
    // stream to hold the compressed frame
    std::ostringstream m_comp_stream;
    // locally encoded frame in coded order
    const Frame *m_enc_frame;
    // locally encoded frame ME data
    const MEData *m_enc_medata;
    // locally decoded frame number in display order
    int m_decfnum;
    //locally decoded frame type
    FrameSort m_decfsort;
    // locally decoded frame number in display order
    int m_show_fnum;
    // total number of frame loaded so far
    int m_num_loaded_frames;
    // total number of frames encoded so far
    int m_num_coded_frames;
    // verbose flag
    bool m_verbose;
    // input stream for uncompressed input frame
    MemoryStreamInput m_inp_ptr;
    // output stream for locally decoded frame
    MemoryStreamOutput m_out_ptr;
    // buffer to hold locally decoded frame. Set by SetDecodeBuffer
    unsigned char *m_dec_buf;
    // size of buffer to hold locally decoded data. Set by SetDecodeBuffer
    int m_dec_bufsize;
    // Flag that determines if locally decoded frames are to be returned. Set
    // in Constructor
    bool m_return_decoded_frames;
    // Flag that determines if instrumentation data is to be returned. Set
    // in Constructor
    bool m_return_instr_data;
};

/*
    Instrumentation callback. This function is passed as a parameter to the
    SequenceCompressor constructor. It is called by the 
    FrameCompressor::Compress function once the frame is successfully compressed
*/
void DiracEncoder::GetInstrumentationData (dirac_encoder_t *encoder)
{
    dirac_ASSERT (encoder != NULL);
    dirac_instr_t *instr = &encoder->instr;
    dirac_instr_t old_instr = *instr;

    if (!m_return_instr_data)
        return;

    const FrameParams& fparams = m_enc_frame->GetFparams();
    const FrameSort fsort = fparams.FSort();

    instr->fnum = fparams.FrameNum();
    instr->ftype = fsort;
    instr->num_refs = 0;
    encoder->instr_data_avail = 1;

    if (fsort == I_frame)
    {
        // no MV data for Intra coded data
        return;
    }

    TESTM (m_enc_medata != NULL, "ME data available");

    // Reference info
    instr->num_refs = fparams.Refs().size();
    dirac_ASSERTM (instr->num_refs <= 2, "Max # reference frames is 2");

    for (int i=0; i<instr->num_refs; ++i)
        instr->refs[i] = fparams.Refs()[i];

    // Block separation params
    instr->ybsep = m_encparams.LumaBParams(2).Ybsep();
    instr->xbsep = m_encparams.LumaBParams(2).Xbsep();

    // Num macroblocks
    instr->mb_ylen = m_enc_medata->MBSplit().LengthY();
    instr->mb_xlen = m_enc_medata->MBSplit().LengthX();

    // Motion vector array dimensions
    instr->mv_ylen = m_enc_medata->Vectors(1).LengthY();
    instr->mv_xlen = m_enc_medata->Vectors(1).LengthX();

    if (old_instr.mb_ylen != instr->mb_ylen || 
        old_instr.mb_xlen != instr->mb_xlen ||
        old_instr.mv_ylen != instr->mv_ylen || 
        old_instr.mv_xlen != instr->mv_xlen)
    {
        dealloc_instr_data(instr);
        alloc_instr_data(instr);
    }

    copy_2dArray (m_enc_medata->MBSplit(), instr->mb_split_mode);
    copy_2dArray (m_enc_medata->MBCommonMode(), instr->mb_common_mode);
    copy_2dArray (m_enc_medata->MBCosts(), instr->mb_costs);
    copy_2dArray (m_enc_medata->Mode(), instr->pred_mode);
    copy_2dArray (m_enc_medata->IntraCosts(), instr->intra_costs);

    // FIXME: Always allocating for bipred_costs even though no data available
    //  Since medata is always created assuming two references
    // if (instr->num_refs > 1)
    {
        copy_mv_cost (m_enc_medata->BiPredCosts(), instr->bipred_costs);
    }

    copy_2dArray (m_enc_medata->DC( Y_COMP ), instr->dc_ycomp);
    if (m_enc_medata->DC().Length() == 3 && 
        encoder->enc_ctx.seq_params.chroma != Yonly)
    {
        copy_2dArray (m_enc_medata->DC( U_COMP ), instr->dc_ucomp);
        copy_2dArray (m_enc_medata->DC( V_COMP ), instr->dc_vcomp);
    }

    // FIXME: Always allocating for bipred_costs even though no data available
    //  Since medata is always created assuming two references
    // for (int i=1; i<=instr->num_refs; ++i)
    for (int i=1; i<=2; ++i)
    {
        copy_mv (m_enc_medata->Vectors(i), instr->mv[i-1]);
        copy_mv_cost (m_enc_medata->PredCosts(i), instr->pred_costs[i-1]);
    }
}

DiracEncoder::DiracEncoder(const dirac_encoder_context_t *enc_ctx, 
                           bool verbose) :
    m_show_fnum(-1),
    m_num_loaded_frames(0),
    m_num_coded_frames(0),
    m_verbose(verbose),
    m_dec_buf(0),
    m_dec_bufsize(0),
    m_return_decoded_frames(enc_ctx->decode_flag > 0),
    m_return_instr_data(enc_ctx->instr_flag > 0)
{
    // Setup sequence parameters
    SetSequenceParams (enc_ctx);
    // Setup encoder parameters
    m_encparams.SetVerbose( verbose );
    SetEncoderParams (enc_ctx);

    // Set up the input data stream (uncompressed data)
    m_inp_ptr.SetSequenceParams(m_sparams);
    // Set up the output data stream (locally decoded frame)
    m_out_ptr.SetSequenceParams(m_sparams);
    
    // initialise the sequence compressor
    m_comp = new SequenceCompressor (&m_inp_ptr, &m_comp_stream, m_encparams);
}

void DiracEncoder::SetDecodeBuffer (unsigned char *buffer, int buffer_size)
{
    m_dec_buf = buffer;
    m_dec_bufsize = buffer_size;
    m_return_decoded_frames = true;
}

DiracEncoder::~DiracEncoder()
{
    delete m_comp;
}

void DiracEncoder::SetSequenceParams (const dirac_encoder_context_t *enc_ctx)
{
    m_sparams.SetCFormat( enc_ctx->seq_params.chroma );
    m_sparams.SetXl( enc_ctx->seq_params.width );
    m_sparams.SetYl( enc_ctx->seq_params.height );
    m_sparams.SetInterlace( enc_ctx->seq_params.interlace );
    m_sparams.SetTopFieldFirst( enc_ctx->seq_params.topfieldfirst );
    m_sparams.SetFrameRate( enc_ctx->seq_params.frame_rate.numerator /
                            enc_ctx->seq_params.frame_rate.denominator );
}

void DiracEncoder::SetEncoderParams (const dirac_encoder_context_t *enc_ctx)
{
    TEST (enc_ctx != NULL);
    OLBParams bparams(12, 12, 8, 8);

    m_encparams.SetQf(enc_ctx->enc_params.qf);
    m_encparams.SetL1Sep(enc_ctx->enc_params.L1_sep);
    m_encparams.SetNumL1(enc_ctx->enc_params.num_L1);
    m_encparams.SetCPD(enc_ctx->enc_params.cpd);
    m_encparams.SetUFactor(3.0f);
    m_encparams.SetVFactor(1.75f);
    bparams.SetYblen( enc_ctx->enc_params.yblen );
    bparams.SetXbsep( enc_ctx->enc_params.xblen );
    bparams.SetYbsep( enc_ctx->enc_params.ybsep );
    bparams.SetXbsep( enc_ctx->enc_params.xbsep );

    // Now rationalise the GOP options
    // this stuff should really be done in a constructor!
    if (m_encparams.NumL1()<0)
    {
        //don't have a proper GOP
        m_encparams.SetL1Sep( std::max(1 , m_encparams.L1Sep()) );
    }
    else if (m_encparams.NumL1() == 0)
    {
        //have I-frame only coding
        m_encparams.SetL1Sep(0);
    }
    m_encparams.SetOrigXl( enc_ctx->seq_params.width );
    m_encparams.SetOrigYl( enc_ctx->seq_params.height );
    m_encparams.SetBlockSizes( bparams , enc_ctx->seq_params.chroma );
}


bool DiracEncoder::LoadNextFrame (unsigned char *data, int size)
{
    TESTM (m_comp->Finished() != true, "Did not reach end of sequence");
    m_inp_ptr.SetMembufReference(data, size);
    if (m_comp->LoadNextFrame())
    {
        m_num_loaded_frames++;
        return true;
    }
    return false;
}

int DiracEncoder::CompressNextFrame ()
{
    TESTM (m_comp->Finished() != true, "Did not reach end of sequence");

    if (!m_num_loaded_frames)
        return 0;

    Frame &myframe = m_comp->CompressNextFrame();

    m_enc_frame = m_comp->GetFrameEncoded();
    m_enc_medata = m_comp->GetMEData();

    m_decfnum = -1;
    if (m_return_decoded_frames && 
            myframe.GetFparams().FrameNum() != m_show_fnum)
    {
        int ret_val;
        m_show_fnum = myframe.GetFparams().FrameNum();
        TEST (! (m_return_decoded_frames && !m_dec_buf) );
        if (m_return_decoded_frames && m_dec_buf)
        {
            // write locally decoded frame to decode buffer
            m_out_ptr.SetMembufReference(m_dec_buf, m_dec_bufsize);
            ret_val = m_out_ptr.WriteNextFrame(myframe);

            if (ret_val)
            {
                m_decfnum = m_show_fnum;
                m_decfsort = myframe.GetFparams().FSort();
            }
        }
    }

    int size = m_comp_stream.str().size();
    if (size > 0)
    {
        m_num_coded_frames++;
        TESTM (m_enc_frame != 0, "Encoder frame available");
    }
    return size;
}

void DiracEncoder::GetFrameStats(dirac_encoder_t *encoder)
{
    const FrameOutputManager& foutput = m_encparams.BitsOut().FrameOutput();
    dirac_enc_framestats_t *fstats = &encoder->enc_fstats;

    fstats->mv_bits = foutput.MVBytes() * 8;
    fstats->mv_hdr_bits = foutput.MVBytes() * 8;

    fstats->ycomp_bits = foutput.ComponentBytes( Y_COMP ) * 8;
    fstats->ycomp_hdr_bits = foutput.ComponentHeadBytes( Y_COMP ) * 8;

    fstats->ucomp_bits = foutput.ComponentBytes( U_COMP ) * 8;
    fstats->ucomp_hdr_bits = foutput.ComponentHeadBytes( U_COMP ) * 8;

    fstats->vcomp_bits = foutput.ComponentBytes( V_COMP ) * 8;
    fstats->vcomp_hdr_bits = foutput.ComponentHeadBytes( V_COMP ) * 8;

    fstats->frame_bits = foutput.FrameBytes() * 8;
    fstats->frame_hdr_bits = foutput.FrameHeadBytes() * 8;
}

int DiracEncoder::GetEncodedData (dirac_encoder_t *encoder)
{
    int size = 0;
    dirac_enc_data_t *encdata = &encoder->enc_buf;

    size = m_comp_stream.str().size();
    if (size > 0)
    {
        if (encdata->size < size )
        {
            return -1;
        }
        memmove (encdata->buffer, m_comp_stream.str().c_str(),  size);
        encoder->enc_fparams.fnum = m_enc_frame->GetFparams().FrameNum();
        encoder->enc_fparams.ftype = m_enc_frame->GetFparams().FSort();

        // Get frame statistics
        GetFrameStats (encoder);
        encdata->size = size;

        GetInstrumentationData(encoder);
        encoder->encoded_frame_avail = 1;

        m_comp_stream.str("");
    }
    else
    {
        encdata->size = 0;
    }
    return size;
}

int DiracEncoder::GetDecodedData (dirac_encoder_t *encoder)
{
    dirac_frameparams_t *fp = &encoder->dec_fparams;

    int ret_stat = (m_decfnum != -1);
    if (m_return_decoded_frames && m_decfnum != -1)
    {
            fp->ftype = m_decfsort;
            fp->fnum = m_decfnum;
            encoder->decoded_frame_avail = 1;
            m_decfnum = -1;
    }
    return ret_stat;
}

void DiracEncoder::GetSequenceStats(dirac_encoder_t *encoder)
{
    dirac_enc_seqstats_t *sstats = &encoder->enc_seqstats;
    dirac_seqparams_t *sparams = &encoder->enc_ctx.seq_params;

    sstats->seq_bits = m_encparams.BitsOut().SequenceBytes() * 8;
    sstats->seq_hdr_bits = m_encparams.BitsOut().SequenceHeadBytes() * 8;

    sstats->mv_bits = m_encparams.BitsOut().MVBytes() * 8;
    sstats->ycomp_bits = m_encparams.BitsOut().ComponentBytes( Y_COMP ) * 8;
    sstats->ucomp_bits = m_encparams.BitsOut().ComponentBytes( U_COMP ) * 8;
    sstats->vcomp_bits = m_encparams.BitsOut().ComponentBytes( V_COMP ) * 8;

    sstats->bit_rate = (sstats->seq_bits * sparams->frame_rate.numerator)/
                        (sparams->frame_rate.denominator * m_num_coded_frames);
}

int DiracEncoder::GetSequenceEnd (dirac_encoder_t *encoder)
{
    dirac_enc_data_t *encdata = &encoder->enc_buf;
    m_comp_stream.str("");
    m_comp->EndSequence();
    int size = m_comp_stream.str().size();
    if (size > 0)
    {
        if (encdata->size < size )
        {
            return -1;
        }
        memmove (encdata->buffer, m_comp_stream.str().c_str(),  size);
        GetSequenceStats(encoder);
        m_comp_stream.str("");
        encdata->size = size;
    }
    else
    {
        encdata->size = 0;
    }
    return size;
}

static bool InitialiseEncoder (const dirac_encoder_context_t *enc_ctx, bool verbose, dirac_encoder_t *encoder)
{
    TEST (enc_ctx != NULL);
    TEST (encoder != NULL);
    
    if (enc_ctx->seq_params.width == 0 || enc_ctx->seq_params.height == 0)
        return false;

    if (enc_ctx->seq_params.chroma < Yonly || 
            enc_ctx->seq_params.chroma > formatNK)
        return false;

    if (!enc_ctx->seq_params.frame_rate.numerator || 
            !enc_ctx->seq_params.frame_rate.denominator)
        return false;

    memmove (&encoder->enc_ctx, enc_ctx, sizeof(dirac_encoder_context_t));

    encoder->dec_buf.id = 0;

    switch ( enc_ctx->seq_params.chroma )
    {
    case Yonly:
         encoder->enc_ctx.seq_params.chroma_width = 0;
         encoder->enc_ctx.seq_params.chroma_height = 0;
        break;
    case format411:
         encoder->enc_ctx.seq_params.chroma_width = enc_ctx->seq_params.width/4;
         encoder->enc_ctx.seq_params.chroma_height = enc_ctx->seq_params.height;
         break;
    case format420:
         encoder->enc_ctx.seq_params.chroma_width = enc_ctx->seq_params.width/2;
         encoder->enc_ctx.seq_params.chroma_height = enc_ctx->seq_params.height/2;
         break;
    case format422:
         encoder->enc_ctx.seq_params.chroma_width = enc_ctx->seq_params.width/2;
         encoder->enc_ctx.seq_params.chroma_height = enc_ctx->seq_params.height;
         break;
    case format444:
    default:
         encoder->enc_ctx.seq_params.chroma_width = enc_ctx->seq_params.width;
         encoder->enc_ctx.seq_params.chroma_height = enc_ctx->seq_params.height;
         break;
    }

    try
    {
        DiracEncoder *comp = new DiracEncoder (&encoder->enc_ctx, verbose);
    
        int bufsize = (encoder->enc_ctx.seq_params.width * encoder->enc_ctx.seq_params.height)+ 2*(encoder->enc_ctx.seq_params.chroma_width*encoder->enc_ctx.seq_params.chroma_height);

        encoder->dec_buf.buf[0] = new unsigned char [bufsize];
        encoder->dec_buf.buf[1] = encoder->dec_buf.buf[0] + 
                (encoder->enc_ctx.seq_params.width * encoder->enc_ctx.seq_params.height);
        encoder->dec_buf.buf[2] = encoder->dec_buf.buf[1] + 
                (encoder->enc_ctx.seq_params.chroma_width*encoder->enc_ctx.seq_params.chroma_height);
    
        encoder->compressor = comp;
        if (encoder->enc_ctx.decode_flag)
        {
            comp->SetDecodeBuffer (encoder->dec_buf.buf[0], bufsize);
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

static void SetSequenceParameters (dirac_encoder_context_t *enc_ctx, dirac_encoder_presets_t preset)
{
    TEST (enc_ctx != NULL);
    dirac_seqparams_t &sparams = enc_ctx->seq_params;

    sparams.chroma = format420;
    switch (preset)
    {
    case SD576:
        sparams.width = 720;
        sparams.height = 576;
        sparams.frame_rate.numerator = 25;
        sparams.frame_rate.denominator = 1;
        sparams.interlace = 1;
        sparams.topfieldfirst = 1;
        break;
    case HD720:
        sparams.width = 1280;
        sparams.height = 720;
        sparams.frame_rate.numerator = 50;
        sparams.frame_rate.denominator = 1;
        sparams.interlace = 0;
        sparams.topfieldfirst = 0;
        break;
    case HD1080:
        sparams.width = 1920;
        sparams.height = 1080;
        sparams.frame_rate.numerator = 25;
        sparams.frame_rate.denominator = 1;
        sparams.interlace = 1;
        sparams.topfieldfirst = 1;
        break;
    case CIF:
    default:
        sparams.width = 352;
        sparams.height = 288;
        sparams.frame_rate.numerator = 13;
        sparams.frame_rate.denominator = 1;
        sparams.interlace = 0;
        sparams.topfieldfirst = 0;
        break;
    }
}

static void SetEncoderParameters (dirac_encoder_context_t *enc_ctx, dirac_encoder_presets_t preset)
{
    TEST (enc_ctx != NULL);

    dirac_encparams_t &encparams = enc_ctx->enc_params;

    encparams.qf = 7.0f;

    switch (preset)
    {
    case SD576:
        encparams.L1_sep = 3;
        encparams.num_L1 = 3;
        encparams.cpd = 32.0f;
        encparams.xblen = 12;
        encparams.yblen = 12;
        encparams.xbsep = 8;
        encparams.ybsep = 8;
        break;
    case HD720:
        encparams.L1_sep = 3;
        encparams.num_L1 = 7;
        encparams.cpd = 20.0f;
        encparams.xblen = 16;
        encparams.yblen = 16;
        encparams.xbsep = 10;
        encparams.ybsep = 12;
        break;
    case HD1080:
        encparams.L1_sep = 3;
        encparams.num_L1 = 3;
        encparams.cpd = 32.0f;
        encparams.xblen = 20;
        encparams.yblen = 20;
        encparams.xbsep = 16;
        encparams.ybsep = 16;
        break;
    case CIF:
    default:
        encparams.L1_sep = 3;
        encparams.num_L1 = 11;
        encparams.cpd = 20.0f;
        encparams.xblen = 12;
        encparams.yblen = 12;
        encparams.xbsep = 8;
        encparams.ybsep = 8;
        break;
    }
}
#ifdef __cplusplus
extern "C" {
#endif

extern DllExport void dirac_encoder_context_init ( dirac_encoder_context_t *enc_ctx, dirac_encoder_presets_t preset)
{
    TEST (enc_ctx != NULL);
    memset (enc_ctx, 0, sizeof(dirac_encoder_context_t));
    SetSequenceParameters (enc_ctx, preset);
    SetEncoderParameters (enc_ctx, preset);
}

extern DllExport dirac_encoder_t *dirac_encoder_init (const dirac_encoder_context_t *enc_ctx, int verbose)
{
    /* Allocate for encoder */
    dirac_encoder_t *encoder = new dirac_encoder_t;

    memset (encoder, 0, sizeof(dirac_encoder_t));

    /* initialse the encoder context */
    if (!InitialiseEncoder(enc_ctx, verbose>0, encoder))
    {
        delete encoder;
        return NULL;
    }

    encoder->encoded_frame_avail = encoder->decoded_frame_avail = 0;
    encoder->instr_data_avail = 0;


    return encoder;
}

extern DllExport int dirac_encoder_load (dirac_encoder_t *encoder, unsigned char *uncdata, int uncdata_size)
{
    TEST (encoder != NULL);
    TEST (encoder->compressor != NULL);
    DiracEncoder *compressor = (DiracEncoder *)encoder->compressor;
    int ret_stat = 0;
    try
    {
        if ( compressor->LoadNextFrame (uncdata, uncdata_size))
        {
            ret_stat = uncdata_size;
        }
    }
    catch (...)
    {
        if (compressor->GetEncParams().Verbose())
            std::cerr << "dirac_encoder_load failed" << std::endl;
        ret_stat = -1;
    }
    return ret_stat;
}

extern DllExport dirac_encoder_state_t 
      dirac_encoder_output (dirac_encoder_t *encoder)
{
    TEST (encoder != NULL);
    TEST (encoder->compressor != NULL);
    TEST (encoder->enc_buf.size != 0);
    TEST (encoder->enc_buf.buffer != NULL);
    DiracEncoder *compressor = (DiracEncoder *)encoder->compressor;
    dirac_encoder_state_t ret_stat = ENC_STATE_BUFFER;

    encoder->encoded_frame_avail = 0;
    encoder->decoded_frame_avail = 0;
    encoder->instr_data_avail = 0;

    try
    {
        compressor->CompressNextFrame();
        if (compressor->GetEncodedData (encoder) < 0)
            ret_stat = ENC_STATE_INVALID;
        else
        {
            if (encoder->enc_buf.size > 0)
            {
                ret_stat = ENC_STATE_AVAIL;
            }
    
            if (encoder->enc_ctx.decode_flag)
                compressor->GetDecodedData(encoder);
        }
    }
    catch (...)
    {
        if (compressor->GetEncParams().Verbose())
            std::cerr << "GetEncodedData failed..." << std::endl;

        ret_stat = ENC_STATE_INVALID;
    }
    return ret_stat;
}

extern DllExport int dirac_encoder_end_sequence (dirac_encoder_t *encoder)
{
    TEST (encoder != NULL);
    TEST (encoder->compressor != NULL);
    DiracEncoder *compressor = (DiracEncoder *)encoder->compressor;
    int ret_stat;

    encoder->encoded_frame_avail = 0;
    encoder->decoded_frame_avail = 0;
    encoder->instr_data_avail = 0;

    try
    {
        ret_stat = compressor->GetSequenceEnd (encoder);
        encoder->end_of_sequence = 1;

        if (compressor->GetDecodedData(encoder))
        {
            encoder->decoded_frame_avail = 1;
        }
    }
    catch (...)
    {
        if (compressor->GetEncParams().Verbose())
            std::cerr << "GetSequenceEnd failed..." << std::endl;
        ret_stat = -1;
    }
    return ret_stat;
}

extern DllExport void dirac_encoder_close (dirac_encoder_t *encoder)
{
    TEST (encoder != NULL);
    TEST (encoder->compressor != NULL);
    
    delete (DiracEncoder *)(encoder->compressor);

    if (encoder->enc_ctx.instr_flag)
    {
        dealloc_instr_data(&encoder->instr);
    }

    if (encoder->enc_ctx.decode_flag)
    {
        delete [] encoder->dec_buf.buf[0];
    }
    delete encoder;
}


#ifdef __cplusplus
}
#endif
