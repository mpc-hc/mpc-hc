/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2016-2017 see Authors.txt
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
#include <afxdlgs.h>
#include <atlpath.h>
#include "resource.h"
#include "../../../Subtitles/VobSubFile.h"
#include "../../../Subtitles/RTS.h"
#include "../../../SubPic/MemSubPic.h"
#include "../../../SubPic/SubPicQueueImpl.h"
#include "vfr.h"

#ifndef _WIN64
#include "vd2/extras/FilterSDK/VirtualDub.h"
#else
#include "vd2/plugin/vdplugin.h"
#include "vd2/plugin/vdvideofilt.h"
#endif

// Size of the char buffer according to VirtualDub Filters SDK doc
#define STRING_PROC_BUFFER_SIZE 128

//
// Generic interface
//

namespace Plugin
{

    class CFilter : public CAMThread, public CCritSec
    {
    private:
        CString m_fn;

    protected:
        float m_fps;
        CCritSec m_csSubLock;
        CComPtr<ISubPicQueue> m_pSubPicQueue;
        CComPtr<ISubPicProvider> m_pSubPicProvider;
        DWORD_PTR m_SubPicProviderId;

    public:
        CFilter()
            : m_fps(-1)
            , m_SubPicProviderId(0) {
            CAMThread::Create();
        }
        virtual ~CFilter() {
            CAMThread::CallWorker(0);
        }

        CString GetFileName() {
            CAutoLock cAutoLock(this);
            return m_fn;
        }
        void SetFileName(CString fn) {
            CAutoLock cAutoLock(this);
            m_fn = fn;
        }

        bool Render(SubPicDesc& dst, REFERENCE_TIME rt, float fps) {
            if (!m_pSubPicProvider) {
                return false;
            }

            CSize size(dst.w, dst.h);

            if (!m_pSubPicQueue) {
                CComPtr<ISubPicAllocator> pAllocator = DEBUG_NEW CMemSubPicAllocator(dst.type, size);

                HRESULT hr = E_FAIL;
                if (!(m_pSubPicQueue = DEBUG_NEW CSubPicQueueNoThread(SubPicQueueSettings(0, 0, false, 50, 100, false), pAllocator, &hr)) || FAILED(hr)) {
                    m_pSubPicQueue = nullptr;
                    return false;
                }
            }

            if (m_SubPicProviderId != (DWORD_PTR)(ISubPicProvider*)m_pSubPicProvider) {
                m_pSubPicQueue->SetSubPicProvider(m_pSubPicProvider);
                m_SubPicProviderId = (DWORD_PTR)(ISubPicProvider*)m_pSubPicProvider;
            }

            CComPtr<ISubPic> pSubPic;
            if (!m_pSubPicQueue->LookupSubPic(rt, pSubPic)) {
                return false;
            }

            CRect r;
            pSubPic->GetDirtyRect(r);

            if (dst.type == MSP_RGB32 || dst.type == MSP_RGB24 || dst.type == MSP_RGB16 || dst.type == MSP_RGB15) {
                dst.h = -dst.h;
            }

            pSubPic->AlphaBlt(r, r, &dst);

            return true;
        }

        DWORD ThreadProc() {
            SetThreadPriority(m_hThread, THREAD_PRIORITY_LOWEST);

            CAtlArray<HANDLE> handles;
            handles.Add(GetRequestHandle());

            CString fn = GetFileName();
            CFileStatus fs;
            fs.m_mtime = 0;
            CFileGetStatus(fn, fs);

            for (;;) {
                DWORD i = WaitForMultipleObjects((DWORD)handles.GetCount(), handles.GetData(), FALSE, 1000);

                if (WAIT_OBJECT_0 == i) {
                    Reply(S_OK);
                    break;
                } else if (WAIT_OBJECT_0 + 1 >= i && i <= WAIT_OBJECT_0 + handles.GetCount()) {
                    if (FindNextChangeNotification(handles[i - WAIT_OBJECT_0])) {
                        CFileStatus fs2;
                        fs2.m_mtime = 0;
                        CFileGetStatus(fn, fs2);

                        if (fs.m_mtime < fs2.m_mtime) {
                            fs.m_mtime = fs2.m_mtime;

                            if (CComQIPtr<ISubStream> pSubStream = m_pSubPicProvider) {
                                CAutoLock cAutoLock(&m_csSubLock);
                                pSubStream->Reload();
                            }
                        }
                    }
                } else if (WAIT_TIMEOUT == i) {
                    CString fn2 = GetFileName();

                    if (fn != fn2) {
                        CPath p(fn2);
                        p.RemoveFileSpec();
                        HANDLE h = FindFirstChangeNotification(p, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
                        if (h != INVALID_HANDLE_VALUE) {
                            fn = fn2;
                            handles.SetCount(1);
                            handles.Add(h);
                        }
                    }
                } else { // if (WAIT_ABANDONED_0 == i || WAIT_FAILED == i)
                    break;
                }
            }

            m_hThread = 0;

            for (size_t i = 1; i < handles.GetCount(); i++) {
                FindCloseChangeNotification(handles[i]);
            }

            return 0;
        }
    };

