#ifndef f_VD2_PLUGIN_VDAUDIOFILT_H
#define f_VD2_PLUGIN_VDAUDIOFILT_H

///////////////////////////////////////////////////////////////////////////
//
//	Audio filter support

struct VDAudioFilterDefinition;
struct VDXWaveFormat;
struct VDPluginCallbacks;

enum {
	kVDPlugin_AudioAPIVersion		= 2
};

struct VDAudioFilterPin {
	unsigned			mGranularity;			// Block size a filter reads/writes this pin.
	unsigned			mDelay;					// Delay in samples on this input.
	unsigned			mBufferSize;			// The size, in samples, of the buffer.
	unsigned			mCurrentLevel;			// The number of samples currently in the buffer.
	sint64				mLength;				// Approximate length of this stream in us.
	const VDXWaveFormat *mpFormat;
	bool				mbVBR;
	bool				mbEnded;
	char				_pad[2];
	void				*mpBuffer;
	unsigned			mSamplesWritten;		// The number of samples just written to the buffer.
	unsigned			mAvailSpace;			// Available room pointed to by mpBuffer (output pins only).

	uint32 (VDAPIENTRY *mpReadProc)(VDAudioFilterPin *pPin, void *dst, uint32 samples, bool bAllowFill, int format);

	// These helpers are non-virtual inlines and are compiled into filters.
	uint32 Read(void *dst, uint32 samples, bool bAllowFill, int format) {
		return mpReadProc(this, dst, samples, bAllowFill, format);
	}
};

struct VDAudioFilterContext;

struct VDAudioFilterCallbacks {
	VDXWaveFormat *(VDAPIENTRY *AllocPCMWaveFormat)(unsigned sampling_rate, unsigned channels, unsigned bits, bool bFloat);
	VDXWaveFormat *(VDAPIENTRY *AllocCustomWaveFormat)(unsigned extra_size);
	VDXWaveFormat *(VDAPIENTRY *CopyWaveFormat)(const VDXWaveFormat *);
	void (VDAPIENTRY *FreeWaveFormat)(const VDXWaveFormat *);
	void (VDAPIENTRY *Wake)(const VDAudioFilterContext *pContext);
};

struct VDAudioFilterContext {
	void *mpFilterData;
	VDAudioFilterPin	**mpInputs;
	VDAudioFilterPin	**mpOutputs;
	IVDPluginCallbacks *mpServices;
	const VDAudioFilterCallbacks *mpAudioCallbacks;
	const VDAudioFilterDefinition *mpDefinition;
	uint32	mAPIVersion;
	uint32	mInputSamples;			// Number of input samples available on all pins.
	uint32	mInputGranules;			// Number of input granules available on all pins.
	uint32	mInputsEnded;			// Number of inputs that have ended.
	uint32	mOutputSamples;			// Number of output sample spaces available on all pins.
	uint32	mOutputGranules;		// Number of output granule spaces available on all pins.
	uint32	mCommonSamples;			// Number of input samples and output sample spaces.
	uint32	mCommonGranules;		// Number of input and output granules.
};

// This structure is intentionally identical to WAVEFORMATEX, with one
// exception -- mExtraSize is *always* present, even for PCM.

struct VDXWaveFormat {
	enum { kTagPCM = 1 };

	uint16		mTag;
	uint16		mChannels;
	uint32		mSamplingRate;
	uint32		mDataRate;
	uint16		mBlockSize;
	uint16		mSampleBits;
	uint16		mExtraSize;
};

enum {
	kVFARun_OK				= 0,
	kVFARun_Finished		= 1,
	kVFARun_InternalWork	= 2,

	kVFAPrepare_OK			= 0,
	kVFAPrepare_BadFormat	= 1
};

enum {
	kVFARead_Native			= 0,
	kVFARead_PCM8			= 1,
	kVFARead_PCM16			= 2,
	kVFARead_PCM32F			= 3
};

typedef void *		(VDAPIENTRY *VDAudioFilterExtProc			)(const VDAudioFilterContext *pContext, const char *pInterfaceName);
typedef uint32		(VDAPIENTRY *VDAudioFilterRunProc			)(const VDAudioFilterContext *pContext);
typedef sint64		(VDAPIENTRY *VDAudioFilterSeekProc			)(const VDAudioFilterContext *pContext, sint64 microsecs);
typedef uint32		(VDAPIENTRY *VDAudioFilterPrepareProc		)(const VDAudioFilterContext *pContext);
typedef void		(VDAPIENTRY *VDAudioFilterStartProc			)(const VDAudioFilterContext *pContext);
typedef void		(VDAPIENTRY *VDAudioFilterStopProc			)(const VDAudioFilterContext *pContext);
typedef void		(VDAPIENTRY *VDAudioFilterInitProc			)(const VDAudioFilterContext *pContext);
typedef void		(VDAPIENTRY *VDAudioFilterDestroyProc		)(const VDAudioFilterContext *pContext);
typedef unsigned	(VDAPIENTRY *VDAudioFilterSuspendProc		)(const VDAudioFilterContext *pContext, void *dst, unsigned size);
typedef void		(VDAPIENTRY *VDAudioFilterResumeProc		)(const VDAudioFilterContext *pContext, const void *src, unsigned size);
typedef unsigned	(VDAPIENTRY *VDAudioFilterGetParamProc		)(const VDAudioFilterContext *pContext, unsigned idx, void *dst, unsigned size);
typedef void		(VDAPIENTRY *VDAudioFilterSetParamProc		)(const VDAudioFilterContext *pContext, unsigned idx, const void *src, unsigned variant_count);
typedef bool		(VDAPIENTRY *VDAudioFilterConfigProc		)(const VDAudioFilterContext *pContext, struct HWND__ *hwnd);

enum {
	kVFAF_Zero				= 0,
	kVFAF_HasConfig			= 1,				// Filter has a configuration dialog.
	kVFAF_SerializedIO		= 2,				// Filter must execute in the serialized I/O thread.

	kVFAF_Max				= 0xFFFFFFFF,
};

struct VDAudioFilterVtbl {
	uint32								mSize;
	VDAudioFilterDestroyProc			mpDestroy;
	VDAudioFilterPrepareProc			mpPrepare;
	VDAudioFilterStartProc				mpStart;
	VDAudioFilterStopProc				mpStop;
	VDAudioFilterRunProc				mpRun;
	VDAudioFilterSeekProc				mpSeek;
	VDAudioFilterSuspendProc			mpSuspend;
	VDAudioFilterResumeProc				mpResume;
	VDAudioFilterGetParamProc			mpGetParam;
	VDAudioFilterSetParamProc			mpSetParam;
	VDAudioFilterConfigProc				mpConfig;
	VDAudioFilterExtProc				mpExt;
};

struct VDAudioFilterDefinition {
	uint32							mSize;				// size of this structure in bytes
	uint32							mFlags;

	uint32							mFilterDataSize;
	uint32							mInputPins;
	uint32							mOutputPins;

	const VDXPluginConfigEntry		*mpConfigInfo;

	VDAudioFilterInitProc			mpInit;
	const VDAudioFilterVtbl			*mpVtbl;
};

#endif
