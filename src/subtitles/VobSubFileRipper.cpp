/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
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

#include "StdAfx.h"
#include "vobsubfileripper.h"
#include "..\decss\VobDec.h"
#include "..\subtitles\CCDecoder.h"

//
// CVobSubFileRipper
//

CVobSubFileRipper::CVobSubFileRipper()
	: CVobSubFile(NULL)
	, m_fThreadActive(false)
	, m_fBreakThread(false)
	, m_fIndexing(false)
{
	m_rd.Reset();
	CAMThread::Create();
}

CVobSubFileRipper::~CVobSubFileRipper()
{
	CAMThread::CallWorker(CMD_EXIT);
	CAMThread::Close();
}

STDMETHODIMP CVobSubFileRipper::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return 
		QI(IVSFRipper)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

void CVobSubFileRipper::Log(log_t type, LPCTSTR lpszFormat, ...)
{
	CAutoLock cAutoLock(&m_csCallback);
	if(!m_pCallback) return;

	TCHAR buff[1024];

	va_list args;
	va_start(args, lpszFormat);
	_vstprintf(buff, lpszFormat, args);
	va_end(args);

	CString msg;
	switch(type)
	{
	default:
	case LOG_INFO: msg = _T(""); break;
	case LOG_WARNING: msg = _T("WARNING: "); break;
	case LOG_ERROR: msg = _T("ERROR: "); break;
	}

	msg += buff;

	m_pCallback->OnMessage(msg);
}

void CVobSubFileRipper::Progress(double progress)
{
	CAutoLock cAutoLock(&m_csCallback);
	if(!m_pCallback) return;

	m_pCallback->OnProgress(progress);
}

void CVobSubFileRipper::Finished(bool fSucceeded)
{
	CAutoLock cAutoLock(&m_csCallback);
	if(!m_pCallback) return;

	m_pCallback->OnFinished(fSucceeded);
}

#define ReadBEb(var) \
	f.Read(&((BYTE*)&var)[0], 1); \

#define ReadBEw(var) \
	f.Read(&((BYTE*)&var)[1], 1); \
	f.Read(&((BYTE*)&var)[0], 1); \

#define ReadBEdw(var) \
    f.Read(&((BYTE*)&var)[3], 1); \
	f.Read(&((BYTE*)&var)[2], 1); \
	f.Read(&((BYTE*)&var)[1], 1); \
	f.Read(&((BYTE*)&var)[0], 1); \