    class CVobSubFilter : virtual public CFilter
    {
    public:
        CVobSubFilter(CString fn = _T("")) {
            if (!fn.IsEmpty()) {
                Open(fn);
            }
        }

        bool Open(CString fn) {
            SetFileName(_T(""));
            m_pSubPicProvider = nullptr;

            if (CVobSubFile* vsf = DEBUG_NEW CVobSubFile(&m_csSubLock)) {
                m_pSubPicProvider = (ISubPicProvider*)vsf;
                if (vsf->Open(CString(fn))) {
                    SetFileName(fn);
                } else {
                    m_pSubPicProvider = nullptr;
                }
            }

            return !!m_pSubPicProvider;
        }
    };

    class CTextSubFilter : virtual public CFilter
    {
        int m_CharSet;

    public:
        CTextSubFilter(CString fn = _T(""), int CharSet = DEFAULT_CHARSET, float fps = -1)
            : m_CharSet(CharSet) {
            m_fps = fps;
            if (!fn.IsEmpty()) {
                Open(fn, CharSet);
            }
        }

        int GetCharSet() {
            return m_CharSet;
        }

        bool Open(CString fn, int CharSet = DEFAULT_CHARSET) {
            SetFileName(_T(""));
            m_pSubPicProvider = nullptr;

            if (!m_pSubPicProvider) {
                if (CRenderedTextSubtitle* rts = DEBUG_NEW CRenderedTextSubtitle(&m_csSubLock)) {
                    m_pSubPicProvider = (ISubPicProvider*)rts;
                    if (rts->Open(CString(fn), CharSet)) {
                        SetFileName(fn);
                    } else {
                        m_pSubPicProvider = nullptr;
                    }
                }
            }

            return !!m_pSubPicProvider;
        }
    };

#ifndef _WIN64
    //
    // old VirtualDub interface
    //

    namespace VirtualDub
    {
        class CVirtualDubFilter : virtual public CFilter
        {
        public:
            CVirtualDubFilter() {}
            virtual ~CVirtualDubFilter() {}

            virtual int RunProc(const FilterActivation* fa, const FilterFunctions* ff) {
                SubPicDesc dst;
                dst.type = MSP_RGB32;
                dst.w = fa->src.w;
                dst.h = fa->src.h;
                dst.bpp = 32;
                dst.pitch = fa->src.pitch;
                dst.bits = (BYTE*)fa->src.data;

                Render(dst, 10000i64 * fa->pfsi->lSourceFrameMS, 1000.0f / fa->pfsi->lMicrosecsPerFrame);

                return 0;
            }

            virtual long ParamProc(FilterActivation* fa, const FilterFunctions* ff) {
                fa->dst.offset  = fa->src.offset;
                fa->dst.modulo  = fa->src.modulo;
                fa->dst.pitch   = fa->src.pitch;

                return 0;
            }

            virtual int ConfigProc(FilterActivation* fa, const FilterFunctions* ff, HWND hwnd) = 0;
            virtual void StringProc(const FilterActivation* fa, const FilterFunctions* ff, char* str) = 0;
            virtual bool FssProc(FilterActivation* fa, const FilterFunctions* ff, char* buf, int buflen) = 0;
        };

        class CVobSubVirtualDubFilter : public CVobSubFilter, public CVirtualDubFilter
        {
        public:
            CVobSubVirtualDubFilter(CString fn = _T(""))
                : CVobSubFilter(fn) {}

            int ConfigProc(FilterActivation* fa, const FilterFunctions* ff, HWND hwnd) {
                AFX_MANAGE_STATE(AfxGetStaticModuleState());

                CFileDialog fd(TRUE, nullptr, GetFileName(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY,
                               _T("VobSub files (*.idx;*.sub)|*.idx;*.sub||"), CWnd::FromHandle(hwnd), 0);

                if (fd.DoModal() != IDOK) {
                    return 1;
                }

                return Open(fd.GetPathName()) ? 0 : 1;
            }

            void StringProc(const FilterActivation* fa, const FilterFunctions* ff, char* str) {
                sprintf_s(str, STRING_PROC_BUFFER_SIZE, " (%s)", GetFileName().IsEmpty() ? " (empty)" : CStringA(GetFileName()).GetString());
            }

            bool FssProc(FilterActivation* fa, const FilterFunctions* ff, char* buf, int buflen) {
                CStringA fn(GetFileName());
                fn.Replace("\\", "\\\\");
                _snprintf_s(buf, buflen, buflen, "Config(\"%s\")", fn.GetString());
                return true;
            }
        };

        class CTextSubVirtualDubFilter : public CTextSubFilter, public CVirtualDubFilter
        {
        public:
            CTextSubVirtualDubFilter(CString fn = _T(""), int CharSet = DEFAULT_CHARSET)
                : CTextSubFilter(fn, CharSet) {}

