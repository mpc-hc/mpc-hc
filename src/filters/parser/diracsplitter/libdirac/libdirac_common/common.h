/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: common.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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
*                 Tim Borer
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

#include <libdirac_common/bit_manager.h>
#include <libdirac_common/arrays.h>
#include <libdirac_common/common_types.h>
#include <vector>
#include <cmath>

namespace dirac
{
    /*! \file
        This file contains common classes used throughout the encoder and 
        decoder.  The main classes are the encoder and decoder parameters for 
        controlling the encode and decode processes. These are passed 
        throughout the codec.  There are also parameter classes for sequences 
        and frames.
    */


    //Some basic enumeration types used throughout the codec ...//
    //////////////////////////////////////////////////////////////

    //! Prediction modes for blocks
    enum PredMode{ INTRA , REF1_ONLY , REF2_ONLY , REF1AND2 };

    //! Types of picture component
    enum CompSort{ Y_COMP , U_COMP , V_COMP , R_COMP , G_COMP , B_COMP };

    //! Addition or subtraction
    enum AddOrSub{ ADD , SUBTRACT };

    //! Forward or backward
    enum Direction { FORWARD , BACKWARD };

    //! Currently only Daubechies (DAUB) implemented
    enum WltFilter { DAUB , HAAR };

    //! Contexts used for coefficient coding
    enum CtxAliases
    {//used for residual coding
        SIGN0_CTX,          //0     -sign, previous symbol is 0
        SIGN_POS_CTX,       //1     -sign, previous symbol is +ve
        SIGN_NEG_CTX,       //2     -sign, previous symbol is -ve
        
        
        Z_BIN1z_CTX,        //3     -bin 1, parent is zero, neighbours zero
        Z_BIN1nz_CTX,       //4     -bin 1, parent is zero, neighbours non-zero
        Z_BIN2_CTX,         //5     -bin 2, parent is zero
        Z_BIN3_CTX,         //6     -bin 3, parent is zero
        Z_BIN4_CTX,         //7     -bin 4, parent is zero
        Z_BIN5plus_CTX,     //8     -bins 5 plus, parent is zero
        
        NZ_BIN1z_CTX,       //9     -bin 1, parent is non-zero, neighbours zero
        NZ_BIN1a_CTX,       //10    -bin 1, parent is non-zero, neighbours small
        NZ_BIN1b_CTX,       //11    -bin 1, parent is non-zero, neighbours large
        NZ_BIN2_CTX,        //12    -bin 2, parent is non-zero
        NZ_BIN3_CTX,        //13    -bin 3, parent is non-zero
        NZ_BIN4_CTX,        //14    -bin 4, parent is non-zero
        NZ_BIN5plus_CTX,    //15    -bins 5 plus, parent is non-zero
        
        ZTz_CTX,            //16    -zerotree, neighbouring symbols are zerotree elements
        ZTnz_CTX,           //17    -zerotree, neighbouring symbols are not zerotree elements
        ZTzb_CTX,           //16    -zerotree, neighbouring symbols are zerotree elements
        ZTnzb_CTX           //17    -zerotree, neighbouring symbols are not zerotree elements
    };

    //! Contexts used for MV data coding
    enum MvCtxAliases
    {
        
        YDC_BIN1_CTX,       //0     -1st bin of DC value for Y
        YDC_BIN2plus_CTX,   //1     -remaining DC bins
        YDC_SIGN0_CTX,      //2     -sign of Y DC value, previous value 0
        UDC_BIN1_CTX,       //3     --ditto
        UDC_BIN2plus_CTX,   //4     --for
        UDC_SIGN0_CTX,      //5     --U
        VDC_BIN1_CTX,       //6     --and
        VDC_BIN2plus_CTX,   //7     --V
        VDC_SIGN0_CTX,      //8     --components
        
        REF1x_BIN1_CTX,     //9     -bin 1, REF1 x vals
        REF1x_BIN2_CTX,     //10    -bin 2, REF1 x vals
        REF1x_BIN3_CTX,     //11    -bin 3, REF1 x vals
        REF1x_BIN4_CTX,     //12    -bin 4, REF1 x vals
        REF1x_BIN5plus_CTX, //13    -bin 5, REF1 x vals
        REF1x_SIGN0_CTX,    //14    -sign, REF1 x vals, previous value 0
        REF1x_SIGNP_CTX,    //15    -sign, REF1 x vals, previous value +ve
        REF1x_SIGNN_CTX,    //16    -sign, REF1 x vals, previous value -ve
        
