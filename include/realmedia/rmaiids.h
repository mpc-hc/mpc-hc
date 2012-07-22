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
 *  Exhaustive list of IID's used in IRMA interfaces
 *
 *  Note: These IIDs generally are duplicated in the headers that are specific
 *  to each interface, so if you change this file, change the other file(s) as
 *  well.  Having all these IIDS in one files is convenient to some folks, but
 *  not everyone includes this file, hence the need to keep them in individual 
 *  files as well.
 */

#ifndef _RMAIIDS_H_
#define _RMAIIDS_H_

/*
 *  File:
 *	pncom.h
 *  Description:
 *	Interfaces defined by COM.
 *  Interfaces:
 *	IID_IUnknown:		    {00000000-0000-0000-C000000000000046}
 *	IID_IMalloc:		    {00000002-0000-0000-C000000000000046}
 */

/*
 * These GUIDs are defined in pncom.h:
 *
 * DEFINE_GUID(IID_IUnknown,   0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
 * DEFINE_GUID(IID_IMalloc,    0x00000002, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
 *
 */

/*
 *  File:
 *	rmacomm.h
 *  Description:
 *	RealMedia Common Utility interfaces
 *  Interfaces:
 *	IID_IRMACommonClassFactory: {00000000-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAStatistics:	    {00000001-0901-11d1-8B06-00A024406D59}
 *	IID_IRMARegistryID:	    {00000002-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAServerFork:	    {00000003-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAServerControl:	    {00000004-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAServerControl2:	    {00000005-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAReconfigServerResponse:	    {00000006-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMACOMM_H_
DEFINE_GUID(IID_IRMACommonClassFactory,	    0x00000000, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAStatistics,	    	    0x00000001, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMARegistryID,		    0x00000002, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAServerFork,		    0x00000003, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAServerControl,	    0x00000004, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAServerControl2,	    0x00000005, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAReconfigServerResponse, 0x00000006, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAServerReconfigNotification, 0x00000007, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAWantServerReconfigNotification, 0x00000008, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif

/*
 *  File:
 *	rmaengin.h
 *  Description:
 *	Interfaces related to callbacks, networking, and scheduling.
 *  Interfaces:
 *	IID_IRMACallback:	    {00000100-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAScheduler:	    {00000101-0901-11d1-8B06-00A024406D59}
 *	IID_IRMATCPResponse:	    {00000102-0901-11d1-8B06-00A024406D59}
 *	IID_IRMATCPSocket:	    {00000103-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAListenResponse:	    {00000104-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAListenSocket:	    {00000105-0901-11d1-8B06-00A024406D59}
 *	IID_IRMANetworkServices:    {00000106-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAUDPResponse:	    {00000107-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAUDPSocket:	    {00000108-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAResolver:           {00000109-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAResolverResponse:   {0000010A-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAInterruptSafe:      {0000010B-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAAsyncIOSelection:   {0000010C-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAUDPMulticastInit:   {0000010D-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAInterruptState:     {0000010E-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAOptimizedScheduler: {0000010F-0901-11d1-8B06-00A024406D59}
 *	IID_IRMALoadBalancedListen: {00000110-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAOverrideDefaultServices: {00000111-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAHTTPPostObject:	    {00000112-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAHTTPPostResponse:   {00000113-0901-11d1-8B06-00A024406D59}
 *	IID_IRMASetSocketOption:    {00000114-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAThreadSafeMethods:  {00000115-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAMutex:		    {00000116-0901-11d1-8B06-00A024406D59}
 *
 */
