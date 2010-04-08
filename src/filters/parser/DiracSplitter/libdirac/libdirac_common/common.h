/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: common.h,v 1.79 2008/10/01 01:26:47 asuraparaju Exp $ $Name:  $
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

#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef  _MSC_VER
#define  _CRT_SECURE_NO_DEPRECATE
#endif // _MSC_VER

#include <libdirac_common/arrays.h>
#include <libdirac_common/common_types.h>
#include <libdirac_common/dirac_assertions.h>
#include <vector>
#include <cmath>
namespace dirac
{
/*! \file
    This file contains common classes used throughout the encoder and
    decoder.  The main classes are the encoder and decoder parameters for
    controlling the encode and decode processes. These are passed
    throughout the codec.  There are also parameter classes for sequences
    and Pictures.
*/


//Some basic types used throughout the codec ...//
//////////////////////////////////////////////////////////////

//! Type of picture data (including motion compensated residuals)
typedef short ValueType;

#if !defined(HAVE_MMX)
//! Type of wavelet coefficient data (should be larger than ValueType)
typedef int CoeffType;
#else
//! Type of wavelet coefficient data (should be larger than ValueType)
typedef short CoeffType;
#endif

//! Type for performing calculations on ValueType and CoeffType. Should be >ValueType, >=CoeffType
typedef int CalcValueType;

//! Prediction modes for blocks
enum PredMode { INTRA , REF1_ONLY , REF2_ONLY , REF1AND2, UNDEFINED };

//! Types of picture component
enum CompSort { Y_COMP , U_COMP , V_COMP };

//! Addition or subtraction
enum AddOrSub { ADD , SUBTRACT };

//! Forward or backward
enum Direction { FORWARD , BACKWARD };

//! Contexts used for coefficient coding
enum CtxAliases
{
    //used for residual coding
    SIGN0_CTX,          // -sign, previous symbol is 0
    SIGN_POS_CTX,       // -sign, previous symbol is +ve
    SIGN_NEG_CTX,       // -sign, previous symbol is -ve

    // Follow bit contexts
    Z_FBIN1z_CTX,       // -bin 1, parent is zero, neighbours zero
    Z_FBIN1nz_CTX,      // -bin 1, parent is zero, neighbours non-zero
    Z_FBIN2_CTX,        // -bin 2, parent is zero
    Z_FBIN3_CTX,        // -bin 3, parent is zero
    Z_FBIN4_CTX,        // -bin 4, parent is zero
    Z_FBIN5_CTX,        // -bin 5, parent is zero
    Z_FBIN6plus_CTX,    // -bins 6 plus, parent is zero

    NZ_FBIN1z_CTX,      // -bin 1, parent is non-zero, neighbours zero
    NZ_FBIN1nz_CTX,     // -bin 1, parent is non-zero, neighbours non-zero
    NZ_FBIN2_CTX,       // -bin 2, parent is non-zero
    NZ_FBIN3_CTX,       // -bin 3, parent is non-zero
    NZ_FBIN4_CTX,       // -bin 4, parent is non-zero
    NZ_FBIN5_CTX,       // -bin 5, parent is non-zero
    NZ_FBIN6plus_CTX,   // -bins 6 plus, parent is non-zero

    // Information bit contexts
    INFO_CTX,

    BLOCK_SKIP_CTX,     // - blocks are skipped
    Q_OFFSET_FOLLOW_CTX,   // - code block quantiser offset magnitude
    Q_OFFSET_INFO_CTX,  // - code block quantiser offset info context
    Q_OFFSET_SIGN_CTX,   // - code block quantiser offset sign
    TOTAL_COEFF_CTXS   // The total number of coefficient contexts
};

//! Contexts used for MV data coding
enum MvCtxAliases
{
    // DC value contexts //
    ///////////////////////

    DC_FBIN1_CTX,
    DC_FBIN2plus_CTX,
    DC_INFO_CTX,
    DC_SIGN_CTX,

    // Motion vector contexts //
    ////////////////////////////


    MV_FBIN1_CTX,
    MV_FBIN2_CTX,
    MV_FBIN3_CTX,
    MV_FBIN4_CTX,
    MV_FBIN5plus_CTX,

    MV_INFO_CTX,

    MV_SIGN_CTX,


    // Prediction mode contexts

    PMODE_BIT0_CTX,     // -bit 0, prediction mode value
    PMODE_BIT1_CTX,     // -bin 1, prediction mode value


    // Macroblock contexts

    SB_SPLIT_BIN1_CTX,  // bin 1, SB split mode vals
    SB_SPLIT_BIN2_CTX,  // bin 2, SB split mode vals. Bin 3 not required

    SB_SPLIT_INFO_CTX,  // info context for SB split mode

    TOTAL_MV_CTXS       // The total number of motion vector contexts
};


/**
* Function to convert an integer to a valid VideoFormat
*@param video_format Integer corresponding to a format
*@return Valid video-format (returns VIDEO_FORMAT_UNDEFINED if no valid format found)
*/
VideoFormat IntToVideoFormat(int video_format);

/**
* Function to convert an integer to a valid VideoFormat
*@param chroma_format Integer corresponding to a format
*@return Valid chroma-format (returns formatNK if no valid format found)
*/
ChromaFormat IntToChromaFormat(int chroma_format);

/**
    * Function to convert an integer to a valid FrameRate type
    *@param frame_rate_idx Integer corresponding to a frame-rate in the spec table
    *@return Valid FrameRateType (returns FRAMERATE_UNDEFINED if no valid frame-rate found)
    */
FrameRateType IntToFrameRateType(int frame_rate_idx);

/**
* Function to convert an integer to a valid PixelAspectRatio type
*@param pix_asr_idx Integer corresponding to a pixel aspect ratio in the spec table
*@return Valid PixelAspectRatioType (returns PIXEL_ASPECT_RATIO_UNDEFINED if no valid pixel aspect ratio found)
*/
PixelAspectRatioType IntToPixelAspectRatioType(int pix_asr_idx);

/**
* Function to convert an integer to a valid SignalRange type
*@param signal_range_idx Integer corresponding to a signal-range in the spec table
*@return Valid SignalRangeType (returns SIGNAL_RANGE_UNDEFINED if no valid signal-ratio found)
*/
SignalRangeType IntToSignalRangeType(int signal_range_idx);

/**
    * Function to convert an integer to a valid motion-vector precision type
    *@param mv_prec Integer corresponding to a valid motion-vector precision
    *@return Valid MVPrecisionType (returns SIGNAL_RANGE_UNDEFINED if no valid precision found)
    */
MVPrecisionType IntToMVPrecisionType(int mv_prec);

//Classes used throughout the codec//
/////////////////////////////////////

//! Class defining a rational number
class Rational
{
public:
    //! Numerator
    unsigned int m_num;
    //! Denominator
    unsigned int m_denom;
};

//! Picture type Class
class PictureSort
{
public:
    PictureSort()
    {
        fs = 0x00;    // default intra non-ref
    }

