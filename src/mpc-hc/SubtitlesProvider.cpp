/*
 * (C) 2016-2017 see Authors.txt
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
#include "SubtitlesProvider.h"
#include "SubtitlesProvidersUtils.h"
#include "mplayerc.h"
#include "ISOLang.h"
#include "Logger.h"
#include "base64/base64.h"
#include "tinyxml2/library/tinyxml2.h"
#include "rapidjson/include/rapidjson/document.h"
#include <wincrypt.h>

#define LOG if (AfxGetAppSettings().bEnableLogging) SUBTITLES_LOG
#define LOG_NONE    _T("()")
#define LOG_INPUT   _T("(\"%S\")")
#define LOG_OUTPUT  _T("()=%S")
#define LOG_BOTH    _T("(\"%S\")=%S")
#define LOG_ERROR   _T("() ERROR: %S")

#define GUESSED_NAME_POSTFIX " (*)"
#define CheckAbortAndReturn() { if (IsAborting()) return SR_ABORTED; }

// Uncomment define to compile the temporarily disabled subtitles providers.
// Subtitles providers are disabled in case their API ceases to work.
// In case the API is not restored in due time, the provider will eventually
// be removed from mpc-hc. Upon removal, also remove icon resources.
//#define MPCHC_DISABLED_SUBTITLES_PROVIDER

using namespace SubtitlesProvidersUtils;

class LanguageDownloadException : public std::exception
{
    using exception::exception;
};

/******************************************************************************
** Register providers
******************************************************************************/
void SubtitlesProviders::RegisterProviders()
{
    Register<OpenSubtitles>(this);
    Register<podnapisi>(this);
#ifdef MPCHC_DISABLED_SUBTITLES_PROVIDER
    Register<titlovi>(this);
#endif // MPCHC_DISABLED_SUBTITLES_PROVIDER
    Register<SubDB>(this);
#ifdef MPCHC_DISABLED_SUBTITLES_PROVIDER
    Register<ysubs>(this);
#endif // MPCHC_DISABLED_SUBTITLES_PROVIDER
    Register<Napisy24>(this);
}

/******************************************************************************
** OpenSubtitles
******************************************************************************/

void OpenSubtitles::Initialize()
{
    xmlrpc = std::make_unique<XmlRpcClient>((Url() + "/xml-rpc").c_str());
    xmlrpc->setIgnoreCertificateAuthority();
}

SRESULT OpenSubtitles::Login(const std::string& sUserName, const std::string& sPassword)
{
    if (xmlrpc) {
        XmlRpcValue args, result;
        args[0] = sUserName;
        args[1] = sPassword;
        args[2] = "en";
        const auto& strUA = UserAgent();
        args[3] = strUA.c_str(); // Test with "OSTestUserAgent"
        if (!xmlrpc->execute("LogIn", args, result)) {
            return SR_FAILED;
        }

        if (result["status"].getType() == XmlRpcValue::Type::TypeString) {
            if (result["status"] == std::string("200 OK")) {
                token = result["token"];
            } else if (result["status"] == std::string("401 Unauthorized")) {
                // Notify user that User/Pass provided are invalid.
                CString msg;
                msg.FormatMessage(IDS_SUB_CREDENTIALS_ERROR, Name().c_str(), UserName().c_str());
                AfxMessageBox(msg, MB_ICONERROR | MB_OK);
            }
        }
    }

    LOG(LOG_BOTH, sUserName.c_str(), token.valid() ? (LPCSTR)token : "failed");
    return token.valid() ? SR_SUCCEEDED : SR_FAILED;
}

SRESULT OpenSubtitles::LogOut()
{
    if (xmlrpc && token.valid()) {
        XmlRpcValue args, result;
        args[0] = token;
        VERIFY(xmlrpc->execute("LogOut", args, result));
        token.clear();
    }
    m_nLoggedIn = SPL_UNDEFINED;

    LOG(LOG_NONE);
    return SR_SUCCEEDED;
}

SRESULT OpenSubtitles::Hash(SubtitlesInfo& pFileInfo)
{
    pFileInfo.fileHash = StringFormat("%016I64x", GenerateOSHash(pFileInfo));
    LOG(LOG_OUTPUT, pFileInfo.fileHash.c_str());
    return SR_SUCCEEDED;
}

SRESULT OpenSubtitles::Search(const SubtitlesInfo& pFileInfo)
{
    const auto languages = LanguagesISO6392();
    XmlRpcValue args, result;

    args[0] = token;
    auto& movieInfo = args[1][0];
    args[2]["limit"] = 500;
    movieInfo["sublanguageid"] = !languages.empty() ? JoinContainer(languages, ",") : "all";
    if (pFileInfo.manualSearchString.IsEmpty()) {
        movieInfo["moviehash"] = pFileInfo.fileHash;
        movieInfo["moviebytesize"] = std::to_string(pFileInfo.fileSize);
        //args[1][1]["sublanguageid"] = !languages.empty() ? languages : "all";
        //args[1][1]["tag"] = pFileInfo.fileName + "." + pFileInfo.fileExtension;

        LOG(LOG_INPUT,
            StringFormat("{ sublanguageid=\"%s\", moviehash=\"%s\", moviebytesize=\"%s\", limit=%d }",
            (LPCSTR)movieInfo["sublanguageid"],
                (LPCSTR)movieInfo["moviehash"],
                (LPCSTR)movieInfo["moviebytesize"],
                (int)args[2]["limit"]).c_str());
    } else {
        CT2CA pszConvertedAnsiString(pFileInfo.manualSearchString);
        movieInfo["query"] = std::string(pszConvertedAnsiString);
    }

    if (!xmlrpc->execute("SearchSubtitles", args, result)) {
        LOG(_T("search failed"));
        return SR_FAILED;
    }

    if (result["data"].getType() != XmlRpcValue::Type::TypeArray) {
        LOG(_T("search failed (invalid data)"));
        return SR_FAILED;
    }
    
    int nCount = result["data"].size();
    for (int i = 0; i < nCount; ++i) {
        CheckAbortAndReturn();
        XmlRpcValue& data(result["data"][i]);
        SubtitlesInfo pSubtitlesInfo;
        pSubtitlesInfo.id = (const char*)data["IDSubtitleFile"];
        pSubtitlesInfo.discNumber = data["SubActualCD"];
        pSubtitlesInfo.discCount = data["SubSumCD"];
        pSubtitlesInfo.fileExtension = (const char*)data["SubFormat"];
        pSubtitlesInfo.languageCode = (const char*)data["ISO639"]; //"SubLanguageID"
        pSubtitlesInfo.languageName = (const char*)data["LanguageName"];
        pSubtitlesInfo.downloadCount = data["SubDownloadsCnt"];

        pSubtitlesInfo.fileName = (const char*)data["SubFileName"];
        regexResult results;
        stringMatch("\"([^\"]+)\" (.+)", (const char*)data["MovieName"], results);
        if (!results.empty()) {
            pSubtitlesInfo.title = results[0];
            pSubtitlesInfo.title2 = results[1];
        } else {
            pSubtitlesInfo.title = (const char*)data["MovieName"];
        }
        pSubtitlesInfo.year = (int)data["MovieYear"] == 0 ? -1 : (int)data["MovieYear"];
        pSubtitlesInfo.seasonNumber = (int)data["SeriesSeason"] == 0 ? -1 : (int)data["SeriesSeason"];
        pSubtitlesInfo.episodeNumber = (int)data["SeriesEpisode"] == 0 ? -1 : (int)data["SeriesEpisode"];
        pSubtitlesInfo.hearingImpaired = data["SubHearingImpaired"];
        pSubtitlesInfo.url = (const char*)data["SubtitlesLink"];
        pSubtitlesInfo.releaseNames.emplace_back((const char*)data["MovieReleaseName"]);
        pSubtitlesInfo.imdbid = (const char*)data["IDMovieImdb"];
        pSubtitlesInfo.corrected = (int)data["SubBad"] ? -1 : 0;
        Set(pSubtitlesInfo);
    }
    LOG(std::to_wstring(nCount).c_str());
    return SR_SUCCEEDED;
}

