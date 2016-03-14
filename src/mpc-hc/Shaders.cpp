/*
 * (C) 2013-2016 see Authors.txt
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
#include "Shaders.h"
#include "MainFrm.h"
#include "mplayerc.h"
#include "PathUtils.h"
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/error/en.h"
#include <sstream>
#include <iostream>

#define SHADER_MAX_FILE_SIZE (4 * 1024 * 1024)

Shader::Shader(const CString& path)
    : m_FilePath(path)
{
    Reload();
}

bool Shader::operator==(const Shader& rhs) const
{
    return m_FilePath.CompareNoCase(rhs.m_FilePath) == 0;
}

bool Shader::IsDefault() const
{
    ASSERT(!PathUtils::IsRelative(m_FilePath));
    ASSERT(!PathUtils::IsRelative(ShaderList::GetShadersDir()));
    return PathUtils::IsInDir(m_FilePath, ShaderList::GetShadersDir());
}

bool Shader::IsUsing(const CString& filePath) const
{
    for (const CString& path : m_Pathes) {
        if (path.CompareNoCase(filePath) == 0) {
            return true;
        }
    }

    return false;
}

const CString& Shader::GetFilePath() const
{
    return m_FilePath;
}

const CStringA Shader::GetCode() const
{
    return m_Code.c_str();
}

const std::vector<CString>& Shader::GetPathes() const
{
    return m_Pathes;
}

const std::vector<ShaderTexture>& Shader::GetTextures() const
{
    return m_Textures;
}

const std::vector<ShaderParameter>& Shader::GetParameters() const
{
    return m_Parameters;
}

void Shader::Reload()
{
    m_Code.clear();
    m_Pathes.clear();
    m_Textures.clear();
    m_Parameters.clear();

    Load(m_FilePath);
}

bool Shader::Load(const CString& path)
{
    const std::string include_token = "#include";
    const std::string filePath = CT2CA(path);

    m_Pathes.push_back(path);
    CFile file;
    if (!file.Open(path, CFile::modeRead | CFile::typeBinary)) {
        Log("file is missing : " + filePath);
        return false;
    }

    if (file.GetLength() + m_Code.size() >= SHADER_MAX_FILE_SIZE) {
        Log("file too large : " + filePath);
        return false;
    }

    std::string content;
    content.resize(file.GetLength());
    if (file.Read(&content[0], file.GetLength()) != file.GetLength()) {
        Log("failed to read : " + filePath);
        return false;
    }

    if (!StripComments(content)) {
        Log("invalid comments : " + filePath);
        return false;
    }

    m_Code.reserve(m_Code.size() + content.size());
    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
        size_t pos = line.find(include_token);
        if (pos != std::string::npos) {
            size_t start = line.find('\"', pos + include_token.size());
            size_t stop;
            if (start == std::string::npos) {
                start = line.find('<', pos + include_token.size());
                stop = line.find('>', pos + include_token.size());
            } else {
                stop = line.find('\"', start + 1);
            }

            if (start == std::string::npos || stop == std::string::npos) {
                Log("invalid include : " + line);
                return false;
            }

            CString includePath = PathUtils::CombinePaths(PathUtils::DirName(path), CString(line.substr(start + 1, stop - start - 1).c_str()));

            if (!Load(includePath)) {
                return false;
            }
        } else {
            m_Code += line;
        }
        m_Code += '\n';
    }
    return LoadConfig(path);
}

bool Shader::LoadConfig(const CString& path)
{
    CString folder = PathUtils::DirName(path);
    CString configPath = PathUtils::CombinePaths(folder, PathUtils::FileName(path) + SHADERS_CFG);
    if (!PathUtils::IsFile(configPath)) {
        return true;
    }
    const std::string filePath = CT2CA(configPath);

    m_Pathes.push_back(configPath);
    CFile file;
    if (!file.Open(configPath, CFile::modeRead | CFile::osSequentialScan | CFile::typeBinary) || file.GetLength() > SHADER_MAX_FILE_SIZE) {
        Log("failed to access : " + filePath);
        return false;
    }

    std::string content;
    content.resize(file.GetLength());
    if (file.Read(&content[0], file.GetLength()) != file.GetLength()) {
        Log("failed to read : " + filePath);
        return false;
    }

    if (!StripComments(content)) {
        Log("invalid comments : " + filePath);
        return false;
    }

    rapidjson::Document document;
    if (document.Parse(content.c_str()).HasParseError()) {
        Log(filePath + " : " + rapidjson::GetParseError_En(document.GetParseError()));
        return false;
    }

    for (rapidjson::Value::ConstMemberIterator member = document.MemberBegin(); member != document.MemberEnd(); ++member) {
        if (member->name == "texture") {
            rapidjson::Value::ConstMemberIterator id = member->value.FindMember("id");
            rapidjson::Value::ConstMemberIterator path = member->value.FindMember("path");
            if (path != member->value.MemberEnd() && id != member->value.MemberEnd()) {
                ShaderTexture texture;
                texture.id = id->value.GetInt();
                texture.path = path->value.GetString();
                texture.path.Replace(_T("/"), _T("\\"));

                if (PathUtils::IsFile(texture.path)) {
                    m_Pathes.push_back(texture.path);
                } else {
                    CString texturePath = PathUtils::CombinePaths(folder, texture.path);
                    if (PathUtils::IsFile(CString(texturePath))) {
                        texture.path = texturePath;
                        m_Pathes.push_back(texture.path);
                    } else {
                        Log(std::string(CT2CA(texture.path)) + " not found");
                    }
                }

                texture.filter = 3;
                texture.wrap = 0;

                rapidjson::Value::ConstMemberIterator filter = member->value.FindMember("filter");
                if (filter != member->value.MemberEnd()) {
                    texture.filter = filter->value.GetInt();
                }

                rapidjson::Value::ConstMemberIterator wrap = member->value.FindMember("wrap");
                if (wrap != member->value.MemberEnd()) {
                    texture.wrap = filter->value.GetInt();
                }

                m_Textures.push_back(texture);
            }
        } else if (member->name == "float4") {
            rapidjson::Value::ConstMemberIterator values = member->value.FindMember("values");
            rapidjson::Value::ConstMemberIterator id = member->value.FindMember("id");
            if (values != member->value.MemberEnd() && values->value.IsArray() && values->value.Size() == 4 && id != member->value.MemberEnd()) {
                ShaderParameter param;
                param.id = id->value.GetInt();
                memset(param.values, 0, sizeof(float) * 4);

                for (rapidjson::SizeType comp = 0; comp < values->value.Size(); ++comp) {
                    param.values[comp] = (float)values->value[comp].GetDouble();
                }

                m_Parameters.push_back(param);
            }
        }
    }

    return true;
}

bool Shader::StripComments(std::string& code) const
{
    size_t pos;
    while ((pos = code.find("/*")) != std::string::npos) {
        size_t end = code.find("*/", pos);
        if (end == -1) {
            return false;
        }
        code.erase(pos, (end - pos) + 2);
    }
    while ((pos = code.find("//")) != std::string::npos) {
        size_t end = code.find('\n', pos);
        if (end != std::string::npos) {
            code.erase(pos, end - pos);
        } else {
            code.erase(pos);
        }
    }
    return true;
}

