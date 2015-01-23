/*
 * (C) 2014-2015 see Authors.txt
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

#include "SubtitlesProvidersUtils.h"
#include "MediaInfo/thirdparty/base64/base64.h"
#include "MainFrm.h"
#include "version.h"

#define PROBE_SIZE  64 * 1024

struct SubtitlesInfo;
typedef std::list<SubtitlesInfo> SubtitlesList;
class SubtitlesThread;
class SubtitlesTask;
class SubtitlesProvider;
class SubtitlesProviders;


enum SubtitlesProviderFlags {
    SPF_SEARCH = 0x00000000,
    SPF_LOGIN  = 0x00000001,
    SPF_HASH   = 0x00000002,
    SPF_UPLOAD = 0x00000004,
};

enum SubtitlesProviderLogin {
    SPL_UNDEFINED  = 0x00000000,
    SPL_FAILED     = 0x00000001,
    SPL_ANONYMOUS  = 0x00000002,
    SPL_REGISTERED = 0x00000004,
};

enum SRESULT {
    SR_UNDEFINED,
    SR_SUCCEEDED,
    SR_FAILED,
    SR_ABORTED,
    // Specific to search only
    SR_TOOMANY,
    // Specific to upload only
    SR_EXISTS,
};

enum SubtitlesThreadType {
    STT_UNDEFINED = 0x00000000,
    STT_SEARCH    = 0x00000001,
    STT_DOWNLOAD  = 0x00000002,
    STT_UPLOAD    = 0x00000004,
};


struct SubtitlesInfo {
    SubtitlesInfo() : fileSize(-1), uid(-1)
        , year(-1), seasonNumber(-1), episodeNumber(-1), discNumber(-1)
        , hearingImpaired(-1), discCount(-1), downloadCount(-1), corrected(0), score(0)
        , frameRate(-1), framesNumber(-1), length(-1), fileProvider(nullptr) {}
    bool operator<(const SubtitlesInfo& rhs) const { return score > rhs.score; }
    HRESULT GetFileInfo(const std::string& sFileName = std::string());
    //HRESULT GetCurrentSubtitles();
    void Download(BOOL bActivate);
    void OpenUrl() const;
    SubtitlesProvider& Provider() const { return *fileProvider; }
    void Provider(SubtitlesProvider* pProvider) { fileProvider = pProvider; }
    DWORD Score() const { return score; }
    void Set(SubtitlesProvider* pProvider, BYTE nLanguage, BYTE nHearingImpaired, SHORT nScore) {
        static UINT i(0);
        // run twice to check whether i has reached MAXUINT32 which is invalid
        if (uid == -1) { uid = ++i; if (uid == -1) { uid = ++i; } }
        fileProvider = pProvider;
        score = MAKELONG(nScore + 0x10, MAKEWORD(nHearingImpaired, nLanguage));
    }

    std::string DisplayTitle() const {
        std::string _title(title);
        if (!title2.empty()) { _title.append(": " + title2); }
        if (year != -1) { _title.append(" (" + std::to_string(year) + ")"); }
        return _title;
    }

    std::string NormalizeString(std::string sTitle) const {
        // remove ' and ' from string and replace '!?&:\' with ' ' to get more accurate results
        sTitle = std::regex_replace(sTitle, std::regex(" and ", regex_flags), " ");
        sTitle = std::regex_replace(sTitle, std::regex(" *[!?&:] *", regex_flags), " ");
        sTitle = std::regex_replace(sTitle, std::regex("'", regex_flags), "");

        return sTitle;
    }

    std::string NormalizeTitle() const {
        return NormalizeString(title);
    }

    UINT UID() { return uid; }

private:
    SubtitlesProvider* fileProvider;
    UINT uid;
    DWORD score;
public:
    // file properties
    std::string filePath;
    std::string fileName;
    std::string fileExtension;
    ULONGLONG fileSize;
    std::string fileContents;
    std::string fileHash;
    CComQIPtr<IAsyncReader> pAsyncReader;

    // file name properties
    std::string title;
    std::string country;
    int year;
    std::string episode;
    int seasonNumber;
    int episodeNumber;
    std::string title2;
    std::string resolution;
    std::string format;
    std::string audioCodec;
    std::string videoCodec;
    std::string releaseGroup;
    int discNumber;

    // subtitles properties
    std::string id;
    std::string imdbid;
    std::string languageCode;
    std::string languageName;
    std::string url;
    std::string releaseName;
    int hearingImpaired;
    int discCount;
    int downloadCount;
    int corrected;

    // video properties
    double frameRate;
    int framesNumber;
    ULONGLONG length;
};


class CWinThreadProc
{
public:
    CWinThreadProc() : m_bAbort(FALSE), m_pThread(nullptr) {}
    ~CWinThreadProc() { }
    operator CWinThread* () const { return m_pThread; }

    bool IsThreadRunning() { return m_pThread != nullptr; }
    volatile BOOL& IsThreadAborting() { return m_bAbort; }

    CWinThread* CreateThread() { if (!IsThreadRunning()) { m_pThread = AfxBeginThread(_ThreadProc, this); } return m_pThread; }
    void AbortThread() { if (IsThreadRunning()) { m_bAbort = TRUE; } }
    void WaitThread() { if (IsThreadRunning()) { DWORD dwWait = ::WaitForSingleObjectEx(*m_pThread, INFINITE, TRUE); } }

private:
    static UINT _ThreadProc(LPVOID pThreadParams) {
        CWinThreadProc* pThread = (CWinThreadProc*)pThreadParams;
        pThread->ThreadProc();
        return 0;
    }
    virtual void ThreadProc() PURE;

    CWinThread* m_pThread;
    volatile BOOL m_bAbort;
};


class SubtitlesThread : public CWinThreadProc
{
    friend class SubtitlesProvider;
    friend class SubtitlesTask;
public:
    SubtitlesThread(SubtitlesTask* pTask, SubtitlesInfo pFileInfo, SubtitlesProvider* pProvider)
        : m_pTask(pTask), m_pFileInfo(pFileInfo) { m_pFileInfo.Provider(pProvider); }

private:
    void Set(SubtitlesInfo& pSubtitlesInfo);
    std::string& Languages();

private:
    virtual void ThreadProc();
    void Search();
    void Download(SubtitlesInfo& pFileInfo, BOOL bActivate);
    void Download();
    void Upload();

private:
    void CheckAbortAndThrow() { if (IsThreadAborting()) { throw E_ABORT; } }

private:
    SubtitlesTask* m_pTask;
    SubtitlesInfo m_pFileInfo;
    SubtitlesList m_pSubtitlesList;
};


class SubtitlesTask : public CWinThreadProc
{
    friend class SubtitlesThread;
public:
    // Search
    SubtitlesTask(CMainFrame* pMainFrame, BOOL bAutoDownload, std::string sLanguages);
    // Download
    SubtitlesTask(CMainFrame* pMainFrame, SubtitlesInfo& pSubtitlesInfo, BOOL bActivate);
    // Upload
    SubtitlesTask(CMainFrame* pMainFrame, const SubtitlesInfo& pSubtitlesInfo);

public:
    SubtitlesThreadType Type() { return m_nType; }
    std::string Languages() { return m_sLanguages; };

    void InsertThread(SubtitlesThread* pThread) {
        CAutoLock cAutoLock(&m_csThreads);
        m_pThreads.push_back(pThread);
    }

    void RemoveThread(SubtitlesThread* pThread) {
        CAutoLock cAutoLock(&m_csThreads);
        m_pThreads.remove(pThread);
        delete pThread;
    }

    void Abort() {
        CAutoLock cAutoLock(&m_csThreads);
        for (auto& iter : m_pThreads) {
            iter->AbortThread();
        }
        AbortThread();
    }

private:
    virtual void ThreadProc();

private:
    CMainFrame* m_pMainFrame;
    std::list<SubtitlesThread*> m_pThreads;
    CCritSec m_csThreads;
    CCritSec m_csDownload;

    SubtitlesInfo m_pFileInfo;
    BOOL m_bActivate;
    SubtitlesThreadType m_nType;
    BOOL m_bAutoDownload;
    std::unordered_map<std::string, BOOL> m_AutoDownload;
    std::string m_sLanguages;
};

class SubtitlesProvider
{
protected:
    SubtitlesProvider();

public: // implemented
    virtual std::string Name() PURE;
    virtual std::string Url() PURE;
    virtual std::string Languages() PURE;
    virtual BOOL Flags(DWORD dwFlags) PURE;
    virtual int Icon() PURE;
    virtual SRESULT Search(const SubtitlesInfo& pFileInfo) PURE;
    virtual SRESULT Download(SubtitlesInfo& pSubtitlesInfo) PURE;

protected: // overridden
    virtual void Initialize() {}
    virtual void Uninitialize() {}
public: // overridden
    virtual SRESULT Login(std::string& sUserName, std::string& sPassword) { return SR_UNDEFINED; }
    virtual SRESULT Hash(SubtitlesInfo& pFileInfo) { return SR_UNDEFINED; }
    virtual SRESULT Upload(const SubtitlesInfo& pSubtitlesInfo) { return SR_UNDEFINED; };
    virtual std::string UserAgent() { return string_format("mpc-hc v%S", MAKE_STR(MPC_VERSION_MAJOR) _T(".") MAKE_STR(MPC_VERSION_MINOR) _T(".") MAKE_STR(MPC_VERSION_PATCH)); }

public:
    BOOL Login();
    void OpenUrl();
    size_t Index() const;
    BOOL CheckInternetConnection();
    bool CheckLanguage(const std::string& sLanguageCode) const;
    SRESULT Download(std::string url, std::string referer, std::string& data);
    void Set(SubtitlesInfo& pSubtitlesInfo);
    BOOL IsAborting();


public:
    BOOL Enabled(SubtitlesProviderFlags nFlag) { return nFlag == SPF_UPLOAD ? m_bUpload : m_bSearch; }
    void Enabled(SubtitlesProviderFlags nFlag, BOOL bEnabled) { if (nFlag == SPF_UPLOAD) { m_bUpload = bEnabled; } else { m_bSearch = bEnabled; } }
    std::string UserName() { return m_sUserName; };
    void UserName(std::string sUserName) { m_sUserName = sUserName; };
    std::string Password(BOOL bDecrypt = TRUE) { return bDecrypt ? string_decrypt(m_sPassword, string_generate_unique_key()) : m_sPassword; };
    void Password(std::string sPassword, BOOL bEncrypt = TRUE) { m_sPassword = bEncrypt ? string_encrypt(sPassword, string_generate_unique_key()) : sPassword; };
    SubtitlesProviders& Providers() { return m_Providers; }
    int GetIconIndex() { return m_nIconIndex; }
    void SetIconIndex(int nIconIndex) { m_nIconIndex = nIconIndex; }

private:
    BOOL m_bSearch;
    BOOL m_bUpload;
    std::string m_sUserName;
    std::string m_sPassword;
    SubtitlesProviders& m_Providers;
    SubtitlesProviderLogin m_nLoggedIn;
    int m_nIconIndex;
};

class SubtitlesProviders
{
private:
    SubtitlesProviders();                                     // Private constructor
    ~SubtitlesProviders();                                    // Private destructor
    SubtitlesProviders(SubtitlesProviders const&);            // Prevent copy-construction
    SubtitlesProviders& operator=(SubtitlesProviders const&); // Prevent assignment
public:
    // Instantiated on first use and guaranteed to be destroyed.
    static SubtitlesProviders& Instance() { static SubtitlesProviders that; return that; }

private:
    void RegisterProviders();
    template <class T>
    void Register() {
        m_Providers.push_back((SubtitlesProvider*)&T::Instance());
        auto& provider = m_Providers.back();
        HICON hIcon = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(provider->Icon()));
        provider->SetIconIndex(m_himl.Add(hIcon));
        DestroyIcon(hIcon);
    }

public:
    std::vector<SubtitlesProvider*>& Providers() { return m_Providers; };
    BOOL SubtitlesProviders::CheckInternetConnection();
    void ReadSettings();
    std::string WriteSettings();

    void Search(BOOL bAutoDownload);
    void Upload();
    void Download(SubtitlesInfo& pSubtitlesInfo, BOOL bActivate);
    void Abort(SubtitlesThreadType nType);

    void InsertTask(SubtitlesTask* pTask) {
        CAutoLock cAutoLock(&m_csTasks);
        m_pTasks.push_back(pTask);
    }

    void RemoveTask(SubtitlesTask* pTask) {
        CAutoLock cAutoLock(&m_csTasks);
        m_pTasks.remove(pTask);
        delete pTask;
    }

    void MoveUp(size_t nIndex) {
        std::iter_swap(m_Providers.begin() + nIndex, m_Providers.begin() + nIndex - 1);
    }

    void MoveDown(size_t nIndex) {
        std::iter_swap(m_Providers.begin() + nIndex, m_Providers.begin() + nIndex + 1);
    }

    CImageList& GetImageList() { return m_himl; }

private:
    CMainFrame& m_pMainFrame;

    std::vector<SubtitlesProvider*> m_Providers;

    CCritSec m_csTasks;
    std::list<SubtitlesTask*> m_pTasks;
    CImageList m_himl;
};

