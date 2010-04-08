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

#ifndef _AP4_META_DATA_H_
#define _AP4_META_DATA_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Atom.h"
#include "Ap4AtomFactory.h"
#include "Ap4String.h"
#include "Ap4Array.h"
#include "Ap4List.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_File;
class AP4_MoovAtom;
class AP4_DataBuffer;
class AP4_ContainerAtom;
class AP4_DataAtom;
class AP4_3GppLocalizedStringAtom;
class AP4_DcfStringAtom;
class AP4_DcfdAtom;

/*----------------------------------------------------------------------
|   metadata keys
+---------------------------------------------------------------------*/
const AP4_Atom::Type AP4_ATOM_TYPE_DATA = AP4_ATOM_TYPE('d', 'a', 't', 'a'); // data
const AP4_Atom::Type AP4_ATOM_TYPE_MEAN = AP4_ATOM_TYPE('m', 'e', 'a', 'n'); // namespace
const AP4_Atom::Type AP4_ATOM_TYPE_NAME = AP4_ATOM_TYPE('n', 'a', 'm', 'e'); // name
const AP4_Atom::Type AP4_ATOM_TYPE_dddd = AP4_ATOM_TYPE('-', '-', '-', '-'); // free form
const AP4_Atom::Type AP4_ATOM_TYPE_cNAM = AP4_ATOM_TYPE(0xA9, 'n', 'a', 'm'); // name
const AP4_Atom::Type AP4_ATOM_TYPE_cART = AP4_ATOM_TYPE(0xA9, 'A', 'R', 'T'); // artist
const AP4_Atom::Type AP4_ATOM_TYPE_cCOM = AP4_ATOM_TYPE(0xA9, 'c', 'o', 'm'); // composer
const AP4_Atom::Type AP4_ATOM_TYPE_cWRT = AP4_ATOM_TYPE(0xA9, 'w', 'r', 't'); // writer
const AP4_Atom::Type AP4_ATOM_TYPE_cALB = AP4_ATOM_TYPE(0xA9, 'a', 'l', 'b'); // album
const AP4_Atom::Type AP4_ATOM_TYPE_cGEN = AP4_ATOM_TYPE(0xA9, 'g', 'e', 'n'); // genre
const AP4_Atom::Type AP4_ATOM_TYPE_cGRP = AP4_ATOM_TYPE(0xA9, 'g', 'r', 'p'); // group
const AP4_Atom::Type AP4_ATOM_TYPE_cDAY = AP4_ATOM_TYPE(0xA9, 'd', 'a', 'y'); // date
const AP4_Atom::Type AP4_ATOM_TYPE_cTOO = AP4_ATOM_TYPE(0xA9, 't', 'o', 'o'); // tool
const AP4_Atom::Type AP4_ATOM_TYPE_cCMT = AP4_ATOM_TYPE(0xA9, 'c', 'm', 't'); // comment
const AP4_Atom::Type AP4_ATOM_TYPE_cLYR = AP4_ATOM_TYPE(0xA9, 'l', 'y', 'r'); // lyrics
const AP4_Atom::Type AP4_ATOM_TYPE_TRKN = AP4_ATOM_TYPE('t', 'r', 'k', 'n'); // track#
const AP4_Atom::Type AP4_ATOM_TYPE_DISK = AP4_ATOM_TYPE('d', 'i', 's', 'k'); // disk#
const AP4_Atom::Type AP4_ATOM_TYPE_COVR = AP4_ATOM_TYPE('c', 'o', 'v', 'r'); // cover art
const AP4_Atom::Type AP4_ATOM_TYPE_DESC = AP4_ATOM_TYPE('d', 'e', 's', 'c'); // description
const AP4_Atom::Type AP4_ATOM_TYPE_CPIL = AP4_ATOM_TYPE('c', 'p', 'i', 'l'); // compilation?
const AP4_Atom::Type AP4_ATOM_TYPE_TMPO = AP4_ATOM_TYPE('t', 'm', 'p', 'o'); // tempo
const AP4_Atom::Type AP4_ATOM_TYPE_apID = AP4_ATOM_TYPE('a', 'p', 'I', 'D');
const AP4_Atom::Type AP4_ATOM_TYPE_cnID = AP4_ATOM_TYPE('c', 'n', 'I', 'D');
const AP4_Atom::Type AP4_ATOM_TYPE_cmID = AP4_ATOM_TYPE('c', 'm', 'I', 'D');
const AP4_Atom::Type AP4_ATOM_TYPE_atID = AP4_ATOM_TYPE('a', 't', 'I', 'D');
const AP4_Atom::Type AP4_ATOM_TYPE_plID = AP4_ATOM_TYPE('p', 'l', 'I', 'D');
const AP4_Atom::Type AP4_ATOM_TYPE_geID = AP4_ATOM_TYPE('g', 'e', 'I', 'D');
const AP4_Atom::Type AP4_ATOM_TYPE_sfID = AP4_ATOM_TYPE('s', 'f', 'I', 'D');
const AP4_Atom::Type AP4_ATOM_TYPE_akID = AP4_ATOM_TYPE('a', 'k', 'I', 'D');
const AP4_Atom::Type AP4_ATOM_TYPE_aART = AP4_ATOM_TYPE('a', 'A', 'R', 'T');
const AP4_Atom::Type AP4_ATOM_TYPE_TVNN = AP4_ATOM_TYPE('t', 'v', 'n', 'n'); // TV network
const AP4_Atom::Type AP4_ATOM_TYPE_TVSH = AP4_ATOM_TYPE('t', 'v', 's', 'h'); // TV show
const AP4_Atom::Type AP4_ATOM_TYPE_TVEN = AP4_ATOM_TYPE('t', 'v', 'e', 'n'); // TV episode name
const AP4_Atom::Type AP4_ATOM_TYPE_TVSN = AP4_ATOM_TYPE('t', 'v', 's', 'n'); // TV show season #
const AP4_Atom::Type AP4_ATOM_TYPE_TVES = AP4_ATOM_TYPE('t', 'v', 'e', 's'); // TV show episode #
const AP4_Atom::Type AP4_ATOM_TYPE_STIK = AP4_ATOM_TYPE('s', 't', 'i', 'k');
const AP4_Atom::Type AP4_ATOM_TYPE_PCST = AP4_ATOM_TYPE('p', 'c', 's', 't'); // Podcast?
const AP4_Atom::Type AP4_ATOM_TYPE_PURD = AP4_ATOM_TYPE('p', 'u', 'r', 'd'); //
const AP4_Atom::Type AP4_ATOM_TYPE_PURL = AP4_ATOM_TYPE('p', 'u', 'r', 'l'); // Podcast URL (binary)
const AP4_Atom::Type AP4_ATOM_TYPE_EGID = AP4_ATOM_TYPE('e', 'g', 'i', 'd'); //
const AP4_Atom::Type AP4_ATOM_TYPE_PGAP = AP4_ATOM_TYPE('p', 'g', 'a', 'p'); // Gapless Playback
const AP4_Atom::Type AP4_ATOM_TYPE_CATG = AP4_ATOM_TYPE('c', 'a', 't', 'g'); // Category
const AP4_Atom::Type AP4_ATOM_TYPE_KEYW = AP4_ATOM_TYPE('k', 'e', 'y', 'w'); // Keywords
const AP4_Atom::Type AP4_ATOM_TYPE_SONM = AP4_ATOM_TYPE('s', 'o', 'n', 'm'); // Sort-Order: Name
const AP4_Atom::Type AP4_ATOM_TYPE_SOAL = AP4_ATOM_TYPE('s', 'o', 'a', 'l'); // Sort-Order: Album
const AP4_Atom::Type AP4_ATOM_TYPE_SOAR = AP4_ATOM_TYPE('s', 'o', 'a', 'r'); // Sort-Order: Artist
const AP4_Atom::Type AP4_ATOM_TYPE_SOAA = AP4_ATOM_TYPE('s', 'o', 'a', 'a'); // Sort-Order: Album Artist
const AP4_Atom::Type AP4_ATOM_TYPE_SOCO = AP4_ATOM_TYPE('s', 'o', 'c', 'o'); // Sort-Order: Composer
const AP4_Atom::Type AP4_ATOM_TYPE_SOSN = AP4_ATOM_TYPE('s', 'o', 's', 'n'); // Sort-Order: Show

