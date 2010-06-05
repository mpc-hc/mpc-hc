/*****************************************************************
|
|    AP4 - Protected Streams support
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

#ifndef _AP4_PROTECTION_H_
#define _AP4_PROTECTION_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4SampleEntry.h"
#include "Ap4Atom.h"
#include "Ap4AtomFactory.h"
#include "Ap4SampleDescription.h"
#include "Ap4Processor.h"

/*----------------------------------------------------------------------
|   classes
+---------------------------------------------------------------------*/
class AP4_StreamCipher;

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
// this is fixed for now
const unsigned int AP4_PROTECTION_KEY_LENGTH = 16;

const AP4_UI32 AP4_PROTECTION_SCHEME_TYPE_ITUNES = AP4_ATOM_TYPE('i','t','u','n');

/*----------------------------------------------------------------------
|   AP4_EncaSampleEntry
+---------------------------------------------------------------------*/
class AP4_EncaSampleEntry : public AP4_AudioSampleEntry
{
public:
    // methods
    AP4_EncaSampleEntry(AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);
    AP4_EncaSampleEntry(AP4_UI32         type,
                        AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);

    // methods
    AP4_SampleDescription* ToSampleDescription();

    // this method is used as a factory by the ISMACryp classes
    // NOTE: this should be named ToSampleDescription, but C++ has a 
    // problem with that because the base class does not have this
    // overloaded method, but has another other one by that name
    virtual AP4_SampleDescription* ToTargetSampleDescription(AP4_UI32 format);
};

/*----------------------------------------------------------------------
|   AP4_EncvSampleEntry
+---------------------------------------------------------------------*/
class AP4_EncvSampleEntry : public AP4_VisualSampleEntry
{
public:
    // constructors
    AP4_EncvSampleEntry(AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);
    AP4_EncvSampleEntry(AP4_UI32         type,
                        AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);

    // methods
    AP4_SampleDescription* ToSampleDescription();

    // this method is used as a factory by the ISMACryp classes
    // NOTE: this should be named ToSampleDescription, but C++ has a 
    // problem with that because the base class does not have this
    // overloaded method, but has another other one by that name
    virtual AP4_SampleDescription* ToTargetSampleDescription(AP4_UI32 format);
};

/*----------------------------------------------------------------------
|   AP4_DrmsSampleEntry
+---------------------------------------------------------------------*/
class AP4_DrmsSampleEntry : public AP4_EncaSampleEntry
{
public:
    // methods
    AP4_DrmsSampleEntry(AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);
};

/*----------------------------------------------------------------------
|   AP4_DrmiSampleEntry
+---------------------------------------------------------------------*/
class AP4_DrmiSampleEntry : public AP4_EncvSampleEntry
{
public:
    // methods
    AP4_DrmiSampleEntry(AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);
};

/*----------------------------------------------------------------------
|   AP4_ProtectionKeyMap
+---------------------------------------------------------------------*/
class AP4_ProtectionKeyMap
{
public:
    // constructors and destructor
    AP4_ProtectionKeyMap();
    ~AP4_ProtectionKeyMap();

    // methods
    AP4_Result      SetKey(AP4_UI32 track_id, const AP4_UI08* key, const AP4_UI08* iv = NULL);
    AP4_Result      SetKeys(const AP4_ProtectionKeyMap& key_map);
    AP4_Result      GetKeyAndIv(AP4_UI32 track_id, const AP4_UI08*& key, const AP4_UI08*& iv);
    const AP4_UI08* GetKey(AP4_UI32 track_id) const;

private:
    // types
    class KeyEntry {
    public:
        KeyEntry(AP4_UI32 track_id, const AP4_UI08* key, const AP4_UI08* iv = NULL);
        void SetKey(const AP4_UI08* key, const AP4_UI08* iv);
        AP4_Ordinal m_TrackId;
        AP4_UI08    m_Key[AP4_PROTECTION_KEY_LENGTH];
        AP4_UI08    m_IV[AP4_PROTECTION_KEY_LENGTH];
    };

