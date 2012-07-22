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
 *  Group Interface.
 *
 */

#ifndef _IRMAGROUP_H_
#define _IRMAGROUP_H_

/****************************************************************************
 *
 * Forward declarations of some interfaces defined/used here-in.
 */
typedef _INTERFACE   IRMAGroup			    IRMAGroup;
typedef _INTERFACE   IRMAGroupManager		    IRMAGroupManager;
typedef _INTERFACE   IRMAGroupSink		    IRMAGroupSink;
typedef _INTERFACE   IRMAValues			    IRMAValues;


/****************************************************************************
 * 
 *  Interface:
 * 
 *  IRMAGroup
 * 
 *  Purpose:
 * 
 * 
 *  IID_IRMAGroup:
 * 
 *  {0x00002400-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAGroup, 0x00002400, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
            0xa0, 0x24, 0x40, 0x6d, 0x59);


#undef  INTERFACE
#define INTERFACE   IRMAGroup

DECLARE_INTERFACE_(IRMAGroup, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)       (THIS_
                    REFIID riid,
                    void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)      (THIS) PURE;

    STDMETHOD_(ULONG,Release)     (THIS) PURE;

    /*
     *  IRMAGroup methods
     */
    /************************************************************************
    *  Method:
    *      IRMAGroup::SetGroupProperties
    *  Purpose:
    *		Set any group specific information like Title Author 
    *		Copyright etc. 
    */
    STDMETHOD(SetGroupProperties)   (THIS_
				     IRMAValues*  /*IN*/ pProperties) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroup::GetGroupProperties
    *  Purpose:
    *		Get any group specific information. May return NULL.
    */
    STDMETHOD_(IRMAValues*, GetGroupProperties)   (THIS) PURE;
    /************************************************************************
    *  Method:
    *      IRMAGroup::GetTrackCount
    *  Purpose:
    *		Get the number of tracks within this group.
    */
    STDMETHOD_(UINT16,GetTrackCount)    (THIS) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroup::GetTrack
    *  Purpose:
    *		Get ith track in this group
    */
    STDMETHOD(GetTrack)	(THIS_
			UINT16 	    /*IN*/ uTrackIndex,
			REF(IRMAValues*)  /*OUT*/ pTrack) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroup::AddTrack
    *  Purpose:
    *		Add Tracks to the group.
    */
    STDMETHOD(AddTrack)	(THIS_
			IRMAValues*	/*IN*/ pTrack) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroup::RemoveTrack
    *  Purpose:
    *		Remove an already added track
    */
    STDMETHOD(RemoveTrack)  (THIS_
			    UINT16	/*IN*/ uTrackIndex) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *  IRMAGroupManager
 * 
 *  Purpose:
 * 
 * 
 *  IID_IRMAGroupManager:
 * 
 *  {0x00002401-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAGroupManager, 0x00002401, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
            0xa0, 0x24, 0x40, 0x6d, 0x59);


#undef  INTERFACE
#define INTERFACE   IRMAGroupManager

DECLARE_INTERFACE_(IRMAGroupManager, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)       (THIS_
                    REFIID riid,
                    void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)      (THIS) PURE;

    STDMETHOD_(ULONG,Release)     (THIS) PURE;

    /*
     *  IRMAGroupManager methods
     */

    /************************************************************************
    *  Method:
    *      IRMAGroupManager::CreateGroup
    *  Purpose:
    *		Create a group
    */
    STDMETHOD(CreateGroup)    (REF(IRMAGroup*) pGroup) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroupManager::GetGroupCount
    *  Purpose:
    *		Get the number of groups within the presentation.
    */
    STDMETHOD_(UINT16,GetGroupCount)    (THIS) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroupManager::GetGroup
    *  Purpose:
    *		Get ith group in the presentation
    */
    STDMETHOD(GetGroup)	(THIS_
			UINT16 		  /*IN*/  uGroupIndex,
			REF(IRMAGroup*)  /*OUT*/ pGroup) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroupManager::SetCurrentGroup
    *  Purpose:
    *		Play this group in the presentation.
    */
    STDMETHOD(SetCurrentGroup)	(THIS_
				UINT16 	    /*IN*/ uGroupIndex) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroupMgr::GetCurrentGroup
    *  Purpose:
    *		Get the current group index
    */
    STDMETHOD(GetCurrentGroup)	(THIS_
				REF(UINT16) /*OUT*/ uGroupIndex) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroupManager::AddGroup
    *  Purpose:
    *		Add a group to the presentation.
    */
    STDMETHOD(AddGroup)	(THIS_
			IRMAGroup*	    /*IN*/ pGroup) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroupManager::RemoveGroup
    *  Purpose:
    *		Remove an already added group
    */
    STDMETHOD(RemoveGroup)  (THIS_
			    UINT16 	/*IN*/ uGroupIndex) PURE;


    /************************************************************************
    *  Method:
    *      IRMAGroupManager::AddSink
    *  Purpose:
    *		Add a sink to get notifications about any tracks or groups
    *		being added to the presentation.
    */
    STDMETHOD(AddSink)	(THIS_
			IRMAGroupSink* /*IN*/ pGroupSink) PURE;


    /************************************************************************
    *  Method:
    *      IRMAGroupManager::RemoveSink
    *  Purpose:
    *		Remove Sink
    */
    STDMETHOD(RemoveSink)   (THIS_
			    IRMAGroupSink* /*IN*/ pGroupSink) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroup::GetPresentationProperties
    *  Purpose:
    *		Get any presentation information. May return NULL.
    */
    STDMETHOD_(IRMAValues*, GetPresentationProperties)   (THIS) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroup::SetPresentationProperties
    *  Purpose:
    *		Set any presentation information like Title Author 
    *		Copyright etc. 
    */
    STDMETHOD(SetPresentationProperties)   (THIS_
					    IRMAValues*  /*IN*/ pProperties) PURE;
   
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *  IRMAGroupSink
 * 
 *  Purpose:
 * 
 * 
 *  IID_IRMAGroupSink:
 * 
 *  {0x00002402-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAGroupSink, 0x00002402, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
            0xa0, 0x24, 0x40, 0x6d, 0x59);