    void SetIntra()
    {
        fs &= 0xfe;
    }
    void SetInter()
    {
        fs |= 0x01;
    }
    void SetNonRef()
    {
        fs &= 0xfd;
    }
    void SetRef()
    {
        fs |= 0x02;
    }

    bool IsInter() const
    {
        return fs & 0x01;
    }
    bool IsIntra() const
    {
        return !IsInter();
    }
    bool IsRef() const
    {
        return fs & 0x02;
    };
    bool IsNonRef() const
    {
        return !IsRef();
    }

    void SetIntraNonRef()
    {
        SetIntra();
        SetNonRef();
    }
    void SetIntraRef()
    {
        SetIntra();
        SetRef();
    }
    void SetInterNonRef()
    {
        SetInter();
        SetNonRef();
    }
    void SetInterRef()
    {
        SetInter();
        SetRef();
    }

    bool IsIntraNonRef() const
    {
        return (fs & 0x03) == 0x00;
    }
    bool IsIntraRef() const
    {
        return (fs & 0x03) == 0x02;
    }
    bool IsInterNonRef() const
    {
        return (fs & 0x03) == 0x01;
    }
    bool IsInterRef() const
    {
        return (fs & 0x03) == 0x03;
    }

    void Clear()
    {
        fs = 0x00;
    }

    static PictureSort IntraRefPictureSort()
    {
        PictureSort fs;
        fs.SetIntraRef();
        return fs;
    }

    static PictureSort InterRefPictureSort()
    {
        PictureSort fs;
        fs.SetInterRef();
        return fs;
    }

    static PictureSort IntraNonRefPictureSort()
    {
        PictureSort fs;
        fs.SetIntraNonRef();
        return fs;
    }

    static PictureSort InterNonRefPictureSort()
    {
        PictureSort fs;
        fs.SetInterNonRef();
        return fs;
    }

private:
    unsigned char fs;
};

//! Parameters relating to the source material being encoded/decoded
class SourceParams
{
public:
    //! default constructor
    SourceParams(const VideoFormat &vf = VIDEO_FORMAT_CUSTOM,
                 bool set_defaults = true);

    ////////////////////////////////////////////////////////////////////
    //NB: Assume default copy constructor, assignment = and destructor//
    ////////////////////////////////////////////////////////////////////

    // Gets
    //! Returns video-format
    VideoFormat GetVideoFormat() const
    {
        return m_video_format;
    }

    //! Returns the picture width
    unsigned int Xl() const
    {
        return m_xl;
    }

    //! Returns the picture height
    unsigned int Yl() const
    {
        return m_yl;
    }

    //! Returns the chroma format of the sequence (420, 422, 444)
    ChromaFormat CFormat() const
    {
        return m_cformat;
    }

    //! Returns the chroma width
    int ChromaWidth() const;

    //! Returns the chroma height
    int ChromaHeight() const;

    //! Returns the source sampling field of the source scan format
    unsigned int SourceSampling() const
    {
        return m_source_sampling;
    }

    //! Returns true if top field comes first in time
    bool TopFieldFirst() const
    {
        return m_topfieldfirst;
    }

    //! Return the number for frames per second
    Rational FrameRate() const
    {
        return m_framerate;
    }

    //! Return the type from the frame rate table
    FrameRateType FrameRateIndex() const
    {
        return m_fr_idx;
    }

    //! Return the pixel aspect ratio
    Rational PixelAspectRatio() const
    {
        return m_pixel_aspect_ratio;
    }

    //! Return the type from the pixel aspect ratio table
    PixelAspectRatioType PixelAspectRatioIndex() const
    {
        return m_pix_asr_idx;
    }

    // Clean area parameters
    //! Return the Clean area width
    unsigned int CleanWidth() const
    {
        return m_clean_width;
    }
    //! Return the Clean area height
    unsigned int CleanHeight() const
    {
        return m_clean_height;
    }
    //! Return the Clean area left offset
    unsigned int LeftOffset() const
    {
        return m_left_offset;
    }
    //! Return the Clean area top offset
    unsigned int TopOffset() const
    {
        return m_top_offset;
    }

    // Signal Range parameters

    //! Return the type from the signal range table
    SignalRangeType SignalRangeIndex() const
    {
        return m_sr_idx;
    }

    //! Return the luma offset
    unsigned int LumaOffset() const
    {
        return m_luma_offset;
    }
    //! Return the luma excursion
    unsigned int LumaExcursion() const
    {
        return m_luma_excursion;
    }
    //! Return the chroma offset
    unsigned int ChromaOffset() const
    {
        return m_chroma_offset;
    }
    //! Return the chroma excursion
    unsigned int ChromaExcursion() const
    {
        return m_chroma_excursion;
    }

    //! Return the index into the colour specification table
    unsigned int ColourSpecificationIndex() const
    {
        return m_cs_idx;
    }

    //! Return the colour primaries index
    ColourPrimaries ColourPrimariesIndex() const
    {
        return m_col_primary;
    }
    //! Return the colour matrix index
    ColourMatrix ColourMatrixIndex() const
    {
        return m_col_matrix;
    }
    //! Return the transfer function index
    TransferFunction TransferFunctionIndex() const
    {
        return m_transfer_func;
    }

    // Sets

    //! Sets the picture width
    void SetXl(unsigned int xlen)
    {
        m_xl = xlen;
    }

    //! Sets the picture height
    void SetYl(unsigned int ylen)
    {
        m_yl = ylen;
    }

    //! Sets the chroma format (Y only, 420, 422 etc)
    void SetCFormat(ChromaFormat cf)
    {
        m_cformat = cf;
    }

    //! Set if the source sampling field of the scan format
    void SetSourceSampling(unsigned int source_sampling)
    {
        m_source_sampling = source_sampling;
    }

    //! Set Topfield first. True if top field comes first in time
    void SetTopFieldFirst(bool tff)
    {
        m_topfieldfirst = tff;
    }

    //! Sets the video format
    void SetVideoFormat(VideoFormat vf)
    {
        m_video_format = vf;
    }

    //! Set the frame rate
    void SetFrameRate(const Rational &frate)
    {
        m_fr_idx = FRAMERATE_CUSTOM;
        m_framerate = frate;
    }

    //! Set the frame rate
    void SetFrameRate(unsigned int fr_num, unsigned int fr_denom)
    {
        m_fr_idx = FRAMERATE_CUSTOM;
        m_framerate.m_num = fr_num;
        m_framerate.m_denom = fr_denom;
    }