const AP4_Atom::Type AP4_ATOM_TYPE_TITL = AP4_ATOM_TYPE('t', 'i', 't', 'l'); // 3GPP: title
const AP4_Atom::Type AP4_ATOM_TYPE_DSCP = AP4_ATOM_TYPE('d', 's', 'c', 'p'); // 3GPP: description
const AP4_Atom::Type AP4_ATOM_TYPE_CPRT = AP4_ATOM_TYPE('c', 'p', 'r', 't'); // 3GPP, ISO or ILST: copyright
const AP4_Atom::Type AP4_ATOM_TYPE_PERF = AP4_ATOM_TYPE('p', 'e', 'r', 'f'); // 3GPP: performer
const AP4_Atom::Type AP4_ATOM_TYPE_AUTH = AP4_ATOM_TYPE('a', 'u', 't', 'h'); // 3GPP: author
const AP4_Atom::Type AP4_ATOM_TYPE_GNRE = AP4_ATOM_TYPE('g', 'n', 'r', 'e'); // 3GPP or ILST: genre (in 3GPP -> string, in ILST -> ID3v1 index + 1)
const AP4_Atom::Type AP4_ATOM_TYPE_RTNG = AP4_ATOM_TYPE('r', 't', 'n', 'g'); // 3GPP or ILST: rating
const AP4_Atom::Type AP4_ATOM_TYPE_CLSF = AP4_ATOM_TYPE('c', 'l', 's', 'f'); // 3GPP: classification
const AP4_Atom::Type AP4_ATOM_TYPE_KYWD = AP4_ATOM_TYPE('k', 'y', 'w', 'd'); // 3GPP: keywords
const AP4_Atom::Type AP4_ATOM_TYPE_LOCI = AP4_ATOM_TYPE('l', 'o', 'c', 'i'); // 3GPP: location information
const AP4_Atom::Type AP4_ATOM_TYPE_ALBM = AP4_ATOM_TYPE('a', 'l', 'b', 'm'); // 3GPP: album title and track number
const AP4_Atom::Type AP4_ATOM_TYPE_YRRC = AP4_ATOM_TYPE('y', 'r', 'r', 'c'); // 3GPP: recording year
const AP4_Atom::Type AP4_ATOM_TYPE_TSEL = AP4_ATOM_TYPE('t', 's', 'e', 'l'); // 3GPP: track selection

