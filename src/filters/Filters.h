/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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
#include "parser/DSMSplitter/DSMSplitter.h"
#include "parser/StreamDriveThru/StreamDriveThru.h"
#include "reader/CDDAReader/CDDAReader.h"
#include "reader/CDXAReader/CDXAReader.h"
#include "reader/VTSReader/VTSReader.h"
#include "renderer/SyncClock/SyncClock.h"
#include "source/SubtitleSource/SubtitleSource.h"
#include "switcher/AudioSwitcher/AudioSwitcher.h"
#include "transform/AVI2AC3Filter/AVI2AC3Filter.h"
#include "transform/BufferFilter/BufferFilter.h"
#include "transform/DeCSSFilter/DeCSSFilter.h"
#include "RARFileSource/RFS.h"