            int ConfigProc(FilterActivation* fa, const FilterFunctions* ff, HWND hwnd) {
                AFX_MANAGE_STATE(AfxGetStaticModuleState());

                const TCHAR formats[] = _T("TextSub files (*.sub;*.srt;*.smi;*.ssa;*.ass;*.xss;*.psb;*.txt)|*.sub;*.srt;*.smi;*.ssa;*.ass;*.xss;*.psb;*.txt||");
                CFileDialog fd(TRUE, nullptr, GetFileName(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK,
                               formats, CWnd::FromHandle(hwnd), sizeof(OPENFILENAME));
                UINT_PTR CALLBACK OpenHookProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);

                fd.m_pOFN->hInstance = AfxGetResourceHandle();
                fd.m_pOFN->lpTemplateName = MAKEINTRESOURCE(IDD_TEXTSUBOPENTEMPLATE);
                fd.m_pOFN->lpfnHook = (LPOFNHOOKPROC)OpenHookProc;
                fd.m_pOFN->lCustData = (LPARAM)DEFAULT_CHARSET;

                if (fd.DoModal() != IDOK) {
                    return 1;
                }

                return Open(fd.GetPathName(), fd.m_pOFN->lCustData) ? 0 : 1;
            }

            void StringProc(const FilterActivation* fa, const FilterFunctions* ff, char* str) {
                if (!GetFileName().IsEmpty()) {
                    sprintf_s(str, STRING_PROC_BUFFER_SIZE, " (%s, %d)", CStringA(GetFileName()).GetString(), GetCharSet());
                } else {
                    sprintf_s(str, STRING_PROC_BUFFER_SIZE, " (empty)");
                }
            }

            bool FssProc(FilterActivation* fa, const FilterFunctions* ff, char* buf, int buflen) {
                CStringA fn(GetFileName());
                fn.Replace("\\", "\\\\");
                _snprintf_s(buf, buflen, buflen, "Config(\"%s\", %d)", fn.GetString(), GetCharSet());
                return true;
            }
        };

        int vobsubInitProc(FilterActivation* fa, const FilterFunctions* ff)
        {
            *(CVirtualDubFilter**)fa->filter_data = DEBUG_NEW CVobSubVirtualDubFilter();
            return !(*(CVirtualDubFilter**)fa->filter_data);
        }

        int textsubInitProc(FilterActivation* fa, const FilterFunctions* ff)
        {
            *(CVirtualDubFilter**)fa->filter_data = DEBUG_NEW CTextSubVirtualDubFilter();
            return !(*(CVirtualDubFilter**)fa->filter_data);
        }

        void baseDeinitProc(FilterActivation* fa, const FilterFunctions* ff)
        {
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            SAFE_DELETE(f);
        }

        int baseRunProc(const FilterActivation* fa, const FilterFunctions* ff)
        {
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            return f ? f->RunProc(fa, ff) : 1;
        }

        long baseParamProc(FilterActivation* fa, const FilterFunctions* ff)
        {
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            return f ? f->ParamProc(fa, ff) : 1;
        }

        int baseConfigProc(FilterActivation* fa, const FilterFunctions* ff, HWND hwnd)
        {
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            return f ? f->ConfigProc(fa, ff, hwnd) : 1;
        }

        void baseStringProc(const FilterActivation* fa, const FilterFunctions* ff, char* str)
        {
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            if (f) {
                f->StringProc(fa, ff, str);
            }
        }

        bool baseFssProc(FilterActivation* fa, const FilterFunctions* ff, char* buf, int buflen)
        {
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            return f ? f->FssProc(fa, ff, buf, buflen) : false;
        }

        void vobsubScriptConfig(IScriptInterpreter* isi, void* lpVoid, CScriptValue* argv, int argc)
        {
            FilterActivation* fa = (FilterActivation*)lpVoid;
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            if (f) {
                delete f;
            }
            f = DEBUG_NEW CVobSubVirtualDubFilter(CString(*argv[0].asString()));
            *(CVirtualDubFilter**)fa->filter_data = f;
        }

        void textsubScriptConfig(IScriptInterpreter* isi, void* lpVoid, CScriptValue* argv, int argc)
        {
            FilterActivation* fa = (FilterActivation*)lpVoid;
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            if (f) {
                delete f;
            }
            f = DEBUG_NEW CTextSubVirtualDubFilter(CString(*argv[0].asString()), argv[1].asInt());
            *(CVirtualDubFilter**)fa->filter_data = f;
        }

        ScriptFunctionDef vobsub_func_defs[] = {
            { (ScriptFunctionPtr)vobsubScriptConfig, const_cast<char*>("Config"), const_cast<char*>("0s") },
            { nullptr },
        };

        CScriptObject vobsub_obj = {
            nullptr, vobsub_func_defs
        };

        struct FilterDefinition filterDef_vobsub = {
            nullptr, nullptr, nullptr,              // next, prev, module
            "VobSub",                               // name
            "Adds subtitles from a vob sequence.",  // desc
            "MPC-HC",                               // maker
            nullptr,                                // private_data
            sizeof(CVirtualDubFilter**),            // inst_data_size
            vobsubInitProc,                         // initProc
            baseDeinitProc,                         // deinitProc
            baseRunProc,                            // runProc
            baseParamProc,                          // paramProc
            baseConfigProc,                         // configProc
            baseStringProc,                         // stringProc
            nullptr,                                // startProc
            nullptr,                                // endProc
            &vobsub_obj,                            // script_obj
            baseFssProc,                            // fssProc
        };