bool CVobSubFileRipper::LoadIfo(CString fn)
{
	CString str;

	CFileStatus status;
	if(!CFileGetStatus(fn, status) || !status.m_size)
	{
		Log(LOG_ERROR, _T("Invalid ifo"));
		return(false);
	}

	CFile f;
	if(!f.Open(fn, CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone))
	{
		Log(LOG_ERROR, _T("Cannot open ifo"));
		return(false);
	}

	Log(LOG_INFO, _T("Opening ifo OK"));

	char hdr[13];
	f.Read(hdr, 12);
	hdr[12] = 0;
	if(strcmp(hdr, "DVDVIDEO-VTS"))
	{
		Log(LOG_ERROR, _T("Not a Video Title Set IFO file!"));
		return(false);
	}

	// lang ids

	f.Seek(0x254, CFile::begin);

	WORD ids[32];
	memset(ids, 0, sizeof(ids));

	int len = 0;
	ReadBEw(len);

	for(int i = 0; i < len; i++)
	{
		f.Seek(2, CFile::current); // 01 00 ?
		ReadBEw(ids[i]);
		if(ids[i] == 0) ids[i] = '--';
		f.Seek(2, CFile::current); // 00 00 ?
	}

	/* Video info */

	f.Seek(0x200, CFile::begin);
	f.Read(&m_rd.vidinfo, 2);

	SIZE res[4][2] =
	{
		{{720,480},{720,576}},
		{{704,480},{704,576}},
		{{352,480},{352,576}},
		{{352,240},{352,288}}
	};

	m_rd.vidsize = res[m_rd.vidinfo.source_res][m_rd.vidinfo.system&1];

	double rate = (m_rd.vidinfo.system == 0) ? 30.0/29.97 : 1.0;

	/* PGCs */

	{
		DWORD offset;

		DWORD pgcpos;
		f.Seek(0xc0+0x0c, CFile::begin);
		ReadBEdw(pgcpos);
		pgcpos *= 0x800;

		WORD nPGC;
		f.Seek(pgcpos, CFile::begin);
		ReadBEw(nPGC);

		m_rd.pgcs.RemoveAll();
		m_rd.pgcs.SetCount(nPGC);

		for(int i = 0; i < nPGC; i++)
		{
			PGC& pgc = m_rd.pgcs[i];

			f.Seek(pgcpos + 8 + i*8 + 4, CFile::begin);
			ReadBEdw(offset);
			offset += pgcpos;

			BYTE nProgs, nCells;
			f.Seek(offset + 2, CFile::begin);
			ReadBEb(nProgs);
			ReadBEb(nCells);

			//

			memcpy(pgc.ids, ids, sizeof(ids));

			struct splanginfo {BYTE res1, id1, id2, res2;};
			splanginfo splinfo[32];

			f.Seek(offset + 0x1c, CFile::begin);
			f.Read(splinfo, 32*4);

			for(int j = 0; j < 32; j++) 
			{
				if(splinfo[j].id1 || splinfo[i].id2) 
				{
					WORD tmpids[32];
					memset(tmpids, 0, sizeof(tmpids));

					for(j = 0; j < 32; j++) 
					{
						if(!(splinfo[j].res1 & 0x80)) break;

						pgc.ids[splinfo[j].id1] = ids[j];
						pgc.ids[splinfo[j].id2] = ids[j];
					}

					break;
				}
			}

			//

			f.Seek(offset + 0xa4, CFile::begin);

			for(int j = 0; j < 16; j++) 
			{
				BYTE y, u, v, tmp;

				f.Read(&tmp, 1);
				f.Read(&y, 1);
				f.Read(&u, 1);
				f.Read(&v, 1);

				y = (y-16)*255/219;

				pgc.pal[j].rgbRed = (BYTE)min(max(1.0*y + 1.4022*(u-128), 0), 255);
				pgc.pal[j].rgbGreen = (BYTE)min(max(1.0*y - 0.3456*(u-128) - 0.7145*(v-128), 0), 255);
				pgc.pal[j].rgbBlue = (BYTE)min(max(1.0*y + 1.7710*(v-128), 0) , 255);
			}

			//

			WORD progoff, celladdroff, vobcelloff;
			f.Seek(offset + 0xe6, CFile::begin);
			ReadBEw(progoff);
			f.Seek(offset + 0xe8, CFile::begin);
			ReadBEw(celladdroff);
			f.Seek(offset + 0xea, CFile::begin);
			ReadBEw(vobcelloff);

			//

            CAtlArray<BYTE> progs;
			progs.SetCount(nProgs);
			f.Seek(offset + progoff, CFile::begin);
			f.Read(progs.GetData(), nProgs);

			//

			pgc.angles[0].SetCount(nCells);
			pgc.iSelAngle = 0;

			//

			f.Seek(offset + vobcelloff, CFile::begin);
			for(int j = 0; j < nCells; j++)
			{
				ReadBEw(pgc.angles[0][j].vob);
				ReadBEw(pgc.angles[0][j].cell);
			}

			//

			DWORD tOffset = 0, tTotal = 0;

			int iAngle = 0;

			pgc.nAngles = 0;

			f.Seek(offset + celladdroff, CFile::begin);
			for(int j = 0; j < nCells; j++)
			{
				BYTE b;
				ReadBEb(b);
				switch(b>>6)
				{
				case 0: iAngle = 0; break; // normal
				case 1: iAngle = 1; break; // first angle block
				case 2: iAngle++; break; // middle angle block
				case 3: iAngle++; break; // last angle block (no more should follow)
				}
				pgc.angles[0][j].iAngle = iAngle;
				pgc.nAngles = max(pgc.nAngles, iAngle);

				f.Seek(3, CFile::current);
				ReadBEdw(pgc.angles[0][j].tTime);
				ReadBEdw(pgc.angles[0][j].start);
				f.Seek(8, CFile::current);
				ReadBEdw(pgc.angles[0][j].end);

				float fps;
				switch((pgc.angles[0][j].tTime>>6)&0x3)
				{
				default:
				case 3: fps = 30; break;
				case 1: fps = 25; break;
				}

				int t = pgc.angles[0][j].tTime;
				int hh = ((t>>28)&0xf)*10+((t>>24)&0xf);
				int mm = ((t>>20)&0xf)*10+((t>>16)&0xf);
				int ss = ((t>>12)&0xf)*10+((t>>8)&0xf);
				int ms = (int)(1000.0 * (((t>>4)&0x3)*10+((t>>0)&0xf)) / fps);
				pgc.angles[0][j].tTime = (DWORD)((((hh*60+mm)*60+ss)*1000+ms)*rate);

				// time discontinuity
				if(b&0x02) tOffset = tTotal;
				pgc.angles[0][j].fDiscontinuity = !!(b&0x02);

				pgc.angles[0][j].tTotal = tTotal;
				pgc.angles[0][j].tOffset = tOffset;

				tTotal += pgc.angles[0][j].tTime;
			}

			for(iAngle = 1; iAngle <= 9; iAngle++)
			{
				tOffset = tTotal = 0;

				for(int j = 0, k = 0; j < nCells; j++)
				{
					if(pgc.angles[0][j].iAngle != 0
					&& pgc.angles[0][j].iAngle != iAngle)
						continue;

					pgc.angles[iAngle].Add(pgc.angles[0][j]);

					if(pgc.angles[iAngle][k].fDiscontinuity) tOffset = tTotal;

					pgc.angles[iAngle][k].tTotal = tTotal;
					pgc.angles[iAngle][k].tOffset = tOffset;

					tTotal += pgc.angles[iAngle][k].tTime;

					k++;
				}
			}
		}
	}

    Log(LOG_INFO, _T("Parsing ifo OK"));

	return(true);
}

