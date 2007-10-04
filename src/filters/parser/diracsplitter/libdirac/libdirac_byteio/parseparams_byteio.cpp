/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: parseparams_byteio.cpp,v 1.4 2007/09/28 15:46:08 asuraparaju Exp $ $Name: Dirac_0_8_0 $
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

#include <sstream> //For std::ostringstream
#include <libdirac_byteio/parseparams_byteio.h>
#include <libdirac_common/dirac_exception.h>

const unsigned int PP_AU_FRAME_NUM_SIZE = 4;

using namespace dirac;

ParseParamsByteIO::ParseParamsByteIO( const ByteIO& stream_data):
ByteIO(stream_data)
{


}

ParseParamsByteIO::ParseParamsByteIO( const ByteIO& stream_data,
                                     ParseParams &parse_params):
ByteIO(stream_data),
m_parse_params(parse_params)
{


}

ParseParamsByteIO::~ParseParamsByteIO()
{
}

//-------------public---------------------------------------------------------------

void ParseParamsByteIO::Input()
{
    ParseParams def_parse_params;

    //input version
    m_parse_params.SetMajorVersion(InputVarLengthUint());
    m_parse_params.SetMinorVersion(InputVarLengthUint());

    // input profile
    m_parse_params.SetProfile(InputVarLengthUint());

    // input level
    m_parse_params.SetLevel(InputVarLengthUint());

    std::ostringstream errstr;
    // FIXME: for the time being all should be a perfect match until
    // we add support for previous versions
    if (m_parse_params.MajorVersion() != def_parse_params.MajorVersion() ||
        m_parse_params.MinorVersion() != def_parse_params.MinorVersion())
    {
        errstr << "Cannot handle version ";
        errstr << m_parse_params.MajorVersion() << ".";
        errstr << m_parse_params.MinorVersion() << ".";
        errstr << " Supported version is ";
        errstr << def_parse_params.MajorVersion() << ".";
        errstr << def_parse_params.MinorVersion() << std::endl;
    }

    if (m_parse_params.Profile() > def_parse_params.Profile())
    {
        errstr << "Cannot handle profile " << m_parse_params.Profile();
        errstr << ". Supported profile is " << def_parse_params.Profile();
    }

    if (m_parse_params.Level() > def_parse_params.Level())
    {
        errstr << "Cannot handle level " << m_parse_params.Level();
        errstr << ". Supported level is " << def_parse_params.Level();
    }

    if (errstr.str().size())
    {
        DIRAC_THROW_EXCEPTION(
            ERR_UNSUPPORTED_STREAM_DATA,
            errstr.str(),
            SEVERITY_FRAME_ERROR);
    }
}

void ParseParamsByteIO::Output()
{
    ParseParams def_parse_params;
    // output version
    OutputVarLengthUint(def_parse_params.MajorVersion());
    OutputVarLengthUint(def_parse_params.MinorVersion());

    // output profile
    OutputVarLengthUint(def_parse_params.Profile());

    // output level
    OutputVarLengthUint(def_parse_params.Level());
}
