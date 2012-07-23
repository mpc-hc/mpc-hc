/*
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

#pragma once

// Font creation flags
#define D3DFONT_BOLD        0x0001
#define D3DFONT_ITALIC      0x0002
#define D3DFONT_ZENABLE     0x0004

// Font rendering flags
#define D3DFONT_CENTERED_X  0x0001
#define D3DFONT_CENTERED_Y  0x0002
#define D3DFONT_TWOSIDED    0x0004
#define D3DFONT_FILTERED    0x0008


class CD3DFont
{
    TCHAR m_strFontName[80];         // Font properties
    DWORD m_dwFontHeight;
    DWORD m_dwFontFlags;

    LPDIRECT3DDEVICE9  m_pd3dDevice;   // A D3DDevice used for rendering
    LPDIRECT3DTEXTURE9 m_pTexture;     // The d3d texture for this font
    LPDIRECT3DVERTEXBUFFER9 m_pVB;     // VertexBuffer for rendering text
    DWORD m_dwTexWidth;                // Texture dimensions
    DWORD m_dwTexHeight;
    float m_fTextScale;
    float m_fTexCoords[128 - 32][4];
    DWORD m_dwSpacing;                 // Character pixel spacing per side

    // Stateblocks for setting and restoring render states
    LPDIRECT3DSTATEBLOCK9 m_pStateBlockSaved;
    LPDIRECT3DSTATEBLOCK9 m_pStateBlockDrawText;

    HRESULT CreateGDIFont(HDC hDC, HFONT* pFont);
    HRESULT PaintAlphabet(HDC hDC, BOOL bMeasureOnly = FALSE);

public:
    // 2D and 3D text drawing functions
    HRESULT DrawText(float x, float y, DWORD dwColor, const TCHAR* strText, DWORD dwFlags = 0L);
    HRESULT DrawTextScaled(float x, float y, float z,
                           float fXScale, float fYScale, DWORD dwColor,
                           const TCHAR* strText, DWORD dwFlags = 0L);
    HRESULT Render3DText(const TCHAR* strText, DWORD dwFlags = 0L);

    // Function to get extent of text
    HRESULT GetTextExtent(const TCHAR* strText, SIZE* pSize);

    // Initializing and destroying device-dependent objects
    HRESULT InitDeviceObjects(IDirect3DDevice9* pd3dDevice);
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();

    // Constructor / destructor
    CD3DFont(const TCHAR* strFontName, DWORD dwHeight, DWORD dwFlags = 0L);
    ~CD3DFont();
};
