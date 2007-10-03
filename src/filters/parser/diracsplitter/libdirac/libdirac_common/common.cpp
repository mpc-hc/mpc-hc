/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: common.cpp,v 1.46 2007/05/02 16:29:51 asuraparaju Exp $ $Name: Dirac_0_7_0 $
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
* Contributor(s): Thomas Davies (Original Author),
*                 Scott R Ladd,
*                 Tim Borer,
*                 Anuradha Suraparaju,
*                 Andrew Kennedy
*                 Myo Tun (Brunel University, myo.tun@brunel.ac.uk)
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

#include <algorithm>
#include <sstream>
#ifndef _MSC_VER
#include <inttypes.h>
#endif
#include <libdirac_common/common.h>
#include <libdirac_common/video_format_defaults.h>
#include <libdirac_common/dirac_exception.h>
using namespace dirac;

//const dirac::QuantiserLists dirac::dirac_quantiser_lists;

//PicArray functions
PicArray::PicArray(int height, int width, CompSort cs):
    TwoDArray<ValueType>(height , width),
    m_csort(cs)
{
         //Nothing
}

const CompSort& PicArray::CSort() const 
{
    return m_csort;
}

void PicArray::SetCSort(const CompSort cs)
{
    m_csort=cs;
}    



//EntropyCorrector functions

EntropyCorrector::EntropyCorrector(int depth): 
    m_Yfctrs( 3 , 3*depth+1 ),
    m_Ufctrs( 3 , 3*depth+1 ),
    m_Vfctrs( 3 , 3*depth+1 )
{
    Init();
}

float EntropyCorrector::Factor(const int bandnum , const FrameSort fsort ,const CompSort c) const
{
    int idx = fsort.IsIntra() ? 0 : (fsort.IsRef() ? 1 : 2);
    
    if (c == U_COMP)
        return m_Ufctrs[idx][bandnum-1];
    else if (c == V_COMP)
        return m_Vfctrs[idx][bandnum-1];
    else
        return m_Yfctrs[idx][bandnum-1];
}

void EntropyCorrector::Init()
{
    
    //do I-frames
    for (int  i=0 ; i<m_Yfctrs.LengthX() ; ++i )
    {
        if ( i == m_Yfctrs.LastX() )
        {
            // Set factor for Intra frames
            m_Yfctrs[0][i] = 1.0f;
            m_Ufctrs[0][i] = 1.0f;
            m_Vfctrs[0][i] = 1.0f;
            // Set factor for Inter Ref frames
            m_Yfctrs[1][i] = 0.85f;
            m_Ufctrs[1][i] = 0.85f;
            m_Vfctrs[1][i] = 0.85f;
            // Set factor for Inter Non-Ref frames
            m_Yfctrs[2][i] = 0.85f;
            m_Ufctrs[2][i] = 0.85f;
            m_Vfctrs[2][i] = 0.85f;
        }
        else if ( i >= m_Yfctrs.LastX()-3 )
        {
            // Set factor for Intra frames
            m_Yfctrs[0][i] = 0.85f;
            m_Ufctrs[0][i] = 0.85f;
            m_Vfctrs[0][i] = 0.85f;
            // Set factor for Inter Ref frames
            m_Yfctrs[1][i] = 0.75f;
            m_Ufctrs[1][i] = 0.75f;
            m_Vfctrs[1][i] = 0.75f;
            // Set factor for Inter Non-Ref frames
            m_Yfctrs[2][i] = 0.75f;
            m_Ufctrs[2][i] = 0.75f;
            m_Vfctrs[2][i] = 0.75f;            
        }
        else
        {
            // Set factor for Intra frames
            m_Yfctrs[0][i] = 0.75f;
            m_Ufctrs[0][i] = 0.75f;
            m_Vfctrs[0][i] = 0.75f;
            // Set factor for Inter Ref frames
            m_Yfctrs[1][i] = 0.75f;
            m_Ufctrs[1][i] = 0.75f;
            m_Vfctrs[1][i] = 0.75f;
            // Set factor for Inter Non-Ref frames
            m_Yfctrs[2][i] = 0.75f;
            m_Ufctrs[2][i] = 0.75f;
            m_Vfctrs[2][i] = 0.75f;            
        }
    }//i
    
}

void EntropyCorrector::Update(int bandnum , FrameSort fsort , CompSort c ,int est_bits , int actual_bits){
    //updates the factors - note that the estimated bits are assumed to already include the correction factor
    
    float multiplier;
    if (actual_bits != 0 && est_bits != 0)
        multiplier = float(actual_bits)/float(est_bits);
    else
        multiplier=1.0;

    int idx = fsort.IsIntra() ? 0 : (fsort.IsRef() ? 1 : 2);
    if (c == U_COMP)
        m_Ufctrs[idx][bandnum-1] *= multiplier;
    else if (c == V_COMP)
        m_Vfctrs[idx][bandnum-1] *= multiplier;
    else
        m_Yfctrs[idx][bandnum-1] *= multiplier;
}

