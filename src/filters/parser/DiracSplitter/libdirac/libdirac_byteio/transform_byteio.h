/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: transform_byteio.h,v 1.2 2008/01/31 11:25:15 tjdwave Exp $ $Name:  $
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
* Definition of class TransformByteIO
*/
#ifndef TRANSFORM_BYTEIO_H
#define TRANSFORM_BYTEIO_H

// DIRAC INCLUDES
#include <libdirac_common/common.h>             // EncParams

//LOCAL INCLUDES
#include <libdirac_byteio/byteio.h>             // Parent class
#include <libdirac_byteio/component_byteio.h>   // Contains tranform-component


namespace dirac
{
             
    /**
    * Represents compressed sequence-parameter data used in an AccessUnit
    */
    class TransformByteIO : public ByteIO
    {
    public:

        /**
        * Output Constructor
        *@param fparams   Picture parameters
        *@param c_params  Codec params
        */
        TransformByteIO(PictureParams& fparams,
                        CodecParams& c_params);

        /**
        * Input Constructor
        *@param byte_io   ByteIO object for copy constructor
        *@param fparams    Picture parameters
        *@param c_params   Codec params
        */
        TransformByteIO(ByteIO &byte_io, PictureParams& fparams,
                        CodecParams& c_params);

        /**
        * Destructor
        */
        virtual ~TransformByteIO();

        /**
        * Gathers byte stats on the transform data
        *@param dirac_byte_stats Stats container
        */
        void CollateByteStats(DiracByteStats& dirac_byte_stats);

        /**
        * Outputs sequence information to Dirac byte-format
        */
        void Output();

        /**
        * Outputs sequence information to Dirac byte-format
        */
        void Input();


        /**
        * Get string containing coded bytes
        */
        virtual const std::string GetBytes();

        /**
        * Return the size 
        */
        int GetSize() const;
        
        /**
        * Adds a Picture-component in Dirac-bytestream format
        *@param component_byteio Picture-component bytestream
        */
        void AddComponent(ComponentByteIO *component_byteio);

    protected:
    

    private:
    
        /**
        * Sequence paramters for intput/output
        */
        PictureParams&   m_fparams;

        /**
        * Codec params - EncParams for Output and DecParams for input
        */
        CodecParams& m_cparams;
        
        /**
        * Default Codec params - EncParams for Output and DecParams for input
        */
        CodecParams m_default_cparams;

        /***
        * Transform Component data
        */
        std::vector<ComponentByteIO *> m_component_list;
    };

} // namespace dirac

#endif