const AP4_Atom::Type AP4_ATOM_TYPE_ICNU = AP4_ATOM_TYPE('i', 'c', 'n', 'u'); // DCF: icon URI      (OMA DCF 2.1)
const AP4_Atom::Type AP4_ATOM_TYPE_INFU = AP4_ATOM_TYPE('i', 'n', 'f', 'u'); // DCF: info URI      (OMA DCF 2.1)
const AP4_Atom::Type AP4_ATOM_TYPE_CVRU = AP4_ATOM_TYPE('c', 'v', 'r', 'u'); // DCF: cover art URI (OMA DCF 2.1)
const AP4_Atom::Type AP4_ATOM_TYPE_LRCU = AP4_ATOM_TYPE('l', 'r', 'c', 'u'); // DCF: lyrics URI    (OMA DCF 2.1)
const AP4_Atom::Type AP4_ATOM_TYPE_DCFD = AP4_ATOM_TYPE('d', 'c', 'f', 'D'); // DCF: duration      (OMarlin)

/*----------------------------------------------------------------------
|   AP4_MetaData
+---------------------------------------------------------------------*/
class AP4_MetaData
{
public:
    class Key
    {
    public:
        // constructors
        Key(const char* name, const char* ns) :
            m_Name(name), m_Namespace(ns) {}

        // methods
        const AP4_String& GetNamespace() const
        {
            return m_Namespace;
        }
        const AP4_String& GetName()      const
        {
            return m_Name;
        }

    private:
        // members
        const AP4_String m_Name;
        const AP4_String m_Namespace;
    };

    class Value
    {
    public:
        // types
        typedef enum
        {
            TYPE_BINARY,
            TYPE_STRING_UTF_8,
            TYPE_STRING_UTF_16,
            TYPE_STRING_PASCAL,
            TYPE_GIF,
            TYPE_JPEG,
            TYPE_INT_08_BE,
            TYPE_INT_16_BE,
            TYPE_INT_32_BE,
            TYPE_FLOAT_32_BE,
            TYPE_FLOAT_64_BE
        } Type;

