/*
 * (C) 2014 see Authors.txt
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
#include "SubtitlesProviders.h"
#include <wininet.h>

#if USE_STATIC_MEDIAINFO
#include "MediaInfo/MediaInfo.h"
using namespace MediaInfoLib;
#define MediaInfo_int64u ZenLib::int64u
#else
#include "MediaInfoDLL.h"
using namespace MediaInfoDLL;
#endif


/******************************************************************************
** SubtitlesInfo
******************************************************************************/

void SubtitlesInfo::Download(BOOL bActivate)
{
    if (fileProvider) {
        fileProvider->Providers().Download(*this, bActivate);
    }
}

void SubtitlesInfo::OpenUrl() const
{
    if (!url.empty()) {
        ShellExecute((HWND)AfxGetMyApp()->GetMainWnd(), _T("open"), UTF8To16(url.c_str()), nullptr, nullptr, SW_SHOWDEFAULT);
    }
}

//HRESULT SubtitlesInfo::GetCurrentSubtitles()
//{
//    CMainFrame& pMainFrame = *(CMainFrame*)(AfxGetMyApp()->GetMainWnd());
//    int i = 0;
//    SubtitleInput* pSubInput = pMainFrame.GetSubtitleInput(i, true);
//    CStringW content;
//    if (pSubInput) {
//        CLSID clsid;
//        if (FAILED(pSubInput->subStream->GetClassID(&clsid))) {
//            return E_FAIL;
//        }
//
//        CPath suggestedFileName(pMainFrame.GetFileName());
//        suggestedFileName.RemoveExtension(); // exclude the extension, it will be auto-completed
//
//        if (clsid == __uuidof(CRenderedTextSubtitle)) {
//            CRenderedTextSubtitle* pRTS = (CRenderedTextSubtitle*)(ISubStream*)pSubInput->subStream;
//            // Only for external text subtitles
//            if (!pRTS->m_path.IsEmpty()) {
//                GetFileInfo();
//                //GetFileInfo((const char*)UTF16To8(pRTS->m_name));
//                releaseName = (const char*)UTF16To8(pRTS->m_name);
//                if (hearingImpaired == -1) {
//                    hearingImpaired = pRTS->m_fHearingImpaired;
//                }
//
//                if (!languageCode.length() && pRTS->m_lcid != 0) {
//                    CString str;
//                    int len = GetLocaleInfo(pRTS->m_lcid, LOCALE_SISO639LANGNAME, str.GetBuffer(64), 64);
//                    str.ReleaseBufferSetLength(std::max(len - 1, 0));
//                    languageCode = UTF16To8(str);
//                }
//                frameRate = pMainFrame.m_pCAP->GetFPS();
//
//                pRTS->m_encoding;
//                pRTS->m_exttype;
//                CAutoLock cAutoLock(&pMainFrame.m_csSubLock);
//                //pRTS->SaveAs(fd.GetPathName(), (exttype)(fd.m_ofn.nFilterIndex - 1), pMF->m_pCAP->GetFPS(), fd.GetDelay(), fd.GetEncoding());
//                double fps = pMainFrame.m_pCAP->GetFPS();
//                int delay = 0;
//                CStringW fmt(L"%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\n%s\n\n");
//
//                if (pRTS->m_mode == FRAME) {
//                    delay = (int)(delay * fps / 1000);
//                }
//
//                for (int i = 0, j = (int)pRTS->GetCount(), k = 0; i < j; i++) {
//                    STSEntry& stse = pRTS->GetAt(i);
//
//                    int t1 = pRTS->TranslateStart(i, fps) + delay;
//                    if (t1 < 0) {
//                        k++;
//                        continue;
//                    }
//
//                    int t2 = pRTS->TranslateEnd(i, fps) + delay;
//
//                    int hh1 = (t1 / 60 / 60 / 1000);
//                    int mm1 = (t1 / 60 / 1000) % 60;
//                    int ss1 = (t1 / 1000) % 60;
//                    int ms1 = (t1) % 1000;
//                    int hh2 = (t2 / 60 / 60 / 1000);
//                    int mm2 = (t2 / 60 / 1000) % 60;
//                    int ss2 = (t2 / 1000) % 60;
//                    int ms2 = (t2) % 1000;
//
//                    CStringW str(true /*f.IsUnicode()*/
//                                 ? pRTS->GetStrW(i, false)
//                                 : pRTS->GetStrWA(i, false));
//
//                    content.AppendFormat(fmt, i - k + 1, hh1, mm1, ss1, ms1, hh2, mm2, ss2, ms2, str);
//                }
//            }
//        }
//    }
//    fileContents = UTF16To8(content);
//    return S_OK;
//}


#define RE_ITE(_cond, _true, _false)  "(?(" _cond ")" _true "|" _false ")"
#define RE_IT(_cond, _true)  "(?(" _cond ")" _true ")"
#define RE_O(_expr)  _expr "?"
#define RE_CG(_expr)  "(" _expr ")"
#define RE_OCG(_expr)  RE_O(RE_CG(_expr))
#define RE_NCG(_expr)  "(?:" _expr ")"
#define RE_ONCG(_expr) RE_O(RE_NCG(_expr))
#define RE_PLA(_cond, _expr)  "(?=" _cond ")" _expr
#define RE_NLA(_cond, _expr)  "(?!" _cond ")" _expr


#define _RE_DEAD                                                              \
    "[-\\[('*:;.,_ /!@#$%?)\\]]"

#define _RE_NOTDEAD                                                           \
    "[^-\\[('*:;.,_ /!@#$%?)\\]\\\\]"

#define _RE_CAPTURE_TITLE                                                     \
    "([^\\\\]*?)(?:"_RE_DEAD"+(AU|CA|FR|JP|UK|US))??"_RE_DEAD"+"