// Overlapped block parameter functions

OLBParams::OLBParams(const int xblen, int const yblen, int const xbsep, int const ybsep):
    m_xblen(xblen),
    m_yblen(yblen),
    m_xbsep(xbsep),
    m_ybsep(ybsep),
    m_xoffset( (xblen-xbsep)/2 ),
    m_yoffset( (yblen-ybsep)/2 )
{}

bool OLBParams::operator ==(const OLBParams bparams) const
{
    if (bparams.Xblen() != m_xblen || 
        bparams.Yblen() != m_yblen ||
        bparams.Xbsep() != m_xbsep ||
        bparams.Ybsep() != m_ybsep)

        return false;

    return true;
}

namespace dirac
{
std::ostream & operator<< (std::ostream & stream, OLBParams & params)
{
    stream << params.Ybsep() << " " << params.Xbsep();
//     stream << " " <<params.Yblen() << " " << params.Xblen();
    
    return stream;
}

std::istream & operator>> (std::istream & stream, OLBParams & params)
{
    int temp;

    stream >> temp;
    params.SetYbsep(temp);

    stream >> temp;
    params.SetXbsep(temp);

//     stream >> temp;
//     params.SetYblen(temp);

//     stream >> temp;
//     params.SetXblen(temp);
    
    return stream;
}

}

// Codec params functions

CodecParams::CodecParams(const VideoFormat &vd, FrameType ftype, unsigned int num_refs, bool set_defaults): 
    m_x_num_mb(0),
    m_y_num_mb(0),
    m_x_num_blocks(0),
    m_y_num_blocks(0),
    m_verbose(false),
    m_lbparams(3),
    m_cbparams(3),
    m_video_format(vd)
{
    if (set_defaults)
        SetDefaultCodecParameters(*this, ftype, num_refs);
}