SRESULT OpenSubtitles::Download(SubtitlesInfo& pSubtitlesInfo)
{
    XmlRpcValue args, result;
    args[0] = token;
    args[1][0] = pSubtitlesInfo.id;
    if (!xmlrpc->execute("DownloadSubtitles", args, result)) {
        return SR_FAILED;
    }

    LOG(LOG_INPUT, pSubtitlesInfo.id.c_str());

    if (result["data"].getType() != XmlRpcValue::Type::TypeArray) {
        LOG(_T("download failed (invalid type)"));
        return SR_FAILED;
    }

    pSubtitlesInfo.fileContents = Base64::decode(std::string(result["data"][0]["data"]));
    return SR_SUCCEEDED;
}

SRESULT OpenSubtitles::Upload(const SubtitlesInfo& pSubtitlesInfo)
{
    XmlRpcValue args, result;
    args[0] = token;

    //TODO: Ask how to obtain commented values !!!
    args[1]["cd1"]["subhash"] = StringToHash(pSubtitlesInfo.fileContents, CALG_MD5);
    args[1]["cd1"]["subfilename"] = pSubtitlesInfo.fileName + ".srt";
    args[1]["cd1"]["moviehash"] = pSubtitlesInfo.fileHash;
    args[1]["cd1"]["moviebytesize"] = std::to_string(pSubtitlesInfo.fileSize);
    //args[1]["cd1"]["movietimems"];
    //args[1]["cd1"]["movieframes"];
    //args[1]["cd1"]["moviefps"];
    args[1]["cd1"]["moviefilename"] = pSubtitlesInfo.fileName + "." + pSubtitlesInfo.fileExtension;

    CheckAbortAndReturn();
    if (!xmlrpc->execute("TryUploadSubtitles", args, result)) {
        LOG(_T("TryUploadSubtitles failed"));
        return SR_FAILED;
    }
    CheckAbortAndReturn();

    if ((int)result["alreadyindb"] == 1) {
        LOG(_T("File already in database"));
        return SR_EXISTS;
    } else if ((int)result["alreadyindb"] == 0) {
        LOG(_T("Trying to determine IMDB ID"));
        // We need imdbid to proceed
        if (result["data"].getType() == XmlRpcValue::Type::TypeArray) {
            args[1]["baseinfo"]["idmovieimdb"] = result["data"][0]["IDMovieImdb"];
        } else if (!pSubtitlesInfo.imdbid.empty()) {
            args[1]["baseinfo"]["idmovieimdb"] = pSubtitlesInfo.imdbid;
        } else {
            std::string title(StringReplace(pSubtitlesInfo.title, "and", "&"));
            if (!args[1]["baseinfo"]["idmovieimdb"].valid()) {
                XmlRpcValue _args, _result;
                _args[0] = token;
                _args[1][0] = pSubtitlesInfo.fileHash;
                if (!xmlrpc->execute("CheckMovieHash", _args, _result)) {
                    LOG(_T("CheckMovieHash fail"));
                    return SR_FAILED;
                }

                if (_result["data"].getType() == XmlRpcValue::Type::TypeStruct) {
                    //regexResults results;
                    //stringMatch("\"(.+)\" (.+)", (const char*)data["MovieName"], results);
                    //if (!results.empty()) {
                    //    pSubtitlesInfo.title = results[0][0];
                    //    pSubtitlesInfo.title2 = results[0][1];
                    //} else {
                    //    pSubtitlesInfo.title = (const char*)data["MovieName"];
                    //}
                    regexResults results;
                    stringMatch("\"(.+)\" .+|(.+)", StringReplace((const char*)_result["data"][pSubtitlesInfo.fileHash]["MovieName"], "and", "&"), results);
                    std::string _title(results[0][0] + results[0][1]);

                    if (_stricmp(title.c_str(), _title.c_str()) == 0 /*&& (pSubtitlesInfo.year == -1 || (pSubtitlesInfo.year != -1 && pSubtitlesInfo.year == atoi(_result["data"][pSubtitlesInfo.fileHash]["MovieYear"])))*/) {
                        args[1]["baseinfo"]["idmovieimdb"] = _result["data"][pSubtitlesInfo.fileHash]["MovieImdbID"]; //imdbid
                    }
                }
            }

            if (!args[1]["baseinfo"]["idmovieimdb"].valid()) {
                XmlRpcValue _args, _result;
                _args[0] = token;
                _args[1][0] = pSubtitlesInfo.fileHash;
                if (!xmlrpc->execute("CheckMovieHash2", _args, _result)) {
                    return SR_FAILED;
                }

                if (_result["data"].getType() == XmlRpcValue::Type::TypeArray) {
                    int nCount = _result["data"][pSubtitlesInfo.fileHash].size();
                    for (int i = 0; i < nCount; ++i) {
                        regexResults results;
                        stringMatch("\"(.+)\" .+|(.+)", StringReplace((const char*)_result["data"][pSubtitlesInfo.fileHash][i]["MovieName"], "and", "&"), results);
                        std::string _title(results[0][0] + results[0][1]);

                        if (_stricmp(title.c_str(), _title.c_str()) == 0 /*&& (pSubtitlesInfo.year == -1 || (pSubtitlesInfo.year != -1 && pSubtitlesInfo.year == atoi(_result["data"][pSubtitlesInfo.fileHash][i]["MovieYear"])))*/) {
                            args[1]["baseinfo"]["idmovieimdb"] = _result["data"][pSubtitlesInfo.fileHash][i]["MovieImdbID"]; //imdbid
                            break;
                        }
                    }
                }
            }

            if (!args[1]["baseinfo"]["idmovieimdb"].valid()) {
                XmlRpcValue _args, _result;
                _args[0] = token;
                _args[1] = title;
                if (!xmlrpc->execute("SearchMoviesOnIMDB", _args, _result)) {
                    return SR_FAILED;
                }
                if (_result["data"].getType() == XmlRpcValue::Type::TypeArray) {
                    int nCount = _result["data"].size();
                    for (int i = 0; i < nCount; ++i) {
                        regexResults results;
                        stringMatch("(.+) [(](\\d{4})[)]", StringReplace((const char*)_result["data"][i]["title"], "and", "&"), results);
                        if (results.size() == 1) {
                            std::string _title(results[0][0]);

                            if (_stricmp(title.c_str(), _title.c_str()) == 0 /*&& (pSubtitlesInfo.year == -1 || (pSubtitlesInfo.year != -1 && pSubtitlesInfo.year == atoi(results[0][1].c_str())))*/) {
                                args[1]["baseinfo"]["idmovieimdb"] = _result["data"][i]["id"]; //imdbid
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (args[1]["baseinfo"]["idmovieimdb"].valid()) {
            XmlRpcValue _args, _result;
            _args[0] = token;
            _args[1][0]["moviehash"] = pSubtitlesInfo.fileHash;
            _args[1][0]["moviebytesize"] = (int)pSubtitlesInfo.fileSize;
            _args[1][0]["imdbid"] = args[1]["baseinfo"]["idmovieimdb"];
            //_args[1][0]["movietimems"];
            //_args[1][0]["moviefps"];
            _args[1][0]["moviefilename"] = pSubtitlesInfo.fileName + "." + pSubtitlesInfo.fileExtension;
            if (!xmlrpc->execute("InsertMovieHash", _args, _result)) {
                LOG(_T("InsertMovieHash fail"));
                return SR_FAILED;
            }
            // REsult value is irrelevant
            _result["data"]["accepted_moviehashes"];


            //args[1]["baseinfo"]["moviereleasename"];
            //args[1]["baseinfo"]["movieaka"];
            //args[1]["baseinfo"]["sublanguageid"];
            //args[1]["baseinfo"]["subauthorcomment"];
            if (pSubtitlesInfo.hearingImpaired != -1) {
                args[1]["baseinfo"]["hearingimpaired"] = pSubtitlesInfo.hearingImpaired;
            }
            //args[1]["baseinfo"]["highdefinition"];
            //args[1]["baseinfo"]["automatictranslation"];

            args[1]["cd1"]["subcontent"] = Base64::encode(StringGzipCompress(pSubtitlesInfo.fileContents));

            if (!xmlrpc->execute("UploadSubtitles", args, result)) {
                LOG(_T("Upload failed"));
                return SR_FAILED;
            }
            LOG(LOG_OUTPUT, (LPCSTR)result["data"]);
            LOG(_T("Upload succeeded"));

            return SR_SUCCEEDED;
        }
    }
    LOG(_T("Upload failed"));
    return SR_FAILED;
}

const std::set<std::string>& OpenSubtitles::Languages() const
{
    static std::once_flag initialized;
    static std::set<std::string> result;
#if 1
    result = {"af","an","ar","at","az","be","bg","bn","br","bs","ca","cs","da","de","el","en","eo","es","et","eu","ex","fa","fi","fr","ga","gd","gl","he","hi","hr","hu","hy","id","ig","is","it","ja","ka","kk","km","kn","ko","ku","lb","lt","lv","ma","me","mk","ml","mn","ms","my","nl","no","oc","pb","pl","pm","pt","ro","ru","sd","se","si","sk","sl","so","sq","sr","sv","sw","sy","ta","te","th","tl","tr","tt","uk","ur","vi","ze","zh","zt"};
#else

    try {
        std::call_once(initialized, [this]() {
            if (!CheckInternetConnection()) {
                throw LanguageDownloadException("No internet connection.");
            }
            XmlRpcValue args, res;
            args = "en";
            if (!xmlrpc->execute("GetSubLanguages", args, res)) {
                throw LanguageDownloadException("Failed to execute xmlrpc command.");
            }
            if (res["data"].getType() != XmlRpcValue::Type::TypeArray) {
                throw LanguageDownloadException("Response is not an array.");
            }

            auto& data = res["data"];
            int count = data.size();
            for (int i = 0; i < count; ++i) {
#ifdef _DEBUG
                // Validate if language code conversion is in sync with OpenSubtitles database.
                std::string subLanguageID = data[i]["SubLanguageID"];
                std::string ISO6391 = data[i]["ISO639"];
                ASSERT(!ISO6391.empty());
                ASSERT(!subLanguageID.empty());
                ASSERT(ISOLang::ISO6391To6392(ISO6391.c_str()) == subLanguageID.c_str());
                ASSERT(ISOLang::ISO6392To6391(subLanguageID.c_str()) == ISO6391.c_str());
                //std::string languageName = data[i]["LanguageName"];
                //ASSERT(ISO639XToLanguage(ISO6391.c_str()) == languageName.c_str());
                //ASSERT(ISO639XToLanguage(subLanguageID.c_str()) == languageName.c_str());
#endif
                result.emplace(data[i]["ISO639"]);
            }
        });
    } catch (const LanguageDownloadException& e) {
        UNREFERENCED_PARAMETER(e);
        LOG(LOG_ERROR, e.what());
    }
#endif
    return result;
}

bool OpenSubtitles::NeedLogin()
{
    // return true to call Login() or false to skip Login()
    if (!token.valid()) {
        return true;
    }

    XmlRpcValue args, result;
    args[0] = token;
    if (!xmlrpc->execute("NoOperation", args, result)) {
        return false;
    }

    if ((result["status"].getType() == XmlRpcValue::Type::TypeString) && (result["status"] == std::string("200 OK"))) {
        return false;
    }

    return true;
}


/******************************************************************************
** SubDB
******************************************************************************/

SRESULT SubDB::Hash(SubtitlesInfo& pFileInfo)
{
    std::vector<BYTE> buffer(2 * PROBE_SIZE);
    if (pFileInfo.pAsyncReader) {
        UINT64 position = 0;
        pFileInfo.pAsyncReader->SyncRead(position, PROBE_SIZE, (BYTE*)&buffer[0]);
        position = std::max((UINT64)0, (UINT64)(pFileInfo.fileSize - PROBE_SIZE));
        pFileInfo.pAsyncReader->SyncRead(position, PROBE_SIZE, (BYTE*)&buffer[PROBE_SIZE]);
    } else {
        CFile file;
        CFileException fileException;
        if (file.Open(CString(pFileInfo.filePathW.c_str()),
                      CFile::modeRead | CFile::osSequentialScan | CFile::shareDenyNone | CFile::typeBinary,
                      &fileException)) {
            file.Read(&buffer[0], PROBE_SIZE);
            file.Seek(std::max((UINT64)0, (UINT64)(pFileInfo.fileSize - PROBE_SIZE)), CFile::begin);
            file.Read(&buffer[PROBE_SIZE], PROBE_SIZE);
        }
    }
    pFileInfo.fileHash = StringToHash(std::string((char*)&buffer[0], buffer.size()), CALG_MD5);
    LOG(LOG_OUTPUT, pFileInfo.fileHash.c_str());
    return SR_SUCCEEDED;
}

SRESULT SubDB::Search(const SubtitlesInfo& pFileInfo)
{
    if (!pFileInfo.manualSearchString.IsEmpty()) {
        return SR_FAILED; //SubDB does not support manual search
    }

    SRESULT searchResult = SR_UNDEFINED;
    std::string url(StringFormat("%s/?action=search&hash=%s", Url().c_str(), pFileInfo.fileHash.c_str()));
    LOG(LOG_INPUT, url.c_str());

    std::string data;
    searchResult = DownloadInternal(url, "", data);

    if (!data.empty()) {
        for (const auto& iter : StringTokenize(data, ",")) {
            CheckAbortAndReturn();
            if (CheckLanguage(iter)) {
                SubtitlesInfo pSubtitlesInfo;
                pSubtitlesInfo.id = pFileInfo.fileHash;
                pSubtitlesInfo.fileExtension = "srt";
                pSubtitlesInfo.fileName = pFileInfo.fileName + GUESSED_NAME_POSTFIX;
                pSubtitlesInfo.languageCode = iter;
                pSubtitlesInfo.languageName = UTF16To8(ISOLang::ISO639XToLanguage(iter.c_str()));
                pSubtitlesInfo.discNumber = 1;
                pSubtitlesInfo.discCount = 1;
                pSubtitlesInfo.title = pFileInfo.title;
                Set(pSubtitlesInfo);
            }
        }
    }

    return searchResult;
}

SRESULT SubDB::Download(SubtitlesInfo& pSubtitlesInfo)
{
    std::string url(StringFormat("%s/?action=download&hash=%s&language=%s", Url().c_str(), pSubtitlesInfo.id.c_str(), pSubtitlesInfo.languageCode.c_str()));
    LOG(LOG_INPUT, url.c_str());
    return DownloadInternal(url, "", pSubtitlesInfo.fileContents);
}

SRESULT SubDB::Upload(const SubtitlesInfo& pSubtitlesInfo)
{
#define MULTIPART_BOUNDARY "xYzZY"
    std::string url(StringFormat("%s/?action=upload&hash=%s", Url().c_str(), pSubtitlesInfo.fileHash.c_str()));
    stringMap headers({
        { "User-Agent", UserAgent() },
        { "Content-Type", "multipart/form-data; boundary=" MULTIPART_BOUNDARY },
    });

    std::string content, data;
    content += StringFormat("--%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n", MULTIPART_BOUNDARY, "hash", pSubtitlesInfo.fileHash.c_str());
    content += StringFormat("--%s\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s.%s\"\r\nContent-Type: application/octet-stream\r\nContent-Transfer-Encoding: binary\r\n\r\n",
                            MULTIPART_BOUNDARY, "file", pSubtitlesInfo.fileHash.c_str(), "srt");
    content += pSubtitlesInfo.fileContents;
    content += StringFormat("\r\n--%s--\r\n", MULTIPART_BOUNDARY);

    CheckAbortAndReturn();

    DWORD dwStatusCode = NULL;
    StringUpload(url, headers, content, data, FALSE, &dwStatusCode);

    LOG(LOG_BOTH, url.c_str(), std::to_string(dwStatusCode).c_str());

    switch (dwStatusCode) {
        case 201:
            return SR_SUCCEEDED;  //'Uploaded':   (HTTP/1.1 201 Created): If everything was OK, the HTTP status code 201 will be returned.
        case 403:
            return SR_EXISTS;     //'Duplicated': (HTTP/1.1 403 Forbidden): If the subtitle file already exists in our database, the HTTP status code 403 will be returned.
        case 400:
            return SR_FAILED;     //'Malformed':  (HTTP/1.1 400 Bad Request): If the request was malformed, the HTTP status code 400 will be returned.
        case 415:
            return SR_FAILED;     //'Invalid':    (HTTP/1.1 415 Unsupported Media Type): If the subtitle file is not supported by our database, the HTTP status code 415 will be returned.
        default:
            return SR_UNDEFINED;
    }
}

const std::set<std::string>& SubDB::Languages() const
{
    static std::once_flag initialized;
    static std::set<std::string> result;
#if 1
    result = {"en","es","fr","it","nl","pl","pt","ro","sv","tr"};
#else
    try {
        std::call_once(initialized, [this]() {
            if (!CheckInternetConnection()) {
                throw LanguageDownloadException("No internet connection.");
            }
            std::string data;
            if (DownloadInternal(Url() + "/?action=languages", "", data) != SR_SUCCEEDED) {
                throw LanguageDownloadException("Failed to download language list.");
            }
            for (const auto& str : StringTokenize(data, ",")) {
                result.emplace(str);
            }
        });
    } catch (const LanguageDownloadException& e) {
        UNREFERENCED_PARAMETER(e);
        LOG(LOG_ERROR, e.what());
    }
#endif
    return result;
}

/******************************************************************************
** podnapisi
******************************************************************************/

SRESULT podnapisi::Login(const std::string& sUserName, const std::string& sPassword)
{
    //TODO: implement
    return SR_UNDEFINED;
}

/*
UPDATED
https://www.podnapisi.net/forum/viewtopic.php?f=62&t=26164#p212652
RESULTS ------------------------------------------------
"/sXML/1/"  //Reply in XML format
"/page//"   //Return nth page of results
SEARCH -------------------------------------------------
"/sT/1/"    //Type: -1=all, 0=movies, 1=series, don't specify for auto detection
"/sAKA/1/"  //Include movie title aliases
"/sM//"     //Movie id from www.omdb.si
"/sK//"     //Title url encoded text
"/sY//"     //Year number
"/sTS//"    //Season number
"/sTE//"    //Episode number
"/sR//"     //Release name url encoded text
"/sJ/0/"    //Languages (old integer IDs), comma delimited, 0=all
"/sL/en/"   //Languages in ISO ISO codes (exception are sr-latn and pt-br), comma delimited
"/sEH//"    //Exact hash match (OSH)
"/sMH//"    //Movie hash (OSH)
SEARCH ADDITIONAL --------------------------------------
"/sFT/0/"   //Subtitles Format: 0=all, 1=MicroDVD, 2=SAMI, 3=SSA, 4=SubRip, 5=SubViewer 2.0, 6=SubViewer, 7=MPSub, 8=Advanced SSA, 9=DVDSubtitle, 10=TMPlayer, 11=MPlayer2
"/sA/0/"    //Search subtitles by user id, 0=all
"/sI//"     //Search subtitles by subtitle id
SORTING ------------------------------------------------
"/sS//"     //Sorting field: movie, year, fps, language, downloads, cds, username, time, rating
"/sO//"     //Soring order: asc, desc
FILTERS ------------------------------------------------
"/sOE/1/"   //Subtitles for extended edition only
"/sOD/1/"   //Subtitles suitable for DVD only
"/sOH/1/"   //Subtitles for high-definition video only
"/sOI/1/"   //Subtitles for hearing impaired only
"/sOT/1/"   //Technically correct only
"/sOL/1/"   //Grammatically correct only
"/sOA/1/"   //Author subtitles only
"/sOCS/1/"  //Only subtitles for a complete season
UNKNOWN ------------------------------------------------
"/sH//"     //Search subtitles by video file hash ??? (not working for me)
*/

SRESULT podnapisi::Search(const SubtitlesInfo& pFileInfo)
{
    SRESULT searchResult = SR_UNDEFINED;
    int page = 1, pages = 1, results = 0;
    do {
        CheckAbortAndReturn();

        std::string url(Url() + "/ppodnapisi/search");
        url += "?sXML=1";
        url += "&sAKA=1";

        if (pFileInfo.manualSearchString.IsEmpty()) {
            std::string search(pFileInfo.title);
            if (!pFileInfo.country.empty()) {
                search += " " + pFileInfo.country;
            }
            search = std::regex_replace(search, std::regex(" and | *[!?&':] *", RegexFlags), " ");

            if (!search.empty()) {
                url += "&sK=" + UrlEncode(search.c_str());
            }
            url += (pFileInfo.year != -1 ? "&sY=" + std::to_string(pFileInfo.year) : "");
            url += (pFileInfo.seasonNumber != -1 ? "&sTS=" + std::to_string(pFileInfo.seasonNumber) : "");
            url += (pFileInfo.episodeNumber != -1 ? "&sTE=" + std::to_string(pFileInfo.episodeNumber) : "");
            url += "&sMH=" + pFileInfo.fileHash;
            //url += "&sR=" + UrlEncode(pFileInfo.fileName.c_str());
        } else {
            CT2CA pszConvertedAnsiString(pFileInfo.manualSearchString);
            std::string search(pszConvertedAnsiString);
            search = std::regex_replace(search, std::regex(" and | *[!?&':] *", RegexFlags), " ");

            if (!search.empty()) {
                url += "&sK=" + UrlEncode(search.c_str());
            }
        }
        const auto languages = LanguagesISO6391();
        url += (!languages.empty() ? "&sL=" + JoinContainer(languages, ",") : "");
        url += "&page=" + std::to_string(page);
        LOG(LOG_INPUT, url.c_str());

        std::string data;
        searchResult = DownloadInternal(url, "", data);

        using namespace tinyxml2;

        tinyxml2::XMLDocument dxml;
        if (dxml.Parse(data.c_str()) == XML_SUCCESS) {

            auto GetChildElementText = [&](XMLElement * pElement, const char* value) -> std::string {
                std::string str;
                XMLElement* pChildElement = pElement->FirstChildElement(value);
                if (pChildElement != nullptr)
                {
                    auto pText = pChildElement->GetText();
                    if (pText != nullptr) {
                        str = pText;
                    }
                }
                return str;
            };

            XMLElement* pRootElmt = dxml.FirstChildElement("results");
            if (pRootElmt) {
                XMLElement* pPaginationElmt = pRootElmt->FirstChildElement("pagination");
                if (pPaginationElmt) {
                    page = atoi(GetChildElementText(pPaginationElmt, "current").c_str());
                    pages = atoi(GetChildElementText(pPaginationElmt, "count").c_str());
                    results = atoi(GetChildElementText(pPaginationElmt, "results").c_str());
                }
                // 30 results per page
                if (page > 1) {
                    return SR_TOOMANY;
                }

                if (results > 0) {
                    XMLElement* pSubtitleElmt = pRootElmt->FirstChildElement("subtitle");

                    while (pSubtitleElmt) {
                        CheckAbortAndReturn();

                        SubtitlesInfo pSubtitlesInfo;

                        pSubtitlesInfo.id = GetChildElementText(pSubtitleElmt, "pid");
                        pSubtitlesInfo.title = HtmlSpecialCharsDecode(GetChildElementText(pSubtitleElmt, "title").c_str());

                        std::string year = GetChildElementText(pSubtitleElmt, "year");
                        pSubtitlesInfo.year = year.empty() ? -1 : atoi(year.c_str());

                        pSubtitlesInfo.url = GetChildElementText(pSubtitleElmt, "url");
                        std::string format = GetChildElementText(pSubtitleElmt, "format");
                        pSubtitlesInfo.fileExtension = (format == "SubRip" || format == "N/A") ? "srt" : format;

                        pSubtitlesInfo.languageCode = podnapisi_languages[atoi(GetChildElementText(pSubtitleElmt, "languageId").c_str())].code;
                        pSubtitlesInfo.languageName = GetChildElementText(pSubtitleElmt, "languageName");
                        pSubtitlesInfo.seasonNumber = atoi(GetChildElementText(pSubtitleElmt, "tvSeason").c_str());
                        pSubtitlesInfo.episodeNumber = atoi(GetChildElementText(pSubtitleElmt, "tvEpisode").c_str());
                        pSubtitlesInfo.discCount = atoi(GetChildElementText(pSubtitleElmt, "cds").c_str());
                        pSubtitlesInfo.discNumber = pSubtitlesInfo.discCount;

                        std::string flags = GetChildElementText(pSubtitleElmt, "flags");
                        pSubtitlesInfo.hearingImpaired = (flags.find("n") != std::string::npos) ? TRUE : FALSE;
                        pSubtitlesInfo.corrected = (flags.find("r") != std::string::npos) ? -1 : 0;
                        pSubtitlesInfo.downloadCount = atoi(GetChildElementText(pSubtitleElmt, "downloads").c_str());
                        pSubtitlesInfo.imdbid = GetChildElementText(pSubtitleElmt, "movieId");
                        pSubtitlesInfo.frameRate = atof(GetChildElementText(pSubtitleElmt, "fps").c_str());

                        XMLElement* pReleasesElem = pSubtitleElmt->FirstChildElement("releases");
                        if (pReleasesElem) {
                            XMLElement* pReleaseElem = pReleasesElem->FirstChildElement("release");

                            while (pReleaseElem) {
                                auto pText = pReleaseElem->GetText();

                                if (!pText) {
                                    continue;
                                }

                                pSubtitlesInfo.releaseNames.emplace_back(pText);

                                if (pSubtitlesInfo.fileName.empty() || pFileInfo.fileName.find(pText) != std::string::npos) {
                                    pSubtitlesInfo.fileName = pText;
                                    pSubtitlesInfo.fileName += "." + pSubtitlesInfo.fileExtension;
                                }
                                pReleaseElem = pReleaseElem->NextSiblingElement();
                            }
                        }

                        if (pSubtitlesInfo.fileName.empty()) {
                            std::string str = pSubtitlesInfo.title;
                            if (!year.empty()) {
                                str += " " + year;
                            }
                            if (pSubtitlesInfo.seasonNumber > 0) {
                                str += StringFormat(" S%02d", pSubtitlesInfo.seasonNumber);
                            }
                            if (pSubtitlesInfo.episodeNumber > 0) {
                                str += StringFormat("%sE%02d", (pSubtitlesInfo.seasonNumber > 0) ? "" : " ", pSubtitlesInfo.episodeNumber);
                            }
                            str += GUESSED_NAME_POSTFIX;
                            pSubtitlesInfo.fileName = str;
                        }

                        Set(pSubtitlesInfo);
                        pSubtitleElmt = pSubtitleElmt->NextSiblingElement();
                    }
                }
            }
        }
    } while (page++ < pages);

    return searchResult;
}

SRESULT podnapisi::Hash(SubtitlesInfo& pFileInfo)
{
    pFileInfo.fileHash = StringFormat("%016I64x", GenerateOSHash(pFileInfo));
    LOG(LOG_OUTPUT, pFileInfo.fileHash.c_str());
    return SR_SUCCEEDED;
}

SRESULT podnapisi::Download(SubtitlesInfo& pSubtitlesInfo)
{
    std::string url = StringFormat("%s/subtitles/%s/download", Url().c_str(), pSubtitlesInfo.id.c_str());
    LOG(LOG_INPUT, url.c_str());
    return DownloadInternal(url, "", pSubtitlesInfo.fileContents);
}

const std::set<std::string>& podnapisi::Languages() const
{
    static std::once_flag initialized;
    static std::set<std::string> result;

    std::call_once(initialized, [this]() {
        for (const auto& iter : podnapisi_languages) {
            if (strlen(iter.code)) {
                result.emplace(iter.code);
            }
        }
    });
    return result;
}

#ifdef MPCHC_DISABLED_SUBTITLES_PROVIDER
/******************************************************************************
** titlovi
******************************************************************************/

/*
 x-dev_api_id=
 uiculture=hr,rs,si,ba,en,mk
 language=hr,rs,sr,si,ba,en,mk
 keyword=
 year=
 mt=numeric value representing type of subtitle (Movie / TV show / documentary 1, 2, 3)
 season=numeric value representing season
 episode=numeric value representing season episode
 forcefilename=true (default is false) return direct download link
*/

SRESULT titlovi::Search(const SubtitlesInfo& pFileInfo)
{
    // Need to filter not supported languages, because their API returns .hr language otherwise.
    auto selectedLanguages = LanguagesISO6391();
    bool userSelectedLanguage = !selectedLanguages.empty();
    const auto languagesIntersection = GetLanguagesIntersection(std::move(selectedLanguages));
    std::string KEY = "WC1ERVYtREVTS1RPUF9maWUyYS1hMVJzYS1hSHc0UA==";
    std::string url(StringFormat("http://api.titlovi.com/xml_get_api.ashx?x-dev_api_id=%s&uiculture=en&forcefilename=true", Base64::decode(KEY).c_str()));
    url += "&mt=" + (pFileInfo.seasonNumber != -1 ? std::to_string(2) : std::to_string(1));
    url += "&keyword=" + UrlEncode(pFileInfo.title.c_str());
    url += (pFileInfo.seasonNumber != -1 ? "&season=" + std::to_string(pFileInfo.seasonNumber) : "");
    url += (pFileInfo.episodeNumber != -1 ? "&episode=" + std::to_string(pFileInfo.episodeNumber) : "");
    url += (pFileInfo.year != -1 ? "&year=" + std::to_string(pFileInfo.year) : "");
    url += (userSelectedLanguage ? "&language=" + JoinContainer(languagesIntersection, ",") : "");
    LOG(LOG_INPUT, url.c_str());

    std::string data;
    SRESULT searchResult = DownloadInternal(url, "", data);

    tinyxml2::XMLDocument dxml;
    if (dxml.Parse(data.c_str()) == tinyxml2::XMLError::XML_SUCCESS) {

        auto GetChildElementText = [&](tinyxml2::XMLElement * pElement, const char* value) -> std::string {
            std::string str;
            auto pChildElement = pElement->FirstChildElement(value);

            if (pChildElement != nullptr)
            {
                auto pText = pChildElement->GetText();
                if (pText != nullptr) {
                    str = pText;
                }
            }
            return str;
        };

        auto pRootElmt = dxml.FirstChildElement("subtitles");

        if (pRootElmt) {
            int num = pRootElmt->IntAttribute("resultsCount");

            if (num > 0/* && num < 50*/) {
                auto pSubtitleElmt = pRootElmt->FirstChildElement();

                while (pSubtitleElmt) {
                    SubtitlesInfo pSubtitlesInfo;

                    pSubtitlesInfo.title = GetChildElementText(pSubtitleElmt, "title");
                    pSubtitlesInfo.languageCode = GetChildElementText(pSubtitleElmt, "language");

                    for (const auto& language : titlovi_languages) {
                        if (pSubtitlesInfo.languageCode == language.code) {
                            pSubtitlesInfo.languageCode = language.name;
                        }
                    }

                    pSubtitlesInfo.languageName = UTF16To8(ISOLang::ISO639XToLanguage(pSubtitlesInfo.languageCode.c_str()));
                    auto releaseNames = StringTokenize(GetChildElementText(pSubtitleElmt, "release"), "/");
                    pSubtitlesInfo.releaseNames = { releaseNames.begin(), releaseNames.end() };
                    pSubtitlesInfo.imdbid = GetChildElementText(pSubtitleElmt, "imdbId");
                    pSubtitlesInfo.frameRate = atof(GetChildElementText(pSubtitleElmt, "fps").c_str());
                    pSubtitlesInfo.year = atoi(GetChildElementText(pSubtitleElmt, "year").c_str());
                    pSubtitlesInfo.discNumber = atoi(GetChildElementText(pSubtitleElmt, "cd").c_str());
                    pSubtitlesInfo.discCount = pSubtitlesInfo.discNumber;
                    pSubtitlesInfo.downloadCount = atoi(GetChildElementText(pSubtitleElmt, "downloads").c_str());

                    auto pSubtitleChildElmt = pSubtitleElmt->FirstChildElement("urls");
                    if (pSubtitleChildElmt) {
                        auto pURLElement = pSubtitleChildElmt->FirstChildElement("url");
                        while (pURLElement) {
                            if (pURLElement->Attribute("what", "download")) {
                                pSubtitlesInfo.url = pURLElement->GetText();
                            }
                            if (pURLElement->Attribute("what", "direct")) {
                                pSubtitlesInfo.id = pURLElement->GetText();
                            }
                            pURLElement = pURLElement->NextSiblingElement();
                        }
                    }

                    if ((pSubtitleChildElmt = pSubtitleElmt->FirstChildElement("TVShow")) != nullptr) {
                        pSubtitlesInfo.seasonNumber = atoi(GetChildElementText(pSubtitleChildElmt, "season").c_str());
                        pSubtitlesInfo.episodeNumber = atoi(GetChildElementText(pSubtitleChildElmt, "episode").c_str());
                    }

                    pSubtitlesInfo.fileName = pSubtitlesInfo.title + " " + std::to_string(pSubtitlesInfo.year);
                    if (pSubtitlesInfo.seasonNumber > 0) {
                        pSubtitlesInfo.fileName += StringFormat(" S%02d", pSubtitlesInfo.seasonNumber);
                    }
                    if (pSubtitlesInfo.episodeNumber > 0) {
                        pSubtitlesInfo.fileName += StringFormat("%sE%02d", (pSubtitlesInfo.seasonNumber > 0) ? "" : " ", pSubtitlesInfo.episodeNumber);
                    }

                    bool found = false;
                    for (const auto& str : pSubtitlesInfo.releaseNames) {
                        if (pFileInfo.fileName.find(str) != std::string::npos) {
                            pSubtitlesInfo.fileName += " " + str;
                            found = true;
                            break;
                        }
                    }

                    if (!found && !pSubtitlesInfo.releaseNames.empty()) {
                        pSubtitlesInfo.fileName += " " + pSubtitlesInfo.releaseNames.front();
                    }
                    pSubtitlesInfo.fileName += GUESSED_NAME_POSTFIX;

                    Set(pSubtitlesInfo);
                    pSubtitleElmt = pSubtitleElmt->NextSiblingElement();
                }
            }
        }
    }
    return searchResult;
}

SRESULT titlovi::Download(SubtitlesInfo& pSubtitlesInfo)
{
    LOG(LOG_INPUT, pSubtitlesInfo.id.c_str());
    return DownloadInternal(pSubtitlesInfo.id, "", pSubtitlesInfo.fileContents);
}

const std::set<std::string>& titlovi::Languages() const
{
    static std::once_flag initialized;
    static std::set<std::string> result;

    std::call_once(initialized, [this]() {
        for (const auto& iter : titlovi_languages) {
            if (strlen(iter.name)) {
                result.emplace(iter.name);
            }
        }
    });
    return result;
}

/******************************************************************************
** ysubs
******************************************************************************/

SRESULT ysubs::Search(const SubtitlesInfo& pFileInfo)
{
    SRESULT searchResult = SR_UNDEFINED;
    using namespace rapidjson;

    if (pFileInfo.year && pFileInfo.seasonNumber == -1 && pFileInfo.episodeNumber == -1) {
        std::string urlApi(StringFormat("https://yts.ag/api/v2/list_movies.json?query_term=%s", UrlEncode(pFileInfo.title.c_str())));
        LOG(LOG_INPUT, urlApi.c_str());

        std::string data;
        searchResult = DownloadInternal(urlApi, "", data);

        Document d;
        if (d.ParseInsitu(&data[0]).HasParseError()) {
            return SR_FAILED;
        }

        auto root = d.FindMember("data");
        if (root != d.MemberEnd()) {
            auto iter = root->value.FindMember("movies");
            if ((iter != root->value.MemberEnd()) && (iter->value.IsArray())) {
                std::set<std::string> imdb_ids;
                for (auto elem = iter->value.Begin(); elem != iter->value.End(); ++elem) {
                    std::string imdb = elem->FindMember("imdb_code")->value.GetString();
                    if (imdb_ids.find(imdb) == imdb_ids.end()) {
                        imdb_ids.insert(imdb);

                        std::string urlSubs(StringFormat("http://api.ysubs.com/subs/%s", imdb.c_str()));
                        LOG(LOG_INPUT, urlSubs.c_str());

                        std::string data1;
                        searchResult = DownloadInternal(urlSubs, "", data1);
                        Document d1;
                        if (d1.ParseInsitu(&data1[0]).HasParseError()) {
                            return SR_FAILED;
                        }

                        auto iter1 = d1.FindMember("subs");
                        if (iter1 != d1.MemberEnd()) {
                            iter1 = iter1->value.FindMember(imdb.c_str());
                            if (iter1 != d1.MemberEnd()) {
                                for (auto elem1 = iter1->value.MemberBegin(); elem1 != iter1->value.MemberEnd(); ++elem1) {
                                    std::string lang = elem1->name.GetString();
                                    std::string lang_code;
                                    for (const auto& language : ysubs_languages) {
                                        if (lang == language.name) {
                                            lang_code = language.code;
                                        }
                                    }
                                    if (CheckLanguage(lang_code)) {
                                        for (auto elem2 = elem1->value.Begin(); elem2 != elem1->value.End(); ++elem2) {
                                            SubtitlesInfo pSubtitlesInfo;

                                            pSubtitlesInfo.title = elem->FindMember("title")->value.GetString();
                                            pSubtitlesInfo.languageCode = lang_code;
                                            pSubtitlesInfo.languageName = UTF16To8(ISOLang::ISO639XToLanguage(pSubtitlesInfo.languageCode.c_str()));
                                            pSubtitlesInfo.releaseNames.emplace_back("YIFY");
                                            pSubtitlesInfo.imdbid = imdb;
                                            pSubtitlesInfo.year = elem->FindMember("year")->value.GetInt();
                                            pSubtitlesInfo.discNumber = 1;
                                            pSubtitlesInfo.discCount = 1;

                                            pSubtitlesInfo.url = "http://www.yifysubtitles.com/movie-imdb/" + imdb;
                                            std::string str = elem2->FindMember("url")->value.GetString();
                                            pSubtitlesInfo.id = "http://www.yifysubtitles.com" + str;
                                            pSubtitlesInfo.hearingImpaired = elem2->FindMember("hi")->value.GetInt();
                                            pSubtitlesInfo.corrected = elem2->FindMember("rating")->value.GetInt();

                                            pSubtitlesInfo.fileName = pFileInfo.fileName;
                                            pSubtitlesInfo.fileName += GUESSED_NAME_POSTFIX;

                                            Set(pSubtitlesInfo);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return searchResult;
}

SRESULT ysubs::Download(SubtitlesInfo& pSubtitlesInfo)
{
    LOG(LOG_INPUT, pSubtitlesInfo.id.c_str());
    return DownloadInternal(pSubtitlesInfo.id, "", pSubtitlesInfo.fileContents);
}

const std::set<std::string>& ysubs::Languages() const
{
    static std::once_flag initialized;
    static std::set<std::string> result;

    std::call_once(initialized, [this]() {
        for (const auto& iter : ysubs_languages) {
            if (strlen(iter.code)) {
                result.emplace(iter.code);
            }
        }
    });
    return result;
}
#endif // MPCHC_DISABLED_SUBTITLES_PROVIDER

/******************************************************************************
** Napisy24
******************************************************************************/

SRESULT Napisy24::Search(const SubtitlesInfo& pFileInfo)
{
    if (!pFileInfo.manualSearchString.IsEmpty()) {
        return SR_FAILED; //napisys24 does not support manual search
    }
    stringMap headers({
        { "User-Agent", UserAgent() },
        { "Content-Type", "application/x-www-form-urlencoded" }
    });
    std::string data;
    std::string url = Url() + "/run/CheckSubAgent.php";
    std::string content = "postAction=CheckSub";
    content += "&ua=mpc-hc";
    content += "&ap=mpc-hc";
    content += "&fh=" + pFileInfo.fileHash;
    content += "&fs=" + std::to_string(pFileInfo.fileSize);
    content += "&fn=" + pFileInfo.fileName;

    LOG(LOG_INPUT, std::string(url + "?" + content).c_str());
    StringUpload(url, headers, content, data);

    if (data.length() < 4) {
        return SR_FAILED;
    }

    // Get status
    std::string status = data.substr(0, 4);
    if (status != "OK-2" && status != "OK-3") {
        return SR_FAILED;
    }
    data.erase(0, 5);

    size_t infoEnd = data.find("||");
    if (infoEnd == std::string::npos) {
        return SR_FAILED;
    }

    // Search already returns whole file
    SubtitlesInfo subtitleInfo;
    subtitleInfo.fileContents = data.substr(infoEnd + 2);
    subtitleInfo.languageCode = "pl"; // API doesn't support other languages yet.

    // Remove subtitle data
    data.erase(infoEnd);

    std::unordered_map<std::string, std::string> subtitleInfoMap;
    std::istringstream stringStream(data);
    std::string entry;
    while (std::getline(stringStream, entry, '|')) {
        auto delimPos = entry.find(':');
        if (delimPos == std::string::npos) {
            continue;
        }
        std::string key = entry.substr(0, delimPos);
        if (entry.length() <= delimPos + 1) {
            continue;
        }
        std::string value = entry.substr(delimPos + 1);
        subtitleInfoMap[key] = value;
    }

    subtitleInfo.url = Url() + "/komentarze?napisId=" + subtitleInfoMap["napisId"];
    subtitleInfo.title = subtitleInfoMap["ftitle"];
    subtitleInfo.imdbid = subtitleInfoMap["fimdb"];

    auto it = subtitleInfoMap.find("fyear");
    if (it != subtitleInfoMap.end()) {
        subtitleInfo.year = std::stoi(it->second);
    }

    it = subtitleInfoMap.find("fps");
    if (it != subtitleInfoMap.end()) {
        subtitleInfo.frameRate = std::stod(it->second);
    }

    int hour, minute, second;
    if (sscanf_s(subtitleInfoMap["time"].c_str(), "%02d:%02d:%02d", &hour, &minute, &second) == 3) {
        subtitleInfo.lengthMs = ((hour * 60 + minute) * 60 + second) * 1000;
    }

    subtitleInfo.fileName = pFileInfo.fileName + "." + pFileInfo.fileExtension;
    subtitleInfo.discNumber = 1;
    subtitleInfo.discCount = 1;

    Set(subtitleInfo);

    return SR_SUCCEEDED;
}

SRESULT Napisy24::Hash(SubtitlesInfo& pFileInfo)
{
    pFileInfo.fileHash = StringFormat("%016I64x", GenerateOSHash(pFileInfo));
    LOG(LOG_OUTPUT, pFileInfo.fileHash.c_str());
    return SR_SUCCEEDED;
}

SRESULT Napisy24::Download(SubtitlesInfo& subtitlesInfo)
{
    LOG(LOG_INPUT, subtitlesInfo.url.c_str());
    return subtitlesInfo.fileContents.empty() ? SR_FAILED : SR_SUCCEEDED;
}

const std::set<std::string>& Napisy24::Languages() const
{
    static std::set<std::string> result = {"pl"};
    return result;
}