        ScriptFunctionDef textsub_func_defs[] = {
            { (ScriptFunctionPtr)textsubScriptConfig, const_cast<char*>("Config"), const_cast<char*>("0si") },
            { nullptr },
        };

        CScriptObject textsub_obj = {
            nullptr, textsub_func_defs
        };

        struct FilterDefinition filterDef_textsub = {
            nullptr, nullptr, nullptr,              // next, prev, module
            "TextSub",                              // name
            "Adds subtitles from srt, sub, psb, smi, ssa, ass file formats.", // desc
            "MPC-HC",                               // maker
            nullptr,                                // private_data
            sizeof(CVirtualDubFilter**),            // inst_data_size
            textsubInitProc,                        // initProc
            baseDeinitProc,                         // deinitProc
            baseRunProc,                            // runProc
            baseParamProc,                          // paramProc
            baseConfigProc,                         // configProc
            baseStringProc,                         // stringProc
            nullptr,                                // startProc
            nullptr,                                // endProc
            &textsub_obj,                           // script_obj
            baseFssProc,                            // fssProc
        };

        static FilterDefinition* fd_vobsub;
        static FilterDefinition* fd_textsub;

        extern "C" __declspec(dllexport) int __cdecl VirtualdubFilterModuleInit2(FilterModule* fm, const FilterFunctions* ff, int& vdfd_ver, int& vdfd_compat)
        {
            fd_vobsub = ff->addFilter(fm, &filterDef_vobsub, sizeof(FilterDefinition));
            if (!fd_vobsub) {
                return 1;
            }
            fd_textsub = ff->addFilter(fm, &filterDef_textsub, sizeof(FilterDefinition));
            if (!fd_textsub) {
                return 1;
            }

            vdfd_ver = VIRTUALDUB_FILTERDEF_VERSION;
            vdfd_compat = VIRTUALDUB_FILTERDEF_COMPATIBLE;

            return 0;
        }

        extern "C" __declspec(dllexport) void __cdecl VirtualdubFilterModuleDeinit(FilterModule* fm, const FilterFunctions* ff)
        {
            ff->removeFilter(fd_textsub);
            ff->removeFilter(fd_vobsub);
        }
    }/**/

#else
    //
    // VirtualDub new plugin interface sdk 1.1
    //
    namespace VirtualDubNew
    {
        class CVirtualDubFilter : virtual public CFilter
        {
        public:
            CVirtualDubFilter() {}
            virtual ~CVirtualDubFilter() {}

            virtual int RunProc(const VDXFilterActivation* fa, const VDXFilterFunctions* ff) {
                SubPicDesc dst;
                dst.type = MSP_RGB32;
                dst.w = fa->src.w;
                dst.h = fa->src.h;
                dst.bpp = 32;
                dst.pitch = int(fa->src.pitch);
                dst.bits = (BYTE*)fa->src.data;

                Render(dst, 10000i64 * fa->pfsi->lSourceFrameMS, 1000.0f / fa->pfsi->lMicrosecsPerFrame);

                return 0;
            }

            virtual long ParamProc(VDXFilterActivation* fa, const VDXFilterFunctions* ff) {
                fa->dst.offset  = fa->src.offset;
                fa->dst.modulo  = fa->src.modulo;
                fa->dst.pitch   = fa->src.pitch;

                return 0;
            }

            virtual int ConfigProc(VDXFilterActivation* fa, const VDXFilterFunctions* ff, VDXHWND hwnd) = 0;
            virtual void StringProc(const VDXFilterActivation* fa, const VDXFilterFunctions* ff, char* str) = 0;
            virtual bool FssProc(VDXFilterActivation* fa, const VDXFilterFunctions* ff, char* buf, int buflen) = 0;
        };

        class CVobSubVirtualDubFilter : public CVobSubFilter, public CVirtualDubFilter
        {
        public:
            CVobSubVirtualDubFilter(CString fn = _T(""))
                : CVobSubFilter(fn) {}

            int ConfigProc(VDXFilterActivation* fa, const VDXFilterFunctions* ff, VDXHWND hwnd) {
                AFX_MANAGE_STATE(AfxGetStaticModuleState());

                CFileDialog fd(TRUE, nullptr, GetFileName(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY,
                               _T("VobSub files (*.idx;*.sub)|*.idx;*.sub||"), CWnd::FromHandle((HWND)hwnd), 0);

                if (fd.DoModal() != IDOK) {
                    return 1;
                }

                return Open(fd.GetPathName()) ? 0 : 1;
            }

            void StringProc(const VDXFilterActivation* fa, const VDXFilterFunctions* ff, char* str) {
                sprintf_s(str, STRING_PROC_BUFFER_SIZE, " (%s)", GetFileName().IsEmpty() ? " (empty)" : CStringA(GetFileName()).GetString());
            }

            bool FssProc(VDXFilterActivation* fa, const VDXFilterFunctions* ff, char* buf, int buflen) {
                CStringA fn(GetFileName());
                fn.Replace("\\", "\\\\");
                _snprintf_s(buf, buflen, buflen, "Config(\"%s\")", fn.GetString());
                return true;
            }
        };