        REF1y_BIN1_CTX,     //17    -bin 1, REF1 y vals
        REF1y_BIN2_CTX,     //18    -bin 2, REF1 y vals
        REF1y_BIN3_CTX,     //19    -bin 3, REF1 y vals
        REF1y_BIN4_CTX,     //20    -bin 4, REF1 y vals
        REF1y_BIN5plus_CTX, //21    -bin 5, REF1 y vals
        REF1y_SIGN0_CTX,    //22    -sign, REF1 y vals, previous value 0
        REF1y_SIGNP_CTX,    //23    -sign, REF1 y vals, previous value +ve
        REF1y_SIGNN_CTX,    //24    -sign, REF1 y vals, previous value -ve
        
        REF2x_BIN1_CTX,     //25    -bin 1, REF2 x vals
        REF2x_BIN2_CTX,     //26    -bin 2, REF2 x vals
        REF2x_BIN3_CTX,     //27    -bin 3, REF2 x vals
        REF2x_BIN4_CTX,     //28    -bin 4, REF2 x vals
        REF2x_BIN5plus_CTX, //29    -bin 5, REF2 x vals
        REF2x_SIGN0_CTX,    //30    -sign, REF2 x vals, previous value 0
        REF2x_SIGNP_CTX,    //31    -sign, REF1 y vals, previous value +ve
        REF2x_SIGNN_CTX,    //32    -sign, REF1 y vals, previous value -ve
        
        REF2y_BIN1_CTX,     //33    -bin 1, REF2 y vals
        REF2y_BIN2_CTX,     //34    -bin 2, REF2 y vals
        REF2y_BIN3_CTX,     //35    -bin 3, REF2 y vals
        REF2y_BIN4_CTX,     //36    -bin 4, REF2 y vals
        REF2y_BIN5plus_CTX, //37    -bin 5, REF2 y vals
        REF2y_SIGN0_CTX,    //38    -sign, REF2 y vals, previous value 0
        REF2y_SIGNP_CTX,    //39    -sign, REF2 y vals, previous value +ve
        REF2y_SIGNN_CTX,    //40    -sign, REF2 y vals, previous value -ve
        
        PMODE_BIN1_CTX,     //41    -bin 1, prediction mode value
        PMODE_BIN2_CTX,     //42    -bin 2, prediction mode value
        PMODE_BIN3_CTX,     //43    -bin 3, prediction mode value. Bin 4 not required
        
        MB_CMODE_CTX,       //44    -context for MB common block mode
        MB_SPLIT_BIN1_CTX,  //45    -bin1, MB split mode vals
        MB_SPLIT_BIN2_CTX   //46    -bin2, MB split mode vals. Bin 3 not required
        
    };


    //Classes used throughout the codec//
    /////////////////////////////////////


    //! A class for picture component data.
    /*!
        A class for encapsulating picture data, derived from TwoDArray. NB: 
        in the future there will be separate classes for input/output picture 
        data, difference picture data, and wavelet coefficient data. Currently 
        PicArray is used for all of these. TJD 13 April 2004.
     */
    class PicArray: public TwoDArray<ValueType>
    {
    public:
        //! Default constructor
        /*!
            Default constructor creates an empty array.
        */
        PicArray(): TwoDArray<ValueType>(){}
        
        //! Constructor.
        /*!
            Contructor creates a two-D array, with specified size and colour 
            format.
        */
        PicArray(int height, int width, CompSort cs=Y_COMP);
        
        //copy constructor and assignment= derived by inheritance
        
        //! Destructor
        ~PicArray(){}
        
        //! Return which component is stored
        const CompSort& CSort() const;
        
        //! Set the type of component being stored
        void SetCSort(const CompSort cs);
        
    private:
        
        CompSort m_csort;
    };


    //! A structure for recording costs, particularly in quantisation.
    class CostType
    {
    public:
        //! The Mean Square Error    
        double MSE;
        
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
        frame and component.
        */
        float Factor(const int bandnum, const FrameSort fsort,const CompSort c) const;

        //! Update the correction factors.
        /*!
        Update the factors for a given subband, component and frame type.
        \param    bandnum    the number of the subband to update
        \param    fsort      frame type
        \param    c          component type
        \param    est_bits    the number of bits it was estimated would be used
        \param    actual_bits    the number of bits that actually were used
         */    
        void Update(int bandnum, FrameSort fsort, CompSort c,int est_bits,int actual_bits);
        
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
        OLBParams(){}
        
