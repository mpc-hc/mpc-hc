/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: mvdata_byteio.h,v 1.5 2008/09/10 12:28:46 asuraparaju Exp $ $Name:  $
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

/**
* Definition of class MvDataByteIO
*/
#ifndef MV_DATA_BYTEIO_H
#define MV_DATA_BYTEIO_H


// DIRAC INCLUDES
#include <libdirac_common/common.h>              // EncParams

//LOCAL INCLUDES
#include <libdirac_byteio/byteio.h>              // Parent class
#include <libdirac_byteio/mvdataelement_byteio.h>// Member byteio class


namespace dirac
{
/**
* Represents compressed sequence-parameter data used in an AccessUnit
*/
class MvDataByteIO : public ByteIO
{
public:

    /**
    * Constructor
    *@param pparams         Picture Params
    *@param picpredparams   Picture prediction parameters
    */
    MvDataByteIO(PictureParams& pparams,
                 PicturePredParams& picpredparams);

    /**
    * Constructor
    *@param byte_io         Input/Output Byte stream
    *@param pparams         Picture Params
    *@param picpredparams   Picture prediction parameters
    */
    MvDataByteIO(ByteIO &byte_io, PictureParams& pparams,
                 PicturePredParams& picpredparams);

    /**
    * Destructor
    */
    virtual ~MvDataByteIO();

    /**
    * Gathers byte stats on the motion vector data
    *@param dirac_byte_stats Stat container
    */
    void CollateByteStats(DiracByteStats& dirac_byte_stats);

    /**
    * Outputs motion vector data Dirac byte-format
    */
    void Output();

    /**
    * Inputs motion vector information
    */
    void Input();


    /**
    * Get string containing coded bytes
    */
    virtual const std::string GetBytes();

    /**
    * Return pointer to the superblock splitting modes ByteIO stream
    */
    MvDataElementByteIO*  SplitModeData()
    {
        return &m_splitmode_data;
    };

    /**
    * Return pointer to the superblock splitting modes ByteIO stream
    */
    MvDataElementByteIO*  PredModeData()
    {
        return &m_predmode_data;
    };

    /**
    * Return pointer to the block MVs reference 1 ByteIO stream
    */
    MvDataElementByteIO*  MV1HorizData()
    {
        return &m_mv1hblock_data;
    };

    /**
    * Return pointer to the block MVs reference 1 ByteIO stream
    */
    MvDataElementByteIO*  MV1VertData()
    {
        return &m_mv1vblock_data;
    };

    /**
    * Return pointer to the block MV reference 2 ByteIO stream
    */
    MvDataElementByteIO*  MV2HorizData()
    {
        return &m_mv2hblock_data;
    };

    /**
    * Return pointer to the block MV reference 2 ByteIO stream
    */
    MvDataElementByteIO*  MV2VertData()
    {
        return &m_mv2vblock_data;
    };

    /**
    * Return pointer to the block Y DC values ByteIO stream
    */
    MvDataElementByteIO*  YDCData()
    {
        return &m_ydcblock_data;
    };

    /**
    * Return pointer to the block U DC values ByteIO stream
    */
    MvDataElementByteIO*  UDCData()
    {
        return &m_udcblock_data;
    };

    /**
    * Return pointer to the block V DC values ByteIO stream
    */
    MvDataElementByteIO*  VDCData()
    {
        return &m_vdcblock_data;
    };

    /**
    * Return the size
    */
    int GetSize() const;

protected:


private:
    /**
    * Inputs block parameters
    */
    void InputBlockParams();

    /**
    * Inputs Motion vector precision data
    */
    void InputMVPrecision();

    /**
    * Inputs global motion parameters
    */
    void InputGlobalMotionParams();

    /**
    * Inputs picture prediction mode
    */
    void InputFramePredictionMode();

    /**
    * Inputs Picture Weights
    */
    void InputPictureWeights();

    /**
    * Outputs block parameters
    */
    void OutputBlockParams();

    /**
    * Outputs Motion vector precision data
    */
    void OutputMVPrecision();

    /**
    * Outputs global motion parameters
    */
    void OutputGlobalMotionParams();

    /**
    * Outputs picture prediction mode
    */
    void OutputFramePredictionMode();

    /**
    * Outputs Picture Weights
    */
    void OutputPictureWeights();

    /**
    * Sequence paramters for intput/output
    */
    PictureParams&   m_pparams;

    /**
    * Codec params - EncParams for Output and DecParams for input
    */
    PicturePredParams& m_picpredparams;

    /**
    * block data containing split modes
    */
    MvDataElementByteIO m_splitmode_data;

    /**
    * block data containing prediction modes
    */
    MvDataElementByteIO m_predmode_data;

    /**
    * block data containing horizontal MV components for reference 1
    */
    MvDataElementByteIO m_mv1hblock_data;

    /**
    * block data containing vertical MV components for reference 1
    */
    MvDataElementByteIO m_mv1vblock_data;

    /**
    * block data containing horizontal MV components for reference 2
    */
    MvDataElementByteIO m_mv2hblock_data;

    /**
    * block data containing vertical MV components for reference 2
    */
    MvDataElementByteIO m_mv2vblock_data;

    /**
    * block data containing Y DC data
    */
    MvDataElementByteIO m_ydcblock_data;

    /**
    * block data containing U DC data
    */
    MvDataElementByteIO m_udcblock_data;

    /**
    * block data containing V DC data
    */
    MvDataElementByteIO m_vdcblock_data;
};

} // namespace dirac

#endif
