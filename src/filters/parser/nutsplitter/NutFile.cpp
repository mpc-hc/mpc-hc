#include "StdAfx.h"
#include "NutFile.h"

CNutFile::CNutFile(IAsyncReader* pAsyncReader, HRESULT& hr)
	: CBaseSplitterFile(pAsyncReader, hr)
{
	if(FAILED(hr)) return;
	hr = Init();
}

HRESULT CNutFile::Init()
{
	Seek(0);

	if(BitRead(64) != NUTM)
		return E_FAIL;

	m_streams.RemoveAll();

	Seek(0);

	while(GetRemaining())
	{
		frame_header fh;
		fh.checksum_flag = 1;

		UINT64 id = 0;

		if(BitRead(1, true) == 0
		|| (id = BitRead(64)) == NUTK)
			break;

		packet_header ph;
		Read(ph);

		if(id == NUTM)
		{
			Read(m_mh);
		}
		else if(id == NUTS)
		{
			CAutoPtr<stream_header> sh(new stream_header());
			Read(*sh);
			if(sh->stream_class == SC_VIDEO) Read(sh->vsh);
			else if(sh->stream_class == SC_AUDIO) Read(sh->ash);
			// else if(sh->stream_class == SC_SUBTITLE) ; // nothing to do
			m_streams.AddTail(sh);
		}
		else if(id == NUTX)
		{
			index_header ih;
			Read(ih);
		}
		else if(id == NUTI)
		{
			info_header ih;
			Read(ih);
		}
		else if(id == 0) // frame
		{
			ASSERT(0);
			break;
		}

		if(fh.checksum_flag)
		{
			Seek(ph.pos + ph.fptr - 4);
			ph.checksum = (UINT32)BitRead(32);
		}

		Seek(ph.pos + ph.fptr);
	}

	Seek(0);

	return m_streams.GetCount() ? S_OK : E_FAIL;
}

void CNutFile::Read(vint& vi)
{
	vi = 0;
	bool more;
	do {more = !!BitRead(1); vi = (vi << 7) | BitRead(7);}
	while(more);
}

void CNutFile::Read(svint& svi)
{
	vint vi;
	Read(vi);
	vi++;
	if(vi&1) svi = -((svint)vi>>1);
	else svi = ((svint)vi>>1);
}

void CNutFile::Read(binary& b)
{
	vint len;
	Read(len);
	b.SetCount((INT_PTR)len);
	for(BYTE* buff = b.GetData(); len-- > 0; *buff++ = (BYTE)BitRead(8));
}

void CNutFile::Read(packet_header& ph)
{
	ph.pos = GetPos();
	Read(ph.fptr);
	Read(ph.bptr);
}

void CNutFile::Read(main_header& mh)
{
	Read(mh.version);
	Read(mh.stream_count);
}

void CNutFile::Read(stream_header& sh)
{
	Read(sh.stream_id);
	Read(sh.stream_class);
    Read(sh.fourcc);
    Read(sh.average_bitrate);
    Read(sh.language_code);
    Read(sh.time_base_nom);
    Read(sh.time_base_denom);
    Read(sh.msb_timestamp_shift);
    Read(sh.shuffle_type);
    sh.fixed_fps = BitRead(1);
    sh.index_flag = BitRead(1);
    sh.reserved = BitRead(6);
	while(1)
	{
		CAutoPtr<codec_specific> p(new codec_specific());
		Read(p->type);
		if(p->type == 0) break;
		Read(p->data);
		sh.cs.AddTail(p);
	}
}

void CNutFile::Read(video_stream_header& vsh)
{
	Read(vsh.width);
	Read(vsh.height);
	Read(vsh.sample_width);
	Read(vsh.sample_height);
	Read(vsh.colorspace_type);
}

void CNutFile::Read(audio_stream_header& ash)
{
	Read(ash.samplerate_mul);
	Read(ash.channel_count);
}

void CNutFile::Read(index_header& ih)
{
	Read(ih.stream_id);

	vint len;
	Read(len);
	ih.ie.SetCount((INT_PTR)len);
	for(index_entry* p = ih.ie.GetData(); len-- > 0;)
	{
		Read(p->timestamp);
		Read(p->position);
	}
}

void CNutFile::Read(info_header& ih)
{
	// TODO
/*
        for(;;){
                id                              v
                if(id==0) break
                name= info_table[id][0]
                type= info_table[id][1]
                if(type==NULL)
                        type                    b
                if(name==NULL)
                        name                    b
                if(type=="v")
                        value                   v
                else if(type=="s")
                        value                   s
                else
                        value                   b
        }
*/
}