        typedef enum
        {
            TYPE_CATEGORY_STRING,
            TYPE_CATEGORY_BINARY,
            TYPE_CATEGORY_INTEGER,
            TYPE_CATEGORY_FLOAT
        } TypeCategory;

        typedef enum
        {
            MEANING_UNKNOWN,
            MEANING_ID3_GENRE,
            MEANING_BOOLEAN,
            MEANING_FILE_KIND,
            MEANING_BINARY_ENCODED_CHARS
        } Meaning;

        // destructor
        virtual ~Value() {}

        // methods
        Type               GetType() const
        {
            return m_Type;
        }
        TypeCategory       GetTypeCategory() const;
        Meaning            GetMeaning() const
        {
            return m_Meaning;
        }
        const AP4_String&  GetLanguage() const
        {
            return m_Language;
        }
        void               SetLanguage(const char* language)
        {
            m_Language = language;
        }
        virtual AP4_String ToString() const = 0;
        virtual AP4_Result ToBytes(AP4_DataBuffer& bytes) const = 0;
        virtual long       ToInteger() const = 0;

    protected:
        // class methods
        static TypeCategory MapTypeToCategory(Type type);

        // constructor
        Value(Type        type,
              Meaning     meaning  = MEANING_UNKNOWN,
              const char* language = NULL) :
            m_Type(type), m_Meaning(meaning), m_Language(language) {}

        // members
        Type       m_Type;
        Meaning    m_Meaning;
        AP4_String m_Language;
    };

    class KeyInfo
    {
    public:
        // members
        const char* name;
        const char* description;
        AP4_UI32    four_cc;
        Value::Type value_type;
    };

    class Entry
    {
    public:
        // constructor
        Entry(const char* name, const char* ns, Value* value) :
            m_Key(name, ns), m_Value(value) {}

        // destructor
        ~Entry()
        {
            delete m_Value;
        }

        // methods
        AP4_Result         AddToFile(AP4_File& file, AP4_Ordinal index = 0);
        AP4_Result         AddToFileIlst(AP4_File& file, AP4_Ordinal index = 0);
        AP4_Result         AddToFileDcf(AP4_File& file, AP4_Ordinal index = 0);
        AP4_Result         RemoveFromFile(AP4_File& file, AP4_Ordinal index);
        AP4_Result         RemoveFromFileIlst(AP4_File& file, AP4_Ordinal index);
        AP4_Result         RemoveFromFileDcf(AP4_File& file, AP4_Ordinal index);
        AP4_ContainerAtom* FindInIlst(AP4_ContainerAtom* ilst) const;
        AP4_Result         ToAtom(AP4_Atom*& atom) const;

        // members
        Key    m_Key;
        Value* m_Value;
    };

    // class members
    static AP4_Array<KeyInfo> KeyInfos;

    // constructor
    AP4_MetaData(AP4_File* file);

    // methods
    AP4_Result ParseMoov(AP4_MoovAtom* moov);
    AP4_Result ParseUdta(AP4_ContainerAtom* udta, const char* namespc);

    // destructor
    ~AP4_MetaData();

    // accessors
    const AP4_List<Entry>& GetEntries() const
    {
        return m_Entries;
    }

    // methods
    AP4_Result ResolveKeyName(AP4_Atom::Type atom_type, AP4_String& value);
    AP4_Result AddIlstEntries(AP4_ContainerAtom* atom, const char* namespc);
    AP4_Result Add3GppEntry(AP4_3GppLocalizedStringAtom* atom, const char* namespc);
    AP4_Result AddDcfStringEntry(AP4_DcfStringAtom* atom, const char* namespc);
    AP4_Result AddDcfdEntry(AP4_DcfdAtom* atom, const char* namespc);

private:
    // members
    AP4_List<Entry> m_Entries;
};

/*----------------------------------------------------------------------
|   AP4_MetaDataAtomTypeHandler
+---------------------------------------------------------------------*/
class AP4_MetaDataAtomTypeHandler : public AP4_AtomFactory::TypeHandler
{
public:
    // constructor
    AP4_MetaDataAtomTypeHandler(AP4_AtomFactory* atom_factory) :
        m_AtomFactory(atom_factory) {}
    virtual AP4_Result CreateAtom(AP4_Atom::Type  type,
                                  AP4_UI32        size,
                                  AP4_ByteStream& stream,
                                  AP4_Atom::Type  context,
                                  AP4_Atom*&      atom);

