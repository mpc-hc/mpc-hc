/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 *  Note: This interface was defined for the matroska container format 
 *  originally, but can be implemented for other formats as well.
 *
 */

#pragma once

typedef enum ChapterType {
    AtomicChapter = 0, // only contain one element
    SubChapter    = 1, // contain a list of elements
};

#pragma pack(push, 1)
struct ChapterElement {
    WORD Size;				// size of this structure
    BYTE Type;				// see ChapterType
    UINT ChapterId;			// unique identifier for this element
    REFERENCE_TIME rtStart;	// REFERENCE_TIME in 100ns
    REFERENCE_TIME rtStop;	// REFERENCE_TIME in 100ns
	struct ChapterElement() {Size = sizeof(*this);}
};
struct ChapterElement2 : ChapterElement {
	BOOL bDisabled;
	struct ChapterElement2() {Size = sizeof(*this);}
};
#pragma pack(pop)

interface __declspec(uuid("8E128709-3DC8-4e49-B632-380FCF496B6D"))
IChapterInfo : public IUnknown
{
	#define CHAPTER_BAD_ID   0xFFFFFFFF
	#define CHAPTER_ROOT_ID   0

	//	\param aChapterID is 0 for the top level one
	STDMETHOD_(UINT, GetChapterCount) (UINT aChapterID) = 0;

	//	\param aIndex start from 1 to GetChapterCount( aParentChapterId )
	STDMETHOD_(UINT, GetChapterId) (UINT aParentChapterId, UINT aIndex) = 0;

	STDMETHOD_(UINT, GetChapterCurrentId) () = 0;

	STDMETHOD_(BOOL, GetChapterInfo) (UINT aChapterID, struct ChapterElement* pStructureToFill) = 0;

	//	\param PreferredLanguage Language code as in ISO-639-2 (3 chars)
	//	\param CountryCode       Country code as in internet domains
	STDMETHOD_(BSTR, GetChapterStringInfo) (UINT aChapterID, CHAR PreferredLanguage[3], CHAR CountryCode[2]) = 0;
};