//    "([^\\\\]*?)"_RE_DEAD"+"
//_RE_CG("[^\\\\]*?")_RE_OCG("AU|CA|FR|JP|UK|US")

#define _RE_YEAR                                                              \
    "\\d{4}"

#define _RE_SEASONEPISODE                                                     \
    "S\\d{1,2}"RE_ONCG("E\\d{1,2}"RE_ONCG(RE_O(_RE_DEAD)"E\\d{1,2}"))         \
    "|E\\d{1,2}"RE_ONCG(RE_O(_RE_DEAD)"E\\d{1,2}")                            \
    "|\\d{1,2}[x#]\\d{1,2}"                                                   \
    "|\\d{3}"                                                                 \
    "|"RE_ONCG("Season|Series"RE_O(_RE_DEAD)"\\d{1,2}"_RE_DEAD)"\\d{1,2}of\\d{1,2}"

#define _RE_CAPTURE_SEASONEPISODE                                             \
    "S(\\d{1,2})"RE_ONCG("E(\\d{1,2})"RE_ONCG(RE_O(_RE_DEAD)"E\\d{1,2}"))     \
    "|E(\\d{1,2})"RE_ONCG(RE_O(_RE_DEAD)"E\\d{1,2}")                          \
    "|(\\d{1,2})[x#](\\d{1,2})"                                               \
    "|(\\d)(\\d{2})"                                                          \
    "|"RE_ONCG("Season|Series"RE_O(_RE_DEAD)"(\\d{1,2})"_RE_DEAD)"(\\d{1,2})of\\d{1,2}"

#define _RE_CAPTURE_TITLE2                                                    \
    "(?:([^\\\\]+?)??"_RE_DEAD"+)??"
//"(?:([^\\\\]+?)"_RE_DEAD "+)?"

#define _RE_RESOLUTION                                                        \
    "(?:480|720|1080)[pi]?"

#define _RE_FORMAT                                                            \
    "HD-?Rip|HD-?DVD(?:-?Rip)?"                                               \
    "|HD-?TV|PD-?TV|DVTV|DVB(?:-?Rip)?|TV-?Rip"                               \
    "|B[DR]Rip|Blu-?Ray"                                                      \
    "|DVD-?(?:Rip|ivX)?|VIDEO-TS"                                             \
    "|DVD-?SCR|SCREENER"                                                      \
    "|WEB-?DL|WEB-?Rip"                                                       \
    "|VHS"                                                                    \
    "|CAM|TS"                                                                 \
    "|R5(?:[-._ ]?LiNE)?"

#define _RE_AUDIOCODEC                                                        \
    "AC3|DTS(?:[-._ ]?ES)?|He-AAC|AAC-He|AAC"

#define _RE_VIDEOCODEC                                                        \
    "XviD|DivX|DVDivX|[hx][-._ ]?264|Rv10|Mpeg2"

#define _RE_CAPTURE_RELEASEGROUP                                              \
    "("_RE_NOTDEAD"+)[\\[(-._ )\\]\\\\]+"

#define _RE_CAPTURE_LANGUAGE                                                  \
    "(?:("_RE_NOTDEAD"+)[\\[(-._ )\\]\\\\]+)?"

#define _RE_CAPTURE_DISC                                                      \
    "(?:CD|DIS[CK])[-._ ]?(\\d)"_RE_DEAD "+"

#define _RE_IGNORE                                                            \
    "(?:DD)?5[-._ ]1|[AS]E|[WF]S|\\d{3}MB|5CH|AUDIO[-._ ]?FIXED|CHRONO|COLORIZED|DC|DUAL[-._ ]?AUDIO|DUBBED|DUPE|DVD[-._ ]?[59]|EXTENDED|FESTIVAL|FINAL|INTERNAL|LIMITED|NFO|PROPER|RATED|READ|REAL|REMASTERED|REMUX|REPACK|RERIP|R[1-6]|RETAIL|RETORRENT|S?TV|SUBBED|THEATRICAL|UNCUT|UNRATED"

#define _RE_CAPTURE_HEARINGIMPAIRED                                           \
    _RE_DEAD"(HI)"_RE_DEAD".*(srt|idx|sub|ssa)$"

#define _RE_CAPTURE_MEDIAEXTENSIONS                                           \
    _RE_DEAD "(3g2|3gp2?|asf|avi|divx|flv|m4v|mk[2av]|mov|mp4a?|mpe?g|og[gvm]|qt|ram?|rm|ts|wav|webm|wm[av])$"

#define _RE_CAPTURE_SUBTITLESEXTENSION                                        \
    _RE_DEAD "(srt|idx|sub|ssa)$"

#define _RE_ADDICT7ED_LANGUAGE                                                \
    "Albanian|Arabic|Armenian|Azerbaijani|Bengali|Bosnian|Bulgarian|" + UTF16To8(_T("Català")) + "|Chinese \\(Simplified\\)|Chinese \\(Traditional\\)|Croatian|Czech|Danish|Dutch|English|Euskera|Finnish|French|Galego|German|Greek|Hebrew|Hungarian|Indonesian|Italian|Japanese|Korean|Macedonian|Malay|Norwegian|Persian|Polish|Portuguese|Portuguese \\(Brazilian\\)|Romanian|Russian|Serbian \\(Cyrillic\\)|Serbian \\(Latin\\)|Slovak|Slovenian|Spanish|Spanish \\(Latin America\\)|Spanish \\(Spain\\)|Swedish|Thai|Turkish|Urainian|Vietnamese"

#define _RE_ADDICT7ED_CORRECTED                                               \
    "C[.]"

