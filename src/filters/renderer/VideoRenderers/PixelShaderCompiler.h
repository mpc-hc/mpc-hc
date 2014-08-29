/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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

#include <vector>
#include <d3d9.h>
#include <D3Dcompiler.h>

class CShaderMacros;

class CPixelShaderCompiler
{
    HINSTANCE m_hDll;

    pD3DCompile m_pD3DCompile;
    pD3DPreprocess m_pD3DPreprocess;
    pD3DDisassemble m_pD3DDisassemble;

    CComPtr<IDirect3DDevice9> m_pD3DDev;

    CPath m_SystemWideIncludesPath;

    HRESULT InternalCompile(
        LPCSTR pSrcData,
        SIZE_T SrcDataSize,
        LPCSTR pSourceName,
        LPCSTR pEntrypoint,
        LPCSTR pProfile,
        DWORD Flags,
        IDirect3DPixelShader9** ppPixelShader,
        CString* pDisasm,
        CString* pErrMsg,
        std::vector<CString>* pIncludedFiles);

public:
    CPixelShaderCompiler(IDirect3DDevice9* pD3DDev, bool fStaySilent = false);
    ~CPixelShaderCompiler();

    void SetSystemWideIncludesPath(const CPath& pPath);

    HRESULT CompileShader(
        LPCSTR pSrcData,
        LPCSTR pEntrypoint,
        LPCSTR pProfile,
        DWORD Flags,
        IDirect3DPixelShader9** ppPixelShader,
        CString* pDisasm = nullptr,
        CString* pErrMsg = nullptr,
        std::vector<CString>* pIncludedFiles = nullptr,
        CString* pSrcFileName = nullptr);

    HRESULT PreprocessShaderFromFile(
        LPCTSTR pSrcFile,
        LPCSTR pProfile,
        IDirect3DPixelShader9** ppPixelShader,
        CString* pFullCode = nullptr,
        CString* pErrMsg = nullptr,
        std::vector<CString>* pIncludedFiles = nullptr);

    HRESULT CompileShaderFromFile(
        LPCTSTR pSrcFile,
        LPCSTR pEntrypoint,
        LPCSTR pProfile,
        DWORD Flags,
        IDirect3DPixelShader9** ppPixelShader,
        CString* pDisasm = nullptr,
        CString* pErrMsg = nullptr,
        std::vector<CString>* pIncludedFiles = nullptr);

private:
    CStringA FindShaderProfile(LPCSTR pProfile) const;
    HRESULT GetShaderProfileMacro(
        LPCSTR pProfile,
        CShaderMacros& pMacros) const;

    CStringA GetStringFromBlob(ID3DBlob* pBlob) const;
    HRESULT ReadSourceFile(LPCTSTR pSrcFile, std::vector<char>& pOut) const;
};

// Class designed to facilitate the definition of shaders macros.
// Returns a D3D compliant array through a temporary class that handles the alloc/release.
class CShaderMacros : public std::vector<std::pair<CStringA, CStringA>>
{
public:
    class Wrapper
    {
        friend CShaderMacros;
    public:
        ~Wrapper();
        operator D3D_SHADER_MACRO* ();
    private:
        D3D_SHADER_MACRO* data;
        Wrapper(CShaderMacros& pSrc);
        Wrapper(Wrapper&& pFrom);
        Wrapper(const Wrapper&) = delete;

        Wrapper& operator=(Wrapper && pFrom);
        Wrapper& operator=(const Wrapper&) = delete;
    };

    void AddMacro(const CStringA& pName, const CStringA& pDefinition);
    Wrapper GetD3DShaderMacro();
};