void CodecParams::SetBlockSizes(const OLBParams& olbparams , const ChromaFormat cformat)
{
    //given the raw overlapped block parameters, set the modified internal parameters to
    //take account of the chroma sampling format and overlapping requirements, as well
    //as the equivalent parameters for sub-MBs and MBs.
    //Does NOT set the number of blocks or macroblocks, as padding may be required.

    OLBParams tmp_olbparams = olbparams;
    // Factors for scaling chroma blocks
    int xcfactor,ycfactor; 
 
    if (cformat == format420)
    {
        xcfactor = 2;
        ycfactor = 2;
    }
    else if (cformat == format422)
    {
        xcfactor = 2;
        ycfactor = 1;
    }
    else
    {// assume 444
        xcfactor = 1;
        ycfactor = 1;
    }


    m_lbparams[2] = tmp_olbparams;
        
    // Check separations are all divisible by 4
    int remainder= m_lbparams[2].Xbsep()%4;
    if ( remainder!=0 || m_lbparams[2].Xbsep()==0 )
    {
        m_lbparams[2].SetXbsep( m_lbparams[2].Xbsep()+(4-remainder));
        m_lbparams[2].SetXblen( m_lbparams[2].Xbsep()+4 );
    }
    remainder= m_lbparams[2].Ybsep()%4;
    if ( remainder!=0 || m_lbparams[2].Ybsep()==0 )
    {
        m_lbparams[2].SetYbsep( m_lbparams[2].Ybsep()+(4-remainder));
        m_lbparams[2].SetYblen( m_lbparams[2].Ybsep()+4 );

    }

    // Now check lengths are divisible by 4
    remainder= m_lbparams[2].Xblen()%4;
    if ( remainder!=0 )
    {
        m_lbparams[2].SetXblen( m_lbparams[2].Xbsep()+4);
    }
    remainder= m_lbparams[2].Yblen()%4;
    if ( remainder!=0 )
    {
        m_lbparams[2].SetYblen( m_lbparams[2].Ybsep()+4);
    }
        
    // Check there's non-negative overlap,
    // XBLEN >= XBSEP, YBLEN >= YBSEP
    if (m_lbparams[2].Xbsep()>m_lbparams[2].Xblen())
    {
        m_lbparams[2].SetXblen( m_lbparams[2].Xbsep()+4);
    }
    if (m_lbparams[2].Ybsep()>m_lbparams[2].Yblen())
    {
        m_lbparams[2].SetYblen( m_lbparams[2].Ybsep()+4);
    }

    // Check the lengths aren't too big (100% is max roll-off)
    // XBLEN <= 2*XBSEP, YBLEN <= 2*YBSEP
    if (2*m_lbparams[2].Xbsep()<m_lbparams[2].Xblen())
    {
        m_lbparams[2].SetXblen( m_lbparams[2].Xbsep()+4);
    }
    if (2*m_lbparams[2].Ybsep()<m_lbparams[2].Yblen())
    {
        m_lbparams[2].SetYblen( m_lbparams[2].Ybsep()+4);
    }

    // Check if the luma block overlap is zero or implies a chroma overlap >1
    int overlap = m_lbparams[2].Xblen() - m_lbparams[2].Xbsep();
    if (overlap!=0 && overlap<2*xcfactor)
    {
        m_lbparams[2].SetXblen( m_lbparams[2].Xbsep()+4);
    }
    overlap = m_lbparams[2].Yblen() - m_lbparams[2].Ybsep();
    if (overlap!=0 && overlap<2*ycfactor)
    {
        m_lbparams[2].SetYblen( m_lbparams[2].Ybsep()+4);
    }    

    // Now check that the overlap is a power of 2
    int count = 0;
    overlap = m_lbparams[2].Xblen() - m_lbparams[2].Xbsep();
    while ((overlap>>=1))
        ++count;

    overlap = m_lbparams[2].Xblen() - m_lbparams[2].Xbsep();
    if ((1<<count) != overlap && overlap!=0)
    {
        m_lbparams[2].SetXblen( m_lbparams[2].Xbsep()+4);
    }
    count = 0;
    overlap = m_lbparams[2].Yblen() - m_lbparams[2].Ybsep();
    while ((overlap>>=1))
        ++count;

    overlap = m_lbparams[2].Yblen() - m_lbparams[2].Ybsep();
    if ((1<<count) != overlap && overlap!=0)
    {
        m_lbparams[2].SetYblen( m_lbparams[2].Ybsep()+4);
    }


    // Set the chroma values
    m_cbparams[2].SetXbsep( m_lbparams[2].Xbsep()/xcfactor );
    m_cbparams[2].SetXblen( m_lbparams[2].Xblen()/xcfactor );
    m_cbparams[2].SetYbsep( m_lbparams[2].Ybsep()/ycfactor );
    m_cbparams[2].SetYblen( m_lbparams[2].Yblen()/ycfactor );


    //Now work out the overlaps for splitting levels 1 and 0
    m_lbparams[1].SetXbsep( m_lbparams[2].Xbsep()*2 );
    m_lbparams[1].SetXblen( m_lbparams[2].Xblen() + m_lbparams[2].Xbsep() );
    m_lbparams[1].SetYbsep( m_lbparams[2].Ybsep()*2 );
    m_lbparams[1].SetYblen( m_lbparams[2].Yblen() + m_lbparams[2].Xbsep() );
    
    m_lbparams[0].SetXbsep( m_lbparams[1].Xbsep()*2 );
    m_lbparams[0].SetXblen( m_lbparams[1].Xblen() + m_lbparams[1].Xbsep() );
    m_lbparams[0].SetYbsep( m_lbparams[1].Ybsep()*2 );
    m_lbparams[0].SetYblen( m_lbparams[1].Yblen() + m_lbparams[1].Xbsep() );        
    
    m_cbparams[1].SetXbsep( m_cbparams[2].Xbsep()*2 );
    m_cbparams[1].SetXblen( m_cbparams[2].Xblen() + m_cbparams[2].Xbsep() );
    m_cbparams[1].SetYbsep( m_cbparams[2].Ybsep()*2 );
    m_cbparams[1].SetYblen( m_cbparams[2].Yblen() + m_cbparams[2].Xbsep() );    
    
    m_cbparams[0].SetXbsep( m_cbparams[1].Xbsep()*2 );
    m_cbparams[0].SetXblen( m_cbparams[1].Xblen() + m_cbparams[1].Xbsep() );
    m_cbparams[0].SetYbsep( m_cbparams[1].Ybsep()*2 );
    m_cbparams[0].SetYblen( m_cbparams[1].Yblen() + m_cbparams[1].Xbsep() );

    if ( m_lbparams[2].Xbsep()!=olbparams.Xbsep() ||
         m_lbparams[2].Ybsep()!=olbparams.Ybsep() ||
         m_lbparams[2].Xblen()!=olbparams.Xblen() ||
         m_lbparams[2].Yblen()!=olbparams.Yblen() )
    {
        std::cout<<std::endl<<"WARNING: block parameters are inconsistent with ";
        std::cout<<"specification requirements, which are:";
        std::cout<<std::endl<<"\t 1. Lengths and separations must be positive multiples of 4";
        std::cout<<std::endl<<"\t 2. Length can't be more than twice separations";
        std::cout<<std::endl<<"\t 3. Lengths must be greater than or equal to separations";
        std::cout<<std::endl<<"\t 4. Overlap=length-separation must be a power of 2 i.e. 0 or 2^k, k>=2";
        std::cout<<std::endl<<std::endl<<"Instead, using:";
        std::cout<<" xblen="<<m_lbparams[2].Xblen();
        std::cout<<" yblen="<<m_lbparams[2].Yblen();
        std::cout<<" xbsep="<<m_lbparams[2].Xbsep();
        std::cout<<" ybsep="<<m_lbparams[2].Ybsep() << std::endl;
    }
}