        //! Constructor
        /*
            Constructor rationalises proposed parameters to allow suitable 
            overlap and fit in with
            chroma format
            \param    xblen    the horizontal block length    
            \param    yblen    the vertical block length
            \param    xblen    the horizontal block separation
            \param    yblen    the vertical block separation

        */
        OLBParams(const int xblen, int const yblen, int const xbsep, int const ybsep);
        
        // Gets ...
        
        //! Returns the horizontal block length
        int Xblen() const {return m_xblen;}
        
        //! Returns the vertical block length
        int Yblen() const {return m_yblen;}
        
        //! Returns the horizontal block separation
        int Xbsep() const {return m_xbsep;}
        
        //! Returns the vertical block separation
        int Ybsep() const {return m_ybsep;}
        
        //! The offset in the horizontal start of the block caused by overlap,=(XBLEN-XBSEP)/2
        int Xoffset() const {return m_xoffset;}
        
        //! The offset in the vertical start of the block caused by overlap,=(YBLEN-YBSEP)/2
        int Yoffset() const {return m_yoffset;}
        
        // ... and sets
        
        //! Sets the block width
        void SetXblen( int xblen ){ m_xblen = xblen; m_xoffset = (m_xblen-m_xbsep)/2;}
        
        //! Sets the block height
        void SetYblen( int yblen ){ m_yblen = yblen; m_yoffset = (m_yblen-m_ybsep)/2;}
        
        //! Sets the block horizontal separation
        void SetXbsep( int xbsep ){ m_xbsep = xbsep; m_xoffset = (m_xblen-m_xbsep)/2;}
        
        //! Sets the block vertical separation
        void SetYbsep( int ybsep ){ m_ybsep = ybsep; m_yoffset = (m_yblen-m_ybsep)/2;}
        
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

    //! Parameters relating to the video sequence being encoded/decoded
    class SeqParams
    {
    public:        
        //! Default Constructor 
        SeqParams();
        
        ////////////////////////////////////////////////////////////////////
        //NB: Assume default copy constructor, assignment = and destructor//
        ////////////////////////////////////////////////////////////////////    
        
        //gets ...
        //! Returns the picture width
        int Xl() const {return m_xl;}
        
        //! Returns the picture height
        int Yl() const {return m_yl;}
        
        //! Returns the chroma format of the sequence (Y only, 420, 422 etc)
        ChromaFormat CFormat() const {return m_cformat;}
        
        //! Returns the chroma width
        int ChromaWidth() const;
        
        //! Returns the chroma height
        int ChromaHeight() const;
        
        //! Returns true if the sequence is interlaced
        bool Interlace() const {return m_interlace;}
        
        //! Returns true if the top field comes first in time
        bool TopFieldFirst() const {return m_topfieldfirst;}
        
        //! Returns the number of frames to be displayed per second
        int FrameRate() const {return m_framerate;}
        
        //! Returns the bitstream version
        int BitstreamVersion() const {return m_bs_ver;}
        
        // ... Sets
        
        //! Sets the picture width
        void SetXl(int xlen) {m_xl = xlen;}
        
        //! Sets the picture height
        void SetYl(int ylen) {m_yl = ylen;}
        
        //! Sets the chroma format (Y only, 420, 422 etc)
        void SetCFormat(ChromaFormat cf) {m_cformat=cf;}
        
        //! Sets the interlace flag: true if the sequence is interlaced, false otherwise
        void SetInterlace(bool ilace) {m_interlace=ilace;}
        
        //! Sets the 'top field first' flag: true if the top field comes first in time
        void SetTopFieldFirst(bool tff) {m_topfieldfirst=tff;}
        
        //! Sets the number of frames to be displayed per second
        void SetFrameRate(int fr){m_framerate=fr;}
        
        //! Sets the bitstream version
        void SetBitstreamVersion(int bs_ver){m_bs_ver=bs_ver;}
        
    private:
        //! Width of video
        int m_xl;
        
        //! Height of video
        int m_yl;
        
        //! Presence of chroma and/or chroma sampling structure 
        ChromaFormat m_cformat;
        
        //! True if interlaced
        bool m_interlace;
        
        //! If interlaced, true if the top field is first in temporal order
        bool m_topfieldfirst;
        
        //! Frame rate, per second
        int m_framerate;
        
        //! Bitsream version.
        unsigned char  m_bs_ver;
    };

    //! Parameters for initialising frame class objects
    class FrameParams 
    {
        
    public:
        //! Default constructor
        FrameParams();
        