#define _RE_ADDICT7ED_HEARINGIMPAIRED                                         \
    "HI[.]"

#define _RE_ADDICT7ED_VERSION                                                 \
    "(?:updated|orig)[.]"

#define _RE_ADDICT7ED_SIGNATURE                                               \
    "Addic[t7]ed[.]com"


#define _RE_PLA(_expr, _group) "(?=(" _expr "))\\" #_group _RE_DEAD"+"
#define _RE_OPLA(_expr, _group) "(?:(?=(" _expr "))\\" #_group _RE_DEAD"+)?"
#define _RE_CG(_expr)  "(" _expr ")" _RE_DEAD "+"
#define _RE_OCG(_expr)  RE_ONCG(RE_CG(_expr) _RE_DEAD "+")
#define _RE_NCG(_expr)  "(?:" _expr ")" _RE_DEAD "+"

#define _RE_ONCG(_expr)  "(?:" _RE_NCG(_expr) ")?"
#define _RE_ORNCG(_expr)  "(?:(?:" _RE_NCG(_expr) ")+)?"


std::regex regex_pattern[] = {
    //--------------------------------------
    std::regex(
        _RE_CAPTURE_TITLE _RE_ORNCG(_RE_IGNORE)
        RE_ONCG(
            RE_NCG(_RE_CG(_RE_YEAR) _RE_OCG(_RE_SEASONEPISODE) "|" _RE_CG(_RE_SEASONEPISODE))
            _RE_ORNCG(_RE_IGNORE) _RE_CAPTURE_TITLE2
        )
        RE_NCG(_RE_CG(_RE_RESOLUTION) "|" _RE_CG(_RE_FORMAT) "|" _RE_CG(_RE_AUDIOCODEC) "|" _RE_CG(_RE_VIDEOCODEC) "|" _RE_NCG(_RE_IGNORE))"+"
        _RE_CAPTURE_RELEASEGROUP
        //_RE_CAPTURE_LANGUAGE
        , regex_flags),
    //--------------------------------------
    std::regex(
        _RE_CAPTURE_TITLE
        RE_NCG(_RE_CG(_RE_YEAR) _RE_OCG(_RE_SEASONEPISODE) "|" _RE_CG(_RE_SEASONEPISODE))
        , regex_flags),
    //--------------------------------------
    std::regex(
        _RE_CAPTURE_SEASONEPISODE
        , regex_flags),
    //--------------------------------------
    std::regex(
        _RE_CAPTURE_DISC
        , regex_flags),
    //--------------------------------------
    std::regex(
        _RE_CAPTURE_HEARINGIMPAIRED
        , regex_flags),
    //--------------------------------------
    std::regex(
        "([^\\\\]+?)(?: (AU|CA|FR|JP|UK|US))?(?: [(](\\d{4})[)])? - (\\d{1,2})x(\\d{1,2}) - ([^.]+?)[.](.+?)[.]" _RE_CG(_RE_ADDICT7ED_LANGUAGE) RE_OCG(_RE_ADDICT7ED_HEARINGIMPAIRED) RE_OCG(_RE_ADDICT7ED_CORRECTED) _RE_ADDICT7ED_VERSION _RE_ADDICT7ED_SIGNATURE
        , regex_flags),
    //--------------------------------------
    std::regex(_RE_DEAD "+", regex_flags),

    //--------------------------------------
    //--------------------------------------
    std::regex(
        _RE_CAPTURE_TITLE
        RE_ONCG(
            RE_NCG(
                _RE_PLA(_RE_YEAR, 2) _RE_OPLA(_RE_SEASONEPISODE, 3)
                "|"_RE_PLA(_RE_SEASONEPISODE, 4)
            )
            _RE_CAPTURE_TITLE2
        )
        RE_NCG(
            _RE_PLA(_RE_RESOLUTION, 6)
            "|"_RE_PLA(_RE_FORMAT, 7)
            "|"_RE_PLA(_RE_AUDIOCODEC, 8)
            "|"_RE_PLA(_RE_VIDEOCODEC, 9)
        )"+"
        _RE_CAPTURE_RELEASEGROUP
        , regex_flags),
    //--------------------------------------
};