bool CVobSubFileRipper::LoadVob(CString fn)
{
	Log(LOG_INFO, _T("Searching vobs..."));
/*
	CAtlList<CString> m_vobs;

	fn = fn.Left(fn.ReverseFind('.')+1);
	fn.TrimRight(_T(".0123456789"));
	for(int i = 0; i < 100; i++)
	{
		CString vob;
		vob.Format(_T("%s%d.vob"), fn, i);

		CFileStatus status;
		if(!(CFileGetStatus(vob, status) && status.m_size))
		{
			if(i > 0) break;
			else continue;
		}

		if(status.m_size&0x7ff)
		{
			Log(LOG_ERROR, _T("Length of %s is not n*2048!"), vob);
			m_vobs.RemoveAll();
			break;
		}

		CString str = _T("Found ") + vob;

		if(i == 0) 
		{
			str += _T(" (skipping, if not a menu vob rename it)");
		}
		else
		{
			m_vobs.AddTail(vob);
		}

		Log(LOG_INFO, str);
	}

	if(m_vobs.GetCount() <= 0)
	{
		Log(LOG_ERROR, _T("Nothing found! (%s*.vob)"), fn);
		return(false);
	}
*/
	CAtlList<CString> vobs;
	if(!m_vob.Open(fn, vobs/*m_vobs*/))
	{
		Log(LOG_ERROR, _T("Cannot open vob sequence"));
		return(false);
	}

	if(vobs.GetCount() <= 0)
	{
		Log(LOG_ERROR, _T("Nothing found! (%s*.vob)"), fn);
		return(false);
	}

	POSITION pos = vobs.GetHeadPosition();
	while(pos) Log(LOG_INFO, _T("Found ") + vobs.GetNext(pos));

	if(m_vob.IsDVD())
	{
		Log(LOG_INFO, _T("DVD detected..."));

		BYTE key[5];

		if(m_vob.HasDiscKey(key))
			Log(LOG_INFO, _T("Disc key: %02x%02x%02x%02x%02x"), key[0], key[1], key[2], key[3], key[4]);
		else
			Log(LOG_WARNING, _T("Couldn't get the disc key"));

		if(m_vob.HasTitleKey(key))
			Log(LOG_INFO, _T("Title key: %02x%02x%02x%02x%02x"), key[0], key[1], key[2], key[3], key[4]);
		else
			Log(LOG_WARNING, _T("Couldn't get the title key"));

		BYTE buff[2048];

		m_vob.Seek(0);
		if(!m_vob.Read(buff))
		{
			Log(LOG_ERROR, _T("Can't read vob, please unlock it with a software player!"));
			return(false);
		}
		m_vob.Seek(0);
	}

	return(true);
}

DWORD CVobSubFileRipper::ThreadProc()
{	
	SetThreadPriority(m_hThread, THREAD_PRIORITY_BELOW_NORMAL);

    while(1)
	{
		DWORD cmd = GetRequest();

		m_fThreadActive = true;

		switch(cmd)
		{
		case CMD_EXIT:
		    Reply(S_OK);
			return 0;

		case CMD_INDEX:
			Reply(S_OK);
			{
			m_fIndexing = true;
			bool fSucceeded = Create();
			m_fIndexing = false;
			Finished(fSucceeded);
			}
			break;

		default:
		    Reply(E_FAIL);
			return -1;
		}

		m_fBreakThread = false;
		m_fThreadActive = false;
	}

	return 1;
}

static int SubPosSortProc(const void* e1, const void* e2)
{
	return((int)(((CVobSubFile::SubPos*)e1)->start - ((CVobSubFile::SubPos*)e2)->start));
}

