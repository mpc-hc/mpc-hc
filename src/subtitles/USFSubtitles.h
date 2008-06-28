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
 */

#pragma once

#include <atlcoll.h>
#include "STS.h"

// metadata
typedef struct {CStringW name, email, url;} author_t;
typedef struct {CStringW code, text;} language_t;
typedef struct {CStringW title, date, comment; author_t author; language_t language, languageext;} metadata_t;
// style
typedef struct {CStringW alignment, relativeto, horizontal_margin, vertical_margin, rotate[3];} posattriblist_t;
typedef struct {CStringW face, size, color[4], weight, italic, underline, alpha, outline, shadow, wrap;} fontstyle_t;
typedef struct {CStringW name; fontstyle_t fontstyle; posattriblist_t pal;} style_t;
// effect
typedef struct {CStringW position; fontstyle_t fontstyle; posattriblist_t pal;} keyframe_t;
typedef struct {CStringW name; CAutoPtrList<keyframe_t> keyframes;} effect_t;
// subtitle/text
typedef struct {int start, stop; CStringW effect, style, str; posattriblist_t pal;} text_t;

class CUSFSubtitles
{
	bool ParseUSFSubtitles(CComPtr<IXMLDOMNode> pNode);
	 void ParseMetadata(CComPtr<IXMLDOMNode> pNode, metadata_t& m);
	 void ParseStyle(CComPtr<IXMLDOMNode> pNode, style_t* s);
	  void ParseFontstyle(CComPtr<IXMLDOMNode> pNode, fontstyle_t& fs);
	  void ParsePal(CComPtr<IXMLDOMNode> pNode, posattriblist_t& pal);
	 void ParseEffect(CComPtr<IXMLDOMNode> pNode, effect_t* e);
	  void ParseKeyframe(CComPtr<IXMLDOMNode> pNode, keyframe_t* k);
	 void ParseSubtitle(CComPtr<IXMLDOMNode> pNode, int start, int stop);
	  void ParseText(CComPtr<IXMLDOMNode> pNode, CStringW& assstr);
	  void ParseShape(CComPtr<IXMLDOMNode> pNode);

public:
	CUSFSubtitles();
	virtual ~CUSFSubtitles();

	bool Read(LPCTSTR fn);
//	bool Write(LPCTSTR fn); // TODO

	metadata_t metadata;
	CAutoPtrList<style_t> styles;
	CAutoPtrList<effect_t> effects;
	CAutoPtrList<text_t> texts;

	bool ConvertToSTS(CSimpleTextSubtitle& sts);
//	bool ConvertFromSTS(CSimpleTextSubtitle& sts); // TODO
};
