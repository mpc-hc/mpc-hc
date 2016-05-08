/*
* (C) 2016 see Authors.txt
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

#include <cstdint>

interface IDirect3DDevice9;
interface IDirect3DPixelShader9;

class CPixelShaderCache
{
    const unsigned int m_Version = 1;
    const LONG m_CachedDaysLimit = 30;
    const CString m_Extension = _T(".cso");

    CComPtr<IDirect3DDevice9> m_pD3DDev;

    CMap<uint64_t, uint64_t, CAutoVectorPtr<DWORD>, CAutoVectorPtr<DWORD>> m_PixelShaders;

public:
    CPixelShaderCache(IDirect3DDevice9* pD3DDev);

    HRESULT CreatePixelShader(
        LPCSTR pProfile,
        LPCSTR pSourceData,
        SIZE_T SourceDataSize,
        IDirect3DPixelShader9** ppPixelShader);

    void SavePixelShader(
        LPCSTR pProfile,
        LPCSTR pSourceData,
        SIZE_T SourceDataSize,
        void* pCompiledData,
        SIZE_T CompiledDataSize);

private:
    void LoadCache();
    static void DestroyCache();
    static bool IsEnabled();
    void TouchFile(const CString& FileName) const;
    bool IsFileOutdated(const CString& FileName) const;
    bool LoadCache(const CString& FileName, const CString& FilePath);
    void DeleteCache(uint64_t Hash);
    bool GetCacheFilePath(CString& CacheFilePath, uint64_t Hash) const;
    static bool GetCacheFolder(CString& CacheFolder);

    uint64_t Hash(LPCSTR pProfile, LPCSTR pSourceData, SIZE_T SourceDataSize) const;
    uint64_t Hash(LPCSTR pStr) const;
};
