/*****************************************************************
|
|    AP4 - Marlin File Format Support
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
#include "Ap4SchmAtom.h"
#include "Ap4StsdAtom.h"
#include "Ap4Sample.h"
#include "Ap4StreamCipher.h"
#include "Ap4FrmaAtom.h"
#include "Ap4Utils.h"
#include "Ap4TrakAtom.h"
#include "Ap4FtypAtom.h"
#include "Ap4IodsAtom.h"
#include "Ap4MoovAtom.h"
#include "Ap4Track.h"
#include "Ap4DescriptorFactory.h"
#include "Ap4CommandFactory.h"
#include "Ap4Marlin.h"
#include "Ap4FileByteStream.h"
#include "Ap4Ipmp.h"
#include "Ap4AesBlockCipher.h"
#include "Ap4SyntheticSampleTable.h"
#include "Ap4HdlrAtom.h"
#include "Ap4Hmac.h"
#include "Ap4KeyWrap.h"
#include "Ap4ByteStream.h"

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpAtomTypeHandler
+---------------------------------------------------------------------*/
class AP4_MarlinIpmpAtomTypeHandler : public AP4_AtomFactory::TypeHandler
{
public:
    // constructor
    AP4_MarlinIpmpAtomTypeHandler(AP4_AtomFactory* atom_factory) :
        m_AtomFactory(atom_factory) {}
    virtual AP4_Result CreateAtom(AP4_Atom::Type  type,
                                  AP4_UI32        size,
                                  AP4_ByteStream& stream,
                                  AP4_Atom::Type  context,
                                  AP4_Atom*&      atom);

private:
    // members
    AP4_AtomFactory* m_AtomFactory;
};

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpAtomFactory
+---------------------------------------------------------------------*/
class AP4_MarlinIpmpAtomFactory : public AP4_DefaultAtomFactory
{
public:
    // class members
    static AP4_MarlinIpmpAtomFactory Instance;

