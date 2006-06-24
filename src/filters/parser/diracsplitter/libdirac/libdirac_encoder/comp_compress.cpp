/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: comp_compress.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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
* Contributor(s): Thomas Davies (Original Author),
*                 Scott R Ladd,
*                 Anuradha Suraparaju,
*                 Peter Meerwald
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


//Compression of an individual component,
//after motion compensation if appropriate
//////////////////////////////////////////

#include <libdirac_encoder/comp_compress.h>
#include <libdirac_common/band_codec.h>
#include <libdirac_common/golomb.h>
using namespace dirac;

#include <ctime>
#include <vector>
#include <iostream>

using std::log;
using std::floor;

static inline double pow4 (double x)
{
    return x * x * x* x;
}

CompCompressor::CompCompressor( EncoderParams& encp,const FrameParams& fp)
: m_encparams(encp),
  m_fparams(fp),
  m_fsort( m_fparams.FSort() ),
  m_cformat( m_fparams.CFormat() ),
  m_qflist(60),
  m_qfinvlist(60),
  m_offset(60)
{}

void CompCompressor::Compress(PicArray& pic_data)
{

    //need to transform, select quantisers for each band, and then compress each component in turn
    m_csort=pic_data.CSort();
    const int depth=4;
    unsigned int num_band_bits;

    // A pointer to an object  for coding the subband data
    BandCodec* bcoder;

    // A pointer to an object for outputting the subband data
    UnitOutputManager* band_op;

    const size_t CONTEXTS_REQUIRED = 24;

    Subband node;

    //set up Lagrangian params    
    if (m_fsort == I_frame) 
        m_lambda= m_encparams.ILambda();
    else if (m_fsort == L1_frame) 
        m_lambda= m_encparams.L1Lambda();
    else 
        m_lambda= m_encparams.L2Lambda();

     if (m_csort == U_COMP) 
         m_lambda*= m_encparams.UFactor();
     if (m_csort == V_COMP) 
         m_lambda*= m_encparams.VFactor();

    WaveletTransform wtransform(depth);

    wtransform.Transform( FORWARD , pic_data );
    wtransform.SetBandWeights( m_encparams.CPD() , m_fparams.FSort() , m_fparams.CFormat(), m_csort);

    SubbandList& bands=wtransform.BandList();

    // Generate all the quantisation data
    GenQuantList();

    // Choose all the quantisers
    OneDArray<unsigned int> estimated_bits( Range( 1 , bands.Length() ) );
    SelectQuantisers( pic_data , bands , estimated_bits );  

    // Loop over all the bands (from DC to HF) quantising and coding them
    for (int b=bands.Length() ; b>=1 ; --b )
    {
        band_op = & m_encparams.BitsOut().FrameOutput().BandOutput( m_csort , b );

        GolombCode( band_op->Header() , bands(b).Qf(0) );

        if (bands(b).Qf(0) != -1)
        {   // If not skipped ...

            bands(b).SetQf( 0 , m_qflist[bands(b).Qf(0)] );

             // Pick the right codec according to the frame type and subband
            if (b >= bands.Length())
            {
                if ( m_fsort == I_frame && b == bands.Length() )
                    bcoder=new IntraDCBandCodec( &( band_op->Data() ) , CONTEXTS_REQUIRED , bands);
                else
                    bcoder=new LFBandCodec( &( band_op->Data() ) ,CONTEXTS_REQUIRED, bands , b);
            }
            else
                bcoder=new BandCodec( &( band_op->Data() ) , CONTEXTS_REQUIRED , bands , b);

            num_band_bits = bcoder->Compress(pic_data);

             // Update the entropy correction factors
            m_encparams.EntropyFactors().Update(b , m_fsort , m_csort , estimated_bits[b] , num_band_bits);

            // Write the length of the data chunk into the header, and flush everything out to file
            UnsignedGolombCode( band_op->Header() , num_band_bits);

            delete bcoder;            
        }
        else
        {   // ... skipped

            if (b == bands.Length() && m_fsort == I_frame)
                SetToVal( pic_data , bands(b) , 2692 );
            else
                SetToVal( pic_data , bands(b) , 0 );
        }        
    }//b

    // Transform back into the picture domain
    wtransform.Transform( BACKWARD , pic_data );

}