    //! Set the frame rate
    void SetFrameRate(FrameRateType fr);

    //! Set the pixel aspect ratio
    void SetPixelAspectRatio(const Rational &pix_asr)
    {
        m_pix_asr_idx = PIXEL_ASPECT_RATIO_CUSTOM;
        m_pixel_aspect_ratio = pix_asr;
    }

    //! Set the pixel aspect ratio
    void SetPixelAspectRatio(unsigned int pix_as_num, unsigned int pix_as_denom)
    {
        m_pix_asr_idx = PIXEL_ASPECT_RATIO_CUSTOM;
        m_pixel_aspect_ratio.m_num = pix_as_num;
        m_pixel_aspect_ratio.m_denom = pix_as_denom;
    }

    //! Set the Pixel Aspect Ratio
    void SetPixelAspectRatio(PixelAspectRatioType pixel_aspect_ratio);

    // Clean area parameters
    //! Set the Clean area width
    void SetCleanWidth(unsigned int clean_width)
    {
        m_clean_width = clean_width;
    }
    //! Set the Clean area height
    void SetCleanHeight(unsigned int clean_height)
    {
        m_clean_height = clean_height;
    }
    //! Set the Clean area left offset
    void SetLeftOffset(unsigned int left_offset)
    {
        m_left_offset = left_offset;
    }
    //! Set the Clean area top offset
    void SetTopOffset(unsigned int top_offset)
    {
        m_top_offset = top_offset;
    }

    // Signal Range parameters
    //! Set the Signal Range parameters
    void SetSignalRange(SignalRangeType sr);

    //! Set the luma offset
    void SetLumaOffset(unsigned int luma_offset)
    {
        m_sr_idx = SIGNAL_RANGE_CUSTOM;
        m_luma_offset = luma_offset;
    }
    //! Set the luma excursion
    void SetLumaExcursion(unsigned int luma_exc)
    {
        m_sr_idx = SIGNAL_RANGE_CUSTOM;
        m_luma_excursion = luma_exc;
    }
    //! Set the chroma offset
    void SetChromaOffset(unsigned int chroma_off)
    {
        m_sr_idx = SIGNAL_RANGE_CUSTOM;
        m_chroma_offset = chroma_off;
    }
    //! Set the chroma excursion
    void SetChromaExcursion(unsigned int chroma_exc)
    {
        m_sr_idx = SIGNAL_RANGE_CUSTOM;
        m_chroma_excursion = chroma_exc;
    }

    //! Set the Colour specification
    void SetColourSpecification(unsigned int cs_idx);
    //! Set the colour primaries index
    void SetColourPrimariesIndex(unsigned int cp);
    //! Set the colour matrix index
    void SetColourMatrixIndex(unsigned int cm);
    //! Set the transfer function index
    void SetTransferFunctionIndex(unsigned int tf);

private:
    //!Video-format
    VideoFormat m_video_format;

    //! Width of video
    unsigned int m_xl;

    //! Height of video
    unsigned int m_yl;

    //! Presence of chroma and/or chroma sampling structure
    ChromaFormat m_cformat;

    //! Source sampling field : 0 - progressive, 1 - interlaced
    unsigned int m_source_sampling;

    //! If m_source_sampling=1, true if the top field is first in temporal order
    bool m_topfieldfirst;

    //! Index into frame rate table
    FrameRateType m_fr_idx;

    //! Frame Rate i.e number of frames per second
    Rational m_framerate;

    //! Index into pixel aspect ratio table
    PixelAspectRatioType m_pix_asr_idx;

    //! Pixel Aspect Ratio
    Rational m_pixel_aspect_ratio;

    // Clean area parameters

    //! Clean area width
    unsigned int m_clean_width;

    //! Clean area height
    unsigned int m_clean_height;

    //! Clean area left offset
    unsigned int m_left_offset;

    //! Clean area top offset
    unsigned int m_top_offset;

    // signal range parameters

    //! Index into signal range table
    SignalRangeType m_sr_idx;

    //! Luma offset
    unsigned int m_luma_offset;
    //! Luma excursion
    unsigned int m_luma_excursion;
    //! Chroma offset
    unsigned int m_chroma_offset;
    //! Chroma excursion
    unsigned int m_chroma_excursion;

    //! Index into colour spec table
    unsigned int m_cs_idx;

    //! Colour Primaries Index
    ColourPrimaries m_col_primary;

    // Colour Matrix index
    ColourMatrix m_col_matrix;

    // Transfer function index
    TransferFunction m_transfer_func;
};


//! Parameters for initialising picture class objects
class PictureParams
{

public:
    //! Default constructor
    PictureParams();

    //! Constructor
    /*!
       Picture chroma format is set Picture sort defaults to I picture.
    */
    PictureParams(const ChromaFormat& cf, int xlen, int ylen,
                  unsigned int luma_depth, unsigned int chroma_depth);

    //! Constructor
    /*!
       Picture chroma format and picture sort are set.
    */
    PictureParams(const ChromaFormat& cf, const PictureSort& fs);

    //! Constructor
    /*!
       Constructor. Parameters are derived from the source parameters
    */
    PictureParams(const SourceParams& sparams);

    ////////////////////////////////////////////////////////////////////
    //NB: Assume default copy constructor, assignment = and destructor//
    ////////////////////////////////////////////////////////////////////

    // Gets ...

    //! Returns the chroma format of the picture
    const ChromaFormat& CFormat() const
    {
        return m_cformat;
    }

    //! Returns the picture width
    int Xl() const
    {
        return m_xl;
    }

    //! Returns the picture height
    int Yl() const
    {
        return m_yl;
    }

    //! Returns the chroma width of the picture
    int ChromaXl() const
    {
        return m_cxl;
    }

    //! Returns the chroma height of the picture
    int ChromaYl() const
    {
        return m_cyl;
    }

    //! Returns the luma depth
    unsigned int LumaDepth() const
    {
        return m_luma_depth;
    }

    //! Returns the chroma depth
    unsigned int ChromaDepth() const
    {
        return m_chroma_depth;
    }

    //! Returns the type of the picture
    const PictureSort& PicSort() const
    {
        return m_psort;
    }

    //! Returns the number of the picture (in time order)
    int PictureNum() const
    {
        return m_fnum;
    }

    //! Returns the retired reference picture number
    int RetiredPictureNum() const
    {
        return m_retd_fnum;
    }

    //! Returns whether the picture is bi-directionally predicted by checking references
    bool IsBPicture() const;

    //! Returns the number of pictures after the current picture number after which the picture can be discarded
    int ExpiryTime() const
    {
        return m_expiry_time;
    }

