/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: motion.cpp,v 1.28 2008/10/01 01:26:47 asuraparaju Exp $ $Name:  $
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
*                 Chris Bowley,
*                 Tim Borer
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


////////////////////////////////////////////////////////////////
//classes and functions for motion estimation and compensation//
////////////////////////////////////////////////////////////////

#include <libdirac_common/motion.h>
using namespace dirac;

#include <cmath>

using namespace std;

//Motion vector and Motion Estimation structures//
//////////////////////////////////////////////////

MvData::MvData( const PicturePredParams& predparams , const int num_refs ):
    m_predparams( predparams ),
    m_vectors( Range(1 , num_refs) ),
    m_gm_vectors( Range(1 , num_refs) ),
    m_modes( predparams.YNumBlocks() , predparams.XNumBlocks() ),
    m_dc( 3 ),
    m_sb_split( predparams.YNumSB() , predparams.XNumSB() ),
    m_gm_params( Range(1 , num_refs) )//,
//    m_num_refs(num_refs)
{
    InitMvData();
}

void MvData::InitMvData()
{
    // Create the arrays of vectors
     for ( int i=m_vectors.First() ; i<=m_vectors.Last() ; ++i ){
         m_vectors[i] = new MvArray( Mode().LengthY() , Mode().LengthX() );
         m_gm_vectors[i] = new MvArray( Mode().LengthY() , Mode().LengthX() );
     }

    // create global motion parameter arrays
    for ( int i=m_gm_params.First() ; i<=m_gm_params.Last() ; ++i ){
         m_gm_params[i] = new OneDArray<float> ( 8 );
    }

     // Create the arrays of dc values
     for ( int i=0 ; i<3 ; ++i )
         m_dc[i] = new TwoDArray<ValueType>( Mode().LengthY() , Mode().LengthX() , 0);
}

MvData::~MvData()
{
   // Delete the arrays of vectors
    for ( int i=m_vectors.First() ; i<=m_vectors.Last() ; ++i ){
        delete m_vectors[i];
        delete m_gm_vectors[i];
    }

     // delete array of global motion parameters
     for ( int i=m_gm_params.First() ; i<=m_gm_params.Last() ; ++i ){
         delete m_gm_params[i];
     }

     // Delete the arrays of dc values
     for ( int i=0 ; i<3 ; ++i )
         delete m_dc[i];
}

MEData::MEData(const PicturePredParams& predparams , const int num_refs ):
     MvData( predparams , num_refs ),
     m_pred_costs( Range( 1 , num_refs ) ),
     m_intra_costs( predparams.YNumBlocks() , predparams.XNumBlocks(), 0 ),
     m_bipred_costs( predparams.YNumBlocks() , predparams.XNumBlocks() ),
     m_SB_costs( predparams.YNumSB() , predparams.XNumSB() ),
     m_lambda_map( predparams.YNumBlocks() , predparams.XNumBlocks() ),
     m_inliers( Range( 1 , num_refs ) ),
     m_intra_block_ratio(0.0)
{
    InitMEData();
}

void MEData::InitMEData()
{
   // Create the arrays of prediction costs
    for ( int i=m_pred_costs.First() ; i<=m_pred_costs.Last() ; ++i )
        m_pred_costs[i] = new TwoDArray<MvCostData>( Mode().LengthY() , Mode().LengthX() );

    // Create the arrays of vectors
     for ( int i=m_inliers.First() ; i<=m_inliers.Last() ; ++i )
         m_inliers[i] = new TwoDArray<int>( Mode().LengthY() , Mode().LengthX() );
}

void MEData::SetLambdaMap( const int num_refs , const float lambda )
{
    TwoDArray<bool> transition_map1( Mode().LengthY() , Mode().LengthX() );
    TwoDArray<bool> transition_map2( Mode().LengthY() , Mode().LengthX() );

    FindTransitions( transition_map1 , 1 );

    if ( num_refs==1 )
    {
        for ( int j=0 ; j<m_lambda_map.LengthY() ; j++)
        {
            for ( int i=0 ; i<m_lambda_map.LengthX() ; i++)
            {
                if ( transition_map1[j][i] )
                    m_lambda_map[j][i] = 0.0;
                else
                    m_lambda_map[j][i] = lambda;
                if ( i<4 || j<4 )
                    m_lambda_map[j][i] /= 5.0; 
            }// i
        }// j
    }
    else if ( num_refs > 1 )
    {
        FindTransitions( transition_map2 , 2 );

        for ( int j=0 ; j<m_lambda_map.LengthY() ; j++)
        {
            for ( int i=0 ; i<m_lambda_map.LengthX() ; i++)
            {
                if ( transition_map1[j][i] && transition_map2[j][i] )
                    m_lambda_map[j][i] = 0.0;
                else if (transition_map1[j][i] || transition_map2[j][i] )
                    m_lambda_map[j][i] = lambda/4.0;
                else
                    m_lambda_map[j][i] = lambda;

                if ( i<4 || j<4 )
                    m_lambda_map[j][i] /= 5.0; 
            }// i
        }// j
    }

}

