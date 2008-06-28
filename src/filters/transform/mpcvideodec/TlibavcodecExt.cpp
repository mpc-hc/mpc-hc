/* 
 * $Id$
 *
 * (C) 2006-2007 see AUTHORS
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


#include "stdafx.h"

#include "PODtypes.h"
#include "avcodec.h"

#include "TlibavcodecExt.h"

void TlibavcodecExt::ConnectTo(AVCodecContext *pAVCtx)
{
	pAVCtx->self						= this;
	pAVCtx->get_buffer					= get_buffer;
	pAVCtx->reget_buffer				= reget_buffer;
	pAVCtx->release_buffer				= release_buffer;
	pAVCtx->handle_user_data			= handle_user_data0;
}

int TlibavcodecExt::get_buffer(AVCodecContext *c, AVFrame *pic)
{
	int ret=c->self->ff_avcodec_default_get_buffer(c,pic);
	if (ret==0)
		c->self->OnGetBuffer(pic);
	return ret;
}
int TlibavcodecExt::reget_buffer(AVCodecContext *c, AVFrame *pic)
{
	int ret=c->self->ff_avcodec_default_reget_buffer(c,pic);
	if (ret==0)
		c->self->OnRegetBuffer(pic);
	return ret;
}
void TlibavcodecExt::release_buffer(AVCodecContext *c, AVFrame *pic)
{
	c->self->ff_avcodec_default_release_buffer(c,pic);
	c->self->OnReleaseBuffer(pic);
}
void TlibavcodecExt::handle_user_data0(AVCodecContext *c, const uint8_t *buf,int buf_len)
{
	c->self->HandleUserData(buf,buf_len);
}
