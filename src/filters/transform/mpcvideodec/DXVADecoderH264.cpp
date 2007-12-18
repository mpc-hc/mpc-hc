/* 
 * $Id: DXVADecoderH264.cpp 249 2007-09-26 11:07:22Z casimir666 $
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
#include "DXVADecoderH264.h"
#include "MPCVideoDecFilter.h"


#define SE_HEADER           0


CDXVADecoderH264::CDXVADecoderH264 (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDecoder, DXVA2Mode nMode)
				: CDXVADecoder (pFilter, pDecoder, nMode)
{
	memset (&m_DXVAPicParams,	0, sizeof(m_DXVAPicParams));
	memset (&m_nQMatrix,		0, sizeof(m_nQMatrix));
	memset (&m_SliceShort,		0, sizeof(m_SliceShort));
	memset (&m_PicParam,		0, sizeof(m_PicParam));
	memset (&m_SeqParam,		0, sizeof(m_SeqParam));

	m_nQMatrix			= Flat16;	// TODO : get from config!
	m_nCurRefFrame		= 0;

	switch (nMode)
	{
	case H264_VLD :
		//m_ExtensionData.Function				= DXVA_PICTURE_DECODING_FUNCTION;
		//m_ExtensionData.pPrivateOutputData		= &m_StatusH264;
		//m_ExtensionData.PrivateOutputDataSize	= sizeof (m_StatusH264);
		AllocExecuteParams (3);
		break;
	}
}



HRESULT CDXVADecoderH264::DecodeFrame (BYTE* pDataIn, UINT nSize, IMediaSample* pOut)
{
	HRESULT						hr;
	CComQIPtr<IMFGetService>	pSampleService;
	CComPtr<IDirect3DSurface9>	pDecoderRenderTarget;
	BYTE*						pDXVABuffer;
	UINT						nDXVASize;
	int							nBuffCount;
	NALU						Nalu;
	SLICE_PARAMETER				Slice;

	//char		strFile[MAX_PATH];
	//static	int	nNb = 1;
	//sprintf (strFile, "D:\\Sources\\mpc-hc\\Samples\\DXVA2\\Trace\\Prison Break S03 - E07\\BitStream_mpc_%d.bin", nNb++);
	//FILE*		hOutFile = fopen (strFile, "wb");
	//if (hOutFile)
	//{
	//	fwrite (pDataIn,1, nSize, hOutFile);
	//	fclose (hOutFile);
	//}
	//return S_OK;

	pSampleService = pOut;

	hr = pSampleService->GetService (MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void**) &pDecoderRenderTarget);
	LOG(_T("pSampleService->GetService  hr=0x%08x"), hr);
	hr = m_pDXDecoder->BeginFrame(pDecoderRenderTarget, NULL);
	LOG(_T("m_pDXDecoder->BeginFrame  hr=0x%08x"), hr);

	// ==>> http://forums.microsoft.com/MSDN/ShowPost.aspx?PostID=1659948&SiteID=1
	bool		bSliceFound = false;
	UINT		nPacketLength;
	while (!bSliceFound)
	{
		nPacketLength = (pDataIn[0] << 24) + (pDataIn[1] << 16) + (pDataIn[2] << 8) + pDataIn[3];

		nSize	-= 4;
		pDataIn += 4;

		ReadNalu (&Nalu, pDataIn, nSize);
		switch (Nalu.nal_unit_type)
		{
		case NALU_TYPE_PPS :
			break;
		case NALU_TYPE_SPS :
			break;
		case NALU_TYPE_SLICE:
		case NALU_TYPE_IDR:
			bSliceFound = true;
			break;
		}

		if (!bSliceFound)
		{
			nSize	-= nPacketLength;
			pDataIn += nPacketLength;
		}
	}


	// Suppression en tête pas bon !!!
	//if (nPacketLength != nSize - 4)		// TODO : read SPS !!!!!!!
	//{
	//	nSize	-= nPacketLength  + 4;
	//	pDataIn += nPacketLength  + 4;

	//	nPacketLength = (pDataIn[0] << 24) + (pDataIn[1] << 16) + (pDataIn[2] << 8) + pDataIn[3];
	//	ASSERT (nPacketLength == nSize - 4);
	//}

	// Step 1 : Parse bitstream and initialize Picture Parameters
	memset (&Slice, 0, sizeof(Slice));
	Slice.idr_flag = (Nalu.nal_unit_type == NALU_TYPE_IDR);
	ReadSliceHeader (&Slice, pDataIn, nSize);
	m_DXVAPicParams.field_pic_flag	= Slice.field_pic_flag;
	m_DXVAPicParams.RefPicFlag		= (Nalu.nal_reference_idc != 0);
	m_DXVAPicParams.IntraPicFlag	= (Slice.slice_type == I_FRAME);
	m_DXVAPicParams.MbaffFrameFlag	= (m_SeqParam.mb_adaptive_frame_field_flag && (Slice.field_pic_flag==0));
	m_DXVAPicParams.frame_num		= Slice.frame_num;

	// Fill CurrPic parameters
	if (Slice.field_pic_flag)
		m_DXVAPicParams.CurrPic.AssociatedFlag	= Slice.delta_pic_order_cnt_bottom;
//	m_DXVAPicParams.CurrPic.bPicEntry			= ;
//	m_DXVAPicParams.CurrPic.Index7Bits			= ;

	// TODO : pas fini !!!
//	m_DXVAPicParams.wBitFields	= 0x7C10 + (nFrameType == I_FRAME ? 0x8000 : 0) +  (bRefFrame ? 0x40 : 0);

	//USES_CONVERSION;
	//static BYTE*		pDurBuff = NULL;
	//static int			nBuff	 = 1;
	//FILE*				hBitstream = NULL;
	//CString				strPath;
	//if (!pDurBuff) pDurBuff = new BYTE[400000];
	//strPath.Format (_T("D:\\Sources\\mpc-hc\\Samples\\DXVA2\\TraceLost S03 - E19 - The Brig\\BitStream_%d.bin"), nBuff++);
	//hBitstream = fopen (W2A(strPath), "rb");
	//nSize = fread (pDurBuff, 1, 400000, hBitstream) + 4;
	//pDataIn = pDurBuff - 4;
	//fclose(hBitstream);


	//static FILE*					hPict = NULL;
	//static DXVA_PicParams_H264*		pParams;
	//static int						nParamIndex = 0;
	//if (!hPict)
	//{
	//	hPict = fopen ("PicParam.bin", "rb");
	//	pParams = (DXVA_PicParams_H264*)new BYTE [500000];
	//	int nByteRead = fread (pParams, 500000, 1, hPict);
	//}
	//memcpy (&m_DXVAPicParams, &pParams[nParamIndex++], sizeof(m_DXVAPicParams));


	// Step 1 : Send picture parameters !
	m_ExecuteParams.NumCompBuffers	= 0;
	hr = AddExecuteBuffer (DXVA2_PictureParametersBufferType, sizeof(m_DXVAPicParams), &m_DXVAPicParams);
	m_DXVAPicParams.StatusReportFeedbackNumber++;
	hr = m_pDXDecoder->Execute(&m_ExecuteParams);

	m_ExecuteParams.NumCompBuffers	= 0;

	hr = AddExecuteBuffer (DXVA2_BitStreamDateBufferType, nSize, pDataIn);

	m_SliceShort.SliceBytesInBuffer = nSize;
	hr = AddExecuteBuffer (DXVA2_SliceControlBufferType, sizeof (m_SliceShort), &m_SliceShort);
	hr = AddExecuteBuffer (DXVA2_InverseQuantizationMatrixBufferType, sizeof (DXVA_Qmatrix_H264), (void*)&g_QMatrixH264[m_nQMatrix]);

	m_DXVAPicParams.StatusReportFeedbackNumber++;
	hr = m_pDXDecoder->Execute(&m_ExecuteParams);
	LOG(_T("m_pDXDecoder->Execute  hr=0x%08x"), hr);

	hr = m_pDXDecoder->EndFrame(NULL);

	UpdatePictureParams (Slice.frame_num, Nalu.nal_reference_idc);


	//CComPtr<IDirect3DDevice9>		pD3DDev;
	//if (SUCCEEDED (pDecoderRenderTarget->GetDevice (&pD3DDev)))
	//{
	//	RECT		rcTearing;
	//	
	//	rcTearing.left		= 0;
	//	rcTearing.top		= 0;
	//	rcTearing.right		= 80;
	//	rcTearing.bottom	= 80;

	//	pD3DDev->ColorFill (pDecoderRenderTarget, &rcTearing, D3DCOLOR_ARGB (255,0,0,255));
	//}

	return hr;
}


void CDXVADecoderH264::InitPictureParams()
{
	m_DXVAPicParams.wBitFields						= 0;	// Reset!
	m_DXVAPicParams.wFrameWidthInMbsMinus1			= m_pFilter->PictWidth()/16 - 1;	// TODO : check this!
	m_DXVAPicParams.wFrameHeightInMbsMinus1			= m_pFilter->PictHeight()/16 - 1;	// TODO : check this!
	m_DXVAPicParams.num_ref_frames					= m_SeqParam.num_ref_frames;
//	m_DXVAPicParams.field_pic_flag					= SET IN DecodeFrame;
//	m_DXVAPicParams.MbaffFrameFlag					= SET IN DecodeFrame;
	m_DXVAPicParams.residual_colour_transform_flag	= m_SeqParam.residual_colour_transform_flag;
//	m_DXVAPicParams.sp_for_switch_flag
	m_DXVAPicParams.chroma_format_idc				= m_SeqParam.chroma_format_idc;
//	m_DXVAPicParams.RefPicFlag						= SET IN DecodeFrame;
	m_DXVAPicParams.constrained_intra_pred_flag		= m_PicParam.constrained_intra_pred_flag;
	m_DXVAPicParams.weighted_pred_flag				= m_PicParam.weighted_pred_flag;
	m_DXVAPicParams.weighted_bipred_idc				= m_PicParam.weighted_bipred_idc;
	m_DXVAPicParams.MbsConsecutiveFlag				= 1;	// TODO : always activate ??
	m_DXVAPicParams.frame_mbs_only_flag				= m_SeqParam.frame_mbs_only_flag;
	m_DXVAPicParams.transform_8x8_mode_flag			= m_PicParam.transform_8x8_mode_flag;
	m_DXVAPicParams.MinLumaBipredSize8x8Flag		= 1;	// TODO : always activate ???
//	m_DXVAPicParams.IntraPicFlag

	m_DXVAPicParams.bit_depth_luma_minus8			= m_SeqParam.bit_depth_luma_minus8;
	m_DXVAPicParams.bit_depth_chroma_minus8			= m_SeqParam.bit_depth_chroma_minus8;
//	m_DXVAPicParams.Reserved16Bits
//	m_DXVAPicParams.StatusReportFeedbackNumber

	for (int i =0; i<16; i++)
	{
		m_DXVAPicParams.RefFrameList[i].AssociatedFlag	= 1;
		m_DXVAPicParams.RefFrameList[i].bPicEntry		= 255;
		m_DXVAPicParams.RefFrameList[i].Index7Bits		= 127;
	}

//	m_DXVAPicParams.CurrFieldOrderCnt
//	m_DXVAPicParams.FieldOrderCntList
	m_DXVAPicParams.ContinuationFlag						= 1;	// TODO : copy structure in bitstream buffer !!
	m_DXVAPicParams.Reserved8BitsA							= 0;
//	m_DXVAPicParams.FrameNumList
//	m_DXVAPicParams.UsedForReferenceFlags
//	m_DXVAPicParams.NonExistingFrameFlags
	m_DXVAPicParams.Reserved8BitsB							= 0;
//	m_DXVAPicParams.SliceGroupMap


	m_DXVAPicParams.log2_max_frame_num_minus4				= m_SeqParam.log2_max_frame_num_minus4;
	m_DXVAPicParams.pic_order_cnt_type						= m_SeqParam.pic_order_cnt_type;
	m_DXVAPicParams.log2_max_pic_order_cnt_lsb_minus4		= m_SeqParam.log2_max_pic_order_cnt_lsb_minus4;
	m_DXVAPicParams.delta_pic_order_always_zero_flag		= m_SeqParam.delta_pic_order_always_zero_flag;
	m_DXVAPicParams.direct_8x8_inference_flag				= m_SeqParam.direct_8x8_inference_flag;
	m_DXVAPicParams.entropy_coding_mode_flag				= m_PicParam.entropy_coding_mode_flag;
	m_DXVAPicParams.pic_order_present_flag					= m_PicParam.pic_order_present_flag;
	m_DXVAPicParams.num_slice_groups_minus1					= m_PicParam.num_slice_groups_minus1;
	m_DXVAPicParams.slice_group_map_type					= m_PicParam.slice_group_map_type;
	m_DXVAPicParams.deblocking_filter_control_present_flag	= m_PicParam.deblocking_filter_control_present_flag;
	m_DXVAPicParams.redundant_pic_cnt_present_flag			= m_PicParam.redundant_pic_cnt_present_flag;
	m_DXVAPicParams.slice_group_change_rate_minus1			= m_PicParam.slice_group_change_rate_minus1;
}


static UINT g_UsedForReferenceFlags[16] =
{
	0x00000003,
	0x0000000F,
	0x0000003F,
	0x000000FF,
	0x000003FF,
	0x00000FFF,
	0x00003FFF,
	0x0000FFFF,
	0x0003FFFF,
	0x000FFFFF,
	0x003FFFFF,
	0x00FFFFFF,
	0x03FFFFFF,
	0x0FFFFFFF,
	0x3FFFFFFF,
	0xFFFFFFFF,
};

void CDXVADecoderH264::UpdatePictureParams (int nFrameNum, bool bRefFrame)
{
	int			i;

	m_DXVAPicParams.UsedForReferenceFlags	= g_UsedForReferenceFlags [min(nFrameNum, m_DXVAPicParams.num_ref_frames-1)];

	// Reset when new picture group detected
	if (nFrameNum == 0)
	{
		for (i=1; i<m_DXVAPicParams.num_ref_frames; i++)
		{
			m_DXVAPicParams.RefFrameList[i].AssociatedFlag	= 1;
			m_DXVAPicParams.RefFrameList[i].bPicEntry		= 255;
			m_DXVAPicParams.RefFrameList[i].Index7Bits		= 127;
			
			m_DXVAPicParams.FieldOrderCntList[i][0]			= 0;
			m_DXVAPicParams.FieldOrderCntList[i][1]			= 0;

			m_DXVAPicParams.FrameNumList[i]					= 0;
		}
	}

	if (bRefFrame)
	{
		// Shift buffers if needed
		if (!m_DXVAPicParams.RefFrameList[m_nCurRefFrame].AssociatedFlag)
		{
			for (i=1; i<m_DXVAPicParams.num_ref_frames; i++)
			{
				m_DXVAPicParams.FrameNumList[i-1] = m_DXVAPicParams.FrameNumList[i];
				memcpy (&m_DXVAPicParams.RefFrameList[i-1], &m_DXVAPicParams.RefFrameList[i], sizeof (DXVA_PicEntry_H264));
			}
		}

		m_DXVAPicParams.FrameNumList[m_nCurRefFrame] = nFrameNum;

		// Update "Reference Frame List"

		m_DXVAPicParams.RefFrameList[m_nCurRefFrame].AssociatedFlag	= 0;
		m_DXVAPicParams.RefFrameList[m_nCurRefFrame].bPicEntry		= m_DXVAPicParams.CurrPic.bPicEntry;
		m_DXVAPicParams.RefFrameList[m_nCurRefFrame].Index7Bits		= m_DXVAPicParams.CurrPic.Index7Bits;

		m_nCurRefFrame = min (m_nCurRefFrame+1, m_DXVAPicParams.num_ref_frames-1);
	}

	m_DXVAPicParams.CurrPic.bPicEntry	= ((m_DXVAPicParams.CurrPic.bPicEntry + 1) % PicEntryNumber);	// TODO : rule for pic entry ???
	m_DXVAPicParams.CurrPic.Index7Bits	= m_DXVAPicParams.CurrPic.bPicEntry;

	CString		strTraceRefFrameList;
	CString		strTraceFrameNumList;
	for (i=0; i<m_DXVAPicParams.num_ref_frames; i++)
	{
		strTraceRefFrameList.AppendFormat (_T("%03d "), m_DXVAPicParams.RefFrameList[i].bPicEntry);
		strTraceFrameNumList.AppendFormat (_T("%03d "), m_DXVAPicParams.FrameNumList[i]);
	}

	TRACE ("%d = %S  %S\n", nFrameNum, strTraceRefFrameList, strTraceFrameNumList);


//	ASSERT ((nFrameType != SP_FRAME) && (nFrameType != SI_FRAME));
}



/*!
 ************************************************************************
 * \brief
 *  read one exp-golomb VLC symbol
 *
 * \param buffer
 *    containing VLC-coded data bits
 * \param totbitoffset
 *    bit offset from start of partition
 * \param  info
 *    returns the value of the symbol
 * \param bytecount
 *    buffer length
 * \return
 *    bits read
 ************************************************************************
 */
