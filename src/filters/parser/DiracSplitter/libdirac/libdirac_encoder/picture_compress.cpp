/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: picture_compress.cpp,v 1.28 2009/01/21 05:20:57 asuraparaju Exp $ $Name:  $
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
*                 Chris Bowley,
*                 Anuradha Suraparaju,
*                 Tim Borer,
*                 Andrew Kennedy
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

//Compression of pictures//
/////////////////////////

#include <libdirac_encoder/picture_compress.h>
#include <libdirac_encoder/comp_compress.h>
#include <libdirac_encoder/prefilter.h>
#include <libdirac_common/mot_comp.h>
#include <libdirac_motionest/pixel_match.h>
#include <libdirac_motionest/me_subpel.h>
#include <libdirac_motionest/me_mode_decn.h>
#include <libdirac_common/mv_codec.h>
#include <libdirac_encoder/quant_chooser.h>
#include <libdirac_common/dirac_assertions.h>
using namespace dirac;

#include <iostream>
#include <sstream>

PictureCompressor::PictureCompressor( EncoderParams& encp ) :
    m_encparams(encp),
    m_skipped(false),
    m_use_global(false),
    m_use_block_mv(true),
    m_global_pred_mode(REF1_ONLY),
    m_me_data(NULL),
    m_medata_avail(false),
    m_is_a_cut(false)
{}

PictureCompressor::~PictureCompressor()
{}


void PictureCompressor::PixelME( EncQueue& my_buffer , int pnum )
{
    PixelMatcher pix_match( m_encparams );
    pix_match.DoSearch( my_buffer , pnum );
}

void PictureCompressor::CalcComplexity( EncQueue& my_buffer, int pnum , const OLBParams& olbparams )
{
    EncPicture& my_picture = my_buffer.GetPicture( pnum );
    PictureParams& pparams = my_picture.GetPparams();

    if ( (my_picture.GetStatus()&DONE_PEL_ME) != 0 ){
        MEData& me_data = my_picture.GetMEData();

        TwoDArray<MvCostData>* pcosts1;
        TwoDArray<MvCostData>* pcosts2;

	pcosts1 = &me_data.PredCosts(1);
	if (pparams.NumRefs()>1)
	    pcosts2 = &me_data.PredCosts(2);
	else
	    pcosts2 = pcosts1;

        float cost1, cost2, cost;
	double total_cost1 = 0.0;
	double total_cost2 = 0.0;
	double total_cost = 0.0;

	int count1=0;int count=0;

	float cost_threshold = float(olbparams.Xblen()*olbparams.Yblen()*10);

	for (int j=4; j<pcosts1->LengthY()-4; ++j){
	    for (int i=4; i<pcosts1->LengthX()-4; ++i){
	        cost1 = (*pcosts1)[j][i].SAD;
	        cost2 = (*pcosts2)[j][i].SAD;
		cost = std::min(cost1, cost2);
		total_cost1 += cost1;
		total_cost2 += cost2;
		total_cost += cost;
		if (pparams.NumRefs()>1 && cost<=cost_threshold){
		    ++count;
                    if (cost1<=cost2)
		        ++count1;
		}
	    }

	}
	total_cost1 *= olbparams.Xbsep()*olbparams.Ybsep();
	total_cost1 /= olbparams.Xblen()*olbparams.Yblen();

	total_cost2 *= olbparams.Xbsep()*olbparams.Ybsep();
	total_cost2 /= olbparams.Xblen()*olbparams.Yblen();

        if (pparams.NumRefs()>1){
	    my_picture.SetPredBias(float(count1)/float(count));
        }
	else
	    my_picture.SetPredBias(0.5);

	total_cost *= olbparams.Xbsep()*olbparams.Ybsep();
	total_cost /= olbparams.Xblen()*olbparams.Yblen();

//	my_picture.SetComplexity( total_cost );
	my_picture.SetComplexity( total_cost*total_cost );

    }

}

