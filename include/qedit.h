

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0499 */
/* Compiler settings for qedit.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

/* verify that the <rpcsal.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCSAL_H_VERSION__
#define __REQUIRED_RPCSAL_H_VERSION__ 100
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __qedit_h__
#define __qedit_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IPropertySetter_FWD_DEFINED__
#define __IPropertySetter_FWD_DEFINED__
typedef interface IPropertySetter IPropertySetter;
#endif 	/* __IPropertySetter_FWD_DEFINED__ */


#ifndef __IDxtCompositor_FWD_DEFINED__
#define __IDxtCompositor_FWD_DEFINED__
typedef interface IDxtCompositor IDxtCompositor;
#endif 	/* __IDxtCompositor_FWD_DEFINED__ */


#ifndef __IDxtAlphaSetter_FWD_DEFINED__
#define __IDxtAlphaSetter_FWD_DEFINED__
typedef interface IDxtAlphaSetter IDxtAlphaSetter;
#endif 	/* __IDxtAlphaSetter_FWD_DEFINED__ */


#ifndef __IDxtJpeg_FWD_DEFINED__
#define __IDxtJpeg_FWD_DEFINED__
typedef interface IDxtJpeg IDxtJpeg;
#endif 	/* __IDxtJpeg_FWD_DEFINED__ */


#ifndef __IDxtKey_FWD_DEFINED__
#define __IDxtKey_FWD_DEFINED__
typedef interface IDxtKey IDxtKey;
#endif 	/* __IDxtKey_FWD_DEFINED__ */


#ifndef __IMediaLocator_FWD_DEFINED__
#define __IMediaLocator_FWD_DEFINED__
typedef interface IMediaLocator IMediaLocator;
#endif 	/* __IMediaLocator_FWD_DEFINED__ */


#ifndef __IMediaDet_FWD_DEFINED__
#define __IMediaDet_FWD_DEFINED__
typedef interface IMediaDet IMediaDet;
#endif 	/* __IMediaDet_FWD_DEFINED__ */


#ifndef __IGrfCache_FWD_DEFINED__
#define __IGrfCache_FWD_DEFINED__
typedef interface IGrfCache IGrfCache;
#endif 	/* __IGrfCache_FWD_DEFINED__ */


#ifndef __IRenderEngine_FWD_DEFINED__
#define __IRenderEngine_FWD_DEFINED__
typedef interface IRenderEngine IRenderEngine;
#endif 	/* __IRenderEngine_FWD_DEFINED__ */


#ifndef __IRenderEngine2_FWD_DEFINED__
#define __IRenderEngine2_FWD_DEFINED__
typedef interface IRenderEngine2 IRenderEngine2;
#endif 	/* __IRenderEngine2_FWD_DEFINED__ */


#ifndef __IFindCompressorCB_FWD_DEFINED__
#define __IFindCompressorCB_FWD_DEFINED__
typedef interface IFindCompressorCB IFindCompressorCB;
#endif 	/* __IFindCompressorCB_FWD_DEFINED__ */


#ifndef __ISmartRenderEngine_FWD_DEFINED__
#define __ISmartRenderEngine_FWD_DEFINED__
typedef interface ISmartRenderEngine ISmartRenderEngine;
#endif 	/* __ISmartRenderEngine_FWD_DEFINED__ */


#ifndef __IAMTimelineObj_FWD_DEFINED__
#define __IAMTimelineObj_FWD_DEFINED__
typedef interface IAMTimelineObj IAMTimelineObj;
#endif 	/* __IAMTimelineObj_FWD_DEFINED__ */


#ifndef __IAMTimelineEffectable_FWD_DEFINED__
#define __IAMTimelineEffectable_FWD_DEFINED__
typedef interface IAMTimelineEffectable IAMTimelineEffectable;
#endif 	/* __IAMTimelineEffectable_FWD_DEFINED__ */


#ifndef __IAMTimelineEffect_FWD_DEFINED__
#define __IAMTimelineEffect_FWD_DEFINED__
typedef interface IAMTimelineEffect IAMTimelineEffect;
#endif 	/* __IAMTimelineEffect_FWD_DEFINED__ */


#ifndef __IAMTimelineTransable_FWD_DEFINED__
#define __IAMTimelineTransable_FWD_DEFINED__
typedef interface IAMTimelineTransable IAMTimelineTransable;
#endif 	/* __IAMTimelineTransable_FWD_DEFINED__ */


#ifndef __IAMTimelineSplittable_FWD_DEFINED__
#define __IAMTimelineSplittable_FWD_DEFINED__
typedef interface IAMTimelineSplittable IAMTimelineSplittable;
#endif 	/* __IAMTimelineSplittable_FWD_DEFINED__ */


#ifndef __IAMTimelineTrans_FWD_DEFINED__
#define __IAMTimelineTrans_FWD_DEFINED__
typedef interface IAMTimelineTrans IAMTimelineTrans;
#endif 	/* __IAMTimelineTrans_FWD_DEFINED__ */


#ifndef __IAMTimelineSrc_FWD_DEFINED__
#define __IAMTimelineSrc_FWD_DEFINED__
typedef interface IAMTimelineSrc IAMTimelineSrc;
#endif 	/* __IAMTimelineSrc_FWD_DEFINED__ */


#ifndef __IAMTimelineTrack_FWD_DEFINED__
#define __IAMTimelineTrack_FWD_DEFINED__
typedef interface IAMTimelineTrack IAMTimelineTrack;
#endif 	/* __IAMTimelineTrack_FWD_DEFINED__ */


#ifndef __IAMTimelineVirtualTrack_FWD_DEFINED__
#define __IAMTimelineVirtualTrack_FWD_DEFINED__
typedef interface IAMTimelineVirtualTrack IAMTimelineVirtualTrack;
#endif 	/* __IAMTimelineVirtualTrack_FWD_DEFINED__ */


#ifndef __IAMTimelineComp_FWD_DEFINED__
#define __IAMTimelineComp_FWD_DEFINED__
typedef interface IAMTimelineComp IAMTimelineComp;
#endif 	/* __IAMTimelineComp_FWD_DEFINED__ */


#ifndef __IAMTimelineGroup_FWD_DEFINED__
#define __IAMTimelineGroup_FWD_DEFINED__
typedef interface IAMTimelineGroup IAMTimelineGroup;
#endif 	/* __IAMTimelineGroup_FWD_DEFINED__ */


#ifndef __IAMTimeline_FWD_DEFINED__
#define __IAMTimeline_FWD_DEFINED__
typedef interface IAMTimeline IAMTimeline;
#endif 	/* __IAMTimeline_FWD_DEFINED__ */


#ifndef __IXml2Dex_FWD_DEFINED__
#define __IXml2Dex_FWD_DEFINED__
typedef interface IXml2Dex IXml2Dex;
#endif 	/* __IXml2Dex_FWD_DEFINED__ */


#ifndef __IAMErrorLog_FWD_DEFINED__
#define __IAMErrorLog_FWD_DEFINED__
typedef interface IAMErrorLog IAMErrorLog;
#endif 	/* __IAMErrorLog_FWD_DEFINED__ */


#ifndef __IAMSetErrorLog_FWD_DEFINED__
#define __IAMSetErrorLog_FWD_DEFINED__
typedef interface IAMSetErrorLog IAMSetErrorLog;
#endif 	/* __IAMSetErrorLog_FWD_DEFINED__ */


#ifndef __ISampleGrabberCB_FWD_DEFINED__
#define __ISampleGrabberCB_FWD_DEFINED__
typedef interface ISampleGrabberCB ISampleGrabberCB;
#endif 	/* __ISampleGrabberCB_FWD_DEFINED__ */


#ifndef __ISampleGrabber_FWD_DEFINED__
#define __ISampleGrabber_FWD_DEFINED__
typedef interface ISampleGrabber ISampleGrabber;
#endif 	/* __ISampleGrabber_FWD_DEFINED__ */


#ifndef __IResize_FWD_DEFINED__
#define __IResize_FWD_DEFINED__
typedef interface IResize IResize;
#endif 	/* __IResize_FWD_DEFINED__ */


#ifndef __AMTimeline_FWD_DEFINED__
#define __AMTimeline_FWD_DEFINED__

#ifdef __cplusplus
typedef class AMTimeline AMTimeline;
#else
typedef struct AMTimeline AMTimeline;
#endif /* __cplusplus */

#endif 	/* __AMTimeline_FWD_DEFINED__ */


#ifndef __AMTimelineObj_FWD_DEFINED__
#define __AMTimelineObj_FWD_DEFINED__

#ifdef __cplusplus
typedef class AMTimelineObj AMTimelineObj;
#else
typedef struct AMTimelineObj AMTimelineObj;
#endif /* __cplusplus */

#endif 	/* __AMTimelineObj_FWD_DEFINED__ */


#ifndef __AMTimelineSrc_FWD_DEFINED__
#define __AMTimelineSrc_FWD_DEFINED__

#ifdef __cplusplus
typedef class AMTimelineSrc AMTimelineSrc;
#else
typedef struct AMTimelineSrc AMTimelineSrc;
#endif /* __cplusplus */

#endif 	/* __AMTimelineSrc_FWD_DEFINED__ */


#ifndef __AMTimelineTrack_FWD_DEFINED__
#define __AMTimelineTrack_FWD_DEFINED__

#ifdef __cplusplus
typedef class AMTimelineTrack AMTimelineTrack;
#else
typedef struct AMTimelineTrack AMTimelineTrack;
#endif /* __cplusplus */

#endif 	/* __AMTimelineTrack_FWD_DEFINED__ */


#ifndef __AMTimelineComp_FWD_DEFINED__
#define __AMTimelineComp_FWD_DEFINED__

#ifdef __cplusplus
typedef class AMTimelineComp AMTimelineComp;
#else
typedef struct AMTimelineComp AMTimelineComp;
#endif /* __cplusplus */

#endif 	/* __AMTimelineComp_FWD_DEFINED__ */


#ifndef __AMTimelineGroup_FWD_DEFINED__
#define __AMTimelineGroup_FWD_DEFINED__

#ifdef __cplusplus
typedef class AMTimelineGroup AMTimelineGroup;
#else
typedef struct AMTimelineGroup AMTimelineGroup;
#endif /* __cplusplus */

#endif 	/* __AMTimelineGroup_FWD_DEFINED__ */


#ifndef __AMTimelineTrans_FWD_DEFINED__
#define __AMTimelineTrans_FWD_DEFINED__

#ifdef __cplusplus
typedef class AMTimelineTrans AMTimelineTrans;
#else
typedef struct AMTimelineTrans AMTimelineTrans;
#endif /* __cplusplus */

#endif 	/* __AMTimelineTrans_FWD_DEFINED__ */


#ifndef __AMTimelineEffect_FWD_DEFINED__
#define __AMTimelineEffect_FWD_DEFINED__

#ifdef __cplusplus
typedef class AMTimelineEffect AMTimelineEffect;
#else
typedef struct AMTimelineEffect AMTimelineEffect;
#endif /* __cplusplus */

#endif 	/* __AMTimelineEffect_FWD_DEFINED__ */


#ifndef __RenderEngine_FWD_DEFINED__
#define __RenderEngine_FWD_DEFINED__

#ifdef __cplusplus
typedef class RenderEngine RenderEngine;
#else
typedef struct RenderEngine RenderEngine;
#endif /* __cplusplus */

#endif 	/* __RenderEngine_FWD_DEFINED__ */


#ifndef __SmartRenderEngine_FWD_DEFINED__
#define __SmartRenderEngine_FWD_DEFINED__

#ifdef __cplusplus
typedef class SmartRenderEngine SmartRenderEngine;
#else
typedef struct SmartRenderEngine SmartRenderEngine;
#endif /* __cplusplus */

#endif 	/* __SmartRenderEngine_FWD_DEFINED__ */


#ifndef __AudMixer_FWD_DEFINED__
#define __AudMixer_FWD_DEFINED__

#ifdef __cplusplus
typedef class AudMixer AudMixer;
#else
typedef struct AudMixer AudMixer;
#endif /* __cplusplus */

#endif 	/* __AudMixer_FWD_DEFINED__ */


#ifndef __Xml2Dex_FWD_DEFINED__
#define __Xml2Dex_FWD_DEFINED__

#ifdef __cplusplus
typedef class Xml2Dex Xml2Dex;
#else
typedef struct Xml2Dex Xml2Dex;
#endif /* __cplusplus */

#endif 	/* __Xml2Dex_FWD_DEFINED__ */


#ifndef __MediaLocator_FWD_DEFINED__
#define __MediaLocator_FWD_DEFINED__

#ifdef __cplusplus
typedef class MediaLocator MediaLocator;
#else
typedef struct MediaLocator MediaLocator;
#endif /* __cplusplus */

#endif 	/* __MediaLocator_FWD_DEFINED__ */


#ifndef __PropertySetter_FWD_DEFINED__
#define __PropertySetter_FWD_DEFINED__

#ifdef __cplusplus
typedef class PropertySetter PropertySetter;
#else
typedef struct PropertySetter PropertySetter;
#endif /* __cplusplus */

#endif 	/* __PropertySetter_FWD_DEFINED__ */


#ifndef __MediaDet_FWD_DEFINED__
#define __MediaDet_FWD_DEFINED__

#ifdef __cplusplus
typedef class MediaDet MediaDet;
#else
typedef struct MediaDet MediaDet;
#endif /* __cplusplus */

#endif 	/* __MediaDet_FWD_DEFINED__ */


#ifndef __SampleGrabber_FWD_DEFINED__
#define __SampleGrabber_FWD_DEFINED__

#ifdef __cplusplus
typedef class SampleGrabber SampleGrabber;
#else
typedef struct SampleGrabber SampleGrabber;
#endif /* __cplusplus */

#endif 	/* __SampleGrabber_FWD_DEFINED__ */


#ifndef __NullRenderer_FWD_DEFINED__
#define __NullRenderer_FWD_DEFINED__

#ifdef __cplusplus
typedef class NullRenderer NullRenderer;
#else
typedef struct NullRenderer NullRenderer;
#endif /* __cplusplus */

#endif 	/* __NullRenderer_FWD_DEFINED__ */


#ifndef __DxtCompositor_FWD_DEFINED__
#define __DxtCompositor_FWD_DEFINED__

#ifdef __cplusplus
typedef class DxtCompositor DxtCompositor;
#else
typedef struct DxtCompositor DxtCompositor;
#endif /* __cplusplus */

#endif 	/* __DxtCompositor_FWD_DEFINED__ */


#ifndef __DxtAlphaSetter_FWD_DEFINED__
#define __DxtAlphaSetter_FWD_DEFINED__

#ifdef __cplusplus
typedef class DxtAlphaSetter DxtAlphaSetter;
#else
typedef struct DxtAlphaSetter DxtAlphaSetter;
#endif /* __cplusplus */

#endif 	/* __DxtAlphaSetter_FWD_DEFINED__ */


#ifndef __DxtJpeg_FWD_DEFINED__
#define __DxtJpeg_FWD_DEFINED__

#ifdef __cplusplus
typedef class DxtJpeg DxtJpeg;
#else
typedef struct DxtJpeg DxtJpeg;
#endif /* __cplusplus */

#endif 	/* __DxtJpeg_FWD_DEFINED__ */


#ifndef __ColorSource_FWD_DEFINED__
#define __ColorSource_FWD_DEFINED__

#ifdef __cplusplus
typedef class ColorSource ColorSource;
#else
typedef struct ColorSource ColorSource;
#endif /* __cplusplus */

#endif 	/* __ColorSource_FWD_DEFINED__ */


#ifndef __DxtKey_FWD_DEFINED__
#define __DxtKey_FWD_DEFINED__

#ifdef __cplusplus
typedef class DxtKey DxtKey;
#else
typedef struct DxtKey DxtKey;
#endif /* __cplusplus */

#endif 	/* __DxtKey_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "dxtrans.h"
#include "amstream.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_qedit_0000_0000 */
/* [local] */ 


typedef /* [public] */ 
enum __MIDL___MIDL_itf_qedit_0000_0000_0001
    {	DEXTERF_JUMP	= 0,
	DEXTERF_INTERPOLATE	= ( DEXTERF_JUMP + 1 ) 
    } 	DEXTERF;

typedef /* [public][public][public][public] */ struct __MIDL___MIDL_itf_qedit_0000_0000_0002
    {
    BSTR Name;
    DISPID dispID;
    LONG nValues;
    } 	DEXTER_PARAM;

typedef /* [public][public][public][public] */ struct __MIDL___MIDL_itf_qedit_0000_0000_0003
    {
    VARIANT v;
    REFERENCE_TIME rt;
    DWORD dwInterp;
    } 	DEXTER_VALUE;


enum __MIDL___MIDL_itf_qedit_0000_0000_0004
    {	DEXTER_AUDIO_JUMP	= 0,
	DEXTER_AUDIO_INTERPOLATE	= ( DEXTER_AUDIO_JUMP + 1 ) 
    } ;
typedef /* [public] */ struct __MIDL___MIDL_itf_qedit_0000_0000_0005
    {
    REFERENCE_TIME rtEnd;
    double dLevel;
    BOOL bMethod;
    } 	DEXTER_AUDIO_VOLUMEENVELOPE;


enum __MIDL___MIDL_itf_qedit_0000_0000_0006
    {	TIMELINE_INSERT_MODE_INSERT	= 1,
	TIMELINE_INSERT_MODE_OVERLAY	= 2
    } ;
typedef /* [public][public][public][public][public][public][public][public] */ 
enum __MIDL___MIDL_itf_qedit_0000_0000_0007
    {	TIMELINE_MAJOR_TYPE_COMPOSITE	= 1,
	TIMELINE_MAJOR_TYPE_TRACK	= 2,
	TIMELINE_MAJOR_TYPE_SOURCE	= 4,
	TIMELINE_MAJOR_TYPE_TRANSITION	= 8,
	TIMELINE_MAJOR_TYPE_EFFECT	= 16,
	TIMELINE_MAJOR_TYPE_GROUP	= 128
    } 	TIMELINE_MAJOR_TYPE;

typedef /* [public] */ 
enum __MIDL___MIDL_itf_qedit_0000_0000_0008
    {	DEXTERF_BOUNDING	= -1,
	DEXTERF_EXACTLY_AT	= 0,
	DEXTERF_FORWARDS	= 1
    } 	DEXTERF_TRACK_SEARCH_FLAGS;

typedef struct _SCompFmt0
    {
    long nFormatId;
    AM_MEDIA_TYPE MediaType;
    } 	SCompFmt0;


enum __MIDL___MIDL_itf_qedit_0000_0000_0009
    {	RESIZEF_STRETCH	= 0,
	RESIZEF_CROP	= ( RESIZEF_STRETCH + 1 ) ,
	RESIZEF_PRESERVEASPECTRATIO	= ( RESIZEF_CROP + 1 ) ,
	RESIZEF_PRESERVEASPECTRATIO_NOLETTERBOX	= ( RESIZEF_PRESERVEASPECTRATIO + 1 ) 
    } ;

enum __MIDL___MIDL_itf_qedit_0000_0000_0010
    {	CONNECTF_DYNAMIC_NONE	= 0,
	CONNECTF_DYNAMIC_SOURCES	= 0x1,
	CONNECTF_DYNAMIC_EFFECTS	= 0x2
    } ;

enum __MIDL___MIDL_itf_qedit_0000_0000_0011
    {	SFN_VALIDATEF_CHECK	= 0x1,
	SFN_VALIDATEF_POPUP	= 0x2,
	SFN_VALIDATEF_TELLME	= 0x4,
	SFN_VALIDATEF_REPLACE	= 0x8,
	SFN_VALIDATEF_USELOCAL	= 0x10,
	SFN_VALIDATEF_NOFIND	= 0x20,
	SFN_VALIDATEF_IGNOREMUTED	= 0x40,
	SFN_VALIDATEF_END	= ( SFN_VALIDATEF_IGNOREMUTED + 1 ) 
    } ;

enum __MIDL___MIDL_itf_qedit_0000_0000_0012
    {	DXTKEY_RGB	= 0,
	DXTKEY_NONRED	= ( DXTKEY_RGB + 1 ) ,
	DXTKEY_LUMINANCE	= ( DXTKEY_NONRED + 1 ) ,
	DXTKEY_ALPHA	= ( DXTKEY_LUMINANCE + 1 ) ,
	DXTKEY_HUE	= ( DXTKEY_ALPHA + 1 ) 
    } ;