int CDXVADecoderH264::GetVLCSymbol (BYTE* buffer,int totbitoffset,int *info, int bytecount)
{

  register int inf;
  long byteoffset = (totbitoffset >> 3);         // byte from start of buffer
  int  bitoffset  = (7 - (totbitoffset & 0x07)); // bit from start of byte
  int  bitcounter = 1;
  int  len        = 0;
  byte *cur_byte = &(buffer[byteoffset]);
  int  ctr_bit    = ((*cur_byte) >> (bitoffset)) & 0x01;  // control bit for current bit posision

  while (ctr_bit == 0)
  {                 // find leading 1 bit
    len++;
    bitcounter++;
    bitoffset--;
    bitoffset &= 0x07;
    cur_byte  += (bitoffset == 7);
    byteoffset+= (bitoffset == 7);      
    ctr_bit    = ((*cur_byte) >> (bitoffset)) & 0x01;
  }

  if (byteoffset + ((len + 7) >> 3) > bytecount)
    return -1;

  // make infoword
  inf = 0;                          // shortest possible code is 1, then info is always 0    

  while (len--)
  {
    bitoffset --;    
    bitoffset &= 0x07;
    cur_byte  += (bitoffset == 7);
    bitcounter++;
    inf <<= 1;    
    inf |= ((*cur_byte) >> (bitoffset)) & 0x01;
  }

  *info = inf;
  return bitcounter;           // return absolute offset in bit from start of frame
}