void PictureCompressor::CalcComplexity2( EncQueue& my_buffer, int pnum )
{
    // to be used after doing motion compensation
    EncPicture& my_picture = my_buffer.GetPicture( pnum );
    const PicArray& pic_data = my_picture.Data( Y_COMP );

    if ( (my_picture.GetStatus()&DONE_MC) != 0 ){

        float cost;
	double total_sq_cost = 0.0;
	double total_cost = 0.0;

	for (int j=0; j<pic_data.LengthY(); ++j){
	    for (int i=0; i<pic_data.LengthX(); ++i){
	        cost = float( pic_data[j][i]  );
		total_cost += cost;
		total_sq_cost += cost*cost;
	    }

	}

        total_cost /= ( pic_data.LengthX()*pic_data.LengthY() );
        total_sq_cost /= ( pic_data.LengthX()*pic_data.LengthY() );

	my_picture.SetComplexity( total_sq_cost - total_cost*total_cost );

    }

}



void PictureCompressor::NormaliseComplexity( EncQueue& my_buffer, int pnum )
{
    EncPicture& my_picture = my_buffer.GetPicture( pnum );

    if ( (my_picture.GetStatus()&DONE_PIC_COMPLEXITY) != 0 ){

         std::vector<int> queue_members = my_buffer.Members();

	 double mean_complexity = 0.0;
         int count = 0;
         for (size_t i=0; i<queue_members.size(); ++ i){
	     int n = queue_members[i];
	     EncPicture& enc_pic = my_buffer.GetPicture( n );

	     if ( (enc_pic.GetStatus()&DONE_PIC_COMPLEXITY) != 0
	           && enc_pic.GetPparams().PicSort().IsInter()
	           && n >= pnum - 10
		   && n <= pnum + 10){
	         mean_complexity += enc_pic.GetComplexity();
                 count++;
	     }

	 }
         mean_complexity /= count;
         my_picture.SetNormComplexity( my_picture.GetComplexity() / mean_complexity );

    }

}

void PictureCompressor::SubPixelME( EncQueue& my_buffer , int pnum )
{
    const std::vector<int>& refs = my_buffer.GetPicture(pnum).GetPparams().Refs();
    const int num_refs = refs.size();

    PictureParams& pparams = my_buffer.GetPicture(pnum).GetPparams();
    MEData& me_data = my_buffer.GetPicture(pnum).GetMEData();
    PicturePredParams& predparams = me_data.GetPicPredParams();

    float lambda;
    if ( pparams.IsBPicture())
        lambda = m_encparams.L2MELambda();
    else
        lambda = m_encparams.L1MELambda();

//lambda *= my_buffer.GetPicture(pnum).GetNormComplexity();

    // Set up the lambda to be used
    me_data.SetLambdaMap( num_refs , lambda );

    m_orig_prec = predparams.MVPrecision();

    // Step 2.
    // Pixel accurate vectors are then refined to sub-pixel accuracy

    if (m_orig_prec != MV_PRECISION_PIXEL)
    {
        SubpelRefine pelrefine( m_encparams );
        pelrefine.DoSubpel( my_buffer , pnum );
    }
    else
    {
        // FIXME: HACK HACK
        // Mutiplying the motion vectors by 2 and setting MV precision to
        // HALF_PIXEL to implement pixel accurate motion estimate
        MvArray &mv_arr1 = me_data.Vectors(1);
        for (int j = 0; j < mv_arr1.LengthY(); ++j)
        {
            for (int i = 0; i < mv_arr1.LengthX(); ++i)
                mv_arr1[j][i] = mv_arr1[j][i] << 1;
        }
        if (num_refs > 1)
        {
            MvArray &mv_arr2 = me_data.Vectors(2);
            for (int j = 0; j < mv_arr2.LengthY(); ++j)
            {
                for (int i = 0; i < mv_arr2.LengthX(); ++i)
                    mv_arr2[j][i] = mv_arr2[j][i] << 1;
            }
        }
        predparams.SetMVPrecision(MV_PRECISION_HALF_PIXEL);
    }

}

