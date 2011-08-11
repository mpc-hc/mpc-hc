/*
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
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

#include "stdafx.h"
#include "MatroskaFile.h"
#include "../../../DSUtil/DSUtil.h"
#include <zlib/zlib.h>

#define DOCTYPE _T("matroska")
#define DOCTYPE_WEBM _T("webm")
#define DOCTYPEVERSION 2

static void LOG(LPCTSTR fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if(FILE* f = _tfopen(_T("c:\\matroskasplitterlog.txt"), _T("at"))) {
		fseek(f, 0, 2);
		_vftprintf(f, fmt, args);
		fclose(f);
	}
	va_end(args);
}

using namespace MatroskaReader;

#define BeginChunk	\
	CheckPointer(pMN0, E_POINTER); \
\
	CAutoPtr<CMatroskaNode> pMN = pMN0->Child(); \
	if(!pMN) return S_FALSE; \
\
	do \
	{ \
		switch(pMN->m_id) \
		{ \
 
#define EndChunk \
		} \
	} \
	while(pMN->Next()); \
\
	return S_OK; \
 
static void bswap(BYTE* s, int len)
{
	for(BYTE* d = s + len-1; s < d; s++, d--) {
		*s ^= *d, *d ^= *s, *s ^= *d;
	}
}

//
// CMatroskaFile
//

CMatroskaFile::CMatroskaFile(IAsyncReader* pAsyncReader, HRESULT& hr)
	: CBaseSplitterFile(pAsyncReader, hr, DEFAULT_CACHE_LENGTH, false)
	, m_rtOffset(0)
{
	if(FAILED(hr)) {
		return;
	}
	hr = Init();
}

HRESULT CMatroskaFile::Init()
{
	DWORD dw;
	if(FAILED(Read(dw)) || dw != 0x1A45DFA3) {
		return E_FAIL;
	}

	CMatroskaNode Root(this);
	if(FAILED(Parse(&Root))) {
		return E_FAIL;
	}

	CAutoPtr<CMatroskaNode> pSegment, pCluster;
	if((pSegment = Root.Child(0x18538067))
			&& (pCluster = pSegment->Child(0x1F43B675))) {
		Cluster c0;
		c0.ParseTimeCode(pCluster);
		m_rtOffset = m_segment.GetRefTime(c0.TimeCode);
	}

	return S_OK;
}

template <class T>
HRESULT CMatroskaFile::Read(T& var)
{
	HRESULT hr = ByteRead((BYTE*)&var, sizeof(var));
	if(S_OK == hr) {
		bswap((BYTE*)&var, sizeof(var));
	}
	return hr;
}

HRESULT CMatroskaFile::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x1A45DFA3:
	m_ebml.Parse(pMN);
	if((m_ebml.DocType != DOCTYPE || m_ebml.DocTypeReadVersion > DOCTYPEVERSION) &&  m_ebml.DocType != DOCTYPE_WEBM) {
		return E_FAIL;
	}
	break;
case 0x18538067:
	if(m_segment.SegmentInfo.SegmentUID.IsEmpty()) {
		m_segment.ParseMinimal(pMN);
	}
	break;
	EndChunk
}

//

HRESULT EBML::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x4286:
	EBMLVersion.Parse(pMN);
	break;
case 0x42F7:
	EBMLReadVersion.Parse(pMN);
	break;
case 0x42F2:
	EBMLMaxIDLength.Parse(pMN);
	break;
case 0x42F3:
	EBMLMaxSizeLength.Parse(pMN);
	break;
case 0x4282:
	DocType.Parse(pMN);
	break;
case 0x4287:
	DocTypeVersion.Parse(pMN);
	break;
case 0x4285:
	DocTypeReadVersion.Parse(pMN);
	break;
	EndChunk
}

HRESULT Segment::Parse(CMatroskaNode* pMN0)
{
	pos = pMN0->GetPos();

	BeginChunk
case 0x1549A966:
	SegmentInfo.Parse(pMN);
	break;
case 0x114D9B74:
	MetaSeekInfo.Parse(pMN);
	break;
case 0x1654AE6B:
	Tracks.Parse(pMN);
	break;
case 0x1F43B675:
	Clusters.Parse(pMN);
	break;
case 0x1C53BB6B:
	Cues.Parse(pMN);
	break;
case 0x1941A469:
	Attachments.Parse(pMN);
	break;
case 0x1043A770:
	Chapters.Parse(pMN);
	break;
	//	case 0x1254C367: Tags.Parse(pMN); break;
	EndChunk
}

HRESULT Segment::ParseMinimal(CMatroskaNode* pMN0)
{
	CheckPointer(pMN0, E_POINTER);

	pos = pMN0->GetPos();
	len = pMN0->m_len;

	CAutoPtr<CMatroskaNode> pMN = pMN0->Child();
	if(!pMN) {
		return S_FALSE;
	}

	int n = 0;

	do {
		switch(pMN->m_id) {
			case 0x1549A966:
				SegmentInfo.Parse(pMN);
				n++;
				break;
			case 0x114D9B74:
				MetaSeekInfo.Parse(pMN);
				n++;
				break;
			case 0x1654AE6B:
				Tracks.Parse(pMN);
				n++;
				break;
			case 0x1C53BB6B:
				Cues.Parse(pMN);
				break;
		}
	} while(n < 3 && pMN->Next());

	if(!pMN->IsRandomAccess()) {
		return S_OK;
	}

	while(MatroskaReader::QWORD pos = pMN->FindPos(0x114D9B74, pMN->GetPos())) {
		pMN->SeekTo(pos);
		pMN->Parse();
		if(pMN->m_id != 0x114D9B74) {
			ASSERT(0);
			break;
		}
		MetaSeekInfo.Parse(pMN);
	}

	if(n == 3) {
		if(Cues.IsEmpty() && (pMN = pMN0->Child(0x1C53BB6B, false))) {
			do {
				Cues.Parse(pMN);
			} while(pMN->Next(true));
		}

		if(Chapters.IsEmpty() && (pMN = pMN0->Child(0x1043A770, false))) {
			do {
				Chapters.Parse(pMN); /*BIG UGLY HACK:*/
				break;
			} while(pMN->Next(true));
		}

		if(Attachments.IsEmpty() && (pMN = pMN0->Child(0x1941A469, false))) {
			do {
				Attachments.Parse(pMN); /*BIG UGLY HACK:*/
				break;
			} while (pMN->Next(true));
		}
	}

	return S_OK;
}