/*!
 ************************************************************************
 * \brief
 *  Reads bits from the bitstream buffer
 *
 * \param buffer
 *    containing VLC-coded data bits
 * \param totbitoffset
 *    bit offset from start of partition
 * \param info
 *    returns value of the read bits
 * \param bytecount
 *    total bytes in bitstream
 * \param numbits
 *    number of bits to read
 *
 ************************************************************************
 */
int CDXVADecoderH264::GetBits (byte buffer[],int totbitoffset,int *info, int bytecount,
             int numbits)
{
  register int inf;
  int  bitoffset  = (totbitoffset & 0x07); // bit from start of byte
  long byteoffset = (totbitoffset >> 3);       // byte from start of buffer
  int  bitcounter = numbits;
  static byte *curbyte;

  if ((byteoffset) + ((numbits + bitoffset)>> 3)  > bytecount)
    return -1;

  curbyte = &(buffer[byteoffset]);

  bitoffset = 7 - bitoffset;

  inf=0;

  while (numbits--)
  {
    inf <<=1;    
    inf |= ((*curbyte)>> (bitoffset--)) & 0x01;    
    //curbyte   += (bitoffset >> 3) & 0x01;
    curbyte   -= (bitoffset >> 3);
    bitoffset &= 0x07;
    //curbyte   += (bitoffset == 7);    
  }

  *info = inf;
  return bitcounter;           // return absolute offset in bit from start of frame
}