HRESULT SubtitlesInfo::GetFileInfo(const std::string& _fileName /*= std::string()*/)
{
    if (_fileName.empty()) {
        CMainFrame& pMainFrame = *(CMainFrame*)(AfxGetMyApp()->GetMainWnd());
        if (CComQIPtr<IBaseFilter> pBF = pMainFrame.m_pFSF) {
            BeginEnumPins(pBF, pEP, pPin) {
                if (pAsyncReader = pPin) {
                    break;
                }
            }
            EndEnumPins;
        }

        if (pAsyncReader && pMainFrame.m_pFSF) {
            LPOLESTR name;
            if (FAILED(pMainFrame.m_pFSF->GetCurFile(&name, nullptr))) {
                return E_FAIL;
            }
            filePath = UTF16To8(name);
            //fileName = UTF16To8(name);
            CoTaskMemFree(name);

            LONGLONG size, available;
            if (pAsyncReader->Length(&size, &available) != S_OK) { // Don't accept estimates
                return E_FAIL;
            }
            fileSize = size;
        } else {
            CString _filePath(pMainFrame.m_wndPlaylistBar.GetCurFileName());
            {
                CFile file;
                CFileException fileException;
                if (!file.Open(_filePath, CFile::modeRead | CFile::osSequentialScan | CFile::shareDenyNone | CFile::typeBinary, &fileException)) {
                    return E_FAIL;
                }

                filePath = UTF16To8(_filePath);
                fileSize = file.GetLength();
            }
            {
                CPath p(_filePath);
                p.RenameExtension(_T(".nfo"));
                CFile file;
                CFileException fileException;
                if (file.Open(p, CFile::modeRead | CFile::osSequentialScan | CFile::shareDenyNone | CFile::typeBinary, &fileException)) {
                    std::string buffer;
                    buffer.resize((std::string::size_type)file.GetLength());
                    UINT len = file.Read(&buffer[0], (UINT)buffer.size());

                    std::smatch match_pieces;
                    if (std::regex_search(buffer, match_pieces, std::regex("imdb[.][a-z]{2,3}/title/tt(\\d+)", regex_flags))) {
                        imdbid = match_pieces[1].str();
                    }
                }
            }
        }
    } else {
        filePath = _fileName;
    }

    CPath p(UTF8To16(filePath.c_str()));
    fileExtension = UTF16To8((LPCTSTR)(p.GetExtension()) + 1);
    p.StripPath();
    p.RemoveExtension();
    fileName = UTF16To8(p);

    regex_result result;

    if (std::regex_search(fileName, std::regex("addic[7t]ed", regex_flags)) && string_regex(regex_pattern[5], filePath, result)) {
        if (title.empty()) { title = result[0]; }
        if (country.empty()) { country = result[1]; }
        if (year == -1) { year = result[2].empty() ? -1 : atoi(result[2].c_str()); }
        if (seasonNumber == -1) { seasonNumber = atoi(result[3].c_str()); }
        if (episodeNumber == -1) { episodeNumber = atoi(result[4].c_str()); }
        if (title2.empty()) { title2 = result[5]; }
        if (releaseGroup.empty()) { releaseGroup = result[6]; }
        if (languageName.empty()) { languageName = result[7]; }
        if (hearingImpaired == -1) { hearingImpaired = result[8].empty() ? FALSE : TRUE; }
    } else if (string_regex(regex_pattern[0], filePath, result)) {
        if (title.empty()) { title = result[0]; }
        if (country.empty()) { country = result[1]; }
        if (year == -1) { year = result[2].empty() ? -1 : atoi(result[2].c_str()); }
        if (episode.empty()) { episode = result[3] + result[4]; }
        //bool b = IsISO639Language(match_pieces[5].str().c_str());
        //bool b1 = IsISO639Language("french");
        //CString lang = LanguageToISO6392(CString(match_pieces[5].str().c_str()));
        //std::string tt = match_pieces[5].str();
        if (title2.empty()) { title2 = result[5]; }
        if (resolution.empty()) { resolution = result[6]; }
        if (format.empty()) { format = result[7]; }
        if (audioCodec.empty()) { audioCodec = result[8]; }
        if (videoCodec.empty()) { videoCodec = result[9]; }
        if (releaseGroup.empty()) { releaseGroup = result[10]; }
        //if (languageCode.empty()) languageCode = result[11];
    } else if (string_regex(regex_pattern[1], filePath, result)) {
        if (title.empty()) { title = result[0]; }
        if (country.empty()) { country = result[1]; }
        if (year == -1) { year = result[2].empty() ? -1 : atoi(result[2].c_str()); }
        if (episode.empty()) { episode = result[3] + result[4]; }
    }

    if (!title.empty()) { title = std::regex_replace(title, regex_pattern[6], " "); }
    if (!title2.empty()) { title2 = std::regex_replace(title2, regex_pattern[6], " "); }

    if ((seasonNumber == -1) && (episodeNumber == -1) && !episode.empty()) {
        if (string_regex(regex_pattern[2], episode, result)) {
            std::string _seasonNumber(result[0] + result[3] + result[5] + result[7]);
            seasonNumber = atoi(_seasonNumber.c_str());
            if (!seasonNumber) {
                seasonNumber = 1;
            }
            std::string _episodeNumber(result[1] + result[2] + result[4] + result[6] + result[8]);
            episodeNumber = atoi(_episodeNumber.c_str());
        }
    }

    if ((discNumber == -1) && string_regex(regex_pattern[3], filePath, result)) {
        discNumber = result[0].empty() ? -1 : atoi(result[0].c_str());
    }

    if ((hearingImpaired == -1) && string_regex(regex_pattern[4], filePath, result)) {
        hearingImpaired = TRUE;
    }
    return S_OK;
}

/******************************************************************************
** SubtitlesProvider
******************************************************************************/

SubtitlesProvider::SubtitlesProvider() : m_Providers(SubtitlesProviders::Instance()), m_Search(FALSE), m_Upload(FALSE), m_LoggedIn(SPL_UNDEFINED)
{
}

void SubtitlesProvider::OpenUrl()
{
    ShellExecute((HWND)AfxGetMyApp()->GetMainWnd(), _T("open"), UTF8To16(Url().c_str()), nullptr, nullptr, SW_SHOWDEFAULT);
}

BOOL SubtitlesProvider::Login()
{
    HRESULT hr = S_OK;
    if (!(m_LoggedIn & (SPL_REGISTERED | SPL_ANONYMOUS))) {
        hr = Login(UserName(), Password());
        m_LoggedIn = (hr == S_OK) ? (!UserName().empty() ? SPL_REGISTERED : SPL_ANONYMOUS) :
                     (hr == E_NOTIMPL) ? SPL_ANONYMOUS : SPL_FAILED;
    }
    return m_LoggedIn & (SPL_REGISTERED | SPL_ANONYMOUS) ? TRUE : FALSE;
}


bool SubtitlesProvider::CheckLanguage(const std::string& languageCode) const
{
    SubtitlesThread& _thread = *((SubtitlesThreadParam*)AfxGetThread()->m_pThreadParams)->pThread;
    return ((_thread.Languages().empty()) || (_thread.Languages().find(languageCode) != std::string::npos));
}