bool CVobSubFileRipper::Create()
{
	CAutoLock cAutoLock(&m_csAccessLock);

	if(m_rd.iSelPGC < 0 || m_rd.iSelPGC >= m_rd.pgcs.GetCount())
	{
		Log(LOG_ERROR, _T("Invalid program chain number (%d)!"), m_rd.iSelPGC);
		return(false);
	}

	PGC& pgc = m_rd.pgcs[m_rd.iSelPGC];

	if(pgc.iSelAngle < 0 || pgc.iSelAngle > 9 || pgc.angles[pgc.iSelAngle].GetCount() == 0)
	{
		Log(LOG_ERROR, _T("Invalid angle number (%d)!"), pgc.iSelAngle);
		return(false);
	}

	CAtlArray<vc_t>& angle = pgc.angles[pgc.iSelAngle];

	if(m_rd.selids.GetCount() == 0 && !m_rd.fClosedCaption)
	{
		Log(LOG_ERROR, _T("No valid stream set to be extacted!"));
		return(false);
	}

	if(m_rd.selvcs.GetCount() == 0)
	{
		Log(LOG_ERROR, _T("No valid vob/cell id set to be extacted!"));
		return(false);
	}

	Log(LOG_INFO, _T("Indexing..."));

	// initalize CVobSubFile
	CVobSubFile::Close();
	InitSettings();
	m_title = m_outfn;
	m_size = m_rd.vidsize;
	TrimExtension(m_title);
	memcpy(m_orgpal, pgc.pal, sizeof(m_orgpal));
	m_sub.SetLength(0);

	CCDecoder ccdec(m_title + _T(".cc.srt"), m_title + _T(".cc.raw"));

	CVobDec vd;

	__int64 SCR, PTS, tOffset = 0, tPrevOffset = 0, tTotal = 0, tStart = 0;
	int vob = 0, cell = 0;
	bool fDiscontinuity = false, fDiscontinuityFixApplied = false, fNavpackFound = false;

	int PTSframeoffset = 0, minPTSframeoffset = 0;

	if(m_rd.fResetTime)
	{
		for(int i = 0; i < angle.GetCount() && ((angle[i].vob<<16)|angle[i].cell) != m_rd.selvcs[0]; i++)
			tStart += angle[i].tTime;

		Log(LOG_INFO, _T("Counting timestamps from %I64dms (v%02dc%02d)"), 
			tStart, m_rd.selvcs[0]>>16, m_rd.selvcs[0]&0xffff);
	}

	CAtlMap<DWORD, int> selvcmap;
	selvcmap.RemoveAll();
	for(int i = 0; i < m_rd.selvcs.GetCount(); i++)
		selvcmap[m_rd.selvcs[i]] = 90000;

	CAtlArray<vcchunk> chunks, foundchunks, loadedchunks;

	if(m_vob.IsDVD())
	{
		Log(LOG_INFO, _T("Indexing mode: DVD"));

		for(int i = 0; i < angle.GetCount(); i++)
		{
			DWORD vc = (angle[i].vob<<16)|angle[i].cell;
			if(!selvcmap.Lookup(vc))
				continue;

			vcchunk c = {2048i64*angle[i].start, 2048i64*angle[i].end+2048, vc};
			chunks.Add(c);

			Log(LOG_INFO, _T("Adding: 0x%x - 0x%x (lba) for vob %d cell %d"), 
				angle[i].start, angle[i].end, angle[i].vob, angle[i].cell);
		}
	}
	else if(LoadChunks(loadedchunks))
	{
		Log(LOG_INFO, _T("Indexing mode: File"));

		for(int i = 0; i < loadedchunks.GetCount(); i++)
		{
			DWORD vcid = loadedchunks[i].vc;
			if(!selvcmap.Lookup(vcid))
				continue;

			chunks.Add(loadedchunks[i]);
		}

		Log(LOG_INFO, _T(".chunk file loaded"));
	}
	else
	{
		Log(LOG_INFO, _T("Indexing mode: File"));

		chunks.RemoveAll();
		vcchunk c = {0, 2048i64*m_vob.GetLength(), 0};
		chunks.Add(c);
	}

	__int64 sizedone = 0, sizetotal = 0;
	for(int i = 0; i < chunks.GetCount(); i++)
		sizetotal += chunks[i].end - chunks[i].start;

	for(int i = 0; !m_fBreakThread && i < chunks.GetCount(); i++)
	{
		__int64 curpos = chunks[i].start, endpos = chunks[i].end;

		vcchunk curchunk = {curpos, curpos, chunks[i].vc};

		for(m_vob.Seek((int)(curpos/2048)); !m_fBreakThread && curpos < endpos; curpos += 2048, sizedone += 2048)
		{
			if(!(curpos&0x7ffff))
				Progress(1.0 * sizedone / sizetotal);

			static BYTE buff[2048];

			if(!m_vob.Read(buff))
			{
				Log(LOG_ERROR, _T("Cannot read, either locked dvd or truncated/missing files!"));
				return(false);
			}

			curchunk.end = curpos;

			if(buff[0x14] & 0x30)
			{
				if(!vd.m_fFoundKey)
				{
					Log(LOG_INFO, _T("Encrypted sector found, searching key..."));

					__int64 savepos = curpos;

					m_vob.Seek(0);
					for(__int64 pos = 0; !m_fBreakThread && pos < endpos; pos += 2048)
					{
						if(!m_vob.Read(buff))
						{
							Log(LOG_ERROR, _T("Cannot read, either locked dvd or truncated/missing files!"));
							return(false);
						}

						if(vd.FindKey(buff))
							break;
					}

					if(m_fBreakThread)
						break;

					if(!vd.m_fFoundKey)
					{
						Log(LOG_ERROR, _T("Key not found, can't decrypt!"));
						return(false);
					}

					Log(LOG_INFO, _T("Key found, continuing extraction..."));

					m_vob.Seek((int)((curpos = savepos)/2048));
					m_vob.Read(buff);
				}

				vd.Decrypt(buff);
			}

			if(*((DWORD*)&buff[0]) != 0xba010000)
			{
				Log(LOG_WARNING, _T("Bad sector header at block %08d!"), (int)(curpos/2048));

				if(AfxMessageBox(_T("Bad packet header found, do you want to continue?"), MB_YESNO) == IDNO)
				{
					Log(LOG_ERROR, _T("Terminated!"));
					return(false);
				}
			}
			
			SCR = (__int64(buff[0x04] & 0x38) >> 3) << 30
				| __int64(buff[0x04] & 0x03) << 28
				| __int64(buff[0x05]) << 20
				| (__int64(buff[0x06] & 0xf8) >> 3) << 15
				| __int64(buff[0x06] & 0x03) << 13
				| __int64(buff[0x07]) << 5
				| (__int64(buff[0x08] & 0xf8) >> 3) << 0;

			bool hasPTS = false;
				
			if((*(DWORD*)&buff[0x0e] == 0xe0010000 || *(DWORD*)&buff[0x0e] == 0xbd010000)
				&& buff[0x15] & 0x80)
			{
				PTS = (__int64)(buff[0x17] & 0x0e) << 29		// 32-30 (+marker)
					| ((__int64)(buff[0x18]) << 22)				// 29-22
					| ((__int64)(buff[0x19] & 0xfe) << 14)		// 21-15 (+marker)
					| ((__int64)(buff[0x1a]) << 7)				// 14-07
					| ((__int64)(buff[0x1b]) >> 1);				// 06-00 (+marker)
				
				hasPTS = true;
			}

			if(*((DWORD*)&buff[0x0e]) == 0xbb010000)
			{
				fNavpackFound = true;

				if(vob == buff[0x420] && cell == buff[0x422])
					continue;

				vob = buff[0x420];
				cell = buff[0x422];

				tOffset = tTotal = 0;

				for(int i = 0; i < angle.GetCount(); i++)
				{
					if(angle[i].vob == vob && angle[i].cell == cell)
					{
						tPrevOffset = tOffset;
						tOffset = (__int64)angle[i].tOffset;
						tTotal = (__int64)angle[i].tTotal;
						fDiscontinuity = angle[i].fDiscontinuity;
						fDiscontinuityFixApplied = false;
						break;
					}
				}

				if(curchunk.vc != ((vob<<16)|cell))
				{
					if(curchunk.vc != 0) foundchunks.Add(curchunk);
					curchunk.start = curchunk.end = curpos;
					curchunk.vc = (vob<<16)|cell;
				}

				CString str, str2;
				str.Format(_T("v%02d c%02d lba%08d"), vob, cell, (int)(curpos/2048));
				UINT vcid = (vob<<16)|cell;
				if(!selvcmap.Lookup(vcid, minPTSframeoffset)) str2 = _T(", skipping");
				else str2.Format(_T(", total=%I64dms, off=%I64dms, corr=%I64dms, discont.:%d"), 
					tTotal, tOffset, -tStart, (int)fDiscontinuity);
				Log(LOG_INFO, str + str2);
			}

			DWORD vcid = (vob<<16)|cell;
			if(!selvcmap.Lookup(vcid, minPTSframeoffset))
				continue;

			if(hasPTS && fDiscontinuity && !fDiscontinuityFixApplied)
			{
				__int64 tDiff = tOffset - tPrevOffset;
				if(tDiff > 0 && tDiff < (PTS/90+1000))
				{
					CString str;
					str.Format(_T("False discontinuity detected, correcting time by %I64dms"), -tDiff);
					Log(LOG_INFO, str);

					tStart += tDiff;
				}
				fDiscontinuityFixApplied = true;
			}

			if(*(DWORD*)&buff[0x0e] == 0xe0010000)
			{
				if(fDiscontinuity)
				{
					if(PTS < minPTSframeoffset)
					{
						selvcmap[vcid] = PTSframeoffset = PTS;
					}

					fDiscontinuity = false;
				}

				if(m_rd.fClosedCaption)
					ccdec.ExtractCC(buff, 2048, tOffset + ((PTS - PTSframeoffset) / 90) - tStart);
			}
			else if(*(DWORD*)&buff[0x0e] == 0xbd010000)
			{
				BYTE id = buff[0x17 + buff[0x16]], iLang = id&0x1f;

				if((id & 0xe0) == 0x20 && m_rd.selids.Lookup(iLang))
				{
					if(hasPTS)
					{
						SubPos sb;
						sb.filepos = m_sub.GetPosition();
						sb.start = tOffset + ((PTS - PTSframeoffset) / 90) - tStart;
						sb.vobid = (char)vob;
						sb.cellid = (char)cell;
						sb.celltimestamp = tTotal;
						sb.fValid = true;
						m_langs[iLang].subpos.Add(sb);
					}

					m_sub.Write(buff, 2048);
				}
			}
		}

		if(curchunk.vc != ((vob<<16)|cell))
		{
			if(curchunk.vc != 0) foundchunks.Add(curchunk);
			curchunk.start = curchunk.end = curpos;
			curchunk.vc = (vob<<16)|cell;
		}
	}

	if(sizedone < sizetotal)
	{
		Log(LOG_ERROR, _T("Indexing terminated before reaching the end!"));
		Progress(0);
		return(false);
	}

	if(!fNavpackFound)
	{
		Log(LOG_ERROR, _T("Could not find any system header start code! (0x000001bb)"));
		if(!m_vob.IsDVD()) Log(LOG_ERROR, _T("Make sure the ripper doesn't strip these packets."));
		Progress(0);
		return(false);
	}

	Log(LOG_INFO, _T("Indexing finished"));
	Progress(1);

	for(int i = 0; i < 32; i++)
	{
		if(m_iLang == -1 && m_langs[i].subpos.GetCount() > 0) m_iLang = i;
		m_langs[i].id = pgc.ids[i];
		m_langs[i].name = m_langs[i].alt = FindLangFromId(m_langs[i].id);

		CAtlArray<SubPos>& sp = m_langs[i].subpos;
		qsort(sp.GetData(), sp.GetCount(), sizeof(SubPos), SubPosSortProc);

		if(m_rd.fForcedOnly)
		{
			Log(LOG_INFO, _T("Searching for forced subs..."));
			Progress(0);

			for(int j = 0, len = sp.GetCount(); j < len; j++)
			{
				Progress(1.0 * j / len);

				sp[j].fValid = false;
				int packetsize = 0, datasize = 0;
				if(BYTE* buff = GetPacket(j, packetsize, datasize, i))
				{
					m_img.GetPacketInfo(buff, packetsize, datasize);
					sp[j].fValid = m_img.fForced;
					delete [] buff;
				}
			}

			Progress(1);
		}
	}

	Log(LOG_INFO, _T("Saving files..."));

	if(m_iLang != -1)
	{
		if(!Save(m_title))
		{
			Log(LOG_ERROR, _T("Could not save output files!"));
			return(false);
		}
	}

	Log(LOG_INFO, _T("Subtitles saved"));

	if(!m_vob.IsDVD() && loadedchunks.GetCount() == 0)
	{
		if(SaveChunks(foundchunks))
		{
			Log(LOG_INFO, _T(".chunk file saved"));
		}
	}

	Log(LOG_INFO, _T("Done!"));

	return(true);
}