#ifndef _RMAENGIN_H_
DEFINE_GUID(IID_IRMACallback,		0x00000100, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAScheduler,		0x00000101, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMATCPResponse,	0x00000102, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMATCPSocket,		0x00000103, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAListenResponse,	0x00000104, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAListenSocket,	0x00000105, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMANetworkServices,	0x00000106, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMANetworkServices2,   0x17951551, 0x5683, 0x11d3, 0xb6, 0xba, 0x0, 0xc0, 0xf0, 0x31, 0xc2, 0x37);
DEFINE_GUID(IID_IRMAUDPResponse,	0x00000107, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAUDPSocket,		0x00000108, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAResolver,		0x00000109, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAResolverResponse,	0x0000010A, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAInterruptSafe,	0x0000010B, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAsyncIOSelection,	0x0000010C, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAUDPMulticastInit,	0x0000010D, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAInterruptState,	0x0000010E, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAOptimizedScheduler,	0x0000010F, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMALoadBalancedListen,	0x00000110, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAOverrideDefaultServices,	0x00000111, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMASetSocketOption,	0x00000114, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAThreadSafeMethods,	0x00000115, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAMutex,		0x00000116, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif


/*
 *  File:
 *	rmafiles.h
 *  Description:
 *	Interfaces related to file systems.
 *  Interfaces:
 *	IID_IRMAFileObject:		{00000200-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAFileResponse:		{00000201-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAFileSystemObject:	{00000202-0901-11d1-8B06-00A024406D59}
 *	IID_IRMADirObject:		{00000203-0901-11d1-8B06-00A024406D59}
 *	IID_IRMADirResponse:		{00000204-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAFileStat:		{00000205-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAFileStatResponse:	{00000206-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAFileSystemManager:	{00000207-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAFileSystemManagerResponse:
 *					{00000208-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAFileExists:		{00000209-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAFileExistsResponse:	{0000020A-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAFileMimeMapper:		{0000020B-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAFileMimeMapperResponse:	{0000020C-0901-11d1-8B06-00A024406D59}
 *	IID_IRMABroadcastMapper:	{0000020D-0901-11d1-8B06-00A024406D59}
 *	IID_BroadcastMimeMapperResponse:{0000020E-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAGetFileFromSamePool:    {0000020F-0901-11d1-8B06-00A024406D59}
 *      IID_GetFileFromSamePoolResponse:{00000210-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAFileAuthenticator:      {00000211-0901-11d1-8B06-00A024406D59} 
 *      IID_IRMARequestHandler:         {00000212-0901-11d1-8B06-00A024406D59} 
 *      IID_IRMARequest:                {00000213-0901-11d1-8B06-00A024406D59} 
 *      IID_IRMAFileRename:             {00000214-0901-11d1-8B06-00A024406D59}
 *      IID_IRMADirHandler:             {00000215-0901-11d1-8B06-00A024406D59}
 *      IID_IRMADirHandlerResponse:     {00000216-0901-11d1-8B06-00A024406D59}
 *	IID_IRMARequestContext		{00000217-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAFileRemove:             {0000021A-0901-11d1-8B06-00A024406D59}
 *      DEPRECATED DEPRECATED           {0000021B-0901-11d1-8B06-00A024406D59}
 *
 */
#ifndef _RMAFILES_H_
DEFINE_GUID(IID_IRMAHTTPPostObject,	0x00000112, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAHTTPPostResponse,	0x00000113, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFileObject,			0x00000200, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFileResponse,		0x00000201, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFileSystemObject,		0x00000202, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMADirObject,			0x00000203, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);	//NOTE, use is deprecated
DEFINE_GUID(IID_IRMADirResponse,		0x00000204, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);	//NOTE, use is deprecated
DEFINE_GUID(IID_IRMAFileStat,			0x00000205, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFileStatResponse,		0x00000206, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFileSystemManager,		0x00000207, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFileSystemManagerResponse,	0x00000208, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFileExists,			0x00000209, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFileExistsResponse,		0x0000020a, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFileMimeMapper,		0x0000020b, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFileMimeMapperResponse,	0x0000020c, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMABroadcastMapper,		0x0000020d, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMABroadcastMapperResponse,	0x0000020e, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAGetFileFromSamePool,	0x0000020f, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAGetFileFromSamePoolResponse,0x00000210, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFileAuthenticator,          0x00000211, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMARequestHandler,             0x00000212, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMARequest,                    0x00000213, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFileRename,                 0x00000214, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMADirHandler,                 0x00000215, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMADirHandlerResponse,         0x00000216, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMARequestContext,		0x00000217, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFileRemove,                 0x0000021a, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif
/*
 *  File:
 *	rmarendr.h
 *  Description:
 *	Interfaces related to renderers.
 *  Interfaces:
 *	IID_IRMARenderer:	    	{00000300-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMARenderer,		    0x00000300, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  File:
 *	rmacore.h
 *  Description:
 *	Interfaces related to the client core services.
 *  Interfaces:
 *	IID_IRMAStream:		    {00000400-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAStreamSource	    {00000401-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPlayer:		    {00000402-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAClientEngine:	    {00000403-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAClientEngineSelector{00000404-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAClientEngineSetup:  {00000405-0901-11d1-8B06-00A024406D59}
 *				 :  {00000406-0901-11d1-8B06-00A024406D59}  -- Deprecated 
 *	IID_IRMAInfoLogger:	    {00000409-0901-11d1-8B06-00A024406D59}
 *	    			    {0000040F-0901-11d1-8B06-00A024406D59}  -- Deprecated
 *	IID_IRMAPlayer2:	    {00000411-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMACORE_H_
DEFINE_GUID(IID_IRMAStream,	    0x00000400, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAStreamSource,   0x00000401, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPlayer,	    0x00000402, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAClientEngine,   0x00000403, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#ifdef _UNIX
DEFINE_GUID(IID_IRMAClientEngineSelector,	0x00000404, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif
DEFINE_GUID(IID_IRMAClientEngineSetup,		0x00000405, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAInfoLogger, 		0x00000409, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPlayer2,			0x00000411, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif
/*
 *  File:
 *	rmaprefs.h
 *  Description:
 *	Interfaces related to persistent preferences services.
 *  Interfaces:
 *	IID_IRMAPreferences:	    {00000500-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMAPREFS_H_
DEFINE_GUID(IID_IRMAPreferences,    0x00000500, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPreferences2, 0x00000503, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPreferenceEnumerator, 0x00000504, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif

/*
 *  File:
 *	rmamon.h
 *  Description:
 *	Interfaces related to Monitor plugins.
 *  Interfaces:
 *	IID_IRMAPNRegistry:	    {00000600-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPropWatch:    	    {00000601-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPropWatchResponse:  {00000602-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAActiveRegistry:	    {00000603-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAActivePropUser:	    {00000604-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAActivePropUserResponse:	{00000605-0901-11d1-8B06-00A024406D59}
 *	IID_IRMACopyRegistry:	{00000606-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPNRegistryAltStringHandling:	{00000607-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMAMON_H_
DEFINE_GUID(IID_IRMAPNRegistry,	    0x00000600, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPropWatch,	    0x00000601, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPropWatchResponse,	    0x00000602, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAActiveRegistry,	0x00000603, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAActivePropUser,	0x00000604, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAActivePropUserResponse, 0x00000605, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMACopyRegistry, 0x00000606, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPNRegistryAltStringHandling, 0x00000607, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif
/*
 *  File:
 *	rmaausvc.h
 *  Description:
 *	Interfaces related to audio services.
 *  Interfaces:
 *	IID_IRMAAudioPlayer:        	 {00000700-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAAudioPlayerResponse:	 {00000701-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAAudioStream: 	    	 {00000702-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAAudioDevice: 	    	 {00000703-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAAudioDeviceResponse:	 {00000704-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAAudioHook:	     	 {00000705-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAAudioStreamInfoResponse: {00000706-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAVolume: 		 {00000707-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAVolumeAdviseSink: 	 {00000708-0901-11d1-8B06-00A024406D59}
 *	IID_IRMADryNotification: 	 {00000709-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAAudioDeviceManager: 	 {0000070A-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAAudioCrossFade: 	 {0000070B-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAAudioStream2: 		 {0000070C-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAAudioPushdown: 		 {0000070D-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAAudioHookManager:	 {0000070E-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAAudioPlayer,	     0x00000700, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAudioPlayerResponse,     0x00000701, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAudioStream,	     0x00000702, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAudioDevice,	     0x00000703, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAudioDeviceResponse,     0x00000704, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAudioHook,		     0x00000705, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAudioStreamInfoResponse, 0x00000706, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAVolume, 		     0x00000707, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAVolumeAdviseSink, 	     0x00000708, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMADryNotification,	     0x00000709, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAudioDeviceManager,	     0x0000070A, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAudioCrossFade,	     0x0000070B, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAudioStream2,	     0x0000070C, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAudioPushdown,	     0x0000070D, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAudioHookManager,	     0x0000070E, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  File:
 *	rmaerror.h
 *  Description:
 *	Interfaces related to error reporting and receiving notification of errors.
 *  Interfaces:
 *	IID_IRMAErrorMessages:	    {00000800-0901-11d1-8B06-00A024406D59}
 *  	IID_IRMAErrorSink:	    {00000801-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAErrorSinkControl:   {00000802-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMAERROR_H_
DEFINE_GUID(IID_IRMAErrorMessages, 	    0x00000800, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAErrorSink,		    0x00000801, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAErrorSinkControl,	    0x00000802, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif
/*
 *  File:
 *	rmahyper.h
 *  Description:
 *	Simple Hyper Navigation Interfaces
 *  Interfaces:
 *	IID_IRMAHyperNavigate:			{00000900-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAHyperNavigate2:			{00000901-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAHyperNavigateWithContext:	{00000902-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAHyperNavigate,		    0x00000900, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  File:
 *	rmaclsnk.h
 *  Description:
 *	Client Advise Sink Interfaces
 *  Interfaces:
 *	IID_IRMAClientAdviseSink:   {00000B00-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAClientAdviseSink,  0x00000B00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  File:
 *	rmaplugn.h
 *  Description:
 *	Plugin inspector interface
 *  Interfaces:
 *	IID_IRMAPlugin:			{00000C00-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPluginEnumerator	{00000C01-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPluginGroupEnumerator   {00000C02-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPluginReloader		{00000C03-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPluginFactory		{00000C04-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMAPLUGN_H_
DEFINE_GUID(IID_IRMAPlugin,		0x00000C00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPluginEnumerator,	0x00000C01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPluginGroupEnumerator, 0x00000C02, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPluginReloader,	0x00000C03, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPluginFactory,	0x00000C04, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif
/*
 *  File:
 *	rmawin.h
 *  Description:
 *	Site interfaces
 *  Interfaces:
 *	IID_IRMASiteWindowed:				{00000D01-0901-11d1-8B06-00A024406D59}
 *	IID_IRMASiteWindowless:				{00000D02-0901-11d1-8B-6-00A024406D59}
 *	IID_IRMASite:					{00000D03-0901-11d1-8B-6-00A024406D59}
 *	IID_IRMASiteUser:				{00000D04-0901-11d1-8B-6-00A024406D59}
 *	IID_IRMASiteWatcher:				{00000D05-0901-11d1-8B-6-00A024406D59}
 *	IID_IRMASiteUserSupplier:			{00000D06-0901-11d1-8B-6-00A024406D59}
 *	IID_IRMASiteSupplier:				{00000D07-0901-11d1-8B-6-00A024406D59}
 *	IID_IRMASiteManager:				{00000D08-0901-11d1-8B-6-00A024406D59}
 *	IID_IRMAMultiInstanceSiteUserSupplier:		{00000D09-0901-11d1-8B-6-00A024406D59}
 *	IID_IRMASite2:					{00000D0A-0901-11d1-8B-6-00A024406D59}
 *	IID_IRMASiteFullScreen				{00000D0B-0901-11d1-8B-6-00A024406D59}
 *	IID_IRMAEventHookMgr				{00000D0D-0901-11d1-8B-6-00A024406D59}
 *	IID_IRMAEventHook				{00000D0E-0901-11d1-8B-6-00A024406D59}
 *	IID_IRMAPassiveSiteWatcher			{00000D0F-0901-11d1-8B-6-00A024406D59}
 *	IID_IRMAStatusMessage				{00000D10-0901-11d1-8B-6-00A024406D59}
 */
#ifndef _RMAWIN_H_
DEFINE_GUID(IID_IRMASiteWindowed,		    0x00000D01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMASiteWindowless,		    0x00000D02, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMASite, 			    0x00000D03,	0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMASiteUser,			    0x00000D04, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMASiteWatcher,		    0x00000D05, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMASiteUserSupplier,		    0x00000D06, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMASiteSupplier,		    0x00000D07, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMASiteManager,		    0x00000D08, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAMultiInstanceSiteUserSupplier,  0x00000D09, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMASite2,  			    0x00000D0A, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMASiteFullScreen,		    0x00000D0B, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAEventHookMgr,		    0x00000D0D, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAEventHook,			    0x00000D0E, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPassiveSiteWatcher,		    0x00000D0F, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAStatusMessage,		    0x00000D10, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif
/*
 *  File:
 *	rmaformt.h
 *  Description:
 *	Interfaces related to file and broadcast format plugins.
 *  Interfaces:
 *
 *	IID_IRMAFileFormatObject:	{00000F00-0901-11d1-8B06-00A024406D59}
 *	IID_IRMABroadcastFormatObject:	{00000F01-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAFormatResponse:		{00000F02-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPacketFormat:		{00000F03-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPacketTimeOffsetHandler {00000F04-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAPacketTimeOffsetHandlerResponse {00000F05-0901-11d1-8B06-00A024406D59}
 *      IID_IRMALiveFileFormatInfo      {00000F06-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMAFORMT_H_
DEFINE_GUID(IID_IRMAFileFormatObject,	    0x00000F00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMABroadcastFormatObject,  0x00000F01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFormatResponse,	    0x00000F02, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPacketFormat,	    0x00000F03, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPacketTimeOffsetHandler, 0x00000F04, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPacketTimeOffsetHandlerResponse, 0x00000F05, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMALiveFileFormatInfo,     0x00000F06, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif

/*
 *  File:
 *	rmapends.h
 *  Description:
 *	Interfaces related to get pending status from objects
 *  Interfaces:
 *	IRMAPendingStatus:	{00001100-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMAPENDS_H_
DEFINE_GUID(IID_IRMAPendingStatus,	    0x00001100, 0x901, 0x11d1, 0x8b, 0x6, 0x0,  0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif
/*
 *  File:
 *	rmapckts.h
 *  Description:
 *	Interfaces related to buffers, packets, streams, etc.
 *  Interfaces:
 *	IID_IRMABuffer:		    {00001300-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPacket:		    {00001301-0901-11d1-8B06-00A024406D59}
 *	IID_IRMARTPPacket	    {0169A731-1ED0-11d4-952B-00902742C923}
 *	IID_IRMAValues:		    {00001302-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAValuesRemove:	    {00001303-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMAPCKTS_H_
DEFINE_GUID(IID_IRMABuffer,		    0x00001300, 0x0901, 0x11d1, 0x8b, 0x06, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59); 
DEFINE_GUID(IID_IRMAPacket,		    0x00001301, 0x0901, 0x11d1, 0x8b, 0x06, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMARTPPacket,		    0x0169a731, 0x1ed0, 0x11d4, 0x95, 0x2b, 0x0, 0x90, 0x27, 0x42, 0xc9, 0x23);
DEFINE_GUID(IID_IRMAValues,		    0x00001302, 0x0901, 0x11d1, 0x8b, 0x06, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59); 
DEFINE_GUID(IID_IRMAValuesRemove,	    0x00001303, 0x0901, 0x11d1, 0x8b, 0x06, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59); 
#endif
/*
 *  File:
 *      rmaasm.h
 *  Description:
 *	Interfaces related to abm and back channel support.
 * 
 *  Interfaces:
 *  	IID_IRMABackChannel:	    {00001500-0901-11d1-8B06-00A024406D59}
 *  	IID_IRMAASMSource:	    {00001501-0901-11d1-8B06-00A024406D59}
 *  	IID_IRMAASMStream:	    {00001502-0901-11d1-8B06-00A024406D59}
 *  	IID_IRMAASMStreamSink:	    {00001503-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMAASM_H_
DEFINE_GUID(IID_IRMABackChannel,	    0x00001500, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAASMSource,		    0x00001501, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAASMStream,		    0x00001502, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAASMStreamSink,	    0x00001503, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif
/*
 *  File:
 *      rmaencod.h
 *  Description:
 *      Interfaces related to superencoders.
 *
 *  Interaces:
 *      IID_IRMAEncoderResponse     {00001600-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAEncoder             {00001601-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAEncoderCompletion   {00001602-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAConnectionlessControl
                                    {00001603-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAEncoderResponseCompletion
                                    {00001604-0901-11d1-8B06-00A024406D59}
 *      IID_IRMATransportControl    {00001605-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMAENCOD_H_
DEFINE_GUID(IID_IRMAEncoderResponse, 	0x00001600, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAEncoder,		0x00001601, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAEncoderCompletion,	0x00001602, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAConnectionlessControl,
					0x00001603, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAEncoderResponseCompletion,
					0x00001604, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMATransportControl,	0x00001605, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif
/*
 *  File:
 *      rmaauth.h
 *  Description:
 *      Password handling API
 *  Interfaces:
 *      IID_IRMAPassword            {00001700-0901-11d1-8B06-00A024406D59}
 */

/*
 * 000017**-0901-11d1-8B06-00A024406D59 is reserved for interfaces in rmaauth.h (below)
 */

/*
 *  File:
 *      rmaauth.h
 *  Description:
 *      Authentication API
 *  Interfaces:
 *      IID_IRMAAuthenticator	      {00001800-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAAuthenticatorResponse {00001801-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAAuthenticatorRequest  {00001802-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPassword	      {00001700-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAAuthenticationManager {00001A00-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAAuthenticationManagerResponse 
 *                                    {00001A01-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMAAUTH_H_
DEFINE_GUID(IID_IRMAAuthenticator, 	    0x00001800, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAuthenticatorResponse,  0x00001801, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAuthenticatorRequest,   0x00001802, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPassword, 		    0x00001700, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAuthenticationManager,	    0x00001a00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAuthenticationManagerResponse,  0x00001a01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif
/*
 *  File:
 *      rmasdesc.h
 *  Description:
 *      Stream description API
 *  Interfaces:
 *      IID_IRMAStreamDescription    {00001900-0901-11d1-8B06-00A024406D59}
 *      IID_IRMARTPPacketInfo        {00001901-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMASDESC_
DEFINE_GUID(IID_IRMAStreamDescription, 0x00001900, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMARTPPacketInfo,     0x00001901, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif
/*
 * 00001A**-0901-11d1-8B06-00A024406D59 is reserved for interfaces in rmaauth.h (above)
 */

/*
 *  File:
 *      rmalvtxt.h
 *  Description:
 *      Interfaces related to live text superencoder.
 *
 *  Interaces:
 *      IID_IRMALiveText         {00001b00-0901-11d1-8B06-00A024406D59}
 *      IID_IRMALiveText2        {00001b01-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMALVTXT_H_
DEFINE_GUID(IID_IRMALiveText,	0x00001b00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMALiveText2,	0x00001b01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif
/*
 *  File:
 *      rmacfg.h
 *  Description:
 *      Interfaces used by server configuration tools.
 *
 *  Interfaces:
 *      IID_IRMAConfigFile	{00001c00-0901-11d1-8B06-00A024406D59}
 *	IID_IRMARegConfig	{00001c01-0901-11d1-8B06-00A024406D59}
 *
 */
#ifndef _RMACFG_H_
DEFINE_GUID(IID_IRMAConfigFile, 0x00001c00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
	 		0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMARegConfig, 0x00001c01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
				0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif

/*
 *  File:
 *      rmappv.h
 *  Description:
 *      Interfaces related to Pay Per View Database Plugins
 *  Interfaces:
 *      IID_IRMAPPVDatabase {00001d00-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAPPVDatabase,  
			0x00001d00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);


/*
 *  00001e**-0901-11d1-8B06-00A024406D59 is reserved for an interface which
 *  has been deprecated.
 */

/*
 *  File:
 *      rmacmenu.h
 *  Description:
 *      Interfaces used by renderers for context menus.
 *
 *  Interfaces:
 *  IID_IRMAContextMenu		    {00001f00-0901-11d1-8B06-00A024406D59}
 *  IID_IRMAContextMenuResponse	    {00001f01-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMACMENU_H_
DEFINE_GUID(IID_IRMAContextMenu, 0x00001f00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAContextMenuResponse, 0x00001f01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#endif
/*
 *  File:
 *      rmaphook.h
 *  Description:
 *      Interfaces used by the top level client. client core and renderer to 
 *	support Selective Record.
 *
 *  Interfaces:
 *      IID_IRMAPacketHook		    {00002000-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPacketHookManager	    {00002001-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPacketHookHelper	    {00002002-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPacketHookHelperResponse    {00002003-0901-11d1-8B06-00A024406D59}
 */

DEFINE_GUID(IID_IRMAPacketHook,			0x00002000, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPacketHookManager,		0x00002001, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPacketHookHelper,		0x00002002, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPacketHookHelperResponse,	0x00002003, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  File:
 *      rmapsink.h
 *  Description:
 *      Interfaces used by the top level client or renderers to determine
 *	that a player has been created or closed.
 *
 *  Interfaces:
 *      IID_IRMAPlayerCreationSink	    {00002100-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAPlayerSinkControl    	    {00002101-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAPlayerCreationSink,		0x00002100, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPlayerSinkControl, 		0x00002101, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  File:
 *      rmavsurf.h
 *  Description:
 *      Interface used by renderers to blt data to the screen (when in
 *	full screen mode).
 *
 *  Interfaces:
 *      IID_IRMAVideoSurface	    	    {00002200-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAVideoSurface, 		0x00002200, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);


/*
 *  File:
 *      rmagroup.h
 *  Description:
 *      Client side Group related interfaces
 *
 *  Interfaces:
 *	IID_IRMAGroup			{00002400-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAGroupManager		{00002401-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAGroupSink		{00002402-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAGroup,		0x00002400, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAGroupManager,	0x00002401, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAGroupSink,		0x00002402, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  File:
 *      rmaupgrd.h
 *  Description:
 *      Interfaces used by player for auto-upgrade.
 *
 *  Interfaces:
 *  IID_IRMAUpgradeCollection    {00002500-0901-11d1-8B06-00A024406D59}
 *  IID_IRMAUpgradeHandler	 {00002501-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAUpgradeCollection,
                        0x00002500, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAUpgradeHandler,
                        0x00002501, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  File:
 *      rmaallow.h
 *  Description:
 *      Interfaces related to Allowance plugins
 *  Interfaces:
 *      IID_IRMAPlayerConnectionAdviseSink {00002600-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAPlayerConnectionResponse   {00002601-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAPlayerController           {00002602-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAPlayerConnectionAdviseSinkManager
                                           {00002603-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAProxyConnectionAdviseSink  {00002604-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAProxyConnectionResponse    {00002605-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAProxyController            {00002605-0901-11d1-8B06-00A024406D59}
 *      IID_IRMAPlayerControllerProxyRedirect {00002607-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAPlayerConnectionAdviseSink,  
			0x00002600, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPlayerConnectionResponse,    
			0x00002601, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPlayerController,            
			0x00002602, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPlayerConnectionAdviseSinkManager,            
			0x00002603, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

DEFINE_GUID(IID_IRMAProxyConnectionAdviseSink,  
			0x00002604, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAProxyConnectionResponse,  
			0x00002605, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAProxyController,  
			0x00002606, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPlayerControllerProxyRedirect, 0x00002607, 0x901, 0x11d1,
            0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  File:
 *      rmaaconf.h
 *  Description:
 *      Interfaces used by the top level client. client core to 
 *	support Auto. Transport Configuration
 *
 *  Interfaces:
 *	IID_IRMAAutoConfig		    {00002700-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAAutoConfigResponse	    {00002701-0901-11d1-8B06-00A024406D59}
 */

DEFINE_GUID(IID_IRMAAutoConfig,			0x00002700, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAutoConfigResponse,		0x00002701, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  File:
 *      rmaauthn.h
 *  Description:
 *      Interfaces used to validate a users access to content.
 *
 *  Interfaces:
 *	IID_IRMACredRequestResponse,	{00002800-0901-11d1-8B06-00A024406D59}
 *	IID_IRMACredRequest,		{00002801-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAClientAuthResponse,	{00002802-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAClientAuthConversation,	{00002803-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAServerAuthResponse,	{00002804-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAServerAuthConversation,	{00002805-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAUserContext,		{00002806-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAUserProperties,		{00002807-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAUserImpersonation,	{00002808-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAUserDB,			{00002809-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAChallengeResponse,	{0000280A-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAChallenge,		{0000280B-0901-11d1-8B06-00A024406D59}
 */

DEFINE_GUID(IID_IRMACredRequestResponse,    0x00002800, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMACredRequest,	    0x00002801, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAClientAuthResponse,	    0x00002802, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAClientAuthConversation, 0x00002803, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAServerAuthResponse,	    0x00002804, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAServerAuthConversation, 0x00002805, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAUserContext,	    0x00002806, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAUserProperties,	    0x00002807, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAUserImpersonation,	    0x00002808, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAChallengeResponse,	    0x00002809, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAChallenge,		    0x0000280A, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);


/*
 * File:
 *    rmaplgns.h
 *
 * Description:
 *    Interfaces for Plugins:
 *	IRMAObjectConfiguration	- Consistant configuration.
 *	IRMAPluginProperties	- Consistant property retrival.
 *
 * Interfaces:
 *    IID_IRMAObjectConfiguration:  {00002900-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAPluginProperties:	    {00002901-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAObjectConfiguration,    0x00002900, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPluginProperties,	    0x00002901, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 * File:
 *    rmadb.h
 *
 * Description:
 *    Interfaces for Plugins:
 *	IRMADatabaseManager			- Creates Configured Database Instances
 *	IRMAAuthenticationDBManagerResponse	- Provides Callbacks for IRMAAuthenticationDBManager
 *	IRMAAuthenticationDBManager		- Functions to add and remove users from a database
 *	IRMAAsyncEnumAuthenticationDBResponse	- Provides Callbacks for IRMAAsyncEnumAuthenticationDB
 *	IRMAAsyncEnumAuthenticationDB		- Functions to enumerate the list of users in a database
 *	IRMAAuthenticationDBAccessResponse	- Provides Callbacks for IRMAAuthenticationDBAccess
 *	IRMAAuthenticationDBAccess		- Functions to access a users info in the database
 *	IRMAGUIDDBManagerResponse		- Provides Callbacks for IRMAGUIDDBManager
 *	IRMAGUIDDBManager			- Functions to add and remove GUID's from a database
 *	IRMAPPVDBManagerResponse		- Provides Callbacks for IRMAPPVDBManager
 *	IRMAPPVDBManager			- Functions to add, remove, and adjust a user's permissions from a database
 *	IRMARedirectDBManagerResponse		- Provides Callbacks for IRMARedirectDBManager
 *	IRMARedirectDBManager			- Functions to add and remove URL Redirects from a database
 *	IRMARegistrationLoggerResponse		- Provides Callbacks for IRMARegistrationLogger
 *	IRMARegistrationLogger			- Functions to Log registration Activity.
 *
 * Interfaces:
 *    IID_IRMADatabaseManager:			{00002A00-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAAuthenticationDBManagerResponse:  {00002A01-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAAuthenticationDBManager:		{00002A02-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAAsyncEnumAuthenticationDBResponse:{00002A03-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAAsyncEnumAuthenticationDB:	{00002A04-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAAuthenticationDBAccessResponse:	{00002A05-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAAuthenticationDBAccess:		{00002A06-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAGUIDDBManagerResponse:		{00002A07-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAGUIDDBManager:			{00002A08-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAPPVDBManagerResponse:		{00002A09-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAPPVDBManager:			{00002A0A-0901-11d1-8B06-00A024406D59}
 *    IID_IRMARedirectDBManagerResponse:	{00002A0B-0901-11d1-8B06-00A024406D59}
 *    IID_IRMARedirectDBManager:		{00002A0C-0901-11d1-8B06-00A024406D59}
 *    IID_IRMARegistrationLoggerResponse:	{00002A0D-0901-11d1-8B06-00A024406D59}
 *    IID_IRMARegistrationLogger:		{00002A0E-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMADatabaseManager,			    0x00002A00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAuthenticationDBManagerResponse,	    0x00002A01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAuthenticationDBManager,		    0x00002A02, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAsyncEnumAuthenticationDBResponse,	    0x00002A03, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAsyncEnumAuthenticationDB,		    0x00002A04, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAuthenticationDBAccessResponse,	    0x00002A05, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAAuthenticationDBAccess,		    0x00002A06, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAGUIDDBManagerResponse,		    0x00002A07, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAGUIDDBManager,			    0x00002A08, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPPVDBManagerResponse,		    0x00002A09, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAPPVDBManager,			    0x00002A0A, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMARedirectDBManagerResponse,		    0x00002A0B, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMARedirectDBManager,			    0x00002A0C, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMARegistrationLoggerResponse,		    0x00002A0D, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMARegistrationLogger,			    0x00002A0E, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);


/*
 * File:
 *	rmaxmltg.h
 *
 * Description:
 *  Interfaces for Plugins:
 *	IRMAXMLTagHandler: Interface for registering for a specific tag
 *	and providing an IRMAXMLTagObject to tagfsys.
 *	(Works like IRMAFileSystemObject)
 *
 *	IRMAXMLTagObject: Interface for receiving the contents of a tag
 *	for which the creating IRMAXMLTagHandler has registerd.
 *
 *	IRMAXMLTagObjectResponse: Interface for IRMAXMLTagObject to return
 *	the replacement for the tag.  This is implemented by tagfsys.
 *
 *
 * Interfaces:
 *	IID_IRMAXMLTagObjectResponse: {00002C02-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAXMLTagHandler:	{00002C03-0901-11d1-8B06-00A024406D59}
 *	IID_IRMAXMLTagObject:	{00002C04-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMAXMLTG_H
DEFINE_GUID(IID_IRMAXMLTagObjectResponse, 0x00002C02, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAXMLTagHandler, 0x00002C03, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAXMLTagObject, 0x00002C04, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif


/*
 * File:
 *    rmacache.h
 *
 * Description:
 *    Interfaces for caching services:
 *	IRMACache				- Creates IRMACacheFiles
 *	IRMACacheResponse			- Response object for IRMACache
 *	IRMACacheFile				- Persistant store object for caching
 *	IRMACacheFileResponse			- Response object for IRMACacheFile
 *
 * Interfaces:
 *    IID_IRMACache:				{00002E00-0901-11d1-8B06-00A024406D59}
 *    IID_IRMACacheResponse:			{00002E01-0901-11d1-8B06-00A024406D59}
 *    IID_IRMACacheFile:			{00002E02-0901-11d1-8B06-00A024406D59}
 *    IID_IRMACacheFileResponse:		{00002E03-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAMIIFetch:				{00002E04-0901-11d1-8B06-00A024406D59}
 */

DEFINE_GUID(IID_IRMACache,			0x00002E00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMACacheResponse,		0x00002E01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMACacheFile,			0x00002E02, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMACacheFileResponse,		0x00002E03, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAMIIFetch,			0x00002E04, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);


/*
 *  File: intrpm.h
 *
 *  IID_IRMAInterPluginMessenger:	{00003000-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAInterPluginMessenger,	0x00003000, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  File: rmavalue.h
 *
 *  DEPRECATED:			{00003100-0901-11d1-8B06-00A024406D59}
 *  DEPRECATED:			{00003101-0901-11d1-8B06-00A024406D59}
 *  DEPRECATED:			{00003102-0901-11d1-8B06-00A024406D59}
 *  DEPRECATED:			{00003103-0901-11d1-8B06-00A024406D59}
 *  DEPRECATED:			{00003104-0901-11d1-8B06-00A024406D59}
 *  DEPRECATED:			{00003105-0901-11d1-8B06-00A024406D59}
 *  DEPRECATED:			{00003106-0901-11d1-8B06-00A024406D59}
 *  DEPRECATED:			{00003107-0901-11d1-8B06-00A024406D59}
 *
 *  IID_IRMAKeyValueList:	{00003108-0901-11d1-8B06-00A024406D59}
 *  IID_IRMAKeyValueListIter:	{00003109-0901-11d1-8B06-00A024406D59}
 *  IID_IRMAKeyValueListIterOneKey: {00003110-0901-11d1-8B06-00A024406D59}
 *  IID_IRMAOptions:		{00003111-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMAVALUE_H_
/* DEPRECATED 3100 - 3107 */
DEFINE_GUID(IID_IRMAKeyValueList,	0x00003108, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAKeyValueListIter,	0x00003109, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAKeyValueListIterOneKey,	0x00003110, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAOptions,		0x00003111, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif

/*
 * File:
 *    rmcookie.h
 *
 * Description:
 *    Interfaces for Plugins:
 *	IRMACookies		- Cookie database management APIs
 *	IRMACookiesHelper	- Cookie output helper APIs
 *
 * Interfaces:
 *    IID_IRMACookies:				{00003200-0901-11d1-8B06-00A024406D59}
 *    IID_IRMACookiesHelper:			{00003201-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMACookies,			0x00003200, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMACookiesHelper,		0x00003201, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  File: addrpool.h
 *
 *  IID_IRMAMulticastAddressPool:	{00003300-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAMulticastAddressPool,	0x00003300, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  File: sapmgr.h
 *
 *  IID_IRMASapManager:	{00003400-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMASapManager,	0x00003400, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 * File:
 *    rmavsrc.h
 *
 * Description:
 *    Interfaces for Plugins:
 *	IRMAFileViewSource		- Interface so file formats can support view source.
 *	IRMAFileViewSourceResponse	- Response interface.
 *
 * Interfaces:
 *    IID_IRMAFileViewSource:				{00003500-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAFileViewSourceResponse:			{00003501-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAViewSourceCommand:			{00003504-0901-11d1-8B06-00A024406D59}
 *    IID_IRMAViewSourceURLResponse			{00003505-0901-11d1-8B06-00A024406D59}
 */
#ifndef _RMAVSRC_H_
DEFINE_GUID(IID_IRMAFileViewSource,		0x00003500, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAFileViewSourceResponse,	0x00003501, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAViewSourceCommand,		0x00003504, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMAViewSourceURLResponse,	0x00003505, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#endif





/* File:
 *	embdengn.h
 *
 * Description:
 *
 *  IRCAEmbeddedPlayerEngine - RCA embedded player engine
 */

DEFINE_GUID(IID_IRCAEmbeddedEngine, 
    0x00003800, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/* File:
 *	embdctxt.h
 *
 * Description:
 *
 *  IRCAEmbeddedContext - RCA embedded player engine context
 */

DEFINE_GUID(IID_IRCAEmbeddedContext,
    0x00003801, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/* File:
 *	embdplay.h
 *
 * Description:
 *
 *  IRCAEmbeddedPlayer - RCA embedded player interface
 */

DEFINE_GUID(IID_IRCAEmbeddedPlayer,
    0x00003802, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/* File:
 *	embdsink.h
 *
 * Description:
 *
 *  IRCAEmbeddedSink	     - RCA embedded player event sink
 *  IRCAEmbeddedSinkResponse - RCA embedded player event sink response object
 */

DEFINE_GUID(IID_IRCAEmbeddedSink,
    0x00003803, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRCAEmbeddedSinkResponse,
    0x00003804, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/* File:
 *	embdstrm.h
 *
 * Description:
 *
 *  IRCAEmbeddedStreamManager - RCA embedded player stream manager
 *  IRCAEmbeddedStream	      - RCA embedded player stream
 */

DEFINE_GUID(IID_IRCAEmbeddedStreamManager,
    0x00003805, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRCAEmbeddedStream,
    0x00003806, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/* File:
 *	rmadtcvt.h
 *
 * Description
 *  IRMADataConvertSystemObject - RMA Stream data conversion creator
 *  IRMADataConvert - RMA Stream data conversion
 *  IRMADataConvertResponse - response for above
 *  IRMADataRevert - RMA Stream data reversion
 *  IRMADataRevertResponse - response for above
 */
DEFINE_GUID(IID_IRMADataConvertSystemObject,
    0x00003900, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMADataConvert,
    0x00003901, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMADataConvertResponse,
    0x00003902, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMADataRevert,
    0x00003903, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
DEFINE_GUID(IID_IRMADataRevertResponse,
    0x00003904, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  File:
 *	rmaslta.h
 *
 *  Description:
 *
 *    IRMASLTA - RMA version of slta.  Simulates a live stream from file format.
 *
 *    IRMASltaEvent - Allows events to be sent in an SLTA stream
 */
DEFINE_GUID(IID_IRMASLTA,
    0x00000D00, 0xb4c8, 0x11d0, 0x99, 0x95, 0x0, 0xa0, 0x24, 0x8d, 0xa5, 0xf0);

DEFINE_GUID(IID_IRMASltaEvent,
    0x00000D01, 0xb4c8, 0x11d0, 0x99, 0x95, 0x0, 0xa0, 0x24, 0x8d, 0xa5, 0xf0);

#endif /* _RMAIIDS_H_ */

