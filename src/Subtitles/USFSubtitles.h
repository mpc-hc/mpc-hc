/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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

#include <atlcoll.h>
#include "STS.h"

// metadata
struct author_t {
    CStringW name, email, url;
};

struct language_t {
    CStringW code, text;
};

struct metadata_t {
    CStringW title, date, comment;
    author_t author;
    language_t language, languageext;
};

// style
struct posattriblist_t {
    CStringW alignment, relativeto, horizontal_margin, vertical_margin, rotate[3];
};

struct fontstyle_t {
    CStringW face, size, color[4], weight, italic, underline, alpha, outline, shadow, wrap;
};

struct style_t {
    CStringW name;
    fontstyle_t fontstyle;
    posattriblist_t pal;
};

// effect
struct keyframe_t {
    CStringW position;
    fontstyle_t fontstyle;
    posattriblist_t pal;
};

struct effect_t {
    CStringW name;
    CAutoPtrList<keyframe_t> keyframes;
};

// subtitle/text
struct text_t {
    int start, stop;
    CStringW effect, style, str;
    posattriblist_t pal;
};

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
    //bool Write(LPCTSTR fn); // TODO

    metadata_t metadata;
    CAutoPtrList<style_t> styles;
    CAutoPtrList<effect_t> effects;
    CAutoPtrList<text_t> texts;

    bool ConvertToSTS(CSimpleTextSubtitle& sts);
    //bool ConvertFromSTS(CSimpleTextSubtitle& sts); // TODO
};