static const DWORD s_version = 1;

bool CVobSubFileRipper::LoadChunks(CAtlArray<vcchunk>& chunks)
{
	CFile f;

	CString fn = m_infn;
	TrimExtension(fn);
	fn += _T(".chunks");

	DWORD chksum = 0, chunklen, version;
	__int64 voblen;

	if(!f.Open(fn, CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone))
		return(false);
	f.Read(&version, sizeof(version));
	if(version == 1)
	{
		f.Read(&chksum, sizeof(chksum));
		f.Read(&voblen, sizeof(voblen));
		f.Read(&chunklen, sizeof(chunklen));
		chunks.SetCount(chunklen);
		f.Read(chunks.GetData(), sizeof(vcchunk)*chunks.GetCount());
	}
	f.Close();

	if(voblen != m_vob.GetLength())
	{
		chunks.RemoveAll();
		return(false);
	}

	if(!f.Open(m_infn, CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone))
		return(false);
	DWORD dw, chksum2 = 0;
	while(f.Read(&dw, sizeof(dw)) == sizeof(dw)) chksum2 += dw;
	f.Close();

	if(chksum != chksum2)
	{
		chunks.RemoveAll();
		return(false);
	}

	return(true);
}

bool CVobSubFileRipper::SaveChunks(CAtlArray<vcchunk>& chunks)
{
	CFile f;

	CString fn = m_infn;
	TrimExtension(fn);
	fn += _T(".chunks");

	DWORD chksum = 0, chunklen = chunks.GetCount();
	__int64 voblen = m_vob.GetLength();

	if(!f.Open(m_infn, CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone))
		return(false);
	DWORD dw;
	while(f.Read(&dw, sizeof(dw)) == sizeof(dw)) chksum += dw;
	f.Close();

	if(!f.Open(fn, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary|CFile::shareDenyWrite))
		return(false);
	f.Write(&s_version, sizeof(s_version));
	f.Write(&chksum, sizeof(chksum));
	f.Write(&voblen, sizeof(voblen));
	f.Write(&chunklen, sizeof(chunklen));
	f.Write(chunks.GetData(), sizeof(vcchunk)*chunklen);
	f.Close();

	return(true);
}

