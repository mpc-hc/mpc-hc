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

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4IsmaCryp.h"
#include "Ap4SchmAtom.h"
#include "Ap4StsdAtom.h"
#include "Ap4Sample.h"
#include "Ap4StreamCipher.h"
#include "Ap4IsfmAtom.h"
#include "Ap4FrmaAtom.h"
#include "Ap4IkmsAtom.h"
#include "Ap4IsfmAtom.h"
#include "Ap4Utils.h"
#include "Ap4TrakAtom.h"

/*----------------------------------------------------------------------
|       AP4_EncaSampleEntry::AP4_EncaSampleEntry
+---------------------------------------------------------------------*/
AP4_EncaSampleEntry::AP4_EncaSampleEntry(AP4_UI32          sample_rate, 
                                         AP4_UI16          sample_size,
                                         AP4_UI16          channel_count,
                                         AP4_EsDescriptor* descriptor) :
AP4_AudioSampleEntry(AP4_ATOM_TYPE_ENCA, 
                     descriptor,
                     sample_rate, 
                     sample_size, 
                     channel_count)
{
}

/*----------------------------------------------------------------------
|       AP4_EncaSampleEntry::AP4_EncaSampleEntry
+---------------------------------------------------------------------*/
AP4_EncaSampleEntry::AP4_EncaSampleEntry(AP4_Size         size,
                                         AP4_ByteStream&  stream,
                                         AP4_AtomFactory& atom_factory) :
AP4_AudioSampleEntry(AP4_ATOM_TYPE_ENCA, size, stream, atom_factory)
{
}

/*----------------------------------------------------------------------
|       AP4_EncaSampleEntry::ToSampleDescription
+---------------------------------------------------------------------*/
AP4_SampleDescription*
AP4_EncaSampleEntry::ToSampleDescription()
{
    // get the original sample format
    AP4_FrmaAtom* frma = (AP4_FrmaAtom*)FindChild("sinf/frma");

    // get the scheme info
    AP4_SchmAtom* schm = (AP4_SchmAtom*)FindChild("sinf/schm");
    if (schm == NULL) return NULL;

    // get the sample description for the original sample entry
    AP4_MpegAudioSampleDescription* original_sample_description = 
        (AP4_MpegAudioSampleDescription*)
        AP4_AudioSampleEntry::ToSampleDescription();

    // get the schi atom
    AP4_ContainerAtom* schi;
    schi = static_cast<AP4_ContainerAtom*>(FindChild("sinf/schi"));

    // create the sample description
    return DNew AP4_IsmaCrypSampleDescription(
        original_sample_description,
        frma?frma->GetOriginalFormat():AP4_ATOM_TYPE_MP4A,
        schm->GetSchemeType(),
        schm->GetSchemeVersion(),
        schm->GetSchemeUri().c_str(),
        schi);
}

/*----------------------------------------------------------------------
|       AP4_EncvSampleEntry::AP4_EncaSampleEntry
+---------------------------------------------------------------------*/
AP4_EncvSampleEntry::AP4_EncvSampleEntry(AP4_UI16          width,
                                         AP4_UI16          height,
                                         AP4_UI16          depth,
                                         const char*       compressor_name,
                                         AP4_EsDescriptor* descriptor) :
    AP4_VisualSampleEntry(AP4_ATOM_TYPE_ENCV,
                          descriptor,
                          width,
                          height, 
                          depth,
                          compressor_name)
{
}

/*----------------------------------------------------------------------
|       AP4_EncvSampleEntry::AP4_EncvSampleEntry
+---------------------------------------------------------------------*/
AP4_EncvSampleEntry::AP4_EncvSampleEntry(AP4_Size         size,
                                         AP4_ByteStream&  stream,
                                         AP4_AtomFactory& atom_factory) :
    AP4_VisualSampleEntry(AP4_ATOM_TYPE_ENCV, size, stream, atom_factory)
{
}