int CDXVADecoderH264::ReadSyntaxElement_VLC(SyntaxElement *sym, BYTE* pBuffer, UINT nBufferLength, UINT& nBitOffset)
{
  sym->len =  GetVLCSymbol (pBuffer, nBitOffset, &(sym->inf), nBufferLength);
  if (sym->len == -1)
    return -1;
  nBitOffset += sym->len;
  sym->mapping(sym->len,sym->inf,&(sym->value1),&(sym->value2));

  return 1;
}

int CDXVADecoderH264::ReadSyntaxElement_FLC(SyntaxElement *sym, BYTE* pBuffer, UINT nBufferLength, UINT& nBitOffset)
{
  if ((GetBits(pBuffer, nBitOffset, &(sym->inf), nBufferLength, sym->len)) < 0)
    return -1;

  sym->value1 = sym->inf;
  nBitOffset += sym->len; // move bitstream pointer

  return 1;
}



/*!
 ************************************************************************
 * \brief
 *    mapping rule for ue(v) syntax elements
 * \par Input:
 *    lenght and info
 * \par Output:
 *    number in the code table
 ************************************************************************
 */
void CDXVADecoderH264::LInfo_Ue(int len, int info, int *value1, int *dummy)
{
  ASSERT ((len >> 1) < 32);
  *value1 = (1 << (len >> 1)) + info - 1;
}

/*!
 ************************************************************************
 * \brief
 *    mapping rule for se(v) syntax elements
 * \par Input:
 *    lenght and info
 * \par Output:
 *    signed mvd
 ************************************************************************
 */
void CDXVADecoderH264::LInfo_Se(int len,  int info, int *value1, int *dummy)
{
  int n;
  ASSERT ((len >> 1) < 32);
  n = (1 << (len >> 1)) + info - 1;
  *value1 = (n + 1) >> 1;
  if((n & 0x01) == 0)                           // lsb is signed bit
    *value1 = -*value1;
}

int CDXVADecoderH264::ue_v (BYTE* pBuffer, UINT nBufferLength, UINT& nBitOffset)
{
  SyntaxElement symbol;

  symbol.type = SE_HEADER;
  symbol.mapping = LInfo_Ue;   // Mapping rule

  ReadSyntaxElement_VLC (&symbol, pBuffer, nBufferLength, nBitOffset);
  return symbol.value1;
}