void PictureCompressor::ModeDecisionME( EncQueue& my_buffer, int pnum )
{
    MEData& me_data = my_buffer.GetPicture(pnum).GetMEData();
    PictureParams& pparams = my_buffer.GetPicture(pnum).GetPparams();
    PicturePredParams& predparams = me_data.GetPicPredParams();

    ModeDecider my_mode_dec( m_encparams );
    my_mode_dec.DoModeDecn( my_buffer , pnum );

    const int num_refs = pparams.NumRefs();

    if (m_orig_prec ==  MV_PRECISION_PIXEL)
    {
        // FIXME: HACK HACK
        // Divide the motion vectors by 2 to convert back to pixel
        // accurate motion vectors and reset MV precision to
        // PIXEL accuracy
        MvArray &mv_arr1 = me_data.Vectors(1);
        for (int j = 0; j < mv_arr1.LengthY(); ++j)
        {
            for (int i = 0; i < mv_arr1.LengthX(); ++i)
                mv_arr1[j][i] = mv_arr1[j][i] >> 1;
        }
        if (num_refs > 1)
        {
            MvArray &mv_arr2 = me_data.Vectors(2);
            for (int j = 0; j < mv_arr2.LengthY(); ++j)
            {
                for (int i = 0; i < mv_arr2.LengthX(); ++i)
                    mv_arr2[j][i] = mv_arr2[j][i]>>1;
            }
        }
        predparams.SetMVPrecision(MV_PRECISION_PIXEL);
    }

}

void PictureCompressor::IntraModeAnalyse( EncQueue& my_buffer, int pnum )
{
    MEData& me_data = my_buffer.GetPicture(pnum).GetMEData();

    // Count the number of intra blocks
    const TwoDArray<PredMode>& modes = me_data.Mode();

    int count_intra = 0;
    for ( int j=0 ; j<modes.LengthY() ; ++j )
    {
        for ( int i=0 ; i<modes.LengthX() ; ++i )
        {
            if ( modes[j][i] == INTRA )
                count_intra++;
        }
    }// j

    me_data.SetIntraBlockRatio(static_cast<double>( count_intra ) /
                          static_cast<double>( modes.LengthX() * modes.LengthY() ) );

}

void PictureCompressor::MotionCompensate( EncQueue& my_buffer, int pnum,
                                          AddOrSub dirn )
{
    EncPicture* my_pic = &my_buffer.GetPicture(pnum);
    std::vector<int>& my_refs = my_pic->GetPparams().Refs();
    Picture* ref_pics[2];

    ref_pics[0]=&my_buffer.GetPicture(my_refs[0]);
    if (my_refs.size()>1)
        ref_pics[1]=&my_buffer.GetPicture(my_refs[1]);
    else
        ref_pics[1]=&my_buffer.GetPicture(my_refs[0]);

    PicturePredParams& predparams = my_pic->GetMEData().GetPicPredParams();
    MotionCompensator::CompensatePicture( predparams , dirn ,
                                          my_pic->GetMEData() , my_pic, ref_pics );
}

void PictureCompressor::Prefilter( EncQueue& my_buffer, int pnum )
{
    Picture& my_picture = my_buffer.GetPicture( pnum );

    for (int c=0; c<3; ++c ){
        if ( m_encparams.Prefilter() == RECTLP )
                LPFilter( my_picture.Data( (CompSort) c) , m_encparams.Qf(),
                           m_encparams.PrefilterStrength() );

        if ( m_encparams.Prefilter() == DIAGLP )
//	    DiagFilter( my_picture.Data( (CompSort) c), 3.0, 5 );
                DiagFilter( my_picture.Data( (CompSort) c) , m_encparams.Qf(),
                           m_encparams.PrefilterStrength() );
    }

}

