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
#include "SubtitlesProvider.h"

/******************************************************************************
** Register providers
******************************************************************************/
void SubtitlesProviders::RegisterProviders()
{
    Register<OpenSubtitles>();
    Register<OpenSubtitlesISDB>();
    Register<SubDB>();
    Register<TVsubtitles>();
    Register<Moviesubtitles>();
    Register<podnapisi>();
    Register<addic7ed>();
}

#define CheckAbortAndReturn() { if (_bAborting) return SR_ABORTED; }

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

SRESULT OpenSubtitles::Login(std::string& _UserName, std::string& _Password)
{
    if (xmlrpc) {
        XmlRpcValue args, result;
        args[0] = _UserName;
        args[1] = _Password;
        args[2] = "en";
#if (defined(_DEBUG) || MPC_NIGHTLY_RELEASE == 1)
        args[3] = "OS Test User Agent";
#else
        args[3] = UserAgent().c_str();
#endif
        if (!xmlrpc->execute("LogIn", args, result)) { return SR_FAILED; }
        token = result["token"];
    }
    return token.valid() ? SR_SUCCEEDED : SR_FAILED;
}

SRESULT OpenSubtitles::Hash(SubtitlesInfo& fileInfo)
{
    UINT64 fileHash = fileInfo.fileSize;
    if (fileInfo.pAsyncReader) {
        UINT64 position = 0;
        for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && SUCCEEDED(fileInfo.pAsyncReader->SyncRead(position, sizeof(tmp), (BYTE*)&tmp)); fileHash += tmp, position += sizeof(tmp), ++i);
        position = std::max((UINT64)0, (UINT64)(fileInfo.fileSize - PROBE_SIZE));
        for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && SUCCEEDED(fileInfo.pAsyncReader->SyncRead(position, sizeof(tmp), (BYTE*)&tmp)); fileHash += tmp, position += sizeof(tmp), ++i);
    } else {
        CFile file;
        CFileException fileException;
        if (file.Open(CString(fileInfo.filePath.c_str()), CFile::modeRead | CFile::osSequentialScan | CFile::shareDenyNone | CFile::typeBinary, &fileException)) {
            for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && file.Read(&tmp, sizeof(tmp)); fileHash += tmp, ++i);
            file.Seek(std::max((UINT64)0, (UINT64)(fileInfo.fileSize - PROBE_SIZE)), CFile::begin);
            for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && file.Read(&tmp, sizeof(tmp)); fileHash += tmp, ++i);
        }
    }
    fileInfo.fileHash = string_format("%016I64x", fileHash);
    return SR_SUCCEEDED;
}

SRESULT OpenSubtitles::Search(const SubtitlesInfo& fileInfo, volatile BOOL& _bAborting)
{
    std::string languages(LanguagesISO6392());
    XmlRpcValue args, result;
    args[0] = token;
    args[1][0]["sublanguageid"] = !languages.empty() ? languages : "all";
    args[1][0]["moviehash"] = fileInfo.fileHash;
    args[1][0]["moviebytesize"] = (int)fileInfo.fileSize;
    args[2]["limit"] = 500;

    if (!xmlrpc->execute("SearchSubtitles", args, result)) { return SR_FAILED; }

    if (result["data"].getType() != XmlRpcValue::Type::TypeArray) { return SR_FAILED; }

    int nCount = result["data"].size();
    for (int i = 0; i < nCount; ++i) {
        CheckAbortAndReturn();
        XmlRpcValue& data(result["data"][i]);
        SubtitlesInfo subtitlesInfo;
        subtitlesInfo.id = (const char*)data["IDSubtitleFile"];
        subtitlesInfo.discNumber = data["SubActualCD"];
        subtitlesInfo.discCount = data["SubSumCD"];
        subtitlesInfo.fileExtension = (const char*)data["SubFormat"];
        subtitlesInfo.languageCode = (const char*)data["ISO639"]; //"SubLanguageID"
        subtitlesInfo.languageName = (const char*)data["LanguageName"];
        subtitlesInfo.downloadCount = data["SubDownloadsCnt"];

        subtitlesInfo.fileName = (const char*)data["SubFileName"];
        regex_result results;
        string_regex("\"([^\"]+)\" (.+)", (const char*)data["MovieName"], results);
        if (!results.empty()) {
            subtitlesInfo.title = results[0];
            subtitlesInfo.title2 = results[1];
        } else {
            subtitlesInfo.title = (const char*)data["MovieName"];
        }
        subtitlesInfo.year = (int)data["MovieYear"] == 0 ? -1 : (int)data["MovieYear"];
        subtitlesInfo.seasonNumber = (int)data["SeriesSeason"] == 0 ? -1 : (int)data["SeriesSeason"];
        subtitlesInfo.episodeNumber = (int)data["SeriesEpisode"] == 0 ? -1 : (int)data["SeriesEpisode"];
        subtitlesInfo.hearingImpaired = data["SubHearingImpaired"];
        subtitlesInfo.url = (const char*)data["SubtitlesLink"];
        subtitlesInfo.releaseName = (const char*)data["MovieReleaseName"];
        subtitlesInfo.imdbid = (const char*)data["IDMovieImdb"];
        subtitlesInfo.corrected = (int)data["SubBad"] ? -1 : 0;
        Set(fileInfo, subtitlesInfo, _bAborting);
    }
    return SR_SUCCEEDED;
}