void Shader::Log(std::string message)
{
    m_Code += "#error " + message + "\n";
}

ShaderList::ShaderList()
{
}

ShaderList::ShaderList(const CString& src)
{
    CString tok, dir = GetShadersDir();
    int pos = 0;
    do {
        tok = src.Tokenize(_T(";"), pos);
        if (!tok.IsEmpty()) {
            if (PathUtils::IsRelative(tok)) {
                // path is relative, convert it to absolute
                tok = PathUtils::CombinePaths(dir, tok);
            }
            push_back(Shader(tok));
        }
    } while (pos != -1);
}

CString ShaderList::ToString() const
{
    CString ret, tok, dir = GetShadersDir();
    for (auto it = cbegin(); it != cend(); ++it) {
        tok = it->GetFilePath();
        // convert path to relative when possible
        if (PathUtils::IsInDir(tok, dir)) {
            bool rel;
            tok = PathUtils::ToRelative(dir, tok, &rel);
            ASSERT(rel);
        }
        // append separator
        if (!ret.IsEmpty()) {
            ret.AppendChar(_T(';'));
        }
        ret.Append(tok);
    }
    return ret;
}

CString ShaderList::GetShadersDir()
{
    return PathUtils::CombinePaths(PathUtils::GetProgramPath(false), SHADERS_DIR);
}