WltFilter CodecParams::TransformFilter (unsigned int wf_idx)
{
    if (wf_idx >= filterNK)
        DIRAC_THROW_EXCEPTION(
            ERR_UNSUPPORTED_STREAM_DATA,
            "Wavelet filter idx out of range [0-7]",
            SEVERITY_FRAME_ERROR);

    if (wf_idx==FIDELITY)
    {
        std::ostringstream errstr;
        errstr << "Wavelet Filter " << wf_idx << " currently not supported";
        DIRAC_THROW_EXCEPTION(
            ERR_UNSUPPORTED_STREAM_DATA,
            errstr.str(),
            SEVERITY_FRAME_ERROR);
    }
    return static_cast<WltFilter>(wf_idx);
}

void CodecParams::SetTransformFilter(unsigned int wf_idx)
{
    SetTransformFilter(TransformFilter(wf_idx));
}

void CodecParams::SetTransformDepth (unsigned int wd)
{
    m_wlt_depth = wd;
    // Resize the code block size array.
    m_cb.Resize(wd+1);
}

void CodecParams::SetCodeBlocks (unsigned int level, 
                                   unsigned int hblocks,
                                   unsigned int vblocks)
{
    if (level > m_wlt_depth)
    {
        std::ostringstream errstr;
        errstr << "level " << level << " out of range [0-" << m_wlt_depth  << "]";
        DIRAC_THROW_EXCEPTION(
            ERR_UNSUPPORTED_STREAM_DATA,
            errstr.str(),
            SEVERITY_FRAME_ERROR);
    }

    m_cb[level].SetHorizontalCodeBlocks(hblocks);
    m_cb[level].SetVerticalCodeBlocks(vblocks);
}

void CodecParams::SetDefaultCodeBlocks ( const FrameType &ftype)
{
    // No subband splitting if  spatial partitioning if false
    // Since this function is common to encoder and decoder we allow the
    // setting of code blocks without checking if DefaultSpatialPartition is
    // true.
    if (SpatialPartition() == false)
        return;

    SetCodeBlocks(0, 1, 1);
    if (m_wlt_depth == 0)
        return;

    switch (GetVideoFormat())
    {
    case VIDEO_FORMAT_QSIF:
    case VIDEO_FORMAT_QCIF:
        // No splitting by default for QSIF and CIF
        break;

    case VIDEO_FORMAT_CUSTOM:
    case VIDEO_FORMAT_SIF:
    case VIDEO_FORMAT_CIF:
    case VIDEO_FORMAT_4CIF:
    case VIDEO_FORMAT_4SIF:
    case VIDEO_FORMAT_SD_525_DIGITAL:
    case VIDEO_FORMAT_SD_625_DIGITAL:
    case VIDEO_FORMAT_HD_720:
    case VIDEO_FORMAT_HD_1080:
        if (ftype == INTRA_FRAME)
        {
            // Intra frame default number of coeff blocks per level
            // only the highest two levels are split.
            int level = TransformDepth();
            int i = 1;
            for (; i<= level-2 || i == 1; ++i)
            {
                SetCodeBlocks(i, 1, 1);
            }
            for (; i <=level; ++i)
            {
                SetCodeBlocks(i, 4, 3);
            }
        }
        else
        {
            // Inter frame default number of coeff blocks per level
            // only the highest three levels are split.
            int level = TransformDepth();
            int i = 1;
            for (; i<= level-3 || i == 1; ++i)
            {
                SetCodeBlocks(i, 1, 1);
            }
            if (i  <= level-2)
                SetCodeBlocks(level-2, 8, 6);
            ++i;

            for (; i <=level; ++i)
            {
                SetCodeBlocks(i, 12, 8);
            }
        }
        break;

    default:
        DIRAC_THROW_EXCEPTION(
            ERR_INVALID_VIDEO_FORMAT,
            "Unsupported video format",
            SEVERITY_FRAME_ERROR);
        break;
    }
}

const CodeBlocks &CodecParams::GetCodeBlocks (unsigned int level) const 
{
    if (level > m_wlt_depth)
    {
        std::ostringstream errstr;
        errstr << "level " << level << " out of range [0-" << m_wlt_depth  << "]";
        DIRAC_THROW_EXCEPTION(
            ERR_UNSUPPORTED_STREAM_DATA,
            errstr.str(),
            SEVERITY_FRAME_ERROR);
    }

    return m_cb[level];
}