    // methods
    KeyEntry* GetEntry(AP4_UI32 track_id) const;

    // members
    AP4_List<KeyEntry> m_KeyEntries;
};

/*----------------------------------------------------------------------
|   AP4_TrackPropertyMap
+---------------------------------------------------------------------*/
class AP4_TrackPropertyMap
{
public:
    // methods
    AP4_Result  SetProperty(AP4_UI32 track_id, const char* name, const char* value);
    AP4_Result  SetProperties(const AP4_TrackPropertyMap& properties);
    const char* GetProperty(AP4_UI32 track_id, const char* name);
    AP4_Result  GetTextualHeaders(AP4_UI32 track_id, AP4_DataBuffer& buffer);
    

    // destructor
    virtual ~AP4_TrackPropertyMap();

private:
    // types
    class Entry {
    public:
        Entry(AP4_UI32 track_id, const char* name, const char* value) :
          m_TrackId(track_id), m_Name(name), m_Value(value) {}
        AP4_UI32   m_TrackId;
        AP4_String m_Name;
        AP4_String m_Value;
    };

    // members
    AP4_List<Entry> m_Entries;
};

/*----------------------------------------------------------------------
|   AP4_ProtectionSchemeInfo
+---------------------------------------------------------------------*/
class AP4_ProtectionSchemeInfo
{
public:
    // constructors and destructor
    AP4_ProtectionSchemeInfo(AP4_ContainerAtom* schi);
    virtual ~AP4_ProtectionSchemeInfo();

    // accessors
    AP4_ContainerAtom* GetSchiAtom() { return m_SchiAtom; }

protected:
    AP4_ContainerAtom* m_SchiAtom;
};

/*----------------------------------------------------------------------
|   AP4_ProtectedSampleDescription
+---------------------------------------------------------------------*/
class AP4_ProtectedSampleDescription : public AP4_SampleDescription
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_ProtectedSampleDescription, AP4_SampleDescription)

    // constructor and destructor
    AP4_ProtectedSampleDescription(AP4_UI32               format,
                                   AP4_SampleDescription* original_sample_description,
                                   AP4_UI32               original_format,
                                   AP4_UI32               scheme_type,
                                   AP4_UI32               scheme_version,
                                   const char*            scheme_uri,
                                   AP4_ContainerAtom*     schi_atom, // will be cloned
                                   bool                   transfer_ownership_of_original=true);
    ~AP4_ProtectedSampleDescription();
    
    // accessors
    AP4_SampleDescription* GetOriginalSampleDescription() {
        return m_OriginalSampleDescription;
    }
    AP4_UI32          GetOriginalFormat() const { return m_OriginalFormat; }
    AP4_UI32          GetSchemeType()     const { return m_SchemeType;     }
    AP4_UI32          GetSchemeVersion()  const { return m_SchemeVersion;  }
    const AP4_String& GetSchemeUri()      const { return m_SchemeUri;      }
    AP4_ProtectionSchemeInfo* GetSchemeInfo() const { 
        return m_SchemeInfo; 
    }

    // implementation of abstract base class methods
    virtual AP4_Atom* ToAtom() const;

private:
    // members
    AP4_SampleDescription*    m_OriginalSampleDescription;
    bool                      m_OriginalSampleDescriptionIsOwned;
    AP4_UI32                  m_OriginalFormat;
    AP4_UI32                  m_SchemeType;
    AP4_UI32                  m_SchemeVersion;
    AP4_String                m_SchemeUri;
    AP4_ProtectionSchemeInfo* m_SchemeInfo;
};

/*----------------------------------------------------------------------
|   AP4_BlockCipher
+---------------------------------------------------------------------*/
class AP4_BlockCipher
{
public:
    // types
    typedef enum {
        ENCRYPT,
        DECRYPT
    } CipherDirection;

    typedef enum {
        AES_128
    } CipherType;