    //! Returns an indication of whether the picture has been output yet
    bool Output() const
    {
        return m_output;
    }

    //! Returns a const C++ reference to the set of reference picture numbers (will be empty if the picture is an I picture)
    const std::vector<int>& Refs() const
    {
        return m_refs;
    }

    //! Returns non-const C++ referece to the vector of reference pictures, to allow them to be set
    std::vector<int>& Refs()
    {
        return m_refs;
    }

    //! Return the number of reference pictures
    unsigned int NumRefs()const
    {
        return m_refs.size();
    }

    //! Returns type of picture (see enum)
    PictureType GetPictureType() const
    {
        return m_picture_type;
    }

    //! Returns reference picture type (see enum)
    ReferenceType GetReferenceType() const
    {
        return m_reference_type;
    }

    //! Returns true is entropy coding using Arithmetic coding
    bool UsingAC() const
    {
        return m_using_ac;
    }

    // ... Sets

    //! Sets the type of picture
    void SetPicSort(const PictureSort& ps);

    //! Sets the picture to be Intra/Inter
    void SetPictureType(const PictureType ftype);

    //! Sets the picture to be a reference or not
    void SetReferenceType(const ReferenceType rtype);

    //! Sets the picture number
    void SetPictureNum(const int fn)
    {
        m_fnum = fn;
    }

    //! Sets how long the picture will stay in the buffer (encoder only)
    void SetExpiryTime(const int expt)
    {
        m_expiry_time = expt;
    }

    //! Sets a flag to indicate that the picture has been output
    void SetAsOutput()
    {
        m_output = true;
    }

    //! Sets the chroma format
    void SetCFormat(ChromaFormat cf)
    {
        m_cformat = cf;
    }

    //! Sets the picture width
    void SetXl(int xlen);

    //! Sets the picture height
    void SetYl(int ylen);

    //! Set Luma Depth
    void SetLumaDepth(unsigned int luma_depth)
    {
        m_luma_depth = luma_depth;
    }

    //! Set Chroma Depth
    void SetChromaDepth(unsigned int chroma_depth)
    {
        m_chroma_depth = chroma_depth;
    }

    //! Sets the retired reference picture number
    void SetRetiredPictureNum(int retd_fnum)
    {
        m_retd_fnum = retd_fnum;
    }

    //! Sets the arithmetic coding flag
    void SetUsingAC(bool using_ac)
    {
        m_using_ac = using_ac;
    }

private:

    //! The chroma format
    ChromaFormat m_cformat;

    //! The picture sort
    PictureSort m_psort;

    //! The set of picture numbers of reference pictures
    std::vector<int> m_refs;

    //! The number of pictures, after the current picture number, after the (de)coding of which the picture can be deleted
    int m_expiry_time;

    //! The picture number, in temporal order
    int m_fnum;

    //! Picture type
    PictureType m_picture_type;

    //! Reference type
    ReferenceType m_reference_type;

    //! True if the picture has been output, false if not
    bool m_output;

    //! The picture number of the retired picture
    mutable  int m_retd_fnum;

    //! Picture luma width
    int m_xl;

    //! Picture luma height
    int m_yl;

    //! Picture chroma width
    int m_cxl;

    //! Picture chroma height
    int m_cyl;

    //! Luma depth - number of bits required for lumz
    unsigned int m_luma_depth;

    //! chroma depth - number of bits required for luma
    unsigned int m_chroma_depth;

    //! arithmetic coding flag
    bool m_using_ac;
};


//! A class for picture component data.
/*!
    A class for encapsulating picture data, derived from TwoDArray.
 */
class PicArray: public TwoDArray<ValueType>
{
public:
    //! Default constructor
    /*!
        Default constructor creates an empty array.
    */
    PicArray(): TwoDArray<ValueType>() {}

    //! Constructor.
    /*!
        Contructor creates a two-D array, with specified size and colour
        format.
    */
    PicArray(int height, int width, CompSort cs = Y_COMP):
        TwoDArray<ValueType>(height, width), m_csort(cs) {}

    //copy constructor and assignment= derived by inheritance

    //! Destructor
    ~PicArray() {}

    //! Return which component is stored
    const CompSort& CSort() const
    {
        return m_csort;
    }

    //! Set the type of component being stored
    void SetCSort(const CompSort cs)
    {
        m_csort = cs;
    }

private:

    CompSort m_csort;
};


//! A structure for recording costs, particularly in quantisation.
class CostType
{
public:
    //! The error (MSE or 4th power)
    double Error;

    //! The entropy in bits per symbol.
    double ENTROPY;

    //! The Lagrangian combination of MSE+lambda*entropy
    double TOTAL;
};


//! A class used for correcting estimates of entropy.
/*!
    A class used by the encoder for correcting estimates of entropy. Used
    for selecting quantisers in subband coefficient coding. Factors can be
    adjusted in the light of previous experience.
 */
class EntropyCorrector
{
public:
    //! Constructor.
    /*!
    Constructs arrays of correction factors of size.
    \param    depth    the depth of the wavelet transform.
    */
    EntropyCorrector(int depth);

    ////////////////////////////////////////////////////////////////////
    //NB: Assume default copy constructor, assignment = and destructor//
    ////////////////////////////////////////////////////////////////////

    //! Returns the correction factor.
    /*!
    Returns the correction factor for the band given also the type of
    picture and component.
    */
    float Factor(const int bandnum, const PictureParams& pp,
                 const CompSort c) const;

    //! Update the correction factors.
    /*!
    Update the factors for a given subband, component and picture type.
    \param    bandnum    the number of the subband to update
    \param    pp         picture parameters
    \param    c          component type
    \param    est_bits    the number of bits it was estimated would be used
    \param    actual_bits    the number of bits that actually were used
     */
    void Update(int bandnum, const PictureParams& pp,
                CompSort c, int est_bits, int actual_bits);

private:
    //! Initialises the correction factors
    void Init();

    TwoDArray<float> m_Yfctrs;
    TwoDArray<float> m_Ufctrs;
    TwoDArray<float> m_Vfctrs;
};

//! Parameters for overlapped block motion compensation
class OLBParams
{

public:

    //! Default constructor does nothing
    OLBParams() {}

    //! Constructor
    /*
        Constructor rationalises proposed parameters to allow suitable
        overlap and fit in with chroma format
        \param    xblen    the horizontal block length
        \param    yblen    the vertical block length
        \param    xblen    the horizontal block separation
        \param    yblen    the vertical block separation

    */
    OLBParams(const int xblen, const int yblen,
              const int xbsep, const int ybsep);

    // Gets ...

    //! Returns the horizontal block length
    int Xblen() const
    {
        return m_xblen;
    }

    //! Returns the vertical block length
    int Yblen() const
    {
        return m_yblen;
    }