ShaderList ShaderList::GetDefaultShaders()
{
    ShaderList ret;
    ASSERT(CString(SHADERS_EXT).Left(1) == _T('.'));
    const CString mask = _T("*.*");
    std::set<CString, CStringUtils::LogicalLess> dirs, files;
    dirs.insert(PathUtils::CombinePaths(ShaderList::GetShadersDir(), mask));
    CFileFind finder;
    while (!dirs.empty()) {
        auto it = dirs.cbegin();
        BOOL next = finder.FindFile(*it);
        while (next) {
            next = finder.FindNextFile();
            CString path = finder.GetFilePath();
            if (!finder.IsDots()) {
                if (finder.IsDirectory()) {
                    dirs.insert(PathUtils::CombinePaths(path, mask));
                } else if (PathUtils::FileExt(path).CompareNoCase(SHADERS_EXT) == 0) {
                    files.insert(path);
                }
            }
        }
        for (const auto& file : files) {
            ret.emplace_back(file);
        }
        files.clear();
        dirs.erase(it);
    }
    return ret;
}

FileChangeNotifier::FileChangeNotifier(bool bLockDirs/* = false */)
    : m_bLockDirs(bLockDirs)
{
}

FileChangeNotifier::~FileChangeNotifier()
{
    for (auto& item : m_State) {
        VERIFY(CancelIo(item->hDir));
        item->pOwner = nullptr;
    }
}

