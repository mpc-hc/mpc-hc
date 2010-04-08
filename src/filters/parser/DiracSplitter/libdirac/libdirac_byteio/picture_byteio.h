/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: picture_byteio.h,v 1.1 2008/01/31 11:25:15 tjdwave Exp $ $Name:  $
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
* Contributor(s): Andrew Kennedy (Original Author)
*                 Anuradha Suraparaju
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
* Definition of class PictureByteIO
*/
#ifndef picture_byteio_h
#define picture_byteio_h


//LOCAL INCLUDES
#include <libdirac_byteio/parseunit_byteio.h>       // Parent class
#include <libdirac_byteio/mvdata_byteio.h>          // Contains mvdata streams
#include <libdirac_byteio/transform_byteio.h>       // Transform header

// DIRAC includes
#include <libdirac_common/common.h>                 // PictureType etc


namespace dirac
{
/**
* A compressed picture in Dirac bytestream format
*/
class PictureByteIO : public ParseUnitByteIO
{
public:

    /**
    * Constructor
    *@param frame_params Picture parameters
    *@param frame_num Picture number
    */
    PictureByteIO(PictureParams& frame_params,
                  int frame_num);


    /**
    * Constructor
    *@param frame_params Destination of data
    *@param parseunit_byteio Source of data
    */
    PictureByteIO(PictureParams& frame_params,
                  const ParseUnitByteIO& parseunit_byteio);

    /**
    * Destructor
    */
    virtual ~PictureByteIO();

    /**
            * Gathers byte stats on the picture data
            *@param dirac_byte_stats Stat container
            */
    void CollateByteStats(DiracByteStats& dirac_byte_stats);

    /**
    * Inputs data from Dirac stream-format
    */
    bool Input();

    /**
    * Outputs picture values to Dirac stream-format
    */
    void Output();



    const std::string GetBytes();

    int GetSize() const;

    /**
    * Gets parse-unit type
    */
    ParseUnitType GetType() const
    {
        return PU_PICTURE;
    }

    /**
    * Returns true is picture in Reference picture
    */
    int IsRef() const
    {
        return (GetParseCode() & 0x0C) == 0x0C;
    }

    /**
    * Returns true is picture in Non-Reference picture
    */
    int IsNonRef() const
    {
        return (GetParseCode() & 0x0C) == 0x08;
    }

    /**
    * Gets parse-unit type
    */
    int NumRefs() const
    {
        return (GetParseCode() & 0x03);
    }

    /**
    * Returns true is picture is Intra picture
    */
    bool IsIntra() const
    {
        return IsPicture() && (NumRefs() == 0) ;
    }

    /**
    * Returns true is picture is Inter picture
    */
    bool IsInter() const
    {
        return IsPicture() && (NumRefs() > 0) ;
    }

    /***
    * Sets the MVDataIO
    */
    void SetMvData(MvDataByteIO *mv_data)
    {
        m_mv_data = mv_data;
    }

    /***
    * Sets the TransformByteIo
    */
    void SetTransformData(TransformByteIO *transform_data)
    {
        m_transform_data = transform_data;
    }

protected:


private:

    /**
    * Calculates parse-code based on picture parameters
    *@return Char bit-set
    */
    unsigned char CalcParseCode() const;

    /**
    * Reads reference-picture data
    */
    void InputReferencePictures();

    /**
    * Reads retired picture number
    */
    void InputRetiredPicture();

    /**
    * Calculates picture-type (eg INTRA/INTER) of picture
    */
    void SetPictureType();

    /**
    * Calculates reference-type of picture
    */
    void SetReferenceType();

    /**
    * Sets the entropy coding flag in the picture parameters
    */
    void SetEntropyCodingFlag();

    /**
    * Picture parameters
    */
    PictureParams&        m_frame_params;

    /**
    * Picture number
    */
    int     m_frame_num;

    /**
    * MV data
    */
    MvDataByteIO * m_mv_data;

    /**
    * Transform data
    */
    TransformByteIO * m_transform_data;

};

} // namespace dirac

#endif