void CodecParams::SetCodeBlockMode (unsigned int cb_mode)
{
    if (cb_mode >= QUANT_UNDEF)
    {
        std::ostringstream errstr;
        errstr << "Code Block mode " << cb_mode << " out of supported range [0-" << QUANT_MULTIPLE  << "]";
        DIRAC_THROW_EXCEPTION(
            ERR_UNSUPPORTED_STREAM_DATA,
            errstr.str(),
            SEVERITY_FRAME_ERROR);
    }

    m_cb_mode = static_cast<CodeBlockMode>(cb_mode);
}

//EncoderParams functions

//Default constructor    
EncoderParams::EncoderParams(const VideoFormat& video_format,
                             FrameType ftype,
                             unsigned int num_refs,
                             bool set_defaults):
    CodecParams(video_format, ftype, num_refs, set_defaults),
    m_loc_decode(true),
    m_ufactor(1.0),
    m_vfactor(1.0),
    m_denoise(false),
    m_I_lambda(0.0f),
    m_L1_lambda(0.f),
    m_L2_lambda(0.0f),
    m_L1_me_lambda(0.0f),
    m_L2_me_lambda(0.0f),
    m_ent_correct(0),
    m_target_rate(0)
{
    if(set_defaults)
        SetDefaultEncoderParameters(*this);
}

void EncoderParams::CalcLambdas(const float qf)
{
    if (!m_lossless )
    { 
        m_I_lambda = std::pow( 10.0 , (10.0-qf )/2.5 )/16.0;
        m_L1_lambda = m_I_lambda*32.0;
        m_L2_lambda = m_I_lambda*256.0;

        // Set the lambdas for motion estimation
        const double me_ratio = 2.0;

        // Use the same ME lambda for L1 and L2 frames
        m_L1_me_lambda = std::sqrt(m_L1_lambda)*me_ratio;
        m_L2_me_lambda = m_L1_me_lambda;
    }
    else
    {
        m_I_lambda = 0.0;
        m_L1_lambda = 0.0;
        m_L2_lambda = 0.0;

        m_L1_me_lambda = 0.0;
        m_L2_me_lambda = 0.0;        
    }
}

void EncoderParams::SetIntraTransformFilter(unsigned int wf_idx)
{
    SetIntraTransformFilter(TransformFilter(wf_idx));
}

void EncoderParams::SetInterTransformFilter(unsigned int wf_idx)
{
    SetInterTransformFilter(TransformFilter(wf_idx));
}

DecoderParams::DecoderParams(const VideoFormat& video_format,
                             FrameType ftype,
                             unsigned int num_refs,
                             bool set_defaults):
    CodecParams(video_format, ftype, num_refs, set_defaults)
{
}

// ParseParams functions
// constructor
ParseParams::ParseParams(unsigned int au_pnum /*=0*/):
    m_au_pnum(au_pnum),
    m_major_ver(0),
    m_minor_ver(1),
    m_profile(0),
    m_level(0)
{}

//SeqParams functions
//constructor
SeqParams::SeqParams(const VideoFormat& video_format,
                     bool set_defaults):
m_xl(0),
m_yl(0),
m_cformat(format422),
m_video_format(video_format)
{
    // set default parameters
    if(set_defaults)
        SetDefaultSequenceParameters(*this);

}

int SeqParams::ChromaWidth() const
{
    switch (m_cformat)
    {
    case format420:
    case format422:
        return m_xl/2;

    case format444:
    default:
        return m_xl;
    }
}

int SeqParams::ChromaHeight() const
{
    switch (m_cformat)
    {
    case format420:
        return m_yl/2;

    case format422:
    case format444:
    default:
        return m_yl;
    }
}

//Source functions
//constructor
SourceParams::SourceParams(const VideoFormat& video_format,
                           bool set_defaults)
{
    // set default parameters
    if(set_defaults)
        SetDefaultSourceParameters(video_format, *this);
}

void SourceParams::SetFrameRate (FrameRateType fr)
{
    m_fr_idx = fr;
    switch (fr)
    {
    case FRAMERATE_23p97_FPS:
        m_framerate.m_num = 24000;
        m_framerate.m_denom = 1001;
        break;
    case FRAMERATE_24_FPS:
        m_framerate.m_num = 24;
        m_framerate.m_denom = 1;
        break;
    case FRAMERATE_25_FPS:
        m_framerate.m_num = 25;
        m_framerate.m_denom = 1;
        break;
    case FRAMERATE_29p97_FPS:
        m_framerate.m_num = 30000;
        m_framerate.m_denom = 1001;
        break;
    case FRAMERATE_30_FPS:
        m_framerate.m_num = 30;
        m_framerate.m_denom = 1;
        break;
    case FRAMERATE_50_FPS:
        m_framerate.m_num = 50;
        m_framerate.m_denom = 1;
        break;
    case FRAMERATE_59p94_FPS:
        m_framerate.m_num = 60000;
        m_framerate.m_denom = 1001;
        break;
    case FRAMERATE_60_FPS:
        m_framerate.m_num = 60;
        m_framerate.m_denom = 1;
    default:
        m_fr_idx = FRAMERATE_CUSTOM;
        m_framerate.m_num = m_framerate.m_denom = 0;
        break;
    }
}