UINT64 Segment::GetMasterTrack()
{
	UINT64 TrackNumber = 0, AltTrackNumber = 0;

	POSITION pos1 = Tracks.GetHeadPosition();
	while(pos1 && TrackNumber == 0) {
		Track* pT = Tracks.GetNext(pos1);

		POSITION pos2 = pT->TrackEntries.GetHeadPosition();
		while(pos2 && TrackNumber == 0) {
			TrackEntry* pTE = pT->TrackEntries.GetNext(pos2);

			if(pTE->TrackType == TrackEntry::TypeVideo) {
				TrackNumber = pTE->TrackNumber;
				break;
			} else if(pTE->TrackType == TrackEntry::TypeAudio && AltTrackNumber == 0) {
				AltTrackNumber = pTE->TrackNumber;
			}
		}
	}

	if(TrackNumber == 0) {
		TrackNumber = AltTrackNumber;
	}
	if(TrackNumber == 0) {
		TrackNumber = 1;
	}

	return TrackNumber;
}

ChapterAtom* ChapterAtom::FindChapterAtom(UINT64 id)
{
	if(ChapterUID == id) {
		return(this);
	}

	POSITION pos = ChapterAtoms.GetHeadPosition();
	while(pos) {
		ChapterAtom* ca = ChapterAtoms.GetNext(pos)->FindChapterAtom(id);
		if(ca) {
			return ca;
		}
	}

	return(NULL);
}

ChapterAtom* Segment::FindChapterAtom(UINT64 id, int nEditionEntry)
{
	POSITION pos1 = Chapters.GetHeadPosition();
	while(pos1) {
		Chapter* c = Chapters.GetNext(pos1);

		POSITION pos2 = c->EditionEntries.GetHeadPosition();
		while(pos2) {
			EditionEntry* ee = c->EditionEntries.GetNext(pos2);

			if(nEditionEntry-- == 0) {
				return id == 0 ? ee : ee->FindChapterAtom(id);
			}
		}
	}

	return(NULL);
}

HRESULT Info::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x73A4:
	SegmentUID.Parse(pMN);
	break;
case 0x7384:
	SegmentFilename.Parse(pMN);
	break;
case 0x3CB923:
	PrevUID.Parse(pMN);
	break;
case 0x3C83AB:
	PrevFilename.Parse(pMN);
	break;
case 0x3EB923:
	NextUID.Parse(pMN);
	break;
case 0x3E83BB:
	NextFilename.Parse(pMN);
	break;
case 0x2AD7B1:
	TimeCodeScale.Parse(pMN);
	break;
case 0x4489:
	Duration.Parse(pMN);
	break;
case 0x4461:
	DateUTC.Parse(pMN);
	break;
case 0x7BA9:
	Title.Parse(pMN);
	break;
case 0x4D80:
	MuxingApp.Parse(pMN);
	break;
case 0x5741:
	WritingApp.Parse(pMN);
	break;
	EndChunk
}

HRESULT Seek::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x4DBB:
	SeekHeads.Parse(pMN);
	break;
	EndChunk
}

HRESULT SeekHead::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x53AB:
	SeekID.Parse(pMN);
	break;
case 0x53AC:
	SeekPosition.Parse(pMN);
	break;
	EndChunk
}

HRESULT Track::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0xAE:
	TrackEntries.Parse(pMN);
	break;
	EndChunk
}

HRESULT TrackEntry::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0xD7:
	TrackNumber.Parse(pMN);
	break;
case 0x73C5:
	TrackUID.Parse(pMN);
	break;
case 0x83:
	TrackType.Parse(pMN);
	break;
