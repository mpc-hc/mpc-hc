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

#include "stdafx.h"
#include "PixelShaderCompiler.h"
#include "ShaderIncludeHandler.h"
#include "../../../mpc-hc/resource.h"
#include <share.h>
#include <fstream>

/******************************************************************************/
void CPixelShaderCompiler::SetSystemWideIncludesPath(const CPath& pPath)
{
    m_SystemWideIncludesPath = pPath;
}

/******************************************************************************/
CPixelShaderCompiler::CPixelShaderCompiler(IDirect3DDevice9* pD3DDev, bool fStaySilent)
    : m_hDll(nullptr)
    , m_pD3DDev(pD3DDev)
    , m_pD3DCompile(nullptr)
    , m_pD3DPreprocess(nullptr)
    , m_pD3DDisassemble(nullptr)
    , m_SystemWideIncludesPath()
{
    m_hDll = LoadLibrary(D3DCOMPILER_DLL);

    if (m_hDll) {
        m_pD3DCompile = (pD3DCompile)GetProcAddress(m_hDll, "D3DCompile");
        m_pD3DPreprocess = (pD3DPreprocess)GetProcAddress(m_hDll, "D3DPreprocess");
        m_pD3DDisassemble = (pD3DDisassemble)GetProcAddress(m_hDll, "D3DDisassemble");
    }

    if (!fStaySilent) {
        if (!m_hDll) {
            CString msg;
            msg.Format(IDS_SHADER_DLL_ERR_0, D3DCOMPILER_DLL);
            AfxMessageBox(msg, MB_ICONWARNING | MB_OK);
		} else if (!m_pD3DCompile || !m_pD3DDisassemble)
		{
            CString msg;
            msg.Format(IDS_SHADER_DLL_ERR_1, D3DCOMPILER_DLL);
            AfxMessageBox(msg, MB_ICONWARNING | MB_OK);
        }
    }
}

/******************************************************************************/
CPixelShaderCompiler::~CPixelShaderCompiler()
{
    if (m_hDll) {
        VERIFY(FreeLibrary(m_hDll));
    }
}

/******************************************************************************/
CStringA CPixelShaderCompiler::FindShaderProfile(LPCSTR pProfile) const
{
    LPCSTR pSelProfile = pProfile;
    if (!pSelProfile || *pSelProfile == '\0') {
        D3DCAPS9 caps;
        if (m_pD3DDev && m_pD3DDev->GetDeviceCaps(&caps) == D3D_OK) {
            switch (D3DSHADER_VERSION_MAJOR(caps.PixelShaderVersion)) {
                case 2:
                    if (caps.PS20Caps.NumInstructionSlots < 512) {
                        pSelProfile = "ps_2_0";
                    } else if (caps.PS20Caps.Caps > 0) {
                        pSelProfile = "ps_2_a";
                    } else {
                        pSelProfile = "ps_2_b";
                    }
                    break;
                case 3:
                    pSelProfile = "ps_3_0";
                    break;
            }
        } else {
            ASSERT(FALSE);
        }
    }

    return CStringA(pSelProfile);
}

/******************************************************************************/
HRESULT CPixelShaderCompiler::GetShaderProfileMacro(
    LPCSTR pProfile,
    CShaderMacros& pMacros) const
{

    if (!pProfile || *pProfile == '\0') {
        return E_FAIL;
    }

    LPCSTR defProfile = "MPC_HC_SHADER_PROFILE";
    LPCSTR defProfileVal;
    if (!strcmp(pProfile, "ps_2_0")) {
        defProfileVal = "0";
    } else if (!strcmp(pProfile, "ps_2_b")) {
        defProfileVal = "1";
    } else if (!strcmp(pProfile, "ps_2_a") || !strcmp(pProfile, "ps_2_sw")) {
        defProfileVal = "2";
    } else if (!strcmp(pProfile, "ps_3_0") || !strcmp(pProfile, "ps_3_sw")) {
        defProfileVal = "3";
    } else {
        defProfileVal = "-1";
    }

    pMacros.AddMacro(defProfile, defProfileVal);

    return S_OK;
}

