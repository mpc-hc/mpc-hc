/*****************************************************************
|
|    AP4 - Sample Description Objects
|
|    Copyright 2002 Gilles Boccon-Gibod & Julien Boeuf
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

#ifndef _AP4_ISMACRYP_H_
#define _AP4_ISMACRYP_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4SampleEntry.h"
#include "Ap4Atom.h"
#include "Ap4AtomFactory.h"
#include "Ap4SampleDescription.h"
#include "Ap4Processor.h"

/*----------------------------------------------------------------------
|       class references
+---------------------------------------------------------------------*/
class AP4_StreamCipher;

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
const AP4_UI32 AP4_ISMACRYP_SCHEME_TYPE_IAEC = AP4_ATOM_TYPE('i','A','E','C');
const AP4_Size AP4_ISMACRYP_IAEC_KEY_LENGTH = 16;

/*----------------------------------------------------------------------
|       AP4_EncaSampleEntry
+---------------------------------------------------------------------*/
class AP4_EncaSampleEntry : public AP4_AudioSampleEntry
{
public:
    // methods
    AP4_EncaSampleEntry(AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);
    AP4_EncaSampleEntry(AP4_UI32          sample_rate, 
                        AP4_UI16          sample_size,
                        AP4_UI16          channel_count,
                        AP4_EsDescriptor* descriptor);

    // methods
    AP4_SampleDescription* ToSampleDescription();
};

/*----------------------------------------------------------------------
|       AP4_EncvSampleEntry
+---------------------------------------------------------------------*/
class AP4_EncvSampleEntry : public AP4_VisualSampleEntry
{
public:
    // constructors
    AP4_EncvSampleEntry(AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);
    AP4_EncvSampleEntry(AP4_UI16          width,
                        AP4_UI16          height,
                        AP4_UI16          depth,
                        const char*       compressor_name,
                        AP4_EsDescriptor* descriptor);

    // methods
    AP4_SampleDescription* ToSampleDescription();
};

/*----------------------------------------------------------------------
|       AP4_IsmaKeyMap
+---------------------------------------------------------------------*/
class AP4_IsmaKeyMap
{
public:
    // constructors and destructor
    AP4_IsmaKeyMap();
    ~AP4_IsmaKeyMap();

    // methods
    AP4_Result SetKey(AP4_UI32 track_id, const AP4_UI08* key, const AP4_UI08* salt = NULL);
    AP4_Result GetKey(AP4_UI32 track_id, const AP4_UI08*& key, const AP4_UI08*& salt);

private:
    // types
    class KeyEntry {
    public:
        KeyEntry(AP4_UI32 track_id, const AP4_UI08* key, const AP4_UI08* salt = NULL);
        void SetKey(const AP4_UI08* key, const AP4_UI08* salt);
        AP4_Ordinal m_TrackId;
        AP4_UI08    m_Key[AP4_ISMACRYP_IAEC_KEY_LENGTH];
        AP4_UI08    m_Salt[AP4_ISMACRYP_IAEC_KEY_LENGTH];
    };

    // methods
    KeyEntry* GetEntry(AP4_UI32 track_id);

    // members
    AP4_List<KeyEntry> m_KeyEntries;
};

/*----------------------------------------------------------------------
|       AP4_IsmaSchemeInfo
+---------------------------------------------------------------------*/
class AP4_IsmaCrypSchemeInfo
{
public:
    // constructors and destructor
    AP4_IsmaCrypSchemeInfo(AP4_ContainerAtom* schi);
    virtual ~AP4_IsmaCrypSchemeInfo(){}

    // accessors
    AP4_ContainerAtom& GetSchiAtom() { return m_SchiAtom; }

protected:
    AP4_ContainerAtom m_SchiAtom;
};

/*----------------------------------------------------------------------
|       AP4_IsmaCrypSampleDescription
+---------------------------------------------------------------------*/
class AP4_IsmaCrypSampleDescription : public AP4_SampleDescription
{
public:
    // constructor and destructor
    AP4_IsmaCrypSampleDescription(AP4_MpegSampleDescription* original_sample_description,
                                  AP4_UI32                   original_format,
                                  AP4_UI32                   scheme_type,
                                  AP4_UI32                   scheme_version,
                                  const char*                scheme_uri,
                                  AP4_ContainerAtom*         schi_atom);
    ~AP4_IsmaCrypSampleDescription();
    
    // accessors
    AP4_MpegSampleDescription* GetOriginalSampleDescription() {
        return m_OriginalSampleDescription;
    }
    AP4_UI32    GetOriginalFormat() { return m_OriginalFormat; }
    AP4_UI32    GetSchemeType()     { return m_SchemeType;     }
    AP4_UI32    GetSchemeVersion()  { return m_SchemeVersion;  }
    AP4_String& GetSchemeUri()      { return m_SchemeUri;      }
    AP4_IsmaCrypSchemeInfo* GetSchemeInfo() { return m_SchemeInfo; }

    // implementation of abstract base class methods
    virtual AP4_Atom* ToAtom() const;

private:
    // members
    AP4_MpegSampleDescription* m_OriginalSampleDescription;
    AP4_UI32                   m_OriginalFormat;
    AP4_UI32                   m_SchemeType;
    AP4_UI32                   m_SchemeVersion;
    AP4_String                 m_SchemeUri;
    AP4_IsmaCrypSchemeInfo*    m_SchemeInfo;
};

/*----------------------------------------------------------------------
|       AP4_IsmaCipher
+---------------------------------------------------------------------*/
class AP4_IsmaCipher
{
public:
    // constructor and destructor
    AP4_IsmaCipher(const AP4_UI08* key, 
                   const AP4_UI08* salt,
                   AP4_Size        iv_length,
                   AP4_Size        key_indicator_length,
                   bool            selective_encryption);
   ~AP4_IsmaCipher();
    AP4_Result EncryptSample(AP4_DataBuffer& data_in,
                             AP4_DataBuffer& data_out,
                             AP4_Offset      iv,
                             bool            skip_encryption);
    AP4_Result DecryptSample(AP4_DataBuffer& data_in,
                             AP4_DataBuffer& data_out);

private:
    // members
    AP4_StreamCipher* m_Cipher;
    AP4_Size          m_IvLength;
    AP4_Size          m_KeyIndicatorLength;
    bool              m_SelectiveEncryption;
};

/*----------------------------------------------------------------------
|       AP4_IsmaDecryptingProcessor
+---------------------------------------------------------------------*/
class AP4_IsmaDecryptingProcessor : public AP4_Processor
{
public:
    // accessors
    AP4_IsmaKeyMap& GetKeyMap() { return m_KeyMap; }

    // methods
    virtual AP4_Processor::TrackHandler* CreateTrackHandler(AP4_TrakAtom* trak);

private:
    // members
    AP4_IsmaKeyMap m_KeyMap;
};

/*----------------------------------------------------------------------
|       AP4_IsmaEncryptingProcessor
+---------------------------------------------------------------------*/
class AP4_IsmaEncryptingProcessor : public AP4_Processor
{
public:
    // constructors and destructor
    AP4_IsmaEncryptingProcessor(const char* kms_uri);

    // accessors
    AP4_IsmaKeyMap& GetKeyMap() { return m_KeyMap; }

    // methods
    virtual AP4_Processor::TrackHandler* CreateTrackHandler(AP4_TrakAtom* trak);

private:
    // members
    AP4_IsmaKeyMap m_KeyMap;
    AP4_String     m_KmsUri;
};

#endif // _AP4_ISMACRYP_H_
