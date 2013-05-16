/*
 * (C) 2008-2013 see Authors.txt
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

#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <cwctype>

class CRealTextParser
{
public:
    CRealTextParser();
    virtual ~CRealTextParser();

    struct Tag {
        Tag(): m_bOpen(false), m_bClose(false), m_bComment(false), m_bText(false) {}

        std::wstring m_szName;

        bool m_bOpen;
        bool m_bClose;

        bool m_bComment;
        bool m_bText;

        std::map<std::wstring, std::wstring> m_mapAttributes;
    };

    struct Subtitles {
        Subtitles(): m_WindowTag(), m_FontTag(), m_bCenter(false) {}

        Tag m_WindowTag;
        Tag m_FontTag;

        bool m_bCenter;

        std::map<std::pair<int, int>, std::wstring> m_mapLines;
    };

    bool ParseRealText(std::wstring p_szFile);

    const Subtitles& GetParsedSubtitles();

    bool OutputSRT(std::wostream& p_rOutput);

private:
    bool ExtractTag(std::wstring& p_rszLine, Tag& p_rTag);
    bool ExtractTextTag(std::wstring& p_rszLine, Tag& p_rTag);
    bool ExtractString(std::wstring& p_rszLine, std::wstring& p_rszString);
    bool SkipSpaces(std::wstring& p_rszLine, unsigned int& p_riPos);
    bool GetString(std::wstring& p_rszLine, unsigned int& p_riPos, std::wstring& p_rszString, const std::wstring& p_crszEndChars);
    bool GetAttributes(std::wstring& p_rszLine, unsigned int& p_riPos, std::map<std::wstring, std::wstring>& p_rmapAttributes);

    int GetTimecode(const std::wstring& p_crszTimecode);
    std::wstring FormatTimecode(int iTimecode,
                                int iMillisecondPrecision = 3,
                                bool p_bPadZeroes = true,
                                const std::wstring& p_crszSeparator = L":",
                                const std::wstring& p_crszMillisecondSeparator = L".");

    std::wstring StringToLower(const std::wstring& p_crszString);

    std::wstring RenderTags(const std::list<Tag>& p_crlTags);

    void PopTag(std::list<Tag>& p_rlistTags, const std::wstring& p_crszTagName);

    // Filter out for example multiple font tags opened previously
    // (font tags are not always terminated properly in realtext and can build up)
    //void FilterReduntantTags(list<Tag>& p_rlistTags);

    Subtitles m_RealText;

    bool m_bIgnoreFont;
    bool m_bIgnoreFontSize;
    bool m_bIgnoreFontColor;
    bool m_bIgnoreFontWeight;
    bool m_bIgnoreFontFace;

    int m_iMinFontSize;
    int m_iMaxFontSize;

    int m_iDefaultSubtitleDurationInMillisecs;

    bool m_bTryToIgnoreErrors;
};
