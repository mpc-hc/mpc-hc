/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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

/////////////////////////////////////////////////////////////////////////////
// CShockwaveFlash

class CShockwaveFlash : public CWnd
{
protected:
    DECLARE_DYNCREATE(CShockwaveFlash)
public:
    CLSID const& GetClsid() {
        static CLSID const clsid
            = { 0xD27CDB6E, 0xAE6D, 0x11CF, { 0x96, 0xB8, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } };
        return clsid;
    }
    virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
                        const RECT& rect, CWnd* pParentWnd, UINT nID,
                        CCreateContext* pContext = nullptr) {
        return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID);
    }

    BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
                UINT nID, CFile* pPersist = nullptr, BOOL bStorage = FALSE,
                BSTR bstrLicKey = nullptr) {
        return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
                             pPersist, bStorage, bstrLicKey);
    }

    // Attributes
public:

    // Operations
public:

    long get_ReadyState() {
        long result;
        InvokeHelper(DISPID_READYSTATE, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
        return result;
    }
    long get_TotalFrames() {
        long result;
        InvokeHelper(0x7c, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
        return result;
    }
    BOOL get_Playing() {
        BOOL result;
        InvokeHelper(0x7d, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
        return result;
    }
    void put_Playing(BOOL newValue) {
        static BYTE parms[] = VTS_BOOL;
        InvokeHelper(0x7d, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    long get_Quality() {
        long result;
        InvokeHelper(0x69, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
        return result;
    }
    void put_Quality(long newValue) {
        static BYTE parms[] = VTS_I4;
        InvokeHelper(0x69, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    long get_ScaleMode() {
        long result;
        InvokeHelper(0x78, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
        return result;
    }
    void put_ScaleMode(long newValue) {
        static BYTE parms[] = VTS_I4;
        InvokeHelper(0x78, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    long get_AlignMode() {
        long result;
        InvokeHelper(0x79, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
        return result;
    }
    void put_AlignMode(long newValue) {
        static BYTE parms[] = VTS_I4;
        InvokeHelper(0x79, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    long get_BackgroundColor() {
        long result;
        InvokeHelper(0x7b, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
        return result;
    }
    void put_BackgroundColor(long newValue) {
        static BYTE parms[] = VTS_I4;
        InvokeHelper(0x7b, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    BOOL get_Loop() {
        BOOL result;
        InvokeHelper(0x6a, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
        return result;
    }
    void put_Loop(BOOL newValue) {
        static BYTE parms[] = VTS_BOOL;
        InvokeHelper(0x6a, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    CString get_Movie() {
        CString result;
        InvokeHelper(0x66, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
        return result;
    }
    void put_Movie(LPCTSTR newValue) {
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0x66, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    long get_FrameNum() {
        long result;
        InvokeHelper(0x6b, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
        return result;
    }
    void put_FrameNum(long newValue) {
        static BYTE parms[] = VTS_I4;
        InvokeHelper(0x6b, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    void SetZoomRect(long left, long top, long right, long bottom) {
        static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4;
        InvokeHelper(0x6d, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, left, top, right, bottom);
    }
    void Zoom(long factor) {
        static BYTE parms[] = VTS_I4;
        InvokeHelper(0x76, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, factor);
    }
    void Pan(long x, long y, long mode) {
        static BYTE parms[] = VTS_I4 VTS_I4 VTS_I4;
        InvokeHelper(0x77, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, x, y, mode);
    }
    void Play() {
        InvokeHelper(0x70, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
    }
    void Stop() {
        InvokeHelper(0x71, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
    }
    void Back() {
        InvokeHelper(0x72, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
    }
    void Forward() {
        InvokeHelper(0x73, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
    }
    void Rewind() {
        InvokeHelper(0x74, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
    }
    void StopPlay() {
        InvokeHelper(0x7e, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
    }
    void GotoFrame(long FrameNum) {
        static BYTE parms[] = VTS_I4;
        InvokeHelper(0x7f, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, FrameNum);
    }
    long CurrentFrame() {
        long result;
        InvokeHelper(0x80, DISPATCH_METHOD, VT_I4, (void*)&result, nullptr);
        return result;
    }
    BOOL IsPlaying() {
        BOOL result;
        InvokeHelper(0x81, DISPATCH_METHOD, VT_BOOL, (void*)&result, nullptr);
        return result;
    }
    long PercentLoaded() {
        long result;
        InvokeHelper(0x82, DISPATCH_METHOD, VT_I4, (void*)&result, nullptr);
        return result;
    }
    BOOL FrameLoaded(long FrameNum) {
        BOOL result;
        static BYTE parms[] = VTS_I4;
        InvokeHelper(0x83, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms, FrameNum);
        return result;
    }
    long FlashVersion() {
        long result;
        InvokeHelper(0x84, DISPATCH_METHOD, VT_I4, (void*)&result, nullptr);
        return result;
    }
    CString get_WMode() {
        CString result;
        InvokeHelper(0x85, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
        return result;
    }
    void put_WMode(LPCTSTR newValue) {
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0x85, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    CString get_SAlign() {
        CString result;
        InvokeHelper(0x86, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
        return result;
    }
    void put_SAlign(LPCTSTR newValue) {
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0x86, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    BOOL get_Menu() {
        BOOL result;
        InvokeHelper(0x87, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
        return result;
    }
    void put_Menu(BOOL newValue) {
        static BYTE parms[] = VTS_BOOL;
        InvokeHelper(0x87, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    CString get_Base() {
        CString result;
        InvokeHelper(0x88, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
        return result;
    }
    void put_Base(LPCTSTR newValue) {
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0x88, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    CString get_Scale() {
        CString result;
        InvokeHelper(0x89, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
        return result;
    }
    void put_Scale(LPCTSTR newValue) {
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0x89, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    BOOL get_DeviceFont() {
        BOOL result;
        InvokeHelper(0x8a, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
        return result;
    }
    void put_DeviceFont(BOOL newValue) {
        static BYTE parms[] = VTS_BOOL;
        InvokeHelper(0x8a, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    BOOL get_EmbedMovie() {
        BOOL result;
        InvokeHelper(0x8b, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
        return result;
    }
    void put_EmbedMovie(BOOL newValue) {
        static BYTE parms[] = VTS_BOOL;
        InvokeHelper(0x8b, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    CString get_BGColor() {
        CString result;
        InvokeHelper(0x8c, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
        return result;
    }
    void put_BGColor(LPCTSTR newValue) {
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0x8c, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    CString get_Quality2() {
        CString result;
        InvokeHelper(0x8d, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
        return result;
    }
    void put_Quality2(LPCTSTR newValue) {
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0x8d, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    void LoadMovie(long layer, LPCTSTR url) {
        static BYTE parms[] = VTS_I4 VTS_BSTR;
        InvokeHelper(0x8e, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, layer, url);
    }
    void TGotoFrame(LPCTSTR target, long FrameNum) {
        static BYTE parms[] = VTS_BSTR VTS_I4;
        InvokeHelper(0x8f, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, target, FrameNum);
    }
    void TGotoLabel(LPCTSTR target, LPCTSTR label) {
        static BYTE parms[] = VTS_BSTR VTS_BSTR;
        InvokeHelper(0x90, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, target, label);
    }
    long TCurrentFrame(LPCTSTR target) {
        long result;
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0x91, DISPATCH_METHOD, VT_I4, (void*)&result, parms, target);
        return result;
    }
    CString TCurrentLabel(LPCTSTR target) {
        CString result;
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0x92, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, target);
        return result;
    }
    void TPlay(LPCTSTR target) {
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0x93, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, target);
    }
    void TStopPlay(LPCTSTR target) {
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0x94, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, target);
    }
    void SetVariable(LPCTSTR name, LPCTSTR value) {
        static BYTE parms[] = VTS_BSTR VTS_BSTR;
        InvokeHelper(0x97, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, name, value);
    }
    CString GetVariable(LPCTSTR name) {
        CString result;
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0x98, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, name);
        return result;
    }
    void TSetProperty(LPCTSTR target, long property, LPCTSTR value) {
        static BYTE parms[] = VTS_BSTR VTS_I4 VTS_BSTR;
        InvokeHelper(0x99, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, target, property, value);
    }
    CString TGetProperty(LPCTSTR target, long property) {
        CString result;
        static BYTE parms[] = VTS_BSTR VTS_I4;
        InvokeHelper(0x9a, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, target, property);
        return result;
    }
    void TCallFrame(LPCTSTR target, long FrameNum) {
        static BYTE parms[] = VTS_BSTR VTS_I4;
        InvokeHelper(0x9b, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, target, FrameNum);
    }
    void TCallLabel(LPCTSTR target, LPCTSTR label) {
        static BYTE parms[] = VTS_BSTR VTS_BSTR;
        InvokeHelper(0x9c, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, target, label);
    }
    void TSetPropertyNum(LPCTSTR target, long property, double value) {
        static BYTE parms[] = VTS_BSTR VTS_I4 VTS_R8;
        InvokeHelper(0x9d, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, target, property, value);
    }
    double TGetPropertyNum(LPCTSTR target, long property) {
        double result;
        static BYTE parms[] = VTS_BSTR VTS_I4;
        InvokeHelper(0x9e, DISPATCH_METHOD, VT_R8, (void*)&result, parms, target, property);
        return result;
    }
    double TGetPropertyAsNumber(LPCTSTR target, long property) {
        double result;
        static BYTE parms[] = VTS_BSTR VTS_I4;
        InvokeHelper(0xac, DISPATCH_METHOD, VT_R8, (void*)&result, parms, target, property);
        return result;
    }
    CString get_SWRemote() {
        CString result;
        InvokeHelper(0x9f, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
        return result;
    }
    void put_SWRemote(LPCTSTR newValue) {
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0x9f, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    CString get_FlashVars() {
        CString result;
        InvokeHelper(0xaa, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
        return result;
    }
    void put_FlashVars(LPCTSTR newValue) {
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0xaa, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }
    CString get_AllowScriptAccess() {
        CString result;
        InvokeHelper(0xab, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
        return result;
    }
    void put_AllowScriptAccess(LPCTSTR newValue) {
        static BYTE parms[] = VTS_BSTR;
        InvokeHelper(0xab, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
    }

};
