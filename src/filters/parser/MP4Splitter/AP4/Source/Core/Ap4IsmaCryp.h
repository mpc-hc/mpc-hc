/*****************************************************************
|
|    AP4 - ISMA E&A support
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

#ifndef _AP4_ISMACRYP_H_
#define _AP4_ISMACRYP_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4SampleEntry.h"
#include "Ap4Atom.h"
#include "Ap4AtomFactory.h"
#include "Ap4SampleDescription.h"
#include "Ap4Processor.h"
#include "Ap4Protection.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_CtrStreamCipher;
class AP4_IsfmAtom;

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI32 AP4_PROTECTION_SCHEME_TYPE_IAEC = AP4_ATOM_TYPE('i', 'A', 'E', 'C');

/*----------------------------------------------------------------------
|   AP4_IsmaCipher
+---------------------------------------------------------------------*/
class AP4_IsmaCipher : public AP4_SampleDecrypter
{
public:
    // factory
    static AP4_Result CreateSampleDecrypter(AP4_ProtectedSampleDescription* sample_description,
                                            const AP4_UI08*                 key,
                                            AP4_Size                        key_size,
                                            AP4_BlockCipherFactory*         block_cipher_factory,
                                            AP4_IsmaCipher*&                decrypter);

    // constructor and destructor
    AP4_IsmaCipher(AP4_BlockCipher* block_cipher,
                   const AP4_UI08*  salt,
                   AP4_UI08         iv_length,
                   AP4_UI08         key_indicator_length,
                   bool             selective_encryption);
    ~AP4_IsmaCipher();
    AP4_Result EncryptSampleData(AP4_DataBuffer& data_in,
                                 AP4_DataBuffer& data_out,
                                 AP4_UI32        offset);
    AP4_Result DecryptSampleData(AP4_DataBuffer& data_in,
                                 AP4_DataBuffer& data_out,
                                 const AP4_UI08* iv = NULL);
    AP4_Size   GetDecryptedSampleSize(AP4_Sample& sample);
    AP4_CtrStreamCipher* GetCipher()
    {
        return m_Cipher;
    }
    AP4_UI08             GetIvLength()
    {
        return m_IvLength;
    }
    AP4_UI08             GetKeyIndicatorLength()
    {
        return m_KeyIndicatorLength;
    }
    bool                 GetSelectiveEncryption()
    {
        return m_SelectiveEncryption;
    }

private:
    // members
    AP4_CtrStreamCipher* m_Cipher;
    AP4_UI08             m_IvLength;
    AP4_UI08             m_KeyIndicatorLength;
    bool                 m_SelectiveEncryption;
};

/*----------------------------------------------------------------------
|   AP4_IsmaTrackDecrypter
+---------------------------------------------------------------------*/
class AP4_IsmaTrackDecrypter : public AP4_Processor::TrackHandler
{
public:
    // construction
    static AP4_Result Create(const AP4_UI08*                 key,
                             AP4_Size                        key_size,
                             AP4_ProtectedSampleDescription* sample_description,
                             AP4_SampleEntry*                sample_entry,
                             AP4_BlockCipherFactory*         block_cipher_factory,
                             AP4_IsmaTrackDecrypter*&        decrypter);

    virtual ~AP4_IsmaTrackDecrypter();

    // methods
    virtual AP4_Size   GetProcessedSampleSize(AP4_Sample& sample);
    virtual AP4_Result ProcessTrack();
    virtual AP4_Result ProcessSample(AP4_DataBuffer& data_in,
                                     AP4_DataBuffer& data_out);

private:
    // constructor
    AP4_IsmaTrackDecrypter(AP4_IsmaCipher*  cipher,
                           AP4_SampleEntry* sample_entry,
                           AP4_UI32         original_format);

    // members
    AP4_IsmaCipher*  m_Cipher;
    AP4_SampleEntry* m_SampleEntry;
    AP4_UI32         m_OriginalFormat;
};

/*----------------------------------------------------------------------
|   AP4_IsmaEncryptingProcessor
+---------------------------------------------------------------------*/
class AP4_IsmaEncryptingProcessor : public AP4_Processor
{
public:
    // constructors and destructor
    AP4_IsmaEncryptingProcessor(const char*             kms_uri,
                                AP4_BlockCipherFactory* block_cipher_factory = NULL);

    // accessors
    AP4_ProtectionKeyMap& GetKeyMap()
    {
        return m_KeyMap;
    }

    // methods
    virtual AP4_Processor::TrackHandler* CreateTrackHandler(AP4_TrakAtom* trak);

private:
    // members
    AP4_ProtectionKeyMap    m_KeyMap;
    AP4_String              m_KmsUri;
    AP4_BlockCipherFactory* m_BlockCipherFactory;
};

#endif // _AP4_ISMACRYP_H_
