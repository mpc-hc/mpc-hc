/*
 * $Id$
 *
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#pragma once

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif


struct AVCodecContext;
struct AVFrame;

typedef int				(*FUNC_AV_DEFAULT_GET_BUFFER)(AVCodecContext *s, AVFrame *pic);
typedef void			(*FUNC_AV_DEFAULT_RELEASE_BUFFER)(AVCodecContext *s, AVFrame *pic);
typedef int				(*FUNC_AV_DEFAULT_REGET_BUFFER)(AVCodecContext *s, AVFrame *pic);


struct TlibavcodecExt {
protected:
	static int get_buffer(AVCodecContext *s, AVFrame *pic);
	static void release_buffer(AVCodecContext *s, AVFrame *pic);
	static int reget_buffer(AVCodecContext *s, AVFrame *pic);

	FUNC_AV_DEFAULT_GET_BUFFER				ff_avcodec_default_get_buffer;
	FUNC_AV_DEFAULT_RELEASE_BUFFER			ff_avcodec_default_release_buffer;
	FUNC_AV_DEFAULT_REGET_BUFFER			ff_avcodec_default_reget_buffer;

public:
	virtual ~TlibavcodecExt() {}
	void			ConnectTo(AVCodecContext *pAVCtx);
	virtual void	OnGetBuffer(AVFrame *pic) {}
	virtual void	OnRegetBuffer(AVFrame *pic) {}
	virtual void	OnReleaseBuffer(AVFrame *pic) {}
};
