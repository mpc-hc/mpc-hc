/*****************************************************************
|
|    AP4 - PIFF support
|
|    Copyright 2002-2009 Axiomatic Systems, LLC
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

#ifndef _AP4_PIFF_H_
#define _AP4_PIFF_H_

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
#include "Ap4UuidAtom.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_StreamCipher;
class AP4_CbcStreamCipher;
class AP4_CtrStreamCipher;
class AP4_PiffSampleEncryptionAtom;

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI32 AP4_PROTECTION_SCHEME_TYPE_PIFF = AP4_ATOM_TYPE('p','i','f','f');
const AP4_UI32 AP4_PROTECTION_SCHEME_VERSION_PIFF_10 =  0x00010000;
const AP4_UI32 AP4_PIFF_BRAND = AP4_ATOM_TYPE('p','i','f','f');
const AP4_UI32 AP4_PIFF_ALGORITHM_ID_NONE = 0; 
const AP4_UI32 AP4_PIFF_ALGORITHM_ID_CTR  = 1; 
const AP4_UI32 AP4_PIFF_ALGORITHM_ID_CBC  = 2; 

extern AP4_UI08 const AP4_UUID_PIFF_TRACK_ENCRYPTION_ATOM[16];
extern AP4_UI08 const AP4_UUID_PIFF_SAMPLE_ENCRYPTION_ATOM[16];

const unsigned int AP4_PIFF_SAMPLE_ENCRYPTION_FLAG_OVERRIDE_TRACK_ENCRYPTION_DEFAULTS = 1;

typedef enum {
    AP4_PIFF_CIPHER_MODE_CTR,
    AP4_PIFF_CIPHER_MODE_CBC
} AP4_PiffCipherMode;
                  
/*----------------------------------------------------------------------
|   AP4_PiffSampleEncrypter
+---------------------------------------------------------------------*/
class AP4_PiffSampleEncrypter
{
public:
    // constructor and destructor
    AP4_PiffSampleEncrypter() { AP4_SetMemory(m_Iv, 0, 16); };
    virtual ~AP4_PiffSampleEncrypter() {}

    // methods
    virtual AP4_Result EncryptSampleData(AP4_DataBuffer& data_in,
                                         AP4_DataBuffer& data_out) = 0;    

    void            SetIv(const AP4_UI08* iv) { AP4_CopyMemory(m_Iv, iv, 16); }
    const AP4_UI08* GetIv()                   { return m_Iv;                  }
    
protected:
    AP4_UI08 m_Iv[16];
};

/*----------------------------------------------------------------------
|   AP4_PiffCtrSampleEncrypter
+---------------------------------------------------------------------*/
class AP4_PiffCtrSampleEncrypter : public AP4_PiffSampleEncrypter
{
public:
    // constructor and destructor
    AP4_PiffCtrSampleEncrypter(AP4_BlockCipher* block_cipher);
    ~AP4_PiffCtrSampleEncrypter();

    // methods
    virtual AP4_Result EncryptSampleData(AP4_DataBuffer& data_in,
                                         AP4_DataBuffer& data_out);
    
protected:
    // members
    AP4_CtrStreamCipher* m_Cipher;
};

/*----------------------------------------------------------------------
|   AP4_PiffAvcCtrSampleEncrypter
+---------------------------------------------------------------------*/
class AP4_PiffAvcCtrSampleEncrypter : public AP4_PiffCtrSampleEncrypter
{
public:
    // constructor and destructor
    AP4_PiffAvcCtrSampleEncrypter(AP4_BlockCipher* block_cipher,
                                  AP4_Size         nalu_length_size) :
        AP4_PiffCtrSampleEncrypter(block_cipher),
        m_NaluLengthSize(nalu_length_size) {}

    // methods
    virtual AP4_Result EncryptSampleData(AP4_DataBuffer& data_in,
                                         AP4_DataBuffer& data_out);
    
private:
    // members
    AP4_Size m_NaluLengthSize;
};