extern RPC_IF_HANDLE __MIDL_itf_qedit_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_qedit_0000_0000_v0_0_s_ifspec;

#ifndef __IPropertySetter_INTERFACE_DEFINED__
#define __IPropertySetter_INTERFACE_DEFINED__

/* interface IPropertySetter */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IPropertySetter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AE9472BD-B0C3-11D2-8D24-00A0C9441E20")
    IPropertySetter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE LoadXML( 
            /* [in] */ __RPC__in_opt IUnknown *pxml) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PrintXML( 
            /* [out] */ __RPC__out char *pszXML,
            /* [in] */ int cbXML,
            /* [out] */ __RPC__out int *pcbPrinted,
            /* [in] */ int indent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CloneProps( 
            /* [out] */ __RPC__deref_out_opt IPropertySetter **ppSetter,
            /* [in] */ REFERENCE_TIME rtStart,
            /* [in] */ REFERENCE_TIME rtStop) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddProp( 
            /* [in] */ DEXTER_PARAM Param,
            /* [in] */ __RPC__in DEXTER_VALUE *paValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProps( 
            /* [out] */ __RPC__out LONG *pcParams,
            /* [out] */ __RPC__deref_out_opt DEXTER_PARAM **paParam,
            /* [out] */ __RPC__deref_out_opt DEXTER_VALUE **paValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeProps( 
            /* [in] */ LONG cParams,
            /* [in] */ __RPC__in DEXTER_PARAM *paParam,
            /* [in] */ __RPC__in DEXTER_VALUE *paValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearProps( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveToBlob( 
            /* [out] */ __RPC__out LONG *pcSize,
            /* [out] */ __RPC__deref_out_opt BYTE **ppb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadFromBlob( 
            /* [in] */ LONG cSize,
            /* [in] */ __RPC__in BYTE *pb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProps( 
            /* [in] */ __RPC__in_opt IUnknown *pTarget,
            /* [in] */ REFERENCE_TIME rtNow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PrintXMLW( 
            /* [out] */ __RPC__out WCHAR *pszXML,
            /* [in] */ int cchXML,
            /* [out] */ __RPC__out int *pcchPrinted,
            /* [in] */ int indent) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPropertySetterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertySetter * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertySetter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertySetter * This);
        
        HRESULT ( STDMETHODCALLTYPE *LoadXML )( 
            IPropertySetter * This,
            /* [in] */ __RPC__in_opt IUnknown *pxml);
        
        HRESULT ( STDMETHODCALLTYPE *PrintXML )( 
            IPropertySetter * This,
            /* [out] */ __RPC__out char *pszXML,
            /* [in] */ int cbXML,
            /* [out] */ __RPC__out int *pcbPrinted,
            /* [in] */ int indent);
        
        HRESULT ( STDMETHODCALLTYPE *CloneProps )( 
            IPropertySetter * This,
            /* [out] */ __RPC__deref_out_opt IPropertySetter **ppSetter,
            /* [in] */ REFERENCE_TIME rtStart,
            /* [in] */ REFERENCE_TIME rtStop);
        
        HRESULT ( STDMETHODCALLTYPE *AddProp )( 
            IPropertySetter * This,
            /* [in] */ DEXTER_PARAM Param,
            /* [in] */ __RPC__in DEXTER_VALUE *paValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetProps )( 
            IPropertySetter * This,
            /* [out] */ __RPC__out LONG *pcParams,
            /* [out] */ __RPC__deref_out_opt DEXTER_PARAM **paParam,
            /* [out] */ __RPC__deref_out_opt DEXTER_VALUE **paValue);
        
        HRESULT ( STDMETHODCALLTYPE *FreeProps )( 
            IPropertySetter * This,
            /* [in] */ LONG cParams,
            /* [in] */ __RPC__in DEXTER_PARAM *paParam,
            /* [in] */ __RPC__in DEXTER_VALUE *paValue);
        
        HRESULT ( STDMETHODCALLTYPE *ClearProps )( 
            IPropertySetter * This);
        
        HRESULT ( STDMETHODCALLTYPE *SaveToBlob )( 
            IPropertySetter * This,
            /* [out] */ __RPC__out LONG *pcSize,
            /* [out] */ __RPC__deref_out_opt BYTE **ppb);
        
        HRESULT ( STDMETHODCALLTYPE *LoadFromBlob )( 
            IPropertySetter * This,
            /* [in] */ LONG cSize,
            /* [in] */ __RPC__in BYTE *pb);
        
        HRESULT ( STDMETHODCALLTYPE *SetProps )( 
            IPropertySetter * This,
            /* [in] */ __RPC__in_opt IUnknown *pTarget,
            /* [in] */ REFERENCE_TIME rtNow);
        
        HRESULT ( STDMETHODCALLTYPE *PrintXMLW )( 
            IPropertySetter * This,
            /* [out] */ __RPC__out WCHAR *pszXML,
            /* [in] */ int cchXML,
            /* [out] */ __RPC__out int *pcchPrinted,
            /* [in] */ int indent);
        
        END_INTERFACE
    } IPropertySetterVtbl;

    interface IPropertySetter
    {
        CONST_VTBL struct IPropertySetterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertySetter_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPropertySetter_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPropertySetter_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPropertySetter_LoadXML(This,pxml)	\
    ( (This)->lpVtbl -> LoadXML(This,pxml) ) 

#define IPropertySetter_PrintXML(This,pszXML,cbXML,pcbPrinted,indent)	\
    ( (This)->lpVtbl -> PrintXML(This,pszXML,cbXML,pcbPrinted,indent) ) 

#define IPropertySetter_CloneProps(This,ppSetter,rtStart,rtStop)	\
    ( (This)->lpVtbl -> CloneProps(This,ppSetter,rtStart,rtStop) ) 

#define IPropertySetter_AddProp(This,Param,paValue)	\
    ( (This)->lpVtbl -> AddProp(This,Param,paValue) ) 

#define IPropertySetter_GetProps(This,pcParams,paParam,paValue)	\
    ( (This)->lpVtbl -> GetProps(This,pcParams,paParam,paValue) ) 

#define IPropertySetter_FreeProps(This,cParams,paParam,paValue)	\
    ( (This)->lpVtbl -> FreeProps(This,cParams,paParam,paValue) ) 

#define IPropertySetter_ClearProps(This)	\
    ( (This)->lpVtbl -> ClearProps(This) ) 

#define IPropertySetter_SaveToBlob(This,pcSize,ppb)	\
    ( (This)->lpVtbl -> SaveToBlob(This,pcSize,ppb) ) 

#define IPropertySetter_LoadFromBlob(This,cSize,pb)	\
    ( (This)->lpVtbl -> LoadFromBlob(This,cSize,pb) ) 

#define IPropertySetter_SetProps(This,pTarget,rtNow)	\
    ( (This)->lpVtbl -> SetProps(This,pTarget,rtNow) ) 

#define IPropertySetter_PrintXMLW(This,pszXML,cchXML,pcchPrinted,indent)	\
    ( (This)->lpVtbl -> PrintXMLW(This,pszXML,cchXML,pcchPrinted,indent) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPropertySetter_INTERFACE_DEFINED__ */


#ifndef __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtCompositor_INTERFACE_DEFINED__

/* interface IDxtCompositor */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IDxtCompositor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BB44391E-6ABD-422f-9E2E-385C9DFF51FC")
    IDxtCompositor : public IDXEffect
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OffsetX( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_OffsetX( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OffsetY( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_OffsetY( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Width( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Width( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Height( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SrcOffsetX( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SrcOffsetX( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SrcOffsetY( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SrcOffsetY( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SrcWidth( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SrcWidth( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SrcHeight( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SrcHeight( 
            /* [in] */ long newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDxtCompositorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDxtCompositor * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDxtCompositor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDxtCompositor * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDxtCompositor * This,
            /* [out] */ __RPC__out UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDxtCompositor * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDxtCompositor * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDxtCompositor * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Capabilities )( 
            IDxtCompositor * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Progress )( 
            IDxtCompositor * This,
            /* [retval][out] */ __RPC__out float *pVal);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Progress )( 
            IDxtCompositor * This,
            /* [in] */ float newVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StepResolution )( 
            IDxtCompositor * This,
            /* [retval][out] */ __RPC__out float *pVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IDxtCompositor * This,
            /* [retval][out] */ __RPC__out float *pVal);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Duration )( 
            IDxtCompositor * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OffsetX )( 
            IDxtCompositor * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_OffsetX )( 
            IDxtCompositor * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OffsetY )( 
            IDxtCompositor * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_OffsetY )( 
            IDxtCompositor * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Width )( 
            IDxtCompositor * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Width )( 
            IDxtCompositor * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Height )( 
            IDxtCompositor * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Height )( 
            IDxtCompositor * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SrcOffsetX )( 
            IDxtCompositor * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SrcOffsetX )( 
            IDxtCompositor * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SrcOffsetY )( 
            IDxtCompositor * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SrcOffsetY )( 
            IDxtCompositor * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SrcWidth )( 
            IDxtCompositor * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SrcWidth )( 
            IDxtCompositor * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SrcHeight )( 
            IDxtCompositor * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SrcHeight )( 
            IDxtCompositor * This,
            /* [in] */ long newVal);
        
        END_INTERFACE
    } IDxtCompositorVtbl;

    interface IDxtCompositor
    {
        CONST_VTBL struct IDxtCompositorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDxtCompositor_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDxtCompositor_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDxtCompositor_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDxtCompositor_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IDxtCompositor_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IDxtCompositor_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IDxtCompositor_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IDxtCompositor_get_Capabilities(This,pVal)	\
    ( (This)->lpVtbl -> get_Capabilities(This,pVal) ) 

#define IDxtCompositor_get_Progress(This,pVal)	\
    ( (This)->lpVtbl -> get_Progress(This,pVal) ) 

#define IDxtCompositor_put_Progress(This,newVal)	\
    ( (This)->lpVtbl -> put_Progress(This,newVal) ) 

#define IDxtCompositor_get_StepResolution(This,pVal)	\
    ( (This)->lpVtbl -> get_StepResolution(This,pVal) ) 

#define IDxtCompositor_get_Duration(This,pVal)	\
    ( (This)->lpVtbl -> get_Duration(This,pVal) ) 

#define IDxtCompositor_put_Duration(This,newVal)	\
    ( (This)->lpVtbl -> put_Duration(This,newVal) ) 


#define IDxtCompositor_get_OffsetX(This,pVal)	\
    ( (This)->lpVtbl -> get_OffsetX(This,pVal) ) 

#define IDxtCompositor_put_OffsetX(This,newVal)	\
    ( (This)->lpVtbl -> put_OffsetX(This,newVal) ) 

#define IDxtCompositor_get_OffsetY(This,pVal)	\
    ( (This)->lpVtbl -> get_OffsetY(This,pVal) ) 

#define IDxtCompositor_put_OffsetY(This,newVal)	\
    ( (This)->lpVtbl -> put_OffsetY(This,newVal) ) 

#define IDxtCompositor_get_Width(This,pVal)	\
    ( (This)->lpVtbl -> get_Width(This,pVal) ) 

#define IDxtCompositor_put_Width(This,newVal)	\
    ( (This)->lpVtbl -> put_Width(This,newVal) ) 

#define IDxtCompositor_get_Height(This,pVal)	\
    ( (This)->lpVtbl -> get_Height(This,pVal) ) 

#define IDxtCompositor_put_Height(This,newVal)	\
    ( (This)->lpVtbl -> put_Height(This,newVal) ) 

#define IDxtCompositor_get_SrcOffsetX(This,pVal)	\
    ( (This)->lpVtbl -> get_SrcOffsetX(This,pVal) ) 

#define IDxtCompositor_put_SrcOffsetX(This,newVal)	\
    ( (This)->lpVtbl -> put_SrcOffsetX(This,newVal) ) 

#define IDxtCompositor_get_SrcOffsetY(This,pVal)	\
    ( (This)->lpVtbl -> get_SrcOffsetY(This,pVal) ) 

#define IDxtCompositor_put_SrcOffsetY(This,newVal)	\
    ( (This)->lpVtbl -> put_SrcOffsetY(This,newVal) ) 

#define IDxtCompositor_get_SrcWidth(This,pVal)	\
    ( (This)->lpVtbl -> get_SrcWidth(This,pVal) ) 

#define IDxtCompositor_put_SrcWidth(This,newVal)	\
    ( (This)->lpVtbl -> put_SrcWidth(This,newVal) ) 

#define IDxtCompositor_get_SrcHeight(This,pVal)	\
    ( (This)->lpVtbl -> get_SrcHeight(This,pVal) ) 

#define IDxtCompositor_put_SrcHeight(This,newVal)	\
    ( (This)->lpVtbl -> put_SrcHeight(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDxtCompositor_INTERFACE_DEFINED__ */


#ifndef __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__

/* interface IDxtAlphaSetter */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IDxtAlphaSetter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4EE9EAD9-DA4D-43d0-9383-06B90C08B12B")
    IDxtAlphaSetter : public IDXEffect
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Alpha( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Alpha( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AlphaRamp( 
            /* [retval][out] */ __RPC__out double *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AlphaRamp( 
            /* [in] */ double newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDxtAlphaSetterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDxtAlphaSetter * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDxtAlphaSetter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDxtAlphaSetter * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDxtAlphaSetter * This,
            /* [out] */ __RPC__out UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDxtAlphaSetter * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDxtAlphaSetter * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDxtAlphaSetter * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Capabilities )( 
            IDxtAlphaSetter * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Progress )( 
            IDxtAlphaSetter * This,
            /* [retval][out] */ __RPC__out float *pVal);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Progress )( 
            IDxtAlphaSetter * This,
            /* [in] */ float newVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StepResolution )( 
            IDxtAlphaSetter * This,
            /* [retval][out] */ __RPC__out float *pVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IDxtAlphaSetter * This,
            /* [retval][out] */ __RPC__out float *pVal);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Duration )( 
            IDxtAlphaSetter * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Alpha )( 
            IDxtAlphaSetter * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Alpha )( 
            IDxtAlphaSetter * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AlphaRamp )( 
            IDxtAlphaSetter * This,
            /* [retval][out] */ __RPC__out double *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AlphaRamp )( 
            IDxtAlphaSetter * This,
            /* [in] */ double newVal);
        
        END_INTERFACE
    } IDxtAlphaSetterVtbl;

    interface IDxtAlphaSetter
    {
        CONST_VTBL struct IDxtAlphaSetterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDxtAlphaSetter_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDxtAlphaSetter_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDxtAlphaSetter_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDxtAlphaSetter_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IDxtAlphaSetter_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IDxtAlphaSetter_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IDxtAlphaSetter_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IDxtAlphaSetter_get_Capabilities(This,pVal)	\
    ( (This)->lpVtbl -> get_Capabilities(This,pVal) ) 

#define IDxtAlphaSetter_get_Progress(This,pVal)	\
    ( (This)->lpVtbl -> get_Progress(This,pVal) ) 

#define IDxtAlphaSetter_put_Progress(This,newVal)	\
    ( (This)->lpVtbl -> put_Progress(This,newVal) ) 

#define IDxtAlphaSetter_get_StepResolution(This,pVal)	\
    ( (This)->lpVtbl -> get_StepResolution(This,pVal) ) 

#define IDxtAlphaSetter_get_Duration(This,pVal)	\
    ( (This)->lpVtbl -> get_Duration(This,pVal) ) 

#define IDxtAlphaSetter_put_Duration(This,newVal)	\
    ( (This)->lpVtbl -> put_Duration(This,newVal) ) 


#define IDxtAlphaSetter_get_Alpha(This,pVal)	\
    ( (This)->lpVtbl -> get_Alpha(This,pVal) ) 

#define IDxtAlphaSetter_put_Alpha(This,newVal)	\
    ( (This)->lpVtbl -> put_Alpha(This,newVal) ) 

#define IDxtAlphaSetter_get_AlphaRamp(This,pVal)	\
    ( (This)->lpVtbl -> get_AlphaRamp(This,pVal) ) 

#define IDxtAlphaSetter_put_AlphaRamp(This,newVal)	\
    ( (This)->lpVtbl -> put_AlphaRamp(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDxtAlphaSetter_INTERFACE_DEFINED__ */


#ifndef __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__

/* interface IDxtJpeg */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IDxtJpeg;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DE75D011-7A65-11D2-8CEA-00A0C9441E20")
    IDxtJpeg : public IDXEffect
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MaskNum( 
            /* [retval][out] */ __RPC__out long *__MIDL__IDxtJpeg0000) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MaskNum( 
            /* [in] */ long __MIDL__IDxtJpeg0001) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MaskName( 
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MaskName( 
            /* [in] */ __RPC__in BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ScaleX( 
            /* [retval][out] */ __RPC__out double *__MIDL__IDxtJpeg0002) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ScaleX( 
            /* [in] */ double __MIDL__IDxtJpeg0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ScaleY( 
            /* [retval][out] */ __RPC__out double *__MIDL__IDxtJpeg0004) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ScaleY( 
            /* [in] */ double __MIDL__IDxtJpeg0005) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OffsetX( 
            /* [retval][out] */ __RPC__out long *__MIDL__IDxtJpeg0006) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_OffsetX( 
            /* [in] */ long __MIDL__IDxtJpeg0007) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OffsetY( 
            /* [retval][out] */ __RPC__out long *__MIDL__IDxtJpeg0008) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_OffsetY( 
            /* [in] */ long __MIDL__IDxtJpeg0009) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReplicateX( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReplicateX( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReplicateY( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReplicateY( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BorderColor( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BorderColor( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BorderWidth( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BorderWidth( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BorderSoftness( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BorderSoftness( 
            /* [in] */ long newVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ApplyChanges( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadDefSettings( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDxtJpegVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDxtJpeg * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDxtJpeg * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDxtJpeg * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDxtJpeg * This,
            /* [out] */ __RPC__out UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDxtJpeg * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDxtJpeg * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDxtJpeg * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Capabilities )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Progress )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__out float *pVal);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Progress )( 
            IDxtJpeg * This,
            /* [in] */ float newVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StepResolution )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__out float *pVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__out float *pVal);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Duration )( 
            IDxtJpeg * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MaskNum )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__out long *__MIDL__IDxtJpeg0000);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_MaskNum )( 
            IDxtJpeg * This,
            /* [in] */ long __MIDL__IDxtJpeg0001);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MaskName )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_MaskName )( 
            IDxtJpeg * This,
            /* [in] */ __RPC__in BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ScaleX )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__out double *__MIDL__IDxtJpeg0002);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ScaleX )( 
            IDxtJpeg * This,
            /* [in] */ double __MIDL__IDxtJpeg0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ScaleY )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__out double *__MIDL__IDxtJpeg0004);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ScaleY )( 
            IDxtJpeg * This,
            /* [in] */ double __MIDL__IDxtJpeg0005);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OffsetX )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__out long *__MIDL__IDxtJpeg0006);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_OffsetX )( 
            IDxtJpeg * This,
            /* [in] */ long __MIDL__IDxtJpeg0007);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OffsetY )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__out long *__MIDL__IDxtJpeg0008);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_OffsetY )( 
            IDxtJpeg * This,
            /* [in] */ long __MIDL__IDxtJpeg0009);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReplicateX )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReplicateX )( 
            IDxtJpeg * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReplicateY )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReplicateY )( 
            IDxtJpeg * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BorderColor )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BorderColor )( 
            IDxtJpeg * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BorderWidth )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BorderWidth )( 
            IDxtJpeg * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BorderSoftness )( 
            IDxtJpeg * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BorderSoftness )( 
            IDxtJpeg * This,
            /* [in] */ long newVal);
        
        HRESULT ( STDMETHODCALLTYPE *ApplyChanges )( 
            IDxtJpeg * This);
        
        HRESULT ( STDMETHODCALLTYPE *LoadDefSettings )( 
            IDxtJpeg * This);
        
        END_INTERFACE
    } IDxtJpegVtbl;

    interface IDxtJpeg
    {
        CONST_VTBL struct IDxtJpegVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDxtJpeg_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDxtJpeg_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDxtJpeg_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDxtJpeg_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IDxtJpeg_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IDxtJpeg_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IDxtJpeg_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IDxtJpeg_get_Capabilities(This,pVal)	\
    ( (This)->lpVtbl -> get_Capabilities(This,pVal) ) 

#define IDxtJpeg_get_Progress(This,pVal)	\
    ( (This)->lpVtbl -> get_Progress(This,pVal) ) 

#define IDxtJpeg_put_Progress(This,newVal)	\
    ( (This)->lpVtbl -> put_Progress(This,newVal) ) 

#define IDxtJpeg_get_StepResolution(This,pVal)	\
    ( (This)->lpVtbl -> get_StepResolution(This,pVal) ) 

#define IDxtJpeg_get_Duration(This,pVal)	\
    ( (This)->lpVtbl -> get_Duration(This,pVal) ) 

#define IDxtJpeg_put_Duration(This,newVal)	\
    ( (This)->lpVtbl -> put_Duration(This,newVal) ) 


#define IDxtJpeg_get_MaskNum(This,__MIDL__IDxtJpeg0000)	\
    ( (This)->lpVtbl -> get_MaskNum(This,__MIDL__IDxtJpeg0000) ) 

#define IDxtJpeg_put_MaskNum(This,__MIDL__IDxtJpeg0001)	\
    ( (This)->lpVtbl -> put_MaskNum(This,__MIDL__IDxtJpeg0001) ) 

#define IDxtJpeg_get_MaskName(This,pVal)	\
    ( (This)->lpVtbl -> get_MaskName(This,pVal) ) 

#define IDxtJpeg_put_MaskName(This,newVal)	\
    ( (This)->lpVtbl -> put_MaskName(This,newVal) ) 

#define IDxtJpeg_get_ScaleX(This,__MIDL__IDxtJpeg0002)	\
    ( (This)->lpVtbl -> get_ScaleX(This,__MIDL__IDxtJpeg0002) ) 

#define IDxtJpeg_put_ScaleX(This,__MIDL__IDxtJpeg0003)	\
    ( (This)->lpVtbl -> put_ScaleX(This,__MIDL__IDxtJpeg0003) ) 

#define IDxtJpeg_get_ScaleY(This,__MIDL__IDxtJpeg0004)	\
    ( (This)->lpVtbl -> get_ScaleY(This,__MIDL__IDxtJpeg0004) ) 

#define IDxtJpeg_put_ScaleY(This,__MIDL__IDxtJpeg0005)	\
    ( (This)->lpVtbl -> put_ScaleY(This,__MIDL__IDxtJpeg0005) ) 

#define IDxtJpeg_get_OffsetX(This,__MIDL__IDxtJpeg0006)	\
    ( (This)->lpVtbl -> get_OffsetX(This,__MIDL__IDxtJpeg0006) ) 

#define IDxtJpeg_put_OffsetX(This,__MIDL__IDxtJpeg0007)	\
    ( (This)->lpVtbl -> put_OffsetX(This,__MIDL__IDxtJpeg0007) ) 

#define IDxtJpeg_get_OffsetY(This,__MIDL__IDxtJpeg0008)	\
    ( (This)->lpVtbl -> get_OffsetY(This,__MIDL__IDxtJpeg0008) ) 

#define IDxtJpeg_put_OffsetY(This,__MIDL__IDxtJpeg0009)	\
    ( (This)->lpVtbl -> put_OffsetY(This,__MIDL__IDxtJpeg0009) ) 

#define IDxtJpeg_get_ReplicateX(This,pVal)	\
    ( (This)->lpVtbl -> get_ReplicateX(This,pVal) ) 

#define IDxtJpeg_put_ReplicateX(This,newVal)	\
    ( (This)->lpVtbl -> put_ReplicateX(This,newVal) ) 

#define IDxtJpeg_get_ReplicateY(This,pVal)	\
    ( (This)->lpVtbl -> get_ReplicateY(This,pVal) ) 

#define IDxtJpeg_put_ReplicateY(This,newVal)	\
    ( (This)->lpVtbl -> put_ReplicateY(This,newVal) ) 

#define IDxtJpeg_get_BorderColor(This,pVal)	\
    ( (This)->lpVtbl -> get_BorderColor(This,pVal) ) 

#define IDxtJpeg_put_BorderColor(This,newVal)	\
    ( (This)->lpVtbl -> put_BorderColor(This,newVal) ) 

#define IDxtJpeg_get_BorderWidth(This,pVal)	\
    ( (This)->lpVtbl -> get_BorderWidth(This,pVal) ) 

#define IDxtJpeg_put_BorderWidth(This,newVal)	\
    ( (This)->lpVtbl -> put_BorderWidth(This,newVal) ) 

#define IDxtJpeg_get_BorderSoftness(This,pVal)	\
    ( (This)->lpVtbl -> get_BorderSoftness(This,pVal) ) 

#define IDxtJpeg_put_BorderSoftness(This,newVal)	\
    ( (This)->lpVtbl -> put_BorderSoftness(This,newVal) ) 

#define IDxtJpeg_ApplyChanges(This)	\
    ( (This)->lpVtbl -> ApplyChanges(This) ) 

#define IDxtJpeg_LoadDefSettings(This)	\
    ( (This)->lpVtbl -> LoadDefSettings(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDxtJpeg_INTERFACE_DEFINED__ */


#ifndef __IDxtKey_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__

/* interface IDxtKey */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IDxtKey;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3255de56-38fb-4901-b980-94b438010d7b")
    IDxtKey : public IDXEffect
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_KeyType( 
            /* [retval][out] */ __RPC__out int *__MIDL__IDxtKey0000) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_KeyType( 
            /* [in] */ int __MIDL__IDxtKey0001) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Hue( 
            /* [retval][out] */ __RPC__out int *__MIDL__IDxtKey0002) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Hue( 
            /* [in] */ int __MIDL__IDxtKey0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Luminance( 
            /* [retval][out] */ __RPC__out int *__MIDL__IDxtKey0004) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Luminance( 
            /* [in] */ int __MIDL__IDxtKey0005) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RGB( 
            /* [retval][out] */ __RPC__out DWORD *__MIDL__IDxtKey0006) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RGB( 
            /* [in] */ DWORD __MIDL__IDxtKey0007) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Similarity( 
            /* [retval][out] */ __RPC__out int *__MIDL__IDxtKey0008) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Similarity( 
            /* [in] */ int __MIDL__IDxtKey0009) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Invert( 
            /* [retval][out] */ __RPC__out BOOL *__MIDL__IDxtKey0010) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Invert( 
            /* [in] */ BOOL __MIDL__IDxtKey0011) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDxtKeyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDxtKey * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDxtKey * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDxtKey * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDxtKey * This,
            /* [out] */ __RPC__out UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDxtKey * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDxtKey * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDxtKey * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Capabilities )( 
            IDxtKey * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Progress )( 
            IDxtKey * This,
            /* [retval][out] */ __RPC__out float *pVal);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Progress )( 
            IDxtKey * This,
            /* [in] */ float newVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StepResolution )( 
            IDxtKey * This,
            /* [retval][out] */ __RPC__out float *pVal);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IDxtKey * This,
            /* [retval][out] */ __RPC__out float *pVal);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Duration )( 
            IDxtKey * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_KeyType )( 
            IDxtKey * This,
            /* [retval][out] */ __RPC__out int *__MIDL__IDxtKey0000);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_KeyType )( 
            IDxtKey * This,
            /* [in] */ int __MIDL__IDxtKey0001);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Hue )( 
            IDxtKey * This,
            /* [retval][out] */ __RPC__out int *__MIDL__IDxtKey0002);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Hue )( 
            IDxtKey * This,
            /* [in] */ int __MIDL__IDxtKey0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Luminance )( 
            IDxtKey * This,
            /* [retval][out] */ __RPC__out int *__MIDL__IDxtKey0004);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Luminance )( 
            IDxtKey * This,
            /* [in] */ int __MIDL__IDxtKey0005);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RGB )( 
            IDxtKey * This,
            /* [retval][out] */ __RPC__out DWORD *__MIDL__IDxtKey0006);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RGB )( 
            IDxtKey * This,
            /* [in] */ DWORD __MIDL__IDxtKey0007);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Similarity )( 
            IDxtKey * This,
            /* [retval][out] */ __RPC__out int *__MIDL__IDxtKey0008);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Similarity )( 
            IDxtKey * This,
            /* [in] */ int __MIDL__IDxtKey0009);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Invert )( 
            IDxtKey * This,
            /* [retval][out] */ __RPC__out BOOL *__MIDL__IDxtKey0010);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Invert )( 
            IDxtKey * This,
            /* [in] */ BOOL __MIDL__IDxtKey0011);
        
        END_INTERFACE
    } IDxtKeyVtbl;

    interface IDxtKey
    {
        CONST_VTBL struct IDxtKeyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDxtKey_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDxtKey_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDxtKey_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDxtKey_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IDxtKey_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IDxtKey_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IDxtKey_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IDxtKey_get_Capabilities(This,pVal)	\
    ( (This)->lpVtbl -> get_Capabilities(This,pVal) ) 

#define IDxtKey_get_Progress(This,pVal)	\
    ( (This)->lpVtbl -> get_Progress(This,pVal) ) 

#define IDxtKey_put_Progress(This,newVal)	\
    ( (This)->lpVtbl -> put_Progress(This,newVal) ) 

#define IDxtKey_get_StepResolution(This,pVal)	\
    ( (This)->lpVtbl -> get_StepResolution(This,pVal) ) 

#define IDxtKey_get_Duration(This,pVal)	\
    ( (This)->lpVtbl -> get_Duration(This,pVal) ) 

#define IDxtKey_put_Duration(This,newVal)	\
    ( (This)->lpVtbl -> put_Duration(This,newVal) ) 


#define IDxtKey_get_KeyType(This,__MIDL__IDxtKey0000)	\
    ( (This)->lpVtbl -> get_KeyType(This,__MIDL__IDxtKey0000) ) 

#define IDxtKey_put_KeyType(This,__MIDL__IDxtKey0001)	\
    ( (This)->lpVtbl -> put_KeyType(This,__MIDL__IDxtKey0001) ) 

#define IDxtKey_get_Hue(This,__MIDL__IDxtKey0002)	\
    ( (This)->lpVtbl -> get_Hue(This,__MIDL__IDxtKey0002) ) 

#define IDxtKey_put_Hue(This,__MIDL__IDxtKey0003)	\
    ( (This)->lpVtbl -> put_Hue(This,__MIDL__IDxtKey0003) ) 

#define IDxtKey_get_Luminance(This,__MIDL__IDxtKey0004)	\
    ( (This)->lpVtbl -> get_Luminance(This,__MIDL__IDxtKey0004) ) 

#define IDxtKey_put_Luminance(This,__MIDL__IDxtKey0005)	\
    ( (This)->lpVtbl -> put_Luminance(This,__MIDL__IDxtKey0005) ) 

#define IDxtKey_get_RGB(This,__MIDL__IDxtKey0006)	\
    ( (This)->lpVtbl -> get_RGB(This,__MIDL__IDxtKey0006) ) 

#define IDxtKey_put_RGB(This,__MIDL__IDxtKey0007)	\
    ( (This)->lpVtbl -> put_RGB(This,__MIDL__IDxtKey0007) ) 

#define IDxtKey_get_Similarity(This,__MIDL__IDxtKey0008)	\
    ( (This)->lpVtbl -> get_Similarity(This,__MIDL__IDxtKey0008) ) 

#define IDxtKey_put_Similarity(This,__MIDL__IDxtKey0009)	\
    ( (This)->lpVtbl -> put_Similarity(This,__MIDL__IDxtKey0009) ) 

#define IDxtKey_get_Invert(This,__MIDL__IDxtKey0010)	\
    ( (This)->lpVtbl -> get_Invert(This,__MIDL__IDxtKey0010) ) 

#define IDxtKey_put_Invert(This,__MIDL__IDxtKey0011)	\
    ( (This)->lpVtbl -> put_Invert(This,__MIDL__IDxtKey0011) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDxtKey_INTERFACE_DEFINED__ */


#ifndef __IMediaLocator_INTERFACE_DEFINED__
#define __IMediaLocator_INTERFACE_DEFINED__

/* interface IMediaLocator */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IMediaLocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("288581E0-66CE-11d2-918F-00C0DF10D434")
    IMediaLocator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindMediaFile( 
            __RPC__in BSTR Input,
            __RPC__in BSTR FilterString,
            __RPC__deref_in_opt BSTR *pOutput,
            long Flags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddFoundLocation( 
            __RPC__in BSTR DirectoryName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMediaLocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMediaLocator * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMediaLocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMediaLocator * This);
        
        HRESULT ( STDMETHODCALLTYPE *FindMediaFile )( 
            IMediaLocator * This,
            __RPC__in BSTR Input,
            __RPC__in BSTR FilterString,
            __RPC__deref_in_opt BSTR *pOutput,
            long Flags);
        
        HRESULT ( STDMETHODCALLTYPE *AddFoundLocation )( 
            IMediaLocator * This,
            __RPC__in BSTR DirectoryName);
        
        END_INTERFACE
    } IMediaLocatorVtbl;

    interface IMediaLocator
    {
        CONST_VTBL struct IMediaLocatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaLocator_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IMediaLocator_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IMediaLocator_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IMediaLocator_FindMediaFile(This,Input,FilterString,pOutput,Flags)	\
    ( (This)->lpVtbl -> FindMediaFile(This,Input,FilterString,pOutput,Flags) ) 

#define IMediaLocator_AddFoundLocation(This,DirectoryName)	\
    ( (This)->lpVtbl -> AddFoundLocation(This,DirectoryName) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IMediaLocator_INTERFACE_DEFINED__ */


#ifndef __IMediaDet_INTERFACE_DEFINED__
#define __IMediaDet_INTERFACE_DEFINED__

/* interface IMediaDet */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IMediaDet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("65BD0710-24D2-4ff7-9324-ED2E5D3ABAFA")
    IMediaDet : public IUnknown
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Filter( 
            /* [retval][out] */ __RPC__deref_out_opt IUnknown **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Filter( 
            /* [in] */ __RPC__in_opt IUnknown *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OutputStreams( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentStream( 
            /* [retval][out] */ __RPC__out long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CurrentStream( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StreamType( 
            /* [retval][out] */ __RPC__out GUID *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StreamTypeB( 
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StreamLength( 
            /* [retval][out] */ __RPC__out double *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Filename( 
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Filename( 
            /* [in] */ __RPC__in BSTR newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetBitmapBits( 
            double StreamTime,
            __RPC__in long *pBufferSize,
            __RPC__in char *pBuffer,
            long Width,
            long Height) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE WriteBitmapBits( 
            double StreamTime,
            long Width,
            long Height,
            __RPC__in BSTR Filename) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StreamMediaType( 
            /* [retval][out] */ __RPC__out AM_MEDIA_TYPE *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSampleGrabber( 
            /* [out] */ __RPC__deref_out_opt ISampleGrabber **ppVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FrameRate( 
            /* [retval][out] */ __RPC__out double *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnterBitmapGrabMode( 
            double SeekTime) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMediaDetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMediaDet * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMediaDet * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMediaDet * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Filter )( 
            IMediaDet * This,
            /* [retval][out] */ __RPC__deref_out_opt IUnknown **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Filter )( 
            IMediaDet * This,
            /* [in] */ __RPC__in_opt IUnknown *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OutputStreams )( 
            IMediaDet * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CurrentStream )( 
            IMediaDet * This,
            /* [retval][out] */ __RPC__out long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CurrentStream )( 
            IMediaDet * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StreamType )( 
            IMediaDet * This,
            /* [retval][out] */ __RPC__out GUID *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StreamTypeB )( 
            IMediaDet * This,
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StreamLength )( 
            IMediaDet * This,
            /* [retval][out] */ __RPC__out double *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Filename )( 
            IMediaDet * This,
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Filename )( 
            IMediaDet * This,
            /* [in] */ __RPC__in BSTR newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetBitmapBits )( 
            IMediaDet * This,
            double StreamTime,
            __RPC__in long *pBufferSize,
            __RPC__in char *pBuffer,
            long Width,
            long Height);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *WriteBitmapBits )( 
            IMediaDet * This,
            double StreamTime,
            long Width,
            long Height,
            __RPC__in BSTR Filename);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StreamMediaType )( 
            IMediaDet * This,
            /* [retval][out] */ __RPC__out AM_MEDIA_TYPE *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSampleGrabber )( 
            IMediaDet * This,
            /* [out] */ __RPC__deref_out_opt ISampleGrabber **ppVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FrameRate )( 
            IMediaDet * This,
            /* [retval][out] */ __RPC__out double *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EnterBitmapGrabMode )( 
            IMediaDet * This,
            double SeekTime);
        
        END_INTERFACE
    } IMediaDetVtbl;

    interface IMediaDet
    {
        CONST_VTBL struct IMediaDetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaDet_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IMediaDet_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IMediaDet_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IMediaDet_get_Filter(This,pVal)	\
    ( (This)->lpVtbl -> get_Filter(This,pVal) ) 

#define IMediaDet_put_Filter(This,newVal)	\
    ( (This)->lpVtbl -> put_Filter(This,newVal) ) 

#define IMediaDet_get_OutputStreams(This,pVal)	\
    ( (This)->lpVtbl -> get_OutputStreams(This,pVal) ) 

#define IMediaDet_get_CurrentStream(This,pVal)	\
    ( (This)->lpVtbl -> get_CurrentStream(This,pVal) ) 

#define IMediaDet_put_CurrentStream(This,newVal)	\
    ( (This)->lpVtbl -> put_CurrentStream(This,newVal) ) 

#define IMediaDet_get_StreamType(This,pVal)	\
    ( (This)->lpVtbl -> get_StreamType(This,pVal) ) 

#define IMediaDet_get_StreamTypeB(This,pVal)	\
    ( (This)->lpVtbl -> get_StreamTypeB(This,pVal) ) 

#define IMediaDet_get_StreamLength(This,pVal)	\
    ( (This)->lpVtbl -> get_StreamLength(This,pVal) ) 

#define IMediaDet_get_Filename(This,pVal)	\
    ( (This)->lpVtbl -> get_Filename(This,pVal) ) 

#define IMediaDet_put_Filename(This,newVal)	\
    ( (This)->lpVtbl -> put_Filename(This,newVal) ) 

#define IMediaDet_GetBitmapBits(This,StreamTime,pBufferSize,pBuffer,Width,Height)	\
    ( (This)->lpVtbl -> GetBitmapBits(This,StreamTime,pBufferSize,pBuffer,Width,Height) ) 

#define IMediaDet_WriteBitmapBits(This,StreamTime,Width,Height,Filename)	\
    ( (This)->lpVtbl -> WriteBitmapBits(This,StreamTime,Width,Height,Filename) ) 

#define IMediaDet_get_StreamMediaType(This,pVal)	\
    ( (This)->lpVtbl -> get_StreamMediaType(This,pVal) ) 

#define IMediaDet_GetSampleGrabber(This,ppVal)	\
    ( (This)->lpVtbl -> GetSampleGrabber(This,ppVal) ) 

#define IMediaDet_get_FrameRate(This,pVal)	\
    ( (This)->lpVtbl -> get_FrameRate(This,pVal) ) 

#define IMediaDet_EnterBitmapGrabMode(This,SeekTime)	\
    ( (This)->lpVtbl -> EnterBitmapGrabMode(This,SeekTime) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IMediaDet_INTERFACE_DEFINED__ */


#ifndef __IGrfCache_INTERFACE_DEFINED__
#define __IGrfCache_INTERFACE_DEFINED__

/* interface IGrfCache */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IGrfCache;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AE9472BE-B0C3-11D2-8D24-00A0C9441E20")
    IGrfCache : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddFilter( 
            __RPC__in_opt IGrfCache *ChainedCache,
            LONGLONG ID,
            __RPC__in_opt const IBaseFilter *pFilter,
            __RPC__in LPCWSTR pName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ConnectPins( 
            __RPC__in_opt IGrfCache *ChainedCache,
            LONGLONG PinID1,
            __RPC__in_opt const IPin *pPin1,
            LONGLONG PinID2,
            __RPC__in_opt const IPin *pPin2) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetGraph( 
            __RPC__in_opt const IGraphBuilder *pGraph) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DoConnectionsNow( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGrfCacheVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGrfCache * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGrfCache * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGrfCache * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGrfCache * This,
            /* [out] */ __RPC__out UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGrfCache * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGrfCache * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGrfCache * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddFilter )( 
            IGrfCache * This,
            __RPC__in_opt IGrfCache *ChainedCache,
            LONGLONG ID,
            __RPC__in_opt const IBaseFilter *pFilter,
            __RPC__in LPCWSTR pName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ConnectPins )( 
            IGrfCache * This,
            __RPC__in_opt IGrfCache *ChainedCache,
            LONGLONG PinID1,
            __RPC__in_opt const IPin *pPin1,
            LONGLONG PinID2,
            __RPC__in_opt const IPin *pPin2);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetGraph )( 
            IGrfCache * This,
            __RPC__in_opt const IGraphBuilder *pGraph);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DoConnectionsNow )( 
            IGrfCache * This);
        
        END_INTERFACE
    } IGrfCacheVtbl;

    interface IGrfCache
    {
        CONST_VTBL struct IGrfCacheVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGrfCache_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IGrfCache_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IGrfCache_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IGrfCache_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IGrfCache_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IGrfCache_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IGrfCache_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IGrfCache_AddFilter(This,ChainedCache,ID,pFilter,pName)	\
    ( (This)->lpVtbl -> AddFilter(This,ChainedCache,ID,pFilter,pName) ) 

#define IGrfCache_ConnectPins(This,ChainedCache,PinID1,pPin1,PinID2,pPin2)	\
    ( (This)->lpVtbl -> ConnectPins(This,ChainedCache,PinID1,pPin1,PinID2,pPin2) ) 

#define IGrfCache_SetGraph(This,pGraph)	\
    ( (This)->lpVtbl -> SetGraph(This,pGraph) ) 

#define IGrfCache_DoConnectionsNow(This)	\
    ( (This)->lpVtbl -> DoConnectionsNow(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IGrfCache_INTERFACE_DEFINED__ */


#ifndef __IRenderEngine_INTERFACE_DEFINED__
#define __IRenderEngine_INTERFACE_DEFINED__

/* interface IRenderEngine */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IRenderEngine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6BEE3A81-66C9-11d2-918F-00C0DF10D434")
    IRenderEngine : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetTimelineObject( 
            __RPC__in_opt IAMTimeline *pTimeline) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTimelineObject( 
            /* [out] */ __RPC__deref_out_opt IAMTimeline **ppTimeline) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFilterGraph( 
            /* [out] */ __RPC__deref_out_opt IGraphBuilder **ppFG) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFilterGraph( 
            __RPC__in_opt IGraphBuilder *pFG) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInterestRange( 
            REFERENCE_TIME Start,
            REFERENCE_TIME Stop) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInterestRange2( 
            double Start,
            double Stop) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRenderRange( 
            REFERENCE_TIME Start,
            REFERENCE_TIME Stop) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRenderRange2( 
            double Start,
            double Stop) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGroupOutputPin( 
            long Group,
            /* [out] */ __RPC__deref_out_opt IPin **ppRenderPin) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ScrapIt( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RenderOutputPins( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVendorString( 
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pVendorID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectFrontEnd( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSourceConnectCallback( 
            __RPC__in_opt IGrfCache *pCallback) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDynamicReconnectLevel( 
            long Level) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DoSmartRecompression( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UseInSmartRecompressionGraph( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSourceNameValidation( 
            __RPC__in BSTR FilterString,
            __RPC__in_opt IMediaLocator *pOverride,
            LONG Flags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Decommit( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCaps( 
            long Index,
            __RPC__in long *pReturn) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRenderEngineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRenderEngine * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRenderEngine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetTimelineObject )( 
            IRenderEngine * This,
            __RPC__in_opt IAMTimeline *pTimeline);
        
        HRESULT ( STDMETHODCALLTYPE *GetTimelineObject )( 
            IRenderEngine * This,
            /* [out] */ __RPC__deref_out_opt IAMTimeline **ppTimeline);
        
        HRESULT ( STDMETHODCALLTYPE *GetFilterGraph )( 
            IRenderEngine * This,
            /* [out] */ __RPC__deref_out_opt IGraphBuilder **ppFG);
        
        HRESULT ( STDMETHODCALLTYPE *SetFilterGraph )( 
            IRenderEngine * This,
            __RPC__in_opt IGraphBuilder *pFG);
        
        HRESULT ( STDMETHODCALLTYPE *SetInterestRange )( 
            IRenderEngine * This,
            REFERENCE_TIME Start,
            REFERENCE_TIME Stop);
        
        HRESULT ( STDMETHODCALLTYPE *SetInterestRange2 )( 
            IRenderEngine * This,
            double Start,
            double Stop);
        
        HRESULT ( STDMETHODCALLTYPE *SetRenderRange )( 
            IRenderEngine * This,
            REFERENCE_TIME Start,
            REFERENCE_TIME Stop);
        
        HRESULT ( STDMETHODCALLTYPE *SetRenderRange2 )( 
            IRenderEngine * This,
            double Start,
            double Stop);
        
        HRESULT ( STDMETHODCALLTYPE *GetGroupOutputPin )( 
            IRenderEngine * This,
            long Group,
            /* [out] */ __RPC__deref_out_opt IPin **ppRenderPin);
        
        HRESULT ( STDMETHODCALLTYPE *ScrapIt )( 
            IRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *RenderOutputPins )( 
            IRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetVendorString )( 
            IRenderEngine * This,
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pVendorID);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectFrontEnd )( 
            IRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSourceConnectCallback )( 
            IRenderEngine * This,
            __RPC__in_opt IGrfCache *pCallback);
        
        HRESULT ( STDMETHODCALLTYPE *SetDynamicReconnectLevel )( 
            IRenderEngine * This,
            long Level);
        
        HRESULT ( STDMETHODCALLTYPE *DoSmartRecompression )( 
            IRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *UseInSmartRecompressionGraph )( 
            IRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSourceNameValidation )( 
            IRenderEngine * This,
            __RPC__in BSTR FilterString,
            __RPC__in_opt IMediaLocator *pOverride,
            LONG Flags);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *Decommit )( 
            IRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCaps )( 
            IRenderEngine * This,
            long Index,
            __RPC__in long *pReturn);
        
        END_INTERFACE
    } IRenderEngineVtbl;

    interface IRenderEngine
    {
        CONST_VTBL struct IRenderEngineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRenderEngine_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRenderEngine_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRenderEngine_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRenderEngine_SetTimelineObject(This,pTimeline)	\
    ( (This)->lpVtbl -> SetTimelineObject(This,pTimeline) ) 

#define IRenderEngine_GetTimelineObject(This,ppTimeline)	\
    ( (This)->lpVtbl -> GetTimelineObject(This,ppTimeline) ) 

#define IRenderEngine_GetFilterGraph(This,ppFG)	\
    ( (This)->lpVtbl -> GetFilterGraph(This,ppFG) ) 

#define IRenderEngine_SetFilterGraph(This,pFG)	\
    ( (This)->lpVtbl -> SetFilterGraph(This,pFG) ) 

#define IRenderEngine_SetInterestRange(This,Start,Stop)	\
    ( (This)->lpVtbl -> SetInterestRange(This,Start,Stop) ) 

#define IRenderEngine_SetInterestRange2(This,Start,Stop)	\
    ( (This)->lpVtbl -> SetInterestRange2(This,Start,Stop) ) 

#define IRenderEngine_SetRenderRange(This,Start,Stop)	\
    ( (This)->lpVtbl -> SetRenderRange(This,Start,Stop) ) 

#define IRenderEngine_SetRenderRange2(This,Start,Stop)	\
    ( (This)->lpVtbl -> SetRenderRange2(This,Start,Stop) ) 

#define IRenderEngine_GetGroupOutputPin(This,Group,ppRenderPin)	\
    ( (This)->lpVtbl -> GetGroupOutputPin(This,Group,ppRenderPin) ) 

#define IRenderEngine_ScrapIt(This)	\
    ( (This)->lpVtbl -> ScrapIt(This) ) 

#define IRenderEngine_RenderOutputPins(This)	\
    ( (This)->lpVtbl -> RenderOutputPins(This) ) 

#define IRenderEngine_GetVendorString(This,pVendorID)	\
    ( (This)->lpVtbl -> GetVendorString(This,pVendorID) ) 

#define IRenderEngine_ConnectFrontEnd(This)	\
    ( (This)->lpVtbl -> ConnectFrontEnd(This) ) 

#define IRenderEngine_SetSourceConnectCallback(This,pCallback)	\
    ( (This)->lpVtbl -> SetSourceConnectCallback(This,pCallback) ) 

#define IRenderEngine_SetDynamicReconnectLevel(This,Level)	\
    ( (This)->lpVtbl -> SetDynamicReconnectLevel(This,Level) ) 

#define IRenderEngine_DoSmartRecompression(This)	\
    ( (This)->lpVtbl -> DoSmartRecompression(This) ) 

#define IRenderEngine_UseInSmartRecompressionGraph(This)	\
    ( (This)->lpVtbl -> UseInSmartRecompressionGraph(This) ) 

#define IRenderEngine_SetSourceNameValidation(This,FilterString,pOverride,Flags)	\
    ( (This)->lpVtbl -> SetSourceNameValidation(This,FilterString,pOverride,Flags) ) 

#define IRenderEngine_Commit(This)	\
    ( (This)->lpVtbl -> Commit(This) ) 

#define IRenderEngine_Decommit(This)	\
    ( (This)->lpVtbl -> Decommit(This) ) 

#define IRenderEngine_GetCaps(This,Index,pReturn)	\
    ( (This)->lpVtbl -> GetCaps(This,Index,pReturn) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IRenderEngine_INTERFACE_DEFINED__ */


#ifndef __IRenderEngine2_INTERFACE_DEFINED__
#define __IRenderEngine2_INTERFACE_DEFINED__

/* interface IRenderEngine2 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IRenderEngine2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6BEE3A82-66C9-11d2-918F-00C0DF10D434")
    IRenderEngine2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetResizerGUID( 
            GUID ResizerGuid) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRenderEngine2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRenderEngine2 * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRenderEngine2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRenderEngine2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetResizerGUID )( 
            IRenderEngine2 * This,
            GUID ResizerGuid);
        
        END_INTERFACE
    } IRenderEngine2Vtbl;

    interface IRenderEngine2
    {
        CONST_VTBL struct IRenderEngine2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRenderEngine2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRenderEngine2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRenderEngine2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRenderEngine2_SetResizerGUID(This,ResizerGuid)	\
    ( (This)->lpVtbl -> SetResizerGUID(This,ResizerGuid) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IRenderEngine2_INTERFACE_DEFINED__ */


#ifndef __IFindCompressorCB_INTERFACE_DEFINED__
#define __IFindCompressorCB_INTERFACE_DEFINED__

/* interface IFindCompressorCB */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IFindCompressorCB;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F03FA8DE-879A-4d59-9B2C-26BB1CF83461")
    IFindCompressorCB : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCompressor( 
            __RPC__in AM_MEDIA_TYPE *pType,
            __RPC__in AM_MEDIA_TYPE *pCompType,
            /* [out] */ __RPC__deref_out_opt IBaseFilter **ppFilter) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IFindCompressorCBVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFindCompressorCB * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFindCompressorCB * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFindCompressorCB * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompressor )( 
            IFindCompressorCB * This,
            __RPC__in AM_MEDIA_TYPE *pType,
            __RPC__in AM_MEDIA_TYPE *pCompType,
            /* [out] */ __RPC__deref_out_opt IBaseFilter **ppFilter);
        
        END_INTERFACE
    } IFindCompressorCBVtbl;

    interface IFindCompressorCB
    {
        CONST_VTBL struct IFindCompressorCBVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFindCompressorCB_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFindCompressorCB_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFindCompressorCB_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFindCompressorCB_GetCompressor(This,pType,pCompType,ppFilter)	\
    ( (This)->lpVtbl -> GetCompressor(This,pType,pCompType,ppFilter) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IFindCompressorCB_INTERFACE_DEFINED__ */


#ifndef __ISmartRenderEngine_INTERFACE_DEFINED__
#define __ISmartRenderEngine_INTERFACE_DEFINED__

/* interface ISmartRenderEngine */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ISmartRenderEngine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F03FA8CE-879A-4d59-9B2C-26BB1CF83461")
    ISmartRenderEngine : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetGroupCompressor( 
            long Group,
            __RPC__in_opt IBaseFilter *pCompressor) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGroupCompressor( 
            long Group,
            __RPC__deref_in_opt IBaseFilter **pCompressor) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFindCompressorCB( 
            __RPC__in_opt IFindCompressorCB *pCallback) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISmartRenderEngineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISmartRenderEngine * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISmartRenderEngine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISmartRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetGroupCompressor )( 
            ISmartRenderEngine * This,
            long Group,
            __RPC__in_opt IBaseFilter *pCompressor);
        
        HRESULT ( STDMETHODCALLTYPE *GetGroupCompressor )( 
            ISmartRenderEngine * This,
            long Group,
            __RPC__deref_in_opt IBaseFilter **pCompressor);
        
        HRESULT ( STDMETHODCALLTYPE *SetFindCompressorCB )( 
            ISmartRenderEngine * This,
            __RPC__in_opt IFindCompressorCB *pCallback);
        
        END_INTERFACE
    } ISmartRenderEngineVtbl;

    interface ISmartRenderEngine
    {
        CONST_VTBL struct ISmartRenderEngineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISmartRenderEngine_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISmartRenderEngine_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISmartRenderEngine_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISmartRenderEngine_SetGroupCompressor(This,Group,pCompressor)	\
    ( (This)->lpVtbl -> SetGroupCompressor(This,Group,pCompressor) ) 

#define ISmartRenderEngine_GetGroupCompressor(This,Group,pCompressor)	\
    ( (This)->lpVtbl -> GetGroupCompressor(This,Group,pCompressor) ) 

#define ISmartRenderEngine_SetFindCompressorCB(This,pCallback)	\
    ( (This)->lpVtbl -> SetFindCompressorCB(This,pCallback) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISmartRenderEngine_INTERFACE_DEFINED__ */


#ifndef __IAMTimelineObj_INTERFACE_DEFINED__
#define __IAMTimelineObj_INTERFACE_DEFINED__

/* interface IAMTimelineObj */
/* [unique][helpstring][uuid][local][object] */ 


EXTERN_C const IID IID_IAMTimelineObj;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("78530B77-61F9-11D2-8CAD-00A024580902")
    IAMTimelineObj : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetStartStop( 
            REFERENCE_TIME *pStart,
            REFERENCE_TIME *pStop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetStartStop2( 
            REFTIME *pStart,
            REFTIME *pStop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FixTimes( 
            REFERENCE_TIME *pStart,
            REFERENCE_TIME *pStop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FixTimes2( 
            REFTIME *pStart,
            REFTIME *pStop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetStartStop( 
            REFERENCE_TIME Start,
            REFERENCE_TIME Stop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetStartStop2( 
            REFTIME Start,
            REFTIME Stop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPropertySetter( 
            /* [retval][out] */ IPropertySetter **pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetPropertySetter( 
            IPropertySetter *newVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSubObject( 
            /* [retval][out] */ IUnknown **pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetSubObject( 
            IUnknown *newVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetSubObjectGUID( 
            GUID newVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetSubObjectGUIDB( 
            BSTR newVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSubObjectGUID( 
            GUID *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSubObjectGUIDB( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSubObjectLoaded( 
            BOOL *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetTimelineType( 
            TIMELINE_MAJOR_TYPE *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetTimelineType( 
            TIMELINE_MAJOR_TYPE newVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetUserID( 
            long *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetUserID( 
            long newVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetGenID( 
            long *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetUserName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetUserName( 
            BSTR newVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetUserData( 
            BYTE *pData,
            long *pSize) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetUserData( 
            BYTE *pData,
            long Size) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMuted( 
            BOOL *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetMuted( 
            BOOL newVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetLocked( 
            BOOL *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetLocked( 
            BOOL newVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDirtyRange( 
            REFERENCE_TIME *pStart,
            REFERENCE_TIME *pStop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDirtyRange2( 
            REFTIME *pStart,
            REFTIME *pStop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetDirtyRange( 
            REFERENCE_TIME Start,
            REFERENCE_TIME Stop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetDirtyRange2( 
            REFTIME Start,
            REFTIME Stop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ClearDirty( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Remove( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RemoveAll( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTimelineNoRef( 
            IAMTimeline **ppResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGroupIBelongTo( 
            /* [out] */ IAMTimelineGroup **ppGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEmbedDepth( 
            long *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMTimelineObjVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMTimelineObj * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMTimelineObj * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMTimelineObj * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetStartStop )( 
            IAMTimelineObj * This,
            REFERENCE_TIME *pStart,
            REFERENCE_TIME *pStop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetStartStop2 )( 
            IAMTimelineObj * This,
            REFTIME *pStart,
            REFTIME *pStop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *FixTimes )( 
            IAMTimelineObj * This,
            REFERENCE_TIME *pStart,
            REFERENCE_TIME *pStop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *FixTimes2 )( 
            IAMTimelineObj * This,
            REFTIME *pStart,
            REFTIME *pStop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetStartStop )( 
            IAMTimelineObj * This,
            REFERENCE_TIME Start,
            REFERENCE_TIME Stop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetStartStop2 )( 
            IAMTimelineObj * This,
            REFTIME Start,
            REFTIME Stop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPropertySetter )( 
            IAMTimelineObj * This,
            /* [retval][out] */ IPropertySetter **pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetPropertySetter )( 
            IAMTimelineObj * This,
            IPropertySetter *newVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetSubObject )( 
            IAMTimelineObj * This,
            /* [retval][out] */ IUnknown **pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetSubObject )( 
            IAMTimelineObj * This,
            IUnknown *newVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetSubObjectGUID )( 
            IAMTimelineObj * This,
            GUID newVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetSubObjectGUIDB )( 
            IAMTimelineObj * This,
            BSTR newVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetSubObjectGUID )( 
            IAMTimelineObj * This,
            GUID *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetSubObjectGUIDB )( 
            IAMTimelineObj * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetSubObjectLoaded )( 
            IAMTimelineObj * This,
            BOOL *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetTimelineType )( 
            IAMTimelineObj * This,
            TIMELINE_MAJOR_TYPE *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetTimelineType )( 
            IAMTimelineObj * This,
            TIMELINE_MAJOR_TYPE newVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetUserID )( 
            IAMTimelineObj * This,
            long *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetUserID )( 
            IAMTimelineObj * This,
            long newVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetGenID )( 
            IAMTimelineObj * This,
            long *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetUserName )( 
            IAMTimelineObj * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetUserName )( 
            IAMTimelineObj * This,
            BSTR newVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetUserData )( 
            IAMTimelineObj * This,
            BYTE *pData,
            long *pSize);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetUserData )( 
            IAMTimelineObj * This,
            BYTE *pData,
            long Size);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetMuted )( 
            IAMTimelineObj * This,
            BOOL *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetMuted )( 
            IAMTimelineObj * This,
            BOOL newVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetLocked )( 
            IAMTimelineObj * This,
            BOOL *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetLocked )( 
            IAMTimelineObj * This,
            BOOL newVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetDirtyRange )( 
            IAMTimelineObj * This,
            REFERENCE_TIME *pStart,
            REFERENCE_TIME *pStop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetDirtyRange2 )( 
            IAMTimelineObj * This,
            REFTIME *pStart,
            REFTIME *pStop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetDirtyRange )( 
            IAMTimelineObj * This,
            REFERENCE_TIME Start,
            REFERENCE_TIME Stop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetDirtyRange2 )( 
            IAMTimelineObj * This,
            REFTIME Start,
            REFTIME Stop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *ClearDirty )( 
            IAMTimelineObj * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IAMTimelineObj * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RemoveAll )( 
            IAMTimelineObj * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTimelineNoRef )( 
            IAMTimelineObj * This,
            IAMTimeline **ppResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetGroupIBelongTo )( 
            IAMTimelineObj * This,
            /* [out] */ IAMTimelineGroup **ppGroup);
        
        HRESULT ( STDMETHODCALLTYPE *GetEmbedDepth )( 
            IAMTimelineObj * This,
            long *pVal);
        
        END_INTERFACE
    } IAMTimelineObjVtbl;

    interface IAMTimelineObj
    {
        CONST_VTBL struct IAMTimelineObjVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMTimelineObj_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAMTimelineObj_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAMTimelineObj_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAMTimelineObj_GetStartStop(This,pStart,pStop)	\
    ( (This)->lpVtbl -> GetStartStop(This,pStart,pStop) ) 

#define IAMTimelineObj_GetStartStop2(This,pStart,pStop)	\
    ( (This)->lpVtbl -> GetStartStop2(This,pStart,pStop) ) 

#define IAMTimelineObj_FixTimes(This,pStart,pStop)	\
    ( (This)->lpVtbl -> FixTimes(This,pStart,pStop) ) 

#define IAMTimelineObj_FixTimes2(This,pStart,pStop)	\
    ( (This)->lpVtbl -> FixTimes2(This,pStart,pStop) ) 

#define IAMTimelineObj_SetStartStop(This,Start,Stop)	\
    ( (This)->lpVtbl -> SetStartStop(This,Start,Stop) ) 

#define IAMTimelineObj_SetStartStop2(This,Start,Stop)	\
    ( (This)->lpVtbl -> SetStartStop2(This,Start,Stop) ) 

#define IAMTimelineObj_GetPropertySetter(This,pVal)	\
    ( (This)->lpVtbl -> GetPropertySetter(This,pVal) ) 

#define IAMTimelineObj_SetPropertySetter(This,newVal)	\
    ( (This)->lpVtbl -> SetPropertySetter(This,newVal) ) 

#define IAMTimelineObj_GetSubObject(This,pVal)	\
    ( (This)->lpVtbl -> GetSubObject(This,pVal) ) 

#define IAMTimelineObj_SetSubObject(This,newVal)	\
    ( (This)->lpVtbl -> SetSubObject(This,newVal) ) 

#define IAMTimelineObj_SetSubObjectGUID(This,newVal)	\
    ( (This)->lpVtbl -> SetSubObjectGUID(This,newVal) ) 

#define IAMTimelineObj_SetSubObjectGUIDB(This,newVal)	\
    ( (This)->lpVtbl -> SetSubObjectGUIDB(This,newVal) ) 

#define IAMTimelineObj_GetSubObjectGUID(This,pVal)	\
    ( (This)->lpVtbl -> GetSubObjectGUID(This,pVal) ) 

#define IAMTimelineObj_GetSubObjectGUIDB(This,pVal)	\
    ( (This)->lpVtbl -> GetSubObjectGUIDB(This,pVal) ) 

#define IAMTimelineObj_GetSubObjectLoaded(This,pVal)	\
    ( (This)->lpVtbl -> GetSubObjectLoaded(This,pVal) ) 

#define IAMTimelineObj_GetTimelineType(This,pVal)	\
    ( (This)->lpVtbl -> GetTimelineType(This,pVal) ) 

#define IAMTimelineObj_SetTimelineType(This,newVal)	\
    ( (This)->lpVtbl -> SetTimelineType(This,newVal) ) 

#define IAMTimelineObj_GetUserID(This,pVal)	\
    ( (This)->lpVtbl -> GetUserID(This,pVal) ) 

#define IAMTimelineObj_SetUserID(This,newVal)	\
    ( (This)->lpVtbl -> SetUserID(This,newVal) ) 

#define IAMTimelineObj_GetGenID(This,pVal)	\
    ( (This)->lpVtbl -> GetGenID(This,pVal) ) 

#define IAMTimelineObj_GetUserName(This,pVal)	\
    ( (This)->lpVtbl -> GetUserName(This,pVal) ) 

#define IAMTimelineObj_SetUserName(This,newVal)	\
    ( (This)->lpVtbl -> SetUserName(This,newVal) ) 

#define IAMTimelineObj_GetUserData(This,pData,pSize)	\
    ( (This)->lpVtbl -> GetUserData(This,pData,pSize) ) 

#define IAMTimelineObj_SetUserData(This,pData,Size)	\
    ( (This)->lpVtbl -> SetUserData(This,pData,Size) ) 

#define IAMTimelineObj_GetMuted(This,pVal)	\
    ( (This)->lpVtbl -> GetMuted(This,pVal) ) 

#define IAMTimelineObj_SetMuted(This,newVal)	\
    ( (This)->lpVtbl -> SetMuted(This,newVal) ) 

#define IAMTimelineObj_GetLocked(This,pVal)	\
    ( (This)->lpVtbl -> GetLocked(This,pVal) ) 

#define IAMTimelineObj_SetLocked(This,newVal)	\
    ( (This)->lpVtbl -> SetLocked(This,newVal) ) 

#define IAMTimelineObj_GetDirtyRange(This,pStart,pStop)	\
    ( (This)->lpVtbl -> GetDirtyRange(This,pStart,pStop) ) 

#define IAMTimelineObj_GetDirtyRange2(This,pStart,pStop)	\
    ( (This)->lpVtbl -> GetDirtyRange2(This,pStart,pStop) ) 

#define IAMTimelineObj_SetDirtyRange(This,Start,Stop)	\
    ( (This)->lpVtbl -> SetDirtyRange(This,Start,Stop) ) 

#define IAMTimelineObj_SetDirtyRange2(This,Start,Stop)	\
    ( (This)->lpVtbl -> SetDirtyRange2(This,Start,Stop) ) 

#define IAMTimelineObj_ClearDirty(This)	\
    ( (This)->lpVtbl -> ClearDirty(This) ) 

#define IAMTimelineObj_Remove(This)	\
    ( (This)->lpVtbl -> Remove(This) ) 

#define IAMTimelineObj_RemoveAll(This)	\
    ( (This)->lpVtbl -> RemoveAll(This) ) 

#define IAMTimelineObj_GetTimelineNoRef(This,ppResult)	\
    ( (This)->lpVtbl -> GetTimelineNoRef(This,ppResult) ) 

#define IAMTimelineObj_GetGroupIBelongTo(This,ppGroup)	\
    ( (This)->lpVtbl -> GetGroupIBelongTo(This,ppGroup) ) 

#define IAMTimelineObj_GetEmbedDepth(This,pVal)	\
    ( (This)->lpVtbl -> GetEmbedDepth(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAMTimelineObj_INTERFACE_DEFINED__ */


#ifndef __IAMTimelineEffectable_INTERFACE_DEFINED__
#define __IAMTimelineEffectable_INTERFACE_DEFINED__

/* interface IAMTimelineEffectable */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAMTimelineEffectable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EAE58537-622E-11d2-8CAD-00A024580902")
    IAMTimelineEffectable : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EffectInsBefore( 
            __RPC__in_opt IAMTimelineObj *pFX,
            long priority) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EffectSwapPriorities( 
            long PriorityA,
            long PriorityB) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EffectGetCount( 
            __RPC__in long *pCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEffect( 
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppFx,
            long Which) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMTimelineEffectableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMTimelineEffectable * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMTimelineEffectable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMTimelineEffectable * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EffectInsBefore )( 
            IAMTimelineEffectable * This,
            __RPC__in_opt IAMTimelineObj *pFX,
            long priority);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EffectSwapPriorities )( 
            IAMTimelineEffectable * This,
            long PriorityA,
            long PriorityB);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EffectGetCount )( 
            IAMTimelineEffectable * This,
            __RPC__in long *pCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetEffect )( 
            IAMTimelineEffectable * This,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppFx,
            long Which);
        
        END_INTERFACE
    } IAMTimelineEffectableVtbl;

    interface IAMTimelineEffectable
    {
        CONST_VTBL struct IAMTimelineEffectableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMTimelineEffectable_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAMTimelineEffectable_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAMTimelineEffectable_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAMTimelineEffectable_EffectInsBefore(This,pFX,priority)	\
    ( (This)->lpVtbl -> EffectInsBefore(This,pFX,priority) ) 

#define IAMTimelineEffectable_EffectSwapPriorities(This,PriorityA,PriorityB)	\
    ( (This)->lpVtbl -> EffectSwapPriorities(This,PriorityA,PriorityB) ) 

#define IAMTimelineEffectable_EffectGetCount(This,pCount)	\
    ( (This)->lpVtbl -> EffectGetCount(This,pCount) ) 

#define IAMTimelineEffectable_GetEffect(This,ppFx,Which)	\
    ( (This)->lpVtbl -> GetEffect(This,ppFx,Which) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAMTimelineEffectable_INTERFACE_DEFINED__ */


#ifndef __IAMTimelineEffect_INTERFACE_DEFINED__
#define __IAMTimelineEffect_INTERFACE_DEFINED__

/* interface IAMTimelineEffect */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAMTimelineEffect;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BCE0C264-622D-11d2-8CAD-00A024580902")
    IAMTimelineEffect : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EffectGetPriority( 
            __RPC__in long *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMTimelineEffectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMTimelineEffect * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMTimelineEffect * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMTimelineEffect * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EffectGetPriority )( 
            IAMTimelineEffect * This,
            __RPC__in long *pVal);
        
        END_INTERFACE
    } IAMTimelineEffectVtbl;

    interface IAMTimelineEffect
    {
        CONST_VTBL struct IAMTimelineEffectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMTimelineEffect_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAMTimelineEffect_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAMTimelineEffect_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAMTimelineEffect_EffectGetPriority(This,pVal)	\
    ( (This)->lpVtbl -> EffectGetPriority(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAMTimelineEffect_INTERFACE_DEFINED__ */


#ifndef __IAMTimelineTransable_INTERFACE_DEFINED__
#define __IAMTimelineTransable_INTERFACE_DEFINED__

/* interface IAMTimelineTransable */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAMTimelineTransable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("378FA386-622E-11d2-8CAD-00A024580902")
    IAMTimelineTransable : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE TransAdd( 
            __RPC__in_opt IAMTimelineObj *pTrans) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE TransGetCount( 
            __RPC__in long *pCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetNextTrans( 
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppTrans,
            __RPC__in REFERENCE_TIME *pInOut) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetNextTrans2( 
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppTrans,
            __RPC__in REFTIME *pInOut) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetTransAtTime( 
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppObj,
            REFERENCE_TIME Time,
            long SearchDirection) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetTransAtTime2( 
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppObj,
            REFTIME Time,
            long SearchDirection) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMTimelineTransableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMTimelineTransable * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMTimelineTransable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMTimelineTransable * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *TransAdd )( 
            IAMTimelineTransable * This,
            __RPC__in_opt IAMTimelineObj *pTrans);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *TransGetCount )( 
            IAMTimelineTransable * This,
            __RPC__in long *pCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetNextTrans )( 
            IAMTimelineTransable * This,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppTrans,
            __RPC__in REFERENCE_TIME *pInOut);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetNextTrans2 )( 
            IAMTimelineTransable * This,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppTrans,
            __RPC__in REFTIME *pInOut);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetTransAtTime )( 
            IAMTimelineTransable * This,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppObj,
            REFERENCE_TIME Time,
            long SearchDirection);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetTransAtTime2 )( 
            IAMTimelineTransable * This,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppObj,
            REFTIME Time,
            long SearchDirection);
        
        END_INTERFACE
    } IAMTimelineTransableVtbl;

    interface IAMTimelineTransable
    {
        CONST_VTBL struct IAMTimelineTransableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMTimelineTransable_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAMTimelineTransable_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAMTimelineTransable_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAMTimelineTransable_TransAdd(This,pTrans)	\
    ( (This)->lpVtbl -> TransAdd(This,pTrans) ) 

#define IAMTimelineTransable_TransGetCount(This,pCount)	\
    ( (This)->lpVtbl -> TransGetCount(This,pCount) ) 

#define IAMTimelineTransable_GetNextTrans(This,ppTrans,pInOut)	\
    ( (This)->lpVtbl -> GetNextTrans(This,ppTrans,pInOut) ) 

#define IAMTimelineTransable_GetNextTrans2(This,ppTrans,pInOut)	\
    ( (This)->lpVtbl -> GetNextTrans2(This,ppTrans,pInOut) ) 

#define IAMTimelineTransable_GetTransAtTime(This,ppObj,Time,SearchDirection)	\
    ( (This)->lpVtbl -> GetTransAtTime(This,ppObj,Time,SearchDirection) ) 

#define IAMTimelineTransable_GetTransAtTime2(This,ppObj,Time,SearchDirection)	\
    ( (This)->lpVtbl -> GetTransAtTime2(This,ppObj,Time,SearchDirection) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAMTimelineTransable_INTERFACE_DEFINED__ */


#ifndef __IAMTimelineSplittable_INTERFACE_DEFINED__
#define __IAMTimelineSplittable_INTERFACE_DEFINED__

/* interface IAMTimelineSplittable */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAMTimelineSplittable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A0F840A0-D590-11d2-8D55-00A0C9441E20")
    IAMTimelineSplittable : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SplitAt( 
            REFERENCE_TIME Time) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SplitAt2( 
            REFTIME Time) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMTimelineSplittableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMTimelineSplittable * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMTimelineSplittable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMTimelineSplittable * This);
        
        HRESULT ( STDMETHODCALLTYPE *SplitAt )( 
            IAMTimelineSplittable * This,
            REFERENCE_TIME Time);
        
        HRESULT ( STDMETHODCALLTYPE *SplitAt2 )( 
            IAMTimelineSplittable * This,
            REFTIME Time);
        
        END_INTERFACE
    } IAMTimelineSplittableVtbl;

    interface IAMTimelineSplittable
    {
        CONST_VTBL struct IAMTimelineSplittableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMTimelineSplittable_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAMTimelineSplittable_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAMTimelineSplittable_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAMTimelineSplittable_SplitAt(This,Time)	\
    ( (This)->lpVtbl -> SplitAt(This,Time) ) 

#define IAMTimelineSplittable_SplitAt2(This,Time)	\
    ( (This)->lpVtbl -> SplitAt2(This,Time) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAMTimelineSplittable_INTERFACE_DEFINED__ */


#ifndef __IAMTimelineTrans_INTERFACE_DEFINED__
#define __IAMTimelineTrans_INTERFACE_DEFINED__

/* interface IAMTimelineTrans */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAMTimelineTrans;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BCE0C265-622D-11d2-8CAD-00A024580902")
    IAMTimelineTrans : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCutPoint( 
            __RPC__in REFERENCE_TIME *pTLTime) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCutPoint2( 
            __RPC__in REFTIME *pTLTime) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetCutPoint( 
            REFERENCE_TIME TLTime) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetCutPoint2( 
            REFTIME TLTime) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSwapInputs( 
            __RPC__in BOOL *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetSwapInputs( 
            BOOL pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCutsOnly( 
            __RPC__in BOOL *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetCutsOnly( 
            BOOL pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMTimelineTransVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMTimelineTrans * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMTimelineTrans * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMTimelineTrans * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetCutPoint )( 
            IAMTimelineTrans * This,
            __RPC__in REFERENCE_TIME *pTLTime);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetCutPoint2 )( 
            IAMTimelineTrans * This,
            __RPC__in REFTIME *pTLTime);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetCutPoint )( 
            IAMTimelineTrans * This,
            REFERENCE_TIME TLTime);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetCutPoint2 )( 
            IAMTimelineTrans * This,
            REFTIME TLTime);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetSwapInputs )( 
            IAMTimelineTrans * This,
            __RPC__in BOOL *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetSwapInputs )( 
            IAMTimelineTrans * This,
            BOOL pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetCutsOnly )( 
            IAMTimelineTrans * This,
            __RPC__in BOOL *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetCutsOnly )( 
            IAMTimelineTrans * This,
            BOOL pVal);
        
        END_INTERFACE
    } IAMTimelineTransVtbl;

    interface IAMTimelineTrans
    {
        CONST_VTBL struct IAMTimelineTransVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMTimelineTrans_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAMTimelineTrans_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAMTimelineTrans_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAMTimelineTrans_GetCutPoint(This,pTLTime)	\
    ( (This)->lpVtbl -> GetCutPoint(This,pTLTime) ) 

#define IAMTimelineTrans_GetCutPoint2(This,pTLTime)	\
    ( (This)->lpVtbl -> GetCutPoint2(This,pTLTime) ) 

#define IAMTimelineTrans_SetCutPoint(This,TLTime)	\
    ( (This)->lpVtbl -> SetCutPoint(This,TLTime) ) 

#define IAMTimelineTrans_SetCutPoint2(This,TLTime)	\
    ( (This)->lpVtbl -> SetCutPoint2(This,TLTime) ) 

#define IAMTimelineTrans_GetSwapInputs(This,pVal)	\
    ( (This)->lpVtbl -> GetSwapInputs(This,pVal) ) 

#define IAMTimelineTrans_SetSwapInputs(This,pVal)	\
    ( (This)->lpVtbl -> SetSwapInputs(This,pVal) ) 

#define IAMTimelineTrans_GetCutsOnly(This,pVal)	\
    ( (This)->lpVtbl -> GetCutsOnly(This,pVal) ) 

#define IAMTimelineTrans_SetCutsOnly(This,pVal)	\
    ( (This)->lpVtbl -> SetCutsOnly(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAMTimelineTrans_INTERFACE_DEFINED__ */


#ifndef __IAMTimelineSrc_INTERFACE_DEFINED__
#define __IAMTimelineSrc_INTERFACE_DEFINED__

/* interface IAMTimelineSrc */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAMTimelineSrc;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("78530B79-61F9-11D2-8CAD-00A024580902")
    IAMTimelineSrc : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMediaTimes( 
            __RPC__in REFERENCE_TIME *pStart,
            __RPC__in REFERENCE_TIME *pStop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMediaTimes2( 
            __RPC__in REFTIME *pStart,
            __RPC__in REFTIME *pStop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ModifyStopTime( 
            REFERENCE_TIME Stop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ModifyStopTime2( 
            REFTIME Stop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FixMediaTimes( 
            __RPC__in REFERENCE_TIME *pStart,
            __RPC__in REFERENCE_TIME *pStop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FixMediaTimes2( 
            __RPC__in REFTIME *pStart,
            __RPC__in REFTIME *pStop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetMediaTimes( 
            REFERENCE_TIME Start,
            REFERENCE_TIME Stop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetMediaTimes2( 
            REFTIME Start,
            REFTIME Stop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetMediaLength( 
            REFERENCE_TIME Length) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetMediaLength2( 
            REFTIME Length) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMediaLength( 
            __RPC__in REFERENCE_TIME *pLength) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMediaLength2( 
            __RPC__in REFTIME *pLength) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMediaName( 
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetMediaName( 
            __RPC__in BSTR newVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SpliceWithNext( 
            __RPC__in_opt IAMTimelineObj *pNext) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetStreamNumber( 
            __RPC__in long *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetStreamNumber( 
            long Val) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsNormalRate( 
            __RPC__in BOOL *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDefaultFPS( 
            __RPC__in double *pFPS) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetDefaultFPS( 
            double FPS) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetStretchMode( 
            __RPC__in int *pnStretchMode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetStretchMode( 
            int nStretchMode) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMTimelineSrcVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMTimelineSrc * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMTimelineSrc * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMTimelineSrc * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetMediaTimes )( 
            IAMTimelineSrc * This,
            __RPC__in REFERENCE_TIME *pStart,
            __RPC__in REFERENCE_TIME *pStop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetMediaTimes2 )( 
            IAMTimelineSrc * This,
            __RPC__in REFTIME *pStart,
            __RPC__in REFTIME *pStop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *ModifyStopTime )( 
            IAMTimelineSrc * This,
            REFERENCE_TIME Stop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *ModifyStopTime2 )( 
            IAMTimelineSrc * This,
            REFTIME Stop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *FixMediaTimes )( 
            IAMTimelineSrc * This,
            __RPC__in REFERENCE_TIME *pStart,
            __RPC__in REFERENCE_TIME *pStop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *FixMediaTimes2 )( 
            IAMTimelineSrc * This,
            __RPC__in REFTIME *pStart,
            __RPC__in REFTIME *pStop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetMediaTimes )( 
            IAMTimelineSrc * This,
            REFERENCE_TIME Start,
            REFERENCE_TIME Stop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetMediaTimes2 )( 
            IAMTimelineSrc * This,
            REFTIME Start,
            REFTIME Stop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetMediaLength )( 
            IAMTimelineSrc * This,
            REFERENCE_TIME Length);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetMediaLength2 )( 
            IAMTimelineSrc * This,
            REFTIME Length);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetMediaLength )( 
            IAMTimelineSrc * This,
            __RPC__in REFERENCE_TIME *pLength);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetMediaLength2 )( 
            IAMTimelineSrc * This,
            __RPC__in REFTIME *pLength);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetMediaName )( 
            IAMTimelineSrc * This,
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetMediaName )( 
            IAMTimelineSrc * This,
            __RPC__in BSTR newVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SpliceWithNext )( 
            IAMTimelineSrc * This,
            __RPC__in_opt IAMTimelineObj *pNext);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetStreamNumber )( 
            IAMTimelineSrc * This,
            __RPC__in long *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetStreamNumber )( 
            IAMTimelineSrc * This,
            long Val);
        
        HRESULT ( STDMETHODCALLTYPE *IsNormalRate )( 
            IAMTimelineSrc * This,
            __RPC__in BOOL *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetDefaultFPS )( 
            IAMTimelineSrc * This,
            __RPC__in double *pFPS);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetDefaultFPS )( 
            IAMTimelineSrc * This,
            double FPS);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetStretchMode )( 
            IAMTimelineSrc * This,
            __RPC__in int *pnStretchMode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetStretchMode )( 
            IAMTimelineSrc * This,
            int nStretchMode);
        
        END_INTERFACE
    } IAMTimelineSrcVtbl;

    interface IAMTimelineSrc
    {
        CONST_VTBL struct IAMTimelineSrcVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMTimelineSrc_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAMTimelineSrc_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAMTimelineSrc_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAMTimelineSrc_GetMediaTimes(This,pStart,pStop)	\
    ( (This)->lpVtbl -> GetMediaTimes(This,pStart,pStop) ) 

#define IAMTimelineSrc_GetMediaTimes2(This,pStart,pStop)	\
    ( (This)->lpVtbl -> GetMediaTimes2(This,pStart,pStop) ) 

#define IAMTimelineSrc_ModifyStopTime(This,Stop)	\
    ( (This)->lpVtbl -> ModifyStopTime(This,Stop) ) 

#define IAMTimelineSrc_ModifyStopTime2(This,Stop)	\
    ( (This)->lpVtbl -> ModifyStopTime2(This,Stop) ) 

#define IAMTimelineSrc_FixMediaTimes(This,pStart,pStop)	\
    ( (This)->lpVtbl -> FixMediaTimes(This,pStart,pStop) ) 

#define IAMTimelineSrc_FixMediaTimes2(This,pStart,pStop)	\
    ( (This)->lpVtbl -> FixMediaTimes2(This,pStart,pStop) ) 

#define IAMTimelineSrc_SetMediaTimes(This,Start,Stop)	\
    ( (This)->lpVtbl -> SetMediaTimes(This,Start,Stop) ) 

#define IAMTimelineSrc_SetMediaTimes2(This,Start,Stop)	\
    ( (This)->lpVtbl -> SetMediaTimes2(This,Start,Stop) ) 

#define IAMTimelineSrc_SetMediaLength(This,Length)	\
    ( (This)->lpVtbl -> SetMediaLength(This,Length) ) 

#define IAMTimelineSrc_SetMediaLength2(This,Length)	\
    ( (This)->lpVtbl -> SetMediaLength2(This,Length) ) 

#define IAMTimelineSrc_GetMediaLength(This,pLength)	\
    ( (This)->lpVtbl -> GetMediaLength(This,pLength) ) 

#define IAMTimelineSrc_GetMediaLength2(This,pLength)	\
    ( (This)->lpVtbl -> GetMediaLength2(This,pLength) ) 

#define IAMTimelineSrc_GetMediaName(This,pVal)	\
    ( (This)->lpVtbl -> GetMediaName(This,pVal) ) 

#define IAMTimelineSrc_SetMediaName(This,newVal)	\
    ( (This)->lpVtbl -> SetMediaName(This,newVal) ) 

#define IAMTimelineSrc_SpliceWithNext(This,pNext)	\
    ( (This)->lpVtbl -> SpliceWithNext(This,pNext) ) 

#define IAMTimelineSrc_GetStreamNumber(This,pVal)	\
    ( (This)->lpVtbl -> GetStreamNumber(This,pVal) ) 

#define IAMTimelineSrc_SetStreamNumber(This,Val)	\
    ( (This)->lpVtbl -> SetStreamNumber(This,Val) ) 

#define IAMTimelineSrc_IsNormalRate(This,pVal)	\
    ( (This)->lpVtbl -> IsNormalRate(This,pVal) ) 

#define IAMTimelineSrc_GetDefaultFPS(This,pFPS)	\
    ( (This)->lpVtbl -> GetDefaultFPS(This,pFPS) ) 

#define IAMTimelineSrc_SetDefaultFPS(This,FPS)	\
    ( (This)->lpVtbl -> SetDefaultFPS(This,FPS) ) 

#define IAMTimelineSrc_GetStretchMode(This,pnStretchMode)	\
    ( (This)->lpVtbl -> GetStretchMode(This,pnStretchMode) ) 

#define IAMTimelineSrc_SetStretchMode(This,nStretchMode)	\
    ( (This)->lpVtbl -> SetStretchMode(This,nStretchMode) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAMTimelineSrc_INTERFACE_DEFINED__ */


#ifndef __IAMTimelineTrack_INTERFACE_DEFINED__
#define __IAMTimelineTrack_INTERFACE_DEFINED__

/* interface IAMTimelineTrack */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAMTimelineTrack;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EAE58538-622E-11d2-8CAD-00A024580902")
    IAMTimelineTrack : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SrcAdd( 
            __RPC__in_opt IAMTimelineObj *pSource) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetNextSrc( 
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppSrc,
            __RPC__in REFERENCE_TIME *pInOut) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetNextSrc2( 
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppSrc,
            __RPC__in REFTIME *pInOut) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE MoveEverythingBy( 
            REFERENCE_TIME Start,
            REFERENCE_TIME MoveBy) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE MoveEverythingBy2( 
            REFTIME Start,
            REFTIME MoveBy) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSourcesCount( 
            __RPC__in long *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AreYouBlank( 
            __RPC__in long *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSrcAtTime( 
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppSrc,
            REFERENCE_TIME Time,
            long SearchDirection) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSrcAtTime2( 
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppSrc,
            REFTIME Time,
            long SearchDirection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InsertSpace( 
            REFERENCE_TIME rtStart,
            REFERENCE_TIME rtEnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InsertSpace2( 
            REFTIME rtStart,
            REFTIME rtEnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ZeroBetween( 
            REFERENCE_TIME rtStart,
            REFERENCE_TIME rtEnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ZeroBetween2( 
            REFTIME rtStart,
            REFTIME rtEnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextSrcEx( 
            __RPC__in_opt IAMTimelineObj *pLast,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppNext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMTimelineTrackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMTimelineTrack * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMTimelineTrack * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMTimelineTrack * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SrcAdd )( 
            IAMTimelineTrack * This,
            __RPC__in_opt IAMTimelineObj *pSource);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetNextSrc )( 
            IAMTimelineTrack * This,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppSrc,
            __RPC__in REFERENCE_TIME *pInOut);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetNextSrc2 )( 
            IAMTimelineTrack * This,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppSrc,
            __RPC__in REFTIME *pInOut);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MoveEverythingBy )( 
            IAMTimelineTrack * This,
            REFERENCE_TIME Start,
            REFERENCE_TIME MoveBy);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MoveEverythingBy2 )( 
            IAMTimelineTrack * This,
            REFTIME Start,
            REFTIME MoveBy);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetSourcesCount )( 
            IAMTimelineTrack * This,
            __RPC__in long *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *AreYouBlank )( 
            IAMTimelineTrack * This,
            __RPC__in long *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetSrcAtTime )( 
            IAMTimelineTrack * This,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppSrc,
            REFERENCE_TIME Time,
            long SearchDirection);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetSrcAtTime2 )( 
            IAMTimelineTrack * This,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppSrc,
            REFTIME Time,
            long SearchDirection);
        
        HRESULT ( STDMETHODCALLTYPE *InsertSpace )( 
            IAMTimelineTrack * This,
            REFERENCE_TIME rtStart,
            REFERENCE_TIME rtEnd);
        
        HRESULT ( STDMETHODCALLTYPE *InsertSpace2 )( 
            IAMTimelineTrack * This,
            REFTIME rtStart,
            REFTIME rtEnd);
        
        HRESULT ( STDMETHODCALLTYPE *ZeroBetween )( 
            IAMTimelineTrack * This,
            REFERENCE_TIME rtStart,
            REFERENCE_TIME rtEnd);
        
        HRESULT ( STDMETHODCALLTYPE *ZeroBetween2 )( 
            IAMTimelineTrack * This,
            REFTIME rtStart,
            REFTIME rtEnd);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextSrcEx )( 
            IAMTimelineTrack * This,
            __RPC__in_opt IAMTimelineObj *pLast,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppNext);
        
        END_INTERFACE
    } IAMTimelineTrackVtbl;

    interface IAMTimelineTrack
    {
        CONST_VTBL struct IAMTimelineTrackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMTimelineTrack_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAMTimelineTrack_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAMTimelineTrack_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAMTimelineTrack_SrcAdd(This,pSource)	\
    ( (This)->lpVtbl -> SrcAdd(This,pSource) ) 

#define IAMTimelineTrack_GetNextSrc(This,ppSrc,pInOut)	\
    ( (This)->lpVtbl -> GetNextSrc(This,ppSrc,pInOut) ) 

#define IAMTimelineTrack_GetNextSrc2(This,ppSrc,pInOut)	\
    ( (This)->lpVtbl -> GetNextSrc2(This,ppSrc,pInOut) ) 

#define IAMTimelineTrack_MoveEverythingBy(This,Start,MoveBy)	\
    ( (This)->lpVtbl -> MoveEverythingBy(This,Start,MoveBy) ) 

#define IAMTimelineTrack_MoveEverythingBy2(This,Start,MoveBy)	\
    ( (This)->lpVtbl -> MoveEverythingBy2(This,Start,MoveBy) ) 

#define IAMTimelineTrack_GetSourcesCount(This,pVal)	\
    ( (This)->lpVtbl -> GetSourcesCount(This,pVal) ) 

#define IAMTimelineTrack_AreYouBlank(This,pVal)	\
    ( (This)->lpVtbl -> AreYouBlank(This,pVal) ) 

#define IAMTimelineTrack_GetSrcAtTime(This,ppSrc,Time,SearchDirection)	\
    ( (This)->lpVtbl -> GetSrcAtTime(This,ppSrc,Time,SearchDirection) ) 

#define IAMTimelineTrack_GetSrcAtTime2(This,ppSrc,Time,SearchDirection)	\
    ( (This)->lpVtbl -> GetSrcAtTime2(This,ppSrc,Time,SearchDirection) ) 

#define IAMTimelineTrack_InsertSpace(This,rtStart,rtEnd)	\
    ( (This)->lpVtbl -> InsertSpace(This,rtStart,rtEnd) ) 

#define IAMTimelineTrack_InsertSpace2(This,rtStart,rtEnd)	\
    ( (This)->lpVtbl -> InsertSpace2(This,rtStart,rtEnd) ) 

#define IAMTimelineTrack_ZeroBetween(This,rtStart,rtEnd)	\
    ( (This)->lpVtbl -> ZeroBetween(This,rtStart,rtEnd) ) 

#define IAMTimelineTrack_ZeroBetween2(This,rtStart,rtEnd)	\
    ( (This)->lpVtbl -> ZeroBetween2(This,rtStart,rtEnd) ) 

#define IAMTimelineTrack_GetNextSrcEx(This,pLast,ppNext)	\
    ( (This)->lpVtbl -> GetNextSrcEx(This,pLast,ppNext) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAMTimelineTrack_INTERFACE_DEFINED__ */


#ifndef __IAMTimelineVirtualTrack_INTERFACE_DEFINED__
#define __IAMTimelineVirtualTrack_INTERFACE_DEFINED__

/* interface IAMTimelineVirtualTrack */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAMTimelineVirtualTrack;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A8ED5F80-C2C7-11d2-8D39-00A0C9441E20")
    IAMTimelineVirtualTrack : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE TrackGetPriority( 
            __RPC__in long *pPriority) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetTrackDirty( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMTimelineVirtualTrackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMTimelineVirtualTrack * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMTimelineVirtualTrack * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMTimelineVirtualTrack * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *TrackGetPriority )( 
            IAMTimelineVirtualTrack * This,
            __RPC__in long *pPriority);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetTrackDirty )( 
            IAMTimelineVirtualTrack * This);
        
        END_INTERFACE
    } IAMTimelineVirtualTrackVtbl;

    interface IAMTimelineVirtualTrack
    {
        CONST_VTBL struct IAMTimelineVirtualTrackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMTimelineVirtualTrack_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAMTimelineVirtualTrack_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAMTimelineVirtualTrack_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAMTimelineVirtualTrack_TrackGetPriority(This,pPriority)	\
    ( (This)->lpVtbl -> TrackGetPriority(This,pPriority) ) 

#define IAMTimelineVirtualTrack_SetTrackDirty(This)	\
    ( (This)->lpVtbl -> SetTrackDirty(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAMTimelineVirtualTrack_INTERFACE_DEFINED__ */


#ifndef __IAMTimelineComp_INTERFACE_DEFINED__
#define __IAMTimelineComp_INTERFACE_DEFINED__

/* interface IAMTimelineComp */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAMTimelineComp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EAE58536-622E-11d2-8CAD-00A024580902")
    IAMTimelineComp : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE VTrackInsBefore( 
            __RPC__in_opt IAMTimelineObj *pVirtualTrack,
            long Priority) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE VTrackSwapPriorities( 
            long VirtualTrackA,
            long VirtualTrackB) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE VTrackGetCount( 
            __RPC__in long *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetVTrack( 
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppVirtualTrack,
            long Which) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCountOfType( 
            __RPC__in long *pVal,
            __RPC__in long *pValWithComps,
            TIMELINE_MAJOR_TYPE MajorType) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetRecursiveLayerOfType( 
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppVirtualTrack,
            long WhichLayer,
            TIMELINE_MAJOR_TYPE Type) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetRecursiveLayerOfTypeI( 
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppVirtualTrack,
            /* [out][in] */ __RPC__inout long *pWhichLayer,
            TIMELINE_MAJOR_TYPE Type) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextVTrack( 
            __RPC__in_opt IAMTimelineObj *pVirtualTrack,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppNextVirtualTrack) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMTimelineCompVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMTimelineComp * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMTimelineComp * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMTimelineComp * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *VTrackInsBefore )( 
            IAMTimelineComp * This,
            __RPC__in_opt IAMTimelineObj *pVirtualTrack,
            long Priority);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *VTrackSwapPriorities )( 
            IAMTimelineComp * This,
            long VirtualTrackA,
            long VirtualTrackB);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *VTrackGetCount )( 
            IAMTimelineComp * This,
            __RPC__in long *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetVTrack )( 
            IAMTimelineComp * This,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppVirtualTrack,
            long Which);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetCountOfType )( 
            IAMTimelineComp * This,
            __RPC__in long *pVal,
            __RPC__in long *pValWithComps,
            TIMELINE_MAJOR_TYPE MajorType);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetRecursiveLayerOfType )( 
            IAMTimelineComp * This,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppVirtualTrack,
            long WhichLayer,
            TIMELINE_MAJOR_TYPE Type);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetRecursiveLayerOfTypeI )( 
            IAMTimelineComp * This,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppVirtualTrack,
            /* [out][in] */ __RPC__inout long *pWhichLayer,
            TIMELINE_MAJOR_TYPE Type);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextVTrack )( 
            IAMTimelineComp * This,
            __RPC__in_opt IAMTimelineObj *pVirtualTrack,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppNextVirtualTrack);
        
        END_INTERFACE
    } IAMTimelineCompVtbl;

    interface IAMTimelineComp
    {
        CONST_VTBL struct IAMTimelineCompVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMTimelineComp_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAMTimelineComp_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAMTimelineComp_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAMTimelineComp_VTrackInsBefore(This,pVirtualTrack,Priority)	\
    ( (This)->lpVtbl -> VTrackInsBefore(This,pVirtualTrack,Priority) ) 

#define IAMTimelineComp_VTrackSwapPriorities(This,VirtualTrackA,VirtualTrackB)	\
    ( (This)->lpVtbl -> VTrackSwapPriorities(This,VirtualTrackA,VirtualTrackB) ) 

#define IAMTimelineComp_VTrackGetCount(This,pVal)	\
    ( (This)->lpVtbl -> VTrackGetCount(This,pVal) ) 

#define IAMTimelineComp_GetVTrack(This,ppVirtualTrack,Which)	\
    ( (This)->lpVtbl -> GetVTrack(This,ppVirtualTrack,Which) ) 

#define IAMTimelineComp_GetCountOfType(This,pVal,pValWithComps,MajorType)	\
    ( (This)->lpVtbl -> GetCountOfType(This,pVal,pValWithComps,MajorType) ) 

#define IAMTimelineComp_GetRecursiveLayerOfType(This,ppVirtualTrack,WhichLayer,Type)	\
    ( (This)->lpVtbl -> GetRecursiveLayerOfType(This,ppVirtualTrack,WhichLayer,Type) ) 

#define IAMTimelineComp_GetRecursiveLayerOfTypeI(This,ppVirtualTrack,pWhichLayer,Type)	\
    ( (This)->lpVtbl -> GetRecursiveLayerOfTypeI(This,ppVirtualTrack,pWhichLayer,Type) ) 

#define IAMTimelineComp_GetNextVTrack(This,pVirtualTrack,ppNextVirtualTrack)	\
    ( (This)->lpVtbl -> GetNextVTrack(This,pVirtualTrack,ppNextVirtualTrack) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAMTimelineComp_INTERFACE_DEFINED__ */


#ifndef __IAMTimelineGroup_INTERFACE_DEFINED__
#define __IAMTimelineGroup_INTERFACE_DEFINED__

/* interface IAMTimelineGroup */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAMTimelineGroup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9EED4F00-B8A6-11d2-8023-00C0DF10D434")
    IAMTimelineGroup : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetTimeline( 
            __RPC__in_opt IAMTimeline *pTimeline) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetTimeline( 
            /* [out] */ __RPC__deref_out_opt IAMTimeline **ppTimeline) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPriority( 
            __RPC__in long *pPriority) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMediaType( 
            /* [out] */ __RPC__out AM_MEDIA_TYPE *__MIDL__IAMTimelineGroup0000) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetMediaType( 
            /* [in] */ __RPC__in AM_MEDIA_TYPE *__MIDL__IAMTimelineGroup0001) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetOutputFPS( 
            double FPS) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetOutputFPS( 
            __RPC__in double *pFPS) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetGroupName( 
            __RPC__in BSTR pGroupName) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetGroupName( 
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pGroupName) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetPreviewMode( 
            BOOL fPreview) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPreviewMode( 
            __RPC__in BOOL *pfPreview) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetMediaTypeForVB( 
            /* [in] */ long Val) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetOutputBuffering( 
            /* [out] */ __RPC__out int *pnBuffer) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetOutputBuffering( 
            /* [in] */ int nBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSmartRecompressFormat( 
            __RPC__in long *pFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSmartRecompressFormat( 
            __RPC__deref_in_opt long **ppFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsSmartRecompressFormatSet( 
            __RPC__in BOOL *pVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsRecompressFormatDirty( 
            __RPC__in BOOL *pVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearRecompressFormatDirty( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRecompFormatFromSource( 
            __RPC__in_opt IAMTimelineSrc *pSource) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMTimelineGroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMTimelineGroup * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMTimelineGroup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMTimelineGroup * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetTimeline )( 
            IAMTimelineGroup * This,
            __RPC__in_opt IAMTimeline *pTimeline);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetTimeline )( 
            IAMTimelineGroup * This,
            /* [out] */ __RPC__deref_out_opt IAMTimeline **ppTimeline);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPriority )( 
            IAMTimelineGroup * This,
            __RPC__in long *pPriority);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetMediaType )( 
            IAMTimelineGroup * This,
            /* [out] */ __RPC__out AM_MEDIA_TYPE *__MIDL__IAMTimelineGroup0000);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetMediaType )( 
            IAMTimelineGroup * This,
            /* [in] */ __RPC__in AM_MEDIA_TYPE *__MIDL__IAMTimelineGroup0001);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetOutputFPS )( 
            IAMTimelineGroup * This,
            double FPS);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetOutputFPS )( 
            IAMTimelineGroup * This,
            __RPC__in double *pFPS);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetGroupName )( 
            IAMTimelineGroup * This,
            __RPC__in BSTR pGroupName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetGroupName )( 
            IAMTimelineGroup * This,
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pGroupName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetPreviewMode )( 
            IAMTimelineGroup * This,
            BOOL fPreview);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPreviewMode )( 
            IAMTimelineGroup * This,
            __RPC__in BOOL *pfPreview);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetMediaTypeForVB )( 
            IAMTimelineGroup * This,
            /* [in] */ long Val);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetOutputBuffering )( 
            IAMTimelineGroup * This,
            /* [out] */ __RPC__out int *pnBuffer);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetOutputBuffering )( 
            IAMTimelineGroup * This,
            /* [in] */ int nBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *SetSmartRecompressFormat )( 
            IAMTimelineGroup * This,
            __RPC__in long *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetSmartRecompressFormat )( 
            IAMTimelineGroup * This,
            __RPC__deref_in_opt long **ppFormat);
        
        HRESULT ( STDMETHODCALLTYPE *IsSmartRecompressFormatSet )( 
            IAMTimelineGroup * This,
            __RPC__in BOOL *pVal);
        
        HRESULT ( STDMETHODCALLTYPE *IsRecompressFormatDirty )( 
            IAMTimelineGroup * This,
            __RPC__in BOOL *pVal);
        
        HRESULT ( STDMETHODCALLTYPE *ClearRecompressFormatDirty )( 
            IAMTimelineGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetRecompFormatFromSource )( 
            IAMTimelineGroup * This,
            __RPC__in_opt IAMTimelineSrc *pSource);
        
        END_INTERFACE
    } IAMTimelineGroupVtbl;

    interface IAMTimelineGroup
    {
        CONST_VTBL struct IAMTimelineGroupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMTimelineGroup_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAMTimelineGroup_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAMTimelineGroup_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAMTimelineGroup_SetTimeline(This,pTimeline)	\
    ( (This)->lpVtbl -> SetTimeline(This,pTimeline) ) 

#define IAMTimelineGroup_GetTimeline(This,ppTimeline)	\
    ( (This)->lpVtbl -> GetTimeline(This,ppTimeline) ) 

#define IAMTimelineGroup_GetPriority(This,pPriority)	\
    ( (This)->lpVtbl -> GetPriority(This,pPriority) ) 

#define IAMTimelineGroup_GetMediaType(This,__MIDL__IAMTimelineGroup0000)	\
    ( (This)->lpVtbl -> GetMediaType(This,__MIDL__IAMTimelineGroup0000) ) 

#define IAMTimelineGroup_SetMediaType(This,__MIDL__IAMTimelineGroup0001)	\
    ( (This)->lpVtbl -> SetMediaType(This,__MIDL__IAMTimelineGroup0001) ) 

#define IAMTimelineGroup_SetOutputFPS(This,FPS)	\
    ( (This)->lpVtbl -> SetOutputFPS(This,FPS) ) 

#define IAMTimelineGroup_GetOutputFPS(This,pFPS)	\
    ( (This)->lpVtbl -> GetOutputFPS(This,pFPS) ) 

#define IAMTimelineGroup_SetGroupName(This,pGroupName)	\
    ( (This)->lpVtbl -> SetGroupName(This,pGroupName) ) 

#define IAMTimelineGroup_GetGroupName(This,pGroupName)	\
    ( (This)->lpVtbl -> GetGroupName(This,pGroupName) ) 

#define IAMTimelineGroup_SetPreviewMode(This,fPreview)	\
    ( (This)->lpVtbl -> SetPreviewMode(This,fPreview) ) 

#define IAMTimelineGroup_GetPreviewMode(This,pfPreview)	\
    ( (This)->lpVtbl -> GetPreviewMode(This,pfPreview) ) 

#define IAMTimelineGroup_SetMediaTypeForVB(This,Val)	\
    ( (This)->lpVtbl -> SetMediaTypeForVB(This,Val) ) 

#define IAMTimelineGroup_GetOutputBuffering(This,pnBuffer)	\
    ( (This)->lpVtbl -> GetOutputBuffering(This,pnBuffer) ) 

#define IAMTimelineGroup_SetOutputBuffering(This,nBuffer)	\
    ( (This)->lpVtbl -> SetOutputBuffering(This,nBuffer) ) 

#define IAMTimelineGroup_SetSmartRecompressFormat(This,pFormat)	\
    ( (This)->lpVtbl -> SetSmartRecompressFormat(This,pFormat) ) 

#define IAMTimelineGroup_GetSmartRecompressFormat(This,ppFormat)	\
    ( (This)->lpVtbl -> GetSmartRecompressFormat(This,ppFormat) ) 

#define IAMTimelineGroup_IsSmartRecompressFormatSet(This,pVal)	\
    ( (This)->lpVtbl -> IsSmartRecompressFormatSet(This,pVal) ) 

#define IAMTimelineGroup_IsRecompressFormatDirty(This,pVal)	\
    ( (This)->lpVtbl -> IsRecompressFormatDirty(This,pVal) ) 

#define IAMTimelineGroup_ClearRecompressFormatDirty(This)	\
    ( (This)->lpVtbl -> ClearRecompressFormatDirty(This) ) 

#define IAMTimelineGroup_SetRecompFormatFromSource(This,pSource)	\
    ( (This)->lpVtbl -> SetRecompFormatFromSource(This,pSource) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAMTimelineGroup_INTERFACE_DEFINED__ */


#ifndef __IAMTimeline_INTERFACE_DEFINED__
#define __IAMTimeline_INTERFACE_DEFINED__

/* interface IAMTimeline */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAMTimeline;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("78530B74-61F9-11D2-8CAD-00A024580902")
    IAMTimeline : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CreateEmptyNode( 
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppObj,
            TIMELINE_MAJOR_TYPE Type) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddGroup( 
            __RPC__in_opt IAMTimelineObj *pGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemGroupFromList( 
            __RPC__in_opt IAMTimelineObj *pGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGroup( 
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppGroup,
            long WhichGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGroupCount( 
            __RPC__in long *pCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearAllGroups( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInsertMode( 
            __RPC__in long *pMode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetInsertMode( 
            long Mode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EnableTransitions( 
            BOOL fEnabled) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE TransitionsEnabled( 
            __RPC__in BOOL *pfEnabled) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EnableEffects( 
            BOOL fEnabled) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EffectsEnabled( 
            __RPC__in BOOL *pfEnabled) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetInterestRange( 
            REFERENCE_TIME Start,
            REFERENCE_TIME Stop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDuration( 
            __RPC__in REFERENCE_TIME *pDuration) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDuration2( 
            __RPC__in double *pDuration) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetDefaultFPS( 
            double FPS) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDefaultFPS( 
            __RPC__in double *pFPS) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsDirty( 
            __RPC__in BOOL *pDirty) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDirtyRange( 
            __RPC__in REFERENCE_TIME *pStart,
            __RPC__in REFERENCE_TIME *pStop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCountOfType( 
            long Group,
            __RPC__in long *pVal,
            __RPC__in long *pValWithComps,
            TIMELINE_MAJOR_TYPE MajorType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ValidateSourceNames( 
            long ValidateFlags,
            __RPC__in_opt IMediaLocator *pOverride,
            LONG_PTR NotifyEventHandle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDefaultTransition( 
            __RPC__in GUID *pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultTransition( 
            __RPC__in GUID *pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDefaultEffect( 
            __RPC__in GUID *pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultEffect( 
            __RPC__in GUID *pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDefaultTransitionB( 
            __RPC__in BSTR pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultTransitionB( 
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDefaultEffectB( 
            __RPC__in BSTR pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultEffectB( 
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pGuid) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMTimelineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMTimeline * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMTimeline * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMTimeline * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *CreateEmptyNode )( 
            IAMTimeline * This,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppObj,
            TIMELINE_MAJOR_TYPE Type);
        
        HRESULT ( STDMETHODCALLTYPE *AddGroup )( 
            IAMTimeline * This,
            __RPC__in_opt IAMTimelineObj *pGroup);
        
        HRESULT ( STDMETHODCALLTYPE *RemGroupFromList )( 
            IAMTimeline * This,
            __RPC__in_opt IAMTimelineObj *pGroup);
        
        HRESULT ( STDMETHODCALLTYPE *GetGroup )( 
            IAMTimeline * This,
            /* [out] */ __RPC__deref_out_opt IAMTimelineObj **ppGroup,
            long WhichGroup);
        
        HRESULT ( STDMETHODCALLTYPE *GetGroupCount )( 
            IAMTimeline * This,
            __RPC__in long *pCount);
        
        HRESULT ( STDMETHODCALLTYPE *ClearAllGroups )( 
            IAMTimeline * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInsertMode )( 
            IAMTimeline * This,
            __RPC__in long *pMode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetInsertMode )( 
            IAMTimeline * This,
            long Mode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnableTransitions )( 
            IAMTimeline * This,
            BOOL fEnabled);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *TransitionsEnabled )( 
            IAMTimeline * This,
            __RPC__in BOOL *pfEnabled);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnableEffects )( 
            IAMTimeline * This,
            BOOL fEnabled);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EffectsEnabled )( 
            IAMTimeline * This,
            __RPC__in BOOL *pfEnabled);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetInterestRange )( 
            IAMTimeline * This,
            REFERENCE_TIME Start,
            REFERENCE_TIME Stop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetDuration )( 
            IAMTimeline * This,
            __RPC__in REFERENCE_TIME *pDuration);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetDuration2 )( 
            IAMTimeline * This,
            __RPC__in double *pDuration);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetDefaultFPS )( 
            IAMTimeline * This,
            double FPS);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetDefaultFPS )( 
            IAMTimeline * This,
            __RPC__in double *pFPS);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *IsDirty )( 
            IAMTimeline * This,
            __RPC__in BOOL *pDirty);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetDirtyRange )( 
            IAMTimeline * This,
            __RPC__in REFERENCE_TIME *pStart,
            __RPC__in REFERENCE_TIME *pStop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetCountOfType )( 
            IAMTimeline * This,
            long Group,
            __RPC__in long *pVal,
            __RPC__in long *pValWithComps,
            TIMELINE_MAJOR_TYPE MajorType);
        
        HRESULT ( STDMETHODCALLTYPE *ValidateSourceNames )( 
            IAMTimeline * This,
            long ValidateFlags,
            __RPC__in_opt IMediaLocator *pOverride,
            LONG_PTR NotifyEventHandle);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefaultTransition )( 
            IAMTimeline * This,
            __RPC__in GUID *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultTransition )( 
            IAMTimeline * This,
            __RPC__in GUID *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefaultEffect )( 
            IAMTimeline * This,
            __RPC__in GUID *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultEffect )( 
            IAMTimeline * This,
            __RPC__in GUID *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefaultTransitionB )( 
            IAMTimeline * This,
            __RPC__in BSTR pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultTransitionB )( 
            IAMTimeline * This,
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefaultEffectB )( 
            IAMTimeline * This,
            __RPC__in BSTR pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultEffectB )( 
            IAMTimeline * This,
            /* [retval][out] */ __RPC__deref_out_opt BSTR *pGuid);
        
        END_INTERFACE
    } IAMTimelineVtbl;

    interface IAMTimeline
    {
        CONST_VTBL struct IAMTimelineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMTimeline_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAMTimeline_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAMTimeline_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAMTimeline_CreateEmptyNode(This,ppObj,Type)	\
    ( (This)->lpVtbl -> CreateEmptyNode(This,ppObj,Type) ) 

#define IAMTimeline_AddGroup(This,pGroup)	\
    ( (This)->lpVtbl -> AddGroup(This,pGroup) ) 

#define IAMTimeline_RemGroupFromList(This,pGroup)	\
    ( (This)->lpVtbl -> RemGroupFromList(This,pGroup) ) 

#define IAMTimeline_GetGroup(This,ppGroup,WhichGroup)	\
    ( (This)->lpVtbl -> GetGroup(This,ppGroup,WhichGroup) ) 

#define IAMTimeline_GetGroupCount(This,pCount)	\
    ( (This)->lpVtbl -> GetGroupCount(This,pCount) ) 

#define IAMTimeline_ClearAllGroups(This)	\
    ( (This)->lpVtbl -> ClearAllGroups(This) ) 

#define IAMTimeline_GetInsertMode(This,pMode)	\
    ( (This)->lpVtbl -> GetInsertMode(This,pMode) ) 

#define IAMTimeline_SetInsertMode(This,Mode)	\
    ( (This)->lpVtbl -> SetInsertMode(This,Mode) ) 

#define IAMTimeline_EnableTransitions(This,fEnabled)	\
    ( (This)->lpVtbl -> EnableTransitions(This,fEnabled) ) 

#define IAMTimeline_TransitionsEnabled(This,pfEnabled)	\
    ( (This)->lpVtbl -> TransitionsEnabled(This,pfEnabled) ) 

#define IAMTimeline_EnableEffects(This,fEnabled)	\
    ( (This)->lpVtbl -> EnableEffects(This,fEnabled) ) 

#define IAMTimeline_EffectsEnabled(This,pfEnabled)	\
    ( (This)->lpVtbl -> EffectsEnabled(This,pfEnabled) ) 

#define IAMTimeline_SetInterestRange(This,Start,Stop)	\
    ( (This)->lpVtbl -> SetInterestRange(This,Start,Stop) ) 

#define IAMTimeline_GetDuration(This,pDuration)	\
    ( (This)->lpVtbl -> GetDuration(This,pDuration) ) 

#define IAMTimeline_GetDuration2(This,pDuration)	\
    ( (This)->lpVtbl -> GetDuration2(This,pDuration) ) 

#define IAMTimeline_SetDefaultFPS(This,FPS)	\
    ( (This)->lpVtbl -> SetDefaultFPS(This,FPS) ) 

#define IAMTimeline_GetDefaultFPS(This,pFPS)	\
    ( (This)->lpVtbl -> GetDefaultFPS(This,pFPS) ) 

#define IAMTimeline_IsDirty(This,pDirty)	\
    ( (This)->lpVtbl -> IsDirty(This,pDirty) ) 

#define IAMTimeline_GetDirtyRange(This,pStart,pStop)	\
    ( (This)->lpVtbl -> GetDirtyRange(This,pStart,pStop) ) 

#define IAMTimeline_GetCountOfType(This,Group,pVal,pValWithComps,MajorType)	\
    ( (This)->lpVtbl -> GetCountOfType(This,Group,pVal,pValWithComps,MajorType) ) 

#define IAMTimeline_ValidateSourceNames(This,ValidateFlags,pOverride,NotifyEventHandle)	\
    ( (This)->lpVtbl -> ValidateSourceNames(This,ValidateFlags,pOverride,NotifyEventHandle) ) 

#define IAMTimeline_SetDefaultTransition(This,pGuid)	\
    ( (This)->lpVtbl -> SetDefaultTransition(This,pGuid) ) 

#define IAMTimeline_GetDefaultTransition(This,pGuid)	\
    ( (This)->lpVtbl -> GetDefaultTransition(This,pGuid) ) 

#define IAMTimeline_SetDefaultEffect(This,pGuid)	\
    ( (This)->lpVtbl -> SetDefaultEffect(This,pGuid) ) 

#define IAMTimeline_GetDefaultEffect(This,pGuid)	\
    ( (This)->lpVtbl -> GetDefaultEffect(This,pGuid) ) 

#define IAMTimeline_SetDefaultTransitionB(This,pGuid)	\
    ( (This)->lpVtbl -> SetDefaultTransitionB(This,pGuid) ) 

#define IAMTimeline_GetDefaultTransitionB(This,pGuid)	\
    ( (This)->lpVtbl -> GetDefaultTransitionB(This,pGuid) ) 

#define IAMTimeline_SetDefaultEffectB(This,pGuid)	\
    ( (This)->lpVtbl -> SetDefaultEffectB(This,pGuid) ) 

#define IAMTimeline_GetDefaultEffectB(This,pGuid)	\
    ( (This)->lpVtbl -> GetDefaultEffectB(This,pGuid) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAMTimeline_INTERFACE_DEFINED__ */


#ifndef __IXml2Dex_INTERFACE_DEFINED__
#define __IXml2Dex_INTERFACE_DEFINED__

/* interface IXml2Dex */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IXml2Dex;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("18C628ED-962A-11D2-8D08-00A0C9441E20")
    IXml2Dex : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateGraphFromFile( 
            /* [out] */ __RPC__deref_out_opt IUnknown **ppGraph,
            __RPC__in_opt IUnknown *pTimeline,
            __RPC__in BSTR Filename) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE WriteGrfFile( 
            __RPC__in_opt IUnknown *pGraph,
            __RPC__in BSTR FileName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE WriteXMLFile( 
            __RPC__in_opt IUnknown *pTimeline,
            __RPC__in BSTR FileName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReadXMLFile( 
            __RPC__in_opt IUnknown *pTimeline,
            __RPC__in BSTR XMLName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Delete( 
            __RPC__in_opt IUnknown *pTimeline,
            double dStart,
            double dEnd) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE WriteXMLPart( 
            __RPC__in_opt IUnknown *pTimeline,
            double dStart,
            double dEnd,
            __RPC__in BSTR FileName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PasteXMLFile( 
            __RPC__in_opt IUnknown *pTimeline,
            double dStart,
            __RPC__in BSTR FileName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CopyXML( 
            __RPC__in_opt IUnknown *pTimeline,
            double dStart,
            double dEnd) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PasteXML( 
            __RPC__in_opt IUnknown *pTimeline,
            double dStart) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReadXML( 
            __RPC__in_opt IUnknown *pTimeline,
            __RPC__in_opt IUnknown *pXML) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE WriteXML( 
            __RPC__in_opt IUnknown *pTimeline,
            __RPC__deref_in_opt BSTR *pbstrXML) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXml2DexVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXml2Dex * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXml2Dex * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXml2Dex * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IXml2Dex * This,
            /* [out] */ __RPC__out UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IXml2Dex * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IXml2Dex * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IXml2Dex * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CreateGraphFromFile )( 
            IXml2Dex * This,
            /* [out] */ __RPC__deref_out_opt IUnknown **ppGraph,
            __RPC__in_opt IUnknown *pTimeline,
            __RPC__in BSTR Filename);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *WriteGrfFile )( 
            IXml2Dex * This,
            __RPC__in_opt IUnknown *pGraph,
            __RPC__in BSTR FileName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *WriteXMLFile )( 
            IXml2Dex * This,
            __RPC__in_opt IUnknown *pTimeline,
            __RPC__in BSTR FileName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ReadXMLFile )( 
            IXml2Dex * This,
            __RPC__in_opt IUnknown *pTimeline,
            __RPC__in BSTR XMLName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IXml2Dex * This,
            __RPC__in_opt IUnknown *pTimeline,
            double dStart,
            double dEnd);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *WriteXMLPart )( 
            IXml2Dex * This,
            __RPC__in_opt IUnknown *pTimeline,
            double dStart,
            double dEnd,
            __RPC__in BSTR FileName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *PasteXMLFile )( 
            IXml2Dex * This,
            __RPC__in_opt IUnknown *pTimeline,
            double dStart,
            __RPC__in BSTR FileName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CopyXML )( 
            IXml2Dex * This,
            __RPC__in_opt IUnknown *pTimeline,
            double dStart,
            double dEnd);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *PasteXML )( 
            IXml2Dex * This,
            __RPC__in_opt IUnknown *pTimeline,
            double dStart);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IXml2Dex * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ReadXML )( 
            IXml2Dex * This,
            __RPC__in_opt IUnknown *pTimeline,
            __RPC__in_opt IUnknown *pXML);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *WriteXML )( 
            IXml2Dex * This,
            __RPC__in_opt IUnknown *pTimeline,
            __RPC__deref_in_opt BSTR *pbstrXML);
        
        END_INTERFACE
    } IXml2DexVtbl;

    interface IXml2Dex
    {
        CONST_VTBL struct IXml2DexVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXml2Dex_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IXml2Dex_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IXml2Dex_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IXml2Dex_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IXml2Dex_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IXml2Dex_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IXml2Dex_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IXml2Dex_CreateGraphFromFile(This,ppGraph,pTimeline,Filename)	\
    ( (This)->lpVtbl -> CreateGraphFromFile(This,ppGraph,pTimeline,Filename) ) 

#define IXml2Dex_WriteGrfFile(This,pGraph,FileName)	\
    ( (This)->lpVtbl -> WriteGrfFile(This,pGraph,FileName) ) 

#define IXml2Dex_WriteXMLFile(This,pTimeline,FileName)	\
    ( (This)->lpVtbl -> WriteXMLFile(This,pTimeline,FileName) ) 

#define IXml2Dex_ReadXMLFile(This,pTimeline,XMLName)	\
    ( (This)->lpVtbl -> ReadXMLFile(This,pTimeline,XMLName) ) 

#define IXml2Dex_Delete(This,pTimeline,dStart,dEnd)	\
    ( (This)->lpVtbl -> Delete(This,pTimeline,dStart,dEnd) ) 

#define IXml2Dex_WriteXMLPart(This,pTimeline,dStart,dEnd,FileName)	\
    ( (This)->lpVtbl -> WriteXMLPart(This,pTimeline,dStart,dEnd,FileName) ) 

#define IXml2Dex_PasteXMLFile(This,pTimeline,dStart,FileName)	\
    ( (This)->lpVtbl -> PasteXMLFile(This,pTimeline,dStart,FileName) ) 

#define IXml2Dex_CopyXML(This,pTimeline,dStart,dEnd)	\
    ( (This)->lpVtbl -> CopyXML(This,pTimeline,dStart,dEnd) ) 

#define IXml2Dex_PasteXML(This,pTimeline,dStart)	\
    ( (This)->lpVtbl -> PasteXML(This,pTimeline,dStart) ) 

#define IXml2Dex_Reset(This)	\
    ( (This)->lpVtbl -> Reset(This) ) 

#define IXml2Dex_ReadXML(This,pTimeline,pXML)	\
    ( (This)->lpVtbl -> ReadXML(This,pTimeline,pXML) ) 

#define IXml2Dex_WriteXML(This,pTimeline,pbstrXML)	\
    ( (This)->lpVtbl -> WriteXML(This,pTimeline,pbstrXML) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IXml2Dex_INTERFACE_DEFINED__ */


#ifndef __IAMErrorLog_INTERFACE_DEFINED__
#define __IAMErrorLog_INTERFACE_DEFINED__

/* interface IAMErrorLog */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAMErrorLog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E43E73A2-0EFA-11d3-9601-00A0C9441E20")
    IAMErrorLog : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE LogError( 
            long Severity,
            __RPC__in BSTR pErrorString,
            long ErrorCode,
            long hresult,
            /* [in] */ __RPC__in VARIANT *pExtraInfo) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMErrorLogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMErrorLog * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMErrorLog * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMErrorLog * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *LogError )( 
            IAMErrorLog * This,
            long Severity,
            __RPC__in BSTR pErrorString,
            long ErrorCode,
            long hresult,
            /* [in] */ __RPC__in VARIANT *pExtraInfo);
        
        END_INTERFACE
    } IAMErrorLogVtbl;

    interface IAMErrorLog
    {
        CONST_VTBL struct IAMErrorLogVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMErrorLog_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAMErrorLog_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAMErrorLog_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAMErrorLog_LogError(This,Severity,pErrorString,ErrorCode,hresult,pExtraInfo)	\
    ( (This)->lpVtbl -> LogError(This,Severity,pErrorString,ErrorCode,hresult,pExtraInfo) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAMErrorLog_INTERFACE_DEFINED__ */


#ifndef __IAMSetErrorLog_INTERFACE_DEFINED__
#define __IAMSetErrorLog_INTERFACE_DEFINED__

/* interface IAMSetErrorLog */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAMSetErrorLog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("963566DA-BE21-4eaf-88E9-35704F8F52A1")
    IAMSetErrorLog : public IUnknown
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_ErrorLog( 
            /* [retval][out] */ __RPC__deref_out_opt IAMErrorLog **pVal) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_ErrorLog( 
            /* [in] */ __RPC__in_opt IAMErrorLog *newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMSetErrorLogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAMSetErrorLog * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAMSetErrorLog * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAMSetErrorLog * This);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ErrorLog )( 
            IAMSetErrorLog * This,
            /* [retval][out] */ __RPC__deref_out_opt IAMErrorLog **pVal);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ErrorLog )( 
            IAMSetErrorLog * This,
            /* [in] */ __RPC__in_opt IAMErrorLog *newVal);
        
        END_INTERFACE
    } IAMSetErrorLogVtbl;

    interface IAMSetErrorLog
    {
        CONST_VTBL struct IAMSetErrorLogVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMSetErrorLog_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAMSetErrorLog_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAMSetErrorLog_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAMSetErrorLog_get_ErrorLog(This,pVal)	\
    ( (This)->lpVtbl -> get_ErrorLog(This,pVal) ) 

#define IAMSetErrorLog_put_ErrorLog(This,newVal)	\
    ( (This)->lpVtbl -> put_ErrorLog(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAMSetErrorLog_INTERFACE_DEFINED__ */


#ifndef __ISampleGrabberCB_INTERFACE_DEFINED__
#define __ISampleGrabberCB_INTERFACE_DEFINED__

/* interface ISampleGrabberCB */
/* [unique][helpstring][local][uuid][object] */ 


EXTERN_C const IID IID_ISampleGrabberCB;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
    ISampleGrabberCB : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SampleCB( 
            double SampleTime,
            IMediaSample *pSample) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BufferCB( 
            double SampleTime,
            BYTE *pBuffer,
            long BufferLen) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISampleGrabberCBVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISampleGrabberCB * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISampleGrabberCB * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISampleGrabberCB * This);
        
        HRESULT ( STDMETHODCALLTYPE *SampleCB )( 
            ISampleGrabberCB * This,
            double SampleTime,
            IMediaSample *pSample);
        
        HRESULT ( STDMETHODCALLTYPE *BufferCB )( 
            ISampleGrabberCB * This,
            double SampleTime,
            BYTE *pBuffer,
            long BufferLen);
        
        END_INTERFACE
    } ISampleGrabberCBVtbl;

    interface ISampleGrabberCB
    {
        CONST_VTBL struct ISampleGrabberCBVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISampleGrabberCB_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISampleGrabberCB_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISampleGrabberCB_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISampleGrabberCB_SampleCB(This,SampleTime,pSample)	\
    ( (This)->lpVtbl -> SampleCB(This,SampleTime,pSample) ) 

#define ISampleGrabberCB_BufferCB(This,SampleTime,pBuffer,BufferLen)	\
    ( (This)->lpVtbl -> BufferCB(This,SampleTime,pBuffer,BufferLen) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISampleGrabberCB_INTERFACE_DEFINED__ */


#ifndef __ISampleGrabber_INTERFACE_DEFINED__
#define __ISampleGrabber_INTERFACE_DEFINED__

/* interface ISampleGrabber */
/* [unique][helpstring][local][uuid][object] */ 


EXTERN_C const IID IID_ISampleGrabber;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
    ISampleGrabber : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetOneShot( 
            BOOL OneShot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMediaType( 
            const AM_MEDIA_TYPE *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType( 
            AM_MEDIA_TYPE *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBufferSamples( 
            BOOL BufferThem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer( 
            /* [out][in] */ long *pBufferSize,
            /* [out] */ long *pBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentSample( 
            /* [retval][out] */ IMediaSample **ppSample) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCallback( 
            ISampleGrabberCB *pCallback,
            long WhichMethodToCallback) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISampleGrabberVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISampleGrabber * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISampleGrabber * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISampleGrabber * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetOneShot )( 
            ISampleGrabber * This,
            BOOL OneShot);
        
        HRESULT ( STDMETHODCALLTYPE *SetMediaType )( 
            ISampleGrabber * This,
            const AM_MEDIA_TYPE *pType);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnectedMediaType )( 
            ISampleGrabber * This,
            AM_MEDIA_TYPE *pType);
        
        HRESULT ( STDMETHODCALLTYPE *SetBufferSamples )( 
            ISampleGrabber * This,
            BOOL BufferThem);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentBuffer )( 
            ISampleGrabber * This,
            /* [out][in] */ long *pBufferSize,
            /* [out] */ long *pBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentSample )( 
            ISampleGrabber * This,
            /* [retval][out] */ IMediaSample **ppSample);
        
        HRESULT ( STDMETHODCALLTYPE *SetCallback )( 
            ISampleGrabber * This,
            ISampleGrabberCB *pCallback,
            long WhichMethodToCallback);
        
        END_INTERFACE
    } ISampleGrabberVtbl;

    interface ISampleGrabber
    {
        CONST_VTBL struct ISampleGrabberVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISampleGrabber_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISampleGrabber_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISampleGrabber_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISampleGrabber_SetOneShot(This,OneShot)	\
    ( (This)->lpVtbl -> SetOneShot(This,OneShot) ) 

#define ISampleGrabber_SetMediaType(This,pType)	\
    ( (This)->lpVtbl -> SetMediaType(This,pType) ) 

#define ISampleGrabber_GetConnectedMediaType(This,pType)	\
    ( (This)->lpVtbl -> GetConnectedMediaType(This,pType) ) 

#define ISampleGrabber_SetBufferSamples(This,BufferThem)	\
    ( (This)->lpVtbl -> SetBufferSamples(This,BufferThem) ) 

#define ISampleGrabber_GetCurrentBuffer(This,pBufferSize,pBuffer)	\
    ( (This)->lpVtbl -> GetCurrentBuffer(This,pBufferSize,pBuffer) ) 

#define ISampleGrabber_GetCurrentSample(This,ppSample)	\
    ( (This)->lpVtbl -> GetCurrentSample(This,ppSample) ) 

#define ISampleGrabber_SetCallback(This,pCallback,WhichMethodToCallback)	\
    ( (This)->lpVtbl -> SetCallback(This,pCallback,WhichMethodToCallback) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISampleGrabber_INTERFACE_DEFINED__ */



#ifndef __DexterLib_LIBRARY_DEFINED__
#define __DexterLib_LIBRARY_DEFINED__

/* library DexterLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_DexterLib;

#ifndef __IResize_INTERFACE_DEFINED__
#define __IResize_INTERFACE_DEFINED__

/* interface IResize */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IResize;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4ada63a0-72d5-11d2-952a-0060081840bc")
    IResize : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE get_Size( 
            /* [out] */ __RPC__out int *piHeight,
            /* [out] */ __RPC__out int *piWidth,
            /* [out] */ __RPC__out long *pFlag) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_InputSize( 
            /* [out] */ __RPC__out int *piHeight,
            /* [out] */ __RPC__out int *piWidth) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_Size( 
            /* [in] */ int Height,
            /* [in] */ int Width,
            /* [in] */ long Flag) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_MediaType( 
            /* [out] */ __RPC__out AM_MEDIA_TYPE *pmt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_MediaType( 
            /* [in] */ __RPC__in const AM_MEDIA_TYPE *pmt) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IResizeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IResize * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IResize * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IResize * This);
        
        HRESULT ( STDMETHODCALLTYPE *get_Size )( 
            IResize * This,
            /* [out] */ __RPC__out int *piHeight,
            /* [out] */ __RPC__out int *piWidth,
            /* [out] */ __RPC__out long *pFlag);
        
        HRESULT ( STDMETHODCALLTYPE *get_InputSize )( 
            IResize * This,
            /* [out] */ __RPC__out int *piHeight,
            /* [out] */ __RPC__out int *piWidth);
        
        HRESULT ( STDMETHODCALLTYPE *put_Size )( 
            IResize * This,
            /* [in] */ int Height,
            /* [in] */ int Width,
            /* [in] */ long Flag);
        
        HRESULT ( STDMETHODCALLTYPE *get_MediaType )( 
            IResize * This,
            /* [out] */ __RPC__out AM_MEDIA_TYPE *pmt);
        
        HRESULT ( STDMETHODCALLTYPE *put_MediaType )( 
            IResize * This,
            /* [in] */ __RPC__in const AM_MEDIA_TYPE *pmt);
        
        END_INTERFACE
    } IResizeVtbl;

    interface IResize
    {
        CONST_VTBL struct IResizeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IResize_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IResize_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IResize_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IResize_get_Size(This,piHeight,piWidth,pFlag)	\
    ( (This)->lpVtbl -> get_Size(This,piHeight,piWidth,pFlag) ) 

#define IResize_get_InputSize(This,piHeight,piWidth)	\
    ( (This)->lpVtbl -> get_InputSize(This,piHeight,piWidth) ) 

#define IResize_put_Size(This,Height,Width,Flag)	\
    ( (This)->lpVtbl -> put_Size(This,Height,Width,Flag) ) 

#define IResize_get_MediaType(This,pmt)	\
    ( (This)->lpVtbl -> get_MediaType(This,pmt) ) 

#define IResize_put_MediaType(This,pmt)	\
    ( (This)->lpVtbl -> put_MediaType(This,pmt) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IResize_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_AMTimeline;

#ifdef __cplusplus

class DECLSPEC_UUID("78530B75-61F9-11D2-8CAD-00A024580902")
AMTimeline;
#endif

EXTERN_C const CLSID CLSID_AMTimelineObj;

#ifdef __cplusplus

class DECLSPEC_UUID("78530B78-61F9-11D2-8CAD-00A024580902")
AMTimelineObj;
#endif

EXTERN_C const CLSID CLSID_AMTimelineSrc;

#ifdef __cplusplus

class DECLSPEC_UUID("78530B7A-61F9-11D2-8CAD-00A024580902")
AMTimelineSrc;
#endif

EXTERN_C const CLSID CLSID_AMTimelineTrack;

#ifdef __cplusplus

class DECLSPEC_UUID("8F6C3C50-897B-11d2-8CFB-00A0C9441E20")
AMTimelineTrack;
#endif

EXTERN_C const CLSID CLSID_AMTimelineComp;

#ifdef __cplusplus

class DECLSPEC_UUID("74D2EC80-6233-11d2-8CAD-00A024580902")
AMTimelineComp;
#endif

EXTERN_C const CLSID CLSID_AMTimelineGroup;

#ifdef __cplusplus

class DECLSPEC_UUID("F6D371E1-B8A6-11d2-8023-00C0DF10D434")
AMTimelineGroup;
#endif

EXTERN_C const CLSID CLSID_AMTimelineTrans;

#ifdef __cplusplus

class DECLSPEC_UUID("74D2EC81-6233-11d2-8CAD-00A024580902")
AMTimelineTrans;
#endif

EXTERN_C const CLSID CLSID_AMTimelineEffect;

#ifdef __cplusplus

class DECLSPEC_UUID("74D2EC82-6233-11d2-8CAD-00A024580902")
AMTimelineEffect;
#endif

EXTERN_C const CLSID CLSID_RenderEngine;

#ifdef __cplusplus

class DECLSPEC_UUID("64D8A8E0-80A2-11d2-8CF3-00A0C9441E20")
RenderEngine;
#endif

EXTERN_C const CLSID CLSID_SmartRenderEngine;

#ifdef __cplusplus

class DECLSPEC_UUID("498B0949-BBE9-4072-98BE-6CCAEB79DC6F")
SmartRenderEngine;
#endif

EXTERN_C const CLSID CLSID_AudMixer;

#ifdef __cplusplus

class DECLSPEC_UUID("036A9790-C153-11d2-9EF7-006008039E37")
AudMixer;
#endif

EXTERN_C const CLSID CLSID_Xml2Dex;

#ifdef __cplusplus

class DECLSPEC_UUID("18C628EE-962A-11D2-8D08-00A0C9441E20")
Xml2Dex;
#endif

EXTERN_C const CLSID CLSID_MediaLocator;

#ifdef __cplusplus

class DECLSPEC_UUID("CC1101F2-79DC-11D2-8CE6-00A0C9441E20")
MediaLocator;
#endif

EXTERN_C const CLSID CLSID_PropertySetter;

#ifdef __cplusplus

class DECLSPEC_UUID("ADF95821-DED7-11d2-ACBE-0080C75E246E")
PropertySetter;
#endif

EXTERN_C const CLSID CLSID_MediaDet;

#ifdef __cplusplus

class DECLSPEC_UUID("65BD0711-24D2-4ff7-9324-ED2E5D3ABAFA")
MediaDet;
#endif

EXTERN_C const CLSID CLSID_SampleGrabber;

#ifdef __cplusplus

class DECLSPEC_UUID("C1F400A0-3F08-11d3-9F0B-006008039E37")
SampleGrabber;
#endif

EXTERN_C const CLSID CLSID_NullRenderer;

#ifdef __cplusplus

class DECLSPEC_UUID("C1F400A4-3F08-11d3-9F0B-006008039E37")
NullRenderer;
#endif

EXTERN_C const CLSID CLSID_DxtCompositor;

#ifdef __cplusplus

class DECLSPEC_UUID("BB44391D-6ABD-422f-9E2E-385C9DFF51FC")
DxtCompositor;
#endif

EXTERN_C const CLSID CLSID_DxtAlphaSetter;

#ifdef __cplusplus

class DECLSPEC_UUID("506D89AE-909A-44f7-9444-ABD575896E35")
DxtAlphaSetter;
#endif

EXTERN_C const CLSID CLSID_DxtJpeg;

#ifdef __cplusplus

class DECLSPEC_UUID("DE75D012-7A65-11D2-8CEA-00A0C9441E20")
DxtJpeg;
#endif

EXTERN_C const CLSID CLSID_ColorSource;

#ifdef __cplusplus

class DECLSPEC_UUID("0cfdd070-581a-11d2-9ee6-006008039e37")
ColorSource;
#endif

EXTERN_C const CLSID CLSID_DxtKey;

#ifdef __cplusplus

class DECLSPEC_UUID("C5B19592-145E-11d3-9F04-006008039E37")
DxtKey;
#endif
#endif /* __DexterLib_LIBRARY_DEFINED__ */

/* interface __MIDL_itf_qedit_0001_0097 */
/* [local] */ 


enum __MIDL___MIDL_itf_qedit_0001_0097_0001
    {	E_NOTINTREE	= 0x80040400,
	E_RENDER_ENGINE_IS_BROKEN	= 0x80040401,
	E_MUST_INIT_RENDERER	= 0x80040402,
	E_NOTDETERMINED	= 0x80040403,
	E_NO_TIMELINE	= 0x80040404,
	S_WARN_OUTPUTRESET	= 40404
    } ;
#define DEX_IDS_BAD_SOURCE_NAME    1400
#define DEX_IDS_BAD_SOURCE_NAME2    1401
#define DEX_IDS_MISSING_SOURCE_NAME    1402
#define DEX_IDS_UNKNOWN_SOURCE    1403
#define DEX_IDS_INSTALL_PROBLEM    1404
#define DEX_IDS_NO_SOURCE_NAMES    1405
#define DEX_IDS_BAD_MEDIATYPE    1406
#define DEX_IDS_STREAM_NUMBER    1407
#define DEX_IDS_OUTOFMEMORY        1408
#define DEX_IDS_DIBSEQ_NOTALLSAME    1409
#define DEX_IDS_CLIPTOOSHORT        1410
#define DEX_IDS_INVALID_DXT        1411
#define DEX_IDS_INVALID_DEFAULT_DXT    1412
#define DEX_IDS_NO_3D        1413
#define DEX_IDS_BROKEN_DXT        1414
#define DEX_IDS_NO_SUCH_PROPERTY    1415
#define DEX_IDS_ILLEGAL_PROPERTY_VAL    1416
#define DEX_IDS_INVALID_XML        1417
#define DEX_IDS_CANT_FIND_FILTER    1418
#define DEX_IDS_DISK_WRITE_ERROR    1419
#define DEX_IDS_INVALID_AUDIO_FX    1420
#define DEX_IDS_CANT_FIND_COMPRESSOR 1421
#define DEX_IDS_TIMELINE_PARSE    1426
#define DEX_IDS_GRAPH_ERROR        1427
#define DEX_IDS_GRID_ERROR        1428
#define DEX_IDS_INTERFACE_ERROR    1429
EXTERN_GUID(CLSID_VideoEffects1Category, 0xcc7bfb42, 0xf175, 0x11d1, 0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59);
EXTERN_GUID(CLSID_VideoEffects2Category, 0xcc7bfb43, 0xf175, 0x11d1, 0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59);
EXTERN_GUID(CLSID_AudioEffects1Category, 0xcc7bfb44, 0xf175, 0x11d1, 0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59);
EXTERN_GUID(CLSID_AudioEffects2Category, 0xcc7bfb45, 0xf175, 0x11d1, 0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59);


extern RPC_IF_HANDLE __MIDL_itf_qedit_0001_0097_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_qedit_0001_0097_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

unsigned long             __RPC_USER  BSTR_UserSize64(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal64(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal64(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree64(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize64(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal64(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal64(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree64(     unsigned long *, VARIANT * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



