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

#ifndef _RMAVALUE_H_
#define _RMAVALUE_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE  IUnknown			    IUnknown;
typedef _INTERFACE  IRMABuffer			    IRMABuffer;
typedef _INTERFACE  IRMAKeyValueList		    IRMAKeyValueList;
typedef _INTERFACE  IRMAKeyValueListIter            IRMAKeyValueListIter;
typedef _INTERFACE  IRMAKeyValueListIterOneKey      IRMAKeyValueListIterOneKey;
typedef _INTERFACE  IRMAValues			    IRMAValues;
typedef _INTERFACE  IRMAOptions			    IRMAOptions;

/* Note : GUIDS 3101 - 3107 are deprecated. */

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAKeyValueList
 *
 *  Purpose:
 *
 *	Stores a list of strings, where strings are keyed by not necessarily
 *      unique keys.
 *	
 *
 *  IRMAKeyValueList:
 *
 *	{0x00003108-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAKeyValueList, 0x00003108, 0x901, 0x11d1, 
	    0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#define CLSID_IRMAKeyValueList IID_IRMAKeyValueList

#undef  INTERFACE
#define INTERFACE   IRMAKeyValueList

DECLARE_INTERFACE_(IRMAKeyValueList, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     * Regular methods
     */

     /************************************************************************
     *	Method:
     *	    IRMAKeyValueList::AddKeyValue
     *	Purpose:
     *      Add a new key/value tuple to our list of strings.  You can have
     *      multiple strings for the same key.
     */
    STDMETHOD(AddKeyValue)	(THIS_
				const char* pKey,
				IRMABuffer* pStr) PURE;

     /************************************************************************
     *	Method:
     *	    IRMAKeyValueList::GetIter
     *	Purpose:
     *      Return an iterator that allows you to iterate through all the 
     *      key/value tuples in our list of strings.
     */
    STDMETHOD(GetIter)		(THIS_
				REF(IRMAKeyValueListIter*) pIter) PURE;


     /************************************************************************
     *	Method:
     *	    IRMAKeyValueList::GetIterOneKey
     *	Purpose:
     *      Return an iterator that allows you to iterate through all the 
     *      strings for a particular key.
     */
    STDMETHOD(GetIterOneKey)	(THIS_
				const char* pKey,
				REF(IRMAKeyValueListIterOneKey*) pIter) PURE;

     /************************************************************************
     *	Method:
     *	    IRMAKeyValueList::AppendAllListItems
     *	Purpose:
     *      Append all the key/string tuples from another list to this list.
     *      (You can have duplicate keys.)
     */
    STDMETHOD(AppendAllListItems)   (THIS_
				    IRMAKeyValueList* pList) PURE;
     /************************************************************************
     *	Method:
     *	    IRMAKeyValueList::KeyExists
     *	Purpose:
     *      See whether any strings exist for a particular key.
     */
    STDMETHOD_(BOOL,KeyExists)  (THIS_
				const char* pKey) PURE;

     /************************************************************************
     *	Method:
     *	    IRMAKeyValueList::CreateObject
     *	Purpose:
     *      Create an empty object that is the same class as the current object.
     */
    STDMETHOD(CreateObject)	(THIS_
				REF(IRMAKeyValueList*) pNewList) PURE;

     /************************************************************************
     *	Method:
     *	    IRMAKeyValueList::ImportValues.
     *	Purpose:
     *      Import all the strings from an IRMAValues object into this object.
     *      If this object also supports IRMAValues, it should also import the 
     *      ULONGs and Buffers.  You can have duplicate keys, and old data is 
     *      left untouched.
     */
    STDMETHOD(ImportValues)	(THIS_
				IRMAValues* pValues) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAKeyValueListIter
 *
 *  Purpose:
 *
 *	Iterate over all the items in a CKeyValueList.
 *      Call IRMAKeyValueList::GetIter to create an iterator.
 *	
 *
 *  IRMAKeyValueListIter:
 *
 *	{0x00003109-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAKeyValueListIter,   0x00003109, 0x901, 0x11d1, 
	    0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#define CLSID_IRMAKeyValueListIter IID_IRMAKeyValueListIter

#undef  INTERFACE
#define INTERFACE   IRMAKeyValueListIter

DECLARE_INTERFACE_(IRMAKeyValueListIter, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;


    /*
     * Regular methods
     */

     /************************************************************************
     *	Method:
     *	    IRMAKeyValueListIter::GetNextPair
     *	Purpose:
     *      Each call to this method returns one key/value tuple from your
     *      list of strings.  Strings are returned in same order that they
     *      were inserted.
     */
    STDMETHOD(GetNextPair)	(THIS_
				REF(const char*) pKey,
				REF(IRMABuffer*) pStr) PURE;

     /************************************************************************
     *	Method:
     *	    IRMAKeyValueListIter::ReplaceCurr
     *	Purpose:
     *      Replaces the value in the key/value tuple that was returned 
     *      in the last call to GetNextPair with a new string.
     */
    STDMETHOD(ReplaceCurr)	(THIS_
				IRMABuffer* pStr) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAKeyValueListIterOneKey
 *
 *  Purpose:
 *
 *	Iterate over all the items in a CKeyValueList that match a particular key.
 *      Call IRMAKeyValueList::GetIterOneKey to create an iterator.
 *	
 *
 *  IRMAKeyValueListIterOneKey:
 *
 *	{0x00003110-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAKeyValueListIterOneKey,   0x00003110, 0x901, 0x11d1, 
	    0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#define CLSID_IRMAKeyValueListIterOneKey IID_IRMAKeyValueListIterOneKey

#undef  INTERFACE
#define INTERFACE   IRMAKeyValueListIterOneKey

DECLARE_INTERFACE_(IRMAKeyValueListIterOneKey, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;


    /*
     * Regular methods
     */

     /************************************************************************
     *	Method:
     *	    IRMAKeyValueListIterOneKey::GetNextString
     *	Purpose:
     *      Each call to this method returns one string that matches the 
     *      key for this iterator.  Strings are returned in same order that they
     *      were inserted.
     *      
     */
    STDMETHOD(GetNextString)	(THIS_
				REF(IRMABuffer*) pStr) PURE;

     /************************************************************************
     *	Method:
     *	    IRMAKeyValueListIterOneKey::ReplaceCurr
     *	Purpose:
     *      Replaces the value in the key/value tuple that was referenced
     *      in the last call to GetNextString with a new string.
     *      
     */
    STDMETHOD(ReplaceCurr)	(THIS_
				IRMABuffer* pStr) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAOptions
 *
 *  Purpose:
 *
 *	This is a generic options interface, implemented by any object to
 *	allow its options to be read and set by another component of the
 *	system.
 *	
 *
 *  IRMAOptions:
 *
 *	{0x00003111-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAOptions,   0x00003111, 0x901, 0x11d1, 
	    0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#define CLSID_IRMAOptions IID_IRMAOptions

#undef  INTERFACE
#define INTERFACE   IRMAOptions

DECLARE_INTERFACE_(IRMAOptions, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;


    /*
     * Regular methods
     */

     /************************************************************************
     *	Method:
     *	    IRMAOptions::GetOptions
     *	Purpose:
     *      This method returns a list of the options supported by this
     *	    particular object, along with the value currently set for each
     *	    option. Enumerate the members of the returned IRMAValues object
     *	    to discover what options a component supports and the type of
     *	    each of those options. The value for each name-value pair is
     *	    the current setting for that option.
     *      
     */
    STDMETHOD(GetOptions)	(THIS_
				REF(IRMAValues*) pOptions) PURE;

     /************************************************************************
     *	Method:
     *	    IRMAOptions::SetOptionULONG32
     *	Purpose:
     *      Sets the value of a ULONG32 option. The return value indicates
     *	    whether or not the SetOptionULONG32 call succeeded.
     *      
     */
    STDMETHOD(SetOptionULONG32)	(THIS_
				const char* pName,
				ULONG32 ulValue) PURE;

     /************************************************************************
     *	Method:
     *	    IRMAOptions::SetOptionCString
     *	Purpose:
     *      Sets the value of a CString option. The return value indicates
     *	    whether or not the SetOptionCString call succeeded.
     *      
     */
    STDMETHOD(SetOptionCString)	(THIS_
				const char* pName,
				IRMABuffer* pValue) PURE;

     /************************************************************************
     *	Method:
     *	    IRMAOptions::SetOptionBuffer
     *	Purpose:
     *      Sets the value of a Buffer option. The return value indicates
     *	    whether or not the SetOptionBuffer call succeeded.
     *      
     */
    STDMETHOD(SetOptionBuffer)	(THIS_
				const char* pName,
				IRMABuffer* pValue) PURE;
};


#endif /* !_RMAVALUE_H_ */
