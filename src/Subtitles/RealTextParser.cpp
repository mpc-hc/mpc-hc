/*
 * (C) 2008-2015 see Authors.txt
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

#include "stdafx.h"
#include "RealTextParser.h"

CRealTextParser::CRealTextParser()
    : m_bIgnoreFont(false)
    , m_bIgnoreFontSize(false)
    , m_bIgnoreFontColor(false)
    , m_bIgnoreFontWeight(false)
    , m_bIgnoreFontFace(false)
    , m_iMinFontSize(14)
    , m_iMaxFontSize(25)
    , m_iDefaultSubtitleDurationInMillisecs(4000)
    , m_bTryToIgnoreErrors(true)
{
}

CRealTextParser::~CRealTextParser()
{
}

bool CRealTextParser::ParseRealText(std::wstring p_szFile)
{
    std::vector<int> vStartTimecodes;
    std::vector<int> vEndTimecodes;
    bool bPrevEndTimeMissing = false;
    std::list<Tag> listTags;
    std::list<Tag> listPreviousOpenTags;

    while (!p_szFile.empty()) {
        if (p_szFile.at(0) == '<') {
            Tag oTag;
            if (!ExtractTag(p_szFile, oTag)) {
                return false;
            }

            if (oTag.m_bComment) {
                continue;
            }

            if (oTag.m_szName == L"time") {
                int iStartTimecode = GetTimecode(oTag.m_mapAttributes[L"begin"]);
                int iEndTimecode = GetTimecode(oTag.m_mapAttributes[L"end"]);

                //FilterReduntantTags(listTags);
                std::wstring szLine = RenderTags(listTags);

                if (bPrevEndTimeMissing) {
                    std::pair<int, int> pairTimecodes(vStartTimecodes.back(), iStartTimecode);

                    // Fix issues where the next time code isn't valid end time code for the previous subtitle
                    if (pairTimecodes.first >= pairTimecodes.second) {
                        pairTimecodes.second = pairTimecodes.first + m_iDefaultSubtitleDurationInMillisecs;
                    }

                    if (!szLine.empty()) {
                        m_RealText.m_mapLines[pairTimecodes] = szLine;
                    }

                    bPrevEndTimeMissing = false;
                } else if (!vStartTimecodes.empty() && !vEndTimecodes.empty()) {
                    std::pair<int, int> pairTimecodes(vStartTimecodes.back(), vEndTimecodes.back());

                    if (!szLine.empty()) {
                        m_RealText.m_mapLines[pairTimecodes] = szLine;
                    }

                } else if (vStartTimecodes.empty() && vEndTimecodes.empty() && m_RealText.m_mapLines.empty() && iStartTimecode > 0) {
                    // Handle first line
                    if (!szLine.empty()) {
                        m_RealText.m_mapLines[std::make_pair(0, iStartTimecode)] = szLine;
                    }
                }

                vStartTimecodes.push_back(iStartTimecode);
                if (iEndTimecode <= 0) {
                    bPrevEndTimeMissing = true;
                } else {
                    vEndTimecodes.push_back(iEndTimecode);
                }
            } else if (oTag.m_szName == L"b" || oTag.m_szName == L"i" || oTag.m_szName == L"font") {
                if (oTag.m_bOpen) {
                    listPreviousOpenTags.push_back(oTag);
                }

                if (oTag.m_bClose) {
                    PopTag(listPreviousOpenTags, oTag.m_szName);
                }

                listTags.push_back(oTag);
            } else if (oTag.m_szName == L"clear") {
                listTags.clear();

                // set existing tags
                listTags.insert(listTags.end(), listPreviousOpenTags.begin(), listPreviousOpenTags.end());
            } else if (oTag.m_szName == L"window") {
                if (oTag.m_bOpen) {
                    m_RealText.m_WindowTag = oTag;
                }

                // Ignore close
            } else if (oTag.m_szName == L"center") {
                m_RealText.m_bCenter = true;
            } else if (oTag.m_szName == L"required") {
                // Ignore
            } else if (oTag.m_szName.empty()) {
                // Ignore
            } else {
                // assume formating tag (handled later)
                listTags.push_back(oTag);
            }
        } else {
            Tag oTextTag;
            if (!ExtractTextTag(p_szFile, oTextTag)) {
                return false;
            }

            listTags.push_back(oTextTag);
        }
    }

    // Handle final line
    //FilterReduntantTags(listTags);
    std::wstring szLine = RenderTags(listTags);

    if (bPrevEndTimeMissing) {
        std::pair<int, int> pairTimecodes(vStartTimecodes.back(), vStartTimecodes.back() + m_iDefaultSubtitleDurationInMillisecs);

        if (!szLine.empty()) {
            m_RealText.m_mapLines[pairTimecodes] = szLine;
        }

    } else if (!vStartTimecodes.empty() && !vEndTimecodes.empty()) {
        std::pair<int, int> pairTimecodes(vStartTimecodes.back(), vEndTimecodes.back());

        if (!szLine.empty()) {
            m_RealText.m_mapLines[pairTimecodes] = szLine;
        }

    }

    return true;
}

const CRealTextParser::Subtitles& CRealTextParser::GetParsedSubtitles()
{
    return m_RealText;
}

bool CRealTextParser::ExtractTag(std::wstring& p_rszLine, Tag& p_rTag)
{
    if (p_rszLine.length() < 2 || p_rszLine.at(0) != '<') {
        if (m_bTryToIgnoreErrors) {
            size_t iTempPos = p_rszLine.find_first_of('<');

            if (iTempPos != std::wstring::npos) {
                p_rszLine = p_rszLine.substr(iTempPos);

                if (p_rszLine.length() < 2) {
                    return false;
                }
            }

        } else {
            return false;
        }
    }

    unsigned int iPos = 1;

    // skip comments
    if (p_rszLine.at(iPos) == '!') {
        p_rTag.m_bComment = true;

        std::wstring szComment;
        GetString(p_rszLine, iPos, szComment, L">");
        p_rTag.m_szName = szComment;

        ++iPos; // Skip >
        if (iPos < p_rszLine.length()) {
            p_rszLine = p_rszLine.substr(iPos);
            return true;
        } else {
            return false;
        }
    } else {
        p_rTag.m_bComment = false;
    }

    if (!SkipSpaces(p_rszLine, iPos)) {
        return false;
    }

    if (p_rszLine.at(iPos) == '/') {
        p_rTag.m_bOpen = false;
        p_rTag.m_bClose = true;
        ++iPos;
    } else {
        p_rTag.m_bOpen = true;
        p_rTag.m_bClose = false;
    }

    if (!GetString(p_rszLine, iPos, p_rTag.m_szName, L"\r\n\t />")) {
        return false;
    }

    p_rTag.m_szName = StringToLower(p_rTag.m_szName);

    if (!GetAttributes(p_rszLine, iPos, p_rTag.m_mapAttributes)) {
        return false;
    }

    if (p_rszLine.at(iPos) == '/') {
        ++iPos;
        p_rTag.m_bClose = true;
        if (iPos >= p_rszLine.length()) {
            return false;
        }
    }

    if (p_rszLine.at(iPos) == '>') {
        ++iPos;
        p_rszLine = p_rszLine.substr(iPos);
        return true;
    } else {
        if (m_bTryToIgnoreErrors) {
            size_t iTempPos = p_rszLine.find_first_of('>');

            if (iTempPos != std::wstring::npos) {
                if (iTempPos - 1 >= p_rszLine.length()) {
                    return false;
                }

                p_rszLine = p_rszLine.substr(iTempPos + 1);
                return true;
            } else {
                return false;
            }

        } else {
            return false;
        }
    }
}

bool CRealTextParser::ExtractTextTag(std::wstring& p_rszLine, Tag& p_rTag)
{
    p_rTag.m_bText = true;
    return ExtractString(p_rszLine, p_rTag.m_szName);
}

bool CRealTextParser::ExtractString(std::wstring& p_rszLine, std::wstring& p_rszString)
{
    if (p_rszLine.empty() || p_rszLine.at(0) == '<') {
        if (m_bTryToIgnoreErrors) {
            p_rszString = L"";
            return true;
        } else {
            return false;
        }
    }

    unsigned int iPos = 0;

    if (!SkipSpaces(p_rszLine, iPos)) {
        return false;
    }

    if (!GetString(p_rszLine, iPos, p_rszString, L"<")) {
        return false;
    }

    p_rszLine = p_rszLine.substr(iPos);
    return true;
}

bool CRealTextParser::SkipSpaces(std::wstring& p_rszLine, unsigned int& p_riPos)
{
    while (p_rszLine.length() > p_riPos && iswspace(p_rszLine.at(p_riPos))) {
        ++p_riPos;
    }

    return p_rszLine.length() > p_riPos;
}

bool CRealTextParser::GetString(std::wstring& p_rszLine, unsigned int& p_riPos, std::wstring& p_rszString, const std::wstring& p_crszEndChars)
{
    while (p_rszLine.length() > p_riPos && p_crszEndChars.find(p_rszLine.at(p_riPos)) == std::wstring::npos) {
        p_rszString += p_rszLine.at(p_riPos);
        ++p_riPos;
    }

    return p_rszLine.length() > p_riPos;
}

bool CRealTextParser::GetAttributes(std::wstring& p_rszLine, unsigned int& p_riPos, std::map<std::wstring, std::wstring>& p_rmapAttributes)
{
    if (!SkipSpaces(p_rszLine, p_riPos)) {
        return false;
    }

    while (p_riPos < p_rszLine.length() && p_rszLine.at(p_riPos) != '/' && p_rszLine.at(p_riPos) != '>') {
        std::wstring szName;
        if (!GetString(p_rszLine, p_riPos, szName, L"\r\n\t =")) {
            return false;
        }

        if (!SkipSpaces(p_rszLine, p_riPos)) {
            return false;
        }

        if (p_rszLine.at(p_riPos) != '=') {
            if (m_bTryToIgnoreErrors) {
                size_t pos = p_rszLine.find_first_of('=', p_riPos);
                if (pos == std::wstring::npos) {
                    return false;
                } else {
                    p_riPos = (unsigned int)pos;
                }
            } else {
                return false;
            }
        }

        ++p_riPos;

        if (!SkipSpaces(p_rszLine, p_riPos)) {
            return false;
        }

        bool bUsesQuotes = false;
        if (p_rszLine.at(p_riPos) == '\'' || p_rszLine.at(p_riPos) == '\"') {
            ++p_riPos;
            bUsesQuotes = true;
        }

        if (!SkipSpaces(p_rszLine, p_riPos)) {
            return false;
        }

        std::wstring szValue;
        if (bUsesQuotes) {
            if (!GetString(p_rszLine, p_riPos, szValue, L"\"\'/>")) {
                return false;
            }
        } else {
            if (!GetString(p_rszLine, p_riPos, szValue, L" \t/>")) {
                return false;
            }
        }

        p_rmapAttributes[StringToLower(szName)] = szValue;

        if (!SkipSpaces(p_rszLine, p_riPos)) {
            return false;
        }

        if (p_rszLine.at(p_riPos) == '\'' || p_rszLine.at(p_riPos) == '\"') {
            ++p_riPos;
        }

        if (!SkipSpaces(p_rszLine, p_riPos)) {
            return false;
        }
    }

    return p_rszLine.length() > p_riPos;
}

int CRealTextParser::GetTimecode(const std::wstring& p_crszTimecode)
{
    int iTimecode = 0;
    int iMultiplier = 1;

    // Exception: if the timecode doesn't contain any separators, assume the time code is in seconds (and change multiplier to reflect that)
    if (p_crszTimecode.find_first_of('.') == std::wstring::npos && p_crszTimecode.find_first_of(':') == std::wstring::npos) {
        iMultiplier = 1000;
    }

    std::wstring szCurrentPart;

    for (ptrdiff_t i = p_crszTimecode.length() - 1; i >= 0; --i) {
        if (p_crszTimecode.at(i) == '.' || p_crszTimecode.at(i) == ':') {
            if (iMultiplier == 1) {
                while (szCurrentPart.length() < 3) {
                    szCurrentPart += L"0";
                }
            }

            iTimecode += iMultiplier * ::_wtoi(szCurrentPart.c_str());

            if (iMultiplier == 1) {
                iMultiplier = 1000;
            } else {
                iMultiplier *= 60;
            }

            szCurrentPart = L"";
        } else {
            szCurrentPart = p_crszTimecode.substr(i, 1) + szCurrentPart;
        }
    }

    iTimecode += iMultiplier * ::_wtoi(szCurrentPart.c_str());

    return iTimecode;
}

std::wstring CRealTextParser::FormatTimecode(int iTimecode,
                                             int iMillisecondPrecision/* = 3*/,
                                             bool p_bPadZeroes/* = true*/,
                                             const std::wstring& p_crszSeparator/* = ":"*/,
                                             const std::wstring& p_crszMillisecondSeparator/* = "."*/)
{
    std::wostringstream ossTimecode;

    int iHours = iTimecode / 1000 / 60 / 60;

    ossTimecode << iHours;

    int iMinutes = (iTimecode / 1000 / 60) % 60;

    ossTimecode << p_crszSeparator;
    ossTimecode << iMinutes;

    int iSeconds = (iTimecode / 1000) % 60;

    ossTimecode << p_crszSeparator;
    ossTimecode << iSeconds;

    int iMilliSeconds = iTimecode % 1000;

    if (iMillisecondPrecision < 3) {
        iMilliSeconds /= 10 * (3 - iMillisecondPrecision);
    }

    ossTimecode << p_crszMillisecondSeparator;
    ossTimecode << iMilliSeconds;

    return ossTimecode.str();
}

