/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2017 see Authors.txt
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
#include "../../../mpc-hc/resource.h"
#include <mpc-hc_config.h>
#include <d3d9.h>

CPixelShaderCompiler::CPixelShaderCompiler(IDirect3DDevice9* pD3DDev, bool fStaySilent)
    : m_hDll(nullptr)
    , m_pD3DCompile(nullptr)
    , m_pD3DDisassemble(nullptr)
    , m_pD3DDev(pD3DDev)
    , m_Cache(pD3DDev)
{
    static_assert(D3D_COMPILER_VERSION == MPC_D3D_COMPILER_VERSION,
                  "MPC_D3D_COMPILER_VERSION define used in the installer needs to be updated.");
    m_hDll = LoadLibrary(D3DCOMPILER_DLL);

    if (m_hDll) {
        m_pD3DCompile = (pD3DCompile)GetProcAddress(m_hDll, "D3DCompile");
        m_pD3DDisassemble = (pD3DDisassemble)GetProcAddress(m_hDll, "D3DDisassemble");
    }

    if (!fStaySilent) {
        if (!m_hDll) {
            CString msg;
            msg.Format(IDS_SHADER_DLL_ERR_0, D3DCOMPILER_DLL);
            AfxMessageBox(msg, MB_ICONWARNING | MB_OK);
        } else if (!m_pD3DCompile || !m_pD3DDisassemble) {
            CString msg;
            msg.Format(IDS_SHADER_DLL_ERR_1, D3DCOMPILER_DLL);
            AfxMessageBox(msg, MB_ICONWARNING | MB_OK);
        }
    }
}

CPixelShaderCompiler::~CPixelShaderCompiler()
{
    if (m_hDll) {
        VERIFY(FreeLibrary(m_hDll));
    }
}

HRESULT CPixelShaderCompiler::InternalCompile(
    LPCSTR pSrcData,
    SIZE_T SrcDataSize,
    LPCSTR pSourceName,
    LPCSTR pEntrypoint,
    LPCSTR pProfile,
    DWORD Flags,
    IDirect3DPixelShader9** ppPixelShader,
    CString* pDisasm,
    CString* pErrMsg)
{
    if (!m_pD3DCompile) {
        return E_FAIL;
    }

    if (pDisasm && !m_pD3DDisassemble) {
        return E_FAIL;
    }

    if (ppPixelShader && !m_pD3DDev) {
        return E_FAIL;
    }

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

    if (!pSelProfile || *pSelProfile == '\0') {
        return E_FAIL;
    }

    LPCSTR defProfile = "MPC_HC_SHADER_PROFILE";
    LPCSTR defProfileVal;
    if (!strcmp(pSelProfile, "ps_2_0")) {
        defProfileVal = "0";
    } else if (!strcmp(pSelProfile, "ps_2_b")) {
        defProfileVal = "1";
    } else if (!strcmp(pSelProfile, "ps_2_a") || !strcmp(pSelProfile, "ps_2_sw")) {
        defProfileVal = "2";
    } else if (!strcmp(pSelProfile, "ps_3_0") || !strcmp(pSelProfile, "ps_3_sw")) {
        defProfileVal = "3";
    } else {
        defProfileVal = "-1";
    }

    if (ppPixelShader && SUCCEEDED(m_Cache.CreatePixelShader(defProfileVal, pSrcData, SrcDataSize, ppPixelShader))) {
        return S_OK;
    }

    D3D_SHADER_MACRO macros[] = { { defProfile, defProfileVal }, { 0 } };

    CComPtr<ID3DBlob> pShaderBlob, pErrorBlob;
    HRESULT hr = m_pD3DCompile(pSrcData, SrcDataSize, pSourceName, macros, nullptr, pEntrypoint,
                               pSelProfile, Flags, 0, &pShaderBlob, &pErrorBlob);

    if (pErrMsg) {
        CStringA msg;
        if (pErrorBlob) {
            auto len = pErrorBlob->GetBufferSize();
            VERIFY(memcpy_s(msg.GetBufferSetLength((int)len), len, pErrorBlob->GetBufferPointer(), len) == 0);
            msg.ReleaseBuffer((int)len);
        }
        *pErrMsg = msg;
    }

    if (FAILED(hr)) {
        return hr;
    }

    if (ppPixelShader) {
        hr = m_pD3DDev->CreatePixelShader((DWORD*)pShaderBlob->GetBufferPointer(), ppPixelShader);
        if (FAILED(hr)) {
            return hr;
        }

        m_Cache.SavePixelShader(defProfileVal, pSrcData, SrcDataSize,
                                (void*)pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize());
    }

    if (pDisasm) {
        CComPtr<ID3DBlob> pDisasmBlob;
        CStringA defs;
        for (auto pMacro = macros; pMacro && pMacro->Name && pMacro->Definition; pMacro++) {
            defs.Append("// #define ");
            defs.Append(pMacro->Name);
            defs.Append(" ");
            defs.Append(pMacro->Definition);
            defs.Append("\n");
        }
        hr = m_pD3DDisassemble(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(),
                               0, defs, &pDisasmBlob);
        if (SUCCEEDED(hr)) {
            CStringA disasm;
            auto len = pDisasmBlob->GetBufferSize();
            VERIFY(memcpy_s(disasm.GetBufferSetLength((int)len), len, pDisasmBlob->GetBufferPointer(), len) == 0);
            disasm.ReleaseBuffer((int)len);
            *pDisasm = disasm;
        }
    }

    return S_OK;
}

HRESULT CPixelShaderCompiler::CompileShader(
    LPCSTR pSrcData,
    LPCSTR pEntrypoint,
    LPCSTR pProfile,
    DWORD Flags,
    IDirect3DPixelShader9** ppPixelShader,
    CString* pDisasm,
    CString* pErrMsg)
{
    return InternalCompile(pSrcData, strlen(pSrcData), nullptr, pEntrypoint,
                           pProfile, Flags, ppPixelShader, pDisasm, pErrMsg);
}

HRESULT CPixelShaderCompiler::CompileShaderFromFile(
    LPCTSTR pSrcFile,
    LPCSTR pEntrypoint,
    LPCSTR pProfile,
    DWORD Flags,
    IDirect3DPixelShader9** ppPixelShader,
    CString* pDisasm,
    CString* pErrMsg)
{
    HRESULT ret = E_FAIL;
    if (FILE* fp = _tfsopen(pSrcFile, _T("rb"), _SH_SECURE)) {
        VERIFY(fseek(fp, 0, SEEK_END) == 0);
        long size = ftell(fp);
        rewind(fp);
        if (size > 0) {
            auto data = new (std::nothrow) char[(size_t)size];
            if (data) {
                if (fread(data, size, 1, fp) == 1) {
                    ret = InternalCompile(data, (size_t)size, CT2A(pSrcFile), pEntrypoint,
                                          pProfile, Flags, ppPixelShader, pDisasm, pErrMsg);
                } else {
                    ASSERT(FALSE);
                }
                delete[] data;
            } else {
                ASSERT(FALSE);
            }
        }
        VERIFY(fclose(fp) == 0);
    }
    return ret;
}
