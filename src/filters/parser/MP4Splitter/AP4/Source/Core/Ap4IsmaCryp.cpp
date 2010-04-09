/*****************************************************************
|
|    AP4 - ISMA E&A Support
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

/*----------------------------------------------------------------------
|   includes
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
#include "Ap4IsltAtom.h"
#include "Ap4Utils.h"
#include "Ap4TrakAtom.h"
#include "Ap4HdlrAtom.h"

/*----------------------------------------------------------------------
|   AP4_IsmaCipher::CreateSampleDecrypter
+---------------------------------------------------------------------*/
AP4_Result
AP4_IsmaCipher::CreateSampleDecrypter(AP4_ProtectedSampleDescription* sample_description, 
                                      const AP4_UI08*                 key, 
                                      AP4_Size                        key_size,
                                      AP4_BlockCipherFactory*         block_cipher_factory,
                                      AP4_IsmaCipher*&                decrypter)
{
    // check parameters
    if (key == NULL || block_cipher_factory == NULL) {
        return AP4_ERROR_INVALID_PARAMETERS;
    }
    decrypter = NULL;

    // create the cipher
    AP4_BlockCipher* block_cipher = NULL;
    AP4_Result result = block_cipher_factory->Create(AP4_BlockCipher::AES_128,
                                                     AP4_BlockCipher::ENCRYPT,
                                                     key,
                                                     key_size,
                                                     block_cipher);
    if (AP4_FAILED(result)) return result;

    // get the scheme info atom
    AP4_ContainerAtom* schi = sample_description->GetSchemeInfo()->GetSchiAtom();
    if (schi == NULL) return AP4_ERROR_INVALID_FORMAT;

    // get the cipher params
    AP4_IsfmAtom* isfm = AP4_DYNAMIC_CAST(AP4_IsfmAtom, schi->FindChild("iSFM"));
    if (isfm == NULL) return AP4_ERROR_INVALID_FORMAT;
    
    // get the salt
    AP4_IsltAtom* salt = AP4_DYNAMIC_CAST(AP4_IsltAtom, schi->FindChild("iSLT"));

    // instantiate the decrypter
    decrypter = new AP4_IsmaCipher(block_cipher, 
                                   salt?salt->GetSalt():NULL, 
                                   isfm->GetIvLength(),
                                   isfm->GetKeyIndicatorLength(),
                                   isfm->GetSelectiveEncryption());
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_IsmaCrypCipher::AP4_IsmaCipher
+---------------------------------------------------------------------*/
AP4_IsmaCipher::AP4_IsmaCipher(AP4_BlockCipher* block_cipher, 
                               const AP4_UI08*  salt, 
                               AP4_UI08         iv_length, 
                               AP4_UI08         key_indicator_length, 
                               bool             selective_encryption) :
    m_IvLength(iv_length),
    m_KeyIndicatorLength(key_indicator_length),
    m_SelectiveEncryption(selective_encryption)
{
    // NOTE: we do not handle key indicators yet, so there is only one key.

    // left-align the salt
    unsigned char salt_128[AP4_CIPHER_BLOCK_SIZE];
    unsigned int i=0;
    if (salt) {
        for (; i<8; i++) {
            salt_128[i] = salt[i];
        }
    }
    for (; i<AP4_CIPHER_BLOCK_SIZE; i++) {
        salt_128[i] = 0;
    }
    
    // create a cipher
    m_Cipher = new AP4_CtrStreamCipher(block_cipher, salt_128, iv_length);
}

/*----------------------------------------------------------------------
|   AP4_IsmaCipher::~AP4_IsmaCipher
+---------------------------------------------------------------------*/
AP4_IsmaCipher::~AP4_IsmaCipher()
{
    delete m_Cipher;
}

/*----------------------------------------------------------------------
|   AP4_IsmaCipher::GetDecryptedSampleSize
+---------------------------------------------------------------------*/
AP4_Size   
AP4_IsmaCipher::GetDecryptedSampleSize(AP4_Sample& sample)
{
    AP4_Size isma_header_size = m_KeyIndicatorLength + m_IvLength;
    if (m_SelectiveEncryption) {
        ++isma_header_size;
    }
    return sample.GetSize()-isma_header_size;
}

/*----------------------------------------------------------------------
|   AP4_IsmaCipher::DecryptSampleData
+---------------------------------------------------------------------*/
AP4_Result 
AP4_IsmaCipher::DecryptSampleData(AP4_DataBuffer& data_in,
                                  AP4_DataBuffer& data_out,
                                  const AP4_UI08* /*iv*/)
{
    bool            is_encrypted = true;
    const AP4_UI08* in = data_in.GetData();
    AP4_Size        in_size = data_in.GetDataSize();
    AP4_Size        header_size;

    // default to 0 output 
    data_out.SetDataSize(0);

    // check the selective encryption flag
    if (in_size < 1) return AP4_ERROR_INVALID_FORMAT;
    if (m_SelectiveEncryption) {
        is_encrypted = ((in[0]&0x80)!=0);
        in++;
    }

    // check the header size
    header_size = (m_SelectiveEncryption?1:0)+
                  (is_encrypted?m_KeyIndicatorLength+m_IvLength:0);
    if (header_size > in_size) return AP4_ERROR_INVALID_FORMAT;

    // process the sample data
    AP4_Size payload_size = in_size-header_size;
    data_out.SetDataSize(payload_size);
    AP4_UI08* out = data_out.UseData();
    if (is_encrypted) {
        // get the IV
        const AP4_UI08* iv = in;
        in += m_IvLength;

        // get the key indicator (we only support up to 32 bits)
        unsigned int to_read = m_KeyIndicatorLength;
        while (to_read > 4) {
            // skip anything above 4 bytes
            to_read--;
            in++;
        }
        AP4_UI32 key_indicator = 0;
        while (to_read--) {
            key_indicator = (key_indicator<<8) | *in++; 
            header_size++;
        }
        // we only support key indicator = 0 for now... (TODO)
        if (key_indicator != 0) {
            return AP4_ERROR_NOT_SUPPORTED;
        }

        m_Cipher->SetIV(iv);
        m_Cipher->ProcessBuffer(in, payload_size, out);
    } else {
        AP4_CopyMemory(out, in, payload_size);
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_IsmaCipher::EncryptSampleData
+---------------------------------------------------------------------*/
AP4_Result 
AP4_IsmaCipher::EncryptSampleData(AP4_DataBuffer& data_in,
                                  AP4_DataBuffer& data_out,
                                  AP4_UI32        iv)
{
    // setup the buffers
    const unsigned char* in = data_in.GetData();
    data_out.SetDataSize(data_in.GetDataSize()+4);
    unsigned char* out = data_out.UseData();

    // IV on 4 bytes
    AP4_BytesFromUInt32BE(out, iv);

    // encrypt the payload
    m_Cipher->SetIV(out);
    AP4_Size data_size = data_in.GetDataSize();
    m_Cipher->ProcessBuffer(in, data_size, out+4);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_IsmaTrackDecrypter::Create
+---------------------------------------------------------------------*/
AP4_Result
AP4_IsmaTrackDecrypter::Create(const AP4_UI08*                 key, 
                               AP4_Size                        key_size,
                               AP4_ProtectedSampleDescription* sample_description,
                               AP4_SampleEntry*                sample_entry,
                               AP4_BlockCipherFactory*         block_cipher_factory,
                               AP4_IsmaTrackDecrypter*&        decrypter)
{
    // instantiate the cipher
    AP4_IsmaCipher* cipher = NULL;
    decrypter              = NULL;
    AP4_Result result = AP4_IsmaCipher::CreateSampleDecrypter(sample_description,
                                                              key,
                                                              key_size,
                                                              block_cipher_factory,
                                                              cipher);
    if (AP4_FAILED(result)) return result;

    // instanciate the object
    decrypter = new AP4_IsmaTrackDecrypter(cipher, 
                                           sample_entry, 
                                           sample_description->GetOriginalFormat());
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_IsmaTrackDecrypter::AP4_IsmaTrackDecrypter
+---------------------------------------------------------------------*/
AP4_IsmaTrackDecrypter::AP4_IsmaTrackDecrypter(AP4_IsmaCipher*   cipher,
                                               AP4_SampleEntry*  sample_entry,
                                               AP4_UI32          original_format) :
    m_Cipher(cipher),
    m_SampleEntry(sample_entry),
    m_OriginalFormat(original_format)
{
}

/*----------------------------------------------------------------------
|   AP4_IsmaTrackDecrypter::~AP4_IsmaTrackDecrypter
+---------------------------------------------------------------------*/
AP4_IsmaTrackDecrypter::~AP4_IsmaTrackDecrypter()
{
    delete m_Cipher;
}

/*----------------------------------------------------------------------
|   AP4_IsmaTrackDecrypter::GetProcessedSampleSize
+---------------------------------------------------------------------*/
AP4_Size   
AP4_IsmaTrackDecrypter::GetProcessedSampleSize(AP4_Sample& sample)
{
    return m_Cipher->GetDecryptedSampleSize(sample);
}

/*----------------------------------------------------------------------
|   AP4_IsmaTrackDecrypter::ProcessTrack
+---------------------------------------------------------------------*/
AP4_Result   
AP4_IsmaTrackDecrypter::ProcessTrack()
{
    m_SampleEntry->SetType(m_OriginalFormat);
    m_SampleEntry->DeleteChild(AP4_ATOM_TYPE_SINF);
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_IsmaDecrypter::ProcessSample
+---------------------------------------------------------------------*/
AP4_Result 
AP4_IsmaTrackDecrypter::ProcessSample(AP4_DataBuffer& data_in,
                                      AP4_DataBuffer& data_out)
{
    return m_Cipher->DecryptSampleData(data_in, data_out);
}

/*----------------------------------------------------------------------
|   AP4_IsmaTrackEncrypter
+---------------------------------------------------------------------*/
class AP4_IsmaTrackEncrypter : public AP4_Processor::TrackHandler {
public:
    // constructor
    AP4_IsmaTrackEncrypter(const char*      kms_uri,
                           AP4_BlockCipher* block_cipher,
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
    AP4_UI32         m_Counter;
};

/*----------------------------------------------------------------------
|   AP4_IsmaTrackEncrypter::AP4_IsmaTrackEncrypter
+---------------------------------------------------------------------*/
AP4_IsmaTrackEncrypter::AP4_IsmaTrackEncrypter(
    const char*      kms_uri,
    AP4_BlockCipher* block_cipher,
    const AP4_UI08*  salt,
    AP4_SampleEntry* sample_entry,
    AP4_UI32         format) :
    m_KmsUri(kms_uri),
    m_SampleEntry(sample_entry),
    m_Format(format),
    m_Counter(0)
{
    // instantiate the cipher (fixed params for now)
    m_Cipher = new AP4_IsmaCipher(block_cipher, salt, 4, 0, false);
}

/*----------------------------------------------------------------------
|   AP4_IsmaTrackEncrypter::~AP4_IsmaTrackEncrypter
+---------------------------------------------------------------------*/
AP4_IsmaTrackEncrypter::~AP4_IsmaTrackEncrypter()
{
    delete m_Cipher;
}

/*----------------------------------------------------------------------
|   AP4_IsmaTrackEncrypter::GetProcessedSampleSize
+---------------------------------------------------------------------*/
AP4_Size   
AP4_IsmaTrackEncrypter::GetProcessedSampleSize(AP4_Sample& sample)
{
    return sample.GetSize()+4; //fixed header size for now
}

/*----------------------------------------------------------------------
|   AP4_IsmaTrackEncrypter::ProcessTrack
+---------------------------------------------------------------------*/
AP4_Result   
AP4_IsmaTrackEncrypter::ProcessTrack()
{
    // sinf container
    AP4_ContainerAtom* sinf = new AP4_ContainerAtom(AP4_ATOM_TYPE_SINF);

    // original format
    AP4_FrmaAtom* frma = new AP4_FrmaAtom(m_SampleEntry->GetType());
    
    // scheme
    AP4_SchmAtom* schm = new AP4_SchmAtom(AP4_PROTECTION_SCHEME_TYPE_IAEC, 1);
    
    // scheme info
    AP4_ContainerAtom* schi = new AP4_ContainerAtom(AP4_ATOM_TYPE_SCHI);
    AP4_IkmsAtom*      ikms = new AP4_IkmsAtom(m_KmsUri.GetChars());
    AP4_IsfmAtom*      isfm = new AP4_IsfmAtom(m_Cipher->GetSelectiveEncryption(), 
                                               m_Cipher->GetKeyIndicatorLength(), 
                                               m_Cipher->GetIvLength());
    AP4_IsltAtom*      islt = new AP4_IsltAtom(m_Cipher->GetCipher()->GetIV());

    // populate the schi container
    schi->AddChild(ikms);
    schi->AddChild(isfm);
    schi->AddChild(islt);

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
|   AP4_IsmaTrackEncrypter::ProcessSample
+---------------------------------------------------------------------*/
AP4_Result 
AP4_IsmaTrackEncrypter::ProcessSample(AP4_DataBuffer& data_in,
                                      AP4_DataBuffer& data_out)
{
    AP4_Result result = m_Cipher->EncryptSampleData(data_in, data_out, m_Counter);
    if (AP4_FAILED(result)) return result;

    m_Counter += (data_in.GetDataSize()+AP4_CIPHER_BLOCK_SIZE-1)/AP4_CIPHER_BLOCK_SIZE;
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_IsmaEncryptingProcessor::AP4_IsmaEncryptingProcessor
+---------------------------------------------------------------------*/
AP4_IsmaEncryptingProcessor::AP4_IsmaEncryptingProcessor(const char*             kms_uri,
                                                         AP4_BlockCipherFactory* block_cipher_factory) :
    m_KmsUri(kms_uri)
{
    if (block_cipher_factory == NULL) {
        m_BlockCipherFactory = &AP4_DefaultBlockCipherFactory::Instance;
    } else {
        m_BlockCipherFactory = block_cipher_factory;
    }
}

/*----------------------------------------------------------------------
|   AP4_IsmaEncryptingProcessor:CreateTrackHandler
+---------------------------------------------------------------------*/
AP4_Processor::TrackHandler* 
AP4_IsmaEncryptingProcessor::CreateTrackHandler(AP4_TrakAtom* trak)
{
    // find the stsd atom
    AP4_StsdAtom* stsd = AP4_DYNAMIC_CAST(AP4_StsdAtom, trak->FindChild("mdia/minf/stbl/stsd"));

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
    if (AP4_SUCCEEDED(m_KeyMap.GetKeyAndIv(trak->GetId(), key, salt))) {
        switch (entry->GetType()) {
            case AP4_ATOM_TYPE_MP4A:
                format = AP4_ATOM_TYPE_ENCA;
                break;

            case AP4_ATOM_TYPE_MP4V:
            case AP4_ATOM_TYPE_AVC1:
                format = AP4_ATOM_TYPE_ENCV;
                break;

            default: {
                // try to find if this is audio or video
                AP4_HdlrAtom* hdlr = AP4_DYNAMIC_CAST(AP4_HdlrAtom, trak->FindChild("mdia/hdlr"));
                if (hdlr) {
                    switch (hdlr->GetHandlerType()) {
                        case AP4_HANDLER_TYPE_SOUN:
                            format = AP4_ATOM_TYPE_ENCA;
                            break;

                        case AP4_HANDLER_TYPE_VIDE:
                            format = AP4_ATOM_TYPE_ENCV;
                            break;
                    }
                }
                break;
            }
        }
        if (format) {
            // create the block cipher
            AP4_BlockCipher* block_cipher = NULL;
            AP4_Result result = m_BlockCipherFactory->Create(AP4_BlockCipher::AES_128, 
                                                             AP4_BlockCipher::ENCRYPT, 
                                                             key, 
                                                             AP4_CIPHER_BLOCK_SIZE, 
                                                             block_cipher);
            if (AP4_FAILED(result)) return NULL;

            // create the encrypter
            return new AP4_IsmaTrackEncrypter(m_KmsUri.GetChars(), 
                                              block_cipher, 
                                              salt, 
                                              entry,
                                              format);
        }
    }

    return NULL;
}
