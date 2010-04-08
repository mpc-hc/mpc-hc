/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_byte_stats.cpp,v 1.3 2008/04/29 12:27:49 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Andrew Kennedy
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

#include <libdirac_byteio/dirac_byte_stats.h>

using namespace dirac;
using namespace std;


DiracByteStats::DiracByteStats()
{
}

DiracByteStats::DiracByteStats(const DiracByteStats& dirac_byte_stats):
    m_byte_count(dirac_byte_stats.m_byte_count)
{
}

DiracByteStats::~DiracByteStats()
{
}


void DiracByteStats::Clear()
{
    m_byte_count.clear();
}

int64_t DiracByteStats::GetBitCount(const StatType& stat_type) const
{
    return GetByteCount(stat_type) * CHAR_BIT;
}

int64_t DiracByteStats::GetByteCount(const StatType& stat_type) const
{
    std::map<StatType, int64_t>::const_iterator it;
    it = m_byte_count.find(stat_type);
    if(it == m_byte_count.end())
        return 0;

    return it->second;
}

void DiracByteStats::SetByteCount(const StatType& stat_type, int64_t count)
{
    if(m_byte_count.find(stat_type) == m_byte_count.end())
        m_byte_count[stat_type] = 0;

    m_byte_count[stat_type] += count;
}
