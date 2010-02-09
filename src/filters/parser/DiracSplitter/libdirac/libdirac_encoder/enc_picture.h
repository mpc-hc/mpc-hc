/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: enc_picture.h,v 1.6 2008/10/01 01:26:47 asuraparaju Exp $ $Name:  $
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
* Portions created by the Initial Developer are Copyright (C) 2008.
* All Rights Reserved.
*
* Contributor(s): Thomas Davies (Original Author),
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

#ifndef _ENC_PICTURE_H_
#define _ENC_PICTURE_H_

#include <libdirac_common/picture.h>
#include <libdirac_common/motion.h>


namespace dirac
{
static const unsigned int DONE_ME_INIT = 0x1;
static const unsigned int DONE_PEL_ME = 0x2;
static const unsigned int DONE_SUBPEL_ME = 0x4;
static const unsigned int DONE_ME_MODE_DECN = 0x8;
static const unsigned int DONE_MV_CODING = 0x10;
static const unsigned int DONE_MC = 0x20;
static const unsigned int DONE_DWT = 0x40;
static const unsigned int DONE_QUANT_SEL = 0x80;
static const unsigned int DONE_RES_CODING = 0x100;
static const unsigned int DONE_IDWT = 0x200;
static const unsigned int DONE_MC_BACK = 0x400;
static const unsigned int DONE_SET_PTYPE = 0x800;
static const unsigned int DONE_PIC_COMPLEXITY = 0x1000;

static const unsigned int ALL_ENC = 0xFFFFFFFF;
static const unsigned int NO_ENC = 0;

class EncPicture : public Picture
{
public:
    EncPicture( const PictureParams& pp );

    virtual ~EncPicture();

    //! Initialise the motion estimation data arrays
    void InitMEData( const PicturePredParams& predparams, const int num_refs);

    //! Returns the motion data
    MEData& GetMEData(){ return *m_me_data;}

    //! Returns the motion data
    const MEData& GetMEData() const { return *m_me_data;}

    //! Drops a reference from the motion vector data
    void DropRef( int rindex );


    //! Returns a given component of the original data
    const PicArray& OrigData(CompSort c) const { return *m_orig_data[(int) c];}

    //! Returns a given upconverted component of the original data
    const PicArray& UpOrigData(CompSort cs) const;

    //! Initialises a copy of the data arrays into the original data
    void SetOrigData();

    //! Returns a version of the picture data suitable for motion estimation
    const PicArray& DataForME(bool combined_me) const;

    //! Returns a version of the picture data suitable for subpel motion estimation
    const PicArray& UpDataForME(bool combined_me) const;


    void UpdateStatus( const unsigned int mask ){ m_status |= mask; }

    void FlipStatus( const unsigned int mask){ m_status ^= mask; }

    void SetStatus( const int status ){ m_status = status; }

    unsigned int GetStatus() const{ return m_status; }


    double GetComplexity() const {return m_complexity; }

    void SetComplexity(double c){ m_complexity = c; }

    double GetNormComplexity() const { return m_norm_complexity; }

    void SetNormComplexity( double c ){ m_norm_complexity = c; }

    double GetPredBias() const { return m_pred_bias; }

    void SetPredBias( double b ){ m_pred_bias = b; }


private:

    virtual void ClearData();

    //! Filters a (field) picture vertically to reduce aliasing for motion estimation purposes
    void AntiAliasFilter( PicArray& out_data, const PicArray& in_data ) const;

    //! Returns an anti-aliased version of the original data
    const PicArray& FiltData(CompSort c) const;

    const PicArray& CombinedData() const;
    const PicArray& UpCombinedData() const;
    void Combine( PicArray& comb_data, const PicArray& y_data,
                          const PicArray& u_data, const PicArray& v_data ) const;

    //! Returns an upconverted anti-aliased version of the original data
    const PicArray& UpFiltData(CompSort c) const;


    void SetOrigData(const int c);

private:

    PicArray* m_orig_data[3];
    mutable PicArray* m_orig_up_data[3];
    mutable PicArray* m_filt_data[3];
    mutable PicArray* m_filt_up_data[3];

    MEData* m_me_data;

    unsigned int m_status;

    double m_complexity;
    double m_norm_complexity;

    double m_pred_bias;
};


}

#endif