/*!
 *************************************************************************************
 * \brief
 *    ue_v, reads an se(v) syntax element, the length in bits is stored in
 *    the global UsedBits variable
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
int CDXVADecoderH264::se_v (BYTE* pBuffer, UINT nBufferLength, UINT& nBitOffset)
{
  SyntaxElement symbol;

  symbol.type = SE_HEADER;
  symbol.mapping = LInfo_Se;   // Mapping rule: signed integer

  ReadSyntaxElement_VLC (&symbol, pBuffer, nBufferLength, nBitOffset);
  return symbol.value1;
}


/*!
 *************************************************************************************
 * \brief
 *    ue_v, reads an u(1) syntax element, the length in bits is stored in
 *    the global UsedBits variable
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
bool CDXVADecoderH264::u_1 (BYTE* pBuffer, UINT nBufferLength, UINT& nBitOffset)
{
  return (bool) u_v (1, pBuffer, nBufferLength, nBitOffset);
}

 //*************************************************************************************
 //* \brief
 //*    ue_v, reads an u(v) syntax element, the length in bits is stored in
 //*    the global UsedBits variable
 //*
 //* \param LenInBits
 //*    length of the syntax element
 //*
 //* \param tracestring
 //*    the string for the trace file
 //*
 //* \param bitstream
 //*    the stream to be read from
 //*
 //* \return
 //*    the value of the coded syntax element
 //*
 //*************************************************************************************
int CDXVADecoderH264::u_v (int LenInBits, BYTE* pBuffer, UINT nBufferLength, UINT& nBitOffset)
{
  SyntaxElement symbol;
  symbol.inf = 0;

  symbol.type = SE_HEADER;
  symbol.mapping = LInfo_Ue;   // Mapping rule
  symbol.len = LenInBits;

  ReadSyntaxElement_FLC (&symbol, pBuffer, nBufferLength, nBitOffset);

  return symbol.inf;
}


void CDXVADecoderH264::ReadSliceHeader(SLICE_PARAMETER* pSlice, BYTE* pBuffer, UINT nBufferLength)
{
	UINT			nBitOffset = 8;
	int				first_mb_in_slice;
	int				pic_parameter_set_id;
	int				tmp;

	pSlice->first_mb_in_slice = ue_v (pBuffer, nBufferLength, nBitOffset);

	tmp = ue_v (pBuffer, nBufferLength, nBitOffset);
	if (tmp>4) tmp -=5;
	pSlice->slice_type = (FRAME_TYPE) tmp;

	pSlice->pic_parameter_set_id = ue_v (pBuffer, nBufferLength, nBitOffset);

	if (m_SeqParam.residual_colour_transform_flag)
		pSlice->colour_plane_id = u_v (2, pBuffer, nBufferLength, nBitOffset);

	pSlice->frame_num = (UINT)u_v (m_SeqParam.log2_max_frame_num_minus4 + 4, pBuffer, nBufferLength, nBitOffset);

	if (m_SeqParam.frame_mbs_only_flag)
	{
		pSlice->field_pic_flag = 0;
	}
	else
	{
		pSlice->field_pic_flag = u_1(pBuffer, nBufferLength, nBitOffset);
		if (pSlice->field_pic_flag)
		  pSlice->bottom_field_flag = u_1(pBuffer, nBufferLength, nBitOffset);
	}

	if (pSlice->idr_flag)
		pSlice->idr_pic_id = ue_v(pBuffer, nBufferLength, nBitOffset);

	if (m_SeqParam.pic_order_cnt_type == 0)
	{
		pSlice->pic_order_cnt_lsb = u_v(m_SeqParam.log2_max_pic_order_cnt_lsb_minus4 + 4, pBuffer, nBufferLength, nBitOffset);
		if( m_PicParam.pic_order_present_flag  ==  1 &&  !pSlice->field_pic_flag )
			pSlice->delta_pic_order_cnt_bottom = se_v(pBuffer, nBufferLength, nBitOffset);
		else
			pSlice->delta_pic_order_cnt_bottom = 0;
	}
	if (m_SeqParam.pic_order_cnt_type == 1 && !m_SeqParam.delta_pic_order_always_zero_flag )
	{
		pSlice->delta_pic_order_cnt[ 0 ] = se_v(pBuffer, nBufferLength, nBitOffset);
		if( m_PicParam.pic_order_present_flag  ==  1  &&  !pSlice->field_pic_flag )
			pSlice->delta_pic_order_cnt[ 1 ] = se_v(pBuffer, nBufferLength, nBitOffset);
	}
	else
	{
		if (m_SeqParam.pic_order_cnt_type == 1)
		{
			pSlice->delta_pic_order_cnt[ 0 ] = 0;
			pSlice->delta_pic_order_cnt[ 1 ] = 0;
		}
	}

	//! redundant_pic_cnt is missing here
}



void CDXVADecoderH264::ReadNalu (NALU* pNalu, BYTE* pDataIn, UINT nSize)
{
  pNalu->forbidden_bit		= (pDataIn[0]>>7) & 1;
  pNalu->nal_reference_idc	= (pDataIn[0]>>5) & 3;
  pNalu->nal_unit_type		= (pDataIn[0])	  & 0x1f;
}


/*!
 ************************************************************************
 * \brief
 *    test if bit buffer contains only stop bit
 *
 * \param buffer
 *    buffer containing VLC-coded data bits
 * \param totbitoffset
 *    bit offset from start of partition
 * \param bytecount
 *    buffer length
 * \return
 *    true if more bits available
 ************************************************************************
 */
int CDXVADecoderH264::MoreRBSPData (BYTE* pBuffer,int totbitoffset,int bytecount)
{
  int bitoffset   = (7 - (totbitoffset & 0x07));      // bit from start of byte
  long byteoffset = (totbitoffset >> 3);      // byte from start of pBuffer
  byte *cur_byte  = &(pBuffer[byteoffset]);
  int ctr_bit     = 0;      // control bit for current bit posision
  int cnt         = 0;

  ASSERT (byteoffset<bytecount);

  // there is more until we're in the last byte
  if (byteoffset < (bytecount - 1)) return TRUE;

  // read one bit
  ctr_bit = ((*cur_byte)>> (bitoffset--)) & 0x01;

  // a stop bit has to be one
  if (ctr_bit==0) return TRUE;  

  while (bitoffset>=0 && !cnt)
  {
    cnt |= ((*cur_byte)>> (bitoffset--)) & 0x01;   // set up control bit
  }

  return (cnt);
}