void SourceParams::SetAspectRatio (AspectRatioType aspect_ratio)
{
    m_asr_idx = aspect_ratio;

    switch (aspect_ratio)
    {
    case ASPECT_RATIO_1_1:
        m_aspect_ratio.m_num = m_aspect_ratio.m_denom = 1;
        break;
    case ASPECT_RATIO_10_11:
        m_aspect_ratio.m_num = 10;
        m_aspect_ratio.m_denom = 11;
        break;
    case ASPECT_RATIO_12_11:
        m_aspect_ratio.m_num = 12;
        m_aspect_ratio.m_denom = 11;
        break;
    default:
        m_asr_idx = ASPECT_RATIO_CUSTOM;
        m_aspect_ratio.m_num = m_aspect_ratio.m_denom = 0;
        break;
    }
}

void SourceParams::SetSignalRange (SignalRangeType sr)
{
    m_sr_idx = sr;
    switch (sr)
    {
    case SIGNAL_RANGE_8BIT_FULL:
        m_luma_offset = 0;
        m_luma_excursion = 255;
        m_chroma_offset = 128;
        m_chroma_excursion = 254;
        break;
    case SIGNAL_RANGE_8BIT_VIDEO:
        m_luma_offset = 16;
        m_luma_excursion = 235;
        m_chroma_offset = 128;
        m_chroma_excursion = 244;
        break;
    case SIGNAL_RANGE_10BIT_VIDEO:
        m_luma_offset = 64;
        m_luma_excursion = 876;
        m_chroma_offset = 512;
        m_chroma_excursion = 896;
        break;
    default:
        m_sr_idx = SIGNAL_RANGE_CUSTOM;
        m_luma_offset = 0;
        m_luma_excursion = 0;
        m_chroma_offset = 0;
        m_chroma_excursion = 0;
        break;
    }
}

void SourceParams::SetColourSpecification (unsigned int cs_idx)
{
    m_cs_idx = cs_idx;
    switch(cs_idx)
    {
    case 1:
        m_col_primary = CP_SMPTE_C;
        m_col_matrix = CM_SDTV;
        m_transfer_func = TF_TV;
        break;
    case 2:
        m_col_primary = CP_EBU_3213;
        m_col_matrix = CM_SDTV;
        m_transfer_func = TF_TV;
        break;
    case 3:
        m_col_primary = CP_CIE_XYZ;
        m_col_matrix = CM_DCINEMA;
        m_transfer_func = TF_DCINEMA;
        break;
    default:
        m_cs_idx = 0;
        m_col_primary = CP_ITU_709;
        m_col_matrix = CM_HDTV_COMP_INTERNET;
        m_transfer_func = TF_TV;
        break;
    }
}

void SourceParams::SetColourPrimariesIndex (unsigned int cp)
{
    m_cs_idx = 0;
    if (cp >= CP_UNDEF)
    {
        //TODO: flag a warning 
    }
    m_col_primary = static_cast<ColourPrimaries>(cp);
}

void SourceParams::SetColourMatrixIndex (unsigned int cm)
{
    m_cs_idx = 0;
    if (cm >= CM_UNDEF)
    {
        //TODO: flag a warning 
    }
    m_col_matrix = static_cast<ColourMatrix>(cm);
}

void SourceParams::SetTransferFunctionIndex (unsigned int tf)
{
    m_cs_idx = 0;
    if (tf >= TF_UNDEF)
    {
        //TODO: flag a warning 
    }
    m_transfer_func = static_cast<TransferFunction>(tf);
}


//FrameParams functions

// Default constructor
FrameParams::FrameParams():
m_fsort(FrameSort::IntraRefFrameSort()),
m_frame_type( INTRA_FRAME ),
m_reference_type( REFERENCE_FRAME ),    
m_output(false)
{}    

// Constructor 
FrameParams::FrameParams(const ChromaFormat& cf, int xlen, int ylen, int c_xlen, int c_ylen, unsigned int video_depth):
    m_cformat(cf),
    m_xl(xlen),
    m_yl(ylen),
    m_fsort(FrameSort::IntraRefFrameSort()),
    m_frame_type( INTRA_FRAME ),
    m_reference_type( REFERENCE_FRAME ),    
    m_output(false),
    m_chroma_xl(c_xlen),
    m_chroma_yl(c_ylen),
    m_video_depth(video_depth)
{}

