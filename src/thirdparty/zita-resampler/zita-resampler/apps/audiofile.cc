// ----------------------------------------------------------------------------
//
//  Copyright (C) 2006-2011 Fons Adriaensen <fons@linuxaudio.org>
//    
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ----------------------------------------------------------------------------


#include <stdlib.h>
#include <string.h>
#include "audiofile.h"


Audiofile::Audiofile (void)
{
    reset ();
}


Audiofile::~Audiofile (void)
{
    close ();
}


void Audiofile::reset (void)
{
    _sndfile = 0;
    _mode = MODE_NONE;
    _type = TYPE_OTHER;
    _form = FORM_OTHER;
    _rate = 0;
    _chan = 0;
    _size = 0;
    _dith_type = 0;
    _dith_proc = 0;
    _dith_buff = 0;
}


int Audiofile::open_read (const char *name)
{
    SF_INFO I;

    if (_mode) return ERR_MODE;
    reset ();

    if ((_sndfile = sf_open (name, SFM_READ, &I)) == 0) return ERR_OPEN;

    _mode = MODE_READ;

    switch (I.format & SF_FORMAT_TYPEMASK)
    {
    case SF_FORMAT_CAF:
	_type = TYPE_CAF;
	break;
    case SF_FORMAT_WAV:
	_type = TYPE_WAV;
	break;
    case SF_FORMAT_WAVEX:
        if (sf_command (_sndfile, SFC_WAVEX_GET_AMBISONIC, 0, 0) == SF_AMBISONIC_B_FORMAT)
	    _type = TYPE_AMB;
	else
            _type = TYPE_WAV;
    }

    switch (I.format & SF_FORMAT_SUBMASK)
    {
    case SF_FORMAT_PCM_16:
	_form = FORM_16BIT;
	break;
    case SF_FORMAT_PCM_24:
	_form = FORM_24BIT;
	break;
    case SF_FORMAT_PCM_32:
	_form = FORM_32BIT;
	break;
    case SF_FORMAT_FLOAT:
	_form = FORM_FLOAT;
	break;
    }

    _rate = I.samplerate;
    _chan = I.channels;
    _size = I.frames;

    return 0;
}


int Audiofile::open_write (const char *name, int type, int form, int rate, int chan)    
{
    SF_INFO I;

    if (_mode) return ERR_MODE;
    if (!rate || !chan) return ERR_OPEN;
    reset ();

    switch (type)
    {
    case TYPE_CAF:
	I.format = SF_FORMAT_CAF;
	break;
    case TYPE_WAV:
    case TYPE_AMB:
        I.format = (chan > 2) ? SF_FORMAT_WAVEX : SF_FORMAT_WAV;
	break;
    default:
        return ERR_TYPE;
    }

    switch (form)
    {
    case FORM_16BIT:
	I.format |= SF_FORMAT_PCM_16;
	break;
    case FORM_24BIT:
	I.format |= SF_FORMAT_PCM_24;
	break;
    case FORM_32BIT:
	I.format |= SF_FORMAT_PCM_32;
	break;
    case FORM_FLOAT:
	I.format |= SF_FORMAT_FLOAT;
	break;
    default:
        return ERR_FORM;
    }

    I.samplerate = rate;
    I.channels = chan;
    I.sections = 1;

    if ((_sndfile = sf_open (name, SFM_WRITE, &I)) == 0) return ERR_OPEN;

    if (type == TYPE_AMB)
    {
        sf_command (_sndfile, SFC_WAVEX_SET_AMBISONIC, 0, SF_AMBISONIC_B_FORMAT);
    }

    _mode = MODE_WRITE;
    _type = type;
    _form = form;
    _rate = rate;
    _chan = chan;

    return 0;
}


int Audiofile::set_dither (int type)
{
    if (_mode != MODE_WRITE) return ERR_MODE;
    if (_form != FORM_16BIT) return ERR_FORM; 
    if (type != DITHER_NONE)
    {
	if (_dith_type == DITHER_NONE)
	{
	    _dith_proc = new Dither [_chan];
	    _dith_buff = new int16_t [_chan * BUFFSIZE]; 
	}
    }
    _dith_type = type;
    return 0;
}


int Audiofile::close (void)
{
    if (_sndfile) sf_close (_sndfile);
    delete[] _dith_proc;
    delete[] _dith_buff;
    reset ();
    return 0;
}


int Audiofile::seek (uint32_t posit)
{
    if (!_sndfile) return ERR_MODE;
    if (sf_seek (_sndfile, posit, SEEK_SET) != posit) return ERR_SEEK;
    return 0;
}


int Audiofile::read (float *data, uint32_t frames)
{
    if (_mode != MODE_READ) return ERR_MODE;
    return sf_readf_float (_sndfile, data, frames);
}


int Audiofile::write (float *data, uint32_t frames)
{
    int       i;
    uint32_t  k, n, r;
    float     *p, v;
    int16_t   *q;
    Dither    *D;

    if (_mode != MODE_WRITE) return ERR_MODE;
    if (_dith_type == DITHER_NONE)
    {
	if (_form != FORM_FLOAT)
	{
  	    for (i = 0; i < _chan; i++)
	    {
		p = data + i;
	        for (k = 0; k < frames; k++)
	        {
		    v = *p;
		    if      (v >  1.0f) v =  1.0f;
		    else if (v < -1.0f) v = -1.0f;
		    *p = v;
		    p += _chan;
		}
	    }
	}
        return sf_writef_float (_sndfile, data, frames);
    }
    else
    {
	n = 0;
	while (frames)
	{
	    k = (frames > BUFFSIZE) ? BUFFSIZE : frames;
  	    p = data;
            q = _dith_buff;
            D = _dith_proc;
	    for (i = 0; i < _chan; i++)
	    {
		switch (_dith_type)
		{
		case DITHER_RECT:
		    D->proc_rectangular (p, q, _chan, k);
		    break;
		case DITHER_TRIA:
		    D->proc_triangular (p, q, _chan, k);
		    break;
		case DITHER_LIPS:
		    D->proc_lipschitz (p, q, _chan, k);
		    break;
		}
		p++;
		q++;
		D++;
	    }
            r = sf_writef_short (_sndfile, _dith_buff, k);
	    n += r;
	    if (r != k) return n;
	    data += k * _chan;
	    frames -= k;
	}
    }
    return 0;
}


