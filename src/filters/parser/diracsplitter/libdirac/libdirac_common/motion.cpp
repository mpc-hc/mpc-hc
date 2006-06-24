/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: motion.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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
* Contributor(s): Thomas Davies (Original Author), Chris Bowley
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

MvData::MvData( const int xnumMB, const int ynumMB , 
                const int xnumblocks, const int ynumblocks , const int num_refs ):
    m_vectors( Range(1 , num_refs) ),
    m_gm_vectors( Range(1 , num_refs) ),
    m_modes( ynumblocks , xnumblocks ),
    m_dc( 3 ),
    m_mb_split( ynumMB , xnumMB ),
    m_mb_common( ynumMB , xnumMB ),
    m_gm_params( Range(1 , num_refs) )
{

    InitMvData();
}

MvData::MvData( const int xnumMB , const int ynumMB , const int num_refs ):
    m_vectors( Range(1 , num_refs) ),
    m_gm_vectors( Range(1 , num_refs) ),
    m_modes( 4*ynumMB , 4*xnumMB ),
    m_dc( 3 ),
    m_mb_split( ynumMB , xnumMB ),
    m_mb_common( ynumMB , xnumMB ),
    m_gm_params( Range(1 , num_refs) )
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
         m_dc[i] = new TwoDArray<ValueType>( Mode().LengthY() , Mode().LengthX() );
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



MEData::MEData(const int xnumMB , const int ynumMB ,
                const int xnumblocks , const int ynumblocks , const int num_refs ):
     MvData( xnumMB , ynumMB , xnumblocks , ynumblocks , num_refs ),
     m_pred_costs( Range( 1 , num_refs ) ),
     m_intra_costs( ynumblocks , xnumblocks ),
     m_bipred_costs( ynumblocks , xnumblocks ),
     m_MB_costs( ynumMB , xnumMB ),
     m_lambda_map( ynumblocks , xnumblocks ),
     m_inliers( Range( 1 , num_refs ) )
{
    InitMEData();
}

MEData::MEData( const int xnumMB , const int ynumMB ,  const int num_refs ):
     MvData( xnumMB , ynumMB , num_refs ),
     m_pred_costs( Range( 1 , num_refs ) ),
     m_intra_costs( 4*ynumMB , 4*xnumMB ),
     m_bipred_costs( 4*ynumMB , 4*xnumMB ),
     m_MB_costs( ynumMB , xnumMB ),
     m_lambda_map( 4*ynumMB , 4*xnumMB ),
     m_inliers( Range( 1 , num_refs ) )
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

    if ( num_refs > 1 )
    {
        FindTransitions( transition_map2 , 2 );

        for ( int j=0 ; j<m_lambda_map.LengthY() ; j++)
        {
            for ( int i=0 ; i<m_lambda_map.LengthX() ; i++)
            {
                if ( transition_map1[j][i] || transition_map2[j][i] )
                    m_lambda_map[j][i] = 0.0;
                else
                    m_lambda_map[j][i] = lambda;
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

    // Compute mean and standard deviation of local motion vector variance //
    /////////////////////////////////////////////////////////////////////////

    long double total_cost = 0.0;
    long double mean_cost;
 
    // first, mean
    for ( int j=0 ; j<mv_array.LengthY() ; ++j )
        for ( int i=0 ; i<mv_array.LengthX() ; ++i )
            total_cost += PredCosts( ref_num )[j][i].SAD;

    mean_cost = total_cost / 
                   static_cast<long double>( mv_array.LengthX()*mv_array.LengthY() );

    // next , Standard Deviation
    long double sd_cost = 0.0;
    double diff;
    
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

    float threshold = static_cast<float>( mean_cost + 1.5*sd_cost );

    // now go through and mark those that go above the threshold
    for ( int j=0 ; j<mv_array.LengthY() ; ++j )
        for ( int i=0 ; i<mv_array.LengthX() ; ++i )
            trans_map[j][i] = ( PredCosts( ref_num )[j][i].SAD >= threshold )? true : false;

//
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

    threshold = static_cast<float>( mean_cost + 1.5*sd_cost );

    // now go through and mark those that go above the threshold
    for ( int j=0 ; j<mv_array.LengthY() ; ++j )
        for ( int i=0 ; i<mv_array.LengthX() ; ++i )
//            trans_map[j][i] = ( val_array[j][i] >= threshold )? true : false;
trans_map[j][i] = false;

//     bool contains_trans;

//     for ( int j=0 ; j<mv_array.LengthY()/4 ; ++j )
//     {
//         for ( int i=0 ; i<mv_array.LengthX()/4 ; ++i )
//         {     
//             contains_trans = false;
//             for ( int q=4*j ; q<4*(j+1) ; ++q )
//             {
//                 for ( int p=4*i ; p<4*(i+1) ; ++p )
//                 {
//                     if (trans_map[q][p])
//                         contains_trans = true;
//                 }// p
//             }// q
//             for ( int q=4*j ; q<4*(j+1) ; ++q )
//                 for ( int p=4*i ; p<4*(i+1) ; ++p )
//                     trans_map[q][p] = contains_trans;

//         }// i
//     }// j

     
}


MEData::~MEData()
{
    // Delete the arrays of prediction costs
     for ( int i=m_pred_costs.First() ; i<=m_pred_costs.Last() ; ++i )
         delete m_pred_costs[i];

     for ( int i=m_inliers.First() ; i<=m_inliers.Last() ; ++i )
        delete m_inliers[i];
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
    stream >> me_data.MBSplit();
    stream >> me_data.MBCommonMode();
    stream >> me_data.MBCosts();
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
    stream << endl << endl << me_data.MBSplit();
    stream << endl << me_data.MBCommonMode();
    stream << endl << me_data.MBCosts();
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

} // namespace dirac