/******************************************************************************/
HRESULT CPixelShaderCompiler::InternalCompile(
    LPCSTR pSrcData,
    SIZE_T SrcDataSize,
    LPCSTR pSourceName,
    LPCSTR pEntrypoint,
    LPCSTR pProfile,
    DWORD Flags,
    IDirect3DPixelShader9** ppPixelShader,
    CString* pDisasm,
    CString* pErrMsg,
    std::vector<CString>* pIncludedFiles)
{
    // Basic checks
    if (!m_pD3DCompile) {
        return E_FAIL;
    }

    if (pDisasm && !m_pD3DDisassemble) {
        return E_FAIL;
    }

    if (ppPixelShader && !m_pD3DDev) {
        return E_FAIL;
    }

    // Gather the macros
    CShaderMacros lMacros;
    CStringA lProfile = FindShaderProfile(pProfile);
    GetShaderProfileMacro(pProfile, lMacros);

    // Compiles
    CComPtr<ID3DBlob> pShaderBlob, pErrorBlob;
    CShaderIncludeHandler lIncludeHandler(m_SystemWideIncludesPath, CA2T(pSourceName));
    HRESULT hr = m_pD3DCompile(
                     pSrcData,
                     SrcDataSize,
                     pSourceName,
                     lMacros.GetD3DShaderMacro(),
                     &lIncludeHandler,
                     pEntrypoint,
                     lProfile,
                     Flags,
                     0,
                     &pShaderBlob,
                     &pErrorBlob);

    // Copy back of the included files list
    if (pIncludedFiles != nullptr) {
        const std::vector<CString>& lFiles = lIncludeHandler.getIncludedFiles();
        pIncludedFiles->assign(lFiles.begin(), lFiles.end());
    }

    // Generate the returns
    if (pErrMsg) {
        *pErrMsg = GetStringFromBlob(pErrorBlob);
    }

    if (FAILED(hr)) {
        return hr;
    }

    // Generate the pixel shader from the compiled source code
    if (ppPixelShader) {
        hr = m_pD3DDev->CreatePixelShader((DWORD*)pShaderBlob->GetBufferPointer(), ppPixelShader);
        if (FAILED(hr)) {
            return hr;
        }
    }

    // Generate a dissambly of the shader, for debug
    if (pDisasm) {

        // Writes down all the macros that were used during compilation
        CStringA defs;
        for (const auto& lMacro : lMacros) {
            defs.Append("// #define ");
            defs.Append(lMacro.first);
            defs.Append(" ");
            defs.Append(lMacro.second);
            defs.Append("\n");
        }

        CComPtr<ID3DBlob> pDisasmBlob;
        hr = m_pD3DDisassemble(
                 pShaderBlob->GetBufferPointer(),
                 pShaderBlob->GetBufferSize(),
                 0,
                 defs,
                 &pDisasmBlob);

        if (SUCCEEDED(hr)) {
            *pDisasm = GetStringFromBlob(pDisasmBlob);;
        }
    }

    return S_OK;
}

/******************************************************************************/
CStringA CPixelShaderCompiler::GetStringFromBlob(ID3DBlob* pBlob) const
{
    CStringA lResult;

    if (pBlob) {
        auto len = pBlob->GetBufferSize();
        VERIFY(memcpy_s(lResult.GetBufferSetLength((int)len), len, pBlob->GetBufferPointer(), len) == 0);
        lResult.ReleaseBuffer((int)len);
    }

    return lResult;
}

/******************************************************************************/
HRESULT CPixelShaderCompiler::CompileShader(
    LPCSTR pSrcData,
    LPCSTR pEntrypoint,
    LPCSTR pProfile,
    DWORD Flags,
    IDirect3DPixelShader9** ppPixelShader,
    CString* pDisasm,
    CString* pErrMsg,
    std::vector<CString>* pIncludedFiles,
    CString* pSrcFileName)
{
    CString lSrcFileName;
    if (pSrcFileName != nullptr) {
        lSrcFileName = *pSrcFileName;
    }

    return InternalCompile(
               pSrcData,
               strlen(pSrcData),
               CT2A(lSrcFileName),
               pEntrypoint,
               pProfile,
               Flags,
               ppPixelShader,
               pDisasm,
               pErrMsg,
               pIncludedFiles);
}

/******************************************************************************/
HRESULT CPixelShaderCompiler::CompileShaderFromFile(
    LPCTSTR pSrcFile,
    LPCSTR pEntrypoint,
    LPCSTR pProfile,
    DWORD Flags,
    IDirect3DPixelShader9** ppPixelShader,
    CString* pDisasm,
    CString* pErrMsg,
    std::vector<CString>* pIncludedFiles)
{
    // Reads the file
    std::vector<char> lData;
    if (FAILED(ReadSourceFile(pSrcFile, lData))) {
        return E_FAIL;
    }

    // Compiles
    HRESULT ret = E_FAIL;
    ret = InternalCompile(
              &lData.at(0),
              lData.size(),
              CT2A(pSrcFile),
              pEntrypoint,
              pProfile,
              Flags,
              ppPixelShader,
              pDisasm,
              pErrMsg,
              pIncludedFiles);

    return ret;
}

