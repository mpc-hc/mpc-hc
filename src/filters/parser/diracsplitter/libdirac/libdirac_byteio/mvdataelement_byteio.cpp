/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: mvdataelement_byteio.cpp,v 1.2 2007/11/16 04:48:44 asuraparaju Exp $ $Name:  $
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
*                 Thomas Davies
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

#include <libdirac_byteio/mvdataelement_byteio.h>
#include <libdirac_common/video_format_defaults.h>
#include <libdirac_common/dirac_exception.h>

using namespace dirac;

MvDataElementByteIO::MvDataElementByteIO():
ByteIO(),
m_block_data()
{
}

MvDataElementByteIO::MvDataElementByteIO(ByteIO &byte_io):
ByteIO(byte_io),
m_block_data(byte_io)
{
}

MvDataElementByteIO::~MvDataElementByteIO()
{
}

/*
void MvDataElementByteIO::CollateByteStats(DiracByteStats& dirac_byte_stats)
{
    // set number of MV bytes
    dirac_byte_stats.SetByteCount(STAT_MV_BYTE_COUNT,
                                  GetSize());
}
*/
int MvDataElementByteIO::GetSize() const
{
    return ByteIO::GetSize() + m_block_data.GetSize();
}

const std::string MvDataElementByteIO::GetBytes()
{
    //Output header and block data
    return ByteIO::GetBytes() + m_block_data.GetBytes();
}

void MvDataElementByteIO::Input()
{
    // Input block data size
    m_block_size = ReadUint();

    // Byte Alignment
    ByteAlignInput();
}

void MvDataElementByteIO::Output()
{    
    //Output size of block data
    WriteUint(m_block_data.GetSize());

    // Byte Align
    ByteAlignOutput();
}