void SubtitlesProvider::Set(const SubtitlesInfo& fileInfo, SubtitlesInfo& subtitlesInfo, volatile BOOL& _bAborting)
{
    SubtitlesThread& _thread = *((SubtitlesThreadParam*)AfxGetThread()->m_pThreadParams)->pThread;
    SubtitlesList& _subtitlesList = ((SubtitlesThreadParam*)AfxGetThread()->m_pThreadParams)->subtitlesList;

    _thread.CheckAbortAndThrow();


    //SubtitlesInfo fileInfo2;
    SubtitlesInfo& fileInfo2 = subtitlesInfo;
    fileInfo2.GetFileInfo(subtitlesInfo.fileName);
    //if (fileInfo2.hearingImpaired == TRUE && subtitlesInfo.hearingImpaired == FALSE) {
    //    subtitlesInfo.hearingImpaired = TRUE;
    //}

    //iter.score = 0; //LevenshteinDistance(fileInfo.fileName, string_(subtitlesName)) * 100;
    SHORT score = ((SHORT)subtitlesInfo.corrected);
    score += (!fileInfo.title.empty() && _strcmpi(fileInfo.title.c_str(), fileInfo2.title.c_str()) == 0) ? 3 : !fileInfo.title.empty() ? -3 : 0;
    score += (!fileInfo.country.empty() && _strcmpi(fileInfo.country.c_str(), fileInfo2.country.c_str()) == 0) ? 2 : !fileInfo.country.empty() ? -2 : 0;
    score += (fileInfo.year != -1 && fileInfo.year == fileInfo2.year) ? 1 : 0;
    score += (fileInfo.seasonNumber != -1 && fileInfo.seasonNumber == fileInfo2.seasonNumber) ? 2 : fileInfo.seasonNumber > 0 ? -2 : 0;
    score += (fileInfo.episodeNumber != -1 && fileInfo.episodeNumber == fileInfo2.episodeNumber) ? 2 : fileInfo.episodeNumber > 0 ? -2 : 0;
    score += (!fileInfo.title2.empty() && _strcmpi(fileInfo.title2.c_str(), fileInfo2.title2.c_str()) == 0) ? 2 : 0;
    score += (!fileInfo.resolution.empty() && _strcmpi(fileInfo.resolution.c_str(), fileInfo2.resolution.c_str()) == 0) ? 1 : 0;
    score += (!fileInfo.format.empty() && _strcmpi(fileInfo.format.c_str(), fileInfo2.format.c_str()) == 0) ? 1 : 0;
    score += (!fileInfo.audioCodec.empty() && _strcmpi(fileInfo.audioCodec.c_str(), fileInfo2.audioCodec.c_str()) == 0) ? 1 : 0;
    score += (!fileInfo.videoCodec.empty() && _strcmpi(fileInfo.videoCodec.c_str(), fileInfo2.videoCodec.c_str()) == 0) ? 1 : 0;
    score += (!fileInfo.releaseGroup.empty() && _strcmpi(fileInfo.releaseGroup.c_str(), fileInfo2.releaseGroup.c_str()) == 0) ? 1 : 0;
    const auto& s = AfxGetAppSettings();
    _thread.CheckAbortAndThrow();

    subtitlesInfo.Set(this,
                      (BYTE)((_thread.Languages().length() - _thread.Languages().find(subtitlesInfo.languageCode) + 1) / 3),
                      (BYTE)(subtitlesInfo.hearingImpaired == (int)s.bPreferHearingImpairedSubtitles),
                      score);

    _subtitlesList.push_back(subtitlesInfo);
}

SRESULT SubtitlesProvider::Download(std::string url, std::string referer, std::string& data)
{
    string_map headers({
        { "User-Agent", UserAgent() },
        { "Referer", referer },
    });

    DWORD dwStatusCode;
    string_download(url, headers, data, TRUE, &dwStatusCode);

    switch (dwStatusCode) {
        case 200:
            return SR_SUCCEEDED;
        default:
            return SR_FAILED;
    }
}

size_t SubtitlesProvider::Index() const
{
    size_t index = 0;
    for (const auto& iter : m_Providers.Providers()) {
        if (iter == this) {
            return index;
        }
        ++index;
    }
    return -1;
}

BOOL SubtitlesProvider::CheckInternetConnection()
{
    DWORD dwFlags = NULL;
    return InternetGetConnectedState(&dwFlags, NULL) == TRUE;
}

/******************************************************************************
** SubtitlesProviders
******************************************************************************/

SubtitlesProviders::SubtitlesProviders() :
    m_pMainFrame(*(CMainFrame*)(AfxGetMyApp()->GetMainWnd()))
{
    RegisterProviders();
    ReadSettings();
}

SubtitlesProviders::~SubtitlesProviders()
{
    Abort(SubtitlesThreadType(STT_SEARCH | STT_UPLOAD | STT_DOWNLOAD));
}

BOOL SubtitlesProviders::CheckInternetConnection()
{
    DWORD dwFlags = NULL;
    return InternetGetConnectedState(&dwFlags, NULL) == TRUE;
}

void SubtitlesProviders::Search(BOOL bAutoDownload)
{
    Abort(SubtitlesThreadType(STT_SEARCH | STT_DOWNLOAD));
    m_pMainFrame.m_wndSubtitlesDownloadDialog.DoClear();

    if (CheckInternetConnection()) {
        SubtitlesThread* pThread = (SubtitlesThread*)AfxBeginThread(RUNTIME_CLASS(SubtitlesThread));
        pThread->Initialize(&m_pMainFrame, this, SubtitlesThreadType(STT_SEARCH | (bAutoDownload ? STT_DOWNLOAD : NULL)));
        pThread->Search(bAutoDownload, LanguagesISO6391());
    } else if (bAutoDownload == FALSE) {
        m_pMainFrame.m_wndSubtitlesDownloadDialog.DoFailed();
    }
}