#undef  INTERFACE
#define INTERFACE   IRMAGroupSink

DECLARE_INTERFACE_(IRMAGroupSink, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)       (THIS_
                    REFIID riid,
                    void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)      (THIS) PURE;

    STDMETHOD_(ULONG,Release)     (THIS) PURE;

    /*
     *  IRMAGroupSink methods
     */
    /************************************************************************
    *  Method:
    *      IRMAGroupSink::GroupAdded
    *  Purpose:
    *		Notification of a new group being added to the presentation.
    */
    STDMETHOD(GroupAdded)    (THIS_
			    UINT16 	    /*IN*/ uGroupIndex,
			    IRMAGroup*	    /*IN*/ pGroup) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroupSink::GroupRemoved
    *  Purpose:
    *		Notification of a group being removed from the presentation.
    */
    STDMETHOD(GroupRemoved)    (THIS_
				UINT16 	    /*IN*/ uGroupIndex,
				IRMAGroup*  /*IN*/ pGroup) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroupSink::AllGroupsRemoved
    *  Purpose:
    *		Notification that all groups have been removed from the 
    *		current presentation.
    */
    STDMETHOD(AllGroupsRemoved)  (THIS) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroupSink::TrackAdded
    *  Purpose:
    *		Notification of a new track being added to a group.
    */
    STDMETHOD(TrackAdded)  (THIS_
			    UINT16 	    /*IN*/ uGroupIndex,
			    UINT16 	    /*IN*/ uTrackIndex,
			    IRMAValues*	    /*IN*/ pTrack) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroupSink::TrackRemoved
    *  Purpose:
    *		Notification of a track being removed from a group.
    */
    STDMETHOD(TrackRemoved)    (THIS_
				UINT16 		/*IN*/ uGroupIndex,
				UINT16 		/*IN*/ uTrackIndex,
				IRMAValues*	/*IN*/ pTrack) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroupSink::TrackStarted
    *  Purpose:
    *		Notification of a track being started (to get duration, for
    *		instance...)
    */
    STDMETHOD(TrackStarted)	(THIS_
				UINT16	    /*IN*/ uGroupIndex,
				UINT16	    /*IN*/ uTrackIndex,
				IRMAValues* /*IN*/ pTrack) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroupSink::TrackStopped
    *  Purpose:
    *		Notification of a track being stopped
    *
    */
    STDMETHOD(TrackStopped)	(THIS_
				UINT16	    /*IN*/ uGroupIndex,
				UINT16	    /*IN*/ uTrackIndex,
				IRMAValues* /*IN*/ pTrack) PURE;

    /************************************************************************
    *  Method:
    *      IRMAGroupSink::CurrentGroupSet
    *  Purpose:
    *		This group is being currently played in the presentation.
    */
    STDMETHOD(CurrentGroupSet)	(THIS_
				UINT16 	    /*IN*/ uGroupIndex,
				IRMAGroup*  /*IN*/ pGroup) PURE;
};




#endif /*_IRMAGROUP_H_*/