// IVSFRipper

STDMETHODIMP CVobSubFileRipper::SetCallBack(IVSFRipperCallback* pCallback)
{
	CAutoLock cAutoLock(&m_csCallback);
	m_pCallback = pCallback;
	return S_OK;
}

STDMETHODIMP CVobSubFileRipper::LoadParamFile(CString fn)
{
	CAutoLock cAutoLock(&m_csAccessLock);

	m_rd.Reset();

	CStdioFile f;
	if(!f.Open(fn, CFile::modeRead|CFile::typeText))
		return E_FAIL;

	TCHAR langid[256];

	enum {P_INPUT, P_OUTPUT, P_PGC, P_ANGLE, P_LANGS, P_OPTIONS};
	int phase = P_INPUT;

	CString line;
	while(f.ReadString(line))
	{
		if(line.Trim().IsEmpty() || line[0] == '#') continue;

		if(phase == P_INPUT)
		{
			if(S_OK != SetInput(line)) break;
			phase = P_OUTPUT;
		}
		else if(phase == P_OUTPUT)
		{
			if(S_OK != SetOutput(line)) break;
			phase = P_PGC;
		}
		else if(phase == P_PGC)
		{
			m_rd.iSelPGC = _tcstol(line, NULL, 10)-1;
			if(m_rd.iSelPGC < 0 || m_rd.iSelPGC >= m_rd.pgcs.GetCount()) break;
			phase = P_ANGLE;
		}
		else if(phase == 3)
		{
			PGC& pgc = m_rd.pgcs[m_rd.iSelPGC];

			pgc.iSelAngle = _tcstol(line, NULL, 10);
			if(pgc.iSelAngle < 0 || pgc.iSelAngle > max(1, pgc.nAngles) || pgc.iSelAngle > 9) break;

			CAtlArray<vc_t>& angle = pgc.angles[pgc.iSelAngle];

			if(line.Find('v') >= 0)
			{
				int vob = 0, cell = 0;

				line += ' ';

				TCHAR* s = (LPTSTR)(LPCTSTR)line;
				TCHAR* e = s + line.GetLength();
				while(s < e)
				{
					if(*s == 'v' || s == e-1)
					{
						s++;
						if(vob != 0 && cell == 0)
						{
							for(int i = 0; i < angle.GetCount(); i++)
							{
								if(angle[i].vob == vob)
									m_rd.selvcs.Add((angle[i].vob<<16)|angle[i].cell);
							}
						}

						vob = _tcstol(s, &s, 10);
						cell = 0;
					}
					else if(*s == 'c' && vob > 0)
					{
						s++;
						cell = _tcstol(s, &s, 10);

						for(int i = 0; i < angle.GetCount(); i++)
						{
							if(angle[i].vob == vob && angle[i].cell == cell)
							{
								m_rd.selvcs.Add((vob<<16)|cell);
								break;
							}
						}
					}
					else
					{
						s++;
					}
				}
			}
			else
			{
				for(int i = 0; i < angle.GetCount(); i++)
					m_rd.selvcs.Add((angle[i].vob<<16)|angle[i].cell);
			}

			phase = P_LANGS;
		}
		else if(phase == 4)
		{
			if(!line.CompareNoCase(_T("ALL")))
			{
				for(int i = 0; i < 32; i++) m_rd.selids[i] = true;
				m_rd.fClosedCaption = true;
				phase = P_OPTIONS;
			}
			else
			{
				line += ' ';

				while(line.GetLength() > 0)
				{
					int n = line.Find(_T(" "));

					CString lang = line.Left(n);

					line = line.Mid(n);
					line.TrimLeft();

					n = 0;

					int langnum;

					if(_istdigit(lang[0])) 
					{
						n = _stscanf(lang, _T("%d"), &langnum);
						if(n != 1) break;

						m_rd.selids[langnum] = true;
					}
					else if(_istalpha(lang[0])) 
					{
						n = _stscanf(lang, _T("%s"), langid);
						if(n != 1) break;

						int id = (langid[0] << 8) + langid[1];

						if(id == 'cc')
						{
							m_rd.fClosedCaption = true;
						}
						else
						{
							m_rd.selids[id] = true;
						}
					}
					else break;

					if(n != 1) break;
				}

				if((m_rd.selids.GetCount() > 0 || m_rd.fClosedCaption) && line.IsEmpty())
					phase = P_OPTIONS;
			}
		}
		else if(phase == 5 && !line.CompareNoCase(_T("CLOSE")))
			m_rd.fClose = true;
		else if(phase == 5 && !line.CompareNoCase(_T("BEEP")))
			m_rd.fBeep = true;
		else if(phase == 5 && !line.CompareNoCase(_T("RESETTIME")))
			m_rd.fResetTime = true;
		else if(phase == 5 && !line.CompareNoCase(_T("FORCEDONLY")))
			m_rd.fForcedOnly = true;
		else if(phase == 5 && !line.CompareNoCase(_T("CLOSEIGNOREERRORS")))
			m_rd.fCloseIgnoreError = true;
		
	}

	m_rd.fAuto = true;

	return phase == P_OPTIONS ? S_OK : E_FAIL;
}

