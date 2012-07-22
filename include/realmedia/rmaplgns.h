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
 *  RealMedia Architecture Plug-in Interfaces.
 *
 */

#ifndef _RMAPLGNS_H_
#define _RMAPLGNS_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE  IUnknown			    IUnknown;
typedef _INTERFACE  IRMAObjectConfiguration	    IRMAObjectConfiguration;
typedef _INTERFACE  IRMAPluginProperties	    IRMAPluginProperties;
typedef _INTERFACE  IRMABuffer			    IRMABuffer;
typedef _INTERFACE  IRMAValues			    IRMAValues;

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAObjectConfiguration
 *
 *  Purpose:
 *
 *	Interface for setting context and generic means of plugin 
 *	Configuration.
 *
 *  IRMAObjectConfiguration:
 *
 *	{0x00002900-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAObjectConfiguration,   0x00002900, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAObjectConfiguration

DECLARE_INTERFACE_(IRMAObjectConfiguration, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAObjectConfiguration::SetContext
     *	Purpose:
     *	    This function is called to set the context for the plugin.
     *	    Either IRMAPlugin::InitPlugin or this function must be called 
     *	    before calling any other function on the plugin.
     *	    this is intended to be used as a shortcut for the plugin user.
     *	    If one needs to use SetConfiguration they only need to query
     *	    IRMAObjectConfiguration saving them from also querying for 
     *	    IRMAPlugin.
     *
     */
    STDMETHOD(SetContext)
    (
	THIS_
	IUnknown*   pIUnknownContext
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAObjectConfiguration::SetConfiguration
     *	Purpose:
     *	    This allows the user of a plugin to supply configuration
     *	    information.  This is often a set of CString properties
     *	    extracted from a list in the config file.  This allows
     *	    each plugin within a class (auth plugin, database plugin, etc..) 
     *	    to require a different set of parameters.
     *
     */
    STDMETHOD(SetConfiguration)
    (
	THIS_
	IRMAValues* pIRMAValuesConfiguration
    ) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAPluginProperties
 *
 *  Purpose:
 *
 *	This allows plugins to return whatever properties they want.
 *
 *  IRMAPluginProperties:
 *
 *	{0x00002901-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAPluginProperties,   0x00002901, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPluginProperties

DECLARE_INTERFACE_(IRMAPluginProperties, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPluginProperties::GetProperties
     *	Purpose:
     *	    A plugin will implement this in order to return plugin properties
     *	    that will allow it to be identified uniquely.  (PluginID, 
     *	    AuthenticationProtocol, etc..)
     *
     */
    STDMETHOD(GetProperties)
    (
	THIS_
	REF(IRMAValues*) pIRMAValuesProperties
    ) PURE;

};

#endif /* !_RMAPLGNS_H_ */
