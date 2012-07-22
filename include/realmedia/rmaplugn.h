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
 *  Plugin init / inspector interfaces
 *
 */

#ifndef _RMAPLUGN_H_
#define _RMAPLUGN_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE  IUnknown			    IUnknown;
typedef _INTERFACE  IRMAPlugin			    IRMAPlugin;
typedef _INTERFACE  IRMAPluginEnumerator	    IRMAPluginEnumerator;
typedef _INTERFACE  IRMAPluginChallenger	    IRMAPluginChallenger;
typedef _INTERFACE  IRMABuffer			    IRMABuffer;
typedef _INTERFACE  IRMAValues			    IRMAValues;

/****************************************************************************
 * 
 *  Function:
 * 
 *	RMACreateInstance()
 * 
 *  Purpose:
 * 
 *	Function implemented by all plugin DLL's to create an instance of 
 *	any of the objects supported by the DLL. This method is similar to 
 *	Window's CoCreateInstance() in its purpose, except that it only 
 *	creates objects from this plugin DLL.
 *
 *	NOTE: Aggregation is never used. Therefore an outer unknown is
 *	not passed to this function, and you do not need to code for this
 *	situation.
 * 
 */
#ifdef _MACINTOSH
#pragma export on
#endif

STDAPI RMACreateInstance
		(
		    IUnknown**  /*OUT*/	ppIUnknown
		);
		
#ifdef _MACINTOSH
#pragma export off
#endif


/****************************************************************************
 * 
 *  Function:
 * 
 *	RMAShutdown()
 * 
 *  Purpose:
 * 
 *	Function implemented by all plugin DLL's to free any *global* 
 *	resources. This method is called just before the DLL is unloaded.
 *
 */
#ifdef _MACINTOSH
#pragma export on
#endif

STDAPI RMAShutdown(void);
		
