/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include "RenderersSettings.h"
#include "../../../mpc-hc/resource.h"


CPixelShaderCompiler::CPixelShaderCompiler(IDirect3DDevice9* pD3DDev, bool fStaySilent)
	: m_pD3DDev(pD3DDev)
	, m_pD3DXCompileShader(NULL)
	, m_pD3DXDisassembleShader(NULL)
{
	HINSTANCE		hDll;
	hDll = GetRenderersData()->GetD3X9Dll();

	if (hDll) {
		m_pD3DXCompileShader = (D3DXCompileShaderPtr)GetProcAddress(hDll, "D3DXCompileShader");
		m_pD3DXDisassembleShader = (D3DXDisassembleShaderPtr)GetProcAddress(hDll, "D3DXDisassembleShader");
	}

	if (!fStaySilent) {
		if (!hDll) {
			AfxMessageBox(IDS_PIXELSHADERCOMPILER_0, MB_ICONWARNING | MB_OK, 0);
		} else if (!m_pD3DXCompileShader || !m_pD3DXDisassembleShader) {
			AfxMessageBox(IDS_PIXELSHADERCOMPILER_1, MB_ICONWARNING | MB_OK, 0);
		}
	}
}

CPixelShaderCompiler::~CPixelShaderCompiler()
{
}

HRESULT CPixelShaderCompiler::CompileShader(
	LPCSTR pSrcData,
	LPCSTR pFunctionName,
	LPCSTR pProfile,
	DWORD Flags,
	IDirect3DPixelShader9** ppPixelShader,
	CString* disasm,
	CString* errmsg)
{
	if (!m_pD3DXCompileShader || !m_pD3DXDisassembleShader) {
		return E_FAIL;
	}

	HRESULT hr;

	CComPtr<ID3DXBuffer> pShader, pDisAsm, pErrorMsgs;
	hr = m_pD3DXCompileShader(pSrcData, strlen(pSrcData), NULL, NULL, pFunctionName, pProfile, Flags, &pShader, &pErrorMsgs, NULL);

	if (FAILED(hr)) {
		if (errmsg) {
			CStringA msg = "Unexpected compiler error";

			if (pErrorMsgs) {
				int len = pErrorMsgs->GetBufferSize();
				memcpy(msg.GetBufferSetLength(len), pErrorMsgs->GetBufferPointer(), len);
			}

			*errmsg = msg;
		}

		return hr;
	}

	if (ppPixelShader) {
		if (!m_pD3DDev) {
			return E_FAIL;
		}
		hr = m_pD3DDev->CreatePixelShader((DWORD*)pShader->GetBufferPointer(), ppPixelShader);
		if (FAILED(hr)) {
			return hr;
		}
	}

	if (disasm) {
		hr = m_pD3DXDisassembleShader((DWORD*)pShader->GetBufferPointer(), FALSE, NULL, &pDisAsm);
		if (SUCCEEDED(hr) && pDisAsm) {
			*disasm = CStringA((const char*)pDisAsm->GetBufferPointer());
		}
	}

	return S_OK;
}