        class CTextSubVirtualDubFilter : public CTextSubFilter, public CVirtualDubFilter
        {
        public:
            CTextSubVirtualDubFilter(CString fn = _T(""), int CharSet = DEFAULT_CHARSET)
                : CTextSubFilter(fn, CharSet) {}

            int ConfigProc(VDXFilterActivation* fa, const VDXFilterFunctions* ff, VDXHWND hwnd) {
                AFX_MANAGE_STATE(AfxGetStaticModuleState());

                /* off encoding changing */
#ifndef _DEBUG
                const TCHAR formats[] = _T("TextSub files (*.sub;*.srt;*.smi;*.ssa;*.ass;*.xss;*.psb;*.txt)|*.sub;*.srt;*.smi;*.ssa;*.ass;*.xss;*.psb;*.txt||");
                CFileDialog fd(TRUE, nullptr, GetFileName(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK,
                               formats, CWnd::FromHandle((HWND)hwnd), sizeof(OPENFILENAME));
                UINT_PTR CALLBACK OpenHookProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);

                fd.m_pOFN->hInstance = AfxGetResourceHandle();
                fd.m_pOFN->lpTemplateName = MAKEINTRESOURCE(IDD_TEXTSUBOPENTEMPLATE);
                fd.m_pOFN->lpfnHook = (LPOFNHOOKPROC)OpenHookProc;
                fd.m_pOFN->lCustData = (LPARAM)DEFAULT_CHARSET;
#else
                const TCHAR formats[] = _T("TextSub files (*.sub;*.srt;*.smi;*.ssa;*.ass;*.xss;*.psb;*.txt)|*.sub;*.srt;*.smi;*.ssa;*.ass;*.xss;*.psb;*.txt||");
                CFileDialog fd(TRUE, nullptr, GetFileName(), OFN_ENABLESIZING | OFN_HIDEREADONLY,
                               formats, CWnd::FromHandle((HWND)hwnd), sizeof(OPENFILENAME));
#endif
                if (fd.DoModal() != IDOK) {
                    return 1;
                }

                return Open(fd.GetPathName(), (int)fd.m_pOFN->lCustData) ? 0 : 1;
            }

            void StringProc(const VDXFilterActivation* fa, const VDXFilterFunctions* ff, char* str) {
                if (!GetFileName().IsEmpty()) {
                    sprintf_s(str, STRING_PROC_BUFFER_SIZE, " (%s, %d)", CStringA(GetFileName()).GetString(), GetCharSet());
                } else {
                    sprintf_s(str, STRING_PROC_BUFFER_SIZE, " (empty)");
                }
            }

            bool FssProc(VDXFilterActivation* fa, const VDXFilterFunctions* ff, char* buf, int buflen) {
                CStringA fn(GetFileName());
                fn.Replace("\\", "\\\\");
                _snprintf_s(buf, buflen, buflen, "Config(\"%s\", %d)", fn.GetString(), GetCharSet());
                return true;
            }
        };

        int vobsubInitProc(VDXFilterActivation* fa, const VDXFilterFunctions* ff)
        {
            return ((*(CVirtualDubFilter**)fa->filter_data = DEBUG_NEW CVobSubVirtualDubFilter()) == nullptr);
        }

        int textsubInitProc(VDXFilterActivation* fa, const VDXFilterFunctions* ff)
        {
            return ((*(CVirtualDubFilter**)fa->filter_data = DEBUG_NEW CTextSubVirtualDubFilter()) == nullptr);
        }

        void baseDeinitProc(VDXFilterActivation* fa, const VDXFilterFunctions* ff)
        {
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            SAFE_DELETE(f);
        }

        int baseRunProc(const VDXFilterActivation* fa, const VDXFilterFunctions* ff)
        {
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            return f ? f->RunProc(fa, ff) : 1;
        }

        long baseParamProc(VDXFilterActivation* fa, const VDXFilterFunctions* ff)
        {
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            return f ? f->ParamProc(fa, ff) : 1;
        }

        int baseConfigProc(VDXFilterActivation* fa, const VDXFilterFunctions* ff, VDXHWND hwnd)
        {
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            return f ? f->ConfigProc(fa, ff, hwnd) : 1;
        }

        void baseStringProc(const VDXFilterActivation* fa, const VDXFilterFunctions* ff, char* str)
        {
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            if (f) {
                f->StringProc(fa, ff, str);
            }
        }

        bool baseFssProc(VDXFilterActivation* fa, const VDXFilterFunctions* ff, char* buf, int buflen)
        {
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            return f ? f->FssProc(fa, ff, buf, buflen) : false;
        }

        void vobsubScriptConfig(IVDXScriptInterpreter* isi, void* lpVoid, VDXScriptValue* argv, int argc)
        {
            VDXFilterActivation* fa = (VDXFilterActivation*)lpVoid;
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            if (f) {
                delete f;
            }
            f = DEBUG_NEW CVobSubVirtualDubFilter(CString(*argv[0].asString()));
            *(CVirtualDubFilter**)fa->filter_data = f;
        }