    //! Returns the horizontal block separation
    int Xbsep() const
    {
        return m_xbsep;
    }

    //! Returns the vertical block separation
    int Ybsep() const
    {
        return m_ybsep;
    }

    //! The offset in the horizontal start of the block caused by overlap,=(XBLEN-XBSEP)/2
    int Xoffset() const
    {
        return m_xoffset;
    }

    //! The offset in the vertical start of the block caused by overlap,=(YBLEN-YBSEP)/2
    int Yoffset() const
    {
        return m_yoffset;
    }

    // ... and sets

    //! Sets the block width
    void SetXblen(int xblen)
    {
        m_xblen = xblen;
        m_xoffset = (m_xblen - m_xbsep) / 2;
    }

    //! Sets the block height
    void SetYblen(int yblen)
    {
        m_yblen = yblen;
        m_yoffset = (m_yblen - m_ybsep) / 2;
    }

    //! Sets the block horizontal separation
    void SetXbsep(int xbsep)
    {
        m_xbsep = xbsep;
        m_xoffset = (m_xblen - m_xbsep) / 2;
    }

    //! Sets the block vertical separation
    void SetYbsep(int ybsep)
    {
        m_ybsep = ybsep;
        m_yoffset = (m_yblen - m_ybsep) / 2;
    }

    bool operator == (const OLBParams bparams) const;

    // overloaded stream operators
    friend std::ostream & operator<< (std::ostream &, OLBParams &);
    friend std::istream & operator>> (std::istream &, OLBParams &);


private:

    int m_xblen;
    int m_yblen;
    int m_xbsep;
    int m_ybsep;
    int m_xoffset;
    int m_yoffset;
};

//! Parameters relating to the complexity of encoder/decoder
class ParseParams
{
public:
    //! Default constructor
    ParseParams();

    // Gets

    //! Get the major version
    unsigned int MajorVersion() const
    {
        return m_major_ver;
    }

    //! Get the minor version
    unsigned int MinorVersion() const
    {
        return m_minor_ver;
    }

    //! Get the Profile
    unsigned int Profile() const
    {
        return m_profile;
    }

    //! Get the Level
    unsigned int Level() const
    {
        return m_level;
    }

    // Sets

    //! Set the major version
    void SetMajorVersion(unsigned int major_ver)
    {
        m_major_ver = major_ver;
    }

    //! Set the minor version
    void SetMinorVersion(unsigned int minor_ver)
    {
        m_minor_ver = minor_ver;
    }

    //! Set the Profile
    void SetProfile(unsigned int profile)
    {
        m_profile = profile;
    }

    //! Set the Level
    void SetLevel(unsigned int level)
    {
        m_level = level;
    }

private:
    //! Major Version
    unsigned int m_major_ver;
    //! Minor Version
    unsigned int m_minor_ver;
    //! Profile
    unsigned int m_profile;
    //! Level
    unsigned int m_level;
};

//! Structure to hold code block sizes when spatial partitioning is used
class CodeBlocks
{
public:
    //! Default Constructor
    CodeBlocks() : m_hblocks(1), m_vblocks(1)
    {}

    //! Constructor
    CodeBlocks(unsigned int  hblocks, unsigned int vblocks) :
        m_hblocks(hblocks),
        m_vblocks(vblocks)
    {}

    // Gets
    //! Return the number of horizontal code blocks
    unsigned int HorizontalCodeBlocks() const
    {
        return m_hblocks;
    }
    //! Return the number of vertical code blocks
    unsigned int VerticalCodeBlocks() const
    {
        return m_vblocks;
    }
    // Sets
    //! Set the number of horizontal code blocks
    void SetHorizontalCodeBlocks(unsigned int hblocks)
    {
        m_hblocks = hblocks;
    }
    //! Set the number of vertical code blocks
    void SetVerticalCodeBlocks(unsigned int vblocks)
    {
        m_vblocks = vblocks;
    }
private:
    //! Number of Horizontal code blocks
    unsigned int m_hblocks;
    //! Number of Vertical code blocks
    unsigned int m_vblocks;
};

//! Structure to hold motion parameters when motion comp is used
class PicturePredParams
{
public:
    PicturePredParams():
        m_lbparams(3),
        m_cbparams(3) {}

    //! Return the global motion flag used for encoding/decoding
    bool UsingGlobalMotion() const
    {
        return m_use_global_motion;
    }

    //! Return the number of picture weight precision bits
    unsigned int PictureWeightsBits() const
    {
        return m_picture_weights_bits;
    }

    //! Return the Ref1 weight
    int Ref1Weight() const
    {
        return m_ref1_weight;
    }

    //! Return the Ref2 weight
    int Ref2Weight() const
    {
        return m_ref2_weight;
    }

    bool CustomRefWeights()
    {
        return (m_picture_weights_bits != 1 ||
                m_ref1_weight != 1 ||
                m_ref2_weight != 1);
    }

    //! Return the number of superblocks horizontally
    int XNumSB() const
    {
        return m_x_num_sb;
    }

    //! Return the number of superblocks vertically
    int YNumSB() const
    {
        return m_y_num_sb;
    }

    //! Return the number of blocks horizontally
    int XNumBlocks() const
    {
        return m_x_num_blocks;
    }

    //! Returns the number of blocks vertically
    int YNumBlocks() const
    {
        return m_y_num_blocks;
    }

    //! Return the Luma block parameters for each macroblock splitting level
    const OLBParams& LumaBParams(int n) const
    {
        return m_lbparams[n];
    }

    //! Return the Chroma block parameters for each macroblock splitting level
    const OLBParams& ChromaBParams(int n) const
    {
        return m_cbparams[n];
    }

    //! Return the number of accuracy bits used for motion vectors
    MVPrecisionType MVPrecision() const
    {
        return m_mv_precision;
    }

    //! Set how many SBs there are horizontally
    void SetXNumSB(const int xn)
    {
        m_x_num_sb = xn;
    }

    //! Set how many SBs there are vertically
    void SetYNumSB(const int yn)
    {
        m_y_num_sb = yn;
    }

    //! Set how many blocks there are horizontally
    void SetXNumBlocks(const int xn)
    {
        m_x_num_blocks = xn;
    }

    //! Set how many blocks there are vertically
    void SetYNumBlocks(const int yn)
    {
        m_y_num_blocks = yn;
    }

    //! Set the block sizes for all SB splitting levels given these prototype block sizes for level=2
    void SetBlockSizes(const OLBParams& olbparams , const ChromaFormat cformat);

    //! Set block level luma params
    void SetLumaBlockParams(const OLBParams& olbparams)
    {
        m_lbparams[2] = olbparams;
    }

