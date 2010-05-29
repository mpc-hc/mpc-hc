/*****************************************************************
|
|    AP4 - MetaData 
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
#include "Ap4File.h"
#include "Ap4Movie.h"
#include "Ap4MetaData.h"
#include "Ap4ContainerAtom.h"
#include "Ap4MoovAtom.h"
#include "Ap4HdlrAtom.h"
#include "Ap4DataBuffer.h"
#include "Ap4Utils.h"
#include "Ap4String.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_3GppLocalizedStringAtom)
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_DcfdAtom)
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_DcfStringAtom)
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_DataAtom)

/*----------------------------------------------------------------------
|   metadata keys
+---------------------------------------------------------------------*/
static const AP4_MetaData::KeyInfo AP4_MetaData_KeyInfos [] = {
    {"Name",                "Name",                 AP4_ATOM_TYPE_cNAM, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Artist",              "Artist",               AP4_ATOM_TYPE_cART, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"AlbumArtist",         "Album Artist",         AP4_ATOM_TYPE_aART, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Composer",            "Composer",             AP4_ATOM_TYPE_cCOM, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Writer",              "Writer",               AP4_ATOM_TYPE_cWRT, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Album",               "Album",                AP4_ATOM_TYPE_cALB, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"GenreCode",           "Genre",                AP4_ATOM_TYPE_GNRE, AP4_MetaData::Value::TYPE_BINARY},
    {"GenreName",           "Genre",                AP4_ATOM_TYPE_cGEN, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Grouping",            "Grouping",             AP4_ATOM_TYPE_cGRP, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Date",                "Date",                 AP4_ATOM_TYPE_cDAY, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Tool",                "Encoding Tool",        AP4_ATOM_TYPE_cTOO, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Comment",             "Comment",              AP4_ATOM_TYPE_cCMT, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Lyrics",              "Lyrics",               AP4_ATOM_TYPE_cLYR, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Copyright",           "Copyright",            AP4_ATOM_TYPE_CPRT, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Track",               "Track Number",         AP4_ATOM_TYPE_TRKN, AP4_MetaData::Value::TYPE_BINARY},
    {"Disc",                "Disc Number",          AP4_ATOM_TYPE_DISK, AP4_MetaData::Value::TYPE_BINARY},
    {"Cover",               "Cover Art",            AP4_ATOM_TYPE_COVR, AP4_MetaData::Value::TYPE_BINARY},
    {"Description",         "Description",          AP4_ATOM_TYPE_DESC, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Rating",              "Rating",               AP4_ATOM_TYPE_RTNG, AP4_MetaData::Value::TYPE_INT_08_BE},
    {"Tempo",               "Tempo",                AP4_ATOM_TYPE_TMPO, AP4_MetaData::Value::TYPE_INT_16_BE},
    {"Compilation",         "Compilation",          AP4_ATOM_TYPE_CPIL, AP4_MetaData::Value::TYPE_INT_08_BE},
    {"IsGapless",           "Is Gapless",           AP4_ATOM_TYPE_PGAP, AP4_MetaData::Value::TYPE_INT_08_BE},
    {"Title",               "Title",                AP4_ATOM_TYPE_TITL, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Description",         "Description",          AP4_ATOM_TYPE_DSCP, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"StoreFrontID",        "Store Front ID",       AP4_ATOM_TYPE_sfID, AP4_MetaData::Value::TYPE_INT_32_BE},
    {"FileKind",            "File Kind",            AP4_ATOM_TYPE_STIK, AP4_MetaData::Value::TYPE_INT_08_BE},
    {"ShowName",            "Show Name",            AP4_ATOM_TYPE_TVSH, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"ShowSeason",          "Show Season Number",   AP4_ATOM_TYPE_TVSN, AP4_MetaData::Value::TYPE_INT_32_BE},
    {"ShowEpisodeNumber",   "Show Episode Number",  AP4_ATOM_TYPE_TVES, AP4_MetaData::Value::TYPE_INT_32_BE},
    {"ShowEpisodeName",     "Show Episode Name",    AP4_ATOM_TYPE_TVEN, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"TVNetworkName",       "TV Network Name",      AP4_ATOM_TYPE_TVNN, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"IsPodcast",           "Is a Podcast",         AP4_ATOM_TYPE_PCST, AP4_MetaData::Value::TYPE_INT_08_BE},
    {"PodcastUrl",          "Podcast URL",          AP4_ATOM_TYPE_PURL, AP4_MetaData::Value::TYPE_BINARY},
    {"PodcastGuid",         "Podcast GUID",         AP4_ATOM_TYPE_EGID, AP4_MetaData::Value::TYPE_BINARY},
    {"PodcastCategory",     "Podcast Category",     AP4_ATOM_TYPE_CATG, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Keywords",            "Keywords",             AP4_ATOM_TYPE_KEYW, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"PurchaseDate",        "Purchase Date",        AP4_ATOM_TYPE_PURD, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"IconUri",             "Icon URI",             AP4_ATOM_TYPE_ICNU, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"InfoUrl",             "Info URL",             AP4_ATOM_TYPE_INFU, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"CoverUri",            "Cover Art URI",        AP4_ATOM_TYPE_CVRU, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"LyricsUri",           "Lyrics URI",           AP4_ATOM_TYPE_LRCU, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Duration",            "Duration",             AP4_ATOM_TYPE_DCFD, AP4_MetaData::Value::TYPE_INT_32_BE},
    {"Performer",           "Performer",            AP4_ATOM_TYPE_PERF, AP4_MetaData::Value::TYPE_STRING_UTF_8},
    {"Author",              "Author",               AP4_ATOM_TYPE_AUTH, AP4_MetaData::Value::TYPE_STRING_UTF_8},
};
AP4_Array<AP4_MetaData::KeyInfo> AP4_MetaData::KeyInfos(
    AP4_MetaData_KeyInfos, 
    sizeof(AP4_MetaData_KeyInfos)/sizeof(KeyInfo));

/*----------------------------------------------------------------------
|   genre IDs
+---------------------------------------------------------------------*/
static const char* const Ap4Id3Genres[] = 
{
    "Blues",
    "Classic Rock",
    "Country",
    "Dance",
    "Disco",
    "Funk",
    "Grunge",
    "Hip-Hop",
    "Jazz",
    "Metal",
    "New Age",
    "Oldies",
    "Other",
    "Pop",
    "R&B",
    "Rap",
    "Reggae",
    "Rock",
    "Techno",
    "Industrial",
    "Alternative",
    "Ska",
    "Death Metal",
    "Pranks",
    "Soundtrack",
    "Euro-Techno",
    "Ambient",
    "Trip-Hop",
    "Vocal",
    "Jazz+Funk",
    "Fusion",
    "Trance",
    "Classical",
    "Instrumental",
    "Acid",
    "House",
    "Game",
    "Sound Clip",
    "Gospel",
    "Noise",
    "AlternRock",
    "Bass",
    "Soul",
    "Punk",
    "Space",
    "Meditative",
    "Instrumental Pop",
    "Instrumental Rock",
    "Ethnic",
    "Gothic",
    "Darkwave",
    "Techno-Industrial",
    "Electronic",
    "Pop-Folk",
    "Eurodance",
    "Dream",
    "Southern Rock",
    "Comedy",
    "Cult",
    "Gangsta",
    "Top 40",
    "Christian Rap",
    "Pop/Funk",
    "Jungle",
    "Native American",
    "Cabaret",
    "New Wave",
    "Psychadelic",
    "Rave",
    "Showtunes",
    "Trailer",
    "Lo-Fi",
    "Tribal",
    "Acid Punk",
    "Acid Jazz",
    "Polka",
    "Retro",
    "Musical",
    "Rock & Roll",
    "Hard Rock",
    "Folk",
    "Folk-Rock",
    "National Folk",
    "Swing",
    "Fast Fusion",
    "Bebob",
    "Latin",
    "Revival",
    "Celtic",
    "Bluegrass",
    "Avantgarde",
    "Gothic Rock",
    "Progressive Rock",
    "Psychedelic Rock",
    "Symphonic Rock",
    "Slow Rock",
    "Big Band",
    "Chorus",
    "Easy Listening",
    "Acoustic",
    "Humour",
    "Speech",
    "Chanson",
    "Opera",
    "Chamber Music",
    "Sonata",
    "Symphony",
    "Booty Bass",
    "Primus",
    "Porn Groove",
    "Satire",
    "Slow Jam",
    "Club",
    "Tango",
    "Samba",
    "Folklore",
    "Ballad",
    "Power Ballad",
    "Rhythmic Soul",
    "Freestyle",
    "Duet",
    "Punk Rock",
    "Drum Solo",
    "Acapella",
    "Euro-House",
    "Dance Hall"
};

static const char*
Ap4StikNames[] = {
    "Movie",            // 0
    "Normal",           // 1
    "Audiobook",        // 2
    "?",                // 3
    "?",                // 4
    "Whacked Bookmark", // 5
    "Music Video",      // 6
    "?",                // 7
    "?",                // 8
    "Short Film",       // 9 
    "TV Show",          // 10
    "Booklet",          // 11
    "?",                // 12
    "?",                // 13
    "Ring Tone"         // 14
};


/* sfID Store Front country
    Australia   => 143460,
    Austria     => 143445,
    Belgium     => 143446,
    Canada      => 143455,
    Denmark     => 143458,
    Finland     => 143447,
    France      => 143442,
    Germany     => 143443,
    Greece      => 143448,
    Ireland     => 143449,
    Italy       => 143450,
    Japan       => 143462,
    Luxembourg  => 143451,
    Netherlands => 143452,
    Norway      => 143457,
    Portugal    => 143453,
    Spain       => 143454,
    Sweden      => 143456,
    Switzerland => 143459,
    UK          => 143444,
    USA         => 143441,
*/

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_Size AP4_DATA_ATOM_MAX_SIZE = 0x40000000;

/*----------------------------------------------------------------------
|   3GPP localized string atoms
+---------------------------------------------------------------------*/
const AP4_Atom::Type AP4_MetaDataAtomTypeHandler::_3gppLocalizedStringTypes[] = {
    AP4_ATOM_TYPE_TITL,
    AP4_ATOM_TYPE_DSCP,
    AP4_ATOM_TYPE_CPRT,
    AP4_ATOM_TYPE_PERF,
    AP4_ATOM_TYPE_AUTH,
    AP4_ATOM_TYPE_GNRE
};
const AP4_MetaDataAtomTypeHandler::TypeList AP4_MetaDataAtomTypeHandler::_3gppLocalizedStringTypeList = {
    _3gppLocalizedStringTypes,
    sizeof(_3gppLocalizedStringTypes)/sizeof(_3gppLocalizedStringTypes[0])
};

/*----------------------------------------------------------------------
|   other 3GPP atoms
+---------------------------------------------------------------------*/
const AP4_Atom::Type AP4_MetaDataAtomTypeHandler::_3gppOtherTypes[] = {
    AP4_ATOM_TYPE_RTNG,
    AP4_ATOM_TYPE_CLSF,
    AP4_ATOM_TYPE_KYWD,
    AP4_ATOM_TYPE_LOCI,
    AP4_ATOM_TYPE_ALBM,
    AP4_ATOM_TYPE_YRRC,
};
const AP4_MetaDataAtomTypeHandler::TypeList AP4_MetaDataAtomTypeHandler::_3gppOtherTypeList = {
    _3gppOtherTypes,
    sizeof(_3gppOtherTypes)/sizeof(_3gppOtherTypes[0])
};

/*----------------------------------------------------------------------
|   DCF string atoms
+---------------------------------------------------------------------*/
const AP4_Atom::Type AP4_MetaDataAtomTypeHandler::DcfStringTypes[] = {
    AP4_ATOM_TYPE_ICNU,
    AP4_ATOM_TYPE_INFU,
    AP4_ATOM_TYPE_CVRU,
    AP4_ATOM_TYPE_LRCU
};
const AP4_MetaDataAtomTypeHandler::TypeList AP4_MetaDataAtomTypeHandler::DcfStringTypeList = {
    DcfStringTypes,
    sizeof(DcfStringTypes)/sizeof(DcfStringTypes[0])
};

/*----------------------------------------------------------------------
|   atom type lists
+---------------------------------------------------------------------*/
const AP4_Atom::Type AP4_MetaDataAtomTypeHandler::IlstTypes[] = 
{
    AP4_ATOM_TYPE_dddd,
    AP4_ATOM_TYPE_cNAM,
    AP4_ATOM_TYPE_cART,
    AP4_ATOM_TYPE_cCOM,
    AP4_ATOM_TYPE_cWRT,
    AP4_ATOM_TYPE_cALB,
    AP4_ATOM_TYPE_cGEN,
    AP4_ATOM_TYPE_cGRP,
    AP4_ATOM_TYPE_cDAY,
    AP4_ATOM_TYPE_cTOO,
    AP4_ATOM_TYPE_cCMT,
    AP4_ATOM_TYPE_CPRT,
    AP4_ATOM_TYPE_TRKN,
    AP4_ATOM_TYPE_DISK,
    AP4_ATOM_TYPE_COVR,
    AP4_ATOM_TYPE_DESC,
    AP4_ATOM_TYPE_GNRE,
    AP4_ATOM_TYPE_CPIL,
    AP4_ATOM_TYPE_TMPO,
    AP4_ATOM_TYPE_RTNG,
    AP4_ATOM_TYPE_apID,
    AP4_ATOM_TYPE_cnID,
    AP4_ATOM_TYPE_cmID,
    AP4_ATOM_TYPE_atID,
    AP4_ATOM_TYPE_plID,
    AP4_ATOM_TYPE_geID,
    AP4_ATOM_TYPE_sfID,
    AP4_ATOM_TYPE_akID,
    AP4_ATOM_TYPE_aART,
    AP4_ATOM_TYPE_TVNN,
    AP4_ATOM_TYPE_TVSH,
    AP4_ATOM_TYPE_TVEN,
    AP4_ATOM_TYPE_TVSN,
    AP4_ATOM_TYPE_TVES,
    AP4_ATOM_TYPE_STIK,
    AP4_ATOM_TYPE_PGAP,
    AP4_ATOM_TYPE_PCST,
    AP4_ATOM_TYPE_PURD,
    AP4_ATOM_TYPE_PURL,
    AP4_ATOM_TYPE_EGID,
    AP4_ATOM_TYPE_SONM,
    AP4_ATOM_TYPE_SOAL,
    AP4_ATOM_TYPE_SOAR,
    AP4_ATOM_TYPE_SOAA,
    AP4_ATOM_TYPE_SOCO,
    AP4_ATOM_TYPE_SOSN
};
const AP4_MetaDataAtomTypeHandler::TypeList AP4_MetaDataAtomTypeHandler::IlstTypeList = {
    IlstTypes,
    sizeof(IlstTypes)/sizeof(IlstTypes[0])
};

/*----------------------------------------------------------------------
|   AP4_MetaDataAtomTypeHandler::CreateAtom
+---------------------------------------------------------------------*/
AP4_Result 
AP4_MetaDataAtomTypeHandler::CreateAtom(AP4_Atom::Type  type,
                                        AP4_UI32        size,
                                        AP4_ByteStream& stream,
                                        AP4_Atom::Type  context,
                                        AP4_Atom*&      atom)
{
    atom = NULL;

    if (context == AP4_ATOM_TYPE_ILST) {
        if (IsTypeInList(type, IlstTypeList)) {
            m_AtomFactory->PushContext(type);
            atom = AP4_ContainerAtom::Create(type, size, false, false, stream, *m_AtomFactory);
            m_AtomFactory->PopContext();
        }
    } else if (type == AP4_ATOM_TYPE_DATA) {
        if (IsTypeInList(context, IlstTypeList)) {
            atom = new AP4_DataAtom(size, stream);
        }
    } else if (context == AP4_ATOM_TYPE_dddd) {
        if (type == AP4_ATOM_TYPE_MEAN || type == AP4_ATOM_TYPE_NAME) {
            atom = new AP4_MetaDataStringAtom(type, size, stream);
        }
    } else if (context == AP4_ATOM_TYPE_UDTA) {
        if (IsTypeInList(type, _3gppLocalizedStringTypeList)) {
            atom = AP4_3GppLocalizedStringAtom::Create(type, size, stream);
        } else if (IsTypeInList(type, DcfStringTypeList)) {
            atom = AP4_DcfStringAtom::Create(type, size, stream);
        } else if (type == AP4_ATOM_TYPE_DCFD) {
            atom = AP4_DcfdAtom::Create(size, stream);
        }
    }

    return atom?AP4_SUCCESS:AP4_FAILURE;
}

/*----------------------------------------------------------------------
|   AP4_MetaDataAtomTypeHandler::IsTypeInList
+---------------------------------------------------------------------*/
bool
AP4_MetaDataAtomTypeHandler::IsTypeInList(AP4_Atom::Type type, const AP4_MetaDataAtomTypeHandler::TypeList& list)
{
    for (unsigned int i=0; i<list.m_Size; i++) {
        if (type == list.m_Types[i]) return true;
    }
    return false;
}

/*----------------------------------------------------------------------
|   AP4_MetaData::AP4_MetaData
+---------------------------------------------------------------------*/
AP4_MetaData::AP4_MetaData(AP4_File* file)
{
    // get the file's movie
    AP4_Movie* movie = file->GetMovie();

    // handle the movie's metadata if there is a movie in the file
    if (movie) {
        AP4_MoovAtom* moov = movie->GetMoovAtom();
        if (moov == NULL) return;
        ParseMoov(moov);
    } else {
        // if we don't have a movie, try to show metadata from a udta atom
        AP4_List<AP4_Atom>& top_level_atoms = file->GetTopLevelAtoms();
        
        AP4_List<AP4_Atom>::Item* atom_item = top_level_atoms.FirstItem();
        while (atom_item) {
            AP4_ContainerAtom* container = AP4_DYNAMIC_CAST(AP4_ContainerAtom, atom_item->GetData());
            if (container) {
                // look for a udta in a DCF layout
                AP4_Atom* udta = container->FindChild("odhe/udta");
                if (udta) {
                    AP4_ContainerAtom* udta_container = AP4_DYNAMIC_CAST(AP4_ContainerAtom, udta);
                    if (udta_container) {
                        ParseUdta(udta_container, "dcf");
                    }
                }
            }
            atom_item = atom_item->GetNext();
        }
    } 
}

/*----------------------------------------------------------------------
|   AP4_MetaData::ParseMoov
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaData::ParseMoov(AP4_MoovAtom* moov)
{
    // look for a 'meta' atom with 'hdlr' type 'mdir'
    AP4_HdlrAtom* hdlr = AP4_DYNAMIC_CAST(AP4_HdlrAtom, moov->FindChild("udta/meta/hdlr"));
    if (hdlr == NULL || hdlr->GetHandlerType() != AP4_HANDLER_TYPE_MDIR) return AP4_ERROR_NO_SUCH_ITEM;

    // get the list of entries
    AP4_ContainerAtom* ilst = AP4_DYNAMIC_CAST(AP4_ContainerAtom, moov->FindChild("udta/meta/ilst"));
    if (ilst == NULL) return AP4_ERROR_NO_SUCH_ITEM;
    
    AP4_List<AP4_Atom>::Item* ilst_item = ilst->GetChildren().FirstItem();
    while (ilst_item) {
        AP4_ContainerAtom* entry_atom = AP4_DYNAMIC_CAST(AP4_ContainerAtom, ilst_item->GetData()); 
        if (entry_atom) {
            AddIlstEntries(entry_atom, "meta");
        }
        ilst_item = ilst_item->GetNext();
    }
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MetaData::ParseUdta
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaData::ParseUdta(AP4_ContainerAtom* udta, const char* namespc)
{
    // check that the atom is indeed a 'udta' atom
    if (udta->GetType() != AP4_ATOM_TYPE_UDTA) {
        return AP4_ERROR_INVALID_PARAMETERS;
    }
    
    AP4_List<AP4_Atom>::Item* udta_item = udta->GetChildren().FirstItem();
    for (; udta_item; udta_item = udta_item->GetNext()) {
        AP4_3GppLocalizedStringAtom* _3gpp_atom = AP4_DYNAMIC_CAST(AP4_3GppLocalizedStringAtom, udta_item->GetData()); 
        if (_3gpp_atom) {
            Add3GppEntry(_3gpp_atom, namespc);
            continue;
        } 
        
        AP4_DcfStringAtom* dcfs_atom = AP4_DYNAMIC_CAST(AP4_DcfStringAtom, udta_item->GetData());
        if (dcfs_atom) {
            AddDcfStringEntry(dcfs_atom, namespc);
            continue;
        } 

        AP4_DcfdAtom* dcfd_atom = AP4_DYNAMIC_CAST(AP4_DcfdAtom, udta_item->GetData());
        if (dcfd_atom) {
            AddDcfdEntry(dcfd_atom, namespc);
        }
    }
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MetaData::~AP4_MetaData
+---------------------------------------------------------------------*/
AP4_MetaData::~AP4_MetaData()
{
    m_Entries.DeleteReferences();
}

/*----------------------------------------------------------------------
|   AP4_MetaData::ResolveKeyName
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaData::ResolveKeyName(AP4_Atom::Type atom_type, AP4_String& value)
{
    const char* key_name = NULL;
    char        four_cc[5];

    // look for a match in the key infos
    for (unsigned int i=0; 
         i<sizeof(AP4_MetaData_KeyInfos)/sizeof(AP4_MetaData_KeyInfos[0]); 
         i++) {
        if (AP4_MetaData_KeyInfos[i].four_cc == atom_type) {
            key_name = AP4_MetaData_KeyInfos[i].name;
            break;
        }
    }
    if (key_name == NULL) {
        // this key was not found in the key infos, create a name for it
        AP4_FormatFourChars(four_cc, (AP4_UI32)atom_type);
        key_name = four_cc;
    }
    value = key_name;
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MetaData::AddIlstEntries
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaData::AddIlstEntries(AP4_ContainerAtom* atom, const char* namespc)
{
    AP4_MetaData::Value* value = NULL;

    if (atom->GetType() == AP4_ATOM_TYPE_dddd) {
        // look for the namespace
        AP4_MetaDataStringAtom* mean = static_cast<AP4_MetaDataStringAtom*>(atom->GetChild(AP4_ATOM_TYPE_MEAN));
        if (mean == NULL) return AP4_ERROR_INVALID_FORMAT;

        // look for the name
        AP4_MetaDataStringAtom* name = static_cast<AP4_MetaDataStringAtom*>(atom->GetChild(AP4_ATOM_TYPE_NAME));
        if (name == NULL) return AP4_ERROR_INVALID_FORMAT;

        // get the value
        AP4_DataAtom* data_atom = static_cast<AP4_DataAtom*>(atom->GetChild(AP4_ATOM_TYPE_DATA));
        value = new AP4_AtomMetaDataValue(data_atom, atom->GetType());
        if (value == NULL) return AP4_ERROR_INVALID_FORMAT;
        
        return m_Entries.Add(new Entry(name->GetValue().GetChars(),
                                       mean->GetValue().GetChars(),
                                       value));
    } else {
        const char* key_name = NULL;
        char        four_cc[5];

        // convert the atom type to a name
        AP4_FormatFourChars(four_cc, (AP4_UI32)atom->GetType());
        key_name = four_cc;

        // add one entry for each data atom
        AP4_List<AP4_Atom>::Item* data_item = atom->GetChildren().FirstItem();
        while (data_item) {
            AP4_Atom* item_atom = data_item->GetData();
            if (item_atom->GetType() == AP4_ATOM_TYPE_DATA) {
                AP4_DataAtom* data_atom = static_cast<AP4_DataAtom*>(item_atom);
                value = new AP4_AtomMetaDataValue(data_atom, atom->GetType());
                m_Entries.Add(new Entry(key_name, namespc, value));
            }
            data_item = data_item->GetNext();
        }

        return AP4_SUCCESS;
    }
}

/*----------------------------------------------------------------------
|   AP4_MetaData::Add3GppEntry
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaData::Add3GppEntry(AP4_3GppLocalizedStringAtom* atom, const char* namespc)
{
    AP4_String key_name;
    ResolveKeyName(atom->GetType(), key_name);
    
    const char* language = NULL;
    if (atom->GetLanguage()[0]) {
        language = atom->GetLanguage();
    }
    AP4_MetaData::Value* value = new AP4_StringMetaDataValue(atom->GetValue().GetChars(),
                                                             language);
    m_Entries.Add(new Entry(key_name.GetChars(), namespc, value));
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MetaData::AddDcfStringEntry
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaData::AddDcfStringEntry(AP4_DcfStringAtom* atom, const char* namespc)
{
    AP4_String key_name;
    ResolveKeyName(atom->GetType(), key_name);
    
    AP4_MetaData::Value* value = new AP4_StringMetaDataValue(atom->GetValue().GetChars());
    m_Entries.Add(new Entry(key_name.GetChars(), namespc, value));
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MetaData::AddDcfdEntry
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaData::AddDcfdEntry(AP4_DcfdAtom* atom, const char* namespc)
{
    AP4_String key_name;
    ResolveKeyName(atom->GetType(), key_name);
    
    AP4_MetaData::Value* value = new AP4_IntegerMetaDataValue(AP4_MetaData::Value::TYPE_INT_32_BE, 
                                                              atom->GetDuration());
    m_Entries.Add(new Entry(key_name.GetChars(), namespc, value));
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MetaData::Value::MapDataTypeToCategory
+---------------------------------------------------------------------*/
AP4_MetaData::Value::TypeCategory 
AP4_MetaData::Value::MapTypeToCategory(Type type)
{
    switch (type) {
        case AP4_MetaData::Value::TYPE_INT_08_BE:
        case AP4_MetaData::Value::TYPE_INT_16_BE:
        case AP4_MetaData::Value::TYPE_INT_32_BE:
            return AP4_MetaData::Value::TYPE_CATEGORY_INTEGER;

        case AP4_MetaData::Value::TYPE_STRING_UTF_8:
        case AP4_MetaData::Value::TYPE_STRING_UTF_16:
        case AP4_MetaData::Value::TYPE_STRING_PASCAL:
            return AP4_MetaData::Value::TYPE_CATEGORY_STRING;

        case AP4_MetaData::Value::TYPE_FLOAT_32_BE:
        case AP4_MetaData::Value::TYPE_FLOAT_64_BE:
            return AP4_MetaData::Value::TYPE_CATEGORY_FLOAT;
            
        default:
            return AP4_MetaData::Value::TYPE_CATEGORY_BINARY;
    }
}

/*----------------------------------------------------------------------
|   AP4_MetaData::Value::GetTypeCategory
+---------------------------------------------------------------------*/
AP4_MetaData::Value::TypeCategory 
AP4_MetaData::Value::GetTypeCategory() const
{
    return MapTypeToCategory(m_Type);
}

/*----------------------------------------------------------------------
|   AP4_MetaData::Entry::ToAtom
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaData::Entry::ToAtom(AP4_Atom*& atom) const
{
    atom = NULL;
    
    if (m_Value == NULL) {
        return AP4_ERROR_INVALID_PARAMETERS;
    }
    
    if (m_Key.GetNamespace() == "meta") {
        // convert the name into an atom type
        if (m_Key.GetName().GetLength() != 4) {
            // the name is not in the right format
            return AP4_ERROR_INVALID_PARAMETERS;
        }
        AP4_Atom::Type atom_type = AP4_Atom::TypeFromString(m_Key.GetName().GetChars()); 
                                   
        // create a container atom for the data
        AP4_ContainerAtom* container = new AP4_ContainerAtom(atom_type);

        // add the data atom
        AP4_DataAtom* data = new AP4_DataAtom(*m_Value);
        container->AddChild(data);
        
        atom = container;
        return AP4_SUCCESS;
    } else if (m_Key.GetNamespace() == "dcf") {
        // convert the name into an atom type
        if (m_Key.GetName().GetLength() != 4) {
            // the name is not in the right format
            return AP4_ERROR_INVALID_PARAMETERS;
        }
        AP4_Atom::Type atom_type = AP4_Atom::TypeFromString(m_Key.GetName().GetChars()); 

        if (AP4_MetaDataAtomTypeHandler::IsTypeInList(atom_type, 
            AP4_MetaDataAtomTypeHandler::DcfStringTypeList)) {
            AP4_String atom_value = m_Value->ToString();
            atom = new AP4_DcfStringAtom(atom_type, atom_value.GetChars());
            return AP4_SUCCESS;
        } else if (AP4_MetaDataAtomTypeHandler::IsTypeInList(atom_type, 
                   AP4_MetaDataAtomTypeHandler::_3gppLocalizedStringTypeList)) {
            AP4_String atom_value = m_Value->ToString();
            const char* language = "eng"; // default
            if (m_Value->GetLanguage().GetLength() != 0) {
                language = m_Value->GetLanguage().GetChars();
            }
            atom = new AP4_3GppLocalizedStringAtom(atom_type, language, atom_value.GetChars());
            return AP4_SUCCESS;
        } else if (atom_type == AP4_ATOM_TYPE_DCFD) {
            atom = new AP4_DcfdAtom(m_Value->ToInteger());
            return AP4_SUCCESS;
        }
    } else {
        // create a '----' atom
        AP4_ContainerAtom* container = new AP4_ContainerAtom(AP4_ATOM_TYPE_dddd);
        
        // add a 'mean' string
        container->AddChild(new AP4_MetaDataStringAtom(AP4_ATOM_TYPE_MEAN, m_Key.GetNamespace().GetChars()));
        
        // add a 'name' string
        container->AddChild(new AP4_MetaDataStringAtom(AP4_ATOM_TYPE_NAME, m_Key.GetName().GetChars()));

        // add the data atom
        AP4_DataAtom* data = new AP4_DataAtom(*m_Value);
        container->AddChild(data);
        
        atom = container;
        return AP4_SUCCESS;
    }

    // not supported
    return AP4_ERROR_NOT_SUPPORTED;
}

/*----------------------------------------------------------------------
|   AP4_MetaData::Entry::FindInIlst
+---------------------------------------------------------------------*/
AP4_ContainerAtom*
AP4_MetaData::Entry::FindInIlst(AP4_ContainerAtom* ilst) const
{
    if (m_Key.GetNamespace() == "meta") {
        AP4_Atom::Type atom_type = AP4_Atom::TypeFromString(m_Key.GetName().GetChars());
        return AP4_DYNAMIC_CAST(AP4_ContainerAtom, ilst->GetChild(atom_type));
    } else {
        AP4_List<AP4_Atom>::Item* ilst_item = ilst->GetChildren().FirstItem();
        while (ilst_item) {
            AP4_ContainerAtom* entry_atom = AP4_DYNAMIC_CAST(AP4_ContainerAtom, ilst_item->GetData()); 
            if (entry_atom) {
                AP4_MetaDataStringAtom* mean = static_cast<AP4_MetaDataStringAtom*>(entry_atom->GetChild(AP4_ATOM_TYPE_MEAN));
                AP4_MetaDataStringAtom* name = static_cast<AP4_MetaDataStringAtom*>(entry_atom->GetChild(AP4_ATOM_TYPE_NAME));
                if (mean && name &&
                    mean->GetValue() == m_Key.GetNamespace() &&
                    name->GetValue() == m_Key.GetName()) {
                    return entry_atom;
                }
            }
            ilst_item = ilst_item->GetNext();
        }
    }
    
    // not found
    return NULL;
}
    
/*----------------------------------------------------------------------
|   AP4_MetaData::Entry::AddToFileIlst
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaData::Entry::AddToFileIlst(AP4_File& file, AP4_Ordinal index)
{
    // check that we have a correct entry
    if (m_Value == NULL) return AP4_ERROR_INVALID_STATE;

    // convert the entry into an atom
    AP4_Atom* atom;
    AP4_Result result = ToAtom(atom);
    if (AP4_FAILED(result)) return result;
    AP4_ContainerAtom* entry_atom = AP4_DYNAMIC_CAST(AP4_ContainerAtom, atom);
    if (entry_atom == NULL) {
        return AP4_ERROR_INVALID_FORMAT;
    }

    // look for the 'moov'
    AP4_Movie* movie = file.GetMovie();
    if (movie == NULL) return AP4_ERROR_INVALID_FORMAT;
    AP4_MoovAtom* moov = movie->GetMoovAtom();
    if (moov == NULL) return AP4_ERROR_INVALID_FORMAT;
    
    // look for 'udta', and create if it does not exist 
    AP4_ContainerAtom* udta = AP4_DYNAMIC_CAST(AP4_ContainerAtom, moov->FindChild("udta", true));
    if (udta == NULL) return AP4_ERROR_INTERNAL;
    
    // look for 'meta', and create if it does not exist ('meta' is a FULL atom)
    AP4_ContainerAtom* meta = AP4_DYNAMIC_CAST(AP4_ContainerAtom, udta->FindChild("meta", true, true));
    if (meta == NULL) return AP4_ERROR_INTERNAL;

    // look for a 'hdlr' atom type 'mdir'
    AP4_HdlrAtom* hdlr = AP4_DYNAMIC_CAST(AP4_HdlrAtom, meta->FindChild("hdlr"));
    if (hdlr == NULL) {
        hdlr = new AP4_HdlrAtom(AP4_HANDLER_TYPE_MDIR, "");
        meta->AddChild(hdlr);
    } else {
        if (hdlr->GetHandlerType() != AP4_HANDLER_TYPE_MDIR) {
            return AP4_ERROR_INVALID_FORMAT;
        }
    }

    // get/create the list of entries
    AP4_ContainerAtom* ilst = AP4_DYNAMIC_CAST(AP4_ContainerAtom, meta->FindChild("ilst", true));
    if (ilst == NULL) return AP4_ERROR_INTERNAL;
    
    // look if there is already a container for this entry
    AP4_ContainerAtom* existing = FindInIlst(ilst);
    if (existing == NULL) {
        // just add the one we have
        ilst->AddChild(entry_atom);
    } else {
        // add the entry's data to the existing entry
        AP4_DataAtom* data_atom = AP4_DYNAMIC_CAST(AP4_DataAtom, entry_atom->GetChild(AP4_ATOM_TYPE_DATA));
        if (data_atom == NULL) return AP4_ERROR_INTERNAL;
        entry_atom->RemoveChild(data_atom);
        existing->AddChild(data_atom, index);
        delete entry_atom;
    }
    
    return AP4_SUCCESS;    
}

/*----------------------------------------------------------------------
|   AP4_MetaData::Entry::AddToFileDcf
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaData::Entry::AddToFileDcf(AP4_File& file, AP4_Ordinal index)
{
    // check that we have a correct entry
    if (m_Value == NULL) return AP4_ERROR_INVALID_STATE;
    
    // look for 'odrm/odhe'
    AP4_ContainerAtom* odhe = AP4_DYNAMIC_CAST(AP4_ContainerAtom, file.FindChild("odrm/odhe"));
    if (odhe == NULL) return AP4_ERROR_NO_SUCH_ITEM;

    // get/create the list of entries
    AP4_ContainerAtom* udta = AP4_DYNAMIC_CAST(AP4_ContainerAtom, odhe->FindChild("udta", true));
    if (udta == NULL) return AP4_ERROR_INTERNAL;
    
    // convert the entry into an atom
    AP4_Atom* data_atom;
    AP4_Result result = ToAtom(data_atom);
    if (AP4_FAILED(result)) return result;

    // add the entry's data to the container
    return udta->AddChild(data_atom, index);
}

/*----------------------------------------------------------------------
|   AP4_MetaData::Entry::AddToFile
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaData::Entry::AddToFile(AP4_File& file, AP4_Ordinal index)
{
    // check that we have a correct entry
    if (m_Value == NULL) return AP4_ERROR_INVALID_STATE;
 
    // check the namespace of the key to know where to add the atom
    if (m_Key.GetNamespace() == "meta") {
        return AddToFileIlst(file, index);
    } else if (m_Key.GetNamespace() == "dcf") {
        return AddToFileDcf(file, index);
    } else {
        // custom namespace
        return AddToFileIlst(file, index);
    }
}

/*----------------------------------------------------------------------
|   AP4_MetaData::Entry::RemoveFromFileIlst
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaData::Entry::RemoveFromFileIlst(AP4_File& file, AP4_Ordinal index)
{
    // look for the 'moov'
    AP4_Movie* movie = file.GetMovie();
    if (movie == NULL) return AP4_ERROR_INVALID_FORMAT;
    AP4_MoovAtom* moov = movie->GetMoovAtom();
    if (moov == NULL) return AP4_ERROR_INVALID_FORMAT;
    
    // look for 'udta/meta/ilst'
    AP4_ContainerAtom* ilst = AP4_DYNAMIC_CAST(AP4_ContainerAtom, moov->FindChild("udta/meta/ilst"));
    if (ilst == NULL) return AP4_ERROR_NO_SUCH_ITEM;
    
    // look if there is already a container for this entry
    AP4_ContainerAtom* existing = FindInIlst(ilst);
    if (existing == NULL) return AP4_ERROR_NO_SUCH_ITEM;
            
    // remove the data atom in the entry
    AP4_Result result = existing->DeleteChild(AP4_ATOM_TYPE_DATA, index);
    if (AP4_FAILED(result)) return result;
    
    // if the entry is empty, remove it
    if (existing->GetChildren().ItemCount() == 0) {
        ilst->RemoveChild(existing);
        delete existing;
    }
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MetaData::Entry::RemoveFromFileDcf
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaData::Entry::RemoveFromFileDcf(AP4_File& file, AP4_Ordinal index)
{
    // look for 'odrm/odhe/udta'
    AP4_ContainerAtom* udta = AP4_DYNAMIC_CAST(AP4_ContainerAtom, file.FindChild("odrm/odhe/udta"));
    if (udta == NULL) return AP4_ERROR_NO_SUCH_ITEM;
                
    // remove the data atom in the entry
    AP4_UI32 type = AP4_BytesToUInt32BE((const unsigned char*)m_Key.GetName().GetChars());
    AP4_Result result = udta->DeleteChild(type, index);
    if (AP4_FAILED(result)) return result;
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MetaData::Entry::RemoveFromFile
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaData::Entry::RemoveFromFile(AP4_File& file, AP4_Ordinal index)
{
    // check the namespace of the key to know where to add the atom
    if (m_Key.GetNamespace() == "meta") {
        return RemoveFromFileIlst(file, index);
    } else if (m_Key.GetNamespace() == "dcf") {
        return RemoveFromFileDcf(file, index);
    } else {
        // unsupported namespace
        return AP4_ERROR_NOT_SUPPORTED;
    }
}

/*----------------------------------------------------------------------
|   AP4_StringMetaDataValue::ToString
+---------------------------------------------------------------------*/
AP4_String
AP4_StringMetaDataValue::ToString() const
{
    return m_Value;
}

/*----------------------------------------------------------------------
|   AP4_StringMetaDataValue::ToBytes
+---------------------------------------------------------------------*/
AP4_Result
AP4_StringMetaDataValue::ToBytes(AP4_DataBuffer& /* bytes */) const
{
    return AP4_ERROR_NOT_SUPPORTED;
}

/*----------------------------------------------------------------------
|   AP4_StringMetaDataValue::ToInteger
+---------------------------------------------------------------------*/
long
AP4_StringMetaDataValue::ToInteger() const
{
    return 0;
}

/*----------------------------------------------------------------------
|   AP4_IntegerMetaDataValue::ToString
+---------------------------------------------------------------------*/
AP4_String
AP4_IntegerMetaDataValue::ToString() const
{
    char value[16];
    AP4_FormatString(value, sizeof(value), "%ld", m_Value);
    return AP4_String(value);
}

/*----------------------------------------------------------------------
|   AP4_IntegerMetaDataValue::ToBytes
+---------------------------------------------------------------------*/
AP4_Result
AP4_IntegerMetaDataValue::ToBytes(AP4_DataBuffer& /* bytes */) const
{
    return AP4_ERROR_NOT_SUPPORTED;
}

/*----------------------------------------------------------------------
|   AP4_IntegerMetaDataValue::ToInteger
+---------------------------------------------------------------------*/
long
AP4_IntegerMetaDataValue::ToInteger() const
{
    return m_Value;
}

/*----------------------------------------------------------------------
|   AP4_BinaryMetaDataValue::ToString
+---------------------------------------------------------------------*/
AP4_String
AP4_BinaryMetaDataValue::ToString() const
{
    return AP4_String(); // not supported
}

/*----------------------------------------------------------------------
|   AP4_BinaryMetaDataValue::ToBytes
+---------------------------------------------------------------------*/
AP4_Result
AP4_BinaryMetaDataValue::ToBytes(AP4_DataBuffer& bytes) const
{
    bytes.SetDataSize(m_Value.GetDataSize());
    AP4_CopyMemory(bytes.UseData(), m_Value.GetData(), m_Value.GetDataSize());
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_BinaryMetaDataValue::ToInteger
+---------------------------------------------------------------------*/
long
AP4_BinaryMetaDataValue::ToInteger() const
{
    return 0; // NOT SUPPORTED
}

/*----------------------------------------------------------------------
|   AP4_AtomMetaDataValue::AP4_AtomMetaDataValue
+---------------------------------------------------------------------*/
AP4_AtomMetaDataValue::AP4_AtomMetaDataValue(AP4_DataAtom*  atom,
                                             AP4_UI32       parent_type) :
    Value(atom->GetValueType()),
    m_DataAtom(atom)
{
    switch (parent_type) {
        case AP4_ATOM_TYPE_GNRE:
            m_Meaning = MEANING_ID3_GENRE;
            break;

        case AP4_ATOM_TYPE_CPIL:
            m_Meaning = MEANING_BOOLEAN;
            break;

        case AP4_ATOM_TYPE_PGAP:
        case AP4_ATOM_TYPE_PCST:
            m_Meaning = MEANING_BOOLEAN;
            break;

        case AP4_ATOM_TYPE_STIK:
            m_Meaning = MEANING_FILE_KIND;
            break;

        case AP4_ATOM_TYPE_PURL:
        case AP4_ATOM_TYPE_EGID:
            m_Meaning = MEANING_BINARY_ENCODED_CHARS;
            break;
            
        default:
            break;
    }
}

/*----------------------------------------------------------------------
|   AP4_AtomMetaDataValue::ToString
+---------------------------------------------------------------------*/
AP4_String 
AP4_AtomMetaDataValue::ToString() const
{
    char string[256] = "";

    AP4_MetaData::Value::Type value_type = m_DataAtom->GetValueType();
    switch (AP4_MetaData::Value::MapTypeToCategory(value_type)) {
        case AP4_MetaData::Value::TYPE_CATEGORY_INTEGER: 
            {
                long value;
                if (AP4_SUCCEEDED(m_DataAtom->LoadInteger(value))) {
                    if (m_Meaning == MEANING_BOOLEAN) {
                        if (value) {
                            return "True";
                        } else {
                            return "False";
                        }
                    } else if (m_Meaning == MEANING_FILE_KIND) {
                        if (value >= 0 && ((unsigned int)value) <= sizeof(Ap4StikNames)/sizeof(Ap4StikNames[0])) {
                            AP4_FormatString(string, sizeof(string), "(%ld) %s", value, Ap4StikNames[value]);
                        } else {
                            return "Unknown";
                        }
                    } else {
                        AP4_FormatString(string, sizeof(string), "%ld", value);
                    }
                }
                return AP4_String((const char*)string);
                break;
            }

        case AP4_MetaData::Value::TYPE_CATEGORY_STRING:
            {
                AP4_String* category_string;
                if (AP4_SUCCEEDED(m_DataAtom->LoadString(category_string))) {
                    AP4_String result(*category_string);
                    delete category_string;
                    return result;
                }
                break;
            }

        case AP4_MetaData::Value::TYPE_CATEGORY_BINARY:
            {
                AP4_DataBuffer data;
                if (AP4_SUCCEEDED(m_DataAtom->LoadBytes(data))) {
                    if (m_Meaning == MEANING_ID3_GENRE && data.GetDataSize() == 2) {
                        unsigned int genre = (data.GetData()[0])*256+data.GetData()[1];
                        if (genre >= 1 && genre <= sizeof(Ap4Id3Genres)/sizeof(Ap4Id3Genres[0])) {
                            AP4_FormatString(string, sizeof(string), "(%d) %s", genre, Ap4Id3Genres[genre-1]);
                            return AP4_String((const char*)string);
                        } else {
                            return "Unknown";
                        }
                    } else if (m_Meaning == MEANING_BINARY_ENCODED_CHARS) {
                        AP4_String result;
                        result.Assign((const char*)data.GetData(), data.GetDataSize());
                        return result;
                    } else {
                        unsigned int dump_length = data.GetDataSize();
                        bool truncate = false;
                        if (dump_length > 16) {
                            dump_length = 16;
                            truncate = true;
                        }
                        char* out = string;
                        for (unsigned int i=0; i<dump_length; i++) {
                            AP4_FormatString(out, sizeof(string)-(out-string), "%02x ", data.GetData()[i]);
                            out += 3;
                        }
                        if (truncate) {
                            *out++='.'; *out++='.'; *out++='.'; *out++=' ';
                        }
                        AP4_FormatString(out, sizeof(string)-(out-string), "[%d bytes]", (int)data.GetDataSize());
                    }
                }
                return AP4_String(string);
            }
        default:
            return AP4_String();
    }

    return AP4_String();
}

/*----------------------------------------------------------------------
|   AP4_AtomMetaDataValue::ToBytes
+---------------------------------------------------------------------*/
AP4_Result
AP4_AtomMetaDataValue::ToBytes(AP4_DataBuffer& bytes) const
{
    return m_DataAtom->LoadBytes(bytes);
}

/*----------------------------------------------------------------------
|   AP4_AtomMetaDataValue::ToInteger
+---------------------------------------------------------------------*/
long       
AP4_AtomMetaDataValue::ToInteger() const
{
    long value;
    if (AP4_SUCCEEDED(m_DataAtom->LoadInteger(value))) {
        return value;
    } else {
        return 0;
    }
}

/*----------------------------------------------------------------------
|   AP4_DataAtom::AP4_DataAtom
+---------------------------------------------------------------------*/
AP4_DataAtom::AP4_DataAtom(const AP4_MetaData::Value& value) :
    AP4_Atom(AP4_ATOM_TYPE_DATA, AP4_ATOM_HEADER_SIZE),
    m_DataType(DATA_TYPE_BINARY)
{
    AP4_MemoryByteStream* memory = new AP4_MemoryByteStream(256);
    AP4_Size payload_size = 8;
    m_Source = memory;
    
    switch (value.GetType()) {
        case AP4_MetaData::Value::TYPE_STRING_UTF_8: {
            m_DataType = DATA_TYPE_STRING_UTF_8;
            AP4_String string_value = value.ToString();
            if (string_value.GetLength()) {
                memory->Write(string_value.GetChars(), string_value.GetLength());
            }
            payload_size += string_value.GetLength();
            break;
        }
         
        case AP4_MetaData::Value::TYPE_INT_08_BE: {
            m_DataType = DATA_TYPE_SIGNED_INT_BE;
            AP4_UI08 int_value = (AP4_UI08)value.ToInteger();
            memory->Write(&int_value, 1);
            payload_size += 1;
            break;
        }
            
        case AP4_MetaData::Value::TYPE_INT_16_BE: {
            m_DataType = DATA_TYPE_SIGNED_INT_BE;
            AP4_UI16 int_value = (AP4_UI16)value.ToInteger();
            memory->Write(&int_value, 2);
            payload_size += 2;
            break;
        }

        case AP4_MetaData::Value::TYPE_INT_32_BE: {
            m_DataType = DATA_TYPE_SIGNED_INT_BE;
            AP4_UI32 int_value = (AP4_UI32)value.ToInteger();
            memory->Write(&int_value, 4);
            payload_size += 4;
            break;
        }

        case AP4_MetaData::Value::TYPE_JPEG:
            m_DataType = DATA_TYPE_JPEG;
            // FALLTHROUGH
        case AP4_MetaData::Value::TYPE_GIF: 
            if (m_DataType == DATA_TYPE_BINARY) m_DataType = DATA_TYPE_GIF;
            // FALLTHROUGH
        case AP4_MetaData::Value::TYPE_BINARY: {
            AP4_DataBuffer buffer;
            value.ToBytes(buffer);
            if (buffer.GetDataSize()) {
                memory->Write(buffer.GetData(), buffer.GetDataSize());
            }
            payload_size += buffer.GetDataSize();
            break;
        }

        default:
            break;
    }
    
    const AP4_String& language = value.GetLanguage();
    if (language == "en") {
        m_DataLang = LANGUAGE_ENGLISH;
    } else {
        // default
        m_DataLang = LANGUAGE_ENGLISH;
    }
    
    m_Size32 += payload_size;
}

/*----------------------------------------------------------------------
|   AP4_DataAtom::AP4_DataAtom
+---------------------------------------------------------------------*/
AP4_DataAtom::AP4_DataAtom(AP4_UI32 size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_DATA, size)
{
    if (size < AP4_ATOM_HEADER_SIZE+8) return;

    AP4_UI32 i;
    stream.ReadUI32(i); m_DataType = (DataType)i;
    stream.ReadUI32(i); m_DataLang = (DataLang)i;

    // the stream for the data is a substream of this source
    AP4_Position data_offset;
    stream.Tell(data_offset);
    AP4_Size data_size = size-AP4_ATOM_HEADER_SIZE-8;
    m_Source = new AP4_SubStream(stream, data_offset, data_size);
}

/*----------------------------------------------------------------------
|   AP4_DataAtom::~AP4_DataAtom
+---------------------------------------------------------------------*/
AP4_DataAtom::~AP4_DataAtom()
{
    delete(m_Source);
}

/*----------------------------------------------------------------------
|   AP4_DataAtom::GetValueType
+---------------------------------------------------------------------*/
AP4_MetaData::Value::Type
AP4_DataAtom::GetValueType()
{
    switch (m_DataType) {
        case DATA_TYPE_BINARY:
            return AP4_MetaData::Value::TYPE_BINARY;

        case DATA_TYPE_SIGNED_INT_BE:
            switch (m_Size32-16) {
                case 1: return AP4_MetaData::Value::TYPE_INT_08_BE;
                case 2: return AP4_MetaData::Value::TYPE_INT_16_BE;
                case 4: return AP4_MetaData::Value::TYPE_INT_32_BE;
                default: return AP4_MetaData::Value::TYPE_BINARY;
            }
            break;
            
        case DATA_TYPE_STRING_UTF_8:
            return AP4_MetaData::Value::TYPE_STRING_UTF_8;
            
        case DATA_TYPE_STRING_UTF_16:
            return AP4_MetaData::Value::TYPE_STRING_UTF_16;
            
        case DATA_TYPE_STRING_PASCAL:
            return AP4_MetaData::Value::TYPE_STRING_PASCAL;

        case DATA_TYPE_GIF:
            return AP4_MetaData::Value::TYPE_GIF;

        case DATA_TYPE_JPEG:
            return AP4_MetaData::Value::TYPE_JPEG;

        default:
            return AP4_MetaData::Value::TYPE_BINARY;
    }
    
    return AP4_MetaData::Value::TYPE_BINARY;
}

/*----------------------------------------------------------------------
|   AP4_DataAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_DataAtom::WriteFields(AP4_ByteStream& stream)
{
    stream.WriteUI32(m_DataType);
    stream.WriteUI32(m_DataLang);
    if (m_Source) {
        AP4_LargeSize size = 0;
        m_Source->GetSize(size);
        m_Source->Seek(0);
        m_Source->CopyTo(stream, size);
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_DataAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_DataAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("type", m_DataType);
    inspector.AddField("lang", m_DataLang);
    if (m_DataType == DATA_TYPE_STRING_UTF_8) {
        AP4_String* str;
        if (AP4_SUCCEEDED(LoadString(str))) {
            inspector.AddField("value", str->GetChars());
            delete str;
        }
    } else if (m_DataType == DATA_TYPE_SIGNED_INT_BE) {
        long value;
        if (AP4_SUCCEEDED(LoadInteger(value))) {
            inspector.AddField("value", value);
        }
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_DataAtom::LoadString
+---------------------------------------------------------------------*/
AP4_Result
AP4_DataAtom::LoadString(AP4_String*& string)
{
    if (m_Source == NULL) {
        string = new AP4_String();
        return AP4_SUCCESS;
    } else {
        // create a string with enough capactiy for the data
        AP4_LargeSize size = 0;
        m_Source->GetSize(size);
        if (size > AP4_DATA_ATOM_MAX_SIZE) return AP4_ERROR_OUT_OF_RANGE;
        string = new AP4_String((AP4_Size)size);

        // read from the start of the stream
        m_Source->Seek(0);
        AP4_Result result = m_Source->Read(string->UseChars(), (AP4_Size)size);
        if (AP4_FAILED(result)) {
            delete string;
            string = NULL;
        }

        return result;
    }
}

/*----------------------------------------------------------------------
|   AP4_DataAtom::LoadBytes
+---------------------------------------------------------------------*/
AP4_Result
AP4_DataAtom::LoadBytes(AP4_DataBuffer& bytes)
{
    if (m_Source == NULL) {
        bytes.SetDataSize(0);
        return AP4_SUCCESS;
    }
    AP4_LargeSize size = 0;
    m_Source->GetSize(size);
    if (size > AP4_DATA_ATOM_MAX_SIZE) return AP4_ERROR_OUT_OF_RANGE;
    bytes.SetDataSize((AP4_Size)size);
    m_Source->Seek(0);
    AP4_Result result = m_Source->Read(bytes.UseData(), (AP4_Size)size);
    if (AP4_FAILED(result)) {
        bytes.SetDataSize(0);
    }
    return result;
}

/*----------------------------------------------------------------------
|   AP4_DataAtom::LoadInteger
+---------------------------------------------------------------------*/
AP4_Result
AP4_DataAtom::LoadInteger(long& value) 
{
    AP4_Result result = AP4_FAILURE;
    value = 0;
    if (m_Source == NULL) return AP4_SUCCESS;
    AP4_LargeSize size = 0;
    m_Source->GetSize(size);
    if (size > 4) {
        return AP4_ERROR_OUT_OF_RANGE;
    }
    unsigned char bytes[4];
    m_Source->Seek(0);
    m_Source->Read(bytes, (AP4_Size)size);
    result = AP4_SUCCESS;
    switch (size) {
        case 1: value = bytes[0]; break;
        case 2: value = AP4_BytesToInt16BE(bytes); break;
        case 4: value = AP4_BytesToInt32BE(bytes); break;
        default: value = 0; result = AP4_ERROR_INVALID_FORMAT; break;
    }
    return result;
}

/*----------------------------------------------------------------------
|   AP4_MetaDataStringAtom::AP4_MetaDataStringAtom
+---------------------------------------------------------------------*/
AP4_MetaDataStringAtom::AP4_MetaDataStringAtom(Type type, const char* value) :
    AP4_Atom(type, AP4_ATOM_HEADER_SIZE),
    m_Value(value)
{
    m_Size32 += 4+m_Value.GetLength();
}

/*----------------------------------------------------------------------
|   AP4_MetaDataStringAtom::AP4_MetaDataStringAtom
+---------------------------------------------------------------------*/
AP4_MetaDataStringAtom::AP4_MetaDataStringAtom(Type type, AP4_UI32 size, AP4_ByteStream& stream) :
    AP4_Atom(type, size),
    m_Value((AP4_Size)(size-AP4_ATOM_HEADER_SIZE-4))
{
    stream.ReadUI32(m_Reserved);
    stream.Read(m_Value.UseChars(), m_Value.GetLength());
}

/*----------------------------------------------------------------------
|   AP4_MetaDataStringAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaDataStringAtom::WriteFields(AP4_ByteStream& stream)
{
    stream.WriteUI32(m_Reserved);
    return stream.Write(m_Value.GetChars(), m_Value.GetLength());
}

/*----------------------------------------------------------------------
|   AP4_MetaDataStringAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_MetaDataStringAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("value", m_Value.GetChars());
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_3GppLocalizedStringAtom::Create
+---------------------------------------------------------------------*/
AP4_3GppLocalizedStringAtom*
AP4_3GppLocalizedStringAtom::Create(Type type, AP4_UI32 size, AP4_ByteStream& stream) 
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version != 0) return NULL;
    return new AP4_3GppLocalizedStringAtom(type, size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_3GppLocalizedStringAtom::AP4_3GppLocalizedStringAtom
+---------------------------------------------------------------------*/
AP4_3GppLocalizedStringAtom::AP4_3GppLocalizedStringAtom(Type        type, 
                                                         const char* language, 
                                                         const char* value) :
    AP4_Atom(type, AP4_FULL_ATOM_HEADER_SIZE+2, 0, 0),
    m_Value(value)
{
    m_Language[0] = language[0];
    m_Language[1] = language[1];
    m_Language[2] = language[2];
    m_Language[3] = language[3];
    
    m_Size32 += m_Value.GetLength()+1;
}

/*----------------------------------------------------------------------
|   AP4_3GppLocalizedStringAtom::AP4_3GppLocalizedStringAtom
+---------------------------------------------------------------------*/
AP4_3GppLocalizedStringAtom::AP4_3GppLocalizedStringAtom(Type            type, 
                                                         AP4_UI32        size, 
                                                         AP4_UI32        version,
                                                         AP4_UI32        flags,
                                                         AP4_ByteStream& stream) :
    AP4_Atom(type, size, version, flags)
{
    // read the language code
    AP4_UI16 packed_language;
    stream.ReadUI16(packed_language);
    m_Language[0] = 0x60+((packed_language>>10)&0x1F);
    m_Language[1] = 0x60+((packed_language>> 5)&0x1F);
    m_Language[2] = 0x60+((packed_language    )&0x1F);
    m_Language[3] = '\0';
    
    // read the value (should be a NULL-terminated string, but we'll
    // allow for strings that are not terminated)
    if (size > AP4_FULL_ATOM_HEADER_SIZE+2) {
        AP4_UI32 value_size = size-(AP4_FULL_ATOM_HEADER_SIZE+2);
        char* value = new char[value_size];
        stream.Read(value, value_size);
        m_Value.Assign(value, value_size);
        delete[] value;
    }
}

/*----------------------------------------------------------------------
|   AP4_3GppLocalizedStringAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_3GppLocalizedStringAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_UI16 packed_language = ((m_Language[0]-0x60)<<10) |
                               ((m_Language[1]-0x60)<< 5) |
                               ((m_Language[2]-0x60));
    stream.WriteUI16(packed_language);
    AP4_Size payload_size = (AP4_UI32)GetSize()-GetHeaderSize();
    if (payload_size < 2) return AP4_ERROR_INVALID_FORMAT;
    AP4_Size value_size = m_Value.GetLength()+1;
    if (value_size > payload_size-2) {
        value_size = payload_size-2;
    }
    stream.Write(m_Value.GetChars(), value_size);
    for (unsigned int i=value_size; i<payload_size-2; i++) {
        stream.WriteUI08(0);
    }
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_3GppLocalizedStringAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_3GppLocalizedStringAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("language", GetLanguage());
    inspector.AddField("value", m_Value.GetChars());
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_DcfStringAtom::Create
+---------------------------------------------------------------------*/
AP4_DcfStringAtom*
AP4_DcfStringAtom::Create(Type type, AP4_UI32 size, AP4_ByteStream& stream) 
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version != 0) return NULL;
    return new AP4_DcfStringAtom(type, size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_DcfStringAtom::AP4_DcfStringAtom
+---------------------------------------------------------------------*/
AP4_DcfStringAtom::AP4_DcfStringAtom(Type type, const char* value) :
    AP4_Atom(type, AP4_FULL_ATOM_HEADER_SIZE, 0, 0),
    m_Value(value)
{
    m_Size32 += m_Value.GetLength();
}

/*----------------------------------------------------------------------
|   AP4_DcfStringAtom::AP4_DcfStringAtom
+---------------------------------------------------------------------*/
AP4_DcfStringAtom::AP4_DcfStringAtom(Type            type, 
                                     AP4_UI32        size, 
                                     AP4_UI32        version,
                                     AP4_UI32        flags,
                                     AP4_ByteStream& stream) :
    AP4_Atom(type, size, version, flags)
{
    if (size > AP4_FULL_ATOM_HEADER_SIZE) {
        AP4_UI32 value_size = size-(AP4_FULL_ATOM_HEADER_SIZE);
        char* value = new char[value_size];
        stream.Read(value, value_size);
        m_Value.Assign(value, value_size);
        delete[] value;
    }
}

/*----------------------------------------------------------------------
|   AP4_DcfStringAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_DcfStringAtom::WriteFields(AP4_ByteStream& stream)
{
    if (m_Value.GetLength()) stream.Write(m_Value.GetChars(), m_Value.GetLength());
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_DcfStringAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_DcfStringAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("value", m_Value.GetChars());
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_DcfdAtom::Create
+---------------------------------------------------------------------*/
AP4_DcfdAtom*
AP4_DcfdAtom::Create(AP4_UI32 size, AP4_ByteStream& stream) 
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version != 0) return NULL;
    if (size != AP4_FULL_ATOM_HEADER_SIZE+4) return NULL;
    return new AP4_DcfdAtom(version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_DcfdAtom::AP4_DcfdAtom
+---------------------------------------------------------------------*/
AP4_DcfdAtom::AP4_DcfdAtom(AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_DCFD, AP4_FULL_ATOM_HEADER_SIZE+4, version, flags)
{
    stream.ReadUI32(m_Duration);
}

/*----------------------------------------------------------------------
|   AP4_DcfdAtom::AP4_DcfdAtom
+---------------------------------------------------------------------*/
AP4_DcfdAtom::AP4_DcfdAtom(AP4_UI32 duration) :
    AP4_Atom(AP4_ATOM_TYPE_DCFD, AP4_FULL_ATOM_HEADER_SIZE+4, 0, 0),
    m_Duration(duration)
{
}

/*----------------------------------------------------------------------
|   AP4_DcfdAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_DcfdAtom::WriteFields(AP4_ByteStream& stream)
{
    stream.WriteUI32(m_Duration);
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_DcfdAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_DcfdAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("duration", m_Duration);
    return AP4_SUCCESS;
}
