/*****************************************************************
|
|    AP4 - sdp Atoms 
|
|    Copyright 2002-2005 Gilles Boccon-Gibod & Julien Boeuf
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
****************************************************************/

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4RtpAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"


/*----------------------------------------------------------------------
|       AP4_RtpAtom::AP4_RtpAtom
+---------------------------------------------------------------------*/
AP4_RtpAtom::AP4_RtpAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_RTP, size, false, stream)
{
    // desc format
    stream.ReadUI32(m_DescriptionFormat);

    // sdptext
    int str_size = size-(AP4_ATOM_HEADER_SIZE+4);
    if (str_size) {
        char* str = new char[str_size+1];
        stream.Read(str, str_size);
        str[str_size] = '\0'; // force null-termination
        m_SdpText = str;
        delete[] str;
    }
}

/*----------------------------------------------------------------------
|       AP4_RtpAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_RtpAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // description format
    result = stream.WriteUI32(m_DescriptionFormat);
    if (AP4_FAILED(result)) return result;

    // sdp text
    result = stream.Write(m_SdpText.c_str(), m_SdpText.length());
    if (AP4_FAILED(result)) return result;

    // pad with zeros if necessary
    AP4_Size padding = m_Size-(AP4_ATOM_HEADER_SIZE+4+m_SdpText.length());
    while (padding--) stream.WriteUI08(0);
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_RtpAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_RtpAtom::InspectFields(AP4_AtomInspector& inspector)
{
    char format_string[5];
    AP4_FormatFourChars(format_string, m_DescriptionFormat);
    inspector.AddField("description_format", format_string);
    inspector.AddField("sdp_text", m_SdpText.c_str());

    return AP4_SUCCESS;
}