    // types
    struct TypeList
    {
        const AP4_Atom::Type* m_Types;
        AP4_Size              m_Size;
    };

    // class constants
    static const AP4_Atom::Type IlstTypes[];
    static const TypeList       IlstTypeList;
    static const AP4_Atom::Type _3gppLocalizedStringTypes[];
    static const TypeList       _3gppLocalizedStringTypeList;
    static const AP4_Atom::Type _3gppOtherTypes[];
    static const TypeList       _3gppOtherTypeList;
    static const AP4_Atom::Type DcfStringTypes[];
    static const TypeList       DcfStringTypeList;

    // class methods
    static bool IsTypeInList(AP4_Atom::Type type, const TypeList& list);

private:
    // members
    AP4_AtomFactory* m_AtomFactory;
};

/*----------------------------------------------------------------------
|   AP4_MetaDataTag
+---------------------------------------------------------------------*/
class AP4_MetaDataTag
{
public:

    // destructor
    virtual ~AP4_MetaDataTag() {}

    // methods
    virtual AP4_Result Write(AP4_ByteStream& stream);
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);

protected:
    // constructor
    AP4_MetaDataTag(AP4_UI32        data_type,
                    AP4_UI32        data_lang,
                    AP4_Size        size,
                    AP4_ByteStream& stream);

};

/*----------------------------------------------------------------------
|   AP4_3GppLocalizedStringAtom
+---------------------------------------------------------------------*/
class AP4_3GppLocalizedStringAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_3GppLocalizedStringAtom, AP4_Atom)

    // factory method
    static AP4_3GppLocalizedStringAtom* Create(Type type, AP4_UI32 size, AP4_ByteStream& stream);

    // constructor
    AP4_3GppLocalizedStringAtom(Type type, const char* language, const char* value);
    AP4_3GppLocalizedStringAtom(Type            type,
                                AP4_UI32        size,
                                AP4_UI32        version,
                                AP4_UI32        flags,
                                AP4_ByteStream& stream);

    // AP4_Atom methods
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    // methods
    const char*       GetLanguage() const
    {
        return m_Language;
    }
    const AP4_String& GetValue()    const
    {
        return m_Value;
    }

private:
    // members
    char       m_Language[4];
    AP4_String m_Value;
};

/*----------------------------------------------------------------------
|   AP4_DcfStringAtom
+---------------------------------------------------------------------*/
class AP4_DcfStringAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_DcfStringAtom, AP4_Atom)

    // factory method
    static AP4_DcfStringAtom* Create(Type type, AP4_UI32 size, AP4_ByteStream& stream);

    // constructor
    AP4_DcfStringAtom(Type type, const char* value);
    AP4_DcfStringAtom(Type            type,
                      AP4_UI32        size,
                      AP4_UI32        version,
                      AP4_UI32        flags,
                      AP4_ByteStream& stream);

    // AP4_Atom methods
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    // methods
    const AP4_String& GetValue() const
    {
        return m_Value;
    }

private:
    // members
    AP4_String m_Value;
};

/*----------------------------------------------------------------------
|   AP4_DcfdAtom
+---------------------------------------------------------------------*/
class AP4_DcfdAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_DcfdAtom, AP4_Atom)

    // factory method
    static AP4_DcfdAtom* Create(AP4_UI32 size, AP4_ByteStream& stream);

    // constructors
    AP4_DcfdAtom(AP4_UI32        version,
                 AP4_UI32        flags,
                 AP4_ByteStream& stream);
    AP4_DcfdAtom(AP4_UI32 duration);

    // AP4_Atom methods
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    // methods
    AP4_UI32 GetDuration() const
    {
        return m_Duration;
    }

private:
    // members
    AP4_UI32 m_Duration;
};

/*----------------------------------------------------------------------
|   AP4_MetaDataStringAtom
+---------------------------------------------------------------------*/
class AP4_MetaDataStringAtom : public AP4_Atom
{
public:
    // constructors
    AP4_MetaDataStringAtom(Type type, const char* value);
    AP4_MetaDataStringAtom(Type type, AP4_UI32 size, AP4_ByteStream& stream);

    // AP4_Atom methods
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    // methods
    const AP4_String& GetValue()
    {
        return m_Value;
    }

private:
    // members
    AP4_UI32   m_Reserved;
    AP4_String m_Value;
};