void PictureCompressor::DoDWT( EncQueue& my_buffer , int pnum, Direction dirn )
{
    Picture& my_picture = my_buffer.GetPicture( pnum );
    PictureParams& pparams = my_picture.GetPparams();
    const PictureSort& psort = pparams.PicSort();

    // Set the wavelet filter
    if ( psort.IsIntra() ){
        m_encparams.SetTransformFilter( m_encparams.IntraTransformFilter() );
        m_encparams.SetUsualCodeBlocks( INTRA_PICTURE );
    }
    else{
        m_encparams.SetTransformFilter( m_encparams.InterTransformFilter() );
        m_encparams.SetUsualCodeBlocks( INTER_PICTURE );
    }

    const int depth=m_encparams.TransformDepth();
    const WltFilter filter = m_encparams.TransformFilter();
    WaveletTransform wtransform( depth, filter );

    if ( dirn==FORWARD )
        my_picture.InitWltData( depth );

    for (int c=0; c<3; ++c){

        PicArray& comp_data = my_buffer.GetPicture( pnum ).Data((CompSort) c );
        CoeffArray& coeff_data = my_buffer.GetPicture( pnum ).WltData((CompSort) c );

        wtransform.Transform( dirn , comp_data, coeff_data );

    }

}

void PictureCompressor::CodeResidue( EncQueue& my_buffer ,
                                        int pnum, PictureByteIO* p_picture_byteio )
{
    EncPicture& my_picture = my_buffer.GetPicture( pnum );

    PictureParams& pparams = my_picture.GetPparams();

    if ( !m_skipped ){
        // If not skipped we continue with the coding ...
        if (m_encparams.Verbose() )
            std::cout<<std::endl<<"Using QF: "<<m_encparams.Qf();

        //Write Transform Header
        TransformByteIO *p_transform_byteio = new TransformByteIO(pparams,
                                static_cast<CodecParams&>(m_encparams));
        p_picture_byteio->SetTransformData(p_transform_byteio);
        p_transform_byteio->Output();

        /* Code component data */
        /////////////////////////

        CompCompressor my_compcoder(m_encparams , pparams );

        const int depth=m_encparams.TransformDepth();

        PicArray* comp_data[3];
        CoeffArray* coeff_data[3];
        OneDArray<unsigned int>* est_bits[3];
        float lambda[3];

        // Construction and definition of objects
        for (int c=0;c<3;++c){
            comp_data[c] = &my_picture.Data((CompSort) c );
	    coeff_data[c] = &my_picture.WltData((CompSort) c );
            est_bits[c] =  new OneDArray<unsigned int>( Range( 1, 3*depth+1 ) );
        }// c

        /* Do the wavelet transforms and select the component
         * quantisers using perceptual weighting
         */
	double cpd_scale;
	if (pparams.PicSort().IsIntra() ){
	    cpd_scale = 1.0;
	}
	else{
	    float intra_ratio = my_picture.GetMEData().IntraBlockRatio();

            cpd_scale = 5.0*intra_ratio*1.0 + (1.0-5.0*intra_ratio)*0.125;
	    cpd_scale = std::max( 0.125, std::min( 1.2, cpd_scale ) );
	}
        for (int c=0; c<3; ++c){
            lambda[c] = GetCompLambda( my_picture, (CompSort) c );

            coeff_data[c]->SetBandWeights( m_encparams , pparams, (CompSort) c, cpd_scale);

            SubbandList& bands = coeff_data[c]->BandList();
            SetupCodeBlocks( bands );
            SelectQuantisers( *(coeff_data[c]) , bands , lambda[c],
                 *est_bits[c] , m_encparams.GetCodeBlockMode(), pparams, (CompSort) c );

            p_transform_byteio->AddComponent( my_compcoder.Compress(
                *(coeff_data[c]), bands, (CompSort) c, *est_bits[c] ) );
        }

        // Destruction of objects
        for (int c=0; c<3; ++c)
            delete est_bits[c];

    }//?m_skipped

}

