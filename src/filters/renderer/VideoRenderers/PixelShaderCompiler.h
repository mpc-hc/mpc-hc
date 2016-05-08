/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2016 see Authors.txt
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

#include "PixelShaderCache.h"
#include <D3Dcompiler.h>

class CPixelShaderCompiler
{
    HINSTANCE m_hDll;

    pD3DCompile m_pD3DCompile;
    pD3DDisassemble m_pD3DDisassemble;

    CComPtr<IDirect3DDevice9> m_pD3DDev;

    CPixelShaderCache m_Cache;

    HRESULT InternalCompile(
        LPCSTR pSrcData,
        SIZE_T SrcDataSize,
        LPCSTR pSourceName,
        LPCSTR pEntrypoint,
        LPCSTR pProfile,
        DWORD Flags,
        IDirect3DPixelShader9** ppPixelShader,
        CString* pDisasm,
        CString* pErrMsg);

public:
    CPixelShaderCompiler(IDirect3DDevice9* pD3DDev, bool fStaySilent = false);
    ~CPixelShaderCompiler();

    HRESULT CompileShader(
        LPCSTR pSrcData,
        LPCSTR pEntrypoint,
        LPCSTR pProfile,
        DWORD Flags,
        IDirect3DPixelShader9** ppPixelShader,
        CString* pDisasm = nullptr,
        CString* pErrMsg = nullptr);

    HRESULT CompileShaderFromFile(
        LPCTSTR pSrcFile,
        LPCSTR pEntrypoint,
        LPCSTR pProfile,
        DWORD Flags,
        IDirect3DPixelShader9** ppPixelShader,
        CString* pDisasm = nullptr,
        CString* pErrMsg = nullptr);
};