        void textsubScriptConfig(IVDXScriptInterpreter* isi, void* lpVoid, VDXScriptValue* argv, int argc)
        {
            VDXFilterActivation* fa = (VDXFilterActivation*)lpVoid;
            CVirtualDubFilter* f = *(CVirtualDubFilter**)fa->filter_data;
            if (f) {
                delete f;
            }
            f = DEBUG_NEW CTextSubVirtualDubFilter(CString(*argv[0].asString()), argv[1].asInt());
            *(CVirtualDubFilter**)fa->filter_data = f;
        }

        VDXScriptFunctionDef vobsub_func_defs[] = {
            { (VDXScriptFunctionPtr)vobsubScriptConfig, "Config", "0s" },
            { nullptr },
        };

        VDXScriptObject vobsub_obj = {
            nullptr, vobsub_func_defs
        };

        struct VDXFilterDefinition filterDef_vobsub = {
            nullptr, nullptr, nullptr,              // next, prev, module
            "VobSub",                               // name
            "Adds subtitles from a vob sequence.",  // desc
            "MPC-HC",                               // maker
            nullptr,                                // private_data
            sizeof(CVirtualDubFilter**),            // inst_data_size
            vobsubInitProc,                         // initProc
            baseDeinitProc,                         // deinitProc
            baseRunProc,                            // runProc
            baseParamProc,                          // paramProc
            baseConfigProc,                         // configProc
            baseStringProc,                         // stringProc
            nullptr,                                // startProc
            nullptr,                                // endProc
            &vobsub_obj,                            // script_obj
            baseFssProc,                            // fssProc
        };

        VDXScriptFunctionDef textsub_func_defs[] = {
            { (VDXScriptFunctionPtr)textsubScriptConfig, "Config", "0si" },
            { nullptr },
        };

        VDXScriptObject textsub_obj = {
            nullptr, textsub_func_defs
        };

        struct VDXFilterDefinition filterDef_textsub = {
            nullptr, nullptr, nullptr,              // next, prev, module
            "TextSub",                              // name
            "Adds subtitles from srt, sub, psb, smi, ssa, ass file formats.", // desc
            "MPC-HC",                               // maker
            nullptr,                                // private_data
            sizeof(CVirtualDubFilter**),            // inst_data_size
            textsubInitProc,                        // initProc
            baseDeinitProc,                         // deinitProc
            baseRunProc,                            // runProc
            baseParamProc,                          // paramProc
            baseConfigProc,                         // configProc
            baseStringProc,                         // stringProc
            nullptr,                                // startProc
            nullptr,                                // endProc
            &textsub_obj,                           // script_obj
            baseFssProc,                            // fssProc
        };

        static VDXFilterDefinition* fd_vobsub;
        static VDXFilterDefinition* fd_textsub;

        extern "C" __declspec(dllexport) int __cdecl VirtualdubFilterModuleInit2(VDXFilterModule* fm, const VDXFilterFunctions* ff, int& vdfd_ver, int& vdfd_compat)
        {
            if (((fd_vobsub = ff->addFilter(fm, &filterDef_vobsub, sizeof(VDXFilterDefinition))) == nullptr)
                    || ((fd_textsub = ff->addFilter(fm, &filterDef_textsub, sizeof(VDXFilterDefinition))) == nullptr)) {
                return 1;
            }

            vdfd_ver = VIRTUALDUB_FILTERDEF_VERSION;
            vdfd_compat = VIRTUALDUB_FILTERDEF_COMPATIBLE;

            return 0;
        }

        extern "C" __declspec(dllexport) void __cdecl VirtualdubFilterModuleDeinit(VDXFilterModule* fm, const VDXFilterFunctions* ff)
        {
            ff->removeFilter(fd_textsub);
            ff->removeFilter(fd_vobsub);
        }
    }
#endif
    //
    // Avisynth interface
    //

    namespace AviSynth1
    {
#include "avisynth/avisynth1.h"

        class CAvisynthFilter : public GenericVideoFilter, virtual public CFilter
        {
        public:
            CAvisynthFilter(PClip c, IScriptEnvironment* env) : GenericVideoFilter(c) {}

            PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) {
                PVideoFrame frame = child->GetFrame(n, env);

                env->MakeWritable(&frame);

                SubPicDesc dst;
                dst.w = vi.width;
                dst.h = vi.height;
                dst.pitch = frame->GetPitch();
                dst.bits = frame->GetWritePtr();
                dst.bpp = vi.BitsPerPixel();
                dst.type =
                    vi.IsRGB32() ? (env->GetVar("RGBA").AsBool() ? MSP_RGBA : MSP_RGB32) :
                    vi.IsRGB24() ? MSP_RGB24 :
                    vi.IsYUY2() ? MSP_YUY2 :
                    -1;

                float fps = m_fps > 0 ? m_fps : (float)vi.fps_numerator / vi.fps_denominator;

                Render(dst, (REFERENCE_TIME)(10000000i64 * n / fps), fps);

                return frame;
            }
        };

        class CVobSubAvisynthFilter : public CVobSubFilter, public CAvisynthFilter
        {
        public:
            CVobSubAvisynthFilter(PClip c, const char* fn, IScriptEnvironment* env)
                : CVobSubFilter(CString(fn))
                , CAvisynthFilter(c, env) {
                if (!m_pSubPicProvider) {
                    env->ThrowError("VobSub: Can't open \"%s\"", fn);
                }
            }
        };