void MEData::SetLambdaMap( const int level , const TwoDArray<float>& l_map , const float wt )
{

    const int factor = 1<<(2-level);
    int xstart , xend , ystart , yend;
 
    for (int j = 0 ; j<m_lambda_map.LengthY() ; ++j )
    {
        for (int i = 0 ; i<m_lambda_map.LengthX() ; ++i )
        {
            xstart = factor * i;
            ystart = factor * j;
            xend = factor * ( i + 1 );
            yend = factor * ( j + 1 );

            m_lambda_map[j][i] = l_map[ystart][xstart];

            for (int q = ystart ; q<yend ; ++q )
                for (int p = xstart ; p<xend ; ++p )
                      m_lambda_map[j][i] = std::max( l_map[q][p] , m_lambda_map[j][i] );

           m_lambda_map[j][i] *= wt;

        }// i
    }// j

}

void MEData::FindTransitions( TwoDArray<bool>& trans_map , const int ref_num )
{

    const MvArray& mv_array = Vectors( ref_num );

    // Start with a statistical approach - determine thresholds later

    // Compute mean and standard deviation of local SAD variance //
    ///////////////////////////////////////////////////////////////

    long double total_cost = 0.0;
    long double mean_cost;
    long double sd_cost = 0.0;
    double diff;
    double threshold;

    // first, mean
    for ( int j=0 ; j<mv_array.LengthY() ; ++j )
        for ( int i=0 ; i<mv_array.LengthX() ; ++i )
            total_cost += PredCosts( ref_num )[j][i].SAD;

    mean_cost = total_cost / 
                   static_cast<long double>( mv_array.LengthX()*mv_array.LengthY() );

    // next , Standard Deviation
    
    for ( int j=0 ; j<mv_array.LengthY() ; ++j )
    {
        for ( int i=0 ; i<mv_array.LengthX() ; ++i )
        {
            diff = PredCosts( ref_num )[j][i].SAD - mean_cost;
            diff *= diff;
            sd_cost += diff;

        }// i
    }// j

    // Get the variance ...
    sd_cost /= static_cast<long double>( mv_array.LengthX()*mv_array.LengthY() );

    // ... and then the SD
    sd_cost = std::sqrt( sd_cost );

    threshold = mean_cost + 3*sd_cost;

    // now go through and mark those that go above the threshold
    for ( int j=0 ; j<mv_array.LengthY() ; ++j )
        for ( int i=0 ; i<mv_array.LengthX() ; ++i )
            trans_map[j][i] = ( PredCosts( ref_num )[j][i].SAD >= threshold )? true : false;


    // Next look at motion-vector costs
    TwoDArray<double> val_array( mv_array.LengthY() , mv_array.LengthX() );

    // first, mean
    total_cost = 0.0;
    for ( int i=0 ; i<mv_array.LengthX() ; ++i )
    {
        val_array[0][i] = 0.0;
        val_array[val_array.LastY()][i] = 0.0;
    }// i

    for ( int j=1 ; j<mv_array.LengthY()-1 ; ++j )
    {
        val_array[j][0] = 0.0;
        val_array[j][val_array.LastX()] = 0.0;
        for ( int i=1 ; i<mv_array.LengthX()-1 ; ++i )
        {
            val_array[j][i] =0.0;
            for (int q=-1 ; q<=1 ; ++q)
                for (int p=-1 ; p<=1 ; ++p)
                    val_array[j][i] = std::max( val_array[j][i] , (double)Norm1( mv_array[j+q][i+p] - mv_array[j][i] ) );

            total_cost += val_array[j][i];

        }// i
    }// j


    mean_cost = total_cost / 
                   static_cast<long double>( mv_array.LengthX()*mv_array.LengthY() );

    // next , Standard Deviation
    sd_cost = 0.0;
    
    for ( int j=1 ; j<mv_array.LengthY()-1 ; ++j )
    {
        for ( int i=1 ; i<mv_array.LengthX()-1 ; ++i )
        {
            diff = val_array[j][i] - mean_cost;
            diff *= diff;

            sd_cost += diff;

        }// i
    }// j

    // Get the variance ...
    sd_cost /= static_cast<long double>( mv_array.LengthX()*mv_array.LengthY() );

    // ... and then the SD
    sd_cost = std::sqrt( sd_cost );

    threshold = mean_cost + 3*sd_cost;

    // now go through and mark those that go above the threshold
    for ( int j=0 ; j<mv_array.LengthY() ; ++j )
        for ( int i=0 ; i<mv_array.LengthX() ; ++i )
            trans_map[j][i] = ( val_array[j][i] >= threshold )? true : false;

     bool contains_trans;

     for ( int j=0 ; j<mv_array.LengthY()/4 ; ++j )
     {
         for ( int i=0 ; i<mv_array.LengthX()/4 ; ++i )
         {     
             contains_trans = false;
             for ( int q=4*j ; q<4*(j+1) ; ++q )
             {
                 for ( int p=4*i ; p<4*(i+1) ; ++p )
                 {
                     if (trans_map[q][p])
                         contains_trans = true;
                 }// p
             }// q
             for ( int q=4*j ; q<4*(j+1) ; ++q )
                 for ( int p=4*i ; p<4*(i+1) ; ++p )
                     trans_map[q][p] = contains_trans;

         }// i
     }// j

}