case 0xB9:
	FlagEnabled.Parse(pMN);
	break;
case 0x88:
	FlagDefault.Parse(pMN);
	break;
case 0x9C:
	FlagLacing.Parse(pMN);
	break;
case 0x55AA:
	FlagForced.Parse(pMN);
	break;
case 0x6DE7:
	MinCache.Parse(pMN);
	break;
case 0x6DF8:
	MaxCache.Parse(pMN);
	break;
case 0x536E:
	Name.Parse(pMN);
	break;
case 0x22B59C:
	Language.Parse(pMN);
	break;
case 0x86:
	CodecID.Parse(pMN);
	break;
case 0x63A2:
	CodecPrivate.Parse(pMN);
	break;
case 0x258688:
	CodecName.Parse(pMN);
	break;
case 0x3A9697:
	CodecSettings.Parse(pMN);
	break;
case 0x3B4040:
	CodecInfoURL.Parse(pMN);
	break;
case 0x26B240:
	CodecDownloadURL.Parse(pMN);
	break;
case 0xAA:
	CodecDecodeAll.Parse(pMN);
	break;
case 0x6FAB:
	TrackOverlay.Parse(pMN);
	break;
case 0x23E383:
case 0x2383E3:
	DefaultDuration.Parse(pMN);
	break;
case 0x23314F:
	TrackTimecodeScale.Parse(pMN);
	break;
case 0xE0:
	if(S_OK == v.Parse(pMN)) {
		DescType |= DescVideo;
	}
	break;
case 0xE1:
	if(S_OK == a.Parse(pMN)) {
		DescType |= DescAudio;
	}
	break;
case 0x6D80:
	ces.Parse(pMN);
	break;
	EndChunk
}

static int cesort(const void* a, const void* b)
{
	UINT64 ce1 = (static_cast<ContentEncoding*>(const_cast<void *>(a)))->ContentEncodingOrder;
	UINT64 ce2 = (static_cast<ContentEncoding*>(const_cast<void *>(b)))->ContentEncodingOrder;

	return (int)ce1 - (int)ce2;
	//return static_cast<int>(ce1) - static_cast<int>(ce2);
}

bool TrackEntry::Expand(CBinary& data, UINT64 Scope)
{
	if(ces.ce.GetCount() == 0) {
		return true;
	}

	CAtlArray<ContentEncoding*> cearray;
	POSITION pos = ces.ce.GetHeadPosition();
	while(pos) {
		cearray.Add(ces.ce.GetNext(pos));
	}
	qsort(cearray.GetData(), cearray.GetCount(), sizeof(ContentEncoding*), cesort);

	for(int i = cearray.GetCount()-1; i >= 0; i--) {
		ContentEncoding* ce = cearray[i];

		if(!(ce->ContentEncodingScope & Scope)) {
			continue;
		}

		if(ce->ContentEncodingType == ContentEncoding::Compression) {
			if(!data.Decompress(ce->cc)) {
				return false;
			}
		} else if(ce->ContentEncodingType == ContentEncoding::Encryption) {
			// TODO
			return false;
		}
	}

	return true;
}

HRESULT Video::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x9A:
	FlagInterlaced.Parse(pMN);
	break;
case 0x53B8:
	StereoMode.Parse(pMN);
	break;
case 0xB0:
	PixelWidth.Parse(pMN);
	if (!DisplayWidth) {
		DisplayWidth.Set(PixelWidth);
	}
	break;
case 0xBA:
	PixelHeight.Parse(pMN);
	if (!DisplayHeight) {
		DisplayHeight.Set(PixelHeight);
	}
	break;
case 0x54B0:
	DisplayWidth.Parse(pMN);
	break;
case 0x54BA:
	DisplayHeight.Parse(pMN);
	break;
case 0x54B2:
	DisplayUnit.Parse(pMN);
	break;
case 0x54B3:
	AspectRatioType.Parse(pMN);
	break;
case 0x2EB524:
	ColourSpace.Parse(pMN);
	break;
case 0x2FB523:
	GammaValue.Parse(pMN);
	break;
case 0x2383E3:
	FramePerSec.Parse(pMN);
	break;
	EndChunk
}

HRESULT Audio::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0xB5:
	SamplingFrequency.Parse(pMN);
	if (!OutputSamplingFrequency) {
		OutputSamplingFrequency.Set(SamplingFrequency);
	}
	break;
case 0x78B5:
	OutputSamplingFrequency.Parse(pMN);
	break;
case 0x9F:
	Channels.Parse(pMN);
	break;
case 0x7D7B:
	ChannelPositions.Parse(pMN);
	break;
case 0x6264:
	BitDepth.Parse(pMN);
	break;
	EndChunk
}

HRESULT ContentEncodings::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x6240:
	ce.Parse(pMN);
	break;
	EndChunk
}

HRESULT ContentEncoding::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x5031:
	ContentEncodingOrder.Parse(pMN);
	break;
case 0x5032:
	ContentEncodingScope.Parse(pMN);
	break;
case 0x5033:
	ContentEncodingType.Parse(pMN);
	break;