void CDXVADecoderH264::ReadPPS(PIC_PARAMETER_SET_RBSP* pps, BYTE* pBuffer, UINT nBufferLength)
{
	UINT		nBitOffset = 8;
	int			NumberBitsPerSliceGroupId;
	int			chroma_format_idc;
	unsigned	i;

	pps->pic_parameter_set_id                  = ue_v (pBuffer, nBufferLength, nBitOffset);
	pps->seq_parameter_set_id                  = ue_v (pBuffer, nBufferLength, nBitOffset);
	pps->entropy_coding_mode_flag              = u_1  (pBuffer, nBufferLength, nBitOffset);

	//! Note: as per JVT-F078 the following bit is unconditional.  If F078 is not accepted, then
	//! one has to fetch the correct SPS to check whether the bit is present (hopefully there is
	//! no consistency problem :-(
	//! The current encoder code handles this in the same way.  When you change this, don't forget
	//! the encoder!  StW, 12/8/02
	pps->pic_order_present_flag                = u_1  (pBuffer, nBufferLength, nBitOffset);

	pps->num_slice_groups_minus1               = ue_v (pBuffer, nBufferLength, nBitOffset);

	// FMO stuff begins here
	if (pps->num_slice_groups_minus1 > 0)
	{
		pps->slice_group_map_type               = ue_v (pBuffer, nBufferLength, nBitOffset);
		if (pps->slice_group_map_type == 0)
		{
			for (i=0; i<=pps->num_slice_groups_minus1; i++)
			pps->run_length_minus1 [i]                  = ue_v (pBuffer, nBufferLength, nBitOffset);
		}
		else if (pps->slice_group_map_type == 2)
		{
			for (i=0; i<pps->num_slice_groups_minus1; i++)
			{
				//! JVT-F078: avoid reference of SPS by using ue(v) instead of u(v)
				pps->top_left [i]                          = ue_v (pBuffer, nBufferLength, nBitOffset);
				pps->bottom_right [i]                      = ue_v (pBuffer, nBufferLength, nBitOffset);
			}
		}
		else if (pps->slice_group_map_type == 3 ||
				 pps->slice_group_map_type == 4 ||
				 pps->slice_group_map_type == 5)
		{
			pps->slice_group_change_direction_flag     = u_1  (pBuffer, nBufferLength, nBitOffset);
			pps->slice_group_change_rate_minus1        = ue_v (pBuffer, nBufferLength, nBitOffset);
		}
		else if (pps->slice_group_map_type == 6)
		{
			if (pps->num_slice_groups_minus1+1 >4)
				NumberBitsPerSliceGroupId = 3;
			else if (pps->num_slice_groups_minus1+1 > 2)
				NumberBitsPerSliceGroupId = 2;
			else
				NumberBitsPerSliceGroupId = 1;

			//! JVT-F078, exlicitly signal number of MBs in the map
			pps->num_slice_group_map_units_minus1      = ue_v (pBuffer, nBufferLength, nBitOffset);
			for (i=0; i<=pps->num_slice_group_map_units_minus1; i++)
				pps->slice_group_id[i] = u_v (NumberBitsPerSliceGroupId, pBuffer, nBufferLength, nBitOffset);
		}
	}

	// End of FMO stuff

	pps->num_ref_idx_l0_active_minus1          = ue_v (pBuffer, nBufferLength, nBitOffset);
	pps->num_ref_idx_l1_active_minus1          = ue_v (pBuffer, nBufferLength, nBitOffset);
	pps->weighted_pred_flag                    = u_1  (pBuffer, nBufferLength, nBitOffset);
	pps->weighted_bipred_idc                   = u_v  ( 2, pBuffer, nBufferLength, nBitOffset);
	pps->pic_init_qp_minus26                   = se_v (pBuffer, nBufferLength, nBitOffset);
	pps->pic_init_qs_minus26                   = se_v (pBuffer, nBufferLength, nBitOffset);

	pps->chroma_qp_index_offset                = se_v (pBuffer, nBufferLength, nBitOffset);

	pps->deblocking_filter_control_present_flag = u_1 (pBuffer, nBufferLength, nBitOffset);
	pps->constrained_intra_pred_flag           = u_1  (pBuffer, nBufferLength, nBitOffset);
	pps->redundant_pic_cnt_present_flag        = u_1  (pBuffer, nBufferLength, nBitOffset);
	
	if(MoreRBSPData(pBuffer, nBitOffset, nBufferLength)) // more_data_in_rbsp()
	{
		//Fidelity Range Extensions Stuff
		pps->transform_8x8_mode_flag           = u_1  (pBuffer, nBufferLength, nBitOffset);
		pps->pic_scaling_matrix_present_flag   =  u_1  (pBuffer, nBufferLength, nBitOffset);

		//if(pps->pic_scaling_matrix_present_flag)
		//{
		//	chroma_format_idc = SeqParSet[pps->seq_parameter_set_id].chroma_format_idc;
		//	n_ScalingList = 6 + ((chroma_format_idc != YUV444) ? 2 : 6) * pps->transform_8x8_mode_flag;
		//	for(i=0; i<n_ScalingList; i++)
		//	{
		//		pps->pic_scaling_list_present_flag[i]= u_1  (pBuffer, nBufferLength, nBitOffset);

		//		if(pps->pic_scaling_list_present_flag[i])
		//		{
		//			if(i<6)
		//			Scaling_List(pps->ScalingList4x4[i], 16, &pps->UseDefaultScalingMatrix4x4Flag[i], s);
		//			else
		//			Scaling_List(pps->ScalingList8x8[i-6], 64, &pps->UseDefaultScalingMatrix8x8Flag[i-6], s);
		//		}
		//	}
		//}
		pps->second_chroma_qp_index_offset      = se_v (pBuffer, nBufferLength, nBitOffset);
	}
	else
	{
		pps->second_chroma_qp_index_offset      = pps->chroma_qp_index_offset;
	}

	pps->Valid = TRUE;
}

