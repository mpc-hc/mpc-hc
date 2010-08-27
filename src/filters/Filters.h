/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include "./reader/CDDAReader/CDDAReader.h"
#include "./reader/CDXAReader/CDXAReader.h"
#include "./reader/UDPReader/UDPReader.h"
#include "./reader/VTSReader/VTSReader.h"
#include "./source/D2VSource/D2VSource.h"
#include "./source/DTSAC3Source/DTSAC3Source.h"
#include "./source/FLICSource/FLICSource.h"
#include "./source/FlacSource/FlacSource.h"
#include "./source/ShoutcastSource/ShoutcastSource.h"
#include "./source/SubtitleSource/SubtitleSource.h"
#include "./switcher/AudioSwitcher/AudioSwitcher.h"
#include "./transform/AVI2AC3Filter/AVI2AC3Filter.h"
#include "./transform/BufferFilter/BufferFilter.h"
#include "./transform/DeCSSFilter/DeCSSFilter.h"
#include "./transform/Mpeg2DecFilter/Mpeg2DecFilter.h"
#include "./transform/MpaDecFilter/MpaDecFilter.h"
#include "./transform/MPCVideoDec/MPCVideoDecFilter.h"
#include "./muxer/WavDest/WavDest.h"
#include "./muxer/DSMMuxer/DSMMuxer.h"
#include "./muxer/MatroskaMuxer/MatroskaMuxer.h"
#include "./parser/StreamDriveThru/StreamDriveThru.h"
#include "./parser/MatroskaSplitter/MatroskaSplitter.h"
#include "./parser/RealMediaSplitter/RealMediaSplitter.h"
#include "./parser/AviSplitter/AviSplitter.h"
#include "./parser/RoQSplitter/RoQSplitter.h"
#include "./parser/OggSplitter/OggSplitter.h"
#include "./parser/MpegSplitter/MpegSplitter.h"
#include "./parser/MpaSplitter/MpaSplitter.h"
#include "./parser/DSMSplitter/DSMSplitter.h"
#include "./parser/MP4Splitter/MP4Splitter.h"
#include "./parser/FLVSplitter/FLVSplitter.h"
#include "./parser/SSFSplitter/SSFSplitter.h"
#include "./renderer/MpcAudioRenderer/MpcAudioRenderer.h"
#include "./renderer/SyncClock/SyncClock.h"
