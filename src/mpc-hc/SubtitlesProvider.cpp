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

#include "stdafx.h"
#include "SubtitlesProvider.h"
#include "MediaInfo/ThirdParty/tinyxml2/tinyxml2.h"
using namespace tinyxml2;

#include "rapidjson/include/rapidjson/rapidjson.h"
#include "rapidjson/include/rapidjson/document.h"

/******************************************************************************
** Register providers
******************************************************************************/
void SubtitlesProviders::RegisterProviders()
{
    Register<OpenSubtitles>();
    Register<podnapisi>();
    Register<addic7ed>();
    Register<titlovi>();
    Register<SubDB>();
    Register<ysubs>();
    Register<TVsubtitles>();
    Register<Moviesubtitles>();
}

#define CheckAbortAndReturn() { if (IsAborting()) return SR_ABORTED; }

/******************************************************************************
** OpenSubtitles
******************************************************************************/

void OpenSubtitles::Initialize()
{
    xmlrpc = DEBUG_NEW XmlRpcClient("http://api.opensubtitles.org/xml-rpc");
    if (xmlrpc) { xmlrpc->setIgnoreCertificateAuthority(); }
}

void OpenSubtitles::Uninitialize()
{
    if (xmlrpc != nullptr) { delete xmlrpc; }
}

SRESULT OpenSubtitles::Login(std::string& sUserName, std::string& sPassword)
{
    if (xmlrpc) {
        XmlRpcValue args, result;
        args[0] = sUserName;
        args[1] = sPassword;
        args[2] = "en";
        args[3] = UserAgent().c_str(); // Test with "OSTestUserAgent"
        if (!xmlrpc->execute("LogIn", args, result)) { return SR_FAILED; }
        token = result["token"];
    }
    return token.valid() ? SR_SUCCEEDED : SR_FAILED;
}

SRESULT OpenSubtitles::Hash(SubtitlesInfo& pFileInfo)
{
    UINT64 fileHash = pFileInfo.fileSize;
    if (pFileInfo.pAsyncReader) {
        UINT64 position = 0;
        for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && SUCCEEDED(pFileInfo.pAsyncReader->SyncRead(position, sizeof(tmp), (BYTE*)&tmp)); fileHash += tmp, position += sizeof(tmp), ++i);
        position = std::max((UINT64)0, (UINT64)(pFileInfo.fileSize - PROBE_SIZE));
        for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && SUCCEEDED(pFileInfo.pAsyncReader->SyncRead(position, sizeof(tmp), (BYTE*)&tmp)); fileHash += tmp, position += sizeof(tmp), ++i);
    } else {
        CFile file;
        CFileException fileException;
        if (file.Open(CString(pFileInfo.filePath.c_str()), CFile::modeRead | CFile::osSequentialScan | CFile::shareDenyNone | CFile::typeBinary, &fileException)) {
            for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && file.Read(&tmp, sizeof(tmp)); fileHash += tmp, ++i);
            file.Seek(std::max((UINT64)0, (UINT64)(pFileInfo.fileSize - PROBE_SIZE)), CFile::begin);
            for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && file.Read(&tmp, sizeof(tmp)); fileHash += tmp, ++i);
        }
    }
    pFileInfo.fileHash = string_format("%016I64x", fileHash);
    return SR_SUCCEEDED;
}

SRESULT OpenSubtitles::Search(const SubtitlesInfo& pFileInfo)
{
    std::string languages(LanguagesISO6392());
    XmlRpcValue args, result;
    args[0] = token;
    args[1][0]["sublanguageid"] = !languages.empty() ? languages : "all";
    args[1][0]["moviehash"] = pFileInfo.fileHash;
    args[1][0]["moviebytesize"] = (int)pFileInfo.fileSize;
    args[2]["limit"] = 500;

    if (!xmlrpc->execute("SearchSubtitles", args, result)) { return SR_FAILED; }

    if (result["data"].getType() != XmlRpcValue::Type::TypeArray) { return SR_FAILED; }

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
        regex_result results;
        string_regex("\"([^\"]+)\" (.+)", (const char*)data["MovieName"], results);
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
        pSubtitlesInfo.releaseName = (const char*)data["MovieReleaseName"];
        pSubtitlesInfo.imdbid = (const char*)data["IDMovieImdb"];
        pSubtitlesInfo.corrected = (int)data["SubBad"] ? -1 : 0;
        Set(pSubtitlesInfo);
    }
    return SR_SUCCEEDED;
}

SRESULT OpenSubtitles::Download(SubtitlesInfo& pSubtitlesInfo)
{
    XmlRpcValue args, result;
    args[0] = token;
    args[1][0] = pSubtitlesInfo.id;
    if (!xmlrpc->execute("DownloadSubtitles", args, result)) { return SR_FAILED; }

    if (result["data"].getType() != XmlRpcValue::Type::TypeArray) { return SR_FAILED; }

    pSubtitlesInfo.fileContents = Base64::decode(std::string(result["data"][0]["data"]));
    return SR_SUCCEEDED;
}

