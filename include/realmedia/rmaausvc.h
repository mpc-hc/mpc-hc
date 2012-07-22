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
 *  RealMedia Architecture Audio Services Interfaces.
 *
 */

#ifndef _RMAAUSVC_H_
#define _RMAAUSVC_H_

/****************************************************************************
 *
 * Forward declarations of some interfaces defined here-in.
 */
typedef _INTERFACE   IRMAAudioPlayer		    IRMAAudioPlayer;
typedef _INTERFACE   IRMAAudioPlayerResponse	    IRMAAudioPlayerResponse;
typedef _INTERFACE   IRMAAudioStream		    IRMAAudioStream;
typedef _INTERFACE   IRMAAudioStream2		    IRMAAudioStream2;
typedef _INTERFACE   IRMAAudioDevice		    IRMAAudioDevice;
typedef _INTERFACE   IRMAAudioDeviceResponse	    IRMAAudioDeviceResponse;
typedef _INTERFACE   IRMAAudioHook		    IRMAAudioHook;
typedef _INTERFACE   IRMAAudioStreamInfoResponse    IRMAAudioStreamInfoResponse;
typedef _INTERFACE   IRMAVolume			    IRMAVolume;
typedef _INTERFACE   IRMAVolumeAdviseSink	    IRMAVolumeAdviseSink;
typedef _INTERFACE   IRMADryNotification	    IRMADryNotification;
typedef _INTERFACE   IRMABuffer			    IRMABuffer;
typedef _INTERFACE   IRMAValues			    IRMAValues;

/****************************************************************************
 *
 *	Audio Services Data Structures
 */
typedef struct _RMAAudioFormat
{
    UINT16	uChannels;	/* Num. of Channels (1=Mono, 2=Stereo, etc. */
    UINT16	uBitsPerSample;	/* 8 or 16				    */
    ULONG32	ulSamplesPerSec;/* Sampling Rate			    */
    UINT16	uMaxBlockSize;	/* Max Blocksize			    */
} RMAAudioFormat;

typedef enum _AudioStreamType
{
    STREAMING_AUDIO	= 0,
    INSTANTANEOUS_AUDIO = 1,
    TIMED_AUDIO		= 2
} AudioStreamType;

typedef struct _RMAAudioData
{
    IRMABuffer*	    pData;		/* Audio data			    */ 
    ULONG32	    ulAudioTime;	/* Start time in milliseconds	    */
    AudioStreamType uAudioStreamType;
} RMAAudioData;