/*----------------------------------------------------------------------
|   AP4_DataAtom
+---------------------------------------------------------------------*/
class AP4_DataAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_DataAtom, AP4_Atom)

    typedef enum
    {
        DATA_TYPE_BINARY        = 0,
        DATA_TYPE_STRING_UTF_8  = 1,
        DATA_TYPE_STRING_UTF_16 = 2,
        DATA_TYPE_STRING_PASCAL = 3,
        DATA_TYPE_GIF           = 13,
        DATA_TYPE_JPEG          = 14,
        DATA_TYPE_SIGNED_INT_BE = 21, /* the size of the integer is derived from the container size */
        DATA_TYPE_FLOAT_32_BE   = 22,
        DATA_TYPE_FLOAT_64_BE   = 23
    } DataType;

    typedef enum
    {
        LANGUAGE_ENGLISH = 0
    } DataLang;

    // constructors
    AP4_DataAtom(const AP4_MetaData::Value& value);
    AP4_DataAtom(AP4_UI32 size, AP4_ByteStream& stream);

    // destructor
    ~AP4_DataAtom();

    // AP4_Atom methods
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    // accessors
    DataType GetDataType()
    {
        return m_DataType;
    }
    DataLang GetDataLang()
    {
        return m_DataLang;
    }
    AP4_MetaData::Value::Type GetValueType();

    // methods
    AP4_Result LoadString(AP4_String*& string);
    AP4_Result LoadBytes(AP4_DataBuffer& bytes);
    AP4_Result LoadInteger(long& value);

private:
    // members
    DataType        m_DataType;
    DataLang        m_DataLang;
    AP4_ByteStream* m_Source;
};

/*----------------------------------------------------------------------
|   AP4_StringMetaDataValue
+---------------------------------------------------------------------*/
class AP4_StringMetaDataValue : public AP4_MetaData::Value
{
public:
    // constructor
    AP4_StringMetaDataValue(const char* value, const char* language = NULL) :
        Value(TYPE_STRING_UTF_8, MEANING_UNKNOWN, language),
        m_Value(value) {}

    // AP4_MetaData::Value methods
    virtual AP4_String ToString() const;
    virtual AP4_Result ToBytes(AP4_DataBuffer& bytes) const;
    virtual long       ToInteger() const;

private:
    // members
    AP4_String m_Value;
};

/*----------------------------------------------------------------------
|   AP4_IntegerMetaDataValue
+---------------------------------------------------------------------*/
class AP4_IntegerMetaDataValue : public AP4_MetaData::Value
{
public:
    // constructor
    AP4_IntegerMetaDataValue(Type type, long value) :
        Value(type), m_Value(value) {}

    // AP4_MetaData::Value methods
    virtual AP4_String ToString() const;
    virtual AP4_Result ToBytes(AP4_DataBuffer& bytes) const;
    virtual long       ToInteger() const;

private:
    // members
    long m_Value;
};

/*----------------------------------------------------------------------
|   AP4_BinaryMetaDataValue
+---------------------------------------------------------------------*/
class AP4_BinaryMetaDataValue : public AP4_MetaData::Value
{
public:
    // constructor
    AP4_BinaryMetaDataValue(Type type, const AP4_UI08* data, AP4_Size size) :
        Value(type), m_Value(data, size) {}

    // AP4_MetaData::Value methods
    virtual AP4_String ToString() const;
    virtual AP4_Result ToBytes(AP4_DataBuffer& bytes) const;
    virtual long       ToInteger() const;

private:
    // members
    AP4_DataBuffer m_Value;
};

/*----------------------------------------------------------------------
|   AP4_AtomMetaDataValue
+---------------------------------------------------------------------*/
class AP4_AtomMetaDataValue : public AP4_MetaData::Value
{
public:
    // constructor
    AP4_AtomMetaDataValue(AP4_DataAtom* data_atom, AP4_UI32 parent_type);

    // AP4_MetaData::Value methods
    virtual AP4_String ToString() const;
    virtual AP4_Result ToBytes(AP4_DataBuffer& bytes) const;
    virtual long       ToInteger() const;

private:
    // members
    AP4_DataAtom* m_DataAtom;
};

#endif // _AP4_META_DATA_H_
