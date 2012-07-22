/****************************************************************************
 * 
 *  Copyright (C) 1995-1999 RealNetworks, Inc. All rights reserved.
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary 
 *  information of Progressive Networks, Inc, and is licensed
 *  subject to restrictions on use and distribution.
 *
 *
 *  Persistent Preferences Interfaces
 *
 *  Here are the preference entries set by the client core and renderers:
 *	    KEY				DEFAULT VALUES
 *	=================	    ====================
 *	AttemptMulticast		    1
 *	AttemptTCP			    1
 *	AttemptUDP			    1
 *	AudioQuality			    0
 *	AutoTransport			    1
 *	Bandwidth			    28800
 *	BitsPerSample			    16
 *	BroadcastPluginInfo		    {dllpath;description;copyright;moreinfo;loadmultiple;type}{ ... }
 *	ClientLicenseKey		    7FF7FF00
 *	EndScan				    10000
 *	FactoryPluginInfo		    
 *	FileFormatPluginInfo		    {dllpath;description;copyright;moreinfo;loadmultiple;mimetype1|mimetype2;extension1|extension2}{ ... }
 *	FileSystemPluginInfo		    {dllpath;description;copyright;moreinfo;loadmultiple;protocol;shortname}{ ... }
 *	GeneralPluginInfo		    {dllpath;description;copyright;moreinfo;loadmultiple}{ ... }
 *	PNAProxyHost
 *	PNAProxyPort			    1090
 *	RTSPProxyHost
 *	RTSPProxyPort			    554
 *	HTTPProxyHost
 *	HTTPProxyPort			    1092
 *	HurledURL			    0
 *	InfoandVolume			    1
 *	LastURL				    
 *	MaxClipCount			    4
 *	MetaFormatPluginInfo		    {dllpath;description;copyright;moreinfo;loadmultiple;mimetype1|mimetype2;extension1|extension2}{ ... }
 *	MiscPluginInfo			    {dllpath;description;copyright;moreinfo;loadmultiple}{ ... }
 *	MulticastTimeout		    2000
 *	NotProxy	    
 *	OnTop				    0
 *	PerfectPlayMode			    0
 *	PerfectPlayTime			    60
 *	PerfPlayEntireClip		    1
 *	PluginDirectory
 *	Presets#
 *	ProxySupport			    0
 *	RendererPluginInfo		    {dllpath;description;copyright;moreinfo;loadmultiple;mimetype1|mimetype2}{ ... }
 *	SamplingRate			    8000
 *	SeekPage			    40
 *	SendStatistics			    1
 *	ServerTimeOut			    90
 *	ShowPresets			    0
 *	StatusBar			    1
 *	StreamDescriptionPluginInfo	    {dllpath;description;copyright;moreinfo;loadmultiple;mimetype}{ ... }
 *	SyncMultimedia			    1
 *	UDPPort				    7070
 *	UDPTimeout			    10000
 *	UpgradeAvailable		    0
 *	UseUDPPort			    0
 *	Volume				    50
 *	x:Pref_windowPositionX
 *	y:Pref_WindowPositionY
 */

#ifndef _RMAPREFS_H_
#define _RMAPREFS_H_

#define RMAPNREGISTRY_PREFPROPNAME	    "ApplicationData"
/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE	IRMABuffer			IRMABuffer;

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAPreferences
 * 
 *  Purpose:
 * 
 *	This interface allows you to store persistant preferences in the
 *	server or player's config / registry.
 * 
 *  IID_IRMAPreferences:
 * 
 *	{00000500-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPreferences, 0x00000500, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPreferences

DECLARE_INTERFACE_(IRMAPreferences, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAPreferences methods
     */

    /************************************************************************
     *	Method:
     *		IRMAPreferences::ReadPref
     *	Purpose:
     *		Read a preference from the registry or configuration.
     */
    STDMETHOD(ReadPref)			(THIS_
					const char* pPrekKey, REF(IRMABuffer*) pBuffer) PURE;

    /************************************************************************
     *	Method:
     *		IRMAPreferences::WritePref
     *	Purpose:
     *		TBD
     */
    STDMETHOD(WritePref)		(THIS_
					const char* pPrekKey, IRMABuffer* pBuffer) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAPreferenceEnumerator
 * 
 *  Purpose:
 * 
 *	Allows preference Enumeration
 *	
 * 
 *  IRMAPreferenceEnumerator:
 * 
 *	{00000504-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPreferenceEnumerator, 0x00000504, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPreferenceEnumerator

DECLARE_INTERFACE_(IRMAPreferenceEnumerator, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAPreferenceEnumerator methods
     */

    /************************************************************************
     *	Method:
     *		IRMAPreferenceEnumerator::EndSubPref
     *	Purpose:
     *		TBD
     */

    STDMETHOD(BeginSubPref) (THIS_ const char* szSubPref) PURE;


    /************************************************************************
     *	Method:
     *		IRMAPreferenceEnumerator::EndSubPref
     *	Purpose:
     *		TBD
     */

   STDMETHOD(EndSubPref) (THIS) PURE;

    /************************************************************************
     *	Method:
     *		IRMAPreferenceEnumerator::GetPrefKey
     *	Purpose:
     *		TBD
     */

   STDMETHOD(GetPrefKey) (THIS_ UINT32 nIndex, REF(IRMABuffer*) pBuffer) PURE;

    /************************************************************************
     *	Method:
     *		IRMAPreferenceEnumerator::ReadPref
     *	Purpose:
     *		TBD
     */
    STDMETHOD(ReadPref)			(THIS_
					    const char* pPrefKey, IRMABuffer*& pBuffer) PURE;

};



/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAPreferences2
 * 
 *  Purpose:
 * 
 *	New interface which gives sub-preference options abilities.
 *	
 * 
 *  IID_IRMAPreferences2:
 * 
 *	{00000503-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPreferences2, 0x00000503, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPreferences2

DECLARE_INTERFACE_(IRMAPreferences2, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAPreferences2 methods
     */

    /************************************************************************
     *	Method:
     *		IRMAPreferences2::GetPreferenceEnumerator
     *	Purpose:
     *		Read a preference from the registry or configuration.
     */

    STDMETHOD(GetPreferenceEnumerator)(THIS_ REF(IRMAPreferenceEnumerator*) /*OUT*/ pEnum) PURE;

    /************************************************************************
     *	Method:
     *		IRMAPreferences2::ResetRoot
     *	Purpose:
     *		Reset the root of the preferences
     */

    STDMETHOD(ResetRoot)(THIS_ const char* pCompanyName, const char* pProductName, 
	int nProdMajorVer, int nProdMinorVer) PURE;
};



#endif /* _RMAPREFS_H_ */
