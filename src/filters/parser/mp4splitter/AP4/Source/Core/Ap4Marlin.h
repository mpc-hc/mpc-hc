/*****************************************************************
|
|    AP4 - Marlin File Format support
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

#ifndef _AP4_MARLIN_H_
#define _AP4_MARLIN_H_

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
#include "Ap4TrefTypeAtom.h"
#include "Ap4ObjectDescriptor.h"
#include "Ap4Command.h"
#include "Ap4UuidAtom.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI32 AP4_MARLIN_BRAND_MGSV       = AP4_ATOM_TYPE('M','G','S','V');
const AP4_UI16 AP4_MARLIN_IPMPS_TYPE_MGSV  = 0xA551;
const AP4_UI32 AP4_PROTECTION_SCHEME_TYPE_MARLIN_ACBC = AP4_ATOM_TYPE('A','C','B','C');
const AP4_UI32 AP4_PROTECTION_SCHEME_TYPE_MARLIN_ACGK = AP4_ATOM_TYPE('A','C','G','K');

const AP4_Atom::Type AP4_ATOM_TYPE_SATR = AP4_ATOM_TYPE('s','a','t','r');
const AP4_Atom::Type AP4_ATOM_TYPE_STYP = AP4_ATOM_TYPE('s','t','y','p');
const AP4_Atom::Type AP4_ATOM_TYPE_HMAC = AP4_ATOM_TYPE('h','m','a','c');
const AP4_Atom::Type AP4_ATOM_TYPE_GKEY = AP4_ATOM_TYPE('g','k','e','y');

const char* const AP4_MARLIN_IPMP_STYP_VIDEO = "urn:marlin:organization:sne:content-type:video";
const char* const AP4_MARLIN_IPMP_STYP_AUDIO = "urn:marlin:organization:sne:content-type:audio";

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpParser
+---------------------------------------------------------------------*/
class AP4_MarlinIpmpParser
{
public:
    // types
    struct SinfEntry {
         SinfEntry(AP4_UI32 track_id, AP4_ContainerAtom* sinf) :
             m_TrackId(track_id), m_Sinf(sinf) {}
        ~SinfEntry() { delete m_Sinf; }
        AP4_UI32           m_TrackId;
        AP4_ContainerAtom* m_Sinf;
    };
    
    // methods
    static AP4_Result Parse(AP4_AtomParent&      top_level, 
                            AP4_ByteStream&      stream,
                            AP4_List<SinfEntry>& sinf_entries,
                            bool                 remove_od_data=false);
    
private:
    AP4_MarlinIpmpParser() {} // this class can't be instantiated
};

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpDecryptingProcessor
+---------------------------------------------------------------------*/
class AP4_MarlinIpmpDecryptingProcessor : public AP4_Processor
{
public:
    // constructor and destructor
    AP4_MarlinIpmpDecryptingProcessor(const AP4_ProtectionKeyMap* key_map = NULL,
                                      AP4_BlockCipherFactory*     block_cipher_factory = NULL);
    ~AP4_MarlinIpmpDecryptingProcessor();
    
    // accessors
    AP4_ProtectionKeyMap& GetKeyMap() { return m_KeyMap; }

    // methods
    virtual AP4_Result Initialize(AP4_AtomParent&   top_level,
                                  AP4_ByteStream&   stream,
                                  ProgressListener* listener);
    virtual AP4_Processor::TrackHandler* CreateTrackHandler(AP4_TrakAtom* trak);

private:
    
    // members
    AP4_BlockCipherFactory*                   m_BlockCipherFactory;
    AP4_ProtectionKeyMap                      m_KeyMap;
    AP4_List<AP4_MarlinIpmpParser::SinfEntry> m_SinfEntries;
};

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpTrackDecrypter
+---------------------------------------------------------------------*/
class AP4_MarlinIpmpTrackDecrypter : public AP4_Processor::TrackHandler
{
public:
    // class methods
    static AP4_Result Create(AP4_BlockCipherFactory&        cipher_factory,
                             const AP4_UI08*                key,
                             AP4_MarlinIpmpTrackDecrypter*& decrypter);
                             
    // destructor
    ~AP4_MarlinIpmpTrackDecrypter();
    
    // AP4_Processor::TrackHandler methods
    virtual AP4_Size GetProcessedSampleSize(AP4_Sample& sample);
    virtual AP4_Result ProcessSample(AP4_DataBuffer& data_in,
                                     AP4_DataBuffer& data_out);


private:
    // constructor
    AP4_MarlinIpmpTrackDecrypter(AP4_StreamCipher* cipher) : m_Cipher(cipher) {}

    // members
    AP4_StreamCipher* m_Cipher;
};

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpEncryptingProcessor
+---------------------------------------------------------------------*/
class AP4_MarlinIpmpEncryptingProcessor : public AP4_Processor
{
public:
    // constructor
    AP4_MarlinIpmpEncryptingProcessor(bool                        use_group_key = false,
                                      const AP4_ProtectionKeyMap* key_map = NULL,
                                      AP4_BlockCipherFactory*     block_cipher_factory = NULL);

    // accessors
    AP4_ProtectionKeyMap& GetKeyMap()      { return m_KeyMap;      }
    AP4_TrackPropertyMap& GetPropertyMap() { return m_PropertyMap; }

    // AP4_Processor methods
    virtual AP4_Result Initialize(AP4_AtomParent&   top_level,
                                  AP4_ByteStream&   stream,
                                  AP4_Processor::ProgressListener* listener = NULL);
    virtual AP4_Processor::TrackHandler* CreateTrackHandler(AP4_TrakAtom* trak);

private:
    // members
    AP4_BlockCipherFactory* m_BlockCipherFactory;
    bool                    m_UseGroupKey;
    AP4_ProtectionKeyMap    m_KeyMap;
    AP4_TrackPropertyMap    m_PropertyMap;
};

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpTrackEncrypter
+---------------------------------------------------------------------*/
class AP4_MarlinIpmpTrackEncrypter : public AP4_Processor::TrackHandler
{
public:
    // class methods
    static AP4_Result Create(AP4_BlockCipherFactory&        cipher_factory,
                             const AP4_UI08*                key,
                             const AP4_UI08*                iv,
                             AP4_MarlinIpmpTrackEncrypter*& encrypter);
                             
    // destructor
    ~AP4_MarlinIpmpTrackEncrypter();
    
    // AP4_Processor::TrackHandler methods
    virtual AP4_Size GetProcessedSampleSize(AP4_Sample& sample);
    virtual AP4_Result ProcessSample(AP4_DataBuffer& data_in,
                                     AP4_DataBuffer& data_out);


private:
    // constructor
    AP4_MarlinIpmpTrackEncrypter(AP4_StreamCipher* cipher, const AP4_UI08* iv);

    // members
    AP4_UI08          m_IV[16];
    AP4_StreamCipher* m_Cipher;
};

#endif // _AP4_MARLIN_H_
