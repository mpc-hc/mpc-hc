/*
 * (C) 2013-2014 see Authors.txt
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

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "EventDispatcher.h"

struct Shader {
    Shader();
    Shader(const CString& path);
    CString filePath;
    bool operator==(const Shader& rhs) const;
    bool IsDefault() const;
    CStringA GetCode() const;
};

class ShaderList : public std::vector<Shader>
{
public:
    ShaderList();
    ShaderList(const CString& src);
    CString ToString() const;
    static CString GetShadersDir();
    static ShaderList GetDefaultShaders();
};

class FileChangeNotifier
{
public:
    FileChangeNotifier(bool bLockDirs = false);
    virtual ~FileChangeNotifier();
    typedef std::set<CString, CStringUtils::IgnoreCaseLess> FileSet;

protected:
    virtual FileSet GetWatchedList() = 0;
    virtual void WatchedFilesChanged(const FileSet& changes) = 0;
    void UpdateNotifierState();

private:
    static void __stdcall NotifierWinapiCallback(DWORD, DWORD, LPOVERLAPPED lpOverlapped);

    struct StateItem {
        CString path;         // watched dir
        FileSet filter;       // watched files in that dir
        HANDLE hDir;          // watched dir handle
        static const DWORD bufferByteSize = 4096 * sizeof(DWORD); // ReadDirectoryChangesW buffer size
        DWORD buffer[bufferByteSize / sizeof(DWORD)];             // ReadDirectoryChangesW buffer, must be aligned to DWORD
        OVERLAPPED overlapped;                                    // ReadDirectoryChangesW overlapped struct
        StateItem();
        ~StateItem();
        std::shared_ptr<StateItem> pApcThis; // we don't want this struct to be destroyed before it gets to NotifierWinapiCallback()
        FileChangeNotifier* pOwner;          // and we need to return control from that callback to FileChangeNotifier class

        StateItem(const StateItem&) = delete;
        StateItem& operator=(const StateItem&) = delete;
    };

    const bool m_bLockDirs;
    static bool ArmStateItem(std::shared_ptr<StateItem> item);
    std::list<std::shared_ptr<StateItem>> m_State;

    FileChangeNotifier(const FileChangeNotifier&) = delete;
    FileChangeNotifier& operator=(const FileChangeNotifier&) = delete;
};

class ShaderPreset
{
public:
    ShaderPreset();
    ShaderPreset(const CString& srcPre, const CString& srcPost);
    bool operator==(const ShaderPreset& rhs) const;
    const ShaderList& GetPreResize() const;
    const ShaderList& GetPostResize() const;
    void SetLists(const ShaderList& preResize, const ShaderList& postResize);
    void ToStrings(CString& outPre, CString& outPost) const;

protected:
    ShaderList m_PreResize, m_PostResize;
};

class ShaderSelection
{
public:
    typedef std::map<CString, ShaderPreset> ShaderPresetMap;

protected:
    class ShaderCurrentPreset : public ShaderPreset, protected FileChangeNotifier
    {
    public:
        ShaderCurrentPreset();
        ~ShaderCurrentPreset();
        virtual void SetLists(const ShaderList& preResize, const ShaderList& postResize);
    protected:
        virtual FileSet GetWatchedList() override;
        virtual void WatchedFilesChanged(const FileSet& changes) override;
        void WatchedFilesCooldownCallback();
        FileSet m_changes;
        EventClient m_eventc;
    };

    CString m_strCurrentPresetName;
    ShaderPresetMap m_presets;
    ShaderCurrentPreset m_currentSelection;

public:
    bool NextPreset(bool bWrap = true);
    bool PrevPreset(bool bWrap = true);
    bool GetCurrentPresetName(CString& out) const;
    const ShaderPresetMap& GetPresets() const;
    void SetPresets(const ShaderPresetMap& presets);
    const ShaderPreset& GetCurrentPreset() const;
    void SetCurrentPreset(const ShaderPreset& preset);
    bool SetCurrentPreset(const CString& name);
};