    // constructor and destructor
    virtual ~AP4_BlockCipher() {}
    
    // methods
    virtual AP4_Result ProcessBlock(const AP4_UI08* block_in, AP4_UI08* block_out) = 0;
};

/*----------------------------------------------------------------------
|   AP4_BlockCipherFactory
+---------------------------------------------------------------------*/
class AP4_BlockCipherFactory
{
public:
    // methods
    virtual ~AP4_BlockCipherFactory() {}
    virtual AP4_Result Create(AP4_BlockCipher::CipherType      type,
                              AP4_BlockCipher::CipherDirection direction,
                              const AP4_UI08*                  key,
                              AP4_Size                         key_size,
                              AP4_BlockCipher*&                cipher) = 0;
};

/*----------------------------------------------------------------------
|   AP4_DefaultBlockCipherFactory
+---------------------------------------------------------------------*/
class AP4_DefaultBlockCipherFactory : public AP4_BlockCipherFactory
{
public:
    // class variables
    static AP4_DefaultBlockCipherFactory Instance;

    // methods
    virtual AP4_Result Create(AP4_BlockCipher::CipherType      type,
                              AP4_BlockCipher::CipherDirection direction,
                              const AP4_UI08*                  key,
                              AP4_Size                         key_size,
                              AP4_BlockCipher*&                cipher);
};

/*----------------------------------------------------------------------
|   AP4_SampleDecrypter
+---------------------------------------------------------------------*/
class AP4_SampleDecrypter
{
public:
    /**
     * Create a sample decrypter given a protected sample description
     */
    static AP4_SampleDecrypter* Create(AP4_ProtectedSampleDescription* sample_description,
                                       const AP4_UI08*                 key,
                                       AP4_Size                        key_size,
                                       AP4_BlockCipherFactory*         block_cipher_factory = NULL);

    /**
     * Create a fragment sample decrypter given a protected sample description and a track fragment
     */
    static AP4_SampleDecrypter* Create(AP4_ProtectedSampleDescription* sample_description,
                                       AP4_ContainerAtom*              traf,
                                       const AP4_UI08*                 key,
                                       AP4_Size                        key_size,
                                       AP4_BlockCipherFactory*         block_cipher_factory = NULL);

    // destructor
    virtual ~AP4_SampleDecrypter() {}

    // methods
    virtual AP4_Size   GetDecryptedSampleSize(AP4_Sample& sample) { return sample.GetSize(); }
    virtual AP4_Result SetSampleIndex(AP4_Ordinal /*index*/)      { return AP4_SUCCESS;      }
    virtual AP4_Result DecryptSampleData(AP4_DataBuffer&    data_in,
                                         AP4_DataBuffer&    data_out,
                                         const AP4_UI08*    iv = NULL) = 0;
};

/*----------------------------------------------------------------------
|   AP4_StandardDecryptingProcessor
+---------------------------------------------------------------------*/
class AP4_StandardDecryptingProcessor : public AP4_Processor
{
public:
    // constructor
    AP4_StandardDecryptingProcessor(const AP4_ProtectionKeyMap* key_map = NULL,
                                    AP4_BlockCipherFactory*     block_cipher_factory = NULL);

    // accessors
    AP4_ProtectionKeyMap& GetKeyMap() { return m_KeyMap; }
    
    // methods
    virtual AP4_Result Initialize(AP4_AtomParent&   top_level,
                                  AP4_ByteStream&   stream,
                                  ProgressListener* listener);
    virtual AP4_Processor::TrackHandler* CreateTrackHandler(AP4_TrakAtom* trak);

private:
    // members
    AP4_BlockCipherFactory* m_BlockCipherFactory;
    AP4_ProtectionKeyMap    m_KeyMap;
};

/*----------------------------------------------------------------------
|   AP4_DecryptingStream
+---------------------------------------------------------------------*/
class AP4_DecryptingStream : public AP4_ByteStream 
{
public:
    typedef enum {
        CIPHER_MODE_CTR,
        CIPHER_MODE_CBC
    } CipherMode;