/*
void CDXVADecoderH264::ReadVUI(SEQ_PARAMETER_SET_RBSP* sps, BYTE* pBuffer, UINT nBufferLength, UINT nBitOffset)
{
	sps->vui_seq_parameters.matrix_coefficients = 2;
	if (sps->vui_parameters_present_flag)
	{
		sps->vui_seq_parameters.aspect_ratio_info_present_flag = u_1  (pBuffer, nBufferLength, nBitOffset);
		if (sps->vui_seq_parameters.aspect_ratio_info_present_flag)
		{
			sps->vui_seq_parameters.aspect_ratio_idc             = u_v  ( 8, pBuffer, nBufferLength, nBitOffset);
			if (255==sps->vui_seq_parameters.aspect_ratio_idc)
			{
				sps->vui_seq_parameters.sar_width                  = u_v  (16, pBuffer, nBufferLength, nBitOffset);
				sps->vui_seq_parameters.sar_height                 = u_v  (16, pBuffer, nBufferLength, nBitOffset);
			}
		}

		sps->vui_seq_parameters.overscan_info_present_flag     = u_1  (pBuffer, nBufferLength, nBitOffset);
		if (sps->vui_seq_parameters.overscan_info_present_flag)
		{
			sps->vui_seq_parameters.overscan_appropriate_flag    = u_1  (pBuffer, nBufferLength, nBitOffset);
		}

		sps->vui_seq_parameters.video_signal_type_present_flag = u_1  (pBuffer, nBufferLength, nBitOffset);
		if (sps->vui_seq_parameters.video_signal_type_present_flag)
		{
			sps->vui_seq_parameters.video_format                    = u_v  ( 3, pBuffer, nBufferLength, nBitOffset);
			sps->vui_seq_parameters.video_full_range_flag           = u_1  (pBuffer, nBufferLength, nBitOffset);
			sps->vui_seq_parameters.colour_description_present_flag = u_1  (pBuffer, nBufferLength, nBitOffset);
			if(sps->vui_seq_parameters.colour_description_present_flag)
			{
				sps->vui_seq_parameters.colour_primaries              = u_v  ( 8,pBuffer, nBufferLength, nBitOffset);
				sps->vui_seq_parameters.transfer_characteristics      = u_v  ( 8,pBuffer, nBufferLength, nBitOffset);
				sps->vui_seq_parameters.matrix_coefficients           = u_v  ( 8,pBuffer, nBufferLength, nBitOffset);
			}
		}
		sps->vui_seq_parameters.chroma_location_info_present_flag = u_1  (pBuffer, nBufferLength, nBitOffset);
		if(sps->vui_seq_parameters.chroma_location_info_present_flag)
		{
			sps->vui_seq_parameters.chroma_sample_loc_type_top_field     = ue_v  (pBuffer, nBufferLength, nBitOffset);
			sps->vui_seq_parameters.chroma_sample_loc_type_bottom_field  = ue_v  (pBuffer, nBufferLength, nBitOffset);
		}
		sps->vui_seq_parameters.timing_info_present_flag          = u_1  (pBuffer, nBufferLength, nBitOffset);
		if (sps->vui_seq_parameters.timing_info_present_flag)
		{
			sps->vui_seq_parameters.num_units_in_tick               = u_v  (32,pBuffer, nBufferLength, nBitOffset);
			sps->vui_seq_parameters.time_scale                      = u_v  (32,pBuffer, nBufferLength, nBitOffset);
			sps->vui_seq_parameters.fixed_frame_rate_flag           = u_1  (   pBuffer, nBufferLength, nBitOffset);
		}
		sps->vui_seq_parameters.nal_hrd_parameters_present_flag   = u_1  (pBuffer, nBufferLength, nBitOffset);
		if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag)
		{
			ReadHRDParameters(p, &(sps->vui_seq_parameters.nal_hrd_parameters));
		}
		sps->vui_seq_parameters.vcl_hrd_parameters_present_flag   = u_1  (pBuffer, nBufferLength, nBitOffset);
		if (sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
		{
			ReadHRDParameters(p, &(sps->vui_seq_parameters.vcl_hrd_parameters));
		}
		if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag || sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
		{
			sps->vui_seq_parameters.low_delay_hrd_flag             =  u_1  (pBuffer, nBufferLength, nBitOffset);
		}
		sps->vui_seq_parameters.pic_struct_present_flag          =  u_1  (pBuffer, nBufferLength, nBitOffset);
		sps->vui_seq_parameters.bitstream_restriction_flag       =  u_1  (pBuffer, nBufferLength, nBitOffset);
		if (sps->vui_seq_parameters.bitstream_restriction_flag)
		{
			sps->vui_seq_parameters.motion_vectors_over_pic_boundaries_flag =  u_1  (pBuffer, nBufferLength, nBitOffset);
			sps->vui_seq_parameters.max_bytes_per_pic_denom                 =  ue_v (pBuffer, nBufferLength, nBitOffset);
			sps->vui_seq_parameters.max_bits_per_mb_denom                   =  ue_v (pBuffer, nBufferLength, nBitOffset);
			sps->vui_seq_parameters.log2_max_mv_length_horizontal           =  ue_v (pBuffer, nBufferLength, nBitOffset);
			sps->vui_seq_parameters.log2_max_mv_length_vertical             =  ue_v (pBuffer, nBufferLength, nBitOffset);
			sps->vui_seq_parameters.num_reorder_frames                      =  ue_v (pBuffer, nBufferLength, nBitOffset);
			sps->vui_seq_parameters.max_dec_frame_buffering                 =  ue_v (pBuffer, nBufferLength, nBitOffset);
		}
	}
}
*/