void SubtitlesProviders::Download(SubtitlesInfo& fileInfo, BOOL bActivate)
{
    if (CheckInternetConnection()) {
        SubtitlesThread* pThread = (SubtitlesThread*)AfxBeginThread(RUNTIME_CLASS(SubtitlesThread));
        pThread->Initialize(&m_pMainFrame, this, STT_DOWNLOAD);
        pThread->Download(fileInfo, bActivate);
    }
}

void SubtitlesProviders::Upload()
{
    if (CheckInternetConnection()) {
        // We get all the information we need within the main thread, to delay closing the file
        // until we have everything we need.
        SubtitlesInfo fileInfo;
        m_pMainFrame.SendMessage(WM_GETSUBTITLES, 0, (LPARAM)&fileInfo);
        //fileInfo.GetCurrentSubtitles();

        if (!fileInfo.fileContents.empty()) {
            SubtitlesThread* pThread = (SubtitlesThread*)AfxBeginThread(RUNTIME_CLASS(SubtitlesThread));
            pThread->Initialize(&m_pMainFrame, this, STT_UPLOAD);
            pThread->Upload(fileInfo);
        }
    }
}

void SubtitlesProviders::Abort(SubtitlesThreadType Type)
{
    CAutoLock cAutoLock(&m_csSyncThreads);
    for (auto& pThread : m_pThreads) {
        if (pThread->Type() & Type) {
            pThread->Abort();
        }
    }
}


void SubtitlesProviders::ReadSettings()
{
    const auto& s = AfxGetAppSettings();
    regex_results results;
    string_regex("<[|]([^|]*?)[|]([^|]*?)[|]([^|]*?)[|]([^|]*?)[|]([^|]*?)[|]>", (const char*)UTF16To8(s.strSubtitlesProviders), results);
    size_t notFount = 0;
    for (const auto& iter : results) {
        size_t index = &iter - &results[0] - notFount;
        bool bFound = false;
        for (auto& iter1 : m_Providers) {
            if (iter[0] == iter1->Name()) {
                bFound = true;
                iter1->UserName(iter[1]);
                iter1->Password(Base64::decode(iter[2]), FALSE);
                iter1->Enabled(SPF_SEARCH, atoi(iter[3].c_str()));
                iter1->Enabled(SPF_UPLOAD, atoi(iter[4].c_str()));
                std::iter_swap(&iter1, m_Providers.begin() + std::min(index, m_Providers.size() - 1));
            }
        }
        if (bFound == false) {
            ++notFount;
        }
    }
}

std::string SubtitlesProviders::WriteSettings()
{
    std::string result;
    for (const auto& iter : m_Providers) {
        result += "<|" + iter->Name() + "|" + iter->UserName() + "|" + Base64::encode(iter->Password(FALSE)) + "|" + std::to_string(iter->Enabled(SPF_SEARCH)) + "|" + std::to_string(iter->Enabled(SPF_UPLOAD)) + "|>";
    }
    return result;
}





IMPLEMENT_DYNCREATE(SubtitlesThread, CWinThread)

BOOL SubtitlesThread::InitInstance()
{
    AfxSocketInit();
    return SUCCEEDED(CoInitialize(0)) ? TRUE : FALSE;
}

int SubtitlesThread::ExitInstance()
{
    m_pProviders->RemoveThread(this);

    CoUninitialize();
    return __super::ExitInstance();
}

void SubtitlesThread::Initialize(CMainFrame* pMainFrame, SubtitlesProviders* pProviders, SubtitlesThreadType Type)
{
    m_pMainFrame = pMainFrame;
    m_pProviders = pProviders;
    m_Type = Type;

    m_pProviders->InsertThread(this);

    SetThreadName(DWORD(-1), ThreadName().c_str());
}

std::string SubtitlesThread::ThreadName()
{
    std::string threadName("SubtitlesThread::");
    if (m_Type & STT_SEARCH) { threadName.append("Search"); }
    if (m_Type & STT_DOWNLOAD) { threadName.append("Download"); }
    if (m_Type & STT_UPLOAD) { threadName.append("Upload"); }
    return threadName;
}

void SubtitlesThread::Quit()
{
    PostThreadMessage(TM_QUIT, 0, 0);
}

void SubtitlesThread::Abort()
{
    m_bAbort = TRUE;
}

void SubtitlesThread::Search(BOOL bAutoDownload, std::string languages)
{
    ATLASSERT(m_Type & STT_SEARCH);
    m_bAutoDownload = bAutoDownload;
    m_Languages = languages;
    if (bAutoDownload) {
        string_array _languages(string_tokenize(languages, ","));
        for (const auto& iter : _languages) {
            m_AutoDownload[iter] = FALSE;
        }
    }
    PostThreadMessage(TM_SEARCH, 0, 0);
}

void SubtitlesThread::Upload(SubtitlesInfo info)
{
    ATLASSERT(m_Type & STT_UPLOAD);
    m_fileInfo = info;
    PostThreadMessage(TM_UPLOAD, 0, 0);
}

void SubtitlesThread::Download(SubtitlesInfo& info, BOOL bActivate)
{
    ATLASSERT(m_Type & STT_DOWNLOAD);
    m_fileInfo = info;
    m_bActivate = bActivate;
    PostThreadMessage(TM_DOWNLOAD, 0, 0);
}