    //! Set the number of accuracy bits for motion vectors
    void SetMVPrecision(const MVPrecisionType p)
    {
        // Assert in debug mode. Maybe we should throw an exception???
        TESTM((p >= 0 && p <= 3), "Motion precision value in range 0..3");
        m_mv_precision = p;
    }

    void SetMVPrecision(const MVPrecisionType p) const
    {
        // Assert in debug mode. Maybe we should throw an exception???
        TESTM((p >= 0 && p <= 3), "Motion precision value in range 0..3");
        m_mv_precision = p;
    }

    //! Set the wavelet filter used for picture (de)coding
    void SetUsingGlobalMotion(bool gm)
    {
        m_use_global_motion = gm;
    }

    //! Set the picture weight precision bits used for (de)coding
    void SetPictureWeightsPrecision(unsigned int wt_prec)
    {
        m_picture_weights_bits = wt_prec;
    }

    //! Set the ref 1 picture weight
    void SetRef1Weight(int wt)
    {
        m_ref1_weight = wt;
    }

    //! Set the ref 2 picture weight
    void SetRef2Weight(int wt)
    {
        m_ref2_weight = wt;
    }

private:

    //! The number of superblocks horizontally
    int m_x_num_sb;

    //! The number of superblocks verticaly
    int m_y_num_sb;

    //! The number of blocks horizontally
    int m_x_num_blocks;

    //! The number of blocks vertically
    int m_y_num_blocks;

    OneDArray<OLBParams> m_lbparams;

    OneDArray<OLBParams> m_cbparams;

    //! The precision of motion vectors (number of accuracy bits eg 1=half-pel accuracy)
    mutable MVPrecisionType m_mv_precision;

    //! picture predicion parameters - precision
    unsigned int m_picture_weights_bits;

    //! picture predicion parameters - reference picture 1 weight
    int m_ref1_weight;

    //! picture predicion parameters - reference picture 2 weight
    int m_ref2_weight;

    //! Global motion fields
    bool m_use_global_motion;

};

//! Parameters common to coder and decoder operation
/*!
    Parameters used throughout both the encoder and the decoder
*/
class CodecParams
{
public:

    //! Default constructor
    CodecParams(const VideoFormat& video_format = VIDEO_FORMAT_CUSTOM,
                PictureType ftype = INTRA_PICTURE,
                unsigned int num_refs = 0,
                bool set_defaults = true);

    ////////////////////////////////////////////////////////////////////
    //NB: Assume default copy constructor, assignment = and destructor//
    ////////////////////////////////////////////////////////////////////

    // Gets ...

    //! Returns the picture coding mode (independent of source format)
    /*! Returns the picture coding mode (independent of source format)
     *  0 = Frame coding (no quincunx)
     *  1 = Field coding (no quincunx)
     */
    int PictureCodingMode() const
    {
        return m_pic_coding_mode;
    }

    //! Returns true if the pictures are being coded as fields (mode 1 or 3)
    bool FieldCoding() const
    {
        return (m_pic_coding_mode == 1);
    }

    //! Returns true if the topmost field comes first in time when coding
    bool TopFieldFirst() const
    {
        return m_topfieldfirst;
    }

    //! Return the picture/field luma width
    int Xl() const
    {
        return m_xl;
    }

    //! Return the picture/field luma height
    int Yl() const
    {
        return m_yl;
    }

    //! Return the picture/field chroma width
    int ChromaXl() const
    {
        return m_cxl;
    }

    //! Return the picture/field chroma height
    int ChromaYl() const
    {
        return m_cyl;
    }

    //! Returns the luma depth
    unsigned int LumaDepth() const
    {
        return m_luma_depth;
    }

    //! Returns the chroma depth
    unsigned int ChromaDepth() const
    {
        return m_chroma_depth;
    }

    //! Return zero transform flag being used for picture (de)coding
    bool ZeroTransform() const
    {
        return m_zero_transform;
    }

    //! Return the wavelet filter currently being used for picture (de)coding
    WltFilter TransformFilter() const
    {
        return m_wlt_filter;
    }

    //! Return the transform depth being used for picture (de)coding
    unsigned int TransformDepth() const
    {
        return m_wlt_depth;
    }

    //! Return multiple quantisers flag being used for picture (de)coding
    CodeBlockMode GetCodeBlockMode() const
    {
        return m_cb_mode;
    }

    //! Return the spatial partitioning flag being used for picture (de)coding
    bool SpatialPartition() const
    {
        return m_spatial_partition;
    }

    //! Return the code blocks for a particular level
    const CodeBlocks &GetCodeBlocks(unsigned int level) const;

    //! Return the video format currently being used for picture (de)coding
    VideoFormat GetVideoFormat() const
    {
        return m_video_format;
    }

    //! Return the picture prediction params
    PicturePredParams& GetPicPredParams()
    {
        return m_picpredparams;
    }

    //! Return the picture prediction params
    const PicturePredParams& GetPicPredParams() const
    {
        return m_picpredparams;
    }

    // ... and Sets
    //! Sets whether input is coded as fields or quincunxially
    void SetPictureCodingMode(int pic_coding)
    {
        m_pic_coding_mode = pic_coding;
    }

    //! Sets whether the topmost field comes first in time [NB: TBD since this duplicates metadata in the sequence header]
    void SetTopFieldFirst(bool topf)
    {
        m_topfieldfirst = topf;
    }

    //! Set the picture/field luma width
    void SetXl(const int x)
    {
        m_xl = x;
    }

    //! Set the picture/field luma height
    void SetYl(const int y)
    {
        m_yl = y;
    }

    //! Set the frame/field chroma width
    void SetChromaXl(const int x)
    {
        m_cxl = x;
    }

    //! Set the frame/field chroma height
    void SetChromaYl(const int y)
    {
        m_cyl = y;
    }

    //! Set Luma Depth
    void SetLumaDepth(unsigned int luma_depth)
    {
        m_luma_depth = luma_depth;
    }

    //! Set Chroma Depth
    void SetChromaDepth(unsigned int chroma_depth)
    {
        m_chroma_depth = chroma_depth;
    }

    //! Set the zero transform flag being used for picture (de)coding
    void SetZeroTransform(bool zero_transform)
    {
        m_zero_transform = zero_transform;
    }

    //! Set the wavelet filter used for picture (de)coding
    void SetTransformFilter(const WltFilter wf)
    {
        m_wlt_filter = wf;
    }

    //! Set the wavelet filter used for picture (de)coding
    void SetTransformFilter(unsigned int wf_idx);

    //! Set the transform depth used for picture (de)coding and allocate for the code blocks array
    void SetTransformDepth(unsigned int wd);

    //! Set the multiple quantisers flag usedto picture (de)coding
    void SetCodeBlockMode(unsigned int cb_mode);

