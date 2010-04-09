/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: endofsequence_byteio.cpp,v 1.2 2007/03/29 16:43:38 asuraparaju Exp $ $Name:  $
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

#include "endofsequence_byteio.h"

using namespace dirac;
using namespace std;


// start & end of end-of-sequence bits
#define PARSE_CODE_END_OF_SEQUENCE      (0x10)


EndOfSequenceByteIO::EndOfSequenceByteIO(const ByteIO& byte_io):
ParseUnitByteIO(byte_io)
{
   
}


EndOfSequenceByteIO::~EndOfSequenceByteIO()
{

}

void EndOfSequenceByteIO::CollateByteStats(DiracByteStats& dirac_byte_stats)
{
    dirac_byte_stats.SetByteCount(STAT_TOTAL_BYTE_COUNT, GetSize());
}



//-------------private-------------------------------------------------------
 
unsigned char EndOfSequenceByteIO::CalcParseCode() const
{
    unsigned char code = 0;

   // set end-of-sequence parse-code
   SetBits(code, PARSE_CODE_END_OF_SEQUENCE);

   return code;
}

