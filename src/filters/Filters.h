/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "muxer/DSMMuxer/DSMMuxer.h"
#include "muxer/MatroskaMuxer/MatroskaMuxer.h"
#include "muxer/WavDest/WavDest.h"
#include "parser/AviSplitter/AviSplitter.h"
#include "parser/DSMSplitter/DSMSplitter.h"
#include "parser/FLVSplitter/FLVSplitter.h"
#include "parser/MatroskaSplitter/MatroskaSplitter.h"
#include "parser/MP4Splitter/MP4Splitter.h"
#include "parser/MpaSplitter/MpaSplitter.h"
#include "parser/MpegSplitter/MpegSplitter.h"
#include "parser/OggSplitter/OggSplitter.h"
#include "parser/RealMediaSplitter/RealMediaSplitter.h"
#include "parser/SSFSplitter/SSFSplitter.h"
#include "parser/StreamDriveThru/StreamDriveThru.h"
#include "reader/CDDAReader/CDDAReader.h"
#include "reader/CDXAReader/CDXAReader.h"
#include "reader/UDPReader/UDPReader.h"
#include "reader/VTSReader/VTSReader.h"
#include "renderer/MpcAudioRenderer/MpcAudioRenderer.h"
#include "renderer/SyncClock/SyncClock.h"
#include "source/D2VSource/D2VSource.h"
#include "source/DTSAC3Source/DTSAC3Source.h"
#include "source/FLACSource/FLACSource.h"
#include "source/FLICSource/FLICSource.h"
#include "source/ShoutcastSource/ShoutcastSource.h"
#include "source/SubtitleSource/SubtitleSource.h"
#include "switcher/AudioSwitcher/AudioSwitcher.h"
#include "transform/AVI2AC3Filter/AVI2AC3Filter.h"
#include "transform/BufferFilter/BufferFilter.h"
#include "transform/DeCSSFilter/DeCSSFilter.h"
#include "transform/MpaDecFilter/MpaDecFilter.h"
#include "transform/MPCVideoDec/MPCVideoDecFilter.h"
#include "transform/Mpeg2DecFilter/Mpeg2DecFilter.h"