void PictureCompressor::CodeMVData(EncQueue& my_buffer, int pnum, PictureByteIO* pic_byteio)
{

    // Code the MV data

    EncPicture& my_picture = my_buffer.GetPicture(pnum);
    PictureParams& pparams = my_picture.GetPparams();
    MvData& mv_data = static_cast<MvData&> (my_picture.GetMEData());

    // If we're using global motion parameters, code them
    if (m_use_global){
    /*
        Code the global motion parameters
           TBC ....
    */
    }

    // If we're using block motion vectors, code them
    if ( m_use_block_mv ){
        MvDataByteIO *mv_byteio = new MvDataByteIO(pparams, mv_data.GetPicPredParams());
        pic_byteio->SetMvData(mv_byteio);

        SplitModeCodec smode_coder( mv_byteio->SplitModeData()->DataBlock(), TOTAL_MV_CTXS);
        smode_coder.Compress( mv_data );
        mv_byteio->SplitModeData()->Output();

        PredModeCodec pmode_coder( mv_byteio->PredModeData()->DataBlock(), TOTAL_MV_CTXS, pparams.NumRefs() );
        pmode_coder.Compress( mv_data );
        mv_byteio->PredModeData()->Output();

        VectorElementCodec vcoder1h( mv_byteio->MV1HorizData()->DataBlock(), 1,
                                     HORIZONTAL, TOTAL_MV_CTXS);
        vcoder1h.Compress( mv_data );
        mv_byteio->MV1HorizData()->Output();

        VectorElementCodec vcoder1v( mv_byteio->MV1VertData()->DataBlock(), 1,
                                     VERTICAL, TOTAL_MV_CTXS);
        vcoder1v.Compress( mv_data );
        mv_byteio->MV1VertData()->Output();

        if ( pparams.NumRefs()>1 )
        {
            VectorElementCodec vcoder2h( mv_byteio->MV2HorizData()->DataBlock(), 2,
                                         HORIZONTAL, TOTAL_MV_CTXS);
            vcoder2h.Compress( mv_data );
            mv_byteio->MV2HorizData()->Output();

            VectorElementCodec vcoder2v( mv_byteio->MV2VertData()->DataBlock(), 2,
                                         VERTICAL, TOTAL_MV_CTXS);
            vcoder2v.Compress( mv_data );
            mv_byteio->MV2VertData()->Output();
        }

        DCCodec ydc_coder( mv_byteio->YDCData()->DataBlock(), Y_COMP, TOTAL_MV_CTXS);
        ydc_coder.Compress( mv_data );
        mv_byteio->YDCData()->Output();

        DCCodec udc_coder( mv_byteio->UDCData()->DataBlock(), U_COMP, TOTAL_MV_CTXS);
        udc_coder.Compress( mv_data );
        mv_byteio->UDCData()->Output();

        DCCodec vdc_coder( mv_byteio->VDCData()->DataBlock(), V_COMP, TOTAL_MV_CTXS);
        vdc_coder.Compress( mv_data );
        mv_byteio->VDCData()->Output();

        mv_byteio->Output();
    }
}

float PictureCompressor::GetCompLambda( const EncPicture& my_picture,
                                      const CompSort csort )
{
    const PictureParams& pparams = my_picture.GetPparams();

    const PictureSort psort = pparams.PicSort();

    float lambda;

    if ( psort.IsIntra() ){
        if ( m_is_a_cut )
            lambda = m_encparams.L1Lambda()/8;
	else
            lambda = m_encparams.ILambda();

    }
    else{
        double log_intra_lambda = std::log10( m_encparams.ILambda() );
/*
double picture_lambda = m_encparams.L1Lambda() / my_picture.GetNormComplexity();
if (pparams.IsBPicture() )
    picture_lambda *= 1.2;

        double log_picture_lambda = std::log10( picture_lambda );
*/
///*
        double log_picture_lambda;
        if (pparams.IsBPicture() )
            log_picture_lambda= std::log10( m_encparams.L2Lambda() );
        else
            log_picture_lambda= std::log10( m_encparams.L1Lambda() );

//*/
        float intra_ratio = my_picture.GetMEData().IntraBlockRatio();

        lambda= std::pow(10.0,  3.0*intra_ratio*log_intra_lambda+
                         (1.0-3.0*intra_ratio)*log_picture_lambda );

//lambda /= my_picture.GetNormComplexity();

    }

    if (csort == U_COMP)
        lambda*= m_encparams.UFactor();
    if (csort == V_COMP)
        lambda*= m_encparams.VFactor();

    return lambda;
}