/******************************************************************************/
HRESULT CPixelShaderCompiler::ReadSourceFile(LPCTSTR pSrcFile, std::vector<char>& pOut) const
{
    std::ifstream lFile(pSrcFile, std::ios_base::in | std::ios_base::binary);
    if (lFile.is_open() == false) {
        return E_FAIL;
    }

    lFile.seekg(0, std::ios_base::end);
    std::size_t lSize = lFile.tellg();
    lFile.seekg(0, std::ios_base::beg);

    pOut.resize(lSize + 1);

    lFile.read(&pOut.at(0), lSize);
    pOut.at(lSize) = 0x00;
    lFile.close();

    return S_OK;
}

/******************************************************************************/
HRESULT CPixelShaderCompiler::PreprocessShaderFromFile(
    LPCTSTR pSrcFile,
    LPCSTR pProfile,
    IDirect3DPixelShader9** ppPixelShader,
    CString* pFullCode,
    CString* pErrMsg,
    std::vector<CString>* pIncludedFiles)
{
    // Basics checks
    if (!m_pD3DPreprocess) {
        return E_FAIL;
    }

    // Reads the file
    std::vector<char> lData;
    if (FAILED(ReadSourceFile(pSrcFile, lData))) {
        return E_FAIL;
    }

    // Gather the macros
    CShaderMacros lMacros;
    CStringA lProfile = FindShaderProfile(pProfile);
    GetShaderProfileMacro(pProfile, lMacros);

    // Preprocess the file and generate the full code
    CShaderIncludeHandler lIncludeHandler(m_SystemWideIncludesPath, pSrcFile);
    CComPtr<ID3DBlob> lResult, lErrMsg;

    HRESULT hr = m_pD3DPreprocess(
                     (LPCVOID) &lData.at(0),
                     lData.size(),
                     CT2A(pSrcFile),
                     lMacros.GetD3DShaderMacro(),
                     &lIncludeHandler,
                     &lResult,
                     &lErrMsg);

    // Generates the results
    if (pErrMsg != nullptr) {
        *pErrMsg = GetStringFromBlob(lErrMsg);
    }

    if (FAILED(hr)) {
        return hr;
    }

    if (pFullCode != nullptr) {
        *pFullCode = GetStringFromBlob(lResult);
    }

    return S_OK;
}

/******************************************************************************/
CShaderMacros::Wrapper::Wrapper(CShaderMacros& pSrc):
    data(nullptr)
{
    // Does not copy content, just pointers.
    // Wrapper is supposed to be temporary, and to be created only by CShaderMacros::GetD3DShaderMacro

    std::size_t lCount = pSrc.size();
    data = new D3D_SHADER_MACRO[lCount + 1];

    int i = 0;
    for (std::pair<CStringA, CStringA>& lPair : pSrc) {
        data[i].Name = lPair.first.GetBuffer();
        data[i].Definition = lPair.second.GetBuffer();
        ++i;
    }

    // Last one must be null
    data[lCount].Name = nullptr;
    data[lCount].Definition = nullptr;
}

/******************************************************************************/
CShaderMacros::Wrapper::~Wrapper()
{
    // Wrapper may have been moved and be now empty
    if (data != nullptr) {
        delete[] data;
    }
}

/******************************************************************************/
CShaderMacros::Wrapper::operator D3D_SHADER_MACRO* ()
{
    return data;
}

/******************************************************************************/
CShaderMacros::Wrapper::Wrapper(CShaderMacros::Wrapper&& pFrom):
    data(pFrom.data)
{
    pFrom.data = nullptr;
}

/******************************************************************************/
CShaderMacros::Wrapper& CShaderMacros::Wrapper::operator=(CShaderMacros::Wrapper && pFrom)
{
    if (this != &pFrom) {
        data = pFrom.data;
        pFrom.data = nullptr;
    }

    return *this;
}

/******************************************************************************/
void CShaderMacros::AddMacro(const CStringA& pName, const CStringA& pDefinition)
{
    push_back(std::make_pair(pName, pDefinition));
}

/******************************************************************************/
CShaderMacros::Wrapper CShaderMacros::GetD3DShaderMacro()
{
    return Wrapper(*this);
}