case 0x5034:
	cc.Parse(pMN);
	break;
case 0x5035:
	ce.Parse(pMN);
	break;
	EndChunk
}

HRESULT ContentCompression::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x4254:
	ContentCompAlgo.Parse(pMN);
	break;
case 0x4255:
	ContentCompSettings.Parse(pMN);
	break;
	EndChunk
}

HRESULT ContentEncryption::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x47e1:
	ContentEncAlgo.Parse(pMN);
	break;
case 0x47e2:
	ContentEncKeyID.Parse(pMN);
	break;
case 0x47e3:
	ContentSignature.Parse(pMN);
	break;
case 0x47e4:
	ContentSigKeyID.Parse(pMN);
	break;
case 0x47e5:
	ContentSigAlgo.Parse(pMN);
	break;
case 0x47e6:
	ContentSigHashAlgo.Parse(pMN);
	break;
	EndChunk
}

HRESULT Cluster::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0xE7:
	TimeCode.Parse(pMN);
	break;
case 0xA7:
	Position.Parse(pMN);
	break;
case 0xAB:
	PrevSize.Parse(pMN);
	break;
case 0xA0:
	BlockGroups.Parse(pMN, true);
	break;
case 0xA3:
	SimpleBlocks.Parse(pMN, true);
	break;
	EndChunk
}

HRESULT Cluster::ParseTimeCode(CMatroskaNode* pMN0)
{
	BeginChunk
case 0xE7:
	TimeCode.Parse(pMN);
	return S_OK;
	EndChunk
}

HRESULT BlockGroup::Parse(CMatroskaNode* pMN0, bool fFull)
{
	BeginChunk
case 0xA1:
	Block.Parse(pMN, fFull);
	break;
case 0xA2: /* TODO: multiple virt blocks? */
	;
	break;
case 0x9B:
	BlockDuration.Parse(pMN);
	break;
case 0xFA:
	ReferencePriority.Parse(pMN);
	break;
case 0xFB:
	ReferenceBlock.Parse(pMN);
	break;
case 0xFD:
	ReferenceVirtual.Parse(pMN);
	break;
case 0xA4:
	CodecState.Parse(pMN);
	break;
case 0xE8:
	TimeSlices.Parse(pMN);
	break;
case 0x75A1:
	if(fFull) {
		ba.Parse(pMN);
	}
	break;
	EndChunk
}

HRESULT SimpleBlock::Parse(CMatroskaNode* pMN, bool fFull)
{
	pMN->SeekTo(pMN->m_start);

	TrackNumber.Parse(pMN);
	CShort s;
	s.Parse(pMN);
	TimeCode.Set(s);
	Lacing.Parse(pMN);

	if(!fFull) {
		return S_OK;
	}

	CAtlList<MatroskaReader::QWORD> lens;
	MatroskaReader::QWORD tlen = 0;
	MatroskaReader::QWORD FrameSize;
	BYTE FramesInLaceLessOne;

	switch((Lacing & 0x06) >> 1) {
		case 0:
			// No lacing
			lens.AddTail((pMN->m_start+pMN->m_len) - (pMN->GetPos()+tlen));
			break;
		case 1:
			// Xiph lacing
			BYTE n;
			pMN->Read(n);
			while(n-- > 0) {
				BYTE b;
				MatroskaReader::QWORD len = 0;
				do {
					pMN->Read(b);
					len += b;
				} while(b == 0xff);
				lens.AddTail(len);
				tlen += len;
			}
			lens.AddTail((pMN->m_start+pMN->m_len) - (pMN->GetPos()+tlen));
			break;
		case 2:
			// Fixed-size lacing
			pMN->Read(FramesInLaceLessOne);
			FramesInLaceLessOne++;
			FrameSize = ((pMN->m_start+pMN->m_len) - (pMN->GetPos()+tlen)) / FramesInLaceLessOne;
			while(FramesInLaceLessOne-- > 0) {
				lens.AddTail(FrameSize);
			}
			break;
		case 3:
			// EBML lacing
			pMN->Read(FramesInLaceLessOne);

			CLength FirstFrameSize;
			FirstFrameSize.Parse(pMN);
			lens.AddTail(FirstFrameSize);
			FramesInLaceLessOne--;
			tlen = FirstFrameSize;

			CSignedLength DiffSize;
			FrameSize = FirstFrameSize;
			while(FramesInLaceLessOne--) {
				DiffSize.Parse(pMN);
				FrameSize += DiffSize;
				lens.AddTail(FrameSize);
				tlen += FrameSize;
			}
			lens.AddTail((pMN->m_start+pMN->m_len) - (pMN->GetPos()+tlen));
			break;
	}

	POSITION pos = lens.GetHeadPosition();
	while(pos) {
		MatroskaReader::QWORD len = lens.GetNext(pos);
		CAutoPtr<CBinary> p(DNew CBinary());
		p->SetCount((INT_PTR)len);
		pMN->Read(p->GetData(), len);
		BlockData.AddTail(p);
	}

	return S_OK;
}