void PictureCompressor::SetupCodeBlocks( SubbandList& bands )
{
    int xregions;
    int yregions;

    for (int band_num = 1; band_num<=bands.Length() ; ++band_num){
        if (m_encparams.SpatialPartition()){
            int level = m_encparams.TransformDepth() - (band_num-1)/3;
            const CodeBlocks &cb = m_encparams.GetCodeBlocks(level);
            xregions = cb.HorizontalCodeBlocks();
            yregions = cb.VerticalCodeBlocks();
        }
        else{
               xregions = 1;
               yregions = 1;
        }

        bands( band_num ).SetNumBlocks( yregions , xregions );
    }// band_num
}

void PictureCompressor::SelectQuantisers( CoeffArray& coeff_data ,
                                       SubbandList& bands ,
                                       const float lambda,
                                       OneDArray<unsigned int>& est_bits,
                                       const CodeBlockMode cb_mode,
                                       const PictureParams& pp,
                                       const CompSort csort )
{

   // Set up the multiquantiser mode
    for ( int b=bands.Length() ; b>=1 ; --b ){
        // Set multiquants flag in the subband only if
        // a. Global m_cb_mode flag is set to QUANT_MULTIPLE in encparams
        //           and
        // b. Current subband has more than one block
        if (
            cb_mode == QUANT_MULTIPLE &&
            (bands(b).GetCodeBlocks().LengthX() > 1  ||
            bands(b).GetCodeBlocks().LengthY() > 1)
           )
            bands(b).SetUsingMultiQuants( true );
        else
            bands(b).SetUsingMultiQuants( false );
    }// b

    // Select all the quantizers
    if ( !m_encparams.Lossless() ){
        // Set quantizers for all bands.
        for ( int b=bands.Length() ; b>=1 ; --b )
            est_bits[b] = SelectMultiQuants( coeff_data , bands , b, lambda,
                                      pp, csort );
    }
    else{
        for ( int b=bands.Length() ; b>=1 ; --b ){
            bands(b).SetQuantIndex( 0 );
            est_bits[b] = 0;
            TwoDArray<CodeBlock>& blocks = bands(b).GetCodeBlocks();
            for (int j=0; j<blocks.LengthY() ;++j)
                for (int i=0; i<blocks.LengthX() ;++i)
                    blocks[j][i].SetQuantIndex( 0 );
        }// b
    }
}

int PictureCompressor::SelectMultiQuants( CoeffArray& coeff_data , SubbandList& bands , 
    const int band_num , const float lambda, const PictureParams& pp, const CompSort csort)
{
    Subband& node( bands( band_num ) );

    // Now select the quantisers //
    ///////////////////////////////

    QuantChooser qchooser( coeff_data , lambda );

    // For the DC band in I pictures, remove the average
    if ( band_num == bands.Length() && pp.PicSort().IsIntra() )
        AddSubAverage( coeff_data , node.Xl() , node.Yl() , SUBTRACT);

    // The total estimated bits for the subband
    int band_bits( 0 );
    qchooser.SetEntropyCorrection( m_encparams.EntropyFactors().Factor( band_num, pp, csort ) );
    band_bits = qchooser.GetBestQuant( node );

    // Put the DC band average back in if necessary
    if ( band_num == bands.Length() && pp.PicSort().IsIntra() )
        AddSubAverage( coeff_data , node.Xl() , node.Yl() , ADD);

    if ( band_bits == 0 )
        node.SetSkip( true );
    else
        node.SetSkip( false );

    return band_bits;
}


void PictureCompressor::AddSubAverage( CoeffArray& coeff_data, int xl, int yl ,
                                    AddOrSub dirn)
{

    ValueType last_val=0;
    ValueType last_val2;

    if ( dirn == SUBTRACT )
    {
        for ( int j=0 ; j<yl ; j++)
            {
            for ( int i=0 ; i<xl ; i++)
                {
                last_val2 = coeff_data[j][i];
                coeff_data[j][i] -= last_val;
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
                coeff_data[j][i] += last_val;
                last_val = coeff_data[j][i];
            }// i
        }// j

    }
}