MEData::~MEData()
{
    // Delete the arrays of prediction costs
     for ( int i=m_pred_costs.First() ; i<=m_pred_costs.Last() ; ++i )
         delete m_pred_costs[i];

     for ( int i=m_inliers.First() ; i<=m_inliers.Last() ; ++i )
        delete m_inliers[i];
}

void MEData::DropRef( const int rindex ){

    if (rindex==2){}
    else if (rindex==1){
       // Swap data for reference 1 and reference 2
       // so that reference 2 becomes the new reference 1

       MvArray* ptr = m_vectors[1];
       m_vectors[1] = m_vectors[2];
       m_vectors[2] = ptr;

       ptr = m_gm_vectors[1];
       m_gm_vectors[1] = m_gm_vectors[2];
       m_gm_vectors[2] = ptr;

       OneDArray<float>* ptr2 = m_gm_params[1];
       m_gm_params[1] = m_gm_params[2];
       m_gm_params[2] = ptr2;

       TwoDArray<MvCostData>* ptr3 = m_pred_costs[1];
       m_pred_costs[1] = m_pred_costs[2];
       m_pred_costs[2] = ptr3;

       TwoDArray<int>* ptr4 = m_inliers[1];
       m_inliers[1] = m_inliers[2];
       m_inliers[2] = ptr4;
    }
}