BEGIN_MESSAGE_MAP(SubtitlesThread, CWinThread)
    ON_THREAD_MESSAGE(TM_QUIT, OnQuit)
    ON_THREAD_MESSAGE(TM_SEARCH, OnSearch)
    ON_THREAD_MESSAGE(TM_UPLOAD, OnUpload)
    ON_THREAD_MESSAGE(TM_DOWNLOAD, OnDownload)
END_MESSAGE_MAP()

void SubtitlesThread::OnQuit(WPARAM wParam, LPARAM lParam)
{
    PostQuitMessage(0);
    if (CAMEvent* e = (CAMEvent*)lParam) {
        e->Set();
    }
}

void SubtitlesThread::OnSearch(WPARAM wParam, LPARAM lParam)
{
    ATLASSERT(m_Type & STT_SEARCH);

    // We get all the information we need within a separate thread,
    // to avoid delaying the video playback.
    SubtitlesInfo fileInfo;
    fileInfo.GetFileInfo();

    const auto& s = AfxGetAppSettings();
    std::string exclude = UTF16To8(s.strAutoDownloadSubtitlesExclude);
    string_array exclude_array = string_tokenize(exclude, "|;, ");
    for (auto& iter : exclude_array) {
        if (fileInfo.filePath.find(iter) != std::string::npos) {
            return Quit();
        }
    }

    if (!CheckAbort()) {
        CAutoLock cAutoLock(&m_csSyncThreads);
        for (const auto& iter : m_pProviders->Providers()) {
            if (iter->Enabled(SPF_SEARCH)) {
                SubtitlesThreadParam* pThreadParam = DEBUG_NEW SubtitlesThreadParam(this, iter, fileInfo);
                m_pThreads.push_back(AfxBeginThread(_WorkerProc, (LPVOID)pThreadParam));
            }
        }

        if (!m_pThreads.empty()) {
            return _SearchInitialize();
        }
    }
    Quit();
}

void SubtitlesThread::OnDownload(WPARAM wParam, LPARAM lParam)
{
    ATLASSERT(m_Type & STT_DOWNLOAD);

    if (!CheckAbort()) {
        CAutoLock cAutoLock(&m_csSyncThreads);
        SubtitlesThreadParam* pThreadParam = DEBUG_NEW SubtitlesThreadParam(this, &m_fileInfo.Provider(), m_fileInfo);
        m_pThreads.push_back(AfxBeginThread(_WorkerProc, (LPVOID)pThreadParam));

        if (!m_pThreads.empty()) {
            return _DownloadInitialize();
        }
    }
    Quit();
}

void SubtitlesThread::OnUpload(WPARAM wParam, LPARAM lParam)
{
    ATLASSERT(m_Type & STT_UPLOAD);

    if (!CheckAbort()) {
        CAutoLock cAutoLock(&m_csSyncThreads);
        for (const auto& iter : m_pProviders->Providers()) {
            if (iter->Enabled(SPF_UPLOAD) && iter->Flags(SPF_UPLOAD)) {
                SubtitlesThreadParam* pThreadParam = DEBUG_NEW SubtitlesThreadParam(this, iter, m_fileInfo);
                m_pThreads.push_back(AfxBeginThread(_WorkerProc, (LPVOID)pThreadParam));
            }
        }

        if (!m_pThreads.empty()) {
            return _UploadInitialize();
        }
    }
    Quit();
}

UINT SubtitlesThread::_WorkerProc(LPVOID pParam)
{
    CWinThread& _this = *AfxGetThread();
    SubtitlesThread& _thread = *((SubtitlesThreadParam*)pParam)->pThread;
    SubtitlesInfo& _fileInfo = ((SubtitlesThreadParam*)pParam)->fileInfo;
    SubtitlesList& _subtitlesList = ((SubtitlesThreadParam*)pParam)->subtitlesList;

    std::string threadName = string_format("%s(%s)", _thread.ThreadName().c_str(), _fileInfo.Provider().Name().c_str());
    SetThreadName(DWORD(-1), threadName.c_str());

    { CAutoLock cAutoLock(&_thread.m_csSyncThreads); }

    try {
        //DEBUGINFO("MPC-HC --> [%05.d] Start Thread: %s", _this.m_nThreadID, threadName.c_str());
        if (_fileInfo.Provider().Login() <= SR_SUCCEEDED) {
            if (_thread.m_Type & STT_SEARCH) {
                _thread._SearchProc(_fileInfo, _subtitlesList);
            } else if (_thread.m_Type & STT_DOWNLOAD) {
                _thread._DownloadProc(_fileInfo, _thread.m_bActivate);
            } else if (_thread.m_Type & STT_UPLOAD) {
                _thread._UploadProc(_fileInfo);
            }
        } else {
            if (_thread.m_Type & STT_SEARCH) {
                _thread.m_pMainFrame->m_wndSubtitlesDownloadDialog.DoCompleted(SR_FAILED, _subtitlesList);
            } else if (_thread.m_Type & STT_UPLOAD) {
                _thread.m_pMainFrame->m_wndSubtitlesUploadDialog.DoCompleted(SR_FAILED, _fileInfo.Provider());
            }
        }
        //DEBUGINFO("MPC-HC --> [%05.d]   End Thread: %s", _this.m_nThreadID, threadName.c_str());
    } catch (/*HRESULT e*/...) {
        if (_thread.m_Type & STT_SEARCH) {
            _thread.m_pMainFrame->m_wndSubtitlesDownloadDialog.DoCompleted(SR_ABORTED, _subtitlesList);
        } else if (_thread.m_Type & STT_UPLOAD) {
            _thread.m_pMainFrame->m_wndSubtitlesUploadDialog.DoCompleted(SR_ABORTED, _fileInfo.Provider());
        }
        //DEBUGINFO("MPC-HC --> [%05.d] Throw Thread: %s", _this.m_nThreadID, threadName.c_str());
    }

    delete(SubtitlesThreadParam*)pParam;
    pParam = nullptr;

    CAutoLock cAutoLock(&_thread.m_csSyncThreads);
    _thread.m_pThreads.remove(&_this);

    if (_thread.m_pThreads.empty()) {
        if (_thread.m_Type & STT_SEARCH) {
            _thread._SearchFinalize();
        } else if (_thread.m_Type & STT_DOWNLOAD) {
            _thread._DownloadFinalize();
        } else if (_thread.m_Type & STT_UPLOAD) {
            _thread._UploadFinalize();
        }

        _thread.Quit();
    }

    return 0;
}