    // constructor
    AP4_MarlinIpmpAtomFactory()
    {
        AddTypeHandler(new AP4_MarlinIpmpAtomTypeHandler(this));
    }
};

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpAtomFactory::Instance
+---------------------------------------------------------------------*/
AP4_MarlinIpmpAtomFactory AP4_MarlinIpmpAtomFactory::Instance;

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpAtomTypeHandler::CreateAtom
+---------------------------------------------------------------------*/
AP4_Result
AP4_MarlinIpmpAtomTypeHandler::CreateAtom(AP4_Atom::Type  type,
        AP4_UI32        size,
        AP4_ByteStream& stream,
        AP4_Atom::Type  /*context*/,
        AP4_Atom*&      atom)
{
    switch(type)
    {
    case AP4_ATOM_TYPE_SATR:
        atom = AP4_ContainerAtom::Create(type, size, false, false, stream, *m_AtomFactory);
        break;

    case AP4_ATOM_TYPE_STYP:
        atom = new AP4_NullTerminatedStringAtom(type, size, stream);
        break;

    default:
        atom = NULL;
    }

    return atom ? AP4_SUCCESS : AP4_FAILURE;
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpParser:Parse
+---------------------------------------------------------------------*/
AP4_Result
AP4_MarlinIpmpParser::Parse(AP4_AtomParent&      top_level,
                            AP4_ByteStream&      stream,
                            AP4_List<SinfEntry>& sinf_entries,
                            bool                 remove_od_data)
{
    // check the file type
    AP4_FtypAtom* ftyp = AP4_DYNAMIC_CAST(AP4_FtypAtom, top_level.GetChild(AP4_ATOM_TYPE_FTYP));
    if(ftyp == NULL ||
       (ftyp->GetMajorBrand() != AP4_MARLIN_BRAND_MGSV && !ftyp->HasCompatibleBrand(AP4_MARLIN_BRAND_MGSV)))
    {
        return AP4_ERROR_INVALID_FORMAT;
    }

    // check the initial object descriptor and get the OD Track ID
    AP4_IodsAtom* iods = AP4_DYNAMIC_CAST(AP4_IodsAtom, top_level.FindChild("moov/iods"));
    AP4_UI32      od_track_id = 0;
    if(iods == NULL) return AP4_ERROR_INVALID_FORMAT;
    const AP4_ObjectDescriptor* od = iods->GetObjectDescriptor();
    if(od == NULL) return AP4_ERROR_INVALID_FORMAT;
    AP4_EsIdIncDescriptor* es_id_inc = AP4_DYNAMIC_CAST(AP4_EsIdIncDescriptor, od->FindSubDescriptor(AP4_DESCRIPTOR_TAG_ES_ID_INC));
    if(es_id_inc == NULL) return AP4_ERROR_INVALID_FORMAT;
    od_track_id = es_id_inc->GetTrackId();

    // find the track pointed to by the descriptor
    AP4_MoovAtom* moov = AP4_DYNAMIC_CAST(AP4_MoovAtom, top_level.GetChild(AP4_ATOM_TYPE_MOOV));
    if(moov == NULL) return AP4_ERROR_INVALID_FORMAT;
    AP4_TrakAtom* od_trak = NULL;
    for(AP4_List<AP4_TrakAtom>::Item* trak_item = moov->GetTrakAtoms().FirstItem();
        trak_item;
        trak_item = trak_item->GetNext())
    {
        AP4_TrakAtom* trak = trak_item->GetData();
        if(trak)
        {
            if(trak->GetId() == od_track_id)
            {
                od_trak = trak;
            }
            else
            {
                sinf_entries.Add(new SinfEntry(trak->GetId(), NULL));
            }
        }
    }

    // check that we have found the OD track
    if(od_trak == NULL) return AP4_ERROR_INVALID_FORMAT;

    // look for the 'mpod' trak references
    AP4_TrefTypeAtom* track_references;
    track_references = AP4_DYNAMIC_CAST(AP4_TrefTypeAtom, od_trak->FindChild("tref/mpod"));
    if(track_references == NULL) return AP4_ERROR_INVALID_FORMAT;

    // create an AP4_Track object from the trak atom and check that it has samples
    AP4_Track* od_track = new AP4_Track(*od_trak, stream, 0);
    if(od_track->GetSampleCount() < 1)
    {
        delete od_track;
        return AP4_ERROR_INVALID_FORMAT;
    }

    // get the first sample (in this version, we only look at a single OD command)
    AP4_Sample od_sample;
    AP4_Result result = od_track->GetSample(0, od_sample);
    if(AP4_FAILED(result))
    {
        delete od_track;
        return AP4_ERROR_INVALID_FORMAT;
    }

    // adapt the sample data into a byte stream for parsing
    AP4_DataBuffer sample_data;
    od_sample.ReadData(sample_data);
    AP4_MemoryByteStream* sample_stream = new AP4_MemoryByteStream(sample_data);

    // look for one ObjectDescriptorUpdate command and
    // one IPMP_DescriptorUpdate command
    AP4_DescriptorUpdateCommand* od_update = NULL;
    AP4_DescriptorUpdateCommand* ipmp_update = NULL;
    do
    {
        AP4_Command* command = NULL;
        result = AP4_CommandFactory::CreateCommandFromStream(*sample_stream, command);
        if(AP4_SUCCEEDED(result))
        {
            // found a command in the sample, check the type
            switch(command->GetTag())
            {
            case AP4_COMMAND_TAG_OBJECT_DESCRIPTOR_UPDATE:
                if(od_update == NULL)
                {
                    od_update = AP4_DYNAMIC_CAST(AP4_DescriptorUpdateCommand, command);
                }
                break;

            case AP4_COMMAND_TAG_IPMP_DESCRIPTOR_UPDATE:
                if(ipmp_update == NULL)
                {
                    ipmp_update = AP4_DYNAMIC_CAST(AP4_DescriptorUpdateCommand, command);
                }
                break;

            default:
                break;
            }
        }
    }
    while(AP4_SUCCEEDED(result));
    sample_stream->Release();
    sample_stream = NULL;

    // check that we have what we need
    if(od_update == NULL || ipmp_update == NULL)
    {
        delete od_track;
        return AP4_ERROR_INVALID_FORMAT;
    }

    // process all the object descriptors in the od update
    for(AP4_List<AP4_Descriptor>::Item* od_item = od_update->GetDescriptors().FirstItem();
        od_item;
        od_item = od_item->GetNext())
    {
        od = AP4_DYNAMIC_CAST(AP4_ObjectDescriptor, od_item->GetData());
        if(od == NULL) continue;

        // find which track this od references
        AP4_EsIdRefDescriptor* es_id_ref;
        es_id_ref = AP4_DYNAMIC_CAST(AP4_EsIdRefDescriptor, od->FindSubDescriptor(AP4_DESCRIPTOR_TAG_ES_ID_REF));
        if(es_id_ref == NULL ||
           es_id_ref->GetRefIndex() > track_references->GetTrackIds().ItemCount() ||
           es_id_ref->GetRefIndex() == 0)
        {
            continue;
        }
        AP4_UI32 track_id = track_references->GetTrackIds()[es_id_ref->GetRefIndex()-1];
        SinfEntry* sinf_entry = NULL;
        for(AP4_List<SinfEntry>::Item* sinf_entry_item = sinf_entries.FirstItem();
            sinf_entry_item;
            sinf_entry_item = sinf_entry_item->GetNext())
        {
            sinf_entry = sinf_entry_item->GetData();
            if(sinf_entry->m_TrackId == track_id)
            {
                break; // match
            }
            else
            {
                sinf_entry = NULL; // no match
            }
        }
        if(sinf_entry == NULL) continue;  // no matching entry
        if(sinf_entry->m_Sinf != NULL) continue;  // entry already populated

        // see what ipmp descriptor this od points to
        AP4_IpmpDescriptorPointer* ipmpd_pointer;
        ipmpd_pointer = AP4_DYNAMIC_CAST(AP4_IpmpDescriptorPointer, od->FindSubDescriptor(AP4_DESCRIPTOR_TAG_IPMP_DESCRIPTOR_POINTER));
        if(ipmpd_pointer == NULL) continue;  // no pointer

        // find the ipmp descriptor referenced by the pointer
        AP4_IpmpDescriptor* ipmpd = NULL;
        for(AP4_List<AP4_Descriptor>::Item* ipmpd_item = ipmp_update->GetDescriptors().FirstItem();
            ipmpd_item;
            ipmpd_item = ipmpd_item->GetNext())
        {
            // check that this descriptor is of the right type
            ipmpd = AP4_DYNAMIC_CAST(AP4_IpmpDescriptor, ipmpd_item->GetData());
            if(ipmpd == NULL || ipmpd->GetIpmpsType() != AP4_MARLIN_IPMPS_TYPE_MGSV) continue;

            // check the descriptor id
            if(ipmpd->GetDescriptorId() == ipmpd_pointer->GetDescriptorId())
            {
                break; // match
            }
            else
            {
                ipmpd = NULL; // no match
            }
        }
        if(ipmpd == NULL) continue;  // no matching entry

        // parse the ipmp data into one or more 'sinf' atoms, and keep the one with the
        // right type
        AP4_MemoryByteStream* data = new AP4_MemoryByteStream(ipmpd->GetData().GetData(),
                ipmpd->GetData().GetDataSize());
        AP4_LargeSize bytes_available = ipmpd->GetData().GetDataSize();
        do
        {
            AP4_Atom* atom = NULL;

            // setup the factory with a context so we can instantiate a 'schm'
            // atom with a slightly different format than the standard 'schm'
            AP4_AtomFactory* factory = &AP4_MarlinIpmpAtomFactory::Instance;
            factory->PushContext(AP4_ATOM_TYPE('m', 'r', 'l', 'n'));

            // parse the next atom in the stream
            result = factory->CreateAtomFromStream(*data, bytes_available, atom);
            factory->PopContext();
            if(AP4_FAILED(result) || atom == NULL) break;

            // check that what we have parsed is indeed an 'sinf' of the right type
            if(atom->GetType() == AP4_ATOM_TYPE_SINF)
            {
                AP4_ContainerAtom* sinf = AP4_DYNAMIC_CAST(AP4_ContainerAtom, atom);
                AP4_SchmAtom* schm = AP4_DYNAMIC_CAST(AP4_SchmAtom, sinf->FindChild("schm"));
                if((schm->GetSchemeType()    == AP4_PROTECTION_SCHEME_TYPE_MARLIN_ACBC &&
                    schm->GetSchemeVersion() == 0x0100) ||
                   (schm->GetSchemeType()    == AP4_PROTECTION_SCHEME_TYPE_MARLIN_ACGK &&
                    schm->GetSchemeVersion() == 0x0100))
                {
                    // store the sinf in the entry for that track
                    sinf_entry->m_Sinf = sinf;
                    break;
                }
            }
            delete atom;
        }
        while(AP4_SUCCEEDED(result));
        data->Release();
    }

    // get rid of entries that have no SINF
    for(AP4_List<SinfEntry>::Item* sinf_entry_item = sinf_entries.FirstItem();
        sinf_entry_item;
        sinf_entry_item = sinf_entry_item->GetNext())
    {
        SinfEntry* sinf_entry = sinf_entry_item->GetData();
        if(sinf_entry->m_Sinf == NULL)
        {
            sinf_entries.Remove(sinf_entry);
            sinf_entry_item = sinf_entries.FirstItem();
            continue;
        }
    }

    // remove the iods atom and the OD track if required
    if(remove_od_data)
    {
        od_trak->Detach();
        delete od_trak;
        iods->Detach();
        delete iods;
    }

    // cleanup
    delete od_track;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpDecryptingProcessor:AP4_MarlinIpmpDecryptingProcessor
+---------------------------------------------------------------------*/
AP4_MarlinIpmpDecryptingProcessor::AP4_MarlinIpmpDecryptingProcessor(
    const AP4_ProtectionKeyMap* key_map,             /* = NULL */
    AP4_BlockCipherFactory*     block_cipher_factory /* = NULL */)
{
    if(key_map)
    {
        // copy the keys
        m_KeyMap.SetKeys(*key_map);
    }

    if(block_cipher_factory == NULL)
    {
        m_BlockCipherFactory = &AP4_DefaultBlockCipherFactory::Instance;
    }
    else
    {
        m_BlockCipherFactory = block_cipher_factory;
    }
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpDecryptingProcessor::~AP4_MarlinIpmpDecryptingProcessor
+---------------------------------------------------------------------*/
AP4_MarlinIpmpDecryptingProcessor::~AP4_MarlinIpmpDecryptingProcessor()
{
    m_SinfEntries.DeleteReferences();
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpDecryptingProcessor:Initialize
+---------------------------------------------------------------------*/
AP4_Result
AP4_MarlinIpmpDecryptingProcessor::Initialize(AP4_AtomParent&   top_level,
        AP4_ByteStream&   stream,
        ProgressListener* /*listener*/)
{
    return AP4_MarlinIpmpParser::Parse(top_level, stream, m_SinfEntries, true);
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpDecryptingProcessor:CreateTrackHandler
+---------------------------------------------------------------------*/
AP4_Processor::TrackHandler*
AP4_MarlinIpmpDecryptingProcessor::CreateTrackHandler(AP4_TrakAtom* trak)
{
    // look for this track in the list of entries
    AP4_MarlinIpmpParser::SinfEntry* sinf_entry = NULL;
    for(AP4_List<AP4_MarlinIpmpParser::SinfEntry>::Item* sinf_entry_item = m_SinfEntries.FirstItem();
        sinf_entry_item;
        sinf_entry_item = sinf_entry_item->GetNext())
    {
        sinf_entry = sinf_entry_item->GetData();
        if(sinf_entry->m_TrackId == trak->GetId())
        {
            break; // match
        }
        else
        {
            sinf_entry = NULL; // no match
        }
    }
    if(sinf_entry == NULL) return NULL;  // no matching entry
    AP4_ContainerAtom* sinf = sinf_entry->m_Sinf;

    // check the scheme
    bool use_group_key;
    AP4_SchmAtom* schm = AP4_DYNAMIC_CAST(AP4_SchmAtom, sinf->GetChild(AP4_ATOM_TYPE_SCHM));
    if(schm == NULL) return NULL;  // no schm
    if(schm->GetSchemeType()    == AP4_PROTECTION_SCHEME_TYPE_MARLIN_ACBC &&
       schm->GetSchemeVersion() == 0x0100)
    {
        use_group_key = false;
    }
    else if(schm->GetSchemeType()    == AP4_PROTECTION_SCHEME_TYPE_MARLIN_ACGK &&
            schm->GetSchemeVersion() == 0x0100)
    {
        use_group_key = true;
    }
    else
    {
        // unsupported scheme
        return NULL;
    }

    // find the key
    const AP4_UI08* key = NULL;
    AP4_DataBuffer  unwrapped_key;
    if(use_group_key)
    {
        const AP4_UI08* group_key = m_KeyMap.GetKey(0);
        if(group_key == NULL) return NULL;  // no group key
        AP4_ContainerAtom* schi = AP4_DYNAMIC_CAST(AP4_ContainerAtom, sinf->GetChild(AP4_ATOM_TYPE_SCHI));
        if(schi == NULL) return NULL;  // no schi
        AP4_Atom* gkey = schi->GetChild(AP4_ATOM_TYPE_GKEY);
        if(gkey == NULL) return NULL;  // no gkey
        AP4_MemoryByteStream* gkey_data = new AP4_MemoryByteStream();
        gkey->WriteFields(*gkey_data);
        AP4_AesKeyUnwrap(group_key, gkey_data->GetData(), gkey_data->GetDataSize(), unwrapped_key);
        key = unwrapped_key.GetData();
        gkey_data->Release();
    }
    else
    {
        key = m_KeyMap.GetKey(sinf_entry->m_TrackId);
    }
    if(key == NULL) return NULL;

    // create the decrypter
    AP4_MarlinIpmpTrackDecrypter* decrypter = NULL;
    AP4_Result result = AP4_MarlinIpmpTrackDecrypter::Create(*m_BlockCipherFactory,
                        key, decrypter);
    if(AP4_FAILED(result)) return NULL;

    return decrypter;
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpTrackDecrypter::Create
+---------------------------------------------------------------------*/
AP4_Result
AP4_MarlinIpmpTrackDecrypter::Create(AP4_BlockCipherFactory&        cipher_factory,
                                     const AP4_UI08*                key,
                                     AP4_MarlinIpmpTrackDecrypter*& decrypter)
{
    // default value
    decrypter = NULL;

    // create a block cipher for the decrypter
    AP4_BlockCipher* block_cipher = NULL;
    AP4_Result result = cipher_factory.Create(AP4_BlockCipher::AES_128,
                        AP4_BlockCipher::DECRYPT,
                        key,
                        AP4_AES_BLOCK_SIZE,
                        block_cipher);
    if(AP4_FAILED(result)) return result;

    // create a CBC cipher
    AP4_CbcStreamCipher* cbc_cipher = new AP4_CbcStreamCipher(block_cipher, AP4_StreamCipher::DECRYPT);

    // create the track decrypter
    decrypter = new AP4_MarlinIpmpTrackDecrypter(cbc_cipher);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpTrackDecrypter::~AP4_MarlinIpmpTrackDecrypter
+---------------------------------------------------------------------*/
AP4_MarlinIpmpTrackDecrypter::~AP4_MarlinIpmpTrackDecrypter()
{
    delete m_Cipher;
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpTrackDecrypter:GetProcessedSampleSize
+---------------------------------------------------------------------*/
AP4_Size
AP4_MarlinIpmpTrackDecrypter::GetProcessedSampleSize(AP4_Sample& sample)
{
    // with CBC, we need to decrypt the last block to know what the padding was
    AP4_Size       encrypted_size = sample.GetSize() - AP4_AES_BLOCK_SIZE;
    AP4_DataBuffer encrypted;
    AP4_DataBuffer decrypted;
    AP4_Size       decrypted_size = AP4_CIPHER_BLOCK_SIZE;
    if(sample.GetSize() < 2 * AP4_CIPHER_BLOCK_SIZE)
    {
        return 0;
    }
    AP4_Size offset = sample.GetSize() - 2 * AP4_CIPHER_BLOCK_SIZE;
    if(AP4_FAILED(sample.ReadData(encrypted, 2 * AP4_CIPHER_BLOCK_SIZE, offset)))
    {
        return 0;
    }
    decrypted.Reserve(decrypted_size);
    m_Cipher->SetIV(encrypted.GetData());
    if(AP4_FAILED(m_Cipher->ProcessBuffer(encrypted.GetData() + AP4_CIPHER_BLOCK_SIZE,
                                          AP4_CIPHER_BLOCK_SIZE,
                                          decrypted.UseData(),
                                          &decrypted_size,
                                          true)))
    {
        return 0;
    }
    unsigned int padding_size = AP4_CIPHER_BLOCK_SIZE - decrypted_size;
    return encrypted_size - padding_size;
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpTrackDecrypter:ProcessSample
+---------------------------------------------------------------------*/
AP4_Result
AP4_MarlinIpmpTrackDecrypter::ProcessSample(AP4_DataBuffer& data_in,
        AP4_DataBuffer& data_out)
{
    AP4_Result result;

    const AP4_UI08* in = data_in.GetData();
    AP4_Size        in_size = data_in.GetDataSize();

    // default to 0 output
    data_out.SetDataSize(0);

    // check that we have at least the minimum size
    if(in_size < 2 * AP4_AES_BLOCK_SIZE) return AP4_ERROR_INVALID_FORMAT;

    // process the sample data
    AP4_Size out_size = in_size - AP4_AES_BLOCK_SIZE; // worst case
    data_out.SetDataSize(out_size);
    AP4_UI08* out = data_out.UseData();

    // decrypt the data
    m_Cipher->SetIV(in);
    result = m_Cipher->ProcessBuffer(in + AP4_AES_BLOCK_SIZE,
                                     in_size - AP4_AES_BLOCK_SIZE,
                                     out,
                                     &out_size,
                                     true);
    if(AP4_FAILED(result)) return result;

    // update the payload size
    data_out.SetDataSize(out_size);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpEncryptingProcessor::AP4_MarlinIpmpEncryptingProcessor
+---------------------------------------------------------------------*/
AP4_MarlinIpmpEncryptingProcessor::AP4_MarlinIpmpEncryptingProcessor(
    bool                        use_group_key,       /* = false */
    const AP4_ProtectionKeyMap* key_map,             /* = NULL  */
    AP4_BlockCipherFactory*     block_cipher_factory /* = NULL  */) :
    m_UseGroupKey(use_group_key)
{
    if(key_map)
    {
        // copy the keys
        m_KeyMap.SetKeys(*key_map);
    }

    if(block_cipher_factory == NULL)
    {
        m_BlockCipherFactory = &AP4_DefaultBlockCipherFactory::Instance;
    }
    else
    {
        m_BlockCipherFactory = block_cipher_factory;
    }
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpEncryptingProcessor::Initialize
+---------------------------------------------------------------------*/
AP4_Result
AP4_MarlinIpmpEncryptingProcessor::Initialize(
    AP4_AtomParent&                  top_level,
    AP4_ByteStream&                  /*stream*/,
    AP4_Processor::ProgressListener* /*listener = NULL*/)
{
    // get the moov atom
    AP4_MoovAtom* moov = AP4_DYNAMIC_CAST(AP4_MoovAtom, top_level.GetChild(AP4_ATOM_TYPE_MOOV));
    if(moov == NULL) return AP4_ERROR_INVALID_FORMAT;

    // deal with the file type
    AP4_FtypAtom* ftyp = AP4_DYNAMIC_CAST(AP4_FtypAtom, top_level.GetChild(AP4_ATOM_TYPE_FTYP));
    if(ftyp)
    {
        // remove the atom, it will be replaced with a new one
        top_level.RemoveChild(ftyp);

        // keep the existing brand and compatible brands
        AP4_Array<AP4_UI32> compatible_brands;
        compatible_brands.EnsureCapacity(ftyp->GetCompatibleBrands().ItemCount() + 1);
        for(unsigned int i = 0; i < ftyp->GetCompatibleBrands().ItemCount(); i++)
        {
            compatible_brands.Append(ftyp->GetCompatibleBrands()[i]);
        }

        // add the MGSV compatible brand if it is not already there
        if(!ftyp->HasCompatibleBrand(AP4_MARLIN_BRAND_MGSV))
        {
            compatible_brands.Append(AP4_MARLIN_BRAND_MGSV);
        }

        // create a replacement for the major brand
        AP4_FtypAtom* new_ftyp = new AP4_FtypAtom(AP4_MARLIN_BRAND_MGSV,
                0x13c078c, //AP4_MARLIN_BRAND_MGSV_MAJOR_VERSION,
                &compatible_brands[0],
                compatible_brands.ItemCount());
        delete ftyp;
        ftyp = new_ftyp;
    }
    else
    {
        AP4_UI32 isom = AP4_FTYP_BRAND_ISOM;
        ftyp = new AP4_FtypAtom(AP4_MARLIN_BRAND_MGSV, 0, &isom, 1);
    }

    // insert the ftyp atom as the first child
    top_level.AddChild(ftyp, 0);

    // create and 'mpod' track reference atom
    AP4_TrefTypeAtom* mpod = new AP4_TrefTypeAtom(AP4_ATOM_TYPE_MPOD);

    // look for an available track ID, starting at 1
    unsigned int od_track_id       = 0;
    unsigned int od_track_position = 0;
    for(AP4_List<AP4_TrakAtom>::Item* trak_item = moov->GetTrakAtoms().FirstItem();
        trak_item;
        trak_item = trak_item->GetNext())
    {
        AP4_TrakAtom* trak = trak_item->GetData();
        if(trak)
        {
            od_track_position++;
            if(trak->GetId() >= od_track_id)
            {
                od_track_id = trak->GetId() + 1;
            }

            // if the track is encrypted, reference it in the mpod
            if(m_KeyMap.GetKey(trak->GetId()))
            {
                mpod->AddTrackId(trak->GetId());
            }

            //m_SinfEntries.Add(new SinfEntry(trak->GetId(), NULL));
        }
    }

    // check that there was at least one track in the file
    if(od_track_id == 0) return AP4_ERROR_INVALID_FORMAT;

    // create an initial object descriptor
    AP4_InitialObjectDescriptor* iod =
        // FIXME: get real values from the property map
        new AP4_InitialObjectDescriptor(AP4_DESCRIPTOR_TAG_MP4_IOD,
                                        1022, // object descriptor id
                                        false,
                                        0xFE,    // OD profile level (0xFE = No OD profile specified)
                                        0xFF,    // scene profile level
                                        0xFE,    // audio profile level
                                        0xFE,    // visual profile level
                                        0xFF);   // graphics profile

    // create an ES_ID_Inc subdescriptor and add it to the initial object descriptor
    AP4_EsIdIncDescriptor* es_id_inc = new AP4_EsIdIncDescriptor(od_track_id);
    iod->AddSubDescriptor(es_id_inc);

    // create an iods atom to hold the initial object descriptor
    AP4_IodsAtom* iods = new AP4_IodsAtom(iod);

    // add the iods atom to the moov atom (try to put it just after mvhd)
    int iods_position = 0;
    int item_position = 0;
    for(AP4_List<AP4_Atom>::Item* moov_item = moov->GetChildren().FirstItem();
        moov_item;
        moov_item = moov_item->GetNext())
    {
        ++item_position;
        if(moov_item->GetData()->GetType() == AP4_ATOM_TYPE_MVHD)
        {
            iods_position = item_position;
            break;
        }
    }
    AP4_Result result = moov->AddChild(iods, iods_position);
    if(AP4_FAILED(result))
    {
        delete iods;
        return result;
    }

    // create a sample table for the OD track
    AP4_SyntheticSampleTable* od_sample_table = new AP4_SyntheticSampleTable();

    // create the sample description for the OD track
    AP4_MpegSystemSampleDescription* od_sample_description;
    od_sample_description = new AP4_MpegSystemSampleDescription(AP4_STREAM_TYPE_OD,
            AP4_OTI_MPEG4_SYSTEM,
            NULL,
            32768, // buffer size
            1024,  // max bitrate
            512);  // avg bitrate
    od_sample_table->AddSampleDescription(od_sample_description, true);

    // create the OD descriptor update
    AP4_DescriptorUpdateCommand od_update(AP4_COMMAND_TAG_OBJECT_DESCRIPTOR_UPDATE);
    for(unsigned int i = 0; i < mpod->GetTrackIds().ItemCount(); i++)
    {
        AP4_ObjectDescriptor* od = new AP4_ObjectDescriptor(AP4_DESCRIPTOR_TAG_MP4_OD, 256 + i); // descriptor id = 256+i
        od->AddSubDescriptor(new AP4_EsIdRefDescriptor(i + 1));   // index into mpod (1-based)
        od->AddSubDescriptor(new AP4_IpmpDescriptorPointer(i + 1)); // descriptor id = i+1
        od_update.AddDescriptor(od);
    }

    // create the IPMP descriptor update
    AP4_DescriptorUpdateCommand ipmp_update(AP4_COMMAND_TAG_IPMP_DESCRIPTOR_UPDATE);
    for(unsigned int i = 0; i < mpod->GetTrackIds().ItemCount(); i++)
    {
        // create the ipmp descriptor
        AP4_IpmpDescriptor* ipmp_descriptor = new AP4_IpmpDescriptor(i + 1, AP4_MARLIN_IPMPS_TYPE_MGSV);

        // create the sinf container
        AP4_ContainerAtom* sinf = new AP4_ContainerAtom(AP4_ATOM_TYPE_SINF);

        // add the scheme type atom
        sinf->AddChild(new AP4_SchmAtom(m_UseGroupKey ?
                                        AP4_PROTECTION_SCHEME_TYPE_MARLIN_ACGK :
                                        AP4_PROTECTION_SCHEME_TYPE_MARLIN_ACBC,
                                        0x0100, NULL, true));

        // create the 'schi' container
        AP4_ContainerAtom* schi = new AP4_ContainerAtom(AP4_ATOM_TYPE_SCHI);

        // add the content ID
        const char* content_id = m_PropertyMap.GetProperty(mpod->GetTrackIds()[i], "ContentId");
        if(content_id)
        {
            // add the content ID (8id_)
            schi->AddChild(new AP4_NullTerminatedStringAtom(AP4_ATOM_TYPE_8ID_, content_id));
        }

        // add the signed attributes, if any
        const char* signed_attributes = m_PropertyMap.GetProperty(mpod->GetTrackIds()[i], "SignedAttributes");
        if(signed_attributes)
        {
            // decode the hex-encoded data
            unsigned int size = (unsigned int)AP4_StringLength(signed_attributes) / 2;
            AP4_DataBuffer attributes_atoms;
            attributes_atoms.SetDataSize(size);
            if(AP4_SUCCEEDED(AP4_ParseHex(signed_attributes, attributes_atoms.UseData(), size)))
            {
                // parse all the atoms encoded in the data and add them to the 'schi' container
                AP4_MemoryByteStream* mbs = new AP4_MemoryByteStream(attributes_atoms.GetData(),
                        attributes_atoms.GetDataSize());
                do
                {
                    AP4_Atom* atom = NULL;
                    result = AP4_DefaultAtomFactory::Instance.CreateAtomFromStream(*mbs, atom);
                    if(AP4_SUCCEEDED(result) && atom)
                    {
                        schi->AddChild(atom);
                    }
                }
                while(AP4_SUCCEEDED(result));
                mbs->Release();
            }
        }

        // find what the track type is (necessary for the next step) and the key
        const AP4_UI08* key;
        unsigned int    key_size = 0;
        AP4_Track::Type track_type = AP4_Track::TYPE_UNKNOWN;
        for(AP4_List<AP4_TrakAtom>::Item* trak_item = moov->GetTrakAtoms().FirstItem();
            trak_item;
            trak_item = trak_item->GetNext())
        {
            AP4_TrakAtom* trak = trak_item->GetData();
            if(trak->GetId() == mpod->GetTrackIds()[i])
            {
                // find the handler type
                AP4_Atom* sub = trak->FindChild("mdia/hdlr");
                if(sub)
                {
                    AP4_HdlrAtom* hdlr = AP4_DYNAMIC_CAST(AP4_HdlrAtom, sub);
                    if(hdlr)
                    {
                        AP4_UI32 type = hdlr->GetHandlerType();
                        if(type == AP4_HANDLER_TYPE_SOUN)
                        {
                            track_type = AP4_Track::TYPE_AUDIO;
                        }
                        else if(type == AP4_HANDLER_TYPE_VIDE)
                        {
                            track_type = AP4_Track::TYPE_VIDEO;
                        }
                    }
                }

                // find the key
                const AP4_UI08* iv = NULL;
                if(AP4_SUCCEEDED(m_KeyMap.GetKeyAndIv(trak->GetId(), key, iv)))
                {
                    key_size = 16;
                }

                break;
            }
        }

        // group key
        if(m_UseGroupKey)
        {
            // find the group key
            const AP4_UI08* iv = NULL;
            const AP4_UI08* group_key;
            if(AP4_SUCCEEDED(m_KeyMap.GetKeyAndIv(0, group_key, iv)))
            {
                AP4_DataBuffer wrapped_key;
                result = AP4_AesKeyWrap(group_key, key, key_size, wrapped_key);
                if(AP4_FAILED(result)) return result;
                AP4_UnknownAtom* gkey = new AP4_UnknownAtom(AP4_ATOM_TYPE_GKEY,
                        wrapped_key.GetData(),
                        wrapped_key.GetDataSize());
                schi->AddChild(gkey);
            }
        }

        // create and add the secure attributes (satr)
        if(track_type != AP4_Track::TYPE_UNKNOWN && key != NULL && key_size != 0)
        {
            AP4_ContainerAtom* satr = new AP4_ContainerAtom(AP4_ATOM_TYPE_SATR);
            switch(track_type)
            {
            case AP4_Track::TYPE_AUDIO:
                satr->AddChild(new AP4_NullTerminatedStringAtom(AP4_ATOM_TYPE_STYP, AP4_MARLIN_IPMP_STYP_AUDIO));
                break;
            case AP4_Track::TYPE_VIDEO:
                satr->AddChild(new AP4_NullTerminatedStringAtom(AP4_ATOM_TYPE_STYP, AP4_MARLIN_IPMP_STYP_VIDEO));
                break;
            default:
                break;
            }

            // compute the hmac
            AP4_MemoryByteStream* mbs = new AP4_MemoryByteStream();
            satr->Write(*mbs);
            AP4_Hmac* digester = NULL;
            AP4_Hmac::Create(AP4_Hmac::SHA256, key, key_size, digester);
            digester->Update(mbs->GetData(), mbs->GetDataSize());
            AP4_DataBuffer hmac_value;
            digester->Final(hmac_value);
            AP4_Atom* hmac = new AP4_UnknownAtom(AP4_ATOM_TYPE_HMAC, hmac_value.GetData(), hmac_value.GetDataSize());

            schi->AddChild(satr);
            schi->AddChild(hmac);

            mbs->Release();
        }

        sinf->AddChild(schi);

        // serialize the sinf atom to a buffer and set it as the ipmp data
        AP4_MemoryByteStream* sinf_data = new AP4_MemoryByteStream((AP4_Size)sinf->GetSize());
        sinf->Write(*sinf_data);
        ipmp_descriptor->SetData(sinf_data->GetData(), sinf_data->GetDataSize());
        sinf_data->Release();

        ipmp_update.AddDescriptor(ipmp_descriptor);
    }

    // add the sample with the descriptors and updates
    AP4_MemoryByteStream* sample_data = new AP4_MemoryByteStream();
    od_update.Write(*sample_data);
    ipmp_update.Write(*sample_data);
    od_sample_table->AddSample(*sample_data, 0, sample_data->GetDataSize(), 0, 0, 0, 0, true);

    // create the OD track
    AP4_TrakAtom* od_track = new AP4_TrakAtom(od_sample_table,
            AP4_HANDLER_TYPE_ODSM,
            "Bento4 Marlin OD Handler",
            od_track_id,
            0, 0,
            1, 1000, 1, 0, "und",
            0, 0);

    // add an entry in the processor's stream table to indicate that the
    // media data for the OD track is not in the file stream, but in our
    // memory stream.
    m_ExternalTrackData.Add(new ExternalTrackData(od_track_id, sample_data));
    sample_data->Release();

    // add a tref track reference atom
    AP4_ContainerAtom* tref = new AP4_ContainerAtom(AP4_ATOM_TYPE_TREF);
    tref->AddChild(mpod);
    od_track->AddChild(tref, 1); // add after 'tkhd'

    // add the track to the moov atoms (just after the last track)
    moov->AddChild(od_track, od_track_position);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpEncryptingProcessor::CreateTrackHandler
+---------------------------------------------------------------------*/
AP4_Processor::TrackHandler*
AP4_MarlinIpmpEncryptingProcessor::CreateTrackHandler(AP4_TrakAtom* trak)
{
    // create a handler for this track if we have a key for it
    const AP4_UI08* key;
    const AP4_UI08* iv;
    if(AP4_SUCCEEDED(m_KeyMap.GetKeyAndIv(trak->GetId(), key, iv)))
    {
        // create the track handler
        AP4_MarlinIpmpTrackEncrypter* handler = NULL;
        AP4_Result result = AP4_MarlinIpmpTrackEncrypter::Create(*m_BlockCipherFactory, key, iv, handler);
        if(AP4_FAILED(result)) return NULL;
        return handler;
    }

    // not encrypted
    return NULL;
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpTrackEncrypter::Create
+---------------------------------------------------------------------*/
AP4_Result
AP4_MarlinIpmpTrackEncrypter::Create(AP4_BlockCipherFactory&        cipher_factory,
                                     const AP4_UI08*                key,
                                     const AP4_UI08*                iv,
                                     AP4_MarlinIpmpTrackEncrypter*& encrypter)
{
    // default value
    encrypter = NULL;

    // create a block cipher
    AP4_BlockCipher* block_cipher = NULL;
    AP4_Result result = cipher_factory.Create(AP4_BlockCipher::AES_128,
                        AP4_BlockCipher::ENCRYPT,
                        key,
                        AP4_AES_BLOCK_SIZE,
                        block_cipher);
    if(AP4_FAILED(result)) return result;

    // create a CBC cipher
    AP4_CbcStreamCipher* cbc_cipher = new AP4_CbcStreamCipher(block_cipher, AP4_StreamCipher::ENCRYPT);

    // create the track encrypter
    encrypter = new AP4_MarlinIpmpTrackEncrypter(cbc_cipher, iv);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpTrackEncrypter::AP4_MarlinIpmpTrackEncrypter
+---------------------------------------------------------------------*/
AP4_MarlinIpmpTrackEncrypter::AP4_MarlinIpmpTrackEncrypter(AP4_StreamCipher* cipher,
        const AP4_UI08*   iv) :
    m_Cipher(cipher)
{
    // copy the IV
    AP4_CopyMemory(m_IV, iv, AP4_AES_BLOCK_SIZE);
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpTrackEncrypter::~AP4_MarlinIpmpTrackEncrypter
+---------------------------------------------------------------------*/
AP4_MarlinIpmpTrackEncrypter::~AP4_MarlinIpmpTrackEncrypter()
{
    delete m_Cipher;
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpTrackEncrypter:GetProcessedSampleSize
+---------------------------------------------------------------------*/
AP4_Size
AP4_MarlinIpmpTrackEncrypter::GetProcessedSampleSize(AP4_Sample& sample)
{
    return AP4_CIPHER_BLOCK_SIZE * (2 + (sample.GetSize() / AP4_CIPHER_BLOCK_SIZE));
}

/*----------------------------------------------------------------------
|   AP4_MarlinIpmpTrackEncrypter:ProcessSample
+---------------------------------------------------------------------*/
AP4_Result
AP4_MarlinIpmpTrackEncrypter::ProcessSample(AP4_DataBuffer& data_in,
        AP4_DataBuffer& data_out)
{
    AP4_Result result;

    const AP4_UI08* in = data_in.GetData();
    AP4_Size        in_size = data_in.GetDataSize();

    // default to 0 output
    data_out.SetDataSize(0);

    // process the sample data
    AP4_Size out_size = AP4_CIPHER_BLOCK_SIZE * (2 + (in_size / AP4_CIPHER_BLOCK_SIZE));
    data_out.SetDataSize(out_size);
    AP4_UI08* out = data_out.UseData();

    // write the IV
    AP4_CopyMemory(out, m_IV, AP4_CIPHER_BLOCK_SIZE);
    out_size -= AP4_CIPHER_BLOCK_SIZE;

    // encrypt the data
    m_Cipher->SetIV(m_IV);
    result = m_Cipher->ProcessBuffer(in,
                                     in_size,
                                     out + AP4_AES_BLOCK_SIZE,
                                     &out_size,
                                     true);
    if(AP4_FAILED(result)) return result;

    // update the payload size
    data_out.SetDataSize(out_size + AP4_AES_BLOCK_SIZE);

    return AP4_SUCCESS;
}