HRESULT BlockAdditions::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0xA6:
	bm.Parse(pMN);
	break;
	EndChunk
}

HRESULT BlockMore::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0xEE:
	BlockAddID.Parse(pMN);
	break;
case 0xA5:
	BlockAdditional.Parse(pMN);
	break;
	EndChunk
}

HRESULT TimeSlice::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0xCC:
	LaceNumber.Parse(pMN);
	break;
case 0xCD:
	FrameNumber.Parse(pMN);
	break;
case 0xCE:
	Delay.Parse(pMN);
	break;
case 0xCF:
	Duration.Parse(pMN);
	break;
	EndChunk
}

HRESULT Cue::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0xBB:
	CuePoints.Parse(pMN);
	break;
	EndChunk
}

HRESULT CuePoint::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0xB3:
	CueTime.Parse(pMN);
	break;
case 0xB7:
	CueTrackPositions.Parse(pMN);
	break;
	EndChunk
}

HRESULT CueTrackPosition::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0xF7:
	CueTrack.Parse(pMN);
	break;
case 0xF1:
	CueClusterPosition.Parse(pMN);
	break;
case 0x5387:
	CueBlockNumber.Parse(pMN);
	break;
case 0xEA:
	CueCodecState.Parse(pMN);
	break;
case 0xDB:
	CueReferences.Parse(pMN);
	break;
	EndChunk
}

HRESULT CueReference::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x96:
	CueRefTime.Parse(pMN);
	break;
case 0x97:
	CueRefCluster.Parse(pMN);
	break;
case 0x535F:
	CueRefNumber.Parse(pMN);
	break;
case 0xEB:
	CueRefCodecState.Parse(pMN);
	break;
	EndChunk
}

HRESULT Attachment::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x61A7:
	AttachedFiles.Parse(pMN);
	break;
	EndChunk
}

HRESULT AttachedFile::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x467E:
	FileDescription.Parse(pMN);
	break;
case 0x466E:
	FileName.Parse(pMN);
	break;
case 0x4660:
	FileMimeType.Parse(pMN);
	break;
case 0x465C: // binary
	FileDataLen = (INT_PTR)pMN->m_len;
	FileDataPos = pMN->m_start;
	break;
case 0x46AE:
	FileUID.Parse(pMN);
	break;
	EndChunk
}

HRESULT Chapter::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x45B9:
	EditionEntries.Parse(pMN);
	break;
	EndChunk
}

HRESULT EditionEntry::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0xB6:
	ChapterAtoms.Parse(pMN);
	break;
	EndChunk
}

HRESULT ChapterAtom::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x73C4:
	ChapterUID.Parse(pMN);
	break;
case 0x91:
	ChapterTimeStart.Parse(pMN);
	break;
case 0x92:
	ChapterTimeEnd.Parse(pMN);
	break;
	//	case 0x8F: // TODO
case 0x80:
	ChapterDisplays.Parse(pMN);
	break;
case 0xB6:
	ChapterAtoms.Parse(pMN);
	break;
case 0x98:
	ChapterFlagHidden.Parse(pMN);
	break;
case 0x4598:
	ChapterFlagEnabled.Parse(pMN);
	break;
	EndChunk
}

HRESULT ChapterDisplay::Parse(CMatroskaNode* pMN0)
{
	BeginChunk
case 0x85:
	ChapString.Parse(pMN);
	break;
case 0x437C:
	ChapLanguage.Parse(pMN);
	break;
case 0x437E:
	ChapCountry.Parse(pMN);
	break;
	EndChunk
}

//

HRESULT CBinary::Parse(CMatroskaNode* pMN)
{
	ASSERT(pMN->m_len <= INT_MAX);
	SetCount((INT_PTR)pMN->m_len);
	return pMN->Read(GetData(), pMN->m_len);
}

bool CBinary::Compress(ContentCompression& cc)
{
	if(cc.ContentCompAlgo == ContentCompression::ZLIB) {
		int res;
		z_stream c_stream;

		c_stream.zalloc = (alloc_func)0;
		c_stream.zfree = (free_func)0;
		c_stream.opaque = (voidpf)0;

		if(Z_OK != (res = deflateInit(&c_stream, 9))) {
			return false;
		}

		c_stream.next_in = GetData();
		c_stream.avail_in = GetCount();

		BYTE* dst = NULL;
		int n = 0;
		do {
			dst = (BYTE*)realloc(dst, ++n*10);
			c_stream.next_out = &dst[(n-1)*10];
			c_stream.avail_out = 10;
			if(Z_OK != (res = deflate(&c_stream, Z_FINISH)) && Z_STREAM_END != res) {
				free(dst);
				return false;
			}
		} while(0 == c_stream.avail_out && Z_STREAM_END != res);

		deflateEnd(&c_stream);

		SetCount(c_stream.total_out);
		memcpy(GetData(), dst, GetCount());

		free(dst);

		return true;
	}

	return false;
}