/*----------------------------------------------------------------------
|       AP4_EncvSampleEntry::ToSampleDescription
+---------------------------------------------------------------------*/
AP4_SampleDescription*
AP4_EncvSampleEntry::ToSampleDescription()
{
    // get the original sample format
    AP4_FrmaAtom* frma = (AP4_FrmaAtom*)FindChild("sinf/frma");

    // get the scheme info
    AP4_SchmAtom* schm = (AP4_SchmAtom*)FindChild("sinf/schm");
    if (schm == NULL) return NULL;

    // get the sample description for the original sample entry
    AP4_MpegVideoSampleDescription* original_sample_description = 
        (AP4_MpegVideoSampleDescription*)
        AP4_VisualSampleEntry::ToSampleDescription();

    // get the schi atom
    AP4_ContainerAtom* schi;
    schi = static_cast<AP4_ContainerAtom*>(FindChild("sinf/schi"));

    // create the sample description
    return DNew AP4_IsmaCrypSampleDescription(
        original_sample_description,
        frma?frma->GetOriginalFormat():AP4_ATOM_TYPE_MP4V,
        schm->GetSchemeType(),
        schm->GetSchemeVersion(),
        schm->GetSchemeUri().c_str(),
        schi);
}

/*----------------------------------------------------------------------
|       AP4_IsmaCrypSampleDescription::AP4_IsmaCrypSampleDescription
+---------------------------------------------------------------------*/
AP4_IsmaCrypSampleDescription::AP4_IsmaCrypSampleDescription(
    AP4_MpegSampleDescription* original_sample_description,
    AP4_UI32                   original_format,
    AP4_UI32                   scheme_type,
    AP4_UI32                   scheme_version,
    const char*                scheme_uri,
    AP4_ContainerAtom*         schi) :
    AP4_SampleDescription(TYPE_ISMACRYP),
    m_OriginalSampleDescription(original_sample_description),
    m_OriginalFormat(original_format),
    m_SchemeType(scheme_type),
    m_SchemeVersion(scheme_version),
    m_SchemeUri(scheme_uri)
{
    m_SchemeInfo = DNew AP4_IsmaCrypSchemeInfo(schi);
}

/*----------------------------------------------------------------------
|       AP4_IsmaCrypSampleDescription::~AP4_IsmaCrypSampleDescription
+---------------------------------------------------------------------*/
AP4_IsmaCrypSampleDescription::~AP4_IsmaCrypSampleDescription()
{
    delete m_SchemeInfo;
    delete m_OriginalSampleDescription;
}

/*----------------------------------------------------------------------
|       AP4_IsmaCrypSampleDescription::ToAtom
+---------------------------------------------------------------------*/
AP4_Atom*
AP4_IsmaCrypSampleDescription::ToAtom() const
{
    // TODO: not implemented yet
    return NULL;
}

/*----------------------------------------------------------------------
|       AP4_IsmaCrypSchemeInfo::AP4_IsmaCrypSchemeInfo
+---------------------------------------------------------------------*/
AP4_IsmaCrypSchemeInfo::AP4_IsmaCrypSchemeInfo(AP4_ContainerAtom* schi) :
    m_SchiAtom(AP4_ATOM_TYPE_SCHI)
{
    if (schi) {
        AP4_List<AP4_Atom>& children = schi->GetChildren();
        AP4_List<AP4_Atom>::Item* child_item = children.FirstItem();
        while (child_item) {
            AP4_Atom* child_atom = child_item->GetData();
            AP4_Atom* clone = child_atom->Clone();
            if (clone) m_SchiAtom.AddChild(clone);
            child_item = child_item->GetNext();
        }
    }
}

/*----------------------------------------------------------------------
|       AP4_IsmaCrypCipher::AP4_IsmaCipher
+---------------------------------------------------------------------*/
AP4_IsmaCipher::AP4_IsmaCipher(const AP4_UI08* key, 
                               const AP4_UI08* salt, 
                               AP4_Size        iv_length, 
                               AP4_Size        key_indicator_length, 
                               bool            selective_encryption) :
    m_IvLength(iv_length),
    m_KeyIndicatorLength(key_indicator_length),
    m_SelectiveEncryption(selective_encryption)
{
    // NOTE: we do not handle key indicators yey, so there is only one key.

    // left-align the salt
    unsigned char salt_128[AP4_ISMACRYP_IAEC_KEY_LENGTH];
    for (unsigned int i=0; i<8; i++) {
        salt_128[i] = salt[i];
    }
    for (unsigned int i=0; i<8; i++) {
        salt_128[8+i] = 0;
    }
    
    // create a cipher
    m_Cipher = DNew AP4_StreamCipher(key, salt_128);
}