    //! Set the spatial partition flag usedto picture (de)coding
    void SetSpatialPartition(bool spatial_partition)
    {
        m_spatial_partition = spatial_partition;
    }

    //! Set the number of code blocks for a particular level
    void  SetCodeBlocks(unsigned int level, unsigned int hblocks, unsigned int vblocks);

    //! Set the video format used for picture (de)coding
    void SetVideoFormat(const VideoFormat vd)
    {
        m_video_format = vd;
    }

protected:
    //! Return the Wavelet filter associated with the wavelet index
    WltFilter TransformFilter(unsigned int wf_idx);
private:

    //! The picture prediction parameters
    PicturePredParams m_picpredparams;

    //! The picture coding mode
    int m_pic_coding_mode;

    //! True if interlaced and top field is first in temporal order
    bool m_topfieldfirst;

    //! The frame/field luma width
    int m_xl;

    //! The frame/field luma height
    int m_yl;

    //! The frame/field chroma width
    int m_cxl;

    //! The frame/field chroma height
    int m_cyl;

    //! Luma depth - number of bits required for lumz
    unsigned int m_luma_depth;

    //! chroma depth - number of bits required for luma
    unsigned int m_chroma_depth;

    //! The video format being used
    VideoFormat m_video_format;

    //! Zero transform flag
    bool m_zero_transform;

    //! The wavelet filter being used
    WltFilter m_wlt_filter;

    //! Wavelet depth
    unsigned int m_wlt_depth;

    //! Code block mode
    CodeBlockMode m_cb_mode;

    //! Spatial partitioning flag
    bool m_spatial_partition;

    //! Code block array. Number of entries is m_wlt_depth+1
    OneDArray<CodeBlocks> m_cb;
};

//! Parameters for the encoding process
/*!
    Parameters for the encoding process, derived from CodecParams.
 */
class EncoderParams: public CodecParams
{
    //codec params plus parameters relating solely to the operation of the encoder

public:
    //! Default constructor
    EncoderParams(const VideoFormat& video_format,
                  PictureType ftype = INTER_PICTURE,
                  unsigned int num_refs = 2,
                  bool set_defaults = true);

    ////////////////////////////////////////////////////////////////////
    //NB: Assume default copy constructor, assignment = and destructor//
    //This means pointers are copied, not the objects they point to.////
    ////////////////////////////////////////////////////////////////////

    // Gets ...


    //! Returns true if we're operating verbosely, false otherwise
    bool Verbose() const
    {
        return m_verbose;
    }

    //! Returns a flag indicating that we're doing local decoding
    bool LocalDecode() const
    {
        return m_loc_decode;
    }

    //! Get whether we're doing lossless coding
    bool Lossless() const
    {
        return m_lossless;
    }

    //! Get whether we're doing full-search motion estimation
    bool FullSearch() const
    {
        return m_full_search;
    }

    //! Get the horizontal search range for full-search motion estimation
    int XRangeME() const
    {
        return m_x_range_me;
    }

    //! Get the vertical search range for full-search motion estimation
    int YRangeME() const
    {
        return m_y_range_me;
    }

    //! Get whether we're doing combined component motion estimation
    bool CombinedME() const
    {
        return m_combined_me;
    }

    //! Get the quality factor
    float Qf() const
    {
        return m_qf;
    }

    //! Return the nominal number of L1 pictures before the next I picture
    /*!
        Return the nominal number of L1 pictures before the next I picture. Can be
        overridden by I-picture insertion

    */
    int NumL1() const
    {
        return m_num_L1;
    }

    //! Return the separation between L1 pictures (and between L1 and I pictures)
    int L1Sep() const
    {
        return m_L1_sep;
    }

    //! Return the amount we're weighting noise in the U component
    float UFactor() const
    {
        return m_ufactor;
    }

    //! Return the amount we're weighting noise in the V component
    float VFactor() const
    {
        return m_vfactor;
    }

    //! Return the number of cycles per degree at the nominal viewing distance for the raster
    float CPD() const
    {
        return m_cpd;
    }

    //! Return what prefiltering is in place
    PrefilterType Prefilter() const
    {
        return m_prefilter;
    }

    //! Return the prefiltering strength
    int PrefilterStrength() const
    {
        return m_prefilter_strength;
    }

    //! Return the Lagrangian parameter to be used for I pictures
    float ILambda() const
    {
        return m_I_lambda;
    }

    //! Return the Lagrangian parameter to be used for L1 pictures
    float L1Lambda() const
    {
        return m_L1_lambda;
    }

    //! Return the Lagrangian parameter to be used for L2 pictures
    float L2Lambda() const
    {
        return m_L2_lambda;
    }

    //! Return the Lagrangian ME parameter to be used for L1 pictures
    float L1MELambda() const
    {
        return m_L1_me_lambda;
    }

    //! Return the Lagrangian ME parameter to be used for L2 pictures
    float L2MELambda() const
    {
        return m_L2_me_lambda;
    }

    //! Return the size of the GOP
    int GOPLength() const;

    //! Return the output path to be used for storing diagnositic data
    char * OutputPath() const
    {
        return (char*) m_output_path.c_str();
    }

    //! Return a reference to the entropy factors
    const EntropyCorrector& EntropyFactors() const
    {
        return *m_ent_correct;
    }

    //! Return a reference to the entropy factors - we need to be able to change the values of the entropy factors in situ
    EntropyCorrector& EntropyFactors()
    {
        return *m_ent_correct;
    }

    //! Return the Wavelet filter to be used for intra pictures
    WltFilter IntraTransformFilter()
    {
        return m_intra_wltfilter;
    }

    //! Return the Wavelet filter to be used for Inter pictures
    WltFilter InterTransformFilter()
    {
        return m_inter_wltfilter;
    }

    //! Return the Target Bit Rate in kbps
    int TargetRate()
    {
        return m_target_rate;
    }

    //! Return true if using Arithmetic coding
    bool UsingAC()  const
    {
        return m_using_ac;
    }

    // ... and Sets

    //! Sets verbosity on or off
    void SetVerbose(bool v)
    {
        m_verbose = v;
    }

    //! Sets a flag indicating that we're producing a locally decoded o/p
    void SetLocalDecode(const bool decode)
    {
        m_loc_decode = decode;
    }

    //! Set whether we're doing lossless coding
    void SetLossless(const bool l)
    {
        m_lossless = l;
    }

    //! Set whether we're doing full-search motion estimation
    void SetFullSearch(const bool fs)
    {
        m_full_search = fs;
    }

    //! Set whether we're doing combined component motion estimation
    void SetCombinedME(const bool cme)
    {
        m_combined_me = cme;
    }