        //! Constructor 
        /*!
           Frame chroma format is set Frame sort defaults to I frame.
        */    
        FrameParams(const ChromaFormat& cf, int xlen, int ylen);
        
        //! Constructor
        /*!
           Frame chroma format and frame sort are set.
        */    
        FrameParams(const ChromaFormat& cf, const FrameSort& fs);
        
        //! Constructor
        /*
            All data is derived from the sequence parameters
        */
        FrameParams(const SeqParams& sparams);
        
        //! Constructor
        /*
           All data is derived from the sequence parameters
        */
        FrameParams(const SeqParams& sparams, const FrameSort& fs);
        
        ////////////////////////////////////////////////////////////////////
        //NB: Assume default copy constructor, assignment = and destructor//
        ////////////////////////////////////////////////////////////////////    
        
        // Gets ...
        
        //! Returns the chroma format of the frame
        const ChromaFormat& CFormat() const{return m_cformat;}
        
        //! Returns the width of the frame
        int Xl() const{return m_xl;}
        
        //! Returns the height of the frame
        int Yl() const{return m_yl;}
        
        //! Returns the type of the frame (I, L1 or L2)
        const FrameSort& FSort() const {return m_fsort;}
        
        //! Returns the number of the frame (in time order)
        int FrameNum() const {return m_fnum;}
        
        //! Returns the number of frames after the current frame number after which the frame can be discarded
        int ExpiryTime() const {return m_expiry_time;}
        
        //! Returns an indication of whether the frame has been output yet
        bool Output() const {return m_output;}
        
        //! Returns a const C++ reference to the set of reference frame numbers (will be empty if the frame is an I frame)
        const std::vector<int>& Refs() const {return m_refs;}
        
        //! Returns non-const C++ referece to the vector of reference frames, to allow them to be set
        std::vector<int>& Refs(){return m_refs;}
        
        
        // ... Sets
        
        //! Sets the type of frame to I, L1 or L2
        void SetFSort( const FrameSort& fs ){ m_fsort=fs; }
        
        //! Sets the frame number
        void SetFrameNum( const int fn ){ m_fnum=fn; }
        
        //! Sets how long the frame will stay in the buffer
        void SetExpiryTime( const int expt ){ m_expiry_time=expt; }
        
        //! Sets a flag to indicate that the frame has been output
        void SetAsOutput(){m_output=true;}
        
    private:
        
        //! The chroma format
        ChromaFormat m_cformat;
        
        //! Frame width
        int m_xl;
        
        //!    Frame height
        int m_yl;
        
        //! The frame sort
        FrameSort m_fsort;
        
        //! The set of frame numbers of reference frames
        std::vector<int> m_refs;
        
        //! The number of frames, after the current frame number, after the (de)coding of which the frame can be deleted
        int m_expiry_time;
        
        //! True if the frame has been output, false if not
        bool m_output;
        
        //! The frame number, in temporal order
        int m_fnum;        
    };


    //! Parameters common to coder and decoder operation
    /*!
        Parameters used throughout both the encoder and the decoder
    */
    class CodecParams
    {
    public:
        
        //! Default constructor 
        CodecParams();
        
            ////////////////////////////////////////////////////////////////////
            //NB: Assume default copy constructor, assignment = and destructor//
            ////////////////////////////////////////////////////////////////////
        
        // Gets ...    
        
        //! Return the number of macroblocks horizontally
        int XNumMB() const {return m_x_num_mb;}
        
        //! Return the number of macroblocks vertically
        int YNumMB() const {return m_y_num_mb;}
        
        //! Return the number of blocks horizontally
        int XNumBlocks() const {return m_x_num_blocks;}
        
        //! Returns the number of blocks vertically
        int YNumBlocks() const {return m_y_num_blocks;}
        
        //! Returns true if we're operating verbosely, false otherwise
        bool Verbose() const {return m_verbose;}
        
        //! Returns true if we're operatung using interlace tools [not currently defined]
        bool Interlace() const {return m_interlace;}
        
        //! Returns true if the topmost field comes first in time [NB: TBD since this duplicates metadata in the sequence header]
        bool TopFieldFirst() const {return m_topfieldfirst;}    
        
        //! Return the Luma block parameters for each macroblock splitting level
        const OLBParams& LumaBParams(int n) const {return m_lbparams[n];}
        
        //! Return the Chroma block parameters for each macroblock splitting level
        const OLBParams& ChromaBParams(int n) const {return m_cbparams[n];}    

        //! Return the original frame width
        int OrigXl() const {return m_orig_xl;}