std::wstring CRealTextParser::StringToLower(const std::wstring& p_crszString)
{
    std::wstring szLowercaseString;
    for (unsigned int i = 0; i < p_crszString.length(); ++i) {
        szLowercaseString += towlower(p_crszString.at(i));
    }
    return szLowercaseString;
}

std::wstring CRealTextParser::RenderTags(const std::list<Tag>& p_crlTags)
{
    bool bEmpty = true;
    std::wstring szString;

    for (auto iter = p_crlTags.cbegin(); iter != p_crlTags.cend(); ++iter) {
        Tag oTag(*iter);

        if (oTag.m_szName == L"br") {
            szString += L"\n";
        } else if (oTag.m_szName == L"b") {
            if (!m_bIgnoreFontWeight) {
                if (oTag.m_bOpen) {
                    szString += L"<b>";
                } else if (oTag.m_bClose) {
                    szString += L"</b>";
                }
            }
        } else if (oTag.m_szName == L"i") {
            if (!m_bIgnoreFontWeight) {
                if (oTag.m_bOpen) {
                    szString += L"<i>";
                } else if (oTag.m_bClose) {
                    szString += L"</i>";
                }
            }
        } else if (oTag.m_szName == L"font") {
            if (!m_bIgnoreFont) {
                if (oTag.m_bOpen) {
                    szString += L"<font";
                    for (std::map<std::wstring, std::wstring>:: iterator i = oTag.m_mapAttributes.begin(); i != oTag.m_mapAttributes.end(); ++i) {
                        if (m_bIgnoreFontSize && i->first == L"size") {
                            continue;
                        }

                        if (m_bIgnoreFontColor && i->first == L"color") {
                            continue;
                        }

                        if (m_bIgnoreFontFace && i->first == L"face") {
                            continue;
                        }

                        if (i->first == L"size" && !i->second.empty() && ::iswdigit(i->second.at(0))) {
                            int iSize = ::_wtoi(i->second.c_str());

                            if (iSize > 0 && iSize < m_iMinFontSize) {
                                continue;
                            }

                            if (iSize > m_iMaxFontSize) {
                                continue;
                            }
                        }

                        szString += L" ";
                        szString += i->first;
                        szString += L"=\"";
                        szString += i->second;
                        szString += L"\"";
                    }
                    szString += L">";
                }

                if (oTag.m_bClose) {
                    szString += L"</font>";
                }
            }
        } else if (oTag.m_bText) {
            szString += oTag.m_szName;

            if (!oTag.m_szName.empty()) {
                bEmpty = false;
            }
        } else {
            //AfxMessageBox(CString(_T("Unknown RealText-tag: ")) + oTag.m_szName.c_str());
        }
    }

    if (bEmpty) {
        return L"";
    } else {
        return szString;
    }
}

