/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: displayparams_byteio.h,v 1.4 2008/01/15 04:36:23 asuraparaju Exp $ $Name:  $
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
* Definition of class SourceParamsByteIO
*/
#ifndef displayparams_byteio_h
#define displayparams_byteio_h


// DIRAC INCLUDES
#include <libdirac_common/common.h>             // SeqParams

//LOCAL INCLUDES
#include <libdirac_byteio/byteio.h>             // Parent class


namespace dirac
{
             
    /**
    * Represents compressed source-parameter data contained in a sequence header
    */
    class SourceParamsByteIO : public ByteIO
    {
    public:

        /**
        * Constructor for Input/Output
        *@param src_params Source parameters
        *@param default_src_params Default Source parameters
        *@param stream_data Source/Destination of data
        */
        SourceParamsByteIO( SourceParams& src_params,
                             const SourceParams& default_src_params,
                             const ByteIO& stream_data);
        /**
        * Destructor
        */
        ~SourceParamsByteIO();

        /**
        * Reads source params information from Dirac byte-format
        */
        void Input();

        /**
        * Outputs source params information to Dirac byte-format
        */
        void Output();

    protected:
    

    private:

        /**
        * Reads frame dimensions
        */
        void InputFrameSize();

        /**
        * Reads Chroma Sampling Format
        */
        void InputChromaSamplingFormat();
    
        /**
        * Reads pixel aspect ratio info
        */
        void InputPixelAspectRatio();

        /**
        * Reads clean-area info
        */
        void InputCleanArea();

        /**
        * Reads colour-matrix info
        */
        void InputColourMatrix();

        /**
        * Reads primary-colour info
        */
        void InputColourPrimaries();

        /**
        * Reads colour spec info
        */
        void InputColourSpecification();

        /**
        * Reads frame-rate info
        */
        void InputFrameRate();

        /**
        * Reads Scan info
        */
        void InputScanFormat();

        /**
        * Reads signal range info
        */
        void InputSignalRange();

        /**
        * Reads transfer-function info
        */
        void InputTransferFunction();
 
        /**
        * Outputs frame dimensions
        */
        void OutputFrameSize();

        /**
        * Outputs Chroma Sampling Format
        */
        void OutputChromaSamplingFormat();

        /**
        * Outputs pixel aspect ratio info
        */
        void OutputPixelAspectRatio();

        /**
        * Outputs clean-area info
        */
        void OutputCleanArea();

        /**
        * Outputs colour spec info
        */
        void OutputColourSpecification();

        /**
        * Outputs frame-rate info
        */
        void OutputFrameRate();

         /**
        * Outputs Scan info
        */
        void OutputScanFormat();

        /**
        * Outputs signal range info
        */
        void OutputSignalRange();

        /**
        * Source parameters for input/ouput
        */
        SourceParams&      m_src_params;

        /**
        * Default source parameters
        */
        const SourceParams& m_default_src_params;
    };


} // namespace dirac

#endif