// Constructor
FrameParams::FrameParams(const ChromaFormat& cf, const FrameSort& fs):
    m_cformat(cf),
    m_output(false)
{
    SetFSort( fs );    
}    

// Constructor
FrameParams::FrameParams(const SeqParams& sparams):
    m_cformat(sparams.CFormat()),
    m_xl(sparams.Xl()),
    m_yl(sparams.Yl()),
    m_fsort(FrameSort::IntraRefFrameSort()),
    m_frame_type( INTRA_FRAME ),
    m_reference_type( REFERENCE_FRAME ),
    m_output(false),
    m_video_depth(sparams.GetVideoDepth())
{
    m_chroma_xl = m_chroma_yl = 0;
    if(m_cformat == format422) 
    {
        m_chroma_xl = m_xl/2;
        m_chroma_yl = m_yl;
    }
    else if (m_cformat == format420)
    {
        m_chroma_xl = m_xl/2;
        m_chroma_yl = m_yl/2;
    }
    else if (m_cformat==format444)
    {
        m_chroma_xl = m_xl;
        m_chroma_yl = m_yl;
    }
}

// Constructor
FrameParams::FrameParams(const SeqParams& sparams, const FrameSort& fs):
    m_cformat(sparams.CFormat()),
    m_xl(sparams.Xl()),
    m_yl(sparams.Yl()),
    m_output(false),
    m_video_depth(sparams.GetVideoDepth())
{
    SetFSort(fs);
    
    m_chroma_xl = m_chroma_yl = 0;
    if(m_cformat == format422) 
    {
        m_chroma_xl = m_xl/2;
        m_chroma_yl = m_yl;
    }
    else if (m_cformat == format420)
    {
        m_chroma_xl = m_xl/2;
        m_chroma_yl = m_yl/2;
    }
    else if (m_cformat==format444)
    {
        m_chroma_xl = m_xl;
        m_chroma_yl = m_yl;
    }
}

bool FrameParams::IsBFrame() const
{
    bool is_B_frame( false );
       
    if ( m_refs.size() == 2 )        
    {
        if ( m_refs[0] < m_fnum && m_refs[1] > m_fnum )
            is_B_frame = true;

        if ( m_refs[0] > m_fnum && m_refs[1] < m_fnum )
            is_B_frame = true;
    }  

    return is_B_frame;        
}

void FrameParams::SetFSort( const FrameSort& fs )
{
    m_fsort=fs;
    if ( fs.IsIntra() )
        m_frame_type = INTRA_FRAME;
    else
        m_frame_type = INTER_FRAME;
        
    if ( fs.IsRef() )
        m_reference_type = REFERENCE_FRAME;
    else
        m_reference_type = NON_REFERENCE_FRAME;

}

void FrameParams::SetFrameType(const FrameType ftype)
{
    m_frame_type = ftype;
    if (ftype == INTRA_FRAME )
        m_fsort.SetIntra();
    else
        m_fsort.SetInter();
}

void FrameParams::SetReferenceType(const ReferenceType rtype)
{
    m_reference_type = rtype;
    if (rtype == REFERENCE_FRAME )
        m_fsort.SetRef();
    else
        m_fsort.SetNonRef();    
}


QuantiserLists::QuantiserLists()
:
    // FIXME: hardcode m_max_qindex to 119. In future this will depend on level
	// As per spec max qf_idx is 127. But for values of qf_idx > 120 we
	// will need more than 32 bits. Hence qf_idx is limited to 119.
    m_max_qindex( 119 ),  
    m_qflist4( m_max_qindex+1 ),
    m_intra_offset4( m_max_qindex+1 ),
    m_inter_offset4( m_max_qindex+1 )
{
    m_qflist4[0] = 4;
    m_qflist4[1] = 5;
    m_intra_offset4[0] = 1;
    m_inter_offset4[0] = 1;
    m_intra_offset4[1] = 2;
    m_inter_offset4[1] = 2;
   
#ifdef _MSC_VER
    unsigned __int64 base, qfactor;
#else
    uint64_t base, qfactor;
#endif //_MSC_VER

    for (unsigned int q=2; q<=m_max_qindex; ++q)
    {
        base = (1<<(q/4));

        switch (q%4)
        {
            case 0: 
                 qfactor = 4*base;
                 break;
            case 1: 
                 qfactor = (503829*base+52958)/105917;
                 break;
            case 2: 
                 qfactor = (665857*base+58854)/117708;
                 break;
            case 3: 
                 qfactor = (440253*base+32722)/65444;
                 break;
            default: //Default case never used
                 qfactor = 0; 
        }
        
        m_qflist4[q] = int( qfactor );
        
        m_intra_offset4[q] = (m_qflist4[q]+1)>>1;
        m_inter_offset4[q] = (3*m_qflist4[q]+4)>>3;
    }// q
}

