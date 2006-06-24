/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: comp_decompress.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
*
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License
* Version 1.1 (the "License"); you may not use this file except in compliance
* with the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
* the specific language governing rights and limitations under the License.
*
* The Original Code is BBC Research and Development code.
*
* The Initial Developer of the Original Code is the British Broadcasting
* Corporation.
* Portions created by the Initial Developer are Copyright (C) 2004.
* All Rights Reserved.
*
* Contributor(s): Thomas Davies (Original Author), Scott R Ladd
*
* Alternatively, the contents of this file may be used under the terms of
* the GNU General Public License Version 2 (the "GPL"), or the GNU Lesser
* Public License Version 2.1 (the "LGPL"), in which case the provisions of
* the GPL or the LGPL are applicable instead of those above. If you wish to
* allow use of your version of this file only under the terms of the either
* the GPL or LGPL and not to allow others to use your version of this file
* under the MPL, indicate your decision by deleting the provisions above
* and replace them with the notice and other provisions required by the GPL
* or LGPL. If you do not delete the provisions above, a recipient may use
* your version of this file under the terms of any one of the MPL, the GPL
* or the LGPL.
* ***** END LICENSE BLOCK ***** */


#include <libdirac_decoder/comp_decompress.h>
#include <libdirac_common/wavelet_utils.h>
#include <libdirac_common/band_codec.h>
#include <libdirac_common/golomb.h>
using namespace dirac;

#include <vector>

#include <ctime>

using std::vector;

//Constructor
CompDecompressor::CompDecompressor( DecoderParams& decp, const FrameParams& fp)
:
m_qflist(60),
m_decparams(decp),
m_fparams(fp)
{}


void CompDecompressor::Decompress(PicArray& pic_data)
{
	const FrameSort& fsort=m_fparams.FSort();
	const int depth=4;
	BandCodec* bdecoder;
    const size_t CONTEXTS_REQUIRED = 24;
	Subband node;
	unsigned int max_bits;
	int qf_idx;

	WaveletTransform wtransform(depth);
	SubbandList& bands=wtransform.BandList();
	bands.Init(depth , pic_data.LengthX() , pic_data.LengthY());

	GenQuantList();

	for (int I=bands.Length();I>=1;--I)
	{

		//read the header data first
		qf_idx=GolombDecode( m_decparams.BitsIn() );
		if (qf_idx!=-1){
			bands(I).SetQf(0,m_qflist[qf_idx]);
			max_bits=UnsignedGolombDecode( m_decparams.BitsIn() );
			m_decparams.BitsIn().FlushInput();

			if (I>=bands.Length()){
				if (fsort==I_frame && I==bands.Length())
					bdecoder=new IntraDCBandCodec( &m_decparams.BitsIn() , CONTEXTS_REQUIRED ,bands);
				else
					bdecoder=new LFBandCodec( &m_decparams.BitsIn() , CONTEXTS_REQUIRED ,bands , I);
			}
			else
				bdecoder=new BandCodec( &m_decparams.BitsIn() , CONTEXTS_REQUIRED , bands , I);

			bdecoder->InitContexts();
			bdecoder->Decompress(pic_data,max_bits);
			delete bdecoder;
		}
		else{
			m_decparams.BitsIn().FlushInput();
			if (I==bands.Length() && fsort==I_frame)
				SetToVal(pic_data,bands(I),2692);
			else
				SetToVal(pic_data,bands(I),0);
		}
	}
	wtransform.Transform(BACKWARD,pic_data);
}

void CompDecompressor::SetToVal(PicArray& pic_data,const Subband& node,ValueType val){
	for (int J=node.Yp();J<node.Yp()+node.Yl();++J)
	{	
		for (int I=node.Xp();I<node.Xp()+node.Xl();++I)
		{
			pic_data[J][I]=val;
		}
	}
}

void CompDecompressor::GenQuantList(){//generates the list of quantisers and inverse quantisers
	//there is some repetition in this list but at the moment this is easiest from the perspective of SelectQuant
	//Need to remove this repetition later TJD 29 March 04.

	m_qflist[0]=1;		
	m_qflist[1]=1;		
	m_qflist[2]=1;		
	m_qflist[3]=1;		
	m_qflist[4]=2;		
	m_qflist[5]=2;		
	m_qflist[6]=2;		
	m_qflist[7]=3;		
	m_qflist[8]=4;		
	m_qflist[9]=4;		
	m_qflist[10]=5;		
	m_qflist[11]=6;		
	m_qflist[12]=8;		
	m_qflist[13]=9;		
	m_qflist[14]=11;		
	m_qflist[15]=13;		
	m_qflist[16]=16;		
	m_qflist[17]=19;		
	m_qflist[18]=22;		
	m_qflist[19]=26;		
	m_qflist[20]=32;		
	m_qflist[21]=38;		
	m_qflist[22]=45;		
	m_qflist[23]=53;		
	m_qflist[24]=64;		
	m_qflist[25]=76;		
	m_qflist[26]=90;		
	m_qflist[27]=107;		
	m_qflist[28]=128;		
	m_qflist[29]=152;		
	m_qflist[30]=181;		
	m_qflist[31]=215;		
	m_qflist[32]=256;		
	m_qflist[33]=304;		
	m_qflist[34]=362;		
	m_qflist[35]=430;		
	m_qflist[36]=512;		
	m_qflist[37]=608;		
	m_qflist[38]=724;		
	m_qflist[39]=861;		
	m_qflist[40]=1024;	
	m_qflist[41]=1217;	
	m_qflist[42]=1448;	
	m_qflist[43]=1722;	
	m_qflist[44]=2048;	
	m_qflist[45]=2435;	
	m_qflist[46]=2896;	
	m_qflist[47]=3444;	
	m_qflist[48]=4096;	
	m_qflist[49]=4870;	
	m_qflist[50]=5792;	
	m_qflist[51]=6888;	
	m_qflist[52]=8192;	
	m_qflist[53]=9741;	
	m_qflist[54]=11585;	
	m_qflist[55]=13777;	
	m_qflist[56]=16384;	
	m_qflist[57]=19483;	
	m_qflist[58]=23170;	
	m_qflist[59]=27554;		
}
