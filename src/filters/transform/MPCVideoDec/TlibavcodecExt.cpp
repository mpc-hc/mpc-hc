/*
 * $Id$
 *
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


#include "stdafx.h"

#include <ffmpeg/libavcodec/avcodec.h>

#include "TlibavcodecExt.h"

void TlibavcodecExt::ConnectTo(AVCodecContext *pAVCtx)
{
	pAVCtx->opaque         = this;
	pAVCtx->get_buffer     = get_buffer;
	pAVCtx->reget_buffer   = reget_buffer;
	pAVCtx->release_buffer = release_buffer;
}

int TlibavcodecExt::get_buffer(AVCodecContext *c, AVFrame *pic)
{
	int ret=c->opaque->ff_avcodec_default_get_buffer(c,pic);
	if (ret==0) {
		c->opaque->OnGetBuffer(pic);
	}
	return ret;
}

int TlibavcodecExt::reget_buffer(AVCodecContext *c, AVFrame *pic)
{
	int ret=c->opaque->ff_avcodec_default_reget_buffer(c,pic);
	if (ret==0) {
		c->opaque->OnRegetBuffer(pic);
	}
	return ret;
}

void TlibavcodecExt::release_buffer(AVCodecContext *c, AVFrame *pic)
{
	c->opaque->ff_avcodec_default_release_buffer(c,pic);
	c->opaque->OnReleaseBuffer(pic);
}