    static AP4_Result Create(CipherMode              mode,
                             AP4_ByteStream&         encrypted_stream,
                             AP4_LargeSize           cleartext_size,
                             const AP4_UI08*         iv,
                             AP4_Size                iv_size,
                             const AP4_UI08*         key,
                             AP4_Size                key_size,
                             AP4_BlockCipherFactory* block_cipher_factory,
                             AP4_ByteStream*&        stream);

    // AP4_ByteStream methods
    virtual AP4_Result ReadPartial(void*     buffer, 
                                   AP4_Size  bytes_to_read, 
                                   AP4_Size& bytes_read);
    virtual AP4_Result WritePartial(const void* buffer, 
                                    AP4_Size    bytes_to_write, 
                                    AP4_Size&   bytes_written);
    virtual AP4_Result Seek(AP4_Position position);
    virtual AP4_Result Tell(AP4_Position& position);
    virtual AP4_Result GetSize(AP4_LargeSize& size);

    // AP4_Referenceable methods
    virtual void AddReference();
    virtual void Release();

private:
    // methods
    AP4_DecryptingStream() {} // private constructor, use the factory instead
    ~AP4_DecryptingStream();

    // members
    CipherMode        m_Mode;
    AP4_LargeSize     m_CleartextSize;
    AP4_Position      m_CleartextPosition;
    AP4_ByteStream*   m_EncryptedStream;
    AP4_LargeSize     m_EncryptedSize;
    AP4_Position      m_EncryptedPosition;
    AP4_StreamCipher* m_StreamCipher;
    AP4_UI08          m_Buffer[16];
    AP4_Size          m_BufferFullness;
    AP4_Size          m_BufferOffset;
    AP4_Cardinal      m_ReferenceCount;
};

/*----------------------------------------------------------------------
|   AP4_EncryptingStream
+---------------------------------------------------------------------*/
class AP4_EncryptingStream : public AP4_ByteStream 
{
public:
    typedef enum {
        CIPHER_MODE_CTR,
        CIPHER_MODE_CBC
    } CipherMode;

    static AP4_Result Create(CipherMode              mode,
                             AP4_ByteStream&         cleartext_stream,
                             const AP4_UI08*         iv,
                             AP4_Size                iv_size,
                             const AP4_UI08*         key,
                             AP4_Size                key_size,
                             bool                    prepend_iv,
                             AP4_BlockCipherFactory* block_cipher_factory,
                             AP4_ByteStream*&        stream);

    // AP4_ByteStream methods
    virtual AP4_Result ReadPartial(void*     buffer, 
                                   AP4_Size  bytes_to_read, 
                                   AP4_Size& bytes_read);
    virtual AP4_Result WritePartial(const void* buffer, 
                                    AP4_Size    bytes_to_write, 
                                    AP4_Size&   bytes_written);
    virtual AP4_Result Seek(AP4_Position position);
    virtual AP4_Result Tell(AP4_Position& position);
    virtual AP4_Result GetSize(AP4_LargeSize& size);

    // AP4_Referenceable methods
    virtual void AddReference();
    virtual void Release();

private:
    // methods
    AP4_EncryptingStream() {} // private constructor, use the factory instead
    ~AP4_EncryptingStream();

    // members
    CipherMode        m_Mode;
    AP4_LargeSize     m_CleartextSize;
    AP4_Position      m_CleartextPosition;
    AP4_ByteStream*   m_CleartextStream;
    AP4_LargeSize     m_EncryptedSize;
    AP4_Position      m_EncryptedPosition;
    AP4_StreamCipher* m_StreamCipher;
    AP4_UI08          m_Buffer[32]; // one cipher block plus one block padding
    AP4_Size          m_BufferFullness;
    AP4_Size          m_BufferOffset;
    AP4_Cardinal      m_ReferenceCount;
};

#endif // _AP4_PROTECTION_H_