void CompCompressor::GenQuantList()
{    //generates the list of quantisers and inverse quantisers
    //there is some repetition in this list but at the moment this is easiest from the perspective of SelectQuant
    //Need to remove this repetition later

    m_qflist[0]=1;        m_qfinvlist[0]=131072;    m_offset[0]=0;
    m_qflist[1]=1;        m_qfinvlist[1]=131072;    m_offset[1]=0;
    m_qflist[2]=1;        m_qfinvlist[2]=131072;    m_offset[2]=0;
    m_qflist[3]=1;        m_qfinvlist[3]=131072;    m_offset[3]=0;
    m_qflist[4]=2;        m_qfinvlist[4]=65536;        m_offset[4]=1;
    m_qflist[5]=2;        m_qfinvlist[5]=65536;        m_offset[5]=1;
    m_qflist[6]=2;        m_qfinvlist[6]=65536;        m_offset[6]=1;
    m_qflist[7]=3;        m_qfinvlist[7]=43690;        m_offset[7]=1;
    m_qflist[8]=4;        m_qfinvlist[8]=32768;        m_offset[8]=2;
    m_qflist[9]=4;        m_qfinvlist[9]=32768;        m_offset[9]=2;
    m_qflist[10]=5;        m_qfinvlist[10]=26214;    m_offset[10]=2;
    m_qflist[11]=6;        m_qfinvlist[11]=21845;    m_offset[11]=2;
    m_qflist[12]=8;        m_qfinvlist[12]=16384;    m_offset[12]=3;
    m_qflist[13]=9;        m_qfinvlist[13]=14563;    m_offset[13]=3;
    m_qflist[14]=11;        m_qfinvlist[14]=11915;    m_offset[14]=4;
    m_qflist[15]=13;        m_qfinvlist[15]=10082;    m_offset[15]=5;
    m_qflist[16]=16;        m_qfinvlist[16]=8192;        m_offset[16]=6;
    m_qflist[17]=19;        m_qfinvlist[17]=6898;        m_offset[17]=7;
    m_qflist[18]=22;        m_qfinvlist[18]=5957;        m_offset[18]=8;
    m_qflist[19]=26;        m_qfinvlist[19]=5041;        m_offset[19]=10;
    m_qflist[20]=32;        m_qfinvlist[20]=4096;        m_offset[20]=12;
    m_qflist[21]=38;        m_qfinvlist[21]=3449;        m_offset[21]=14;
    m_qflist[22]=45;        m_qfinvlist[22]=2912;        m_offset[22]=17;
    m_qflist[23]=53;        m_qfinvlist[23]=2473;        m_offset[23]=20;
    m_qflist[24]=64;        m_qfinvlist[24]=2048;        m_offset[24]=24;
    m_qflist[25]=76;        m_qfinvlist[25]=1724;        m_offset[25]=29;
    m_qflist[26]=90;        m_qfinvlist[26]=1456;        m_offset[26]=34;
    m_qflist[27]=107;        m_qfinvlist[27]=1224;        m_offset[27]=40;
    m_qflist[28]=128;        m_qfinvlist[28]=1024;        m_offset[28]=48;
    m_qflist[29]=152;        m_qfinvlist[29]=862;        m_offset[29]=57;
    m_qflist[30]=181;        m_qfinvlist[30]=724;        m_offset[30]=68;
    m_qflist[31]=215;        m_qfinvlist[31]=609;        m_offset[31]=81;
    m_qflist[32]=256;        m_qfinvlist[32]=512;        m_offset[32]=96;
    m_qflist[33]=304;        m_qfinvlist[33]=431;        m_offset[33]=114;
    m_qflist[34]=362;        m_qfinvlist[34]=362;        m_offset[34]=136;
    m_qflist[35]=430;        m_qfinvlist[35]=304;        m_offset[35]=161;
    m_qflist[36]=512;        m_qfinvlist[36]=256;        m_offset[36]=192;
    m_qflist[37]=608;        m_qfinvlist[37]=215;        m_offset[37]=228;
    m_qflist[38]=724;        m_qfinvlist[38]=181;        m_offset[38]=272;
    m_qflist[39]=861;        m_qfinvlist[39]=152;        m_offset[39]=323;
    m_qflist[40]=1024;    m_qfinvlist[40]=128;        m_offset[40]=384;
    m_qflist[41]=1217;    m_qfinvlist[41]=107;        m_offset[41]=456;
    m_qflist[42]=1448;    m_qfinvlist[42]=90;        m_offset[42]=543;
    m_qflist[43]=1722;    m_qfinvlist[43]=76;        m_offset[43]=646;
    m_qflist[44]=2048;    m_qfinvlist[44]=64;        m_offset[44]=768;
    m_qflist[45]=2435;    m_qfinvlist[45]=53;        m_offset[45]=913;
    m_qflist[46]=2896;    m_qfinvlist[46]=45;        m_offset[46]=1086;
    m_qflist[47]=3444;    m_qfinvlist[47]=38;        m_offset[47]=1292;
    m_qflist[48]=4096;    m_qfinvlist[48]=32;        m_offset[48]=1536;
    m_qflist[49]=4870;    m_qfinvlist[49]=26;        m_offset[49]=1826;
    m_qflist[50]=5792;    m_qfinvlist[50]=22;        m_offset[50]=2172;
    m_qflist[51]=6888;    m_qfinvlist[51]=19;        m_offset[51]=2583;
    m_qflist[52]=8192;    m_qfinvlist[52]=16;        m_offset[52]=3072;
    m_qflist[53]=9741;    m_qfinvlist[53]=13;        m_offset[53]=3653;
    m_qflist[54]=11585;    m_qfinvlist[54]=11;        m_offset[54]=4344;
    m_qflist[55]=13777;    m_qfinvlist[55]=9;        m_offset[55]=5166;
    m_qflist[56]=16384;    m_qfinvlist[56]=8;        m_offset[56]=6144;
    m_qflist[57]=19483;    m_qfinvlist[57]=6;        m_offset[57]=7306;
    m_qflist[58]=23170;    m_qfinvlist[58]=5;        m_offset[58]=8689;
    m_qflist[59]=27554;    m_qfinvlist[59]=4;        m_offset[59]=10333;    
}



