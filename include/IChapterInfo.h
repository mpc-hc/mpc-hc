/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2017 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

enum ChapterType {
    AtomicChapter   = 0,        // only contain one element
    SubChapter      = 1         // contain a list of elements
};

#pragma pack(push, 1)
struct ChapterElement {
    WORD Size;                  // size of this structure
    BYTE Type;                  // see ChapterType
    UINT ChapterId;             // unique identifier for this element
    REFERENCE_TIME rtStart;     // REFERENCE_TIME in 100ns
    REFERENCE_TIME rtStop;      // REFERENCE_TIME in 100ns
    ChapterElement()
        : Size(sizeof(ChapterElement))
        , Type(0)
        , ChapterId(0)
        , rtStart(0)
        , rtStop(0)
    {}
};
#pragma pack(pop)

interface __declspec(uuid("8E128709-3DC8-4e49-B632-380FCF496B6D"))
    IChapterInfo :
    public IUnknown
{
#define CHAPTER_BAD_ID      0xFFFFFFFF
#define CHAPTER_ROOT_ID     0

    //  \param aChapterID is 0 for the top level one
    STDMETHOD_(UINT, GetChapterCount)(UINT aChapterID) = 0;

    //  \param aIndex start from 1 to GetChapterCount(aParentChapterId)
    STDMETHOD_(UINT, GetChapterId)(UINT aParentChapterId, UINT aIndex) = 0;

    STDMETHOD_(UINT, GetChapterCurrentId)() = 0;

    STDMETHOD_(BOOL, GetChapterInfo)(UINT aChapterID, struct ChapterElement* pStructureToFill) = 0;

    //  \param PreferredLanguage Language code as in ISO-639-2 (3 chars)
    //  \param CountryCode       Country code as in internet domains
    STDMETHOD_(BSTR, GetChapterStringInfo)(UINT aChapterID, CHAR PreferredLanguage[3], CHAR CountryCode[2]) = 0;
};
