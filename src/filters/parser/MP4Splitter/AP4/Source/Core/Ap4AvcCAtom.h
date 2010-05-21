/*****************************************************************
|
|    AP4 - avcC Atoms 
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
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

#ifndef _AP4_AVCC_ATOM_H_
#define _AP4_AVCC_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Atom.h"
#include "Ap4Array.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI08 AP4_AVC_PROFILE_BASELINE = 66;
const AP4_UI08 AP4_AVC_PROFILE_MAIN     = 77;
const AP4_UI08 AP4_AVC_PROFILE_EXTENDED = 88;
const AP4_UI08 AP4_AVC_PROFILE_HIGH     = 100;
const AP4_UI08 AP4_AVC_PROFILE_HIGH_10  = 110;
const AP4_UI08 AP4_AVC_PROFILE_HIGH_422 = 122;
const AP4_UI08 AP4_AVC_PROFILE_HIGH_444 = 144;


/*----------------------------------------------------------------------
|   AP4_AvccAtom
+---------------------------------------------------------------------*/
class AP4_AvccAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_AvccAtom, AP4_Atom)

    // class methods
    static AP4_AvccAtom* Create(AP4_Size size, AP4_ByteStream& stream);
    static const char*   GetProfileName(AP4_UI08 profile);

    // constructors
    AP4_AvccAtom();
    AP4_AvccAtom(AP4_UI08 profile,
                 AP4_UI08 level,
                 AP4_UI08 profile_compatibility,
                 AP4_UI08 length_size,
                 const AP4_Array<AP4_DataBuffer>& sequence_parameters,
                 const AP4_Array<AP4_DataBuffer>& picture_parameters);
    AP4_AvccAtom(const AP4_AvccAtom& other); // copy construtor
    
    // methods
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    // accessors
    AP4_UI08 GetConfigurationVersion() const { return m_ConfigurationVersion; }
    AP4_UI08 GetProfile() const              { return m_Profile; }
    AP4_UI08 GetLevel() const                { return m_Level; }
    AP4_UI08 GetProfileCompatibility() const { return m_ProfileCompatibility; }
    AP4_UI08 GetNaluLengthSize() const       { return m_NaluLengthSize; }
    AP4_Array<AP4_DataBuffer>& GetSequenceParameters() { return m_SequenceParameters; }
    AP4_Array<AP4_DataBuffer>& GetPictureParameters()  { return m_PictureParameters; }
    const AP4_DataBuffer& GetRawBytes() const { return m_RawBytes; }

private:
    // methods
    AP4_AvccAtom(AP4_UI32 size, const AP4_UI08* payload);
    void UpdateRawBytes();
    
    // members
    AP4_UI08                  m_ConfigurationVersion;
    AP4_UI08                  m_Profile;
    AP4_UI08                  m_Level;
    AP4_UI08                  m_ProfileCompatibility;
    AP4_UI08                  m_NaluLengthSize;
    AP4_Array<AP4_DataBuffer> m_SequenceParameters;
    AP4_Array<AP4_DataBuffer> m_PictureParameters;
    AP4_DataBuffer            m_RawBytes;
};

#endif // _AP4_AVCC_ATOM_H_