STDMETHODIMP CVobSubFileRipper::SetInput(CString infn)
{
	CAutoLock cAutoLock(&m_csAccessLock);

	m_rd.Reset();

	if(!LoadIfo(infn) || !LoadVob(infn))
		return E_INVALIDARG;

	m_infn = infn;

	return S_OK;
}

STDMETHODIMP CVobSubFileRipper::SetOutput(CString outfn)
{
	CAutoLock cAutoLock(&m_csAccessLock);
	m_outfn = outfn;
	return S_OK;
}

STDMETHODIMP CVobSubFileRipper::GetRipperData(VSFRipperData& rd)
{
	CAutoLock cAutoLock(&m_csAccessLock);
	rd.Copy(m_rd);
	return S_OK;
}

STDMETHODIMP CVobSubFileRipper::UpdateRipperData(VSFRipperData& rd)
{
	CAutoLock cAutoLock(&m_csAccessLock);
	m_rd.Copy(rd);
	return S_OK;
}

STDMETHODIMP CVobSubFileRipper::Index()
{
	if(m_fIndexing) return E_FAIL;
	CAMThread::CallWorker(CMD_INDEX);
	return S_OK;
}


STDMETHODIMP CVobSubFileRipper::IsIndexing()
{
	return m_fIndexing ? S_OK : S_FALSE;
}