/****************************************************************************
 * 
 *  Interface:
 * 
 *  IRMAAudioPlayer
 * 
 *  Purpose:
 * 
 *  This interface provides access to the Audio Player services. Use this
 *  interface to create audio streams, "hook" post-mixed audio data, and to
 *  control volume levels.
 * 
 *  IID_IRMAAudioPlayer:
 * 
 *  {00000700-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAAudioPlayer, 0x00000700, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
            0xa0, 0x24, 0x40, 0x6d, 0x59);


#undef  INTERFACE
#define INTERFACE   IRMAAudioPlayer

DECLARE_INTERFACE_(IRMAAudioPlayer, IUnknown)
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
     *  IRMAAudioPlayer methods
     */
    /************************************************************************
    *  Method:
    *      IRMAAudioPlayer::CreateAudioStream
    *  Purpose:
    *		Call this to create an audio stream.
    */
    STDMETHOD(CreateAudioStream)    (THIS_
				    IRMAAudioStream** /*OUT*/ pAudioStream
				    ) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioPlayer::AddPostMixHook
    *  Purpose:
    *		Call this to hook audio data after all audio streams in this
    *		have been mixed.
    */
    STDMETHOD(AddPostMixHook)	(THIS_
				IRMAAudioHook*	    /*IN*/ pHook,
				const BOOL	    /*IN*/ bDisableWrite,
				const BOOL	    /*IN*/ bFinal
				) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioPlayer::RemovePostMixHook
    *  Purpose:
    *		Call this to remove an already added post hook.
    */
    STDMETHOD(RemovePostMixHook)    (THIS_
				    IRMAAudioHook*    /*IN*/ pHook
				    ) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioPlayer::GetAudioStreamCount
    *  Purpose:
    *		Get the number of audio streams currently active in the 
    *		audio player. Since streams can be added mid-presentation
    *		this function may return different values on different calls.
    *		If the user needs to know about all the streams as they get
    *		get added to the player, IRMAAudioStreamInfoResponse should
    *		be implemented and passed in SetStreamInfoResponse.
    */
    STDMETHOD_(UINT16,GetAudioStreamCount) (THIS) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioPlayer::GetAudioStream
    *  Purpose:
    *		Get an audio stream at position given. 
    */
    STDMETHOD_(IRMAAudioStream*,GetAudioStream) (THIS_
						UINT16	/*IN*/ uIndex
						) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioPlayer::SetStreamInfoResponse
    *  Purpose:
    *		Set a stream info response interface. A client must implement
    *		an IRMAAudioStreamInfoResponse and then call this method with
    *		the IRMAAudioStreamInfoResponse as the parameter. The audio
    *		player will call IRMAAudioStreamInfoResponse::OnStreamsReady
    *		with the total number of audio streams associated with this 
    *		audio player.
    */
    STDMETHOD(SetStreamInfoResponse)	(THIS_
				IRMAAudioStreamInfoResponse* /*IN*/ pResponse
					) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioPlayer::RemoveStreamInfoResponse
    *  Purpose:
    *		Remove stream info response that was added earlier
    */
    STDMETHOD(RemoveStreamInfoResponse) (THIS_
				IRMAAudioStreamInfoResponse* /*IN*/ pResponse
				) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioPlayer::GetAudioVolume
    *  Purpose:
    *		Get the audio player's volume interface. This volume controls
    *		the volume level of all the mixed audio streams for this 
    *		audio player.
    */
    STDMETHOD_(IRMAVolume*,GetAudioVolume) (THIS) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioPlayer::GetDeviceVolume
    *  Purpose:
    *		Get the audio device volume interface. This volume controls
    *		the audio device volume levels.
    */
    STDMETHOD_(IRMAVolume*,GetDeviceVolume) (THIS) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 * 
 *  IRMAAudioPlayerResponse
 * 
 *  Purpose:
 * 
 *  This interface provides access to the Audio Player Response. Use this 
 *  to receive audio player playback notifications. Your implementation of
 *  OnTimeSync() is called with the current audio playback time (millisecs).
 *  This interface is currently to be used ONLY by the RMA engine internally.
 * 
 *  IID_IRMAAudioPlayerResponse:
 * 
 *  {00000701-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAAudioPlayerResponse, 0x00000701, 0x901, 0x11d1, 0x8b, 0x6, 0x0,
            0xa0, 0x24, 0x40, 0x6d, 0x59);


#undef  INTERFACE
#define INTERFACE   IRMAAudioPlayerResponse

DECLARE_INTERFACE_(IRMAAudioPlayerResponse, IUnknown)
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
     *  IRMAAudioPlayerResponse methods
     */

    /************************************************************************
     *  Method:
     *      IRMAAudioPlayerResponse::OnTimeSync
     *  Purpose:
     *	    This method is called with the current audio playback time.
     */
    STDMETHOD(OnTimeSync)   (THIS_
			    ULONG32 /*IN*/ ulTimeEnd
			    ) PURE;

};