bool CBinary::Decompress(ContentCompression& cc)
{
	if(cc.ContentCompAlgo == ContentCompression::ZLIB) {
		int res;
		z_stream d_stream;

		d_stream.zalloc = (alloc_func)0;
		d_stream.zfree = (free_func)0;
		d_stream.opaque = (voidpf)0;

		if(Z_OK != (res = inflateInit(&d_stream))) {
			return false;
		}

		d_stream.next_in = GetData();
		d_stream.avail_in = GetCount();

		BYTE* dst = NULL;
		int n = 0;
		do {
			dst = (unsigned char *)realloc(dst, ++n*1000);
			d_stream.next_out = &dst[(n-1)*1000];
			d_stream.avail_out = 1000;
			if(Z_OK != (res = inflate(&d_stream, Z_NO_FLUSH)) && Z_STREAM_END != res) {
				free(dst);
				return false;
			}
		} while(0 == d_stream.avail_out && 0 != d_stream.avail_in && Z_STREAM_END != res);

		inflateEnd(&d_stream);

		SetCount(d_stream.total_out);
		memcpy(GetData(), dst, GetCount());

		free(dst);

		return true;
	} else if(cc.ContentCompAlgo == ContentCompression::HDRSTRIP) {
		InsertArrayAt(0, &cc.ContentCompSettings);
	}

	return false;
}

HRESULT CANSI::Parse(CMatroskaNode* pMN)
{
	Empty();

	MatroskaReader::QWORD len = pMN->m_len;
	CHAR c;
	while(len-- > 0 && SUCCEEDED(pMN->Read(c))) {
		*this += c;
	}

	return(len == -1 ? S_OK : E_FAIL);
}

HRESULT CUTF8::Parse(CMatroskaNode* pMN)
{
	Empty();
	CAutoVectorPtr<BYTE> buff;
	if(!buff.Allocate((UINT)pMN->m_len + 1) || S_OK != pMN->Read(buff, pMN->m_len)) {
		return E_FAIL;
	}
	buff[pMN->m_len] = 0;
	CStringW::operator = (UTF8To16((LPCSTR)(BYTE*)buff));
	return S_OK;
}

HRESULT CUInt::Parse(CMatroskaNode* pMN)
{
	m_val = 0;
	for(int i = 0; i < (int)pMN->m_len; i++) {
		m_val <<= 8;
		HRESULT hr = pMN->Read(*(BYTE*)&m_val);
		if(FAILED(hr)) {
			return hr;
		}
	}
	m_fValid = true;
	return S_OK;
}

HRESULT CInt::Parse(CMatroskaNode* pMN)
{
	m_val = 0;
	for(int i = 0; i < (int)pMN->m_len; i++) {
		HRESULT hr = pMN->Read(*((BYTE*)&m_val+7-i));
		if(FAILED(hr)) {
			return hr;
		}
	}
	m_val >>= (8-pMN->m_len)*8;
	m_fValid = true;
	return S_OK;
}

HRESULT CFloat::Parse(CMatroskaNode* pMN)
{
	HRESULT hr = E_FAIL;
	m_val = 0;

	if(pMN->m_len == 4) {
		float val = 0;
		hr = pMN->Read(val);
		m_val = val;
	} else if(pMN->m_len == 8) {
		hr = pMN->Read(m_val);
	}
	if(SUCCEEDED(hr)) {
		m_fValid = true;
	}
	return hr;
}


template<class T, class BASE>
HRESULT CSimpleVar<T, BASE>::Parse(CMatroskaNode* pMN)
{
	m_val = 0;
	m_fValid = true;
	return pMN->Read(m_val);
}

HRESULT CID::Parse(CMatroskaNode* pMN)
{
	m_val = 0;

	BYTE b = 0;
	HRESULT hr = pMN->Read(b);
	if(FAILED(hr)) {
		return hr;
	}

	int nMoreBytes = 0;

	if((b&0x80) == 0x80) {
		m_val = b&0xff;
		nMoreBytes = 0;
	} else if((b&0xc0) == 0x40) {
		m_val = b&0x7f;
		nMoreBytes = 1;
	} else if((b&0xe0) == 0x20) {
		m_val = b&0x3f;
		nMoreBytes = 2;
	} else if((b&0xf0) == 0x10) {
		m_val = b&0x1f;
		nMoreBytes = 3;
	} else {
		return E_FAIL;
	}

	while(nMoreBytes-- > 0) {
		m_val <<= 8;
		hr = pMN->Read(*(BYTE*)&m_val);
		if(FAILED(hr)) {
			return hr;
		}
	}

	m_fValid = true;

	return S_OK;
}