void CompCompressor::SelectQuantisers( PicArray& pic_data , SubbandList& bands ,
                                                 OneDArray<unsigned int>& est_counts )
{
    // Select all the quantizers
    for ( int b=bands.Length() ; b>=1 ; --b )
        est_counts[b] = SelectQuant( pic_data , bands , b );
}

int CompCompressor::SelectQuant(PicArray& pic_data,SubbandList& bands,const int band_num)
{

    Subband& node=bands(band_num);

    const int qf_start_idx = 4;

    if (band_num==bands.Length())
        AddSubAverage(pic_data,node.Xl(),node.Yl(),SUBTRACT);

    int min_idx;
    double bandmax=PicAbsMax(pic_data,node.Xp(),node.Yp(),node.Xl(),node.Yl());

    if (bandmax>=1)
        node.SetMax(int(floor(log(float(bandmax))/log(2.0))));
    else
        node.SetMax(0);
    int length=4*node.Max()+5;//this is the number of quantisers that are possible

    OneDArray<int> count0(length);
    int count1;    
    OneDArray<int> countPOS(length);
    OneDArray<int> countNEG(length);    
    OneDArray<double> error_total(length);    
    OneDArray<CostType> costs(length);
    int quant_val;    
    ValueType val,abs_val;
    int error;
    double p0,p1;    
    double sign_entropy;    

    int xp=node.Xp();
    int yp=node.Yp();
    int xl=node.Xl();
    int yl=node.Yl();
    double vol;

    if (bandmax < 1.0 )
    {
        //coefficients are zero so the subband can be skipped
        node.SetQf(0,-1);//indicates that the subband is skipped

        if ( band_num == bands.Length() )
            AddSubAverage(pic_data,node.Xl(),node.Yl(),ADD);
    
        return 0;        
    }
    else
    {
        for ( int q=0 ; q<costs.Length() ; q++)
        {
            error_total[q] = 0.0;            
            count0[q] = 0;
            countPOS[q] = 0;
            countNEG[q] = 0;            
        }

        //first, find to nearest integral number of bits using 1/4 of the data
        //////////////////////////////////////////////////////////////////////
        vol=double((yl/2)*(xl/2));//vol is only 1/4 of the coeffs
        count1=int(vol);
        for ( int j=yp+1 ; j<yp+yl ; j+=2 )
        {
            for ( int i=xp+((j-yp)%4)/2 ; i<xp+xl ; i+=2)
            {

                val = pic_data[j][i];
                quant_val = abs(val);
                abs_val = quant_val;

                for ( int q=qf_start_idx ; q<costs.Length() ; q+=4)
                {
                    quant_val >>= (q/4);                                

                    if (quant_val)
                    {
                        count0[q]+=quant_val;
                        quant_val <<= (q/4);                        
                        if (val>0)
                            countPOS[q]++;
                        else
                            countNEG[q]++;
                    }

                    error = abs_val-quant_val;

                    if ( quant_val != 0)                    
                        error -= m_offset[q];

                    error_total[q] +=  pow4( static_cast<double>(error) );
                }// q
            }// i
        }// j

         //do entropy calculation etc        
        for ( int q=qf_start_idx ; q<costs.Length() ; q+=4 )
        {
            costs[q].MSE = error_total[q]/( vol*node.Wt()*node.Wt() );
//
            costs[q].MSE = std::sqrt( costs[q].MSE );
//     
             //calculate probabilities and entropy
            p0 = double( count0[q] )/double( count0[q]+count1 );
            p1 = 1.0-p0;

            if ( p0 != 0.0 && p1 != 0.0)
                costs[q].ENTROPY =- (p0*log(p0)+p1*log(p1))/log(2.0);
            else
                costs[q].ENTROPY = 0.0;

            //we want the entropy *per symbol*, not per bit ...            
            costs[q].ENTROPY *= double(count0[q]+count1);
            costs[q].ENTROPY /= vol;

            //now add in the sign entropy
            if ( countPOS[q]+countNEG[q] != 0 )
            {
                p0 = float(countNEG[q])/float( countPOS[q]+countNEG[q] );
                p1 = 1.0-p0;
                if ( p0 != 0.0 && p1 != 0.0)
                    sign_entropy = -( (p0*log(p0)+p1*log(p1) ) / log(2.0));
                else
                    sign_entropy = 0.0;
            }
            else
                sign_entropy=0.0;    

              //we want the entropy *per symbol*, not per bit ...
            sign_entropy *= double( countNEG[q]+countPOS[q] );
            sign_entropy /= vol;    

            costs[q].ENTROPY += sign_entropy;

            //sort out correction factors
            costs[q].ENTROPY *= m_encparams.EntropyFactors().Factor(band_num,m_fsort,m_csort);
            costs[q].TOTAL = costs[q].MSE+m_lambda*costs[q].ENTROPY;

        }// q

        //find the qf with the lowest cost
        min_idx=qf_start_idx;
        for ( int q=qf_start_idx ; q<costs.Length() ; q+=4 ) 
        {
            if ( costs[q].TOTAL < costs[min_idx].TOTAL )
                min_idx=q;
        }

        //now repeat to get to 1/2 bit accuracy
        ///////////////////////////////////////
        for ( int q=std::max(0,min_idx-2) ; q<=std::min(costs.Last(),min_idx+2) ; q+=2 )
        {
            if ( q != min_idx )
            {
                error_total[q] = 0.0;            
                count0[q] = 0;
                countPOS[q] = 0;
                countNEG[q] = 0;            
            }
        }

        vol = double( (yl/2) * (xl/2) );
        count1 = int(vol);
        int top_idx = std::min(costs.Last(),min_idx+2);
        int bottom_idx = std::max(0,min_idx-2);

        for (int j=yp+1 ; j<yp+yl ; j+=2 )
        {
            for (int i=xp+1 ; i<xp+xl ; i+=2 )
            {
                val = pic_data[j][i];
                abs_val = abs(val);

                for ( int q=bottom_idx ; q<=top_idx ; q+=2 )
                {
                    if ( q != min_idx )
                    {
                        quant_val = int(abs_val);                    
                        quant_val *= m_qfinvlist[q];
                        quant_val >>= 17;

                        if ( quant_val )
                        {
                            count0[q] += quant_val;
                            quant_val *= m_qflist[q];                        

                            if (val>0.0)
                                countPOS[q]++;
                            else
                                countNEG[q]++;
                        }

                        error = abs_val-quant_val;

                        if ( quant_val != 0 )                    
                            error -= m_offset[q];

                        error_total[q] += pow4( static_cast<double>(error) );
                    }//end of if
                }//q
            }//J
        }//I

         //do entropy calculation        
        for ( int q=bottom_idx ; q<=top_idx ; q+=2 )
        {
            if ( q != min_idx )
            {
                costs[q].MSE = error_total[q] / (vol*node.Wt()*node.Wt());
//
                costs[q].MSE = std::sqrt( costs[q].MSE );
//     


                  //calculate probabilities and entropy
                p0 = double(count0[q]) / double(count0[q]+count1);
                p1 = 1.0-p0;

                if (p0 != 0.0 && p1 != 0.0)
                    costs[q].ENTROPY =- (p0*log(p0) + p1*log(p1)) / log(2.0);
                else
                    costs[q].ENTROPY = 0.0;
                //we want the entropy *per symbol*, not per bit ...            
                costs[q].ENTROPY *= count0[q]+count1;
                costs[q].ENTROPY /= vol;

                  //now add in the sign entropy
                if (countPOS[q]+countNEG[q] != 0)
                {
                    p0 = double( countNEG[q] )/double( countPOS[q] + countNEG[q] );
                    p1 = 1.0-p0;
                    if (p0 != 0.0 && p1 != 0.0)
                        sign_entropy =- ( (p0*log(p0)+p1*log(p1)) / log(2.0) );
                    else
                        sign_entropy = 0.0;
                }
                else
                    sign_entropy = 0.0;    

                  //we want the entropy *per symbol*, not per bit ...
                sign_entropy *= double(countNEG[q]+countPOS[q]);
                sign_entropy /= vol;    

                costs[q].ENTROPY += sign_entropy;
                //sort out correction factors
                costs[q].ENTROPY *= m_encparams.EntropyFactors().Factor(band_num,m_fsort,m_csort);
                costs[q].TOTAL = costs[q].MSE+m_lambda*costs[q].ENTROPY;
            }
        }//q

         //find the qf with the lowest cost
        for ( int q=bottom_idx ; q<=top_idx ; q+=2 )
        {
            if ( costs[q].TOTAL < costs[min_idx].TOTAL )
                min_idx = q;
        }

         //finally use 1/2 the values to get 1/4 bit accuracy
        ////////////////////////////////////////////////////        

        bottom_idx=std::max(0,min_idx-1);
        top_idx=std::min(costs.Length()-1,min_idx+1);

        for ( int q=bottom_idx ; q<=top_idx ; q++ )
        {
            error_total[q] = 0.0;            
            count0[q] = 0;
            countPOS[q] = 0;
            countNEG[q] = 0;            
        }

        vol = double( (yl/2) * xl );
        count1 = int( vol );        

        for (int j=yp ; j<yp+yl ; ++j )
        {                
            for (int i=xp+1 ; i<xp+xl ; i+=2)
            {                
                val = pic_data[j][i];
                abs_val = abs(val);

                for (int q=bottom_idx;q<=top_idx;q++)
                {
                    quant_val = int(abs_val);                    
                    quant_val *= m_qfinvlist[q];
                    quant_val >>= 17;

                    if (quant_val)
                    {
                        count0[q] += quant_val;
                        quant_val *= m_qflist[q];                        

                        if ( val > 0 )
                            countPOS[q]++;
                        else
                            countNEG[q]++;
                    }

                    error = abs_val - quant_val;

                    if ( quant_val != 0 )                    
                        error -= m_offset[q];

                    error_total[q] +=  pow4( static_cast<double>(error) );
                }//q
            }//i
        }//j

         //do entropy calculation        
        for ( int q=bottom_idx ; q<=top_idx ; q++ )
        {
            costs[q].MSE = error_total[q]/(vol*node.Wt()*node.Wt());
//
            costs[q].MSE = std::sqrt( costs[q].MSE );
//
             //calculate probabilities and entropy
            p0 = double( count0[q] )/ double( count0[q]+count1 );
            p1 = 1.0 - p0;

            if ( p0 != 0.0 && p1 != 0.0)
                costs[q].ENTROPY = -( p0*log(p0)+p1*log(p1) ) / log(2.0);
            else
                costs[q].ENTROPY = 0.0;

              //we want the entropy *per symbol*, not per bit ...            
            costs[q].ENTROPY *= double(count0[q]+count1);
            costs[q].ENTROPY /= vol;

             //now add in the sign entropy
            if ( countPOS[q] + countNEG[q] != 0 )
            {
                p0 = double( countNEG[q] )/double( countPOS[q]+countNEG[q] );
                p1 = 1.0-p0;
                if ( p0 != 0.0 && p1 != 0.0)
                    sign_entropy = -( (p0*log(p0)+p1*log(p1) ) / log(2.0));
                else
                    sign_entropy = 0.0;
            }
            else
                sign_entropy = 0.0;    

              //we want the entropy *per symbol*, not per bit ...
            sign_entropy *= double(countNEG[q]+countPOS[q]);
            sign_entropy /= vol;    

            costs[q].ENTROPY += sign_entropy;

            //sort out correction factors
            costs[q].ENTROPY *= m_encparams.EntropyFactors().Factor(band_num,m_fsort,m_csort);
            costs[q].TOTAL = costs[q].MSE+m_lambda*costs[q].ENTROPY;

        }//q

         //find the qf with the lowest cost
        for ( int q=bottom_idx ; q<=top_idx ; q++ )
        {
            if ( costs[q].TOTAL < costs[min_idx].TOTAL )
                min_idx=q;
        }

        if ( costs[min_idx].ENTROPY == 0.0 )//then can skip after all
            node.SetQf(0,-1);
        else
            node.SetQf(0,min_idx);

        if ( band_num == bands.Length())
            AddSubAverage(pic_data,node.Xl(),node.Yl(),ADD);

        return int(costs[min_idx].ENTROPY*double(xl*yl));
    }

}