namespace dirac
{
//! Overloaded operator<< for MvCostData
/*!
    Only writes SAD value to stream
*/
ostream & operator<< (ostream & stream, MvCostData & cost)
{
    stream << cost.SAD << " " << cost.mvcost;

    return stream;
}

//! Overloaded operator>> for MvCostData
/*!
    Only reads SAD value from stream
*/
istream & operator>> (istream & stream, MvCostData & cost)
{
    stream >> cost.SAD >> cost.mvcost;

    return stream;
}

//! Overloaded operator>> for PredMode
/*!
    No operator<< is specified as enumeration is written as integers
    operator>> required to specify PredMode input
*/
istream & operator>> (istream & stream, PredMode & mode)
{
    int temp;
    stream >> temp;
    mode = (PredMode)temp;

    return stream;
}

// Overriden extractor operator for reading MvData data members
istream &operator>> (istream & stream, MEData & me_data)
{
    stream.ignore(1000, '\n');
    
    // input reference-independent information
    stream >> me_data.SBSplit();
    stream >> me_data.SBCosts();
    stream >> me_data.Mode();
    stream >> me_data.IntraCosts();

    if (me_data.m_pred_costs.Length() > 1)
        stream >> me_data.BiPredCosts();

    if (me_data.DC().Length() == 1)
    {
        stream >> me_data.DC( Y_COMP );
    }
    else if (me_data.DC().Length() == 3)
    {
        stream >> me_data.DC( Y_COMP );
        stream >> me_data.DC( U_COMP );
        stream >> me_data.DC( V_COMP );
    }

    // input reference information
    for (int i=1; i<=me_data.m_pred_costs.Length(); ++i)
    {
        stream >> me_data.Vectors(i);
        stream >> me_data.PredCosts(i);
        //stream >> me_data.GlobalMotionParameters(i);
        //stream >> me_data.GlobalMotionVectors(i);
        //stream >> me_data.GlobalMotionInliers(i);
    }

    return stream;
}

// Overriden operator for output of MvData member data (to file)
ostream &operator<< (ostream & stream, MEData & me_data)
{
    // output reference-independent information
    stream << endl << endl << me_data.SBSplit();
    stream << endl << me_data.SBCosts();
    stream << endl << me_data.Mode();
    stream << endl << me_data.IntraCosts() << endl;

    if (me_data.m_pred_costs.Length() > 1)
        stream << me_data.BiPredCosts();

    // output component DC values
    if (me_data.DC().Length() == 1)
    {
        stream << endl << me_data.DC( Y_COMP );
    }
    else if (me_data.DC().Length() == 3)
    {
        stream << endl << me_data.DC( Y_COMP );
        stream << endl << me_data.DC( U_COMP );
        stream << endl << me_data.DC( V_COMP );
    }

    // output reference information
    for (int i=1; i<=me_data.m_pred_costs.Length(); ++i)
    {
        stream << endl << me_data.Vectors(i);
        stream << endl << me_data.PredCosts(i) << endl;
        //stream << endl << me_data.GlobalMotionParameters(i) << endl;
        //stream << endl << me_data.GlobalMotionVectors(i) << endl;
        //stream << endl << me_data.GlobalMotionInliers(i) << endl;
    }
    
    return stream;
}

int Median( const int val1, const int val2, const int val3)
{
    int tmp;

    tmp=val1;
    tmp+=val2;
    tmp+=val3;

    tmp -= std::max( std::max( val1 , val2 ) , val3 );
    tmp -= std::min( std::min( val1 , val2 ) , val3 );
    
    return tmp;
}

MVector MvMedian(const MVector& mv1,const MVector& mv2,const MVector& mv3) {
    //takes median of each vector component    
    MVector tmp_mv;

    tmp_mv.x=mv1.x;
    tmp_mv.x+=mv2.x;
    tmp_mv.x+=mv3.x;

    tmp_mv.x-=std::max(std::max(mv1.x,mv2.x),mv3.x);
    tmp_mv.x-=std::min(std::min(mv1.x,mv2.x),mv3.x);

    tmp_mv.y=mv1.y;
    tmp_mv.y+=mv2.y;
    tmp_mv.y+=mv3.y;

    tmp_mv.y-=std::max(std::max(mv1.y,mv2.y),mv3.y);
    tmp_mv.y-=std::min(std::min(mv1.y,mv2.y),mv3.y);

    return tmp_mv;
}

int Median(const std::vector<int>& val_list)
{   
    // take the median of up to 4 elements

    switch (val_list.size() )
    {
        case 1 :

            return val_list[0];
        case 2 : // return the mean
            return ( ( val_list[0] + val_list[1] + 1 )>>1 );
        case 3 :
            return Median(val_list[0], val_list[1], val_list[2] );
        case 4 :
            {
            int med_val(0);
            int max_val(val_list[0]);
            int min_val(val_list[0]);

            for (int i=0; i<4; ++i )
            {
                med_val += val_list[i];
                max_val = std::max( max_val , val_list[i] );
                min_val = std::min( min_val , val_list[i] );

            }// i
         
            med_val -= ( max_val + min_val );
         
            return ( (med_val + 1)>>1 );
            }
        default :
            return 0;
    }

}

MVector MvMedian(const std::vector<MVector>& vect_list){
    //median of 0-4 vectors
    
    if ( vect_list.size() == 0 )
        return 0;
    else if ( vect_list.size() == 1 )
        return vect_list[0];
    else if ( vect_list.size() == 2 )
        return MvMean( vect_list[0], vect_list[1] );
    else if ( vect_list.size() == 3 )
        return MvMedian(vect_list[0], vect_list[1], vect_list[2] );
    else if ( vect_list.size() == 4 )
    {
         MVector tmp_mv(0);
         MVector max_mv(vect_list[0]);
         MVector min_mv(vect_list[0]);
         for (int i=0; i<4; ++i )
         {
             tmp_mv.x += vect_list[i].x;
             max_mv.x=std::max(max_mv.x, vect_list[i].x);
             min_mv.x=std::min(min_mv.x, vect_list[i].x);

             tmp_mv.y += vect_list[i].y;
             max_mv.y=std::max(max_mv.y, vect_list[i].y);
             min_mv.y=std::min(min_mv.y, vect_list[i].y);            

         }// i
         
         tmp_mv.x -= (max_mv.x+min_mv.x);
         tmp_mv.y -= (max_mv.y+min_mv.y);
         
         tmp_mv.x = (tmp_mv.x+1)>>1;
         tmp_mv.y = (tmp_mv.y+1)>>1;

         return tmp_mv;    
             
    }
    else
    {        
        MVector median;
        int num_vals=int(vect_list.size());
        if (num_vals>0)    {
            int pos=0;
            std::vector<int> ordered_vals(vect_list.size());
            //do x first
            ordered_vals[0]=vect_list[0].x;        
            for (int I=1;I<num_vals;++I){
                for (int K=0;K<I;++K){
                    if (vect_list[I].x<ordered_vals[K]){
                        pos=K;
                        break;
                    }
                    else
                        pos=K+1;
                }//K
                if (pos==I)
                    ordered_vals[I]=vect_list[I].x;
                else{
                    for (int K=I-1;K>=pos;--K){
                        ordered_vals[K+1]=ordered_vals[K];
                    }
                    ordered_vals[pos]=vect_list[I].x;
                }
            }//I
            if (vect_list.size()%2!=0)
                median.x=ordered_vals[(num_vals-1)/2];
            else
                median.x=(ordered_vals[(num_vals/2)-1]+ordered_vals[num_vals/2]+1)>>1;

            //now do y
            ordered_vals[0]=vect_list[0].y;        
            for (int I=1;I<num_vals;++I){
                for (int K=0;K<I;++K){
                    if (vect_list[I].y<ordered_vals[K]){
                        pos=K;
                        break;
                    }
                    else
                        pos=K+1;
                }//K
                if (pos==I)
                    ordered_vals[I]=vect_list[I].y;
                else{
                    for (int K=I-1;K>=pos;--K){
                        ordered_vals[K+1]=ordered_vals[K];
                    }
                    ordered_vals[pos]=vect_list[I].y;
                }
            }//I
            if (num_vals%2!=0)
                median.y=ordered_vals[(num_vals-1)/2];
            else
                median.y=(ordered_vals[(num_vals/2)-1]+ordered_vals[num_vals/2]+1)>>1;        

        }
        else{
            median.x=0;
            median.y=0;
        }

        return median;
    }

}


//! Return the unbiased mean of two motion vectors
MVector MvMean(const MVector& mv1, const MVector& mv2)
{
    //takes mean of each vector component    
    MVector tmp_mv;

    tmp_mv.x = mv1.x;
    tmp_mv.x += mv2.x+1;
    tmp_mv.x >>= 1;

    tmp_mv.y = mv1.y;
    tmp_mv.y += mv2.y+1;
    tmp_mv.y >>= 1;

    return tmp_mv;
}

//! Return the mean of a set of unsigned integer values
unsigned int GetUMean(std::vector<unsigned int>& values)
{
    unsigned int sum=0;
    for (unsigned int I=0;I<values.size();++I)
        sum+=values[I];

    sum+=(values.size()>>1);
    sum/=values.size();

    return sum;
}

//! Return the mean of a set of signed integer values
int GetSMean(std::vector<int>& values)
{
    if (values.size()==0)
        return 0;

    int sum=0;
    for (unsigned int i=0;i<values.size();++i)
        sum+=values[i];
    if ( sum>=0 )
    {
        sum+=(values.size()>>1);
        sum/=values.size();
    }
    else
    {
        int old_sum = sum;
        sum -= values.size()*old_sum;
        sum+=(values.size()>>1);
        sum/=values.size();
        sum += old_sum;
    }

    return sum;
}

} // namespace dirac
