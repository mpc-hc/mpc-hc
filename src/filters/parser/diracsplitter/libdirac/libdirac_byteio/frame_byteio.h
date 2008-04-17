/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: frame_byteio.h,v 1.6 2007/11/16 04:50:47 asuraparaju Exp $ $Name: Dirac_0_9_1 $
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
* Definition of class FrameByteIO
*/
#ifndef frame_byteio_h
#define frame_byteio_h


//LOCAL INCLUDES
#include <libdirac_byteio/parseunit_byteio.h>       // Parent class
#include <libdirac_byteio/mvdata_byteio.h>          // Contains mvdata streams
#include <libdirac_byteio/transform_byteio.h>       // Transform header

// DIRAC includes
#include <libdirac_common/common.h>                 // FrameType etc


namespace dirac
{
    /**
    * A compressed frame in Dirac bytestream format
    */
    class FrameByteIO : public ParseUnitByteIO
    {
    public:

        /**
        * Constructor
        *@param frame_params Frame parameters
        *@param frame_num Frame number
        */
        FrameByteIO(FrameParams& frame_params,
                    int frame_num);


        /**
        * Constructor
        *@param frame_params Destination of data
        *@param parseunit_byteio Source of data
        */
        FrameByteIO(FrameParams& frame_params,
                    const ParseUnitByteIO& parseunit_byteio);

       /**
       * Destructor
       */
        virtual ~FrameByteIO();

         /**
        * Gathers byte stats on the frame data
        *@param dirac_byte_stats Stat container
        */
        void CollateByteStats(DiracByteStats& dirac_byte_stats);

        /**
        * Inputs data from Dirac stream-format
        */
        bool Input();

        /**
        * Outputs frame values to Dirac stream-format
        */
        void Output();

        

        const std::string GetBytes();

        int GetSize() const;

        /**
        * Gets parse-unit type
        */
        ParseUnitType GetType() const { return PU_FRAME;}

        /**
        * Returns true is frame in Reference frame
        */
        int IsRef() const { return (GetParseCode()&0x0C)==0x0C;}

        /**
        * Returns true is frame in Non-Reference frame
        */
        int IsNonRef() const { return (GetParseCode()&0x0C)==0x08;}

        /**
        * Gets parse-unit type
        */
        int NumRefs() const { return (GetParseCode()&0x03);}

        /**
        * Returns true is frame is Intra frame
        */
        bool IsIntra() const { return IsPicture() && (NumRefs()==0) ; }

        /**
        * Returns true is frame is Inter frame
        */
        bool IsInter() const { return IsPicture() && (NumRefs()>0) ; }

        /***
        * Sets the MVDataIO
        */
        void SetMvData(MvDataByteIO *mv_data) {m_mv_data = mv_data; }

        /***
        * Sets the TransformByteIo
        */
        void SetTransformData(TransformByteIO *transform_data) {m_transform_data = transform_data; }

  protected:
        

   private:
      
        /**
        * Calculates parse-code based on frame parameters
        *@return Char bit-set 
        */
        unsigned char CalcParseCode() const;

        /**
        * Reads reference-frame data
        */
        void InputReferencePictures();

        /**
        * Reads retired picture number
        */
        void InputRetiredPicture();

        /**
        * Calculates frame-type (eg INTRA/INTER) of frame
        */
        void SetFrameType();

        /**
        * Calculates reference-type of frame
        */
        void SetReferenceType();

        /**
        * Sets the entropy coding flag in the frame parameters
        */
        void SetEntropyCodingFlag();

        /**
        * Frame parameters
        */
        FrameParams&        m_frame_params;

        /**
        * Frame number
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