ValueType CompCompressor::PicAbsMax(const PicArray& pic_data) const
{
    //finds the maximum absolute value of the picture array
    return PicAbsMax(pic_data,pic_data.FirstX() , pic_data.FirstY(),
                     pic_data.LengthX(),pic_data.LengthY());
}

ValueType CompCompressor::PicAbsMax(const PicArray& pic_data,int xp, int yp ,int xl ,int yl) const
{

    int first_x=std::max(pic_data.FirstX(),xp);    
    int first_y=std::max(pic_data.FirstY(),yp);    
    int last_x=std::min(pic_data.LastX(),xp+xl-1);    
    int last_y=std::min(pic_data.LastY(),yp+yl-1);        
    ValueType val=0;

    for (int j=first_y ; j<=last_y; ++j)
    {
        for (int i=first_x ; i<=last_x; ++i)
        {    
            val = std::max( val , pic_data[j][i] );    
        }// i
    }// j

    return val;
}

void CompCompressor::SetToVal(PicArray& pic_data,const Subband& node,ValueType val){

    for (int j=node.Yp() ; j<node.Yp() + node.Yl() ; ++j)
    {    
        for (int i=node.Xp(); i<node.Xp() + node.Xl() ; ++i)
        {
            pic_data[j][i] = val;
        }// i
    }// j

}

void CompCompressor::AddSubAverage(PicArray& pic_data,int xl,int yl,AddOrSub dirn){

    ValueType last_val=2692;//corresponds to mid-grey in this DC band with these filters
                            //NB this is hard-wired for a level 4 transform
    ValueType last_val2;
 
    if ( dirn == SUBTRACT )
    {
        for ( int j=0 ; j<yl ; j++)
            {
            for ( int i=0 ; i<xl ; i++)
                {
                last_val2 = pic_data[j][i];        
                pic_data[j][i] -= last_val;
                last_val = last_val2;
            }// i
        }// j
    }
    else
    {
        for ( int j=0 ; j<yl ; j++)
        {
            for ( int i=0 ; i<xl; i++ )
            {
                pic_data[j][i] += last_val;
                last_val = pic_data[j][i];
            }// i
        }// j

    }
}