SRESULT OpenSubtitles::Upload(const SubtitlesInfo& pSubtitlesInfo)
{
    XmlRpcValue args, result;
    args[0] = token;

    //TODO: Ask  how to obtain commented values !!!
    args[1]["cd1"]["subhash"] = string_hash(pSubtitlesInfo.fileContents, CALG_MD5);
    args[1]["cd1"]["subfilename"] = pSubtitlesInfo.fileName + ".srt";
    args[1]["cd1"]["moviehash"] = pSubtitlesInfo.fileHash;
    args[1]["cd1"]["moviebytesize"] = (int)pSubtitlesInfo.fileSize;
    //args[1]["cd1"]["movietimems"];
    //args[1]["cd1"]["movieframes"];
    //args[1]["cd1"]["moviefps"];
    args[1]["cd1"]["moviefilename"] = pSubtitlesInfo.fileName + "." + pSubtitlesInfo.fileExtension;

    CheckAbortAndReturn();
    if (!xmlrpc->execute("TryUploadSubtitles", args, result)) { return SR_FAILED; }
    CheckAbortAndReturn();

    if ((int)result["alreadyindb"] == 1) {
        return SR_EXISTS;
    } else if ((int)result["alreadyindb"] == 0) {
        // We need imdbid to proceed
        if (result["data"].getType() == XmlRpcValue::Type::TypeArray) {
            args[1]["baseinfo"]["idmovieimdb"] = result["data"][0]["IDMovieImdb"];
        } else if (!pSubtitlesInfo.imdbid.empty()) {
            args[1]["baseinfo"]["idmovieimdb"] = pSubtitlesInfo.imdbid;
        } else {
            std::string title(string_replace(pSubtitlesInfo.title, "and", "&"));
            if (!args[1]["baseinfo"]["idmovieimdb"].valid()) {
                XmlRpcValue _args, _result;
                _args[0] = token;
                _args[1][0] = pSubtitlesInfo.fileHash;
                if (!xmlrpc->execute("CheckMovieHash", _args, _result)) { return SR_FAILED; }

                if (_result["data"].getType() == XmlRpcValue::Type::TypeStruct) {

                    //regex_results results;
                    //string_regex("\"(.+)\" (.+)", (const char*)data["MovieName"], results);
                    //if (!results.empty()) {
                    //    pSubtitlesInfo.title = results[0][0];
                    //    pSubtitlesInfo.title2 = results[0][1];
                    //} else {
                    //    pSubtitlesInfo.title = (const char*)data["MovieName"];
                    //}

                    regex_results results;
                    string_regex("\"(.+)\" .+|(.+)", string_replace((const char*)_result["data"][pSubtitlesInfo.fileHash]["MovieName"], "and", "&"), results);
                    std::string _title(results[0][0] + results[0][1]);

                    if (_strcmpi(title.c_str(), _title.c_str()) == 0 /*&& (pSubtitlesInfo.year == -1 || (pSubtitlesInfo.year != -1 && pSubtitlesInfo.year == atoi(_result["data"][pSubtitlesInfo.fileHash]["MovieYear"])))*/) {
                        args[1]["baseinfo"]["idmovieimdb"] = _result["data"][pSubtitlesInfo.fileHash]["MovieImdbID"]; //imdbid
                    }
                }
            }

            if (!args[1]["baseinfo"]["idmovieimdb"].valid()) {
                XmlRpcValue _args, _result;
                _args[0] = token;
                _args[1][0] = pSubtitlesInfo.fileHash;
                if (!xmlrpc->execute("CheckMovieHash2", _args, _result)) { return SR_FAILED; }

                if (_result["data"].getType() == XmlRpcValue::Type::TypeArray) {
                    int nCount = _result["data"][pSubtitlesInfo.fileHash].size();
                    for (int i = 0; i < nCount; ++i) {
                        regex_results results;
                        string_regex("\"(.+)\" .+|(.+)", string_replace((const char*)_result["data"][pSubtitlesInfo.fileHash][i]["MovieName"], "and", "&"), results);
                        std::string _title(results[0][0] + results[0][1]);

                        if (_strcmpi(title.c_str(), _title.c_str()) == 0 /*&& (pSubtitlesInfo.year == -1 || (pSubtitlesInfo.year != -1 && pSubtitlesInfo.year == atoi(_result["data"][pSubtitlesInfo.fileHash][i]["MovieYear"])))*/) {
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
                if (!xmlrpc->execute("SearchMoviesOnIMDB", _args, _result)) { return SR_FAILED; }
                if (_result["data"].getType() == XmlRpcValue::Type::TypeArray) {
                    int nCount = _result["data"].size();
                    for (int i = 0; i < nCount; ++i) {
                        regex_results results;
                        string_regex("(.+) [(](\\d{4})[)]", string_replace((const char*)_result["data"][i]["title"], "and", "&"), results);
                        if (results.size() == 1) {
                            std::string _title(results[0][0]);

                            if (_strcmpi(title.c_str(), _title.c_str()) == 0 /*&& (pSubtitlesInfo.year == -1 || (pSubtitlesInfo.year != -1 && pSubtitlesInfo.year == atoi(results[0][1].c_str())))*/) {
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
            if (!xmlrpc->execute("InsertMovieHash", _args, _result)) { return SR_FAILED; }
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

            args[1]["cd1"]["subcontent"] = Base64::encode(string_gzcompress(pSubtitlesInfo.fileContents));

            if (!xmlrpc->execute("UploadSubtitles", args, result)) { return SR_FAILED; }
            //#ifdef _DEBUG
            ShellExecute((HWND)AfxGetMyApp()->GetMainWnd(), _T("open"), UTF8To16(result["data"]), nullptr, nullptr, SW_SHOWDEFAULT);
            //#endif
            return SR_SUCCEEDED;
        }
    }
    return SR_FAILED;
}

std::string OpenSubtitles::Languages()
{
    static std::string data;
    if (data.empty() && CheckInternetConnection()) {
        XmlRpcValue args, result;
        args = "en";
        if (!xmlrpc->execute("GetSubLanguages", args, result)) { return data; }

        if (result["data"].getType() != XmlRpcValue::Type::TypeArray) { return data; }

        int count = result["data"].size();
        for (int i = 0; i < count; ++i) {
            if (i != 0) { data.append(","); }
            data.append(result["data"][i]["SubLanguageID"]);
        }
    }
    return data;
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
        if (file.Open(CString(pFileInfo.filePath.c_str()), CFile::modeRead | CFile::osSequentialScan | CFile::shareDenyNone | CFile::typeBinary, &fileException)) {
            file.Read(&buffer[0], PROBE_SIZE);
            file.Seek(std::max((UINT64)0, (UINT64)(pFileInfo.fileSize - PROBE_SIZE)), CFile::begin);
            file.Read(&buffer[PROBE_SIZE], PROBE_SIZE);
        }
    }
    pFileInfo.fileHash = string_hash(std::string((char*)&buffer[0], buffer.size()), CALG_MD5);
    return SR_SUCCEEDED;
}

SRESULT SubDB::Search(const SubtitlesInfo& pFileInfo)
{
    SRESULT searchResult = SR_UNDEFINED;
    std::string data;
    searchResult = Download(string_format("http://api.thesubdb.com/?action=search&hash=%s", pFileInfo.fileHash.c_str()), "", data);

    if (!data.empty()) {
        string_array result(string_tokenize(data, ","));
        for (const auto& iter : result) {
            CheckAbortAndReturn();
            if (CheckLanguage(iter)) {
                SubtitlesInfo pSubtitlesInfo;
                pSubtitlesInfo.id = pFileInfo.fileHash;
                pSubtitlesInfo.fileExtension = "srt";
                pSubtitlesInfo.fileName = pFileInfo.fileName + " (*)." + pSubtitlesInfo.fileExtension;
                pSubtitlesInfo.languageCode = iter;
                pSubtitlesInfo.languageName = UTF16To8(ISO639XToLanguage(iter.c_str()));
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
    return Download(string_format("http://api.thesubdb.com/?action=download&hash=%s&language=%s", pSubtitlesInfo.id.c_str(), pSubtitlesInfo.languageCode.c_str()), "", pSubtitlesInfo.fileContents);
}

SRESULT SubDB::Upload(const SubtitlesInfo& pSubtitlesInfo)
{
#define MULTIPART_BOUNDARY "xYzZY"
    std::string url(string_format("http://api.thesubdb.com/?action=upload&hash=%s", pSubtitlesInfo.fileHash.c_str()));
    string_map headers({
        { "User-Agent", UserAgent() },
        { "Content-Type", "multipart/form-data; boundary=" MULTIPART_BOUNDARY },
    });

    std::string content, data;
    content += string_format("--%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n", MULTIPART_BOUNDARY, "hash", pSubtitlesInfo.fileHash.c_str());
    content += string_format("--%s\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s.%s\"\r\nContent-Type: application/octet-stream\r\nContent-Transfer-Encoding: binary\r\n\r\n",
                             MULTIPART_BOUNDARY, "file", pSubtitlesInfo.fileHash.c_str(), "srt");
    content += pSubtitlesInfo.fileContents;
    content += string_format("\r\n--%s--\r\n", MULTIPART_BOUNDARY);

    CheckAbortAndReturn();

    DWORD dwStatusCode = NULL;
    string_upload(url, headers, content, data, FALSE, &dwStatusCode);

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

std::string SubDB::Languages()
{
    static std::string result;
    if (result.empty() && CheckInternetConnection()) {
        Download("http://api.thesubdb.com/?action=languages", "", result);
    }
    return result;
}

/******************************************************************************
** TVsubtitles
******************************************************************************/

const std::regex TVsubtitles::regex_pattern[] = {
    std::regex("href=\"/(tvshow-(\\d+)[.]html)\">(.*?) (?:[(]([a-z]{2})[)] )?[(](\\d{4})-(\\d{4})[)]</a>", regex_flags),
    std::regex("<a href=\"/subtitle-(\\d+)[.]html\">[^]+?<img src=\"images/flags/([a-z]{2})[.]gif\"[^]+?</div></a>", regex_flags),
    std::regex("filename:.+?>([^<\n\t]+)[^]+?downloads:.+?>(\\d+)", regex_flags),
};

SRESULT TVsubtitles::Search(const SubtitlesInfo& pFileInfo)
{
    SRESULT searchResult = SR_UNDEFINED;
    if (pFileInfo.seasonNumber != -1) {
        std::string data;
        searchResult = Download(string_format("http://www.tvsubtitles.net/search.php?q=%s", UrlEncode(pFileInfo.title.c_str())), "", data);

        regex_results results;
        string_regex(regex_pattern[0], data, results);
        for (const auto& iter : results) {
            CheckAbortAndReturn();
            if (_strcmpi(pFileInfo.title.c_str(), iter[2].c_str()) != 0) { continue; }
            std::string data1;
            searchResult = Download(string_format("http://www.tvsubtitles.net/tvshow-%s-%d.html", iter[1].c_str(), pFileInfo.seasonNumber), "", data1);

            regex_results results1;
            string_regex(std::regex(string_format("<td>%dx%0*d</td>[^<]*<td[^>]+><a href=\"(episode-(\\d+)[.]html)\"><b>(.*?)</b></a></td>[^<]*<td>([1-9]\\d*)</td>", pFileInfo.seasonNumber, 2, pFileInfo.episodeNumber), regex_flags), data1, results1);
            for (const auto& iter1 : results1) {
                CheckAbortAndReturn();
                std::string data2;
                searchResult = Download(string_format("http://www.tvsubtitles.net/episode-%s.html", iter1[1].c_str()), "", data2);

                regex_results results2;
                //string_regex(string_format("<a href=\"/subtitle-(\\d+)[.]html\">[^]+?<img src=\"images/flags/(%s)[.]gif\"", GetLanguagesString().c_str()), data2, results2);
                //too slow//string_regex(string_format("<a href=\"/subtitle-(\\d+)[.]html\">[^]+?<img src=\"images/flags/(%s)[.]gif\"[^]+?>([^<]+)[^]+?>\n\t([^<]+)[^]+?>\n\t([^<]+)[^]+?>\n\t(\\d+)</p></div></a>", GetLanguagesString().c_str()), data2, results2);
                //TODO: test
                //string_regex("<a href=\"/subtitle-(\\d+)[.]html\">[^]+?<img src=\"images/flags/([a-z]{2})[.]gif\"[^]+?>([^<]+)[^]+?>\n\t([^<]+)[^]+?>\n\t([^<]+)[^]+?>\n\t(\\d+)</p></div></a>", data2, results2);
                string_regex(regex_pattern[1], data2, results2);
                for (auto& iter2 : results2) {
                    CheckAbortAndReturn();
                    for (const auto& language : tvsubtitles_languages) { if (iter2[1] == language.code) { iter2[1] = language.name; } }
                    if (CheckLanguage(iter2[1])) {
                        std::string data3;
                        std::string url = string_format("http://www.tvsubtitles.net/subtitle-%s.html", iter2[0].c_str());
                        searchResult = Download(url, "", data3);

                        regex_results results3;
                        SubtitlesInfo pSubtitlesInfo;
                        string_regex(regex_pattern[2], data3, results3);
                        for (const auto& iter3 : results3) {
                            CheckAbortAndReturn();
                            pSubtitlesInfo.fileName = iter3[0];
                            pSubtitlesInfo.title = iter[2];
                            pSubtitlesInfo.country = iter[3];
                            pSubtitlesInfo.year = iter[4].empty() ? -1 : atoi(iter[4].c_str());
                            pSubtitlesInfo.title2 = iter1[2];
                            pSubtitlesInfo.seasonNumber = pFileInfo.seasonNumber;
                            pSubtitlesInfo.episodeNumber = pFileInfo.episodeNumber;
                            pSubtitlesInfo.id = iter2[0];
                            pSubtitlesInfo.url = url;
                            pSubtitlesInfo.languageCode = iter2[1];
                            pSubtitlesInfo.languageName = UTF16To8(ISO639XToLanguage(pSubtitlesInfo.languageCode.c_str()));
                            pSubtitlesInfo.downloadCount = atoi(iter3[1].c_str());
                            pSubtitlesInfo.discNumber = 1;
                            pSubtitlesInfo.discCount = 1;
                            Set(pSubtitlesInfo);
                        }
                    }
                }
            }
        }
    }
    return searchResult;
}

SRESULT TVsubtitles::Download(SubtitlesInfo& pSubtitlesInfo)
{
    return Download(string_format("http://www.tvsubtitles.net/download-%s.html", pSubtitlesInfo.id.c_str()), "", pSubtitlesInfo.fileContents);
}

std::string TVsubtitles::Languages()
{
    return "en,es,fr,de,br,ru,ua,it,gr,ar,hu,pl,tr,nl,pt,sv,da,fi,ko,cn,jp,bg,cz,ro";
}

/******************************************************************************
** Moviesubtitles
******************************************************************************/

const std::regex Moviesubtitles::regex_pattern[] = {
    std::regex("href=\"/(movie-(\\d+)[.]html)\">(.*?) [(](\\d{4})[)]</a>", regex_flags),
    std::regex("<img src=\"images/flags/([a-z]{2})[.]gif\"[^]+?<a href=\"/subtitle-(\\d+)[.]html\"[^]+?\"Rip\"><nobr>([^<]+?)</[^]+?\"release\">([^<]*?)<[^]+?\"downloaded\">(\\d+)[^]+?\"parts\">(\\d+)<", regex_flags),
    std::regex("filename:[^]+?\"filename\">([^<]+?)<", regex_flags),
};

SRESULT Moviesubtitles::Search(const SubtitlesInfo& pFileInfo)
{
    SRESULT searchResult = SR_UNDEFINED;
    if (pFileInfo.seasonNumber == -1) {
        std::string data;
        searchResult = Download(string_format("http://www.moviesubtitles.org/search.php?q=%s", UrlEncode(pFileInfo.title.c_str())), "", data);

        regex_results results;
        string_regex(regex_pattern[0], data, results);
        for (const auto& iter : results) {
            CheckAbortAndReturn();
            if (_strcmpi(pFileInfo.title.c_str(), iter[2].c_str()) != 0) { continue; }
            std::string data1;
            searchResult = Download(string_format("http://www.moviesubtitles.org/movie-%s.html", iter[1].c_str()), "", data1);

            regex_results results1;
            string_regex(regex_pattern[1], data1, results1);
            for (auto& iter1 : results1) {
                CheckAbortAndReturn();
                for (const auto& language : tvsubtitles_languages) { if (iter1[0] == language.code) { iter1[0] = language.name; } }
                if (CheckLanguage(iter1[0])) {
                    SubtitlesInfo pSubtitlesInfo;
                    std::string data2;
                    std::string url(string_format("http://www.moviesubtitles.org/subtitle-%s.html", iter1[1].c_str()));
                    searchResult = Download(url, "", data2);
                    regex_results results2;
                    string_regex(regex_pattern[2], data2, results2);
                    for (const auto& iter2 : results2) {
                        pSubtitlesInfo.fileName = iter2[0];
                    }
                    pSubtitlesInfo.title = iter[2];
                    pSubtitlesInfo.year = iter[3].empty() ? -1 : atoi(iter[3].c_str());
                    pSubtitlesInfo.seasonNumber = pFileInfo.seasonNumber;
                    pSubtitlesInfo.episodeNumber = pFileInfo.episodeNumber;
                    pSubtitlesInfo.id = iter1[1];
                    pSubtitlesInfo.languageCode = iter1[0];
                    pSubtitlesInfo.languageName = UTF16To8(ISO639XToLanguage(pSubtitlesInfo.languageCode.c_str()));
                    pSubtitlesInfo.downloadCount = atoi(iter1[4].c_str());
                    pSubtitlesInfo.discNumber = atoi(iter1[5].c_str());
                    pSubtitlesInfo.discCount = atoi(iter1[5].c_str());
                    pSubtitlesInfo.url = url;
                    Set(pSubtitlesInfo);
                }
            }
        }
    }
    return searchResult;
}

SRESULT Moviesubtitles::Download(SubtitlesInfo& pSubtitlesInfo)
{
    return Download(string_format("http://www.moviesubtitles.org/download-%s.html", pSubtitlesInfo.id.c_str()), "", pSubtitlesInfo.fileContents);
}

std::string Moviesubtitles::Languages()
{
    return "en,es,fr,de,br,ru,ua,it,gr,ar,hu,pl,tr,nl,pt,sv,da,fi,ko,cn,jp,bg,cz,ro";
}

/******************************************************************************
** addic7ed
******************************************************************************/

const std::regex addic7ed::regex_pattern[] = {
    std::regex("<a href=\"/show/(\\d+)\" >Show <i>(.*?)(?: [(](\\d{4})[)])?(?: [(]?(AU|CA|FR|JP|UK|US)[)]?)?</i></a>", regex_flags),
};

SRESULT addic7ed::Login(std::string& sUserName, std::string& sPassword)
{
    if (!sUserName.empty() && !sPassword.empty()) {
        std::string url("http://www.addic7ed.com/dologin.php");
        string_map headers({
            { "User-Agent", UserAgent() },
            { "Referer", "http://www.addic7ed.com/login.php" },
            { "Content-Type", "application/x-www-form-urlencoded" },
        });

        std::string content, data;
        content += string_format("username=%s&password=%s&Submit=Log+in",
                                 UrlEncode(sUserName.c_str()), UrlEncode(sPassword.c_str()));

        DWORD dwStatusCode = NULL;
        string_upload(url, headers, content, data, FALSE, &dwStatusCode);
        //'Success ':   (HTTP/1.1 302 Found): If everything was OK, the HTTP status code 302 will be returned.
        return (dwStatusCode == 302) ? SR_SUCCEEDED : SR_FAILED;

    }
    return SR_UNDEFINED;
}

SRESULT addic7ed::Search(const SubtitlesInfo& pFileInfo)
{
    SRESULT searchResult = SR_UNDEFINED;
    if (pFileInfo.seasonNumber != -1) {
        std::string data;
        std::string search(pFileInfo.title);
        if (!pFileInfo.country.empty()) { search += " " + pFileInfo.country; }
        if (pFileInfo.year != -1) { search += " (" + std::to_string(pFileInfo.year) + ")"; }
        // remove ' and ' from string and replace '!?&':' with ' ' to get more accurate results
        search = std::regex_replace(search, std::regex(" and |[!?&':]", regex_flags), " ");
        searchResult = Download(string_format("http://www.addic7ed.com/search.php?search=%s", UrlEncode(search .c_str())), "", data);

        regex_results results;
        string_regex(regex_pattern[0], data, results);
        for (const auto& iter : results) {
            CheckAbortAndReturn();
            std::string data1;
            searchResult = Download(string_format("http://www.addic7ed.com/ajax_loadShow.php?show=%s&season=%d&langs=%s&hd=undefined&hi=undefined", iter[0].c_str(), pFileInfo.seasonNumber, GetLanguagesString().c_str()), "", data1);

            regex_results results1;
            string_regex(std::regex(string_format("<tr class=\"epeven completed\"><td>%d</td><td>%d</td><td><a href=\"([^\"]+)\">(.*?)</a></td><td>(.*?)</td><td class=\"c\">(.*?)</td>[^<]*<td class=\"c\">Completed</td><td class=\"c\">(.*?)</td><td class=\"c\">(.*?)</td><td class=\"c\">(.*?)</td><td class=\"c\"><a href=\"(/updated/(\\d+)/\\d+/\\d+)\">Download</a></td>", pFileInfo.seasonNumber, pFileInfo.episodeNumber), regex_flags), data1, results1);
            for (const auto& iter1 : results1) {
                CheckAbortAndReturn();
                SubtitlesInfo pSubtitlesInfo;
                pSubtitlesInfo.fileName = string_format("%s - %dx%0*d - %s.%s.%s.%s%supdated.Addicted.com.srt", pFileInfo.title.c_str(), pFileInfo.seasonNumber, 2, pFileInfo.episodeNumber, HtmlSpecialCharsDecode(iter1[1].c_str()), iter1[3].empty() ? "Undefined" : iter1[3].c_str(), iter1[2].c_str(), (iter1[4].empty() ? "" : "HI."), (iter1[5].empty() ? "" : "C."));
                pSubtitlesInfo.fileExtension = "srt";
                pSubtitlesInfo.id = "http://www.addic7ed.com" + iter1[7];
                pSubtitlesInfo.url = "http://www.addic7ed.com" + iter1[0];
                pSubtitlesInfo.languageCode = addic7ed_languages[atoi(iter1[8].c_str())].code;
                pSubtitlesInfo.languageName = iter1[2];
                pSubtitlesInfo.hearingImpaired = iter1[4].empty() ? FALSE : TRUE;
                pSubtitlesInfo.corrected = (iter1[5].empty()) ? FALSE : TRUE;
                pSubtitlesInfo.seasonNumber = pFileInfo.seasonNumber;
                pSubtitlesInfo.episodeNumber = pFileInfo.episodeNumber;
                pSubtitlesInfo.title = iter[1];
                pSubtitlesInfo.title2 = HtmlSpecialCharsDecode(iter1[1].c_str());
                pSubtitlesInfo.year = iter[2].empty() ? -1 : atoi(iter[2].c_str());
                pSubtitlesInfo.country = iter[3];
                pSubtitlesInfo.discNumber = 1;
                pSubtitlesInfo.discCount = 1;
                Set(pSubtitlesInfo);
            }
        }
    }
    return searchResult;
}

SRESULT addic7ed::Download(SubtitlesInfo& pSubtitlesInfo)
{
    return Download(pSubtitlesInfo.id, pSubtitlesInfo.url, pSubtitlesInfo.fileContents);
}

std::string addic7ed::Languages()
{
    static std::string result;
    if (result.empty()) {
        for (const auto& iter : addic7ed_languages) {
            if (strlen(iter.code) && result.find(iter.code) == std::string::npos) {
                result += (result.empty() ? "" : ",");
                result += iter.code;
            }
        }
    }
    return result;
}

std::string addic7ed::GetLanguagesString()
{
    std::string result;
    std::string languages(LanguagesISO6391());
    if (!languages.empty()) {
        for (const auto& iter : addic7ed_languages) {
            if (strlen(iter.code) && languages.find(iter.code) != std::string::npos) {
                result += "|" + std::to_string(&iter - &addic7ed_languages[0]);
            }
        }
        if (!result.empty()) {
            result += "|";
        }
    }
    return result;
}

/******************************************************************************
** podnapisi
******************************************************************************/

const std::regex podnapisi::regex_pattern[] = {
    std::regex("<pagination>[^<]+<current>(\\d+)</current>[^<]+<count>(\\d+)</count>[^<]+<results>(\\d+)</results>[^<]+</pagination>", regex_flags),
    std::regex("<subtitle>[^<]+<id>(\\d*)</id>[^<]+<title>(.*?)</title>[^<]+<year>(\\d*)</year>[^<]+<movieId>(\\d*)</movieId>[^<]+<url>(.*?)</url>[^<]+<uploaderId>(\\d*)</uploaderId>[^<]+<uploaderName>(.*?)</uploaderName>[^<]+<release>(.*?)</release>[^<]+<languageId>(\\d*)</languageId>[^<]+<languageName>(.*?)</languageName>[^<]+<time>(\\d*)</time>[^<]+<tvSeason>(\\d*)</tvSeason>[^<]+<tvEpisode>(\\d*)</tvEpisode>[^<]+<tvSpecial>(\\d*)</tvSpecial>[^<]+<cds>(\\d*)</cds>[^<]+<format>(.*?)</format>[^<]+<fps>(.*?)</fps>[^<]+<rating>(\\d*)</rating>[^<]+<flags/?>(?:(.*?)</flags>)?[^<]+<downloads>(\\d*)</downloads>[^<]+</subtitle>", regex_flags),
    std::regex("<a href=\"(/en/ppodnapisi/download/i/\\d+/k/[^\"]+)\">", regex_flags),
};

SRESULT podnapisi::Login(std::string& sUserName, std::string& sPassword)
{
    //TODO: implement
    return SR_UNDEFINED;
}

/*
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
 "/sJ/0/"    //Languages, 0=all
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
    int page = 1, pages = 1;
    do {
        CheckAbortAndReturn();
        std::string data;
        std::string languages(GetLanguagesString());

        std::string search(pFileInfo.title);
        if (!pFileInfo.country.empty()) { search += " " + pFileInfo.country; }
        search = std::regex_replace(search, std::regex(" and | *[!?&':] *", regex_flags), " ");

        std::string url("http://simple.podnapisi.net/en/ppodnapisi/search");
        url += "/sXML/1";
        url += "/sAKA/1";
        url += (!search.empty() ? "/sK/" + UrlEncode(search.c_str()) : "");
        url += (pFileInfo.year != -1 ? "/sY/" + std::to_string(pFileInfo.year) : "");
        url += (pFileInfo.seasonNumber != -1 ? "/sTS/" + std::to_string(pFileInfo.seasonNumber) : "");
        url += (pFileInfo.episodeNumber != -1 ? "/sTE/" + std::to_string(pFileInfo.episodeNumber) : "");
        //url += "/sR/" + UrlEncode(pFileInfo.fileName.c_str());
        url += (!languages.empty() ? "/sJ/" + languages : "");
        url += "/page/" + std::to_string(page);
        searchResult = Download(url, "", data);

        regex_results results;
        string_regex(regex_pattern[0], data, results);
        for (const auto& iter : results) {
            page = atoi(iter[0].c_str());
            pages = atoi(iter[1].c_str());;
        }
        // 30 results per page
        if (pages > 3) { return SR_TOOMANY; }

        regex_results results1;
        string_regex(regex_pattern[1], data, results1);
        for (const auto& iter1 : results1) {
            if (pFileInfo.seasonNumber > 0 && pFileInfo.episodeNumber <= 0 && atoi(iter1[12].c_str()) != 0) { continue; }
            CheckAbortAndReturn();
            SubtitlesInfo pSubtitlesInfo;
            pSubtitlesInfo.id = iter1[0];
            pSubtitlesInfo.title =  HtmlSpecialCharsDecode(iter1[1].c_str());
            pSubtitlesInfo.year = iter1[2].empty() ? -1 : atoi(iter1[2].c_str());
            pSubtitlesInfo.url = iter1[4];
            pSubtitlesInfo.fileExtension = iter1[15] == "SubRip" ? "srt" : iter1[15];

            pSubtitlesInfo.releaseName = iter1[7];
            pSubtitlesInfo.languageCode = podnapisi_languages[atoi(iter1[8].c_str())].code;
            pSubtitlesInfo.languageName = iter1[9];
            pSubtitlesInfo.seasonNumber = atoi(iter1[11].c_str());
            pSubtitlesInfo.episodeNumber = atoi(iter1[12].c_str());
            pSubtitlesInfo.discNumber = atoi(iter1[14].c_str());
            pSubtitlesInfo.discCount = atoi(iter1[14].c_str());
            pSubtitlesInfo.hearingImpaired = (iter1[18].find("n") != std::string::npos) ? TRUE : FALSE;
            pSubtitlesInfo.corrected = (iter1[18].find("r") != std::string::npos) ? -1 : 0;
            pSubtitlesInfo.downloadCount = atoi(iter1[19].c_str());

            string_array fileNames(string_tokenize(iter1[7], " "));
            if (fileNames.empty()) {
                std::string str = pSubtitlesInfo.title;
                if (pSubtitlesInfo.year > 0) { str += " " + iter1[2]; }
                if (pSubtitlesInfo.seasonNumber > 0) { str += string_format(" S%02d", pSubtitlesInfo.seasonNumber); }
                if (pSubtitlesInfo.episodeNumber > 0) { str += string_format("%sE%02d", (pSubtitlesInfo.seasonNumber > 0) ? "" : " ", pSubtitlesInfo.episodeNumber); }
                str += " (*)";
                fileNames.push_back(str);
            }
            pSubtitlesInfo.fileName = fileNames[0] + "." + pSubtitlesInfo.fileExtension;
            for (const auto& fileName : fileNames) {
                if (fileName == pFileInfo.fileName) {
                    pSubtitlesInfo.fileName = fileName + "." + pSubtitlesInfo.fileExtension;
                }
            }
            Set(pSubtitlesInfo);
        }
    } while (page++ < pages);

    return searchResult;
}

SRESULT podnapisi::Download(SubtitlesInfo& pSubtitlesInfo)
{
    SRESULT searchResult = SR_UNDEFINED;
    std::string temp;
    searchResult = Download(pSubtitlesInfo.url, "", temp);

    regex_results results;
    string_regex(regex_pattern[2], temp, results);
    for (const auto& iter : results) {
        CheckAbortAndReturn();
        searchResult = Download("http://simple.podnapisi.net" + iter[0], "", pSubtitlesInfo.fileContents);
    }
    return searchResult;
}

std::string podnapisi::Languages()
{
    static std::string result;
    if (result.empty()) {
        for (const auto& iter : podnapisi_languages) {
            if (strlen(iter.code) && result.find(iter.code) == std::string::npos) {
                result += (result.empty() ? "" : ",");
                result += iter.code;
            }
        }
    }
    return result;
}

std::string podnapisi::GetLanguagesString()
{
    std::string result;
    std::string languages(LanguagesISO6391());
    if (!languages.empty()) {
        for (const auto& iter : podnapisi_languages) {
            if (strlen(iter.code) && languages.find(iter.code) != std::string::npos) {
                result += (result.empty() ? "" : ",") + std::to_string(&iter - &podnapisi_languages[0]);
            }
        }
    }
    return result;
}

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
    SRESULT searchResult = SR_UNDEFINED;

    std::string languages = GetLanguagesString();
    if (!LanguagesISO6391().empty() && languages.empty()) {
        return searchResult;
    }

    std::string KEY = "WC1ERVYtREVTS1RPUF9maWUyYS1hMVJzYS1hSHc0UA==";
    std::string url(string_format("http://api.titlovi.com/xml_get_api.ashx?x-dev_api_id=%s&uiculture=en&forcefilename=true", Base64::decode(KEY).c_str()));
    url += "&mt=" + (pFileInfo.seasonNumber != -1 ? std::to_string(2) : std::to_string(1));
    url += "&keyword=" + UrlEncode(pFileInfo.title.c_str());
    url += (pFileInfo.seasonNumber != -1 ? "&season=" + std::to_string(pFileInfo.seasonNumber) : "");
    url += (pFileInfo.episodeNumber != -1 ? "&episode=" + std::to_string(pFileInfo.episodeNumber) : "");
    url += (pFileInfo.year != -1 ? "&year=" + std::to_string(pFileInfo.year) : "");
    url += (!languages.empty() ? "&language=" + languages : "");

    std::string data;
    searchResult = Download(url, "", data);

    tinyxml2::XMLDocument dxml;
    if (dxml.Parse(data.c_str()) == XMLError::XML_SUCCESS) {

        auto GetChildElementText = [&](XMLElement * pElement, const char* value) -> std::string {
            std::string str;
            XMLElement* pChildElement = pElement->FirstChildElement(value);
            if (pChildElement != nullptr)
            {
                auto pText = pChildElement->GetText();
                if (pText != nullptr) { str = pText; }
            }
            return str;
        };

        XMLElement* pRootElmt = dxml.FirstChildElement("subtitles");
        if (pRootElmt) {
            std::string name = pRootElmt->Name();
            std::string strAttr = pRootElmt->Attribute("resultsCount");
            int num = pRootElmt->IntAttribute("resultsCount");
            if (num > 0/* && num < 50*/) {
                XMLElement* pSubtitleElmt = pRootElmt->FirstChildElement();

                while (pSubtitleElmt) {
                    SubtitlesInfo pSubtitlesInfo;

                    pSubtitlesInfo.title = GetChildElementText(pSubtitleElmt, "title");
                    pSubtitlesInfo.languageCode = GetChildElementText(pSubtitleElmt, "language");
                    for (const auto& language : titlovi_languages) { if (pSubtitlesInfo.languageCode == language.code) { pSubtitlesInfo.languageCode = language.name; } }
                    pSubtitlesInfo.languageName = UTF16To8(ISO639XToLanguage(pSubtitlesInfo.languageCode.c_str()));
                    pSubtitlesInfo.releaseName = GetChildElementText(pSubtitleElmt, "release");
                    pSubtitlesInfo.imdbid = GetChildElementText(pSubtitleElmt, "imdbId");
                    pSubtitlesInfo.frameRate = atof(GetChildElementText(pSubtitleElmt, "fps").c_str());
                    pSubtitlesInfo.year = atoi(GetChildElementText(pSubtitleElmt, "year").c_str());
                    pSubtitlesInfo.discNumber = atoi(GetChildElementText(pSubtitleElmt, "cd").c_str());
                    pSubtitlesInfo.discCount = pSubtitlesInfo.discNumber;
                    pSubtitlesInfo.downloadCount = atoi(GetChildElementText(pSubtitleElmt, "downloads").c_str());

                    XMLElement* pSubtitleChildElmt = nullptr;
                    if ((pSubtitleChildElmt = pSubtitleElmt->FirstChildElement("urls")) != nullptr) {
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
                    if (pSubtitlesInfo.seasonNumber > 0) { pSubtitlesInfo.fileName += string_format(" S%02d", pSubtitlesInfo.seasonNumber); }
                    if (pSubtitlesInfo.episodeNumber > 0) { pSubtitlesInfo.fileName += string_format("%sE%02d", (pSubtitlesInfo.seasonNumber > 0) ? "" : " ", pSubtitlesInfo.episodeNumber); }
                    pSubtitlesInfo.fileName += " " + pSubtitlesInfo.releaseName;
                    pSubtitlesInfo.fileName += " (*)";

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
    return Download(pSubtitlesInfo.id.c_str(), "", pSubtitlesInfo.fileContents);
}

std::string titlovi::Languages()
{
    static std::string result;
    if (result.empty()) {
        for (const auto& iter : titlovi_languages) {
            if (strlen(iter.name) && result.find(iter.name) == std::string::npos) {
                result += (result.empty() ? "" : ",");
                result += iter.name;
            }
        }
    }
    return result; // "hr,sr,sl,bs,en,mk";
}

std::string titlovi::GetLanguagesString()
{
    std::string result;
    std::string languages(LanguagesISO6391());
    if (!languages.empty()) {
        for (const auto& iter : titlovi_languages) {
            if (strlen(iter.name) && languages.find(iter.name) != std::string::npos) {
                result += (result.empty() ? "" : ",") + std::string(iter.code);
            }
        }
    }
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
        std::string url(string_format("https://yts.re/api/list.json?keywords=%s+%d", UrlEncode(string_replace(pFileInfo.title, " ", "+").c_str()), pFileInfo.year));
        std::string data;
        searchResult = Download(url, "", data);

        Document d;
        if (d.ParseInsitu(&data[0]).HasParseError()) {
            return SR_FAILED;
        }

        auto iter = d.FindMember("MovieCount");
        if (iter != d.MemberEnd()) {
            iter = d.FindMember("MovieList");
            if ((iter != d.MemberEnd()) && (iter->value.IsArray())) {
                std::set<std::string> imdb_ids;
                for (auto elem = iter->value.Begin(); elem != iter->value.End(); ++elem) {
                    std::string imdb = elem->FindMember("ImdbCode")->value.GetString();
                    if (imdb_ids.find(imdb) == imdb_ids.end()) {
                        imdb_ids.insert(imdb);

                        std::string url(string_format("http://api.ysubs.com/subs/%s", imdb.c_str()));
                        std::string data1;
                        searchResult = Download(url, "", data1);
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
                                    for (const auto& language : ysubs_languages) { if (lang == language.name) { lang_code = language.code; } }
                                    if (CheckLanguage(lang_code)) {
                                        for (auto elem2 = elem1->value.Begin(); elem2 != elem1->value.End(); ++elem2) {
                                            SubtitlesInfo pSubtitlesInfo;

                                            pSubtitlesInfo.title = elem->FindMember("MovieTitleClean")->value.GetString();
                                            pSubtitlesInfo.languageCode = lang_code;
                                            pSubtitlesInfo.languageName = UTF16To8(ISO639XToLanguage(pSubtitlesInfo.languageCode.c_str()));
                                            pSubtitlesInfo.releaseName = "YIFY";
                                            pSubtitlesInfo.imdbid = imdb;
                                            pSubtitlesInfo.year = atoi(elem->FindMember("MovieYear")->value.GetString());
                                            pSubtitlesInfo.discNumber = 1;
                                            pSubtitlesInfo.discCount = 1;

                                            pSubtitlesInfo.url = "http://www.yifysubtitles.com/movie-imdb/" + imdb;
                                            std::string str = elem2->FindMember("url")->value.GetString();
                                            pSubtitlesInfo.id = "http://www.yifysubtitles.com" + str;
                                            pSubtitlesInfo.hearingImpaired = elem2->FindMember("hi")->value.GetInt();
                                            pSubtitlesInfo.corrected = elem2->FindMember("rating")->value.GetInt();

                                            pSubtitlesInfo.fileName = pFileInfo.fileName;
                                            pSubtitlesInfo.fileName += " (*)";

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
    return Download(pSubtitlesInfo.id.c_str(), "", pSubtitlesInfo.fileContents);
}

std::string ysubs::Languages()
{
    static std::string result;
    if (result.empty()) {
        for (const auto& iter : ysubs_languages) {
            if (strlen(iter.code) && result.find(iter.code) == std::string::npos) {
                result += (result.empty() ? "" : ",");
                result += iter.code;
            }
        }
    }
    return result;
}