/*----------------------------------------------------------------------
|       AP4_IsmaCipher::~AP4_IsmaCipher
+---------------------------------------------------------------------*/
AP4_IsmaCipher::~AP4_IsmaCipher()
{
    delete m_Cipher;
}

/*----------------------------------------------------------------------
|       AP4_IsmaCipher::DecryptSample
+---------------------------------------------------------------------*/
AP4_Result 
AP4_IsmaCipher::DecryptSample(AP4_DataBuffer& data_in,
                              AP4_DataBuffer& data_out)
{
    bool                 is_encrypted = true;
    const unsigned char* in = data_in.GetData();
    if (m_SelectiveEncryption) {
        is_encrypted = ((in[0]&1)==1);
        in++;
    }

    // get the IV (this implementation only supports un to 32 bits of IV)
    // so we skip anything beyond the last 4 bytes
    unsigned int to_read = m_IvLength;
    if (to_read > 16 || to_read == 0) return AP4_ERROR_INVALID_FORMAT;
    while (to_read > 4) {
        to_read--;
        in++;
    }
    AP4_UI32 iv = 0;
    while (to_read--) {
        iv = (iv<<8) | *in++; 
    }

    // get the key indicator (we only support up to 32 bits as well)
    to_read = m_KeyIndicatorLength;
    if (to_read > 4 ) return AP4_ERROR_INVALID_FORMAT;
    while (to_read > 4) {
        to_read--;
        in++;
    }
    AP4_UI32 key_indicator = 0;
    while (to_read--) {
        key_indicator = (key_indicator<<8) | *in++; 
    }
    // we only support key indicator = 0 for now... (TODO)
    if (key_indicator != 0) {
        return AP4_FAILURE;
    }

    // process the sample data
    unsigned int header_size = in-data_in.GetData();
    unsigned int payload_size = data_in.GetDataSize()-header_size;
    data_out.SetDataSize(payload_size);
    unsigned char* out = data_out.UseData();
    if (is_encrypted) {
        m_Cipher->SetStreamOffset(iv);
        m_Cipher->ProcessBuffer(in, out, payload_size);
    } else {
        memcpy(out, in, payload_size);
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_IsmaCipher::EncryptSample
+---------------------------------------------------------------------*/
AP4_Result 
AP4_IsmaCipher::EncryptSample(AP4_DataBuffer& data_in,
                              AP4_DataBuffer& data_out,
                              AP4_Offset      iv,
                              bool            skip_encryption)
{
    // setup the buffers
    const unsigned char* in = data_in.GetData();
    data_out.SetDataSize(data_in.GetDataSize()+4);
    unsigned char* out = data_out.UseData();

    // IV on 4 bytes
    AP4_BytesFromUInt32BE(out, iv);
    out += 4;

    // encrypt the payload
    m_Cipher->SetStreamOffset(iv);
    m_Cipher->ProcessBuffer(in, out, data_in.GetDataSize());

    return AP4_FAILURE;
}

/*----------------------------------------------------------------------
|       AP4_IsmaTrackDecrypter
+---------------------------------------------------------------------*/
class AP4_IsmaTrackDecrypter : public AP4_Processor::TrackHandler {
public:
    // constructor
    AP4_IsmaTrackDecrypter(const AP4_UI08*                key,
                           const AP4_UI08*                salt,
                           AP4_IsmaCrypSampleDescription* sample_description,
                           AP4_SampleEntry*               sample_entry);
    virtual ~AP4_IsmaTrackDecrypter();

    // methods
    virtual AP4_Size   GetProcessedSampleSize(AP4_Sample& sample);
    virtual AP4_Result ProcessTrack();
    virtual AP4_Result ProcessSample(AP4_DataBuffer& data_in,
        AP4_DataBuffer& data_out);

private:
    // members
    AP4_IsfmAtom*    m_CipherParams;
    AP4_IsmaCipher*  m_Cipher;
    AP4_SampleEntry* m_SampleEntry;
    AP4_UI32         m_OriginalFormat;
};

/*----------------------------------------------------------------------
|       AP4_IsmaTrackDecrypter::AP4_IsmaTrackDecrypter
+---------------------------------------------------------------------*/
AP4_IsmaTrackDecrypter::AP4_IsmaTrackDecrypter(
    const AP4_UI08*                key,
    const AP4_UI08*                salt,
    AP4_IsmaCrypSampleDescription* sample_description,
    AP4_SampleEntry*               sample_entry) :
    m_SampleEntry(sample_entry)
{
    // get the cipher params
    m_CipherParams = (AP4_IsfmAtom*)sample_description->GetSchemeInfo()->GetSchiAtom().FindChild("iSFM");
    
    // instantiate the cipher
    m_Cipher = DNew AP4_IsmaCipher(key, salt, 
                                  m_CipherParams->GetIvLength(),
                                  m_CipherParams->GetKeyIndicatorLength(),
                                  m_CipherParams->GetSelectiveEncryption());

    // get the sample entry details
    m_OriginalFormat = sample_description->GetOriginalFormat();
}

/*----------------------------------------------------------------------
|       AP4_IsmaTrackDecrypter::~AP4_IsmaTrackDecrypter
+---------------------------------------------------------------------*/
AP4_IsmaTrackDecrypter::~AP4_IsmaTrackDecrypter()
{
    delete m_Cipher;
}

/*----------------------------------------------------------------------
|       AP4_IsmaTrackDecrypter::GetProcessedSampleSize
+---------------------------------------------------------------------*/
AP4_Size   
AP4_IsmaTrackDecrypter::GetProcessedSampleSize(AP4_Sample& sample)
{
    AP4_Size isma_header_size = 
        m_CipherParams->GetKeyIndicatorLength() +
        m_CipherParams->GetIvLength();
    if (m_CipherParams->GetSelectiveEncryption()) {
        isma_header_size++;
    }
    return sample.GetSize()-isma_header_size;
}

/*----------------------------------------------------------------------
|       AP4_IsmaTrackDecrypter::ProcessTrack
+---------------------------------------------------------------------*/
AP4_Result   
AP4_IsmaTrackDecrypter::ProcessTrack()
{
    m_SampleEntry->SetType(m_OriginalFormat);
    m_SampleEntry->DeleteChild(AP4_ATOM_TYPE_SINF);
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_IsmaDecrypter::ProcessSample
+---------------------------------------------------------------------*/
AP4_Result 
AP4_IsmaTrackDecrypter::ProcessSample(AP4_DataBuffer& data_in,
                                      AP4_DataBuffer& data_out)
{
    return m_Cipher->DecryptSample(data_in, data_out);
}

/*----------------------------------------------------------------------
|       AP4_IsmaDecryptingProcessor:CreateTrackHandler
+---------------------------------------------------------------------*/
AP4_Processor::TrackHandler* 
AP4_IsmaDecryptingProcessor::CreateTrackHandler(AP4_TrakAtom* trak)
{
    // find the stsd atom
    AP4_StsdAtom* stsd = dynamic_cast<AP4_StsdAtom*>(
        trak->FindChild("mdia/minf/stbl/stsd"));

    // avoid tracks with no stsd atom (should not happen)
    if (stsd == NULL) return NULL;

    // we only look at the first sample description
    AP4_SampleDescription* desc = stsd->GetSampleDescription(0);
    AP4_SampleEntry* entry = stsd->GetSampleEntry(0);
    if (desc == NULL || entry == NULL) return NULL;
    if (desc->GetType() == AP4_SampleDescription::TYPE_ISMACRYP) {
        // create a handler for this track
        AP4_IsmaCrypSampleDescription* ismacryp_desc = 
            static_cast<AP4_IsmaCrypSampleDescription*>(desc);
        if (ismacryp_desc->GetSchemeType() == AP4_ISMACRYP_SCHEME_TYPE_IAEC) {
            const AP4_UI08* key;
            const AP4_UI08* salt;
            if (AP4_SUCCEEDED(m_KeyMap.GetKey(trak->GetId(), key, salt))) {
                return DNew AP4_IsmaTrackDecrypter(key, salt, ismacryp_desc, entry);
            }
        }
    }

    return NULL;
}

/*----------------------------------------------------------------------
|       AP4_IsmaTrackEncrypter
+---------------------------------------------------------------------*/
class AP4_IsmaTrackEncrypter : public AP4_Processor::TrackHandler {
public:
    // constructor
    AP4_IsmaTrackEncrypter(const char*      kms_uri,
                           const AP4_UI08*  key,
                           const AP4_UI08*  salt,
                           AP4_SampleEntry* sample_entry,
                           AP4_UI32         format);
    virtual ~AP4_IsmaTrackEncrypter();

    // methods
    virtual AP4_Size   GetProcessedSampleSize(AP4_Sample& sample);
    virtual AP4_Result ProcessTrack();
    virtual AP4_Result ProcessSample(AP4_DataBuffer& data_in,
                                     AP4_DataBuffer& data_out);

private:
    // members
    AP4_String       m_KmsUri;
    AP4_IsmaCipher*  m_Cipher;
    AP4_SampleEntry* m_SampleEntry;
    AP4_UI32         m_Format;
    AP4_Offset       m_ByteOffset;
};

/*----------------------------------------------------------------------
|       AP4_IsmaTrackEncrypter::AP4_IsmaTrackEncrypter
+---------------------------------------------------------------------*/
AP4_IsmaTrackEncrypter::AP4_IsmaTrackEncrypter(
    const char*      kms_uri,
    const AP4_UI08*  key,
    const AP4_UI08*  salt,
    AP4_SampleEntry* sample_entry,
    AP4_UI32         format) :
    m_KmsUri(kms_uri),
    m_SampleEntry(sample_entry),
    m_Format(format),
    m_ByteOffset(0)
{
    // instantiate the cipher (fixed params for now)
    m_Cipher = DNew AP4_IsmaCipher(key, salt, 4, 0, false);
}

/*----------------------------------------------------------------------
|       AP4_IsmaTrackEncrypter::~AP4_IsmaTrackEncrypter
+---------------------------------------------------------------------*/
AP4_IsmaTrackEncrypter::~AP4_IsmaTrackEncrypter()
{
    delete m_Cipher;
}

/*----------------------------------------------------------------------
|       AP4_IsmaTrackEncrypter::GetProcessedSampleSize
+---------------------------------------------------------------------*/
AP4_Size   
AP4_IsmaTrackEncrypter::GetProcessedSampleSize(AP4_Sample& sample)
{
    return sample.GetSize()+4; //fixed header size for now
}

/*----------------------------------------------------------------------
|       AP4_IsmaTrackEncrypter::ProcessTrack
+---------------------------------------------------------------------*/
AP4_Result   
AP4_IsmaTrackEncrypter::ProcessTrack()
{
    // sinf container
    AP4_ContainerAtom* sinf = DNew AP4_ContainerAtom(AP4_ATOM_TYPE_SINF);

    // original format
    AP4_FrmaAtom* frma = DNew AP4_FrmaAtom(m_SampleEntry->GetType());
    
    // scheme
    AP4_SchmAtom* schm = DNew AP4_SchmAtom(AP4_ISMACRYP_SCHEME_TYPE_IAEC, 1);
    
    // scheme info
    AP4_ContainerAtom* schi = DNew AP4_ContainerAtom(AP4_ATOM_TYPE_SCHI);
    AP4_IkmsAtom* ikms      = DNew AP4_IkmsAtom(m_KmsUri.c_str());
    AP4_IsfmAtom* isfm      = DNew AP4_IsfmAtom(false, 0, 4);

    // populate the schi container
    schi->AddChild(ikms);
    schi->AddChild(isfm);

    // populate the sinf container
    sinf->AddChild(frma);
    sinf->AddChild(schm);
    sinf->AddChild(schi);

    // add the sinf atom to the sample description
    m_SampleEntry->AddChild(sinf);

    // change the atom type of the sample description
    m_SampleEntry->SetType(m_Format);
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_IsmaTrackEncrypter::ProcessSample
+---------------------------------------------------------------------*/
AP4_Result 
AP4_IsmaTrackEncrypter::ProcessSample(AP4_DataBuffer& data_in,
                                      AP4_DataBuffer& data_out)
{
    AP4_Result result = m_Cipher->EncryptSample(data_in, data_out, m_ByteOffset, false);
    if (AP4_FAILED(result)) return result;

    m_ByteOffset += data_in.GetDataSize();
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_IsmaEncryptingProcessor::AP4_IsmaEncryptingProcessor
+---------------------------------------------------------------------*/
AP4_IsmaEncryptingProcessor::AP4_IsmaEncryptingProcessor(const char* kms_uri) :
    m_KmsUri(kms_uri)
{
}

/*----------------------------------------------------------------------
|       AP4_IsmaEncryptingProcessor:CreateTrackHandler
+---------------------------------------------------------------------*/
AP4_Processor::TrackHandler* 
AP4_IsmaEncryptingProcessor::CreateTrackHandler(AP4_TrakAtom* trak)
{
    // find the stsd atom
    AP4_StsdAtom* stsd = dynamic_cast<AP4_StsdAtom*>(
        trak->FindChild("mdia/minf/stbl/stsd"));

    // avoid tracks with no stsd atom (should not happen)
    if (stsd == NULL) return NULL;

    // only look at the first sample description
    AP4_SampleEntry* entry = stsd->GetSampleEntry(0);
    if (entry == NULL) return NULL;
        
    // create a handler for this track if we have a key for it and we know
    // how to map the type
    const AP4_UI08* key;
    const AP4_UI08* salt;
    AP4_UI32        format = 0;
    if (AP4_SUCCEEDED(m_KeyMap.GetKey(trak->GetId(), key, salt))) {
        switch (entry->GetType()) {
            case AP4_ATOM_TYPE_MP4A:
                format = AP4_ATOM_TYPE_ENCA;
                break;

            case AP4_ATOM_TYPE_MP4V:
            case AP4_ATOM_TYPE_AVC1:
                format = AP4_ATOM_TYPE_ENCV;
                break;
        }
        if (format) {
            return DNew AP4_IsmaTrackEncrypter(m_KmsUri.c_str(), 
                                              key, 
                                              salt, 
                                              entry,
                                              format);
        }
    }

    return NULL;
}

/*----------------------------------------------------------------------
|       AP4_IsmaKeyMap::AP4_IsmaKeyMap
+---------------------------------------------------------------------*/
AP4_IsmaKeyMap::AP4_IsmaKeyMap()
{
}

/*----------------------------------------------------------------------
|       AP4_IsmaKeyMap::~AP4_IsmaKeyMap
+---------------------------------------------------------------------*/
AP4_IsmaKeyMap::~AP4_IsmaKeyMap()
{
    m_KeyEntries.DeleteReferences();
}

/*----------------------------------------------------------------------
|       AP4_IsmaKeyMap::SetKey
+---------------------------------------------------------------------*/
AP4_Result 
AP4_IsmaKeyMap::SetKey(AP4_UI32 track_id, const AP4_UI08* key, const AP4_UI08* salt)
{
    KeyEntry* entry = GetEntry(track_id);
    if (entry == NULL) {
        m_KeyEntries.Add(DNew KeyEntry(track_id, key, salt));
    } else {
        entry->SetKey(key, salt);
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_IsmaKeyMap::GetKey
+---------------------------------------------------------------------*/
AP4_Result 
AP4_IsmaKeyMap::GetKey(AP4_UI32 track_id, const AP4_UI08*& key, const AP4_UI08*& salt)
{
    KeyEntry* entry = GetEntry(track_id);
    if (entry) {
        key = entry->m_Key;
        salt = entry->m_Salt;
        return AP4_SUCCESS;
    } else {
        return AP4_ERROR_NO_SUCH_ITEM;
    }
}

/*----------------------------------------------------------------------
|       AP4_IsmaKeyMap::GetEntry
+---------------------------------------------------------------------*/
AP4_IsmaKeyMap::KeyEntry*
AP4_IsmaKeyMap::GetEntry(AP4_UI32 track_id)
{
    AP4_List<KeyEntry>::Item* item = m_KeyEntries.FirstItem();
    while (item) {
        KeyEntry* entry = (KeyEntry*)item->GetData();
        if (entry->m_TrackId == track_id) return entry;
        item = item->GetNext();
    }

    return NULL;
}

/*----------------------------------------------------------------------
|       AP4_IsmaKeyMap::KeyEntry::KeyEntry
+---------------------------------------------------------------------*/
AP4_IsmaKeyMap::KeyEntry::KeyEntry(AP4_UI32        track_id, 
                                   const AP4_UI08* key, 
                                   const AP4_UI08* salt /* = NULL */) :
    m_TrackId(track_id)
{
    SetKey(key, salt);
}

/*----------------------------------------------------------------------
|       AP4_IsmaKeyMap::KeyEntry::SetKey
+---------------------------------------------------------------------*/
void
AP4_IsmaKeyMap::KeyEntry::SetKey(const AP4_UI08* key, const AP4_UI08* salt)
{
    memcpy(m_Key, key, sizeof(m_Key));
    if (salt) {
        memcpy(m_Salt, salt, sizeof(m_Salt));
    } else {
        memset(m_Salt, 0, sizeof(m_Salt));
    }
}

