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
 *  Server Configuration File Interface
 *
 */

#ifndef _RMACFG_H_
#define _RMACFG_H_

typedef _INTERFACE  IRMABuffer			IRMABuffer;

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAConfigFile
 *
 *  Purpose:
 *
 *  IID_IRMAConfigFile:
 *
 *	{00001c00-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAConfigFile, 0x00001c00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
				0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef INTERFACE
#define INTERFACE IRMAConfigFile

DECLARE_INTERFACE_(IRMAConfigFile, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)           (THIS_
					 REFIID riid,
					 void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)          (THIS) PURE;

    STDMETHOD_(ULONG,Release)         (THIS) PURE;

    /*
     *	IRMAConfigFile methods
     */
    /************************************************************************
     *	Method:
     *	    IRMAConfigFile::LoadFrom
     *	Purpose:
     *
     *	    LoadFrom tells the server to load the config file specified,
     *	    and sets that file as the default for future Reloads and Saves
     */
    STDMETHOD(LoadFrom)                 (THIS_
					 IRMABuffer* filename) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAConfigFile::Reload
     *	Purpose:
     *
     *	    Reload causes the current default config file to be reloaded.
     */
    STDMETHOD(Reload)                   (THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAConfigFile::Save
     *	Purpose:
     *
     *	    Save causes the current configuration to be written to the
     *	    current default file.
     */
    STDMETHOD(Save)                     (THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAConfigFile::SaveAs
     *	Purpose:
     *
     *	    SaveAs writes the configuration to the named file, and sets that
     *	    file as the default.
     */
    STDMETHOD(SaveAs)                   (THIS_
					 IRMABuffer* pFilename) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAConfigFile::GetFilename
     *	Purpose:
     *
     *	    GetFilename returns the current default file
     */
    STDMETHOD(GetFilename)              (THIS_
					 REF(IRMABuffer*) pFilename) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAConfigFile::SetFilename
     *	Purpose:
     *
     *	    SetFilename sets the current default file, but does not read it
     *	    or change its contents.
     */
    STDMETHOD(SetFilename)              (THIS_
					 IRMABuffer* pFilename) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMARegConfig
 *
 *  Purpose:
 *
 *  IID_IRMARegConfig:
 *
 *	{00001c01-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMARegConfig, 0x00001c01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
				0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef INTERFACE
#define INTERFACE IRMARegConfig

DECLARE_INTERFACE_(IRMARegConfig, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)           (THIS_
					 REFIID riid,
					 void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)          (THIS) PURE;

    STDMETHOD_(ULONG,Release)         (THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARegConfig::WriteKey
     *	Purpose:
     *
     *	    Write out the registry from the passed in keyname to the 
     *  currently active permanent config storage area (ex. config file,
     *  registry).
     */
    STDMETHOD(WriteKey)              (THIS_
					const char* pKeyName) PURE;

};

#endif /* _RMACFG_H_ */