void CDXVADecoderH264::ReadSPS(SEQ_PARAMETER_SET_RBSP* sps, BYTE* pBuffer, UINT nBufferLength)
{
	UINT		nBitOffset = 8;
	unsigned	i;
	unsigned	n_ScalingList;
	int			reserved_zero;


	sps->profile_idc                            = u_v  (8, pBuffer, nBufferLength, nBitOffset);

	if ((sps->profile_idc!=66 ) &&
		(sps->profile_idc!=77 ) &&
		(sps->profile_idc!=88 ) &&
		(sps->profile_idc!=FREXT_HP    ) &&
		(sps->profile_idc!=FREXT_Hi10P ) &&
		(sps->profile_idc!=FREXT_Hi422 ) &&
		(sps->profile_idc!=FREXT_Hi444 ) &&
		(sps->profile_idc!=FREXT_CAVLC444 ))
	{
		return;
	}

	sps->constrained_set0_flag                  = u_1  (pBuffer, nBufferLength, nBitOffset);
	sps->constrained_set1_flag                  = u_1  (pBuffer, nBufferLength, nBitOffset);
	sps->constrained_set2_flag                  = u_1  (pBuffer, nBufferLength, nBitOffset);
	sps->constrained_set3_flag                  = u_1  (pBuffer, nBufferLength, nBitOffset);
	reserved_zero                               = u_v  (4, pBuffer, nBufferLength, nBitOffset);
	ASSERT (reserved_zero==0);

	sps->level_idc                              = u_v  (8, pBuffer, nBufferLength, nBitOffset);

	sps->seq_parameter_set_id                   = ue_v (pBuffer, nBufferLength, nBitOffset);

	// Fidelity Range Extensions stuff
	sps->chroma_format_idc = 1;
	sps->bit_depth_luma_minus8   = 0;
	sps->bit_depth_chroma_minus8 = 0;
//	img->lossless_qpprime_flag   = 0;
	sps->residual_colour_transform_flag = 0;

	if((sps->profile_idc==FREXT_HP   ) ||
		(sps->profile_idc==FREXT_Hi10P) ||
		(sps->profile_idc==FREXT_Hi422) ||
		(sps->profile_idc==FREXT_Hi444) ||
		(sps->profile_idc==FREXT_CAVLC444))
	{
		sps->chroma_format_idc                      = ue_v (pBuffer, nBufferLength, nBitOffset);

		if(sps->chroma_format_idc == YUV444)
		{
			sps->residual_colour_transform_flag           = u_1  (pBuffer, nBufferLength, nBitOffset);
		}

		sps->bit_depth_luma_minus8                  = ue_v (pBuffer, nBufferLength, nBitOffset);
		sps->bit_depth_chroma_minus8                = ue_v (pBuffer, nBufferLength, nBitOffset);
		/*img->lossless_qpprime_flag             = */ u_1  (pBuffer, nBufferLength, nBitOffset);

		sps->seq_scaling_matrix_present_flag        = u_1  (pBuffer, nBufferLength, nBitOffset);

		if(sps->seq_scaling_matrix_present_flag)
		{
			n_ScalingList = (sps->chroma_format_idc != YUV444) ? 8 : 12;
			for(i=0; i<n_ScalingList; i++)
			{
				sps->seq_scaling_list_present_flag[i]   = u_1  (pBuffer, nBufferLength, nBitOffset);
				if(sps->seq_scaling_list_present_flag[i])
				{
					//if(i<6)
					//	Scaling_List(sps->ScalingList4x4[i], 16, &sps->UseDefaultScalingMatrix4x4Flag[i], s);
					//else
					//	Scaling_List(sps->ScalingList8x8[i-6], 64, &sps->UseDefaultScalingMatrix8x8Flag[i-6], s);
				}
			}
		}
	}

	sps->log2_max_frame_num_minus4              = ue_v (pBuffer, nBufferLength, nBitOffset);
	sps->pic_order_cnt_type                     = ue_v (pBuffer, nBufferLength, nBitOffset);

	if (sps->pic_order_cnt_type == 0)
		sps->log2_max_pic_order_cnt_lsb_minus4 = ue_v (pBuffer, nBufferLength, nBitOffset);
	else if (sps->pic_order_cnt_type == 1)
	{
		sps->delta_pic_order_always_zero_flag      = u_1  (pBuffer, nBufferLength, nBitOffset);
		sps->offset_for_non_ref_pic                = se_v (pBuffer, nBufferLength, nBitOffset);
		sps->offset_for_top_to_bottom_field        = se_v (pBuffer, nBufferLength, nBitOffset);
		sps->num_ref_frames_in_pic_order_cnt_cycle = ue_v (pBuffer, nBufferLength, nBitOffset);
		for(i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
			sps->offset_for_ref_frame[i]               = se_v (pBuffer, nBufferLength, nBitOffset);
	}
	sps->num_ref_frames                        = ue_v (pBuffer, nBufferLength, nBitOffset);
	sps->gaps_in_frame_num_value_allowed_flag  = u_1  (pBuffer, nBufferLength, nBitOffset);
	sps->pic_width_in_mbs_minus1               = ue_v (pBuffer, nBufferLength, nBitOffset);
	sps->pic_height_in_map_units_minus1        = ue_v (pBuffer, nBufferLength, nBitOffset);
	sps->frame_mbs_only_flag                   = u_1  (pBuffer, nBufferLength, nBitOffset);
	if (!sps->frame_mbs_only_flag)
	{
		sps->mb_adaptive_frame_field_flag        = u_1  (pBuffer, nBufferLength, nBitOffset);
	}
	sps->direct_8x8_inference_flag             = u_1  (pBuffer, nBufferLength, nBitOffset);
	sps->frame_cropping_flag                   = u_1  (pBuffer, nBufferLength, nBitOffset);

	if (sps->frame_cropping_flag)
	{
		sps->frame_cropping_rect_left_offset      = ue_v (pBuffer, nBufferLength, nBitOffset);
		sps->frame_cropping_rect_right_offset     = ue_v (pBuffer, nBufferLength, nBitOffset);
		sps->frame_cropping_rect_top_offset       = ue_v (pBuffer, nBufferLength, nBitOffset);
		sps->frame_cropping_rect_bottom_offset    = ue_v (pBuffer, nBufferLength, nBitOffset);
	}
	sps->vui_parameters_present_flag           = (bool) u_1  (pBuffer, nBufferLength, nBitOffset);

//	ReadVUI(sps, pBuffer, nBufferLength, nBitOffset);

	sps->Valid = TRUE;
}


void CDXVADecoderH264::SetExtraData (BYTE* pDataIn, UINT nSize)
{
	int		nCount = 0;
	int		nNaluSize;
	NALU 	Nalu;
	
	nNaluSize  = (pDataIn[0] << 8) + pDataIn[1] + 2;
	pDataIn   += 2;
	while (nCount < 2)
	{
		ReadNalu (&Nalu, pDataIn, nSize);
		switch (Nalu.nal_unit_type)
		{
		case NALU_TYPE_PPS :
			ReadPPS(&m_PicParam, pDataIn, nSize);
			break;
		case NALU_TYPE_SPS :
			ReadSPS(&m_SeqParam, pDataIn, nSize);
			break;
		}

		pDataIn   += nNaluSize;
		nCount++;
	}

	InitPictureParams();
}