/*----------------------------------------------------------------------
|   AP4_PiffCbcSampleEncrypter
+---------------------------------------------------------------------*/
class AP4_PiffCbcSampleEncrypter : public AP4_PiffSampleEncrypter
{
public:
    // constructor and destructor
    AP4_PiffCbcSampleEncrypter(AP4_BlockCipher* block_cipher);
    ~AP4_PiffCbcSampleEncrypter();

    // methods
    virtual AP4_Result EncryptSampleData(AP4_DataBuffer& data_in,
                                         AP4_DataBuffer& data_out);

protected:
    // members
    AP4_CbcStreamCipher* m_Cipher;
};

/*----------------------------------------------------------------------
|   AP4_PiffAvcCbcSampleEncrypter
+---------------------------------------------------------------------*/
class AP4_PiffAvcCbcSampleEncrypter : public AP4_PiffCbcSampleEncrypter
{
public:
    // constructor and destructor
    AP4_PiffAvcCbcSampleEncrypter(AP4_BlockCipher* block_cipher,
                                  AP4_Size         nalu_length_size) :
        AP4_PiffCbcSampleEncrypter(block_cipher),
        m_NaluLengthSize(nalu_length_size) {}

    // methods
    virtual AP4_Result EncryptSampleData(AP4_DataBuffer& data_in,
                                         AP4_DataBuffer& data_out);

private:
    // members
    AP4_Size m_NaluLengthSize;
};

/*----------------------------------------------------------------------
|   AP4_PiffEncryptingProcessor
+---------------------------------------------------------------------*/
class AP4_PiffEncryptingProcessor : public AP4_Processor
{
public:
    // types
    struct Encrypter {
        Encrypter(AP4_UI32 track_id, AP4_PiffSampleEncrypter* sample_encrypter) :
            m_TrackId(track_id),
            m_SampleEncrypter(sample_encrypter) {}
        Encrypter() { delete m_SampleEncrypter; }
        AP4_UI32                 m_TrackId;
        AP4_PiffSampleEncrypter* m_SampleEncrypter;
    };

    // constructor
    AP4_PiffEncryptingProcessor(AP4_PiffCipherMode      cipher_mode,
                                AP4_BlockCipherFactory* block_cipher_factory = NULL);
    ~AP4_PiffEncryptingProcessor();

    // accessors
    AP4_ProtectionKeyMap& GetKeyMap()      { return m_KeyMap;      }
    AP4_TrackPropertyMap& GetPropertyMap() { return m_PropertyMap; }

    // AP4_Processor methods
    virtual AP4_Result Initialize(AP4_AtomParent&   top_level,
                                  AP4_ByteStream&   stream,
                                  AP4_Processor::ProgressListener* listener = NULL);
    virtual AP4_Processor::TrackHandler*    CreateTrackHandler(AP4_TrakAtom* trak);
    virtual AP4_Processor::FragmentHandler* CreateFragmentHandler(AP4_ContainerAtom* traf);
    
protected:    
    // members
    AP4_PiffCipherMode      m_CipherMode;
    AP4_BlockCipherFactory* m_BlockCipherFactory;
    AP4_ProtectionKeyMap    m_KeyMap;
    AP4_TrackPropertyMap    m_PropertyMap;
    AP4_List<Encrypter>     m_Encrypters;
};

/*----------------------------------------------------------------------
|   AP4_PiffSampleDecrypter
+---------------------------------------------------------------------*/
class AP4_PiffSampleDecrypter : public AP4_SampleDecrypter
{
public:
    // factory
    static AP4_Result Create(AP4_ProtectedSampleDescription* sample_description, 
                             AP4_ContainerAtom*              traf,
                             const AP4_UI08*                 key, 
                             AP4_Size                        key_size,
                             AP4_BlockCipherFactory*         block_cipher_factory,
                             AP4_PiffSampleDecrypter*&       decrypter);

    // methods
    virtual AP4_Result SetSampleIndex(AP4_Ordinal sample_index);

protected:
    AP4_PiffSampleDecrypter(AP4_PiffSampleEncryptionAtom* sample_encryption_atom) :
        m_SampleEncryptionAtom(sample_encryption_atom),
        m_SampleIndex(0) {}
    AP4_PiffSampleEncryptionAtom* m_SampleEncryptionAtom;
    AP4_Ordinal                   m_SampleIndex;
};

/*----------------------------------------------------------------------
|   AP4_PiffNullSampleDecrypter
+---------------------------------------------------------------------*/
class AP4_PiffNullSampleDecrypter : public AP4_PiffSampleDecrypter
{
public:
    // constructor
    AP4_PiffNullSampleDecrypter() : AP4_PiffSampleDecrypter(NULL) {}
    
    // methods
    virtual AP4_Result DecryptSampleData(AP4_DataBuffer& data_in,
                                         AP4_DataBuffer& data_out,
                                         const AP4_UI08* /*iv = NULL*/) {
        data_out.SetData(data_in.GetData(), data_in.GetDataSize());
        return AP4_SUCCESS;
    }
};

/*----------------------------------------------------------------------
|   AP4_PiffCtrSampleDecrypter
+---------------------------------------------------------------------*/
class AP4_PiffCtrSampleDecrypter : public AP4_PiffSampleDecrypter
{
public:
    // constructor and destructor
    AP4_PiffCtrSampleDecrypter(AP4_BlockCipher*              block_cipher,
                               AP4_Size                      iv_size,
                               AP4_PiffSampleEncryptionAtom* sample_encryption_atom);
    ~AP4_PiffCtrSampleDecrypter();

    // methods
    virtual AP4_Result DecryptSampleData(AP4_DataBuffer& data_in,
                                         AP4_DataBuffer& data_out,
                                         const AP4_UI08* iv = NULL);

private:
    // members
    AP4_CtrStreamCipher* m_Cipher;
    AP4_Size             m_IvSize;
};

/*----------------------------------------------------------------------
|   AP4_PiffAvcCtrSampleDecrypter
+---------------------------------------------------------------------*/
class AP4_PiffAvcCtrSampleDecrypter : public AP4_PiffCtrSampleDecrypter
{
public:
    // constructor and destructor
    AP4_PiffAvcCtrSampleDecrypter(AP4_BlockCipher*              block_cipher,
                                  AP4_Size                      iv_size,
                                  AP4_PiffSampleEncryptionAtom* sample_encryption_atom,
                                  AP4_Size                      nalu_length_size) :
        AP4_PiffCtrSampleDecrypter(block_cipher, iv_size, sample_encryption_atom),
        m_NaluLengthSize(nalu_length_size) {}

    // methods
    virtual AP4_Result DecryptSampleData(AP4_DataBuffer& data_in,
                                         AP4_DataBuffer& data_out,
                                         const AP4_UI08* iv = NULL);

private:
    // members
    AP4_Size m_NaluLengthSize;
};

/*----------------------------------------------------------------------
|   AP4_PiffCbcSampleDecrypter
+---------------------------------------------------------------------*/
class AP4_PiffCbcSampleDecrypter : public AP4_PiffSampleDecrypter
{
public:
    // constructor and destructor
    AP4_PiffCbcSampleDecrypter(AP4_BlockCipher*              block_cipher,
                               AP4_PiffSampleEncryptionAtom* sample_encryption_atom);
    ~AP4_PiffCbcSampleDecrypter();

    // methods
    virtual AP4_Result DecryptSampleData(AP4_DataBuffer& data_in,
                                         AP4_DataBuffer& data_out,
                                         const AP4_UI08* iv = NULL);

protected:
    // members
    AP4_CbcStreamCipher* m_Cipher;
};

/*----------------------------------------------------------------------
|   AP4_PiffAvcCbcSampleDecrypter
+---------------------------------------------------------------------*/
class AP4_PiffAvcCbcSampleDecrypter : public AP4_PiffCbcSampleDecrypter
{
public:
    // constructor and destructor
    AP4_PiffAvcCbcSampleDecrypter(AP4_BlockCipher*              block_cipher,
                                  AP4_PiffSampleEncryptionAtom* sample_encryption_atom,
                                  AP4_Size                      nalu_length_size) :
        AP4_PiffCbcSampleDecrypter(block_cipher, sample_encryption_atom),
        m_NaluLengthSize(nalu_length_size) {}

    // methods
    virtual AP4_Result DecryptSampleData(AP4_DataBuffer& data_in,
                                         AP4_DataBuffer& data_out,
                                         const AP4_UI08* iv = NULL);

private:
    // members
    AP4_Size m_NaluLengthSize;
};

/*----------------------------------------------------------------------
|   AP4_PiffTrackEncryptionAtom
+---------------------------------------------------------------------*/
class AP4_PiffTrackEncryptionAtom : public AP4_UuidAtom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_PiffTrackEncryptionAtom, AP4_UuidAtom)

    // class methods
    static AP4_PiffTrackEncryptionAtom* Create(AP4_Size        size, 
                                               AP4_ByteStream& stream);

    // constructors
    AP4_PiffTrackEncryptionAtom(AP4_UI32        default_algorithm_id,
                                AP4_UI08        default_iv_size,
                                const AP4_UI08* default_kid);

    // methods
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    // accessors
    AP4_UI32        GetDefaultAlgorithmId() { return m_DefaultAlgorithmId; }
    AP4_UI08        GetDefaultIvSize()      { return m_DefaultIvSize;      }
    const AP4_UI08* GetDefaultKid()         { return m_DefaultKid;         }

private:
    // methods
    AP4_PiffTrackEncryptionAtom(AP4_UI32        size, 
                                AP4_UI32        version,
                                AP4_UI32        flags,
                                AP4_ByteStream& stream);

    // members
    AP4_UI32 m_DefaultAlgorithmId;
    AP4_UI08 m_DefaultIvSize;
    AP4_UI08 m_DefaultKid[16];
};

/*----------------------------------------------------------------------
|   AP4_PiffSampleEncryptionAtom
+---------------------------------------------------------------------*/
class AP4_PiffSampleEncryptionAtom : public AP4_UuidAtom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_PiffSampleEncryptionAtom, AP4_UuidAtom)

    // class methods
    static AP4_PiffSampleEncryptionAtom* Create(AP4_Size        size, 
                                                AP4_ByteStream& stream);

    // constructors
    AP4_PiffSampleEncryptionAtom(AP4_Cardinal sample_count);
    AP4_PiffSampleEncryptionAtom(AP4_UI32        algorithm_id,
                                 AP4_UI08        iv_size,
                                 const AP4_UI08* kid,
                                 AP4_Cardinal    sample_count);

    // methods
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    // accessors
    AP4_UI32        GetAlgorithmId()        { return m_AlgorithmId; }
    AP4_UI08        GetIvSize()             { return m_IvSize;      }
    AP4_Result      SetIvSize(AP4_UI08 iv_size);
    const AP4_UI08* GetKid()                { return m_Kid;         }
    AP4_Cardinal    GetIvCount()            { return m_IvCount;     }
    const AP4_UI08* GetIv(AP4_Ordinal indx);
    AP4_Result      SetIv(AP4_Ordinal indx, const AP4_UI08* iv);

private:
    // methods
    AP4_PiffSampleEncryptionAtom(AP4_UI32        size, 
                                 AP4_UI32        version,
                                 AP4_UI32        flags,
                                 AP4_ByteStream& stream);

    // members
    AP4_UI32       m_AlgorithmId;
    AP4_UI08       m_IvSize;
    AP4_UI08       m_Kid[16];
    AP4_Cardinal   m_IvCount;
    AP4_DataBuffer m_Ivs;
};

#endif // _AP4_PIFF_H_