bool CRealTextParser::OutputSRT(std::wostream& p_rOutput)
{
    int iCounter = 1;
    for (auto i = m_RealText.m_mapLines.cbegin(); i != m_RealText.m_mapLines.cend(); ++i) {
        p_rOutput << iCounter++;
        p_rOutput << std::endl;

        p_rOutput << FormatTimecode(i->first.first);
        p_rOutput << L" --> ";
        p_rOutput << FormatTimecode(i->first.second);
        p_rOutput << std::endl;

        p_rOutput << i->second;
        p_rOutput << std::endl;
        p_rOutput << std::endl;
    }

    return true;
}

void CRealTextParser::PopTag(std::list<Tag>& p_rlistTags, const std::wstring& p_crszTagName)
{
    for (auto riter = p_rlistTags.crbegin(); riter != p_rlistTags.crend(); ++riter) {
        if (riter->m_szName == p_crszTagName) {
            p_rlistTags.erase((++riter).base());
            return;
        }
    }
}

/*void CRealTextParser::FilterReduntantTags(std::list<Tag>& p_rlistTags)
{
    std::list<Tag>::iterator iterPrev;
    for (std::list<Tag>::iterator iterCurrent = p_rlistTags.begin(); iterCurrent != p_rlistTags.end(); ++iterCurrent) {
        if (iterCurrent != p_rlistTags.begin()) {
            if (iterPrev->m_szName == L"font" && iterCurrent->m_szName == L"font" &&
                    iterPrev->m_bOpen && iterCurrent->m_bOpen) {
                p_rlistTags.erase(iterPrev);
            }
        }
        iterPrev = iterCurrent;
    }
}*/