void FileChangeNotifier::UpdateNotifierState()
{
    // generate new filters
    std::map<CString, FileSet, CStringUtils::IgnoreCaseLess> newFilter;
    for (const auto& file : GetWatchedList()) {
        newFilter[PathUtils::DirName(file)].insert(PathUtils::BaseName(file));
    }
    // reuse as much as we can from the old state
    for (auto it1 = m_State.begin(); it1 != m_State.end();) {
        // check whether the old directory is present in the new filters
        auto it2 = newFilter.find((*it1)->path);
        if (it2 != newFilter.end() && (*it1)->pApcThis) {
            // reuse directory handle, but update its file filter
            (*it1)->filter.swap(it2->second);
            newFilter.erase(it2);
        } else {
            // such directory is not present in the new filters, so we drop it
            (*it1)->pOwner = nullptr;
            VERIFY(CancelIo((*it1)->hDir));
            it1 = m_State.erase(it1);
            continue;
        }
        ++it1;
    }
    // add directories that are not present in the old state
    for (auto& pair : newFilter) {
        auto pItem = std::make_shared<StateItem>();
        pItem->path = pair.first;
        pItem->filter.swap(pair.second);
        pItem->hDir = CreateFile(pItem->path, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE |
                                 (m_bLockDirs ? 0 : FILE_SHARE_DELETE), nullptr, OPEN_EXISTING,
                                 FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
        if (pItem->hDir != INVALID_HANDLE_VALUE && ArmStateItem(pItem)) {
            pItem->pOwner = this;
            m_State.push_back(std::move(pItem));
        } else {
            ASSERT(!PathUtils::Exists(pItem->path));
        }
    }
}

void __stdcall FileChangeNotifier::NotifierWinapiCallback(DWORD, DWORD, LPOVERLAPPED lpOverlapped)
{
    auto item = static_cast<StateItem*>(lpOverlapped->hEvent)->pApcThis;
    item->pApcThis = nullptr;
    if (item->pOwner) {
        FileSet changes;
        DWORD dwOut;
        // get notifications
        if (GetOverlappedResult(item->hDir, &item->overlapped, &dwOut, FALSE)) {
            auto buffer = (BYTE*)item->buffer;
            PFILE_NOTIFY_INFORMATION pInfo;
            size_t offset = 0;
            // loop through all notifications
            do {
                pInfo = (PFILE_NOTIFY_INFORMATION)(buffer + offset);
                // filter out notifications by type
                switch (pInfo->Action) {
                    case FILE_ACTION_MODIFIED:
                    case FILE_ACTION_RENAMED_NEW_NAME:
                        CStringW fileName;
                        DWORD fileNameLength = pInfo->FileNameLength / sizeof(WCHAR);
                        VERIFY(0 == wmemcpy_s(fileName.GetBuffer(fileNameLength), fileNameLength,
                                              pInfo->FileName, fileNameLength));
                        fileName.ReleaseBuffer(fileNameLength);
                        // filter out notifications by filename
                        if (item->filter.find(fileName) != item->filter.end()) {
                            changes.insert(PathUtils::CombinePaths(item->path, fileName));
                        }
                }
                offset += pInfo->NextEntryOffset;
            } while (pInfo->NextEntryOffset);
        } else if (GetLastError() == ERROR_MORE_DATA) {
            // notification buffer was overflown, we assume all files as modified
            changes.insert(item->filter.cbegin(), item->filter.cend());
        } else {
            return;
        }
        // rearm the notifier
        VERIFY(ArmStateItem(item));
        if (!changes.empty()) {
            item->pOwner->WatchedFilesChanged(changes);
        }
    }
}

FileChangeNotifier::StateItem::StateItem()
    : hDir(nullptr)
    , pOwner(nullptr)
{
    ZeroMemory(&overlapped, sizeof(overlapped));
    ZeroMemory(&buffer, sizeof(buffer));
}

FileChangeNotifier::StateItem::~StateItem()
{
    SAFE_CLOSE_HANDLE(hDir);
}

bool FileChangeNotifier::ArmStateItem(std::shared_ptr<StateItem> item)
{
    bool ret = false;
    if (ReadDirectoryChangesW(item->hDir, item->buffer, item->bufferByteSize, FALSE,
                              FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
                              nullptr, &item->overlapped, NotifierWinapiCallback)) {
        item->pApcThis = item;
        item->overlapped.hEvent = item->pApcThis.get();
        ret = true;
    }
    return ret;
}

ShaderPreset::ShaderPreset()
{
}

ShaderPreset::ShaderPreset(const CString& srcPre, const CString& srcPost)
    : m_PreResize(srcPre)
    , m_PostResize(srcPost)
{
}

const ShaderList& ShaderPreset::GetPreResize() const
{
    return m_PreResize;
}

const ShaderList& ShaderPreset::GetPostResize() const
{
    return m_PostResize;
}

void ShaderPreset::SetLists(const ShaderList& preResize, const ShaderList& postResize)
{
    if (&preResize != &m_PreResize) {
        m_PreResize = preResize;
    }
    if (&postResize != &m_PostResize) {
        m_PostResize = postResize;
    }
}

void ShaderPreset::ToStrings(CString& outPre, CString& outPost) const
{
    outPre = m_PreResize.ToString();
    outPost = m_PostResize.ToString();
}

bool ShaderPreset::operator==(const ShaderPreset& rhs) const
{
    return (m_PreResize == rhs.m_PreResize) && (m_PostResize == rhs.m_PostResize);
}

ShaderSelection::ShaderCurrentPreset::ShaderCurrentPreset()
{
    EventRouter::EventSelection fires;
    fires.insert(MpcEvent::SHADER_PRERESIZE_SELECTION_CHANGED);
    fires.insert(MpcEvent::SHADER_POSTRESIZE_SELECTION_CHANGED);
    fires.insert(MpcEvent::SHADER_SELECTION_CHANGED);
    GetEventd().Connect(m_eventc, fires);
}

ShaderSelection::ShaderCurrentPreset::~ShaderCurrentPreset()
{
    if (auto pMainFrame = AfxGetMainFrame()) {
        pMainFrame->m_timerOneTime.Unsubscribe(CMainFrame::TimerOneTimeSubscriber::ACTIVE_SHADER_FILES_CHANGE_COOLDOWN);
    }
}

void ShaderSelection::ShaderCurrentPreset::SetLists(const ShaderList& preResize, const ShaderList& postResize)
{
    ShaderPreset::SetLists(preResize, postResize);
    UpdateNotifierState();
    m_eventc.FireEvent(MpcEvent::SHADER_SELECTION_CHANGED);
}

FileChangeNotifier::FileSet ShaderSelection::ShaderCurrentPreset::GetWatchedList()
{
    FileSet ret;
    for (const auto& shader : m_PreResize) {
        for (const CString& path : shader.GetPathes()) {
            ret.emplace(path);
        }
    }
    for (const auto& shader : m_PostResize) {
        for (const CString& path : shader.GetPathes()) {
            ret.emplace(path);
        }
    }
    return ret;
}

void ShaderSelection::ShaderCurrentPreset::WatchedFilesChanged(const FileSet& changes)
{
    if (!changes.empty()) {
        if (auto pMainFrame = AfxGetMainFrame()) {
            m_changes.insert(changes.begin(), changes.end());
            pMainFrame->m_timerOneTime.Subscribe(CMainFrame::TimerOneTimeSubscriber::ACTIVE_SHADER_FILES_CHANGE_COOLDOWN,
                                                 std::bind(&ShaderSelection::ShaderCurrentPreset::WatchedFilesCooldownCallback, this), 50);
        } else {
            ASSERT(FALSE);
        }
    } else {
        ASSERT(FALSE);
    }
}

void ShaderSelection::ShaderCurrentPreset::WatchedFilesCooldownCallback()
{
    bool setPre = CheckWatchedFiles(m_PreResize);
    bool setPost = CheckWatchedFiles(m_PostResize);

    if (setPre && setPost) {
        m_eventc.FireEvent(MpcEvent::SHADER_SELECTION_CHANGED);
    } else if (setPre) {
        m_eventc.FireEvent(MpcEvent::SHADER_PRERESIZE_SELECTION_CHANGED);
    } else if (setPost) {
        m_eventc.FireEvent(MpcEvent::SHADER_POSTRESIZE_SELECTION_CHANGED);
    }
    m_changes.clear();

    if (setPre || setPost) {
        UpdateNotifierState();
    }
}

bool ShaderSelection::ShaderCurrentPreset::CheckWatchedFiles(ShaderList& shaders)
{
    bool ret = false;
    for (Shader& shader : shaders) {
        bool modified = false;
        for (const CString& change : m_changes) {
            if (shader.IsUsing(change)) {
                modified = true;
                break;
            }
        }

        if (modified) {
            ret = true;
            shader.Reload();
        }
    }
    return ret;
}

bool ShaderSelection::NextPreset(bool bWrap/* = true*/)
{
    bool ret = false;
    if (m_strCurrentPresetName.IsEmpty()) {
        auto it = m_presets.cbegin();
        if (it != m_presets.cend()) {
            VERIFY(SetCurrentPreset(it->first));
            ret = true;
        }
    } else {
        auto it = m_presets.find(m_strCurrentPresetName);
        if (it != m_presets.end()) {
            ++it;
            if (it == m_presets.end() && bWrap) {
                it = m_presets.begin();
            }
            if (it != m_presets.end()) {
                VERIFY(SetCurrentPreset(it->first));
                ret = true;
            }
        } else {
            ASSERT(FALSE);
        }
    }
    return ret;
}

bool ShaderSelection::PrevPreset(bool bWrap/* = true*/)
{
    bool ret = false;
    if (m_strCurrentPresetName.IsEmpty()) {
        auto it = m_presets.cbegin();
        if (it != m_presets.cend()) {
            VERIFY(SetCurrentPreset(it->first));
            ret = true;
        }
    } else {
        auto it = m_presets.find(m_strCurrentPresetName);
        if (it != m_presets.end()) {
            if (it == m_presets.begin() && bWrap) {
                it-- = m_presets.end();
            }
            if (it != m_presets.begin()) {
                VERIFY(SetCurrentPreset((--it)->first));
                ret = true;
            }
        } else {
            ASSERT(FALSE);
        }
    }
    return ret;
}

bool ShaderSelection::GetCurrentPresetName(CString& out) const
{
    out = m_strCurrentPresetName;
    return !out.IsEmpty();
}

const ShaderSelection::ShaderPresetMap& ShaderSelection::GetPresets() const
{
    return m_presets;
}

void ShaderSelection::SetPresets(const ShaderSelection::ShaderPresetMap& presets)
{
    m_presets = presets;
    if (!m_strCurrentPresetName.IsEmpty()) {
        auto it = m_presets.find(m_strCurrentPresetName);
        if (it == m_presets.end() || !(it->second == m_currentSelection)) {
            m_strCurrentPresetName.Empty();
        }
    }
    if (m_strCurrentPresetName.IsEmpty()) {
        for (const auto& pair : m_presets) {
            if (pair.second == m_currentSelection) {
                m_strCurrentPresetName = pair.first;
                break;
            }
        }
    }
}

const ShaderPreset& ShaderSelection::GetCurrentPreset() const
{
    return m_currentSelection;
}

void ShaderSelection::SetCurrentPreset(const ShaderPreset& preset)
{
    m_strCurrentPresetName.Empty();
    m_currentSelection.SetLists(preset.GetPreResize(), preset.GetPostResize());
}

bool ShaderSelection::SetCurrentPreset(const CString& name)
{
    bool ret = false;
    auto it = m_presets.find(name);
    if (it != m_presets.end()) {
        ret = true;
        SetCurrentPreset(it->second);
        m_strCurrentPresetName = it->first;
    }
    return ret;
}