SRESULT OpenSubtitles::Download(SubtitlesInfo& subtitlesInfo, volatile BOOL& _bAborting)
{
    XmlRpcValue args, result;
    args[0] = token;
    args[1][0] = subtitlesInfo.id;
    if (!xmlrpc->execute("DownloadSubtitles", args, result)) { return SR_FAILED; }

    if (result["data"].getType() != XmlRpcValue::Type::TypeArray) { return SR_FAILED; }

    subtitlesInfo.fileContents = Base64::decode(std::string(result["data"][0]["data"]));
    return SR_SUCCEEDED;
}

SRESULT OpenSubtitles::Upload(const SubtitlesInfo& fileInfo, volatile BOOL& _bAborting)
{
    XmlRpcValue args, result;
    args[0] = token;

    //TODO: Ask  how to obtain commented values !!!
    args[1]["cd1"]["subhash"] = string_hash(fileInfo.fileContents, CALG_MD5);
    args[1]["cd1"]["subfilename"] = fileInfo.fileName + ".srt";
    args[1]["cd1"]["moviehash"] = fileInfo.fileHash;
    args[1]["cd1"]["moviebytesize"] = (int)fileInfo.fileSize;
    //args[1]["cd1"]["movietimems"];
    //args[1]["cd1"]["movieframes"];
    //args[1]["cd1"]["moviefps"];
    args[1]["cd1"]["moviefilename"] = fileInfo.fileName + "." + fileInfo.fileExtension;

    CheckAbortAndReturn();
    if (!xmlrpc->execute("TryUploadSubtitles", args, result)) { return SR_FAILED; }
    CheckAbortAndReturn();

    if ((int)result["alreadyindb"] == 1) {
        return SR_EXISTS;
    } else if ((int)result["alreadyindb"] == 0) {
        // We need imdbid to proceed
        if (result["data"].getType() == XmlRpcValue::Type::TypeArray) {
            args[1]["baseinfo"]["idmovieimdb"] = result["data"][0]["IDMovieImdb"];
        } else if (!fileInfo.imdbid.empty()) {
            args[1]["baseinfo"]["idmovieimdb"] = fileInfo.imdbid;
        } else {
            std::string title(string_replace(fileInfo.title, "and", "&"));
            if (!args[1]["baseinfo"]["idmovieimdb"].valid()) {
                XmlRpcValue _args, _result;
                _args[0] = token;
                _args[1][0] = fileInfo.fileHash;
                if (!xmlrpc->execute("CheckMovieHash", _args, _result)) { return SR_FAILED; }

                if (_result["data"].getType() == XmlRpcValue::Type::TypeStruct) {

                    //regex_results results;
                    //string_regex("\"(.+)\" (.+)", (const char*)data["MovieName"], results);
                    //if (!results.empty()) {
                    //    subtitlesInfo.title = results[0][0];
                    //    subtitlesInfo.title2 = results[0][1];
                    //} else {
                    //    subtitlesInfo.title = (const char*)data["MovieName"];
                    //}

                    regex_results results;
                    string_regex("\"(.+)\" .+|(.+)", string_replace((const char*)_result["data"][fileInfo.fileHash]["MovieName"], "and", "&"), results);
                    std::string _title(results[0][0] + results[0][1]);

                    if (_strcmpi(title.c_str(), _title.c_str()) == 0 /*&& (fileInfo.year == -1 || (fileInfo.year != -1 && fileInfo.year == atoi(_result["data"][fileInfo.fileHash]["MovieYear"])))*/) {
                        args[1]["baseinfo"]["idmovieimdb"] = _result["data"][fileInfo.fileHash]["MovieImdbID"]; //imdbid
                    }
                }
            }

            if (!args[1]["baseinfo"]["idmovieimdb"].valid()) {
                XmlRpcValue _args, _result;
                _args[0] = token;
                _args[1][0] = fileInfo.fileHash;
                if (!xmlrpc->execute("CheckMovieHash2", _args, _result)) { return SR_FAILED; }

                if (_result["data"].getType() == XmlRpcValue::Type::TypeArray) {
                    int nCount = _result["data"][fileInfo.fileHash].size();
                    for (int i = 0; i < nCount; ++i) {
                        regex_results results;
                        string_regex("\"(.+)\" .+|(.+)", string_replace((const char*)_result["data"][fileInfo.fileHash][i]["MovieName"], "and", "&"), results);
                        std::string _title(results[0][0] + results[0][1]);

                        if (_strcmpi(title.c_str(), _title.c_str()) == 0 /*&& (fileInfo.year == -1 || (fileInfo.year != -1 && fileInfo.year == atoi(_result["data"][fileInfo.fileHash][i]["MovieYear"])))*/) {
                            args[1]["baseinfo"]["idmovieimdb"] = _result["data"][fileInfo.fileHash][i]["MovieImdbID"]; //imdbid
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

                            if (_strcmpi(title.c_str(), _title.c_str()) == 0 /*&& (fileInfo.year == -1 || (fileInfo.year != -1 && fileInfo.year == atoi(results[0][1].c_str())))*/) {
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
            _args[1][0]["moviehash"] = fileInfo.fileHash;
            _args[1][0]["moviebytesize"] = (int)fileInfo.fileSize;
            _args[1][0]["imdbid"] = args[1]["baseinfo"]["idmovieimdb"];
            //_args[1][0]["movietimems"];
            //_args[1][0]["moviefps"];
            _args[1][0]["moviefilename"] = fileInfo.fileName + "." + fileInfo.fileExtension;
            if (!xmlrpc->execute("InsertMovieHash", _args, _result)) { return SR_FAILED; }
            // REsult value is irrelevant
            _result["data"]["accepted_moviehashes"];


            //args[1]["baseinfo"]["moviereleasename"];
            //args[1]["baseinfo"]["movieaka"];
            //args[1]["baseinfo"]["sublanguageid"];
            //args[1]["baseinfo"]["subauthorcomment"];
            if (fileInfo.hearingImpaired != -1) {
                args[1]["baseinfo"]["hearingimpaired"] = fileInfo.hearingImpaired;
            }
            //args[1]["baseinfo"]["highdefinition"];
            //args[1]["baseinfo"]["automatictranslation"];

            args[1]["cd1"]["subcontent"] = Base64::encode(string_gzcompress(fileInfo.fileContents));

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
** OpenSubtitlesISDB
******************************************************************************/

SRESULT OpenSubtitlesISDB::Hash(SubtitlesInfo& fileInfo)
{
    UINT64 fileHash = fileInfo.fileSize;
    if (fileInfo.pAsyncReader) {
        UINT64 position = 0;
        for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && SUCCEEDED(fileInfo.pAsyncReader->SyncRead(position, sizeof(tmp), (BYTE*)&tmp)); fileHash += tmp, position += sizeof(tmp), ++i);
        position = std::max((UINT64)0, (UINT64)(fileInfo.fileSize - PROBE_SIZE));
        for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && SUCCEEDED(fileInfo.pAsyncReader->SyncRead(position, sizeof(tmp), (BYTE*)&tmp)); fileHash += tmp, position += sizeof(tmp), ++i);
    } else {
        CFile file;
        CFileException fileException;
        if (file.Open(CString(fileInfo.filePath.c_str()), CFile::modeRead | CFile::osSequentialScan | CFile::shareDenyNone | CFile::typeBinary, &fileException)) {
            for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && file.Read(&tmp, sizeof(tmp)); fileHash += tmp, ++i);
            file.Seek(std::max((UINT64)0, (UINT64)(fileInfo.fileSize - PROBE_SIZE)), CFile::begin);
            for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && file.Read(&tmp, sizeof(tmp)); fileHash += tmp, ++i);
        }
    }
    fileInfo.fileHash = string_format("%016I64x", fileHash);
    return SR_SUCCEEDED;
}

SRESULT OpenSubtitlesISDB::Search(const SubtitlesInfo& fileInfo, volatile BOOL& _bAborting)
{
    SRESULT searchResult = SR_UNDEFINED;
    std::string data;
    searchResult = Download(string_format("http://www.opensubtitles.org/isdb/index.php?player=mpc-hc&name[0]=%s&size[0]=%016I64x&hash[0]=%s",
                                          UrlEncode(fileInfo.fileName.c_str()), fileInfo.fileSize, fileInfo.fileHash.c_str()), "", data);

    SubtitlesInfo subtitlesInfo;
    string_array tags(string_tokenize(data, "\n"));
    for (const auto& iter : tags) {
        CheckAbortAndReturn();
        std::string::size_type pos = iter.find("=");
        std::string param(iter.substr(0, pos));
        std::string value(pos != std::string::npos ? iter.substr(pos + 1) : "");

        if (param == "ticket") {
            ticket = value;
        } else if (param == "movie") {
            subtitlesInfo = SubtitlesInfo();
            subtitlesInfo.title = string_replace(string_trim(value, " |"), "|", ", ");
        } else if (param == "subtitle") {
            subtitlesInfo.id = value;
        } else if (param == "name") {
            subtitlesInfo.fileName = value;
        } else if (param == "discs") {
            subtitlesInfo.discCount = atoi(value.c_str());
        } else if (param == "disc_no") {
            subtitlesInfo.discNumber = atoi(value.c_str());
        } else if (param == "format") {
            subtitlesInfo.fileExtension = value;
        } else if (param == "iso639_2") {
            subtitlesInfo.languageCode = value;
        } else if (param == "language") {
            subtitlesInfo.languageName = value;
        } else if (param == "nick") {
            //    subtitlesInfo.nick = value;
        } else if (param == "email") {
            //    //subtitlesInfo.email = value;
        } else if (param == "endsubtitle") {
            if (CheckLanguage(subtitlesInfo.languageCode)) {
                Set(fileInfo, subtitlesInfo, _bAborting);
            }
        } else if (param == "endmovie") {
        } else if (param == "end") {
            break;
        }
    }

    return searchResult;
}

SRESULT OpenSubtitlesISDB::Download(SubtitlesInfo& subtitlesInfo, volatile BOOL& _bAborting)
{
    return Download(string_format("http://www.opensubtitles.org/isdb/dl.php?id=%s&ticket=%s", subtitlesInfo.id.c_str(), ticket.c_str()), "", subtitlesInfo.fileContents);
}

std::string OpenSubtitlesISDB::Languages()
{
    return "af,sq,ar,hy,eu,bn,bs,pb,br,bg,my,ca,zh,hr,cs,da,nl,en,eo,et,fi,fr,gl,ka,de,el,he,hi,hu,is,id,it,ja,kk,km,ko,lv,lt,lb,mk,ms,ml,mn,no,oc,fa,pl,pt,ro,ru,sr,si,sk,sl,es,sw,sv,tl,ta,te,th,tr,uk,ur,vi";
}

/******************************************************************************
** SubDB
******************************************************************************/

SRESULT SubDB::Hash(SubtitlesInfo& fileInfo)
{
    std::vector<BYTE> buffer(2 * PROBE_SIZE);
    if (fileInfo.pAsyncReader) {
        UINT64 position = 0;
        fileInfo.pAsyncReader->SyncRead(position, PROBE_SIZE, (BYTE*)&buffer[0]);
        position = std::max((UINT64)0, (UINT64)(fileInfo.fileSize - PROBE_SIZE));
        fileInfo.pAsyncReader->SyncRead(position, PROBE_SIZE, (BYTE*)&buffer[PROBE_SIZE]);
    } else {
        CFile file;
        CFileException fileException;
        if (file.Open(CString(fileInfo.filePath.c_str()), CFile::modeRead | CFile::osSequentialScan | CFile::shareDenyNone | CFile::typeBinary, &fileException)) {
            file.Read(&buffer[0], PROBE_SIZE);
            file.Seek(std::max((UINT64)0, (UINT64)(fileInfo.fileSize - PROBE_SIZE)), CFile::begin);
            file.Read(&buffer[PROBE_SIZE], PROBE_SIZE);
        }
    }
    fileInfo.fileHash = string_hash(std::string((char*)&buffer[0], buffer.size()), CALG_MD5);
    return SR_SUCCEEDED;
}

SRESULT SubDB::Search(const SubtitlesInfo& fileInfo, volatile BOOL& _bAborting)
{
    SRESULT searchResult = SR_UNDEFINED;
    std::string data;
    searchResult = Download(string_format("http://api.thesubdb.com/?action=search&hash=%s", fileInfo.fileHash.c_str()), "", data);

    if (!data.empty()) {
        string_array result(string_tokenize(data, ","));
        for (const auto& iter : result) {
            CheckAbortAndReturn();
            if (CheckLanguage(iter)) {
                SubtitlesInfo subtitlesInfo;
                subtitlesInfo.id = fileInfo.fileHash;
                subtitlesInfo.fileExtension = "srt";
                subtitlesInfo.fileName = fileInfo.fileName + "." + subtitlesInfo.fileExtension;
                subtitlesInfo.languageCode = iter;
                subtitlesInfo.languageName = UTF16To8(ISO639XToLanguage(iter.c_str()));
                subtitlesInfo.discNumber = 1;
                subtitlesInfo.discCount = 1;
                subtitlesInfo.title = fileInfo.title;
                Set(fileInfo, subtitlesInfo, _bAborting);
            }
        }
    }

    return searchResult;
}

SRESULT SubDB::Download(SubtitlesInfo& subtitlesInfo, volatile BOOL& _bAborting)
{
    return Download(string_format("http://api.thesubdb.com/?action=download&hash=%s&language=%s", subtitlesInfo.id.c_str(), subtitlesInfo.languageCode.c_str()), "", subtitlesInfo.fileContents);
}

SRESULT SubDB::Upload(const SubtitlesInfo& fileInfo, volatile BOOL& _bAborting)
{
#define MULTIPART_BOUNDARY "xYzZY"
    std::string url(string_format("http://api.thesubdb.com/?action=upload&hash=%s", fileInfo.fileHash.c_str()));
    string_map headers({
        { "User-Agent", UserAgent() },
        { "Content-Type", "multipart/form-data; boundary=" MULTIPART_BOUNDARY },
    });

    std::string content, data;
    content += string_format("--%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n", MULTIPART_BOUNDARY, "hash", fileInfo.fileHash.c_str());
    content += string_format("--%s\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s.%s\"\r\nContent-Type: application/octet-stream\r\nContent-Transfer-Encoding: binary\r\n\r\n",
                             MULTIPART_BOUNDARY, "file", fileInfo.fileHash.c_str(), "srt");
    content += fileInfo.fileContents;
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
    std::regex(),
    std::regex("<a href=\"/subtitle-(\\d+)[.]html\">[^]+?<img src=\"images/flags/([a-z]{2})[.]gif\"[^]+?</div></a>", regex_flags),
    std::regex("filename:.+?>([^<\n\t]+)[^]+?downloads:.+?>(\\d+)", regex_flags),
};

SRESULT TVsubtitles::Search(const SubtitlesInfo& fileInfo, volatile BOOL& _bAborting)
{
    SRESULT searchResult = SR_UNDEFINED;
    if (fileInfo.seasonNumber != -1) {
        std::string data;
        searchResult = Download(string_format("http://www.tvsubtitles.net/search.php?q=%s", UrlEncode(fileInfo.title.c_str())), "", data);

        regex_results results;
        string_regex(regex_pattern[0], data, results);
        for (const auto& iter : results) {
            CheckAbortAndReturn();
            if (_strcmpi(fileInfo.title.c_str(), iter[2].c_str()) != 0) { continue; }
            std::string data1;
            searchResult = Download(string_format("http://www.tvsubtitles.net/tvshow-%s-%d.html", iter[1].c_str(), fileInfo.seasonNumber), "", data1);

            regex_results results1;
            string_regex(std::regex(string_format("<td>%dx%0*d</td>[^<]*<td[^>]+><a href=\"(episode-(\\d+)[.]html)\"><b>(.*?)</b></a></td>[^<]*<td>([1-9]\\d*)</td>", fileInfo.seasonNumber, 2, fileInfo.episodeNumber), regex_flags), data1, results1);
            for (const auto& iter1 : results1) {
                CheckAbortAndReturn();
                std::string data2;
                searchResult = Download(string_format("http://www.tvsubtitles.net/episode-%s.html", iter1[1].c_str()), "", data2);

                regex_results results2;
                //string_regex(string_format("<a href=\"/subtitle-(\\d+)[.]html\">[^]+?<img src=\"images/flags/(%s)[.]gif\"", GetLanguagesString().c_str()), data2, results2);
                //too slow//string_regex(string_format("<a href=\"/subtitle-(\\d+)[.]html\">[^]+?<img src=\"images/flags/(%s)[.]gif\"[^]+?>([^<]+)[^]+?>\n\t([^<]+)[^]+?>\n\t([^<]+)[^]+?>\n\t(\\d+)</p></div></a>", GetLanguagesString().c_str()), data2, results2);
                //TODO: test
                //string_regex("<a href=\"/subtitle-(\\d+)[.]html\">[^]+?<img src=\"images/flags/([a-z]{2})[.]gif\"[^]+?>([^<]+)[^]+?>\n\t([^<]+)[^]+?>\n\t([^<]+)[^]+?>\n\t(\\d+)</p></div></a>", data2, results2);
                string_regex(regex_pattern[2], data2, results2);
                for (auto& iter2 : results2) {
                    CheckAbortAndReturn();
                    for (const auto& language : tvsubtitles_languages) { if (iter2[1] == language.code) { iter2[1] = language.name; } }
                    if (CheckLanguage(iter2[1])) {
                        std::string data3;
                        std::string url = string_format("http://www.tvsubtitles.net/subtitle-%s.html", iter2[0].c_str());
                        searchResult = Download(url, "", data3);

                        regex_results results3;
                        SubtitlesInfo subtitlesInfo;
                        string_regex(regex_pattern[3], data3, results3);
                        for (const auto& iter3 : results3) {
                            CheckAbortAndReturn();
                            subtitlesInfo.fileName = iter3[0];
                            subtitlesInfo.title = iter[2];
                            subtitlesInfo.country = iter[3];
                            subtitlesInfo.year = iter[4].empty() ? -1 : atoi(iter[4].c_str());
                            subtitlesInfo.title2 = iter1[2];
                            subtitlesInfo.seasonNumber = fileInfo.seasonNumber;
                            subtitlesInfo.episodeNumber = fileInfo.episodeNumber;
                            subtitlesInfo.id = iter2[0];
                            subtitlesInfo.url = url;
                            subtitlesInfo.languageCode = iter2[1];
                            subtitlesInfo.languageName = UTF16To8(ISO639XToLanguage(subtitlesInfo.languageCode.c_str()));
                            subtitlesInfo.downloadCount = atoi(iter3[1].c_str());
                            subtitlesInfo.discNumber = 1;
                            subtitlesInfo.discCount = 1;
                            Set(fileInfo, subtitlesInfo, _bAborting);
                        }
                    }
                }
            }
        }
    }
    return searchResult;
}

SRESULT TVsubtitles::Download(SubtitlesInfo& subtitlesInfo, volatile BOOL& _bAborting)
{
    return Download(string_format("http://www.tvsubtitles.net/download-%s.html", subtitlesInfo.id.c_str()), "", subtitlesInfo.fileContents);
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

SRESULT Moviesubtitles::Search(const SubtitlesInfo& fileInfo, volatile BOOL& _bAborting)
{
    SRESULT searchResult = SR_UNDEFINED;
    if (fileInfo.seasonNumber == -1) {
        std::string data;
        searchResult = Download(string_format("http://www.moviesubtitles.org/search.php?q=%s", UrlEncode(fileInfo.title.c_str())), "", data);

        regex_results results;
        string_regex(regex_pattern[0], data, results);
        for (const auto& iter : results) {
            CheckAbortAndReturn();
            if (_strcmpi(fileInfo.title.c_str(), iter[2].c_str()) != 0) { continue; }
            std::string data1;
            searchResult = Download(string_format("http://www.moviesubtitles.org/movie-%s.html", iter[1].c_str()), "", data1);

            regex_results results1;
            string_regex(regex_pattern[1], data1, results1);
            for (auto& iter1 : results1) {
                CheckAbortAndReturn();
                for (const auto& language : tvsubtitles_languages) { if (iter1[0] == language.code) { iter1[0] = language.name; } }
                if (CheckLanguage(iter1[0])) {
                    SubtitlesInfo subtitlesInfo;
                    std::string data2;
                    std::string url(string_format("http://www.moviesubtitles.org/subtitle-%s.html", iter1[1].c_str()));
                    searchResult = Download(url, "", data2);
                    regex_results results2;
                    string_regex(regex_pattern[2], data2, results2);
                    for (const auto& iter2 : results2) {
                        subtitlesInfo.fileName = iter2[0];
                    }
                    subtitlesInfo.title = iter[2];
                    subtitlesInfo.year = iter[3].empty() ? -1 : atoi(iter[3].c_str());
                    subtitlesInfo.seasonNumber = fileInfo.seasonNumber;
                    subtitlesInfo.episodeNumber = fileInfo.episodeNumber;
                    subtitlesInfo.id = iter1[1];
                    subtitlesInfo.languageCode = iter1[0];
                    subtitlesInfo.languageName = UTF16To8(ISO639XToLanguage(subtitlesInfo.languageCode.c_str()));
                    subtitlesInfo.downloadCount = atoi(iter1[4].c_str());
                    subtitlesInfo.discNumber = atoi(iter1[5].c_str());
                    subtitlesInfo.discCount = atoi(iter1[5].c_str());
                    subtitlesInfo.url = url;
                    Set(fileInfo, subtitlesInfo, _bAborting);
                }
            }
        }
    }
    return searchResult;
}

SRESULT Moviesubtitles::Download(SubtitlesInfo& subtitlesInfo, volatile BOOL& _bAborting)
{
    return Download(string_format("http://www.moviesubtitles.org/download-%s.html", subtitlesInfo.id.c_str()), "", subtitlesInfo.fileContents);
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
    std::regex(),
};

SRESULT addic7ed::Login(std::string& _UserName, std::string& _Password)
{
    if (!_UserName.empty() && !_Password.empty()) {
        std::string url("http://www.addic7ed.com/dologin.php");
        string_map headers({
            { "User-Agent", UserAgent() },
            { "Referer", "http://www.addic7ed.com/login.php" },
            { "Content-Type", "application/x-www-form-urlencoded" },
        });

        std::string content, data;
        content += string_format("username=%s&password=%s&Submit=Log+in",
                                 UrlEncode(_UserName.c_str()), UrlEncode(_Password.c_str()));

        DWORD dwStatusCode = NULL;
        string_upload(url, headers, content, data, FALSE, &dwStatusCode);
        //'Success ':   (HTTP/1.1 302 Found): If everything was OK, the HTTP status code 302 will be returned.
        return (dwStatusCode == 302) ? SR_SUCCEEDED : SR_FAILED;

    }
    return SR_UNDEFINED;
}

SRESULT addic7ed::Search(const SubtitlesInfo& fileInfo, volatile BOOL& _bAborting)
{
    SRESULT searchResult = SR_UNDEFINED;
    if (fileInfo.seasonNumber != -1) {
        std::string data;
        std::string search(fileInfo.title);
        if (!fileInfo.country.empty()) { search += " " + fileInfo.country; }
        if (fileInfo.year != -1) { search += " (" + std::to_string(fileInfo.year) + ")"; }
        // remove ' and ' from string and replace '!?&':' with ' ' to get more accurate results
        search = std::regex_replace(search, std::regex(" and |[!?&':]", regex_flags), " ");
        searchResult = Download(string_format("http://www.addic7ed.com/search.php?search=%s", UrlEncode(search .c_str())), "", data);

        regex_results results;
        string_regex(regex_pattern[0], data, results);
        for (const auto& iter : results) {
            CheckAbortAndReturn();
            std::string data1;
            searchResult = Download(string_format("http://www.addic7ed.com/ajax_loadShow.php?show=%s&season=%d&langs=%s&hd=undefined&hi=undefined", iter[0].c_str(), fileInfo.seasonNumber, GetLanguagesString().c_str()), "", data1);

            regex_results results1;
            string_regex(std::regex(string_format("<tr class=\"epeven completed\"><td>%d</td><td>%d</td><td><a href=\"([^\"]+)\">(.*?)</a></td><td>(.*?)</td><td class=\"c\">(.*?)</td>[^<]*<td class=\"c\">Completed</td><td class=\"c\">(.*?)</td><td class=\"c\">(.*?)</td><td class=\"c\">(.*?)</td><td class=\"c\"><a href=\"(/updated/(\\d+)/\\d+/\\d+)\">Download</a></td>", fileInfo.seasonNumber, fileInfo.episodeNumber), regex_flags), data1, results1);
            for (const auto& iter1 : results1) {
                CheckAbortAndReturn();
                SubtitlesInfo subtitlesInfo;
                subtitlesInfo.fileName = string_format("%s - %dx%0*d - %s.%s.%s.%s%supdated.Addicted.com.srt", fileInfo.title.c_str(), fileInfo.seasonNumber, 2, fileInfo.episodeNumber, HtmlSpecialCharsDecode(iter1[1].c_str()), iter1[3].empty() ? "Undefined" : iter1[3].c_str(), iter1[2].c_str(), (iter1[4].empty() ? "" : "HI."), (iter1[5].empty() ? "" : "C."));
                subtitlesInfo.fileExtension = "srt";
                subtitlesInfo.id = "http://www.addic7ed.com" + iter1[7];
                subtitlesInfo.url = "http://www.addic7ed.com" + iter1[0];
                subtitlesInfo.languageCode = addic7ed_languages[atoi(iter1[8].c_str())].code;
                subtitlesInfo.languageName = iter1[2];
                subtitlesInfo.hearingImpaired = iter1[4].empty() ? FALSE : TRUE;
                subtitlesInfo.corrected = (iter1[5].empty()) ? FALSE : TRUE;
                subtitlesInfo.seasonNumber = fileInfo.seasonNumber;
                subtitlesInfo.episodeNumber = fileInfo.episodeNumber;
                subtitlesInfo.title = iter[1];
                subtitlesInfo.title2 = HtmlSpecialCharsDecode(iter1[1].c_str());
                subtitlesInfo.year = iter[2].empty() ? -1 : atoi(iter[2].c_str());
                subtitlesInfo.country = iter[3];
                subtitlesInfo.discNumber = 1;
                subtitlesInfo.discCount = 1;
                Set(fileInfo, subtitlesInfo, _bAborting);
            }
        }
    }
    return searchResult;
}

SRESULT addic7ed::Download(SubtitlesInfo& subtitlesInfo, volatile BOOL& _bAborting)
{
    return Download(subtitlesInfo.id, subtitlesInfo.url, subtitlesInfo.fileContents);
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

SRESULT podnapisi::Login(std::string& _UserName, std::string& _Password)
{
    //TODO: implement
    return SR_UNDEFINED;
}

// RESULTS ------------------------------------------------
// "/sXML/1/"  //Reply in XML format
// "/page//"   //Return nth page of results
// SEARCH -------------------------------------------------
// "/sT/1/"    //Type: -1=all, 0=movies, 1=series, don't specify for auto detection
// "/sAKA/1/"  //Include movie title aliases
// "/sM//"     //Movie id from www.omdb.si
// "/sK//"     //Title url encoded text
// "/sY//"     //Year number
// "/sTS//"    //Season number
// "/sTE//"    //Episode number
// "/sR//"     //Release name url encoded text
// "/sJ/0/"    //Languages, 0=all
// SEARCH ADDITIONAL --------------------------------------
// "/sFT/0/"   //Subtitles Format: 0=all, 1=MicroDVD, 2=SAMI, 3=SSA, 4=SubRip, 5=SubViewer 2.0, 6=SubViewer, 7=MPSub, 8=Advanced SSA, 9=DVDSubtitle, 10=TMPlayer, 11=MPlayer2
// "/sA/0/"    //Search subtitles by user id, 0=all
// "/sI//"     //Search subtitles by subtitle id
// SORTING ------------------------------------------------
// "/sS//"     //Sorting field: movie, year, fps, language, downloads, cds, username, time, rating
// "/sO//"     //Soring order: asc, desc
// FILTERS ------------------------------------------------
// "/sOE/1/"   //Subtitles for extended edition only
// "/sOD/1/"   //Subtitles suitable for DVD only
// "/sOH/1/"   //Subtitles for high-definition video only
// "/sOI/1/"   //Subtitles for hearing impaired only
// "/sOT/1/"   //Technically correct only
// "/sOL/1/"   //Grammatically correct only
// "/sOA/1/"   //Author subtitles only
// "/sOCS/1/"  //Only subtitles for a complete season
// UNKNOWN ------------------------------------------------
// "/sH//"     //Search subtitles by video file hash ??? (not working for me)

SRESULT podnapisi::Search(const SubtitlesInfo& fileInfo, volatile BOOL& _bAborting)
{
    SRESULT searchResult = SR_UNDEFINED;
    int page = 1, pages = 1;
    do {
        CheckAbortAndReturn();
        std::string data;
        std::string languages(GetLanguagesString());

        std::string search(fileInfo.title);
        if (!fileInfo.country.empty()) { search += " " + fileInfo.country; }
        search = std::regex_replace(search, std::regex(" and | *[!?&':] *", regex_flags), " ");

        std::string url("http://simple.podnapisi.net/en/ppodnapisi/search");
        url += "/sXML/1";
        url += "/sAKA/1";
        url += (!search.empty() ? "/sK/" + UrlEncode(search.c_str()) : "");
        url += (fileInfo.year != -1 ? "/sY/" + std::to_string(fileInfo.year) : "");
        url += (fileInfo.seasonNumber != -1 ? "/sTS/" + std::to_string(fileInfo.seasonNumber) : "");
        url += (fileInfo.episodeNumber != -1 ? "/sTE/" + std::to_string(fileInfo.episodeNumber) : "");
        //url += "/sR/" + UrlEncode(fileInfo.fileName.c_str());
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
            if (fileInfo.seasonNumber > 0 && fileInfo.episodeNumber <= 0 && atoi(iter1[12].c_str()) != 0) { continue; }
            CheckAbortAndReturn();
            SubtitlesInfo subtitlesInfo;
            subtitlesInfo.id = iter1[0];
            subtitlesInfo.title =  HtmlSpecialCharsDecode(iter1[1].c_str());
            subtitlesInfo.year = iter1[2].empty() ? -1 : atoi(iter1[2].c_str());
            subtitlesInfo.url = iter1[4];
            subtitlesInfo.fileExtension = iter1[15] == "SubRip" ? "srt" : iter1[15];

            string_array fileNames(string_tokenize(iter1[7], " "));
            if (fileNames.empty()) {
                fileNames.push_back(fileInfo.fileName + "." + subtitlesInfo.fileExtension);
            }
            subtitlesInfo.fileName = fileNames[0] + "." + subtitlesInfo.fileExtension;
            for (const auto& fileName : fileNames) {
                if (fileName == fileInfo.fileName) {
                    subtitlesInfo.fileName = fileName + "." + subtitlesInfo.fileExtension;
                }
            }
            subtitlesInfo.releaseName = iter1[7];
            subtitlesInfo.languageCode = podnapisi_languages[atoi(iter1[8].c_str())].code;
            subtitlesInfo.languageName = iter1[9];
            subtitlesInfo.seasonNumber = atoi(iter1[11].c_str());
            subtitlesInfo.episodeNumber = atoi(iter1[12].c_str());
            subtitlesInfo.discNumber = atoi(iter1[14].c_str());
            subtitlesInfo.discCount = atoi(iter1[14].c_str());
            subtitlesInfo.hearingImpaired = (iter1[18].find("n") != std::string::npos) ? TRUE : FALSE;
            subtitlesInfo.corrected = (iter1[18].find("r") != std::string::npos) ? -1 : 0;
            subtitlesInfo.downloadCount = atoi(iter1[19].c_str());
            Set(fileInfo, subtitlesInfo, _bAborting);
        }
    } while (page++ < pages);

    return searchResult;
}

SRESULT podnapisi::Download(SubtitlesInfo& subtitlesInfo, volatile BOOL& _bAborting)
{
    SRESULT searchResult = SR_UNDEFINED;
    std::string temp;
    searchResult = Download(subtitlesInfo.url, "", temp);

    regex_results results;
    string_regex(regex_pattern[2], temp, results);
    for (const auto& iter : results) {
        CheckAbortAndReturn();
        searchResult = Download("http://simple.podnapisi.net" + iter[0], "", subtitlesInfo.fileContents);
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