void SubtitlesThread::_SearchInitialize()
{
    m_pMainFrame->m_wndSubtitlesDownloadDialog.DoSearch((INT)m_pThreads.size());
}

void SubtitlesThread::_SearchProc(SubtitlesInfo& _fileInfo, SubtitlesList& _subtitlesList)
{
    //auto CheckAbortAndThrow = [&]() { if (m_bAbort) throw E_ABORT; };
    CheckAbortAndThrow();
    _fileInfo.Provider().Hash(_fileInfo);
    CheckAbortAndThrow();
    m_pMainFrame->m_wndSubtitlesDownloadDialog.DoSearching(_fileInfo.Provider());
    SRESULT searchResult = _fileInfo.Provider().Search(_fileInfo, m_bAbort);
    CheckAbortAndThrow();
    _subtitlesList.sort();
    CheckAbortAndThrow();
    m_pMainFrame->m_wndSubtitlesDownloadDialog.DoCompleted(searchResult, _subtitlesList);
    CheckAbortAndThrow();
    if (!_subtitlesList.empty() && (m_Type & STT_DOWNLOAD) && !m_AutoDownload.empty()) {
        _DownloadProc(_subtitlesList);
    }
}

void SubtitlesThread::_SearchFinalize()
{
    BOOL bShowDialog = !m_AutoDownload.empty() ? TRUE : m_bAutoDownload;
    for (const auto& iter : m_AutoDownload) {
        if (iter.second == TRUE) {
            bShowDialog = FALSE;
            break;
        }
    }
    m_pMainFrame->m_wndSubtitlesDownloadDialog.DoFinished(m_bAbort, bShowDialog);
}

void SubtitlesThread::_DownloadInitialize()
{
}

void SubtitlesThread::_DownloadProc(SubtitlesList& _subtitlesList)
{
    CAutoLock cAutoLock(&m_csSyncThreads);
    CheckAbortAndThrow();
    for (auto& iter : _subtitlesList) {
        CheckAbortAndThrow();
        BOOL bDownload = FALSE;
        for (const auto& language : m_AutoDownload) {
            if (language.first == iter.languageCode) {
                if (language.second == FALSE) {
                    bDownload = TRUE;
                    break;
                }
            } else if (language.second == TRUE) {
                break;
            }
        }
        if (bDownload == TRUE) {
            const auto& s = AfxGetAppSettings();
            if ((iter.episodeNumber != -1 && (SHORT)LOWORD(iter.Score()) >= s.nAutoDownloadScoreSeries) || (iter.episodeNumber == -1 && (SHORT)LOWORD(iter.Score()) >= s.nAutoDownloadScoreMovies)) {
                CheckAbortAndThrow();
                _DownloadProc(iter, TRUE);
            }
        }
    }
}

void SubtitlesThread::_DownloadProc(SubtitlesInfo& _fileInfo, BOOL _bActivate)
{
    CheckAbortAndThrow();
    m_pMainFrame->m_wndSubtitlesDownloadDialog.DoDownloading(_fileInfo);
    if (_fileInfo.Provider().Download(_fileInfo, m_bAbort) == SR_SUCCEEDED) {
        CheckAbortAndThrow();
        string_map data = string_uncompress(_fileInfo.fileContents, _fileInfo.fileName);
        CheckAbortAndThrow();
        for (const auto& iter : data) {
            CheckAbortAndThrow();
            struct { SubtitlesInfo* fileInfo; BOOL bActivate; std::string fileName; std::string fileContents; } data({ &_fileInfo, _bActivate, iter.first, iter.second });
            if (m_pMainFrame->SendMessage(WM_LOADSUBTITLES, (BOOL)_bActivate, (LPARAM)&data) == TRUE) {
                if (!m_AutoDownload.empty()) {
                    m_AutoDownload[_fileInfo.languageCode] = TRUE;
                }
            }
        }
    }
}

void SubtitlesThread::_DownloadFinalize()
{
}

void SubtitlesThread::_UploadInitialize()
{
    if (!m_pThreads.empty()) {
        m_pMainFrame->m_wndSubtitlesUploadDialog.DoUpload((INT)m_pThreads.size());
    }
}

void SubtitlesThread::_UploadProc(SubtitlesInfo& _fileInfo)
{
    //auto CheckAbortAndThrow = [&]() { if (m_bAbort) throw E_ABORT; };
    CheckAbortAndThrow();
    m_pMainFrame->m_wndSubtitlesUploadDialog.DoUploading(_fileInfo.Provider());
    _fileInfo.Provider().Hash(_fileInfo);
    CheckAbortAndThrow();
    SRESULT uploadResult = _fileInfo.Provider().Upload(_fileInfo, m_bAbort);
    m_pMainFrame->m_wndSubtitlesUploadDialog.DoCompleted(uploadResult, _fileInfo.Provider());
}

void SubtitlesThread::_UploadFinalize()
{
    m_pMainFrame->m_wndSubtitlesUploadDialog.DoFinished(m_bAbort);
}