HRESULT CLength::Parse(CMatroskaNode* pMN)
{
	m_val = 0;

	BYTE b = 0;
	HRESULT hr = pMN->Read(b);
	if(FAILED(hr)) {
		return hr;
	}

	int nMoreBytes = 0;

	if((b&0x80) == 0x80) {
		m_val = b&0x7f;
		nMoreBytes = 0;
	} else if((b&0xc0) == 0x40) {
		m_val = b&0x3f;
		nMoreBytes = 1;
	} else if((b&0xe0) == 0x20) {
		m_val = b&0x1f;
		nMoreBytes = 2;
	} else if((b&0xf0) == 0x10) {
		m_val = b&0x0f;
		nMoreBytes = 3;
	} else if((b&0xf8) == 0x08) {
		m_val = b&0x07;
		nMoreBytes = 4;
	} else if((b&0xfc) == 0x04) {
		m_val = b&0x03;
		nMoreBytes = 5;
	} else if((b&0xfe) == 0x02) {
		m_val = b&0x01;
		nMoreBytes = 6;
	} else if((b&0xff) == 0x01) {
		m_val = b&0x00;
		nMoreBytes = 7;
	} else {
		return E_FAIL;
	}

	//int nMoreBytesTmp = nMoreBytes;

	MatroskaReader::QWORD UnknownSize = (1i64<<(7*(nMoreBytes+1)))-1;

	while(nMoreBytes-- > 0) {
		m_val <<= 8;
		hr = pMN->Read(*(BYTE*)&m_val);
		if(FAILED(hr)) {
			return hr;
		}
	}

	if(m_val == UnknownSize) {
		m_val = pMN->GetLength() - pMN->GetPos();
		TRACE(_T("CLength: Unspecified chunk size at %I64d (corrected to %I64d)\n"), pMN->GetPos(), m_val);
	}

	if(m_fSigned) {
		m_val -= (UnknownSize >> 1);
	}

	m_fValid = true;

	return S_OK;
}
/*
HRESULT CSignedLength::Parse(CMatroskaNode* pMN)
{
//	HRESULT hr = __super::Parse(pMN);
//	if(FAILED(hr)) return hr;

	m_val = 0;

	BYTE b = 0;
	HRESULT hr = pMN->Read(b);
	if(FAILED(hr)) return hr;

	int nMoreBytes = 0;

	if((b&0x80) == 0x80) {m_val = b&0x7f; nMoreBytes = 0;}
	else if((b&0xc0) == 0x40) {m_val = b&0x3f; nMoreBytes = 1;}
	else if((b&0xe0) == 0x20) {m_val = b&0x1f; nMoreBytes = 2;}
	else if((b&0xf0) == 0x10) {m_val = b&0x0f; nMoreBytes = 3;}
	else if((b&0xf8) == 0x08) {m_val = b&0x07; nMoreBytes = 4;}
	else if((b&0xfc) == 0x04) {m_val = b&0x03; nMoreBytes = 5;}
	else if((b&0xfe) == 0x02) {m_val = b&0x01; nMoreBytes = 6;}
	else if((b&0xff) == 0x01) {m_val = b&0x00; nMoreBytes = 7;}
	else return E_FAIL;

	//int nMoreBytesTmp = nMoreBytes;

	MatroskaReader::QWORD UnknownSize = (1i64<<(7*(nMoreBytes+1)))-1;

	while(nMoreBytes-- > 0)
	{
		m_val <<= 8;
		hr = pMN->Read(*(BYTE*)&m_val);
		if(FAILED(hr)) return hr;
	}

	if(m_val == UnknownSize)
	{
		m_val = pMN->GetLength() - pMN->GetPos();
		TRACE(_T("CLength: Unspecified chunk size at %I64d (corrected to %I64d)\n"), pMN->GetPos(), m_val);
	}

	m_val -= (UnknownSize >> 1);

	m_fValid = true;

	return S_OK;
}
*/
template<class T>
HRESULT CNode<T>::Parse(CMatroskaNode* pMN)
{
	CAutoPtr<T> p(DNew T());
	HRESULT hr = E_OUTOFMEMORY;
	if(!p || FAILED(hr = p->Parse(pMN))) {
		return hr;
	}
	AddTail(p);
	return S_OK;
}

HRESULT CBlockGroupNode::Parse(CMatroskaNode* pMN, bool fFull)
{
	CAutoPtr<BlockGroup> p(DNew BlockGroup());
	HRESULT hr = E_OUTOFMEMORY;
	if(!p || FAILED(hr = p->Parse(pMN, fFull))) {
		return hr;
	}
	AddTail(p);
	return S_OK;
}

HRESULT CSimpleBlockNode::Parse(CMatroskaNode* pMN, bool fFull)
{
	CAutoPtr<SimpleBlock> p(DNew SimpleBlock());
	HRESULT hr = E_OUTOFMEMORY;
	if(!p || FAILED(hr = p->Parse(pMN, fFull))) {
		return hr;
	}
	AddTail(p);
	return S_OK;
}

///////////////////////////////

CMatroskaNode::CMatroskaNode(CMatroskaFile* pMF)
	: m_pMF(pMF)
	, m_pParent(NULL)
{
	ASSERT(m_pMF);
	m_start = m_filepos = 0;
	m_len.Set(m_pMF ? m_pMF->GetLength() : 0);
}

CMatroskaNode::CMatroskaNode(CMatroskaNode* pParent)
	: m_pMF(pParent->m_pMF)
	, m_pParent(pParent)
{
	Parse();
}

