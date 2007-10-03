/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: accessunit_byteio.cpp,v 1.1 2006/04/20 10:41:56 asuraparaju Exp $ $Name: Dirac_0_7_0 $
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

#include <libdirac_byteio/accessunit_byteio.h>

using namespace dirac;

AccessUnitByteIO::AccessUnitByteIO(int& accessunit_fnum,
                                   SeqParams& seq_params,
                                   SourceParams& src_params):                  
ParseUnitByteIO(accessunit_fnum),
m_parseparams_byteio(accessunit_fnum,
                     *this),

// create default sequence parameters for comparisions
m_default_seq_params(seq_params.GetVideoFormat()),
// create default sequence parameters for comparisions
m_default_src_params(seq_params.GetVideoFormat()),

m_seqparams_byteio(seq_params,
                   m_default_seq_params,
                   *this),
m_displayparams_byteio(seq_params,
                       src_params,
                       m_default_src_params,
                       *this)
{
}

AccessUnitByteIO::AccessUnitByteIO(const ParseUnitByteIO& parseunit_byteio,
                                   SeqParams& seq_params,
                                   SourceParams& src_params,
                                   ParseParams& parse_params):
ParseUnitByteIO(parseunit_byteio),
m_parseparams_byteio(0, parseunit_byteio, parse_params),

// crate default sequence parameters for comparisions
m_default_seq_params(),

m_seqparams_byteio(seq_params,
                   m_default_seq_params,
                   parseunit_byteio),
m_displayparams_byteio(seq_params,
                       src_params,
                       m_default_src_params,
                       parseunit_byteio)
{
}

AccessUnitByteIO::~AccessUnitByteIO()
{
}

//-----public------------------------------------------------------
bool AccessUnitByteIO::Input() 
{
    //int o=mp_stream->tellg();
    InputParseParams();
    //int p=mp_stream->tellg();
    InputSequenceParams();
    InputDisplayParams();
    //int q=mp_stream->tellg();
    return true;
}

void AccessUnitByteIO::Output()
{
    OutputParseParams();
    OutputSequenceParams();
    OutputDisplayParams();
  
}

int AccessUnitByteIO::GetIdNumber() const
{
    return m_parseparams_byteio.GetIdNumber();
}

int AccessUnitByteIO::GetSize() const
{
    return ParseUnitByteIO::GetSize()+
           m_seqparams_byteio.GetSize()+
           m_parseparams_byteio.GetSize()+
           m_displayparams_byteio.GetSize();
}



//-------private-------------------------------------------------------

unsigned char AccessUnitByteIO::CalcParseCode() const
{
    unsigned char code = 0;

    // no further mods required

    return code;
}


void AccessUnitByteIO::InputDisplayParams()
{
     // copy current input params
    m_displayparams_byteio.SetByteParams(m_seqparams_byteio);

    m_displayparams_byteio.Input();
}

void AccessUnitByteIO::InputParseParams()
{
    m_parseparams_byteio.Input();
}

void AccessUnitByteIO::InputSequenceParams()
{
    // copy current input params
    m_seqparams_byteio.SetByteParams(m_parseparams_byteio);

    m_seqparams_byteio.Input();
}

void AccessUnitByteIO::OutputDisplayParams()
{
    // copy current output params
    m_displayparams_byteio.SetByteParams(m_seqparams_byteio);

    m_displayparams_byteio.Output();
}

void AccessUnitByteIO::OutputParseParams()
{
    m_parseparams_byteio.Output();
}

void AccessUnitByteIO::OutputSequenceParams()
{
    // copy current output params
    m_seqparams_byteio.SetByteParams(m_parseparams_byteio);

    m_seqparams_byteio.Output();
}