    //! Set the horizontal search range for full-search motion estimation
    void SetXRangeME(const int xr)
    {
        m_x_range_me = xr;
    }

    //! Set the vertical search range for full-search motion estimation
    void SetYRangeME(const int yr)
    {
        m_y_range_me = yr;
    }

    //! Set the quality factor
    void SetQf(const float qfac)
    {
        m_qf = qfac;
        CalcLambdas(m_qf);
    }

    //! Set the nominal number of L1 pictures between I pictures
    void SetNumL1(const int nl)
    {
        m_num_L1 = nl;
    }

    //! Set the separation between L1 pictures
    void SetL1Sep(const int lsep)
    {
        m_L1_sep = lsep;
    }

    //! Set the amount to weight noise in the U component
    void SetUFactor(const float uf)
    {
        m_ufactor = uf;
    }

    //! Set the amount to weight noise in the V component
    void SetVFactor(const float vf)
    {
        m_vfactor = vf;
    }

    //! Set the number of cycles per degree at the nominal viewing distance
    void SetCPD(const float cpd)
    {
        m_cpd = cpd;
    }

    //! Set denoising value - true or false
    void SetPrefilter(const PrefilterType pf, const int str)
    {
        m_prefilter = pf;
        m_prefilter_strength = str;
    }

    //! Set the output path to be used for diagnostic data
    void SetOutputPath(const char * op)
    {
        m_output_path = op;
    }

    //! Sets the entropy factors - TBD: set this up in a constructor and pass encoder params around entirely by reference
    void SetEntropyFactors(EntropyCorrector* entcorrect)
    {
        m_ent_correct = entcorrect;
    }
    //! Set the Wavelet filter to be used for intra pictures
    void SetIntraTransformFilter(unsigned int wf_idx);

    //! Set the Wavelet filter to be used for inter pictures
    void SetInterTransformFilter(unsigned int wf_idx);

    //! Set the Wavelet filter to be used for intra pictures
    void SetIntraTransformFilter(WltFilter wf)
    {
        m_intra_wltfilter = wf;
    }

    //! Set the number of code blocks for all levels
    void  SetUsualCodeBlocks(const PictureType& ftype);

    //! Set the Wavelet filter to be used for inter pictures
    void SetInterTransformFilter(WltFilter wf)
    {
        m_inter_wltfilter = wf;
    }

    //! Set the target bit rate
    void SetTargetRate(const int rate)
    {
        m_target_rate = rate;
    }

    //! Set the arithmetic coding flag
    void SetUsingAC(bool using_ac)
    {
        m_using_ac = using_ac;
    }
private:

    //! Calculate the Lagrangian parameters from the quality factor
    void CalcLambdas(const float qf);

private:

    //! Code/decode with commentary if true
    bool m_verbose;

    //! Flag indicating we're doing local decoding
    bool m_loc_decode;

    //! A flag indicating we're doing lossless coding
    bool m_lossless;

    //! A flag indicating whether we're doing full-search block matching
    bool m_full_search;

    //! A flag indicating whether we're doing combined component motion estimation
    bool m_combined_me;

    //! The horizontal range for full-search block matching
    int m_x_range_me;

    //! The vertical range for full-search block matching
    int m_y_range_me;

    //! Quality factor
    float m_qf;

    //! Number of L1 pictures before next I picture
    int m_num_L1;

    //! Separation between L1 pictures
    int m_L1_sep;

    //! factor for weighting U component quantisation errors
    float m_ufactor;

    //! factor for weighting V component quantisation errors
    float m_vfactor;

    //! Cycles per degree assumed for viewing the video
    float m_cpd;

    //! Indicator for prefiltering
    PrefilterType m_prefilter;

    //! Prefiltering strength
    int m_prefilter_strength;

    //! Lagrangian parameter for Intra picture coding
    float m_I_lambda;

    //! Lagrangian parameter for L1 picture coding
    float m_L1_lambda;

    //! Lagrangian parameter for L2 picture coding
    float m_L2_lambda;

    //! Lagrangian param for L1 motion estimation
    float m_L1_me_lambda;

    //! Lagrangian param for L2 motion estimation
    float m_L2_me_lambda;

    //! Correction factors for quantiser selection
    EntropyCorrector* m_ent_correct;

    //! Output file path
    std::string m_output_path;

    //! Wavelet filter for Intra pictures
    WltFilter m_intra_wltfilter;

    //! Wavelet filter for Inter pictures
    WltFilter m_inter_wltfilter;

    //! Target bit rate
    int m_target_rate;

    //! Arithmetic coding flag
    bool m_using_ac;

};

//! Parameters for the decoding process
/*!
    Parameters for the decoding process. Derived from CodecParams.
 */
class DecoderParams: public CodecParams
{
public:
    //! Default constructor
    DecoderParams(const VideoFormat& video_format = VIDEO_FORMAT_CIF, PictureType ftype = INTRA_PICTURE, unsigned int num_refs = 0, bool set_defaults = false);

    //! Returns true if we're operating verbosely, false otherwise
    bool Verbose() const
    {
        return m_verbose;
    }

    //! Sets verbosity on or off
    void SetVerbose(bool v)
    {
        m_verbose = v;
    }

    ////////////////////////////////////////////////////////////////////
    //NB: Assume default copy constructor, assignment = and destructor//
    //This means pointers are copied, not the objects they point to.////
    ////////////////////////////////////////////////////////////////////


private:

    //! Code/decode with commentary if true
    bool m_verbose;

};

//! A simple bounds checking function, very useful in a number of places
inline ValueType BChk(const ValueType &num, const ValueType &max)
{
    if(num < 0) return 0;
    else if(num >= max) return max - 1;
    else return num;
}

//! Class for encapsulating quantiser data
class QuantiserLists
{
public:
    //! Default constructor
    QuantiserLists();

    //! Returns 4 times the quantisation factor
    inline int QuantFactor4(const int index) const
    {
        return m_qflist4[index];
    }

    //! Returns the intra Picture quantisation offset for non-zero values
    inline int IntraQuantOffset4(const int index) const
    {
        return m_intra_offset4[index];
    }
    //! Returns the inter Picture quantisation offset for non-zero values
    inline int InterQuantOffset4(const int index) const
    {
        return m_inter_offset4[index];
    }

    //! Returns the maximum quantiser index supported
    inline int MaxQuantIndex() const
    {
        return m_max_qindex;
    }


private:
    unsigned int m_max_qindex;
    OneDArray<int> m_qflist4;
    OneDArray<int> m_intra_offset4;
    OneDArray<int> m_inter_offset4;

};

//! A constant list of the quantisers being used in Dirac
static const QuantiserLists dirac_quantiser_lists;

} // namespace dirac

#endif
