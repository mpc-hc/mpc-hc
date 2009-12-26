/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_byte_stats.h,v 1.5 2008/05/06 09:35:51 asuraparaju Exp $ $Name:  $
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
* Definition of class DiracByteStats
*/
#ifndef dirac_byte_stats_h
#define dirac_byte_stats_h

// SYSTEM INCLUDES
#include <map>                          // for byte-counts
#include <climits>

#include <libdirac_common/dirac_inttypes.h>

namespace dirac
{

    #ifdef _MSC_VER
    // char length
    #define CHAR_BIT 8
#endif

    typedef enum {
        STAT_TOTAL_BYTE_COUNT=0,
        STAT_MV_BYTE_COUNT,
        STAT_YCOMP_BYTE_COUNT,
        STAT_UCOMP_BYTE_COUNT,
        STAT_VCOMP_BYTE_COUNT
    } StatType;


   /**
   * Class DiracByteStats - for collecting statistics on aspects of the Dirac byte-stream
   */
   class DiracByteStats 
   {
   public:
        /**
        * Constructor
        */
        DiracByteStats();

        /**
        * Copy constructor
        */
        DiracByteStats(const DiracByteStats& dirac_byte_stats);

        /**
        * Destructor
        */
        ~DiracByteStats();

        /**
        * Clears data
        */
        void Clear();

        /**
        * Gets number of bits for a particular stat-type
        */
        int64_t GetBitCount(const StatType& stat_type) const;

        /**
        * Gets number of bytes for a particular stat-type
        */
        int64_t GetByteCount(const StatType& stat_type) const;
      
        /**
        * Sets number of bytes for a particular stat-type
        */
        void SetByteCount(const StatType& stat_type, int64_t count);
       

    private:

        /**
        * Map of byte-counts
        */
        std::map<StatType, int64_t> m_byte_count;

   };

} // namespace dirac

#endif