/****************************************************************************
 * 
 *  Interface:
 * 
 *      IRMAAudioStream
 * 
 *  Purpose:
 * 
 *  This interface provides access to an Audio Stream. Use this to play
 *  audio, "hook" audio stream data, and to get audio stream information.
 * 
 *  IID_IRMAAudioStream:
 * 
 *      {00000702-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAAudioStream, 0x00000702, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
            0xa0, 0x24, 0x40, 0x6d, 0x59);
#undef  INTERFACE
#define INTERFACE   IRMAAudioStream

DECLARE_INTERFACE_(IRMAAudioStream, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;
    
    /*
     *  IRMAAudioStream methods
     */

    /************************************************************************
     *  Method:
     *      IRMAAudioStream::Init
     *  Purpose:
     *	    Initialize an audio stream with the given audio format. The 
     *	    IRMAValues contains stream identification information. 
     */
    STDMETHOD(Init)	(THIS_
			const RMAAudioFormat* /*IN*/ pAudioFormat,
			IRMAValues*	/*IN*/  pValues
			) PURE;

    /************************************************************************
     *  Method:
     *      IRMAAudioStream::Write
     *  Purpose:
     *	    Write audio data to Audio Services. 
     *	    
     *	    NOTE: If the renderer loses packets and there is no loss
     *	    correction, then the renderer should write the next packet 
     *	    using a meaningful start time.  Audio Services will play 
     *      silence where packets are missing.
     */
    STDMETHOD(Write)	(THIS_
			RMAAudioData*		/*IN*/	pAudioData
			) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioStream::AddPreMixHook
    *  Purpose:
    *		Use this to "hook" audio stream data prior to the mixing.
    *		Set bDisableWrite to TRUE to prevent this audio stream data
    *		from being mixed with other audio stream data associated
    *		with this audio player.
    */
    STDMETHOD(AddPreMixHook) (THIS_
                             IRMAAudioHook*    	/*IN*/ pHook,
			     const BOOL	      	/*IN*/ bDisableWrite
			     ) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioStream::RemovePreMixHook
    *  Purpose:
    *		Use this to remove an already added "hook".
    */
    STDMETHOD(RemovePreMixHook) (THIS_
                            	IRMAAudioHook*    	/*IN*/ pHook
			     	) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioStream::AddDryNotification
    *  Purpose:
    *	    Use this to add a notification response object to get 
    *	    notifications when audio stream is running dry.
    */
    STDMETHOD(AddDryNotification)   (THIS_
                            	    IRMADryNotification* /*IN*/ pNotification
			     	    ) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioStream::GetStreamInfo
    *  Purpose:
    *		Use this to get information specific to this audio stream.
    */
    STDMETHOD_(IRMAValues*,GetStreamInfo)      	(THIS) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioStream::GetAudioVolume
    *  Purpose:
    *		Get the audio stream's volume interface. This volume controls
    *		the volume level for this audio stream.
    */
    STDMETHOD_(IRMAVolume*,GetAudioVolume) (THIS) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAAudioDevice
 * 
 *  Purpose:
 * 
 *	Object that exports audio device API
 *	This interface is currently to be used ONLY by the RMA engine 
 *	internally.
 * 
 *  IID_IRMAAudioDevice:
 * 
 *	{00000703-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAAudioDevice, 0x00000703, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
            0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAudioDevice

DECLARE_INTERFACE_(IRMAAudioDevice, IUnknown)
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
     *  IRMAAudioDevice methods
     */

    /************************************************************************
    *  Method:
    *	    IRMAAudioDevice::Open
    *  Purpose:
    *	    The caller calls this to open the audio device using the audio
    *	    format given.
    */
    STDMETHOD(Open) (THIS_
		    const RMAAudioFormat*	/*IN*/ pAudioFormat,
		    IRMAAudioDeviceResponse*	/*IN*/ pStreamResponse) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioDevice::Close
    *  Purpose:
    *		The caller calls this to close the audio device.
    */
    STDMETHOD(Close)	(THIS_
			const BOOL  /*IN*/ bFlush ) PURE;

    /************************************************************************
    *  Method:
    *	    IRMAAudioDevice::Resume
    *  Purpose:
    *	    The caller calls this to start or resume audio playback.
    */
    STDMETHOD(Resume)         (THIS) PURE;

    /************************************************************************
    *  Method:
    *	    IRMAAudioDevice::Pause
    *  Purpose:
    *	    The caller calls this to pause the audio device. If bFlush is
    *	    TRUE, any buffers in the audio device will be flushed; otherwise,
    *	    the buffers are played.
    */
    STDMETHOD(Pause)         (THIS) PURE;

    /************************************************************************
    *  Method:
    *	    IRMAAudioDevice::Write
    *  Purpose:
    *	    The caller calls this to write an audio buffer.
    */
    STDMETHOD(Write)         (THIS_
			     const RMAAudioData* /*IN*/ pAudioData) PURE;

    /************************************************************************
    *  Method:
    *      IRMAAudioDevice::InitVolume
    *  Purpose:
    *	    The caller calls this to inform the audio stream of the client's
    *	    volume range. The audio stream maps the client's volume range
    *	    into the audio device volume range. 
    *	    NOTE: This function returns TRUE if volume is supported by this 
    *	    audio device.
    */
    STDMETHOD_(BOOL,InitVolume)  (THIS_
				 const UINT16	/*IN*/ uMinVolume,
				 const UINT16	/*IN*/ uMaxVolume) PURE;

    /************************************************************************
    *  Method:
    *	    IRMAAudioDevice::SetVolume
    *  Purpose:
    *	    The caller calls this to set the audio device volume level.
    */
    STDMETHOD(SetVolume)         (THIS_
				 const UINT16    /*IN*/ uVolume) PURE;

    /************************************************************************
    *  Method:
    *	    IRMAAudioDevice::GetVolume
    *  Purpose:
    *	    The caller calls this to get the audio device volume level.
    */
    STDMETHOD_(UINT16,GetVolume) (THIS) PURE;

    /************************************************************************
    *  Method:
    *	    IRMAAudioDevice::Reset
    *  Purpose:
    *	    The caller calls this to reset the audio device.
    */
    STDMETHOD(Reset)		(THIS) PURE;

    /************************************************************************
    *  Method:
    *	    IRMAAudioDevice::Drain
    *  Purpose:
    *	    The caller calls this to drain the audio device.
    */
    STDMETHOD(Drain)		(THIS) PURE;

    /************************************************************************
    *  Method:
    *	    IRMAAudioDevice::CheckFormat
    *  Purpose:
    *	    The caller calls this to check the input format with the
    *	    audio device format.
    */
    STDMETHOD(CheckFormat)  (THIS_
			    const RMAAudioFormat* /*IN*/ pAudioFormat ) PURE;

    /************************************************************************
    *  Method:
    *	    IRMAAudioDevice::GetCurrentAudioTime
    *  Purpose:
    *	    The caller calls this to get current system audio time.
    */
    STDMETHOD(GetCurrentAudioTime)  (THIS_
				    REF(ULONG32) /*OUT*/ ulCurrentTime) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *	IRMAAudioDeviceResponse
 * 
 *  Purpose:
 * 
 *	Object that exports audio device Response API
 *	This interface is currently to be used ONLY by the RMA engine 
 *	internally.
 * 
 *  IID_IRMAAudioDeviceResponse:
 * 
 *  {00000704-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAAudioDeviceResponse, 0x00000704, 0x901, 0x11d1, 0x8b, 0x6, 
	    0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAudioDeviceResponse

DECLARE_INTERFACE_(IRMAAudioDeviceResponse, IUnknown)
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
     *  IRMAAudioDeviceResponse methods
     */

    /************************************************************************
     *  Method:
     *      IRMAAudioDeviceResponse::OnTimeSync
     *  Purpose:
     *      Notification interface provided by users of the IRMAAudioDevice
     *      interface. This method is called by the IRMAAudioDevice when
     *      audio playback occurs.
     */
    STDMETHOD(OnTimeSync)         (THIS_
                    		ULONG32         		/*IN*/ ulTimeEnd) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *  IRMAAudioHook
 * 
 *  Purpose:
 * 
 *  Clients must implement this interface to access pre- or post-mixed 
 *  audio data. Use this interface to get post processed audio buffers and
 *  their associated audio format.
 *
 *  IID_IRMAAudioHook:
 * 
 *  {00000705-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAAudioHook, 0x00000705, 0x901, 0x11d1, 0x8b, 0x6, 0x0,
            0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAudioHook

DECLARE_INTERFACE_(IRMAAudioHook, IUnknown)
{
    /*
     *  IUnknown methods!
     */
    STDMETHOD(QueryInterface)		(THIS_
                    REFIID riid,
                    void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *  IRMAAudioHook methods
     */
    /************************************************************************
     *  Method:
     *      IRMAAudioHook::OnInit
     *  Purpose:
     *      Audio Services calls OnInit() with the audio data format of the
     *	    audio data that will be provided in the OnBuffer() method.
     */
    STDMETHOD(OnInit)		(THIS_
                    		RMAAudioFormat*	/*IN*/ pFormat) PURE;

    /************************************************************************
     *  Method:
     *      IRMAAudioHook::OnBuffer
     *  Purpose:
     *      Audio Services calls OnBuffer() with audio data packets. The
     *	    renderer should not modify the data in the IRMABuffer part of
     *	    pAudioInData.  If the renderer wants to write a modified
     *	    version of the data back to Audio Services, then it should 
     *	    create its own IRMABuffer, modify the data and then associate 
     *	    this buffer with the pAudioOutData->pData member.
     */
    STDMETHOD(OnBuffer)		(THIS_
                    		RMAAudioData*	/*IN*/   pAudioInData,
                    		RMAAudioData*	/*OUT*/  pAudioOutData) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *  IRMAAudioStreamInfoResponse
 * 
 *  Purpose:
 * 
 *  Clients must implement this interface when interested in receiving
 *  notification of the total number of streams associated with this
 *  audio player.
 *
 *  IID_IRMAAudioStreamInfoResponse:
 * 
 *  {00000706-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAAudioStreamInfoResponse, 0x00000706, 0x901, 0x11d1, 0x8b, 0x6, 0x0,
            0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAudioStreamInfoResponse

DECLARE_INTERFACE_(IRMAAudioStreamInfoResponse, IUnknown)
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
     *  IRMAAudioStreamInfoResponse methods
     */

    /************************************************************************
     *  Method:
     *      IRMAAudioStreamInfoResponse::OnStream
     *  Purpose:
     *	    The client implements this to get notification of streams 
     *	    associated with this player. Use 
     *	    AudioPlayer::SetStreamInfoResponse() to register your 
     *	    implementation with the AudioPlayer. Once player has been 
     *	    initialized, it will call OnStream() multiple times to pass all 
     *	    the streams. Since a stream can be added mid-presentation, 
     *	    IRMAAudioStreamInfoResponse object should be written to handle 
     *	    OnStream() in the midst of the presentation as well.
     */
    STDMETHOD(OnStream) (THIS_
			IRMAAudioStream* /*IN*/ pAudioStream) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *  IRMAVolume
 * 
 *  Purpose:
 * 
 *  This interface provides access to Audio Services volume control. Use this
 *  interface to get, set, or receive notifications of volume changes. Audio
 *  Services implements IRMAVolume for IRMAAudioPlayer, IRMAAudioStream and 
 *  for the audio device. Clients can use the IRMAVolume interface to get/set
 *  volume levels of each audio stream, to get/set volume levels for the 
 *  audio player's mixed data, or to get/set the volume levels of the audio 
 *  device. See AudioStream::GetStreamVolume() (TBD), AudioPlayer::
 *  GetAudioVolume() and AudioPlayer::GetDeviceVolume().
 *
 *  IID_IRMAVolume:
 * 
 *  {00000707-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAVolume, 0x00000707, 0x901, 0x11d1, 0x8b, 0x6, 0x0,
            0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAVolume

DECLARE_INTERFACE_(IRMAVolume, IUnknown)
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
     *  IRMAVolume methods
     */
    /************************************************************************
     *  Method:
     *      IRMAVolume::SetVolume
     *  Purpose:
     *	    Call this to set the volume level.
     */
    STDMETHOD(SetVolume) (THIS_
                         const	UINT16	/*IN*/ uVolume ) PURE;

    /************************************************************************
     *  Method:
     *      IRMAVolume::GetVolume
     *  Purpose:
     *	    Call this to get the current volume level.
     */
    STDMETHOD_(UINT16,GetVolume)   (THIS) PURE;

     /************************************************************************
     *  Method:
     *      IRMAVolume::SetMute
     *  Purpose:
     *	    Call this to mute the volume.
     */
    STDMETHOD(SetMute)   (THIS_
                         const	BOOL	/*IN*/ bMute ) PURE;

     /************************************************************************
     *  Method:
     *      IRMAVolume::GetMute
     *  Purpose:
     *	    Call this to determine if the volume is muted.
     *	  
     */
    STDMETHOD_(BOOL,GetMute)       (THIS) PURE;

     /************************************************************************
     *  Method:
     *      IRMAVolume::AddAdviseSink
     *  Purpose:
     *	    Call this to register an IRMAVolumeAdviseSink. The advise sink
     *	    methods: OnVolumeChange() and OnMuteChange() are called when
     *	    ever IRMAVolume::SetVolume() and IRMAVolume::SetMute() are
     *	    called.
     */
    STDMETHOD(AddAdviseSink)	(THIS_
				 IRMAVolumeAdviseSink* /*IN*/	pSink
				) PURE;

     /************************************************************************
     *  Method:
     *      IRMAVolume::RemoveAdviseSink
     *  Purpose:
     *	    Call this to unregister an IRMAVolumeAdviseSink. Use this when
     *	    you are no longer interested in receiving volume or mute change
     *	    notifications.
     */
    STDMETHOD(RemoveAdviseSink)	(THIS_
				 IRMAVolumeAdviseSink* /*IN*/	pSink
				) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *  IRMAVolumeAdviseSink
 * 
 *  Purpose:
 * 
 *  This interface provides access to notifications of volume changes. A 
 *  client must implement this interface if they are interested in receiving 
 *  notifications of volume level changes or mute state changes. A client must 
 *  register their volume advise sink using IRMAVolume::AddAdviseSink().
 *  See the IRMAVolume interface.
 * 
 *  IID_IRMAVolumeAdviseSink:
 * 
 *  {00000708-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAVolumeAdviseSink, 0x00000708, 0x901, 0x11d1, 0x8b, 0x6, 0x0,
            0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAVolumeAdviseSink

DECLARE_INTERFACE_(IRMAVolumeAdviseSink, IUnknown)
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
     *  IRMAVolumeAdviseSink methods
     */

    /************************************************************************
     *  Method:
     *      IRMAVolumeAdviseSink::OnVolumeChange
     *  Purpose:
     *	    This interface is called whenever the associated IRMAVolume
     *	    SetVolume() is called.
     */
    STDMETHOD(OnVolumeChange)	(THIS_ 
				const UINT16 uVolume
				) PURE;

    /************************************************************************
     *  Method:
     *      IRMAVolumeAdviseSink::OnMuteChange
     *  Purpose:
     *	    This interface is called whenever the associated IRMAVolume
     *	    SetMute() is called.
     *    
     */
    STDMETHOD(OnMuteChange)     (THIS_
				const BOOL bMute
				) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *  IRMADryNotification
 * 
 *  Purpose:
 * 
 *  Audio Renderer should implement this if it needs notification when the 
 *  audio stream is running dry. 
 *
 *  IID_IRMADryNotification:
 * 
 *  {00000709-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMADryNotification, 0x00000709, 0x901, 0x11d1, 0x8b, 0x6, 0x0,
            0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMADryNotification

DECLARE_INTERFACE_(IRMADryNotification, IUnknown)
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
     *  IRMADryNotification methods
     */

    /************************************************************************
     *  Method:
     *      IRMADryNotification::OnDryNotification
     *  Purpose:
     *	    This function is called when it is time to write to audio device 
     *	    and there is not enough data in the audio stream. The renderer can
     *	    then decide to add more data to the audio stream. This should be 
     *	    done synchronously within the call to this function.
     *	    It is OK to not write any data. Silence will be played instead.
     */
    STDMETHOD(OnDryNotification)    (THIS_
				    UINT32 /*IN*/ ulCurrentStreamTime,
				    UINT32 /*IN*/ ulMinimumDurationRequired
				    ) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *  IRMAAudioDeviceManager
 * 
 *  Purpose:
 * 
 *  Allows the default audio device to be replaced.
 *
 *  IID_IRMAAudioDeviceManager:
 * 
 *  {0000070A-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAAudioDeviceManager, 0x0000070A, 0x901, 0x11d1, 0x8b, 0x6, 0x0,
            0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAudioDeviceManager

DECLARE_INTERFACE_(IRMAAudioDeviceManager, IUnknown)
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
     *  IRMAAudioDeviceManager methods
     */

    /**********************************************************************
     *  Method:
     *      IRMAAudioDeviceManager::Replace
     *  Purpose:
     *  This is used to replace the default implementation of the audio
     *  device by the given audio device interface. 
     */
    STDMETHOD(Replace)         (THIS_
		    IRMAAudioDevice*    /*IN*/ pAudioDevice) PURE;

    /**********************************************************************
     *  Method:
     *      IRMAAudioDeviceManager::Remove
     *  Purpose:
     *  This is used to remove the audio device given to the manager in
     *  the earlier call to Replace.
     */
    STDMETHOD(Remove)         (THIS_
		    IRMAAudioDevice*    /*IN*/ pAudioDevice) PURE;

    /************************************************************************
    *  Method:
    *   IRMAAudioDeviceManager::AddFinalHook
    *  Purpose:
    *	One last chance to modify data being written to the audio device.
    *	This hook allows the user to change the audio format that
    *   is to be written to the audio device. This can be done in call
    *   to OnInit() in IRMAAudioHook.
    */
    STDMETHOD(SetFinalHook)	(THIS_
				IRMAAudioHook*	    /*IN*/ pHook
				) PURE;

    /************************************************************************
    *  Method:
    *   IRMAAudioDeviceManager::RemoveFinalHook
    *  Purpose:
    *	Remove final hook
    */
    STDMETHOD(RemoveFinalHook)	(THIS_
				IRMAAudioHook*    /*IN*/ pHook
				) PURE;

   /************************************************************************
    *  Method:
    *   IRMAAudioDeviceManager::GetAudioFormat
    *  Purpose:
    *	Returns the audio format in which the audio device is opened.
    *	This function will fill in the pre-allocated RMAAudioFormat 
    *	structure passed in.
    */
    STDMETHOD(GetAudioFormat)   (THIS_
			        RMAAudioFormat*	/*IN/OUT*/pAudioFormat) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *      IRMAAudioCrossFade
 * 
 *  Purpose:
 *
 *  This interface can be used to cross-fade two audio streams. It is exposed 
 *  by IRMAAudioPlayer
 * 
 *  IID_IRMAAudioCrossFade:
 * 
 *  {0000070B-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAAudioCrossFade, 0x0000070B, 0x901, 0x11d1, 0x8b, 0x6, 0x0,
            0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAudioCrossFade

DECLARE_INTERFACE_(IRMAAudioCrossFade, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;
    
    /*
     *  IRMAAudioCrossFade methods
     */

    /************************************************************************
     *  Method:
     *      IRMAAudioCrossFade::CrossFade
     *  Purpose:
     *	    Cross-fade two audio streams.
     *	    pStreamFrom		    - Stream to be cross faded from
     *	    pStreamTo		    - Stream to be cross faded to
     *	    ulFromCrossFadeStartTime- "From" Stream time when cross fade is 
     *				      to be started
     *	    ulToCrossFadeStartTime  - "To" Stream time when cross fade is to 
     *				      be started
     *	    ulCrossFadeDuration	    - Duration over which cross-fade needs
     *				      to be done
     *	    
     */
    STDMETHOD(CrossFade)	(THIS_
				IRMAAudioStream* pStreamFrom,
				IRMAAudioStream* pStreamTo,
				UINT32		 ulFromCrossFadeStartTime,
				UINT32		 ulToCrossFadeStartTime,
				UINT32		 ulCrossFadeDuration) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *      IRMAAudioStream2
 * 
 *  Purpose:
 *
 *  This interface contains some last-minute added audio stream functions
 * 
 *  IID_IRMAAudioStream2:
 * 
 *  {0000070C-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAAudioStream2, 0x0000070C, 0x901, 0x11d1, 0x8b, 0x6, 0x0,
            0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAudioStream2

DECLARE_INTERFACE_(IRMAAudioStream2, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;
    
    /*
     *  IRMAAudioStream2 methods
     */
   /************************************************************************
    *  Method:
    *      IRMAAudioStream2::RemoveDryNotification
    *  Purpose:
    *	    Use this to remove itself from the notification response object
    *	    during the stream switching.
    */
    STDMETHOD(RemoveDryNotification)   (THIS_
				   IRMADryNotification* /*IN*/ pNotification
			     		) PURE;

   /************************************************************************
    *  Method:
    *      IRMAAudioStream2::GetAudioFormat
    *  Purpose:
    *	    Returns the input audio format of the data written by the 
    *	    renderer. This function will fill in the pre-allocated 
    *	    RMAAudioFormat structure passed in.
    */
    STDMETHOD(GetAudioFormat)   (THIS_
			        RMAAudioFormat*	/*IN/OUT*/pAudioFormat) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *      IRMAAudioPushdown
 * 
 *  Purpose:
 *
 *  This interface can be used to setup the audio pushdown time.
 * 
 *  IID_IRMAAudioPushdown:
 * 
 *  {0000070D-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAAudioPushdown, 0x0000070D, 0x901, 0x11d1, 0x8b, 0x6, 0x0,
            0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAudioPushdown

DECLARE_INTERFACE_(IRMAAudioPushdown, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;
    
    /*
     *  IRMAAudioPushdown methods
     */
   /************************************************************************
    *  Method:
    *      IRMAAudioPushdown::SetAudioPushdown
    *  Purpose:
    *	    Use this to set the minimum audio pushdown value in ms.
    *	    This is the amount of audio data that is being written 
    *	    to the audio device before starting playback.
    */
    STDMETHOD(SetAudioPushdown)   (THIS_
				   UINT32 /*IN*/ ulAudioPushdown
			     	  ) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *      IRMAAudioHookManager
 * 
 *  Purpose:
 *
 *  This interface can be used to add a hook at the audio device layer.
 * 
 *  IID_IRMAAudioHookManager:
 * 
 *  {0000070E-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAAudioHookManager, 0x0000070E, 0x901, 0x11d1, 0x8b, 0x6, 0x0,
            0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAudioHookManager

DECLARE_INTERFACE_(IRMAAudioHookManager, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;
    
   /*
    *  IRMAAudioHookManager methods
    */
   /************************************************************************
    *  Method:
    *      IRMAAudioHookManager::AddHook
    *  Purpose:
    *	    Use this to add a hook 
    */
    STDMETHOD(AddHook)   (THIS_
			  IRMAAudioHook* /*IN*/ pHook
			  ) PURE;

   /************************************************************************
    *  Method:
    *      IRMAAudioHookManager::RemoveHook
    *  Purpose:
    *	    Use this to remove a hook 
    */
    STDMETHOD(RemoveHook) (THIS_
			  IRMAAudioHook* /*IN*/ pHook
			  ) PURE;
};

#endif  /* _RMAAUSVC_H_ */