#ifdef _MACINTOSH
#pragma export off
#endif


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAPlugin
 * 
 *  Purpose:
 * 
 *	Interface exposed by a plugin DLL to allow inspection of objects
 *	supported by the plugin DLL.
 * 
 *  IID_IRMAPlugin:
 * 
 *	{00000C00-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPlugin, 0x00000C00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPlugin

DECLARE_INTERFACE_(IRMAPlugin, IUnknown)
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
     *	IRMAPlugin methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAPlugin::GetPluginInfo
     *	Purpose:
     *	    Returns the basic information about this plugin. Including:
     *
     *	    bMultipleLoad	Whether or not this plugin can be instantiated
     *				multiple times. All File Formats must set
     *				this value to TRUE.  The only other type of
     *				plugin that can specify bMultipleLoad=TRUE is
     *				a filesystem plugin.  Any plugin that sets
     *				this flag to TRUE must not use global variables
     *				of any type.
     *
     *				Setting this flag to TRUE implies that you
     *				accept that your plugin may be instantiated
     *				multiple times (possibly in different
     *				address spaces).  Plugins are instantiated
     *				multiple times only in the server (for
     *				performance reasons).
     *
     *				An example of a plugin, that must set this
     *				flag to FALSE is a filesystem plugin that 
     *				uses a single TCP connection to communicate
     *				with a database.
     *				
     *	    pDescription	which is used in about UIs (can be NULL)
     *	    pCopyright		which is used in about UIs (can be NULL)
     *	    pMoreInfoURL	which is used in about UIs (can be NULL)
     *	    ulVersionNumber	The version of this plugin.
     */
    STDMETHOD(GetPluginInfo)	(THIS_
				REF(BOOL)	 /*OUT*/ bMultipleLoad,
				REF(const char*) /*OUT*/ pDescription,
				REF(const char*) /*OUT*/ pCopyright,
				REF(const char*) /*OUT*/ pMoreInfoURL,
				REF(ULONG32)	 /*OUT*/ ulVersionNumber) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPlugin::InitPlugin
     *	Purpose:
     *	    Initializes the plugin for use. This interface must always be
     *	    called before any other method is called. This is primarily needed 
     *	    so that the plugin can have access to the context for creation of
     *	    IRMABuffers and IMalloc.
     */
    STDMETHOD(InitPlugin)   (THIS_
			    IUnknown*   /*IN*/  pContext) PURE;

};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAPluginEnumerator
 * 
 *  Purpose:
 * 
 *	provide methods to enumerate through all the plugins installed
 * 
 *  IID_IRMAPluginEnumerator:
 * 
 *	{00000C01-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPluginEnumerator, 0x00000C01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
		0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPluginEnumerator

DECLARE_INTERFACE_(IRMAPluginEnumerator, IUnknown)
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
     *	IRMAPluginEnumerator methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAPluginEnumerator::GetNumOfPlugins
     *
     *	Purpose:    
     *	    return the number of plugins available
     *
     */
    STDMETHOD_(ULONG32,GetNumOfPlugins)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPluginEnumerator::GetPlugin
     *	Purpose:
     *	    Return an instance (IUnknown) of the plugin
     *
     */
    STDMETHOD(GetPlugin)   (THIS_
			   ULONG32	   /*IN*/  ulIndex,
			   REF(IUnknown*)  /*OUT*/ pPlugin) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAPluginGroupEnumerator
 * 
 *  Purpose:
 * 
 *	Provide a way to enumerate through all of the plugins which
 *	implement a specific interface.
 * 
 *  IID_IRMAPluginGroupEnumerator:
 * 
 *	{00000C02-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPluginGroupEnumerator, 0x00000C02, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
		0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPluginGroupEnumerator

#define CLSID_IRMAPluginGroupEnumerator IID_IRMAPluginGroupEnumerator

DECLARE_INTERFACE_(IRMAPluginGroupEnumerator, IUnknown)
{
    /*
     * IUnknown methods
     */

    /*
     * IRMAPluginGroupEnumerator methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /******************************************************************
     * Method:
     *	    IRMAPluginGroupEnumerator::Init
     *
     * Purpose:
     *	    tell the group enumerator which interface to group the plugins
     *     into, this method must be called before the other methods can
     *     be called.
     *
     */
    STDMETHOD(Init)         (THIS_
                            REFIID    iid) PURE;


    /******************************************************************
     * Method:
     *     IRMAPluginGroupEnumerator::GetNumOfPlugins
     *
     * Purpose:
     *     return the number of plugins available that support a
particular
     *     interface.
     *
     */
    STDMETHOD_(ULONG32,GetNumOfPlugins) (THIS) PURE;


    /******************************************************************
     * Method:
     *     IRMAPluginGroupEnumerator::GetPlugin
     * Purpose:
     *     Return an instance (IUnknown) of the plugin
     *
     */
    STDMETHOD(GetPlugin)   (THIS_
      UINT32    /*IN*/  ulIndex,
      REF(IUnknown*)  /*OUT*/ pPlugin) PURE;

};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAPluginReloader
 * 
 *  Purpose:
 * 
 *	Tells the client core to reload all plugins.
 * 
 *  IID_IRMAPluginReloader:
 * 
 *	{00000C03-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPluginReloader, 0x00000C03, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
		0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPluginReloader

DECLARE_INTERFACE_(IRMAPluginReloader, IUnknown)
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
     *	IRMAPluginReloader methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAPluginReloader::ReloadPlugins
     *	Purpose:    
     *	    Causes the client core to reload all plugins.
     *
     */
    STDMETHOD(ReloadPlugins)	(THIS) PURE;
};



/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAPluginFactory
 * 
 *  Purpose:
 * 
 *	This interface is implemented by a plugin in order to have more then
 *	    one "RMA plugin" in a single DLL.  I.e., a plugin author could
 *	    use this interface to have 3 different file format plugins in
 *	    a single DLL.
 * 
 *  IID_IRMAPluginFactory:
 * 
 *	{00000C04-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPluginFactory, 0x00000C04, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
		0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPluginFactory

DECLARE_INTERFACE_(IRMAPluginFactory, IUnknown)
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
     *	IRMAPluginFactory methods
     */
    
    /*****************************************************************
     *	Method:
     *	    IRMAPluginFactory::GetNumPlugins
     *	Purpose:
     *	    Report the number of Plugins within the DLL.
     *
     *	Parameters:
     */
    STDMETHOD_(UINT16, GetNumPlugins) (THIS)  PURE;

    /*****************************************************************
     *	Method:
     *	    IRMAPluginFactory::GetPlugin
     *	Purpose:
     *	    Returns an IUnknown interface to the requested plugin. 
     *	
     *	Parameters:
     */

    STDMETHOD(GetPlugin) (THIS_
			 UINT16 	uIndex, 
			 IUnknown**  	pPlugin) PURE;
};



/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAGenericPlugin
 * 
 *  Purpose:
 * 
 *	Interface exposed by a plugin DLL to inform the client / server core
 *	that your plugin wishes to have InitPlugin called immediately.
 *
 *  IID_IRMAGenericPlugin:
 * 
 *	{00000C09-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAGenericPlugin, 0x00000C09, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAGenericPlugin

DECLARE_INTERFACE_(IRMAGenericPlugin, IUnknown)
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
     *	IRMAGenericPlugin methods
     */

    STDMETHOD(IsGeneric)	(THIS_
				REF(BOOL)	 /*OUT*/ bIsGeneric) PURE;
};


DEFINE_GUID(IID_IRMAPluginHandler,	0x00000200, 0xb4c8, 0x11d0, 0x99, 0x95, 0x0, 0xa0, 0x24, 0x8d, 0xa5, 0xf0);

DEFINE_GUID(IID_IRMAPlugin2Handler,	0x00000201, 0xb4c8, 0x11d0, 0x99, 0x95, 0x0, 0xa0, 0x24, 0x8d, 0xa5, 0xf0);

#undef  INTERFACE
#define INTERFACE   IRMAPlugin2Handler

DECLARE_INTERFACE_(IRMAPlugin2Handler, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMAPlugin2Handler Methods
     */

     /************************************************************************
     *	Method:
     *	    IRMAPlugin2Handler::Init
     *
     *	Purpose:    
     *	    Specifies the context and sets the pluginhandler in motion.
     *
     */
    STDMETHOD(Init)    (THIS_ IUnknown* pContext) PURE;
     
     /************************************************************************
     *	Method:
     *	    IRMAPlugin2Handler::GetNumPlugins2
     *
     *	Purpose:    
     *	    Gets the info of a particular plugin.
     *
     */
    STDMETHOD_(ULONG32,GetNumOfPlugins2)    (THIS) PURE;
    
    /************************************************************************
     *	Method:
     *	    IRMAPlugin2Handler::GetPluginInfo
     *
     *	Purpose:    
     *	    Gets the info of a particular plugin.
     *
     */
    STDMETHOD(GetPluginInfo)	(THIS_ 
				UINT32 unIndex, 
				REF(IRMAValues*) /*OUT*/ Values) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPlugin2Handler::FlushCache()
     *
     *	Purpose:    
     *	    Flushes the LRU cache -- Unloads all DLLs from memory 
     *	    which currenltly have a refcount of 0.
     */

    STDMETHOD(FlushCache)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPlugin2Handler::SetCacheSize
     *
     *	Purpose:    
     *	    This function sets the size of the Cache. The cache is 
     *	    initally set to 1000KB. To disable the cache simply set
     *	    the size to 0.If the cache is disabled a DLL will be 
     *	    unloaded whenever it's refcount becomes zero. Which MAY
     *	    cause performance problems.
     */

    STDMETHOD(SetCacheSize)	(THIS_ ULONG32 nSizeKB) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPlugin2Handler::GetInstance
     *
     *	Purpose:    
     *	    
     *	    This function will return a plugin instance given a plugin index.
     *		
     */

    STDMETHOD(GetInstance) (THIS_ UINT32 index, REF(IUnknown*) pUnknown) PURE; 

    /************************************************************************
     *	Method:
     *	    IRMAPlugin2Handler::FindIndexUsingValues
     *
     *	Purpose:    
     *	    Finds a plugin  which matches the set of values given. An index
     *	    is returned which can be used to either get the values (using 
     *	    GetPluginInfo) or an instance can be created using GetPluing(). 	
     * 
     */

    STDMETHOD(FindIndexUsingValues)	    (THIS_ IRMAValues*, 
						    REF(UINT32) unIndex) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPlugin2Handler::FindPluginUsingValues
     *
     *	Purpose:    
     *	    Finds a plugin  which matches the set of values given. A Plugin
     *	    instance is returned. 
     *	    
     */

    STDMETHOD(FindPluginUsingValues)	    (THIS_ IRMAValues*, 
						    REF(IUnknown*) pUnk) PURE;
    
    /************************************************************************
     *	Method:
     *	    IRMAPlugin2Handler::FindIndexUsingStrings
     *
     *	Purpose:    
     *	    Finds a plugin  which matches the set of values given. An index
     *	    is returned which can be used to either get the values (using 
     *	    GetPluginInfo) or an instance can be created using GetPluing(). 	
     *	    NOTE: that a max of two values may be given.
     */

    STDMETHOD(FindIndexUsingStrings)	    (THIS_ char* PropName1, 
						    char* PropVal1, 
						    char* PropName2, 
						    char* PropVal2, 
						    char* PropName3, 
						    char* PropVal3, 
						    REF(UINT32) unIndex) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPlugin2Handler::FindPluginUsingStrings
     *
     *	Purpose:    
     *	    Finds a plugin  which matches the set of values given. A Plugin
     *	    instance is returned. 
     *	    NOTE: that a max of two values may be given.
     */

    STDMETHOD(FindPluginUsingStrings)	    (THIS_ char* PropName1, 
						    char* PropVal1, 
						    char* PropName2, 
						    char* PropVal2, 
						    char* PropName3, 
						    char* PropVal3, 
						    REF(IUnknown*) pUnk) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPlugin2Handler::FindImplementationFromClassID
     *
     *	Purpose:    
     *	    Finds a CommonClassFactory plugin which supports the 
     *	    ClassID given. An instance of the Class is returned. 
     */

    STDMETHOD(FindImplementationFromClassID)
    (
	THIS_ 
	REFGUID GUIDClassID, 
	REF(IUnknown*) pIUnknownInstance
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPlugin2Handler::Close
     *
     *	Purpose:    
     *	    A function which performs all of the functions of delete.
     *	    
     *
     */
    
    STDMETHOD(Close)		(THIS) PURE; 

    /************************************************************************
     *	Method:
     *	    IRMAPlugin2Handler::SetRequiredPlugins
     *
     *	Purpose:    
     *	    This function sets the required plugin list
     *	    
     *
     */

    STDMETHOD(SetRequiredPlugins) (THIS_ const char** ppszRequiredPlugins) PURE;


};









#endif /* _RMAPLUGN_H_ */
