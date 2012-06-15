/*
 *      Copyright (C) 2010-2012 Hendrik Leppkes
 *      http://www.1f0.de
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "stdafx.h"
#include "ffmpeg/libavcodec/avcodec.h"
#include "H264RandomAccess.h"

CH264RandomAccess::CH264RandomAccess()
{
  flush(0);
}

CH264RandomAccess::~CH264RandomAccess()
{
}

void CH264RandomAccess::flush(int threadCount)
{
  m_RecoveryMode = 1;
  m_RecoveryFrameCount = 0;
  m_ThreadDelay = threadCount;
}

BOOL CH264RandomAccess::searchRecoveryPoint(AVCodecContext* pAVCtx, BYTE *buf, int buf_size)
{
  if (m_RecoveryMode == 1) {
    int recoveryPoint = avcodec_h264_search_recovery_point(pAVCtx, buf, buf_size, &m_RecoveryFrameCount);
    switch (recoveryPoint) {
    case 3: // IDRS
      m_RecoveryMode = 0;
      return TRUE;
    case 2: // SEI recovery
      m_RecoveryMode = 2;
      return TRUE;
    case 1: // I Frame
      m_RecoveryMode = 2;
      m_RecoveryFrameCount = 0;
      return TRUE;
    default:
      return FALSE;
    }
  } else {
    return TRUE;
  }
}

void CH264RandomAccess::judgeFrameUsability(AVFrame *pFrame, int *got_picture_ptr)
{
  if ((m_ThreadDelay > 1 && --m_ThreadDelay > 0) || m_RecoveryMode == 0 || pFrame->h264_max_frame_num == 0) {
    return;
  }

  if (m_RecoveryMode == 1 || m_RecoveryMode == 2) {
    m_RecoveryFrameCount = (m_RecoveryFrameCount + pFrame->h264_frame_num_decoded) % pFrame->h264_max_frame_num;
    m_RecoveryMode = 3;
  }

  if (m_RecoveryMode == 3) {
    if (m_RecoveryFrameCount <= pFrame->h264_frame_num_decoded) {
      m_RecoveryPOC = pFrame->h264_poc_decoded;
      m_RecoveryMode = 4;
    }
  }

  if (m_RecoveryMode == 4) {
    if (pFrame->h264_poc_outputed >= m_RecoveryPOC) {
      m_RecoveryMode = 0;
    }
  }

  if (m_RecoveryMode != 0) {
    *got_picture_ptr = 0;
  }
}
