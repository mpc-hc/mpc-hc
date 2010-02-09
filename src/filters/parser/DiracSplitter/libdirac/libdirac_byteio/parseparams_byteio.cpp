/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: parseparams_byteio.cpp,v 1.9 2008/10/21 00:04:08 asuraparaju Exp $ $Name:  $
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

const unsigned int PP_AU_PICTURE_NUM_SIZE = 4;

using namespace dirac;

ParseParamsByteIO::ParseParamsByteIO( const ByteIO& stream_data,
                                      ParseParams &parse_params,
                                      EncoderParams &enc_params):
ByteIO(stream_data),
m_parse_params(parse_params)
{
    if (enc_params.NumL1() == 0)
    {
        if (!enc_params.UsingAC())
        {
            // Simple Profile
            m_parse_params.SetProfile(1);
        }
        else
        {
            // Main (Intra) profile
            m_parse_params.SetProfile(2);
        }
    }
    else
    {
        // Main (Long GOP) profile
           m_parse_params.SetProfile(8);
    }
    // FIXME - no support for Low Delay Profile
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

void ParseParamsByteIO::CheckVersion()
{
    std::ostringstream errstr;
    ParseParams def_parse_params;

    if (m_parse_params.MajorVersion() > def_parse_params.MajorVersion() ||
        m_parse_params.MajorVersion() == 0                              ||
       (m_parse_params.MajorVersion() == def_parse_params.MajorVersion() &&
        m_parse_params.MinorVersion() > def_parse_params.MinorVersion()))
    {
        errstr << "WARNING: Bitstream version is ";
        errstr << m_parse_params.MajorVersion() << ".";
        errstr << m_parse_params.MinorVersion() << ".";
        errstr << " Supported version is ";
        errstr << def_parse_params.MajorVersion() << ".";
        errstr << def_parse_params.MinorVersion();
        errstr << ". May not be able to decode bitstream correctly" << std::endl;
    }

    if (errstr.str().size())
    {
        DiracException err(
            ERR_UNSUPPORTED_STREAM_DATA,
            errstr.str(),
            SEVERITY_PICTURE_ERROR);
        DIRAC_LOG_EXCEPTION(err);
    }
}

void ParseParamsByteIO::CheckProfile()
{
    std::ostringstream errstr;
    ParseParams def_parse_params;

    // No profiles were specified in versions 1.0, 1.1, and 2.0 and 2.1.
    // So for these versions profile should be 0 in the bitstream
    if (m_parse_params.MajorVersion() <= 2 &&
        m_parse_params.MinorVersion() < 2  &&
        m_parse_params.Profile() != 0)
    {
        errstr << "Cannot handle profile "  << m_parse_params.Profile()
               << " for bitstream version " << m_parse_params.MajorVersion()
               << "." << m_parse_params.MinorVersion();
        errstr << ". May not be able to decode bitstream correctly" << std::endl;
    }
    else if (m_parse_params.MajorVersion() == def_parse_params.MajorVersion() &&
             m_parse_params.MinorVersion() == def_parse_params.MinorVersion() &&
             m_parse_params.Profile() != 1 /* Simple */          &&
             m_parse_params.Profile() != 2 /* Main (Intra) */    &&
             m_parse_params.Profile() != 8 /* Main (Long GOP) */
            )
    {
        errstr << "Cannot handle profile " << m_parse_params.Profile()
               << " for bitstream version " << m_parse_params.MajorVersion()
               << ". " << m_parse_params.MinorVersion()
               << ". Supported profiles are 1 (Simple) "
               << " 2 (Main Intra) and 8 (Long GOP)";
        errstr << ". May not be able to decode bitstream correctly" << std::endl;
    }

    if (errstr.str().size())
    {
        DiracException err(
            ERR_UNSUPPORTED_STREAM_DATA,
            errstr.str(),
            SEVERITY_PICTURE_ERROR);
        DIRAC_LOG_EXCEPTION(err);
    }
}

void ParseParamsByteIO::CheckLevel()
{
    std::ostringstream errstr;
    ParseParams def_parse_params;

    // No resources constraints for decoder
    if (def_parse_params.Level() == 0)
        return;

    // Constraints on Decoder. Can Handles level 1 for Simple and Main (Intra)
    // profiles, and level 128 for Main (Long GOP) Profile.
    if (def_parse_params.Level() != 0)
    {
        if ((m_parse_params.Profile() <= 2 && m_parse_params.Level() != 1) ||
            (m_parse_params.Profile() ==8 && m_parse_params.Level() != 128))
        {
            errstr << "Cannot handle Level " << m_parse_params.Level()
                   << " for bitstream version " << m_parse_params.MajorVersion()
                   << ". " << m_parse_params.MinorVersion()
                   << " Profile " << m_parse_params.Profile()
                   << ". Supported levels are 1 for Profiles 0, 1, 2 "
                   << "  and 128 for Profile 8";
            errstr << ". May not be able to decode bitstream correctly" << std::endl;
        }
    }

    if (errstr.str().size())
    {
        DiracException err(
            ERR_UNSUPPORTED_STREAM_DATA,
            errstr.str(),
            SEVERITY_PICTURE_ERROR);
        DIRAC_LOG_EXCEPTION(err);
    }
    return;
}

//-------------public---------------------------------------------------------------

void ParseParamsByteIO::Input()
{

    //input version
    m_parse_params.SetMajorVersion(ReadUint());
    m_parse_params.SetMinorVersion(ReadUint());

    // input profile
    m_parse_params.SetProfile(ReadUint());

    // input level
    m_parse_params.SetLevel(ReadUint());

    CheckVersion();

    CheckProfile();

    CheckLevel();
}

void ParseParamsByteIO::Output()
{
    // output version
    WriteUint(m_parse_params.MajorVersion());
    WriteUint(m_parse_params.MinorVersion());

    // output profile
    WriteUint(m_parse_params.Profile());

    // output level - does not conform to any defined level
    WriteUint(m_parse_params.Level());
}
