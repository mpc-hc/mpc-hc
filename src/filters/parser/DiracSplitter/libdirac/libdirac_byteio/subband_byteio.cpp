/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: subband_byteio.cpp,v 1.4 2008/04/29 08:51:52 tjdwave Exp $ $Name:  $
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

#include <libdirac_byteio/subband_byteio.h>

using namespace dirac;
using namespace std;

SubbandByteIO::SubbandByteIO(Subband& sub_band,
                             const ByteIO& byteio):
    ByteIO(byteio),
    m_subband(sub_band),
    m_band_data_length(0)
{

}

SubbandByteIO::SubbandByteIO(Subband& sub_band):
    ByteIO(),
    m_subband(sub_band),
    m_band_data_length(0)
{

}

SubbandByteIO::~SubbandByteIO()
{

}

//--------------public----------------------------------------------

bool SubbandByteIO::Input()
{
    // read length of Arith-coded data block
    m_band_data_length = ReadUint();

    // set skip flag if no data
    m_subband.SetSkip(m_band_data_length == 0 ? true : false);

    // check for zero-length sub-band
    if(m_subband.Skipped())
    {
        ByteAlignInput();
        return true;
    }

    // If we're not skipped, we need a quantisation index for the subband
    m_subband.SetQuantIndex(ReadUint());

    if(!m_subband.UsingMultiQuants())
    {
        // Propogate the quantiser index to all the code blocks if we
        // don't have multiquants
        for(int j = 0 ; j < m_subband.GetCodeBlocks().LengthY() ; ++j)
            for(int i = 0 ; i < m_subband.GetCodeBlocks().LengthX() ; ++i)
                m_subband.GetCodeBlocks()[j][i].SetQuantIndex(m_subband.QuantIndex());
    }

    // byte align
    ByteAlignInput();
    //int f=mp_stream->tellg();

    return true;
}

int SubbandByteIO::GetBandDataLength() const
{
    return m_band_data_length;
}

const string SubbandByteIO::GetBytes()
{
    ByteIO byte_io;

    ByteAlignOutput();

    // output size
    byte_io.WriteUint(GetSize());

    // check for zero-length sub-band
    if(GetSize() == 0)
    {
        byte_io.ByteAlignOutput();
        return byte_io.GetBytes();
    }

    // output quantisation
    byte_io.WriteUint(m_subband.QuantIndex());

    // byte align
    byte_io.ByteAlignOutput();

    //std::cerr << "Subband hdr size=" << byte_io.GetSize();
    //std::cerr << " Arithdata size=" << this->GetSize()<< std::endl;

    return byte_io.GetBytes() + ByteIO::GetBytes();
}



//-------------private-------------------------------------------------------

