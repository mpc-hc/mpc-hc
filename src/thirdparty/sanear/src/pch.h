#pragma once

#ifndef NOMINMAX
#   define NOMINMAX
#endif

#include <windows.h>

#include <streams.h>

#include <avrt.h>
#include <audioclient.h>
#include <comdef.h>
#include <malloc.h>
#include <mmdeviceapi.h>
#include <process.h>

#include <FunctionDiscoveryKeys_devpkey.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <random>
#include <string>
#include <sstream>
#include <thread>

#include "Utils.h"

namespace SaneAudioRenderer
{
    _COM_SMARTPTR_TYPEDEF(IGlobalInterfaceTable, __uuidof(IGlobalInterfaceTable));

    _COM_SMARTPTR_TYPEDEF(IMMDeviceEnumerator, __uuidof(IMMDeviceEnumerator));
    _COM_SMARTPTR_TYPEDEF(IMMDeviceCollection, __uuidof(IMMDeviceCollection));
    _COM_SMARTPTR_TYPEDEF(IMMDevice, __uuidof(IMMDevice));
    _COM_SMARTPTR_TYPEDEF(IMMNotificationClient, __uuidof(IMMNotificationClient));

    _COM_SMARTPTR_TYPEDEF(IAudioClient, __uuidof(IAudioClient));
    _COM_SMARTPTR_TYPEDEF(IAudioRenderClient, __uuidof(IAudioRenderClient));
    _COM_SMARTPTR_TYPEDEF(IAudioClock, __uuidof(IAudioClock));
    _COM_SMARTPTR_TYPEDEF(IPropertyStore, __uuidof(IPropertyStore));

    _COM_SMARTPTR_TYPEDEF(IMediaSample, __uuidof(IMediaSample));
    _COM_SMARTPTR_TYPEDEF(IPropertyPageSite, __uuidof(IPropertyPageSite));
    _COM_SMARTPTR_TYPEDEF(IReferenceClock, __uuidof(IReferenceClock));
    _COM_SMARTPTR_TYPEDEF(IAMGraphStreams, __uuidof(IAMGraphStreams));
    _COM_SMARTPTR_TYPEDEF(IAMPushSource, __uuidof(IAMPushSource));
}