        AVSValue __cdecl VobSubCreateS(AVSValue args, void* user_data, IScriptEnvironment* env)
        {
            return (DEBUG_NEW CVobSubAvisynthFilter(args[0].AsClip(), args[1].AsString(), env));
        }

        class CTextSubAvisynthFilter : public CTextSubFilter, public CAvisynthFilter
        {
        public:
            CTextSubAvisynthFilter(PClip c, IScriptEnvironment* env, const char* fn, int CharSet = DEFAULT_CHARSET, float fps = -1)
                : CTextSubFilter(CString(fn), CharSet, fps)
                , CAvisynthFilter(c, env) {
                if (!m_pSubPicProvider) {
                    env->ThrowError("TextSub: Can't open \"%s\"", fn);
                }
            }
        };

        AVSValue __cdecl TextSubCreateS(AVSValue args, void* user_data, IScriptEnvironment* env)
        {
            return (DEBUG_NEW CTextSubAvisynthFilter(args[0].AsClip(), env, args[1].AsString()));
        }

        AVSValue __cdecl TextSubCreateSI(AVSValue args, void* user_data, IScriptEnvironment* env)
        {
            return (DEBUG_NEW CTextSubAvisynthFilter(args[0].AsClip(), env, args[1].AsString(), args[2].AsInt()));
        }

        AVSValue __cdecl TextSubCreateSIF(AVSValue args, void* user_data, IScriptEnvironment* env)
        {
            return (DEBUG_NEW CTextSubAvisynthFilter(args[0].AsClip(), env, args[1].AsString(), args[2].AsInt(), (float)args[3].AsFloat()));
        }

        AVSValue __cdecl MaskSubCreateSIIFI(AVSValue args, void* user_data, IScriptEnvironment* env)
        {
            AVSValue rgb32("RGB32");
            AVSValue  tab[5] = {
                args[1],
                args[2],
                args[3],
                args[4],
                rgb32
            };
            AVSValue value(tab, 5);
            const char* nom[5] = {
                "width",
                "height",
                "fps",
                "length",
                "pixel_type"
            };
            AVSValue clip(env->Invoke("Blackness", value, nom));
            env->SetVar(env->SaveString("RGBA"), true);
            return (DEBUG_NEW CTextSubAvisynthFilter(clip.AsClip(), env, args[0].AsString()));
        }

        extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit(IScriptEnvironment* env)
        {
            env->AddFunction("VobSub", "cs", VobSubCreateS, 0);
            env->AddFunction("TextSub", "cs", TextSubCreateS, 0);
            env->AddFunction("TextSub", "csi", TextSubCreateSI, 0);
            env->AddFunction("TextSub", "csif", TextSubCreateSIF, 0);
            env->AddFunction("MaskSub", "siifi", MaskSubCreateSIIFI, 0);
            env->SetVar(env->SaveString("RGBA"), false);
            return nullptr;
        }
    }

    namespace AviSynth25
    {
#include "avisynth/avisynth25.h"

        static bool s_fSwapUV = false;

        class CAvisynthFilter : public GenericVideoFilter, virtual public CFilter
        {
        public:
            VFRTranslator* vfr;

            CAvisynthFilter(PClip c, IScriptEnvironment* env, VFRTranslator* _vfr = 0) : GenericVideoFilter(c), vfr(_vfr) {}

            PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) {
                PVideoFrame frame = child->GetFrame(n, env);

                env->MakeWritable(&frame);

                SubPicDesc dst;
                dst.w = vi.width;
                dst.h = vi.height;
                dst.pitch = frame->GetPitch();
                dst.pitchUV = frame->GetPitch(PLANAR_U);
                dst.bits = frame->GetWritePtr();
                dst.bitsU = frame->GetWritePtr(PLANAR_U);
                dst.bitsV = frame->GetWritePtr(PLANAR_V);
                dst.bpp = dst.pitch / dst.w * 8; //vi.BitsPerPixel();
                dst.type =
                    vi.IsRGB32() ? (env->GetVar("RGBA").AsBool() ? MSP_RGBA : MSP_RGB32)  :
                    vi.IsRGB24() ? MSP_RGB24 :
                    vi.IsYUY2() ? MSP_YUY2 :
                    /*vi.IsYV12()*/ vi.pixel_type == VideoInfo::CS_YV12 ? (s_fSwapUV ? MSP_IYUV : MSP_YV12) :
                    /*vi.IsIYUV()*/ vi.pixel_type == VideoInfo::CS_IYUV ? (s_fSwapUV ? MSP_YV12 : MSP_IYUV) :
                    -1;

                float fps = m_fps > 0 ? m_fps : (float)vi.fps_numerator / vi.fps_denominator;

                REFERENCE_TIME timestamp;

                if (!vfr) {
                    timestamp = (REFERENCE_TIME)(10000000i64 * n / fps);
                } else {
                    timestamp = (REFERENCE_TIME)(10000000 * vfr->TimeStampFromFrameNumber(n));
                }

                Render(dst, timestamp, fps);

                return frame;
            }
        };