        //! Return the original frame height
        int OrigYl() const {return m_orig_yl;}
        
        // ... and Sets
        //! Set how many MBs there are horizontally
        void SetXNumMB(const int xn){m_x_num_mb=xn;}    
        
        //! Set how many MBs there are vertically
        void SetYNumMB(const int yn){m_y_num_mb=yn;}
        
        //! Set how many blocks there are horizontally
        void SetXNumBlocks(const int xn){m_x_num_blocks=xn;}
        
        //! Set how many blocks there are vertically
        void SetYNumBlocks(const int yn){m_y_num_blocks=yn;}
        
        //! Sets verbosity on or off
        void SetVerbose(bool v){m_verbose=v;}
        
        //! Sets whether interlace tools are to be used
        void SetInterlace(bool intlc){m_interlace=intlc;}
        
        //! Sets whether the topmost field comes first in time [NB: TBD since this duplicates metadata in the sequence header]
        void SetTopFieldFirst(bool topf){m_topfieldfirst=topf;}
        
        //! Set the block sizes for all MB splitting levels given these prototype block sizes for level=2
        void SetBlockSizes(const OLBParams& olbparams , const ChromaFormat cformat);

        //! Set the original frame width
        void SetOrigXl(const int x){m_orig_xl=x;}

        //! Set the original frame height
        void SetOrigYl(const int y){m_orig_yl=y;}

    private:
        
        //! The number of macroblocks horizontally
        int m_x_num_mb;
        
        //! The number of macroblocks verticaly
        int m_y_num_mb;
        
        //! The number of blocks horizontally
        int m_x_num_blocks;    
        
        //! The number of blocks vertically
        int m_y_num_blocks;
        
        //! Code/decode with commentary if true    
        bool m_verbose;
        
        //! True if input is interlaced, false otherwise
        bool m_interlace;
        
        //! True if interlaced and top field is first in temporal order 
        bool m_topfieldfirst;
        
        OneDArray<OLBParams> m_lbparams;
        OneDArray<OLBParams> m_cbparams;

        //! The original frame width
        int m_orig_xl;

        //! The original frame height
        int m_orig_yl;
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
        EncoderParams();
        
            ////////////////////////////////////////////////////////////////////
            //NB: Assume default copy constructor, assignment = and destructor//
            //This means pointers are copied, not the objects they point to.////       
            ////////////////////////////////////////////////////////////////////
        
         // Gets ...

        //! Get the quality factor
        float Qf() const {return m_qf;}

        //! Return the nominal number of L1 frames before the next I frame
        /*! 
            Return the nominal number of L1 frames before the next I frame. Can be
            overridden by I-frame insertion

        */
        int NumL1() const {return m_num_L1;}

        //! Return the separation between L1 frames (and between L1 and I frames)
        int L1Sep() const {return m_L1_sep;}

        //! Return the amount we're weighting noise in the U component
        float UFactor() const {return m_ufactor;}

        //! Return the amount we're weighting noise in the V component
        float VFactor() const {return m_vfactor;}

        //! Return the number of cycles per degree at the nominal viewing distance for the raster
        float CPD() const {return m_cpd;}

        //! Return the Lagrangian parameter to be used for I frames
        float ILambda() const {return m_I_lambda;}

        //! Return the Lagrangian parameter to be used for L1 frames
        float L1Lambda() const {return m_L1_lambda;}

        //! Return the Lagrangian parameter to be used for L2 frames
        float L2Lambda() const {return m_L2_lambda;}

        //! Return the Lagrangian parameter to be used for frames
        float Lambda(const FrameSort& fsort) const;

        //! Return the Lagrangian ME parameter to be used for L1 frames
        float L1MELambda() const {return m_L1_me_lambda;}

        //! Return the Lagrangian ME parameter to be used for L2 frames
        float L2MELambda() const {return m_L2_me_lambda;}

        //! Return the output path to be used for storing diagnositic data
        char * OutputPath() const {return ( char* ) m_output_path.c_str();}
        
        //! Return a reference to the entropy factors
        const EntropyCorrector& EntropyFactors() const {return *m_ent_correct;}
        
        //! Return a reference to the entropy factors - we need to be able to change the values of the entropy factors in situ
        EntropyCorrector& EntropyFactors() {return *m_ent_correct;}
        
        //!Return a reference to the bit output class
        const SequenceOutputManager& BitsOut() const {return *m_bit_out;}
        