HRESULT CMatroskaNode::Parse()
{
	m_filepos = GetPos();
	if(FAILED(m_id.Parse(this)) || FAILED(m_len.Parse(this))) {
		return E_FAIL;
	}
	m_start = GetPos();
	return S_OK;
}

CAutoPtr<CMatroskaNode> CMatroskaNode::Child(DWORD id, bool fSearch)
{
	if(m_len == 0) {
		return CAutoPtr<CMatroskaNode>();
	}
	SeekTo(m_start);
	CAutoPtr<CMatroskaNode> pMN(DNew CMatroskaNode(this));
	if(id && !pMN->Find(id, fSearch)) {
		pMN.Free();
	}
	return pMN;
}

bool CMatroskaNode::Next(bool fSame)
{
	if(!m_pParent) {
		return false;
	}

	CID id = m_id;

	while(m_start+m_len < m_pParent->m_start+m_pParent->m_len) {
		SeekTo(m_start+m_len);

		if(FAILED(Parse())) {
			if(!Resync()) {
				return false;
			}
		}

		if(!fSame || m_id == id) {
			return true;
		}
	}

	return false;
}

bool CMatroskaNode::Find(DWORD id, bool fSearch)
{
	MatroskaReader::QWORD pos = m_pParent && m_pParent->m_id == 0x18538067 /*segment?*/
								? FindPos(id)
								: 0;

	if(pos) {
		SeekTo(pos);
		Parse();
	} else if(fSearch) {
		while(m_id != id && Next()) {
			;
		}
	}

	return(m_id == id);
}

void CMatroskaNode::SeekTo(MatroskaReader::QWORD pos)
{
	m_pMF->Seek(pos);
}

MatroskaReader::QWORD CMatroskaNode::GetPos()
{
	return m_pMF->GetPos();
}

MatroskaReader::QWORD CMatroskaNode::GetLength()
{
	return m_pMF->GetLength();
}

template <class T>
HRESULT CMatroskaNode::Read(T& var)
{
	return m_pMF->Read(var);
}

HRESULT CMatroskaNode::Read(BYTE* pData, MatroskaReader::QWORD len)
{
	return m_pMF->ByteRead(pData, len);
}

MatroskaReader::QWORD CMatroskaNode::FindPos(DWORD id, MatroskaReader::QWORD start)
{
	Segment& sm = m_pMF->m_segment;

	POSITION pos = sm.MetaSeekInfo.GetHeadPosition();
	while(pos) {
		Seek* s = sm.MetaSeekInfo.GetNext(pos);

		POSITION pos2 = s->SeekHeads.GetHeadPosition();
		while(pos2) {
			SeekHead* sh = s->SeekHeads.GetNext(pos2);
			if(sh->SeekID == id && sh->SeekPosition+sm.pos >= start) {
				return sh->SeekPosition+sm.pos;
			}
		}
	}

	return 0;
}

CAutoPtr<CMatroskaNode> CMatroskaNode::Copy()
{
	CAutoPtr<CMatroskaNode> pNewNode(DNew CMatroskaNode(m_pMF));
	pNewNode->m_pParent = m_pParent;
	pNewNode->m_id.Set(m_id);
	pNewNode->m_len.Set(m_len);
	pNewNode->m_filepos = m_filepos;
	pNewNode->m_start = m_start;
	return(pNewNode);
}

CAutoPtr<CMatroskaNode> CMatroskaNode::GetFirstBlock()
{
	CAutoPtr<CMatroskaNode> pNode = Child();
	do {
		if(pNode->m_id == 0xA0 || pNode->m_id == 0xA3) {
			return pNode;
		}
	} while(pNode->Next());
	return CAutoPtr<CMatroskaNode>();
}

bool CMatroskaNode::NextBlock()
{
	if(!m_pParent) {
		return false;
	}

	CID id = m_id;

	while(m_start+m_len < m_pParent->m_start+m_pParent->m_len) {
		SeekTo(m_start+m_len);

		if(FAILED(Parse())) {
			if(!Resync()) {
				return false;
			}
		}

		if(m_id == 0xA0 || m_id == 0xA3) {
			return true;
		}
	}

	return false;
}

bool CMatroskaNode::Resync()
{
	if(m_pParent->m_id == 0x18538067) { /*segment?*/
		SeekTo(m_filepos);

		for(BYTE b = 0; S_OK == Read(b); b = 0) {
			if((b&0xf0) != 0x10) {
				continue;
			}

			DWORD dw = b;
			Read((BYTE*)&dw+1, 3);
			bswap((BYTE*)&dw, 4);

			switch(dw) {
				case 0x1549A966: // SegmentInfo
				case 0x114D9B74: // MetaSeekInfo
				case 0x1654AE6B: // Tracks
				case 0x1F43B675: // Clusters
				case 0x1C53BB6B: // Cues
				case 0x1941A469: // Attachments
				case 0x1043A770: // Chapters
				case 0x1254C367: // Tags
					SeekTo(GetPos()-4);
					return(SUCCEEDED(Parse()));
				default:
					SeekTo(GetPos()-3);
					break;
			}
		}
	}

	return false;
}