        class CVobSubAvisynthFilter : public CVobSubFilter, public CAvisynthFilter
        {
        public:
            CVobSubAvisynthFilter(PClip c, const char* fn, IScriptEnvironment* env)
                : CVobSubFilter(CString(fn))
                , CAvisynthFilter(c, env) {
                if (!m_pSubPicProvider) {
                    env->ThrowError("VobSub: Can't open \"%s\"", fn);
                }
            }
        };

        AVSValue __cdecl VobSubCreateS(AVSValue args, void* user_data, IScriptEnvironment* env)
        {
            return (DEBUG_NEW CVobSubAvisynthFilter(args[0].AsClip(), args[1].AsString(), env));
        }

        class CTextSubAvisynthFilter : public CTextSubFilter, public CAvisynthFilter
        {
        public:
            CTextSubAvisynthFilter(PClip c, IScriptEnvironment* env, const char* fn, int CharSet = DEFAULT_CHARSET, float fps = -1, VFRTranslator* vfr = 0) //vfr patch
                : CTextSubFilter(CString(fn), CharSet, fps)
                , CAvisynthFilter(c, env, vfr) {
                if (!m_pSubPicProvider) {
                    env->ThrowError("TextSub: Can't open \"%s\"", fn);
                }
            }
        };

        AVSValue __cdecl TextSubCreateGeneral(AVSValue args, void* user_data, IScriptEnvironment* env)
        {
            if (!args[1].Defined()) {
                env->ThrowError("TextSub: You must specify a subtitle file to use");
            }
            VFRTranslator* vfr = 0;
            if (args[4].Defined()) {
                vfr = GetVFRTranslator(args[4].AsString());
            }

            return (DEBUG_NEW CTextSubAvisynthFilter(
                        args[0].AsClip(),
                        env,
                        args[1].AsString(),
                        args[2].AsInt(DEFAULT_CHARSET),
                        (float)args[3].AsFloat(-1),
                        vfr));
        }

        AVSValue __cdecl TextSubSwapUV(AVSValue args, void* user_data, IScriptEnvironment* env)
        {
            s_fSwapUV = args[0].AsBool(false);
            return AVSValue();
        }

        AVSValue __cdecl MaskSubCreate(AVSValue args, void* user_data, IScriptEnvironment* env)/*SIIFI*/
        {
            if (!args[0].Defined()) {
                env->ThrowError("MaskSub: You must specify a subtitle file to use");
            }
            if (!args[3].Defined() && !args[6].Defined()) {
                env->ThrowError("MaskSub: You must specify either FPS or a VFR timecodes file");
            }
            VFRTranslator* vfr = 0;
            if (args[6].Defined()) {
                vfr = GetVFRTranslator(args[6].AsString());
            }

            AVSValue rgb32("RGB32");
            AVSValue fps(args[3].AsFloat(25));
            AVSValue  tab[6] = {
                args[1],
                args[2],
                args[3],
                args[4],
                rgb32
            };
            AVSValue value(tab, 5);
            const char* nom[5] = {
                "width",
                "height",
                "fps",
                "length",
                "pixel_type"
            };
            AVSValue clip(env->Invoke("Blackness", value, nom));
            env->SetVar(env->SaveString("RGBA"), true);
            //return (DEBUG_NEW CTextSubAvisynthFilter(clip.AsClip(), env, args[0].AsString()));
            return (DEBUG_NEW CTextSubAvisynthFilter(
                        clip.AsClip(),
                        env,
                        args[0].AsString(),
                        args[5].AsInt(DEFAULT_CHARSET),
                        (float)args[3].AsFloat(-1),
                        vfr));
        }

        extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env)
        {
            env->AddFunction("VobSub", "cs", VobSubCreateS, 0);
            env->AddFunction("TextSub", "c[file]s[charset]i[fps]f[vfr]s", TextSubCreateGeneral, 0);
            env->AddFunction("TextSubSwapUV", "b", TextSubSwapUV, 0);
            env->AddFunction("MaskSub", "[file]s[width]i[height]i[fps]f[length]i[charset]i[vfr]s", MaskSubCreate, 0);
            env->SetVar(env->SaveString("RGBA"), false);
            return nullptr;
        }
    }

}

UINT_PTR CALLBACK OpenHookProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg) {
        case WM_NOTIFY: {
            OPENFILENAME* ofn = ((OFNOTIFY*)lParam)->lpOFN;

            if (((NMHDR*)lParam)->code == CDN_FILEOK) {
                ofn->lCustData = (LPARAM)CharSetList[SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0)];
            }

            break;
        }

        case WM_INITDIALOG: {
            SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);

            for (ptrdiff_t i = 0; i < CharSetLen; i++) {
                CString s;
                s.Format(_T("%s (%u)"), CharSetNames[i], CharSetList[i]);
                SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)s);
                if (CharSetList[i] == (int)((OPENFILENAME*)lParam)->lCustData) {
                    SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_SETCURSEL, i, 0);
                }
            }

            break;
        }

        default:
            break;
    }

    return FALSE;
}