STDMETHODIMP CVobSubFileRipper::Abort(bool fSavePartial)
{
	m_fBreakThread = true;
	return S_OK;
}

//

void VSFRipperData::Reset()
{
	vidsize.SetSize(0,0);
	memset(&vidinfo, 0, sizeof(vidinfo));
	pgcs.RemoveAll();
	iSelPGC = -1;
	fResetTime = fClosedCaption = true;
	fForcedOnly = false;
	fClose = fBeep = fAuto = false;
	fCloseIgnoreError = false;

	selvcs.RemoveAll();
	selids.RemoveAll();
}

void VSFRipperData::Copy(VSFRipperData& rd)
{
	Reset();

	vidsize = rd.vidsize;
	vidinfo = rd.vidinfo;
	if(int len = rd.pgcs.GetCount())
	{
		pgcs.SetCount(len);
		for(int i = 0; i < len; i++)
		{
			PGC& src = rd.pgcs[i];
			PGC& dst = pgcs[i];
			dst.nAngles = src.nAngles;
			for(int i = 0; i < countof(dst.angles); i++)
				dst.angles[i].Copy(src.angles[i]);
			dst.iSelAngle = src.iSelAngle;
			memcpy(dst.pal, src.pal, sizeof(src.pal));
			memcpy(dst.ids, src.ids, sizeof(src.ids));
		}
	}
	iSelPGC = rd.iSelPGC;
	fResetTime = rd.fResetTime;
	fClosedCaption = rd.fClosedCaption;
	fForcedOnly = rd.fForcedOnly;
	fClose = rd.fClose;
	fBeep = rd.fBeep;
	fAuto = rd.fAuto;
	fCloseIgnoreError = rd.fCloseIgnoreError;
	selvcs.Copy(rd.selvcs);
	POSITION pos = rd.selids.GetStartPosition();
	while(pos)
	{
		BYTE key;
		bool val;
		rd.selids.GetNextAssoc(pos, key, val);
		selids[key] = val;
	}
}