        //!Return a reference to the bit output class - we need to output, so non-const
        SequenceOutputManager& BitsOut() {return *m_bit_out;}
        
        // ... and Sets

        //! Set the quality factor
        void SetQf(const float qfac){m_qf=qfac;}

        //! Set the nominal number of L1 frames between I frames
        void SetNumL1(const int nl){m_num_L1=nl;}

        //! Set the separation between L1 frames
        void SetL1Sep(const int lsep){m_L1_sep=lsep;}

        //! Set the amount to weight noise in the U component
        void SetUFactor(const float uf){m_ufactor=uf;}

        //! Set the amount to weight noise in the V component
        void SetVFactor(const float vf){m_vfactor=vf;}

        //! Set the number of cycles per degree at the nominal viewing distance
        void SetCPD(const float cpd){m_cpd=cpd;}

        //! Set the Lagrangian parameter to be used for I frames
        void SetILambda(const float l){m_I_lambda=l;}

        //! Set the Lagrangian parameter to be used for L1 frames
        void SetL1Lambda(const float l){m_L1_lambda=l;}

        //! Set the Lagrangian parameter to be used for L2 frames
        void SetL2Lambda(const float l){m_L2_lambda=l;}

        //! Set the Lagrangian parameters to be used for frames
        void SetLambda(const FrameSort& fsort, const float l);

        //! Set the Lagrangian parameter to be used for L1 motion estimation
        void SetL1MELambda(const float l){m_L1_me_lambda=l;}

        //! Set the Lagrangian parameter to be used for L2 motion estimation
        void SetL2MELambda(const float l){m_L2_me_lambda=l;}

        //! Set the output path to be used for diagnostic data
        void SetOutputPath(const char * op){ m_output_path = op; }
        
        //! Sets the entropy factors - TBD: set this up in a constructor and pass encoder params around entirely by reference
        void SetEntropyFactors(EntropyCorrector* entcorrect){m_ent_correct=entcorrect;}
        
        //! Sets the bit output - TBD: set this up in a constructor and pass encoder params around entirely by reference
        void SetBitsOut( SequenceOutputManager* so ){ m_bit_out=so; }
        
    private:
        //! Quality factor (between 0 and 10)
        float m_qf; 
        
        //! Number of L1 frames before next I frame
        int m_num_L1;
        
        //! Separation between L1 frames
        int m_L1_sep;    
        
        //! factor for weighting U component quantisation errors
        float m_ufactor;
        
        //! factor for weighting V component quantisation errors
        float m_vfactor;
        
        //! Cycles per degree assumed for viewing the video
        float m_cpd;
        
        //! Lagrangian parameter for Intra frame coding
        float m_I_lambda;

        //! Lagrangian parameter for L1 frame coding
        float m_L1_lambda;

        //! Lagrangian parameter for L2 frame coding
        float m_L2_lambda;
        
        //! Lagrangian param for L1 motion estimation
        float m_L1_me_lambda;

        //! Lagrangian param for L2 motion estimation
        float m_L2_me_lambda; 
        
        //! Correction factors for quantiser selection 
        EntropyCorrector* m_ent_correct;
        
        //! Pointer to object for managing bitstream output
        SequenceOutputManager* m_bit_out;   
        
        //! Output file path
        std::string m_output_path;
    };

    //! Parameters for the decoding process
    /*!
        Parameters for the decoding process. Derived from CodecParams.
     */
    class DecoderParams: public CodecParams
    {
    public:
            //! Default constructor
        DecoderParams():
        CodecParams(),
        m_bit_in(0){}
        
            ////////////////////////////////////////////////////////////////////
            //NB: Assume default copy constructor, assignment = and destructor//
            //This means pointers are copied, not the objects they point to.////       
            ////////////////////////////////////////////////////////////////////
        
        //! Return a reference to the bit output class
        const BitInputManager& BitsIn() const {return *m_bit_in;}
        
        //! Return a reference to the bit output class - we need to output, so non-const
        BitInputManager& BitsIn() {return *m_bit_in;}
        
        //! Sets the bit input - TBD: set this up in a constructor and pass decoder params around entirely by reference
        void SetBitsIn(BitInputManager* bi){m_bit_in=bi;}
        
    private:        
        //! Pointer to the bitstream input manager
        BitInputManager* m_bit_in;
    };

    //! A simple bounds checking function, very useful in a number of places
    inline ValueType BChk(const ValueType &num, const ValueType &max)
    {
        if(num < 0) return 0;
        else if(num >= max) return max-1;
        else return num;
    }

} // namespace dirac

#endif
