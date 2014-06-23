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

#pragma once

#include "SubtitlesProvidersUtils.h"
#include "MediaInfo/thirdparty/base64/base64.h"
#include "MainFrm.h"
#include "version.h"

#define PROBE_SIZE  64 * 1024

struct SubtitlesInfo;
typedef std::list<SubtitlesInfo> SubtitlesList;
class SubtitlesProvider;
class SubtitlesProviders;
struct SubtitlesThreadParam;
class SubtitlesThread;


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
    HRESULT GetFileInfo(const std::string& _fileName = std::string());
    //HRESULT GetCurrentSubtitles();
    void Download(BOOL bActivate);
    void OpenUrl() const;
    SubtitlesProvider& Provider() const { return *fileProvider; }
    void Provider(SubtitlesProvider* _fileProvider) { fileProvider = _fileProvider; }
    DWORD Score() const { return score; }
    void Set(SubtitlesProvider* _fileProvider, BYTE _language, BYTE _hearingImpaired, SHORT _score) {
        static UINT i(0);
        // run twice to check whether i has reached MAXUINT32 which is invalid
        if (uid == -1) { uid = ++i; if (uid == -1) { uid = ++i; } }
        fileProvider = _fileProvider;
        score = MAKELONG(_score + 0x10, MAKEWORD(_hearingImpaired, _language));
    }

    std::string DisplayTitle() const {
        std::string _title(title);
        if (!title2.empty()) { _title.append(": " + title2); }
        if (year != -1) { _title.append(" (" + std::to_string(year) + ")"); }
        return _title;
    }

    std::string NormalizeString(std::string _title) const {
        // remove ' and ' from string and replace '!?&:\' with ' ' to get more accurate results
        _title = std::regex_replace(_title, std::regex(" and ", regex_flags), " ");
        _title = std::regex_replace(_title, std::regex(" *[!?&:] *", regex_flags), " ");
        _title = std::regex_replace(_title, std::regex("'", regex_flags), "");

        return _title;
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


class SubtitlesProvider
{
protected:
    SubtitlesProvider();

public: // implemented
    virtual std::string Name() PURE;
    virtual std::string Url() PURE;
    virtual std::string Languages() PURE;
    virtual BOOL Flags(DWORD flag) PURE;
    virtual int Icon() PURE;
    virtual SRESULT Search(const SubtitlesInfo& fileInfo, volatile BOOL& _bAborting) PURE;
    virtual SRESULT Download(SubtitlesInfo& subtitlesInfo, volatile BOOL& _bAborting) PURE;

protected: // overridden
    virtual void Initialize() {}
    virtual void Uninitialize() {}
public: // overridden
    virtual SRESULT Login(std::string& _UserName, std::string& _Password) { return SR_UNDEFINED; }
    virtual SRESULT Hash(SubtitlesInfo& fileInfo) { return SR_UNDEFINED; }
    virtual SRESULT Upload(const SubtitlesInfo& fileInfo, volatile BOOL& _bAborting) { return SR_UNDEFINED; };
    virtual std::string UserAgent() { return string_format("mpc-hc v%S", MPC_VERSION_STR); }

public:
    BOOL Login();
    void OpenUrl();
    size_t Index() const;
    BOOL CheckInternetConnection();
    bool CheckLanguage(const std::string& languageCode) const;
    SRESULT Download(std::string url, std::string referer, std::string& data);
    void Set(const SubtitlesInfo& fileInfo, SubtitlesInfo& subtitlesInfo, volatile BOOL& _bAborting);
public:
    BOOL Enabled(SubtitlesProviderFlags _flag) { return _flag == SPF_UPLOAD ? m_Upload : m_Search; }
    void Enabled(SubtitlesProviderFlags _flag, BOOL _Enabled) { if (_flag == SPF_UPLOAD) { m_Upload = _Enabled; } else { m_Search = _Enabled; } }
    std::string UserName() { return m_UserName; };
    void UserName(std::string _UserName) { m_UserName = _UserName; };
    std::string Password(BOOL _Decrypt = TRUE) { return _Decrypt ? string_decrypt(m_Password, string_generate_unique_key()) : m_Password; };
    void Password(std::string _Password, BOOL _Encrypt = TRUE) { m_Password = _Encrypt ? string_encrypt(_Password, string_generate_unique_key()) : _Password; };
    SubtitlesProviders& Providers() { return m_Providers; }
    int GetIconIndex() { return m_nIconIndex; }
    void SetIconIndex(int nIconIndex) { m_nIconIndex = nIconIndex; }

private:
    BOOL m_Search;
    BOOL m_Upload;
    std::string m_UserName;
    std::string m_Password;
    SubtitlesProviders& m_Providers;
    SubtitlesProviderLogin m_LoggedIn;
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
    void Download(SubtitlesInfo& fileInfo, BOOL bActivate);
    void Abort(SubtitlesThreadType type);

    void InsertThread(SubtitlesThread* pThread) {
        CAutoLock cAutoLock(&m_csSyncThreads);
        m_pThreads.push_back(pThread);
    }

    void RemoveThread(SubtitlesThread* pThread) {
        CAutoLock cAutoLock(&m_csSyncThreads);
        m_pThreads.remove(pThread);
    }

    void MoveUp(size_t index) {
        std::iter_swap(m_Providers.begin() + index, m_Providers.begin() + index - 1);
    }

    void MoveDown(size_t index) {
        std::iter_swap(m_Providers.begin() + index, m_Providers.begin() + index + 1);
    }

    CImageList& GetImageList() { return m_himl; }
private:
    CMainFrame& m_pMainFrame;

    std::vector<SubtitlesProvider*> m_Providers;

    CCritSec m_csSyncThreads;
    std::list<SubtitlesThread*> m_pThreads;
    CImageList m_himl;
};

struct SubtitlesThreadParam {
    SubtitlesThreadParam(SubtitlesThread* _pThread, SubtitlesProvider* _fileProvider, SubtitlesInfo _fileInfo)
        : pThread(_pThread), fileInfo(_fileInfo) { fileInfo.Provider(_fileProvider); }
    SubtitlesThread* pThread;
    SubtitlesInfo fileInfo;
    SubtitlesList subtitlesList;
};


class SubtitlesThread : public CWinThread
{
    DECLARE_DYNCREATE(SubtitlesThread);
public:
    SubtitlesThread() : m_pMainFrame(nullptr), m_pProviders(nullptr), m_bAbort(FALSE), m_bAutoDownload(FALSE), m_Type(STT_UNDEFINED) {}

    BOOL InitInstance();
    int ExitInstance();
    void Initialize(CMainFrame* pMainFrame, SubtitlesProviders* pProviders, SubtitlesThreadType Type);

    SubtitlesThreadType Type() { return m_Type; }
    std::string ThreadName();
    volatile BOOL& CheckAbort() { return m_bAbort; }
    void CheckAbortAndThrow() { if (m_bAbort) { throw E_ABORT; } }
    std::string Languages() { return m_Languages; };


    void Quit();
    void Abort();
    void Search(BOOL bAutoDownload, std::string languages);
    void Upload(SubtitlesInfo info);
    void Download(SubtitlesInfo& info, BOOL bActivate);

    enum {
        TM_QUIT = WM_APP,
        TM_SEARCH,
        TM_UPLOAD,
        TM_DOWNLOAD,
    };

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnQuit(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSearch(WPARAM wParam, LPARAM lParam);
    afx_msg void OnUpload(WPARAM wParam, LPARAM lParam);
    afx_msg void OnDownload(WPARAM wParam, LPARAM lParam);

private:
    static UINT _WorkerProc(LPVOID pParam);

    void _SearchInitialize();
    void _SearchProc(SubtitlesInfo& _fileInfo, SubtitlesList& _subtitlesList);
    void _SearchFinalize();

    void _DownloadInitialize();
    void _DownloadProc(SubtitlesInfo& _fileInfo, BOOL bActivate);
    void _DownloadProc(SubtitlesList& _subtitlesList);
    void _DownloadFinalize();

    void _UploadInitialize();
    void _UploadProc(SubtitlesInfo& _fileInfo);
    void _UploadFinalize();

private:
    CMainFrame* m_pMainFrame;
    SubtitlesProviders* m_pProviders;

    CCritSec m_csSyncThreads;
    std::list<CWinThread*> m_pThreads;

    SubtitlesInfo m_fileInfo;
    BOOL m_bActivate;
    SubtitlesThreadType m_Type;
    volatile BOOL m_bAbort;
    BOOL m_bAutoDownload;
    std::unordered_map<std::string, BOOL> m_AutoDownload;
    std::string m_Languages;
};
