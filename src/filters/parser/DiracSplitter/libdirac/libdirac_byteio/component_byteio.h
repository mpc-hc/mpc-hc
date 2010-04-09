/* ***** BEGIN LICENSE BLOCK *****
*
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
* Definition of class ComponentByteIO
*/
#ifndef component_byteio_h
#define component_byteio_h


//LOCAL INCLUDES
#include <libdirac_byteio/byteio.h>                 // Parent class
#include <libdirac_byteio/subband_byteio.h>          // child-type

// DIRAC includes

// SYSTEM INCLUDES
#include <vector>

namespace dirac
{
    /**
    * Picture component in Dirac bytestream format
    */
    class ComponentByteIO : public ByteIO
    {
    public:

        /**
        * Constructor
        *@param cs Picture-component type
        *@param byteIO Input/output Byte stream
        */
        ComponentByteIO(CompSort cs,
                        const ByteIO& byteIO);

        /**
        * Constructor
        *@param cs Picture-component type
        */
        ComponentByteIO(CompSort cs);

       /**
       * Destructor
       */
        ~ComponentByteIO();

        /**
        * Gathers byte stats on the component data
        *@param dirac_byte_stats Stat container
        */
        void CollateByteStats(DiracByteStats& dirac_byte_stats);

        /**
        * Add a subband byte-stream to this component
        *@param p_subband_byteio Subband to be added
        */
        void AddSubband(SubbandByteIO *p_subband_byteio);

        /**
        * Inputs data from Dirac stream-format
        */
        bool Input();

        /**
        * Outputs picture values to Dirac stream-format
        */
        void Output();

       

  protected:
        

   private:
      
       /**
       * Picture component type
       */
       CompSort m_compsort;

       /**
       * List of subbands in output/input order
       */
       std::vector<ByteIO*> m_subband_list;
       
       
   };

} // namespace dirac

#endif