namespace dirac 
{
VideoFormat IntToVideoFormat(int video_format) 
{
    switch(video_format)
    {
    case VIDEO_FORMAT_CUSTOM:
        return VIDEO_FORMAT_CUSTOM;
    case VIDEO_FORMAT_QSIF:
        return VIDEO_FORMAT_QSIF;
    case VIDEO_FORMAT_QCIF:
        return VIDEO_FORMAT_QCIF;
    case VIDEO_FORMAT_SIF:
        return VIDEO_FORMAT_SIF;
    case VIDEO_FORMAT_CIF:
        return VIDEO_FORMAT_CIF;
    case VIDEO_FORMAT_4CIF:
        return VIDEO_FORMAT_4CIF;
    case VIDEO_FORMAT_4SIF:
        return VIDEO_FORMAT_4SIF;
    case VIDEO_FORMAT_SD_525_DIGITAL:
        return VIDEO_FORMAT_SD_525_DIGITAL;
    case VIDEO_FORMAT_SD_625_DIGITAL:
        return VIDEO_FORMAT_SD_625_DIGITAL;
    case VIDEO_FORMAT_HD_720:
        return VIDEO_FORMAT_HD_720;
    case VIDEO_FORMAT_HD_1080:
        return VIDEO_FORMAT_HD_1080;
    default:
        return VIDEO_FORMAT_UNDEFINED;
    }
}

ChromaFormat IntToChromaFormat(int chroma_format)
{
    switch(chroma_format)
    {
    case format444:
        return format444;
    case format422:
        return format422;
    case format420:
        return format420;
    default:
        return formatNK;
    }
}

FrameRateType IntToFrameRateType(int frame_rate_idx)
{
    switch(frame_rate_idx)
    {
    case FRAMERATE_CUSTOM:
        return FRAMERATE_CUSTOM;
    case FRAMERATE_23p97_FPS:
        return FRAMERATE_23p97_FPS;
    case FRAMERATE_24_FPS:
        return FRAMERATE_24_FPS;
    case FRAMERATE_25_FPS:
        return FRAMERATE_25_FPS;
    case FRAMERATE_29p97_FPS:
        return FRAMERATE_29p97_FPS;
    case FRAMERATE_30_FPS:
        return FRAMERATE_30_FPS;
    case FRAMERATE_50_FPS:
        return FRAMERATE_30_FPS;
    case FRAMERATE_59p94_FPS:
        return FRAMERATE_59p94_FPS;
    case FRAMERATE_60_FPS:
        return FRAMERATE_60_FPS;
    default:
        return FRAMERATE_UNDEFINED;
    }
}
    
AspectRatioType IntToAspectRatioType(int aspect_ratio_idx)
{
    switch(aspect_ratio_idx)
    {
    case ASPECT_RATIO_CUSTOM:
        return ASPECT_RATIO_CUSTOM;
    case ASPECT_RATIO_1_1:
        return ASPECT_RATIO_1_1;
    case ASPECT_RATIO_10_11:
        return ASPECT_RATIO_10_11;
    case ASPECT_RATIO_12_11:
        return ASPECT_RATIO_12_11;
    default:
        return ASPECT_RATIO_UNDEFINED;

    }
}

SignalRangeType IntToSignalRangeType(int signal_range_idx)
{
    switch(signal_range_idx)
    {
    case SIGNAL_RANGE_CUSTOM:
        return SIGNAL_RANGE_CUSTOM;
    case SIGNAL_RANGE_8BIT_FULL:
        return SIGNAL_RANGE_8BIT_FULL;
    case SIGNAL_RANGE_8BIT_VIDEO:
        return SIGNAL_RANGE_8BIT_VIDEO;
    case SIGNAL_RANGE_10BIT_VIDEO:
        return SIGNAL_RANGE_10BIT_VIDEO;
    default:
        return SIGNAL_RANGE_UNDEFINED;
    }
}

MVPrecisionType IntToMVPrecisionType(int mv_prec)
{
    switch(mv_prec)
    {
    case MV_PRECISION_PIXEL:
            return MV_PRECISION_PIXEL;
    case MV_PRECISION_HALF_PIXEL:
        return MV_PRECISION_HALF_PIXEL;
    case MV_PRECISION_QUARTER_PIXEL:
        return MV_PRECISION_QUARTER_PIXEL;
    case MV_PRECISION_EIGHTH_PIXEL:
        return MV_PRECISION_EIGHTH_PIXEL;
    default:
        return MV_PRECISION_UNDEFINED;
    }
}


}
