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

HRESULT SubtitlesInfo::GetFileInfo(const std::string& sFileName /*= std::string()*/)
{
    if (sFileName.empty()) {
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
        filePath = sFileName;
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

SubtitlesProvider::SubtitlesProvider()
    : m_Providers(SubtitlesProviders::Instance()), m_bSearch(FALSE), m_bUpload(FALSE), m_nLoggedIn(SPL_UNDEFINED)
{
}

void SubtitlesProvider::OpenUrl()
{
    ShellExecute((HWND)AfxGetMyApp()->GetMainWnd(), _T("open"), UTF8To16(Url().c_str()), nullptr, nullptr, SW_SHOWDEFAULT);
}

BOOL SubtitlesProvider::Login()
{
    HRESULT hr = S_OK;
    if (!(m_nLoggedIn & (SPL_REGISTERED | SPL_ANONYMOUS))) {
        hr = Login(UserName(), Password());
        m_nLoggedIn = (hr == S_OK) ? (!UserName().empty() ? SPL_REGISTERED : SPL_ANONYMOUS) :
                      (hr == E_NOTIMPL) ? SPL_ANONYMOUS : SPL_FAILED;
    }
    return m_nLoggedIn & (SPL_REGISTERED | SPL_ANONYMOUS) ? TRUE : FALSE;
}


bool SubtitlesProvider::CheckLanguage(const std::string& sLanguageCode) const
{
    SubtitlesThread& pThread = *((SubtitlesThread*)AfxGetThread()->m_pThreadParams);
    return ((pThread.Languages().empty()) || (pThread.Languages().find(sLanguageCode) != std::string::npos));
}

void SubtitlesProvider::Set(SubtitlesInfo& pSubtitlesInfo)
{
    SubtitlesThread& pThread = *((SubtitlesThread*)AfxGetThread()->m_pThreadParams);
    pThread.Set(pSubtitlesInfo);
}

BOOL SubtitlesProvider::IsAborting()
{
    SubtitlesThread& pThread = *((SubtitlesThread*)AfxGetThread()->m_pThreadParams);
    return pThread.IsThreadAborting();
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

SubtitlesProviders::SubtitlesProviders()
    : m_pMainFrame(*(CMainFrame*)(AfxGetMyApp()->GetMainWnd()))
{
    m_himl.Create(16, 16, ILC_COLOR32, 0, 0);
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
        InsertTask(DEBUG_NEW SubtitlesTask(&m_pMainFrame, bAutoDownload, LanguagesISO6391()));
    } else if (bAutoDownload == FALSE) {
        m_pMainFrame.m_wndSubtitlesDownloadDialog.DoFailed();
    }
}

void SubtitlesProviders::Download(SubtitlesInfo& pSubtitlesInfo, BOOL bActivate)
{
    if (CheckInternetConnection()) {
        InsertTask(DEBUG_NEW SubtitlesTask(&m_pMainFrame, pSubtitlesInfo, bActivate));
    }
}

void SubtitlesProviders::Upload()
{
    if (CheckInternetConnection()) {
        // We get all the information we need within the main thread, to delay closing the file
        // until we have everything we need.
        SubtitlesInfo pSubtitlesInfo;
        m_pMainFrame.SendMessage(WM_GETSUBTITLES, 0, (LPARAM)&pSubtitlesInfo);
        //pSubtitlesInfo.GetCurrentSubtitles();

        if (!pSubtitlesInfo.fileContents.empty()) {
            InsertTask(DEBUG_NEW SubtitlesTask(&m_pMainFrame, pSubtitlesInfo));
        }
    }
}

void SubtitlesProviders::Abort(SubtitlesThreadType nType)
{
    CAutoLock cAutoLock(&m_csTasks);
    for (auto& pTask : m_pTasks) {
        if (pTask->Type() & nType) {
            pTask->Abort();
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

/******************************************************************************
** SubtitlesTask
******************************************************************************/

SubtitlesTask::SubtitlesTask(CMainFrame* pMainFrame, BOOL bAutoDownload, std::string sLanguages)
{
    m_pMainFrame = pMainFrame;
    m_nType = SubtitlesThreadType(STT_SEARCH | (bAutoDownload ? STT_DOWNLOAD : NULL));

    m_bAutoDownload = bAutoDownload;
    m_sLanguages = sLanguages;
    if (bAutoDownload) {
        string_array _languages(string_tokenize(sLanguages, ","));
        for (const auto& iter : _languages) {
            m_AutoDownload[iter] = FALSE;
        }
    }
    CreateThread();
}

SubtitlesTask::SubtitlesTask(CMainFrame* pMainFrame, SubtitlesInfo& pSubtitlesInfo, BOOL bActivate)
{
    m_pMainFrame = pMainFrame;
    m_nType = STT_DOWNLOAD;

    m_pFileInfo = pSubtitlesInfo;
    m_bActivate = bActivate;
    CreateThread();
}

SubtitlesTask::SubtitlesTask(CMainFrame* pMainFrame, const SubtitlesInfo& pSubtitlesInfo)
{
    m_pMainFrame = pMainFrame;
    m_nType = STT_UPLOAD;

    m_pFileInfo = pSubtitlesInfo;
    CreateThread();
}

void SubtitlesTask::ThreadProc()
{
    if (m_nType & STT_SEARCH) {
        // We get all the information we need within a separate thread,
        // to avoid delaying the video playback.
        SubtitlesInfo pFileInfo;
        pFileInfo.GetFileInfo();

        const auto& s = AfxGetAppSettings();
        std::string exclude = UTF16To8(s.strAutoDownloadSubtitlesExclude);
        string_array exclude_array = string_tokenize(exclude, "|;, ");
        for (auto& iter : exclude_array) {
            if (pFileInfo.filePath.find(iter) != std::string::npos) {
                return;
            }
        }

        if (!IsThreadAborting()) {
            for (const auto& iter : m_pMainFrame->m_pSubtitlesProviders->Providers()) {
                if (iter->Enabled(SPF_SEARCH)) {
                    InsertThread(DEBUG_NEW SubtitlesThread(this, pFileInfo, iter));
                }
            }
        }

    } else if (m_nType & STT_DOWNLOAD) {
        InsertThread(DEBUG_NEW SubtitlesThread(this, m_pFileInfo, &m_pFileInfo.Provider()));
    } else if (m_nType & STT_UPLOAD) {
        for (const auto& iter : m_pMainFrame->m_pSubtitlesProviders->Providers()) {
            if (iter->Enabled(SPF_UPLOAD) && iter->Flags(SPF_UPLOAD)) {
                InsertThread(DEBUG_NEW SubtitlesThread(this, m_pFileInfo, iter));
            }
        }
    }

    if (!m_pThreads.empty() && !IsThreadAborting()) {
        if (m_nType & STT_SEARCH) {
            m_pMainFrame->m_wndSubtitlesDownloadDialog.DoSearch((INT)m_pThreads.size());
        } else if (m_nType & STT_DOWNLOAD) {
        } else if (m_nType & STT_UPLOAD) {
            m_pMainFrame->m_wndSubtitlesUploadDialog.DoUpload((INT)m_pThreads.size());
        }

        CAutoLock cAutoLock(&m_csThreads);
        for (auto& iter : m_pThreads) {
            iter->CreateThread();

            // Provide a timing advantage for providers with higher priority
            if (m_nType & STT_SEARCH) {
                Sleep(250);
            }
        }
    }

    // Wait here until all threads have finished
    while (!m_pThreads.empty()) { Sleep(0); }

    if (m_nType & STT_SEARCH) {
        BOOL bShowDialog = !m_AutoDownload.empty() ? TRUE : m_bAutoDownload;
        for (const auto& iter : m_AutoDownload) {
            if (iter.second == TRUE) {
                bShowDialog = FALSE;
                break;
            }
        }
        m_pMainFrame->m_wndSubtitlesDownloadDialog.DoFinished(IsThreadAborting(), bShowDialog);
    } else if (m_nType & STT_DOWNLOAD) {
    } else if (m_nType & STT_UPLOAD) {
        m_pMainFrame->m_wndSubtitlesUploadDialog.DoFinished(IsThreadAborting());
    }

    m_pMainFrame->m_pSubtitlesProviders->RemoveTask(this);
}

/******************************************************************************
** SubtitlesThread
******************************************************************************/

void SubtitlesThread::ThreadProc()
{
    try {
        if (m_pFileInfo.Provider().Login() <= SR_SUCCEEDED) {
            if (m_pTask->m_nType & STT_SEARCH) {
                Search();
            } else if (m_pTask->m_nType & STT_DOWNLOAD) {
                Download(m_pFileInfo, m_pTask->m_bActivate);
            } else if (m_pTask->m_nType & STT_UPLOAD) {
                Upload();
            }
        } else {
            if (m_pTask->m_nType & STT_SEARCH) {
                m_pTask->m_pMainFrame->m_wndSubtitlesDownloadDialog.DoCompleted(SR_FAILED, m_pSubtitlesList);
            } else if (m_pTask->m_nType & STT_UPLOAD) {
                m_pTask->m_pMainFrame->m_wndSubtitlesUploadDialog.DoCompleted(SR_FAILED, m_pFileInfo.Provider());
            }
        }
    } catch (/*HRESULT e*/...) {
        if (m_pTask->m_nType & STT_SEARCH) {
            m_pTask->m_pMainFrame->m_wndSubtitlesDownloadDialog.DoCompleted(SR_ABORTED, m_pSubtitlesList);
        } else if (m_pTask->m_nType & STT_UPLOAD) {
            m_pTask->m_pMainFrame->m_wndSubtitlesUploadDialog.DoCompleted(SR_ABORTED, m_pFileInfo.Provider());
        }
    }

    // Don't exit thread until all threads have been created
    m_pTask->RemoveThread(this);
}

void SubtitlesThread::Search()
{
    CheckAbortAndThrow();
    m_pFileInfo.Provider().Hash(m_pFileInfo);
    CheckAbortAndThrow();
    m_pTask->m_pMainFrame->m_wndSubtitlesDownloadDialog.DoSearching(m_pFileInfo);
    SRESULT searchResult = m_pFileInfo.Provider().Search(m_pFileInfo);
    CheckAbortAndThrow();
    m_pSubtitlesList.sort();
    CheckAbortAndThrow();
    m_pTask->m_pMainFrame->m_wndSubtitlesDownloadDialog.DoCompleted(searchResult, m_pSubtitlesList);
    CheckAbortAndThrow();
    if (!m_pSubtitlesList.empty() && (m_pTask->m_nType & STT_DOWNLOAD) && !m_pTask->m_AutoDownload.empty()) {
        Download();
    }
}

void SubtitlesThread::Download()
{
    CAutoLock cAutoLock(&m_pTask->m_csDownload);
    CheckAbortAndThrow();
    for (auto& iter : m_pSubtitlesList) {
        CheckAbortAndThrow();
        BOOL bDownload = FALSE;
        for (const auto& language : m_pTask->m_AutoDownload) {
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
                Download(iter, TRUE);
            }
        }
    }
}

void SubtitlesThread::Download(SubtitlesInfo& pSubtitlesInfo, BOOL bActivate)
{
    CheckAbortAndThrow();
    m_pTask->m_pMainFrame->m_wndSubtitlesDownloadDialog.DoDownloading(pSubtitlesInfo);
    if (pSubtitlesInfo.Provider().Download(pSubtitlesInfo) == SR_SUCCEEDED) {
        CheckAbortAndThrow();
        string_map data = string_uncompress(pSubtitlesInfo.fileContents, pSubtitlesInfo.fileName);
        CheckAbortAndThrow();
        for (const auto& iter : data) {
            CheckAbortAndThrow();
            struct { SubtitlesInfo* pSubtitlesInfo; BOOL bActivate; std::string fileName; std::string fileContents; } data({ &pSubtitlesInfo, bActivate, iter.first, iter.second });
            if (m_pTask->m_pMainFrame->SendMessage(WM_LOADSUBTITLES, (BOOL)bActivate, (LPARAM)&data) == TRUE) {
                if (!m_pTask->m_AutoDownload.empty()) {
                    m_pTask->m_AutoDownload[pSubtitlesInfo.languageCode] = TRUE;
                }
            }
        }
    }
}

void SubtitlesThread::Upload()
{
    CheckAbortAndThrow();
    m_pTask->m_pMainFrame->m_wndSubtitlesUploadDialog.DoUploading(m_pFileInfo.Provider());
    m_pFileInfo.Provider().Hash(m_pFileInfo);
    CheckAbortAndThrow();
    SRESULT uploadResult = m_pFileInfo.Provider().Upload(m_pFileInfo);
    m_pTask->m_pMainFrame->m_wndSubtitlesUploadDialog.DoCompleted(uploadResult, m_pFileInfo.Provider());
}

std::string& SubtitlesThread::Languages()
{
    return m_pTask->m_sLanguages;
}


void SubtitlesThread::Set(SubtitlesInfo& pSubtitlesInfo)
{
    if (IsThreadAborting()) { return; }

    std::string _title = pSubtitlesInfo.title;
    if (!_title.empty()) { pSubtitlesInfo.title.clear(); }
    pSubtitlesInfo.GetFileInfo(pSubtitlesInfo.fileName);

    //iter.score = 0; //LevenshteinDistance(m_pFileInfo.fileName, string_(subtitlesName)) * 100;

    SHORT score = ((SHORT)pSubtitlesInfo.corrected);
    score += (((!m_pFileInfo.title.empty() && _strcmpi(m_pFileInfo.NormalizeTitle().c_str(), pSubtitlesInfo.NormalizeTitle().c_str()) == 0)) ||
              ((!_title.empty() && _strcmpi(m_pFileInfo.NormalizeTitle().c_str(), pSubtitlesInfo.NormalizeString(_title).c_str()) == 0)))
             ? 3 : !m_pFileInfo.title.empty() || !_title.empty() ? -3 : 0;

    if (!_title.empty()) { pSubtitlesInfo.title = _title; }

    score += (!m_pFileInfo.country.empty() && _strcmpi(m_pFileInfo.country.c_str(), pSubtitlesInfo.country.c_str()) == 0) ? 2 : !m_pFileInfo.country.empty() ? -2 : 0;
    score += (m_pFileInfo.year != -1 && m_pFileInfo.year == pSubtitlesInfo.year) ? 1 : 0;
    score += (m_pFileInfo.seasonNumber != -1 && m_pFileInfo.seasonNumber == pSubtitlesInfo.seasonNumber) ? 2 : m_pFileInfo.seasonNumber > 0 ? -2 : 0;
    score += (m_pFileInfo.episodeNumber != -1 && m_pFileInfo.episodeNumber == pSubtitlesInfo.episodeNumber) ? 2 : m_pFileInfo.episodeNumber > 0 ? -2 : 0;
    score += (!m_pFileInfo.title2.empty() && _strcmpi(m_pFileInfo.title2.c_str(), pSubtitlesInfo.title2.c_str()) == 0) ? 2 : 0;
    score += (!m_pFileInfo.resolution.empty() && _strcmpi(m_pFileInfo.resolution.c_str(), pSubtitlesInfo.resolution.c_str()) == 0) ? 1 : 0;
    score += (!m_pFileInfo.format.empty() && _strcmpi(m_pFileInfo.format.c_str(), pSubtitlesInfo.format.c_str()) == 0) ? 1 : 0;
    score += (!m_pFileInfo.audioCodec.empty() && _strcmpi(m_pFileInfo.audioCodec.c_str(), pSubtitlesInfo.audioCodec.c_str()) == 0) ? 1 : 0;
    score += (!m_pFileInfo.videoCodec.empty() && _strcmpi(m_pFileInfo.videoCodec.c_str(), pSubtitlesInfo.videoCodec.c_str()) == 0) ? 1 : 0;
    score += (!m_pFileInfo.releaseGroup.empty() && _strcmpi(m_pFileInfo.releaseGroup.c_str(), pSubtitlesInfo.releaseGroup.c_str()) == 0) ? 1 : 0;
    const auto& s = AfxGetAppSettings();

    if (IsThreadAborting()) { return; }

    pSubtitlesInfo.Set(&m_pFileInfo.Provider(),
                       (BYTE)((Languages().length() - Languages().find(pSubtitlesInfo.languageCode) + 1) / 3),
                       (BYTE)(pSubtitlesInfo.hearingImpaired == (int)s.bPreferHearingImpairedSubtitles),
                       score);

    if (IsThreadAborting()) { return; }

    m_pSubtitlesList.push_back(pSubtitlesInfo);
}
