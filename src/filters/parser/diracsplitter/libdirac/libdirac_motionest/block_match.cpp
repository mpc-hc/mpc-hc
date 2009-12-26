/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: block_match.cpp,v 1.21 2008/10/29 02:46:22 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Thomas Davies (Original Author)
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

#include <libdirac_motionest/block_match.h>
#include <libdirac_motionest/me_utils.h>
using namespace dirac;

#include <cmath>
using std::vector;

namespace dirac
{

void AddNewVlist( CandidateList& vect_list, const MVector& mv, 
                  const int xr , const int yr , const int step )
{
      //Creates a new motion vector list in a square region around mv

    vector<MVector> tmp_list;
    vect_list.push_back(tmp_list);
    int list_num=vect_list.size()-1;    

    MVector tmp_mv( mv );
    AddVect(vect_list , tmp_mv , list_num );

    for ( int i=1 ; i<=xr ; ++i )
    {
        tmp_mv.x = mv.x + i*step;
        AddVect( vect_list , tmp_mv , list_num );

        tmp_mv.x = mv.x - i*step;        
        AddVect( vect_list , tmp_mv , list_num );    
    }

    for ( int j=1 ; j<=yr ; ++j)
    {
        for ( int i=-xr ; i<=xr ; ++i)
        {
            tmp_mv.x = mv.x + i*step;
            tmp_mv.y = mv.y + j*step;
            AddVect(vect_list,tmp_mv,list_num);

            tmp_mv.y = mv.y -j*step;
            AddVect(vect_list,tmp_mv,list_num);            

        }// i        
    }// j

    // If we've not managed to add any element to the list
    // remove the list so we don't ever have to check its size
    if ( vect_list[list_num].size() == 0 )
        vect_list.erase( vect_list.begin() + list_num );
}

void AddNewVlist( CandidateList& vect_list , const MVector& mv , const int xr , const int yr)
{
      // Creates a new motion vector list in a square region around mv
    
    vector<MVector> tmp_list;
    vect_list.push_back(tmp_list);
    int list_num=vect_list.size()-1;    

    MVector tmp_mv(mv);
    AddVect(vect_list,tmp_mv,list_num);

    for ( int i=1 ; i<=xr ; ++i)
    {
        tmp_mv.x = mv.x + i;
        AddVect( vect_list , tmp_mv , list_num );

        tmp_mv.x = mv.x - i;        
        AddVect( vect_list , tmp_mv , list_num );    
    }

    for ( int j=1 ; j<=yr ; ++j)
    {
        for ( int i=-xr ; i<=xr ; ++i)
        {
            tmp_mv.x = mv.x + i;
            tmp_mv.y = mv.y + j;
            AddVect( vect_list , tmp_mv , list_num );

            tmp_mv.y = mv.y-j;
            AddVect( vect_list , tmp_mv , list_num );            
        }        
    }

    // If we've not managed to add any element to the list
    // remove the list so we don't ever have to check its size
    if ( vect_list[list_num].size() == 0 )                 
        vect_list.erase( vect_list.begin() + list_num );
}

void AddNewVlistD( CandidateList& vect_list , const MVector& mv , const int xr , const int yr )
{
      //As above, but using a diamond pattern

    vector<MVector> tmp_list;
    vect_list.push_back( tmp_list );

    int list_num=vect_list.size()-1;    
    int xlim;

    MVector tmp_mv( mv );
    AddVect( vect_list , tmp_mv , list_num );

    for ( int i=1 ; i<=xr ; ++i)
    {
        tmp_mv.x = mv.x + i;
        AddVect( vect_list , tmp_mv , list_num );

        tmp_mv.x = mv.x - i;        
        AddVect( vect_list , tmp_mv , list_num );    
    }

    for ( int j=1 ; j<=yr ; ++j)
    {
        xlim = xr * (yr-std::abs(j)) / yr;        
        for ( int i=-xlim ; i<=xlim ; ++i)
        {
            tmp_mv.x = mv.x + i;
            tmp_mv.y = mv.y + j;
            AddVect( vect_list , tmp_mv , list_num );

            tmp_mv.y = mv.y - j;
            AddVect( vect_list , tmp_mv , list_num );            
        }        
    }

    // If we've not managed to add any element to the list
    // remove the list so we don't ever have to check its size
    if ( vect_list[list_num].size() == 0 )                
        vect_list.erase( vect_list.begin() + list_num );
}

void AddVect(CandidateList& vect_list,const MVector& mv,int list_num)
{

    bool is_in_list=false;
    
    size_t lnum=0;
    size_t i;    

    while( !is_in_list && lnum<vect_list.size() )
    {
        i=0;        
        while( !is_in_list && i<vect_list[lnum].size())
        {        
            if ( vect_list[lnum][i].x == mv.x && vect_list[lnum][i].y == mv.y )    
                is_in_list=true;        
            ++i;    
        }
        ++lnum;
    }

    if ( !is_in_list )
        vect_list[list_num].push_back(mv);
    
}

BlockMatcher::BlockMatcher( const PicArray& pic_data , 
                            const PicArray& ref_data , 
                            const OLBParams& bparams ,
                            const int precision , 
                            const MvArray& mv_array , 
                            const TwoDArray< MvCostData >& cost_array):
    m_pic_data(pic_data), 
    m_ref_data(ref_data),
    m_mv_array(mv_array),
    m_cost_array(cost_array),
    m_peldiff( ref_data , pic_data ), //NB: ORDER!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    m_subpeldiff( 3 ),
    m_bparams( bparams ),
    m_var_max( (pic_data.LengthX()+pic_data.LengthY() )/216 ),
    m_var_max_up( (pic_data.LengthX()+pic_data.LengthY() )/27 ),
    m_precision( precision )
{
    m_subpeldiff[0] = new BlockDiffHalfPel( ref_data, pic_data ); 
    m_subpeldiff[1] = new BlockDiffQuarterPel( ref_data, pic_data );
    m_subpeldiff[2] = new BlockDiffEighthPel( ref_data, pic_data );
}

BlockMatcher::~BlockMatcher()
{
    for (int i=0; i<3; ++i )
        delete m_subpeldiff[i];
}


ValueType BlockMatcher::GetVar( const MVector& predmv , const MVector& mv ) const
{
    MVector diff;
    diff.x = mv.x-predmv.x;
    diff.y = mv.y-predmv.y;    

    return Norm1( diff );
}
 
ValueType BlockMatcher::GetVarUp( const MVector& predmv , const MVector& mv ) const
{
    MVector diff;
    diff.x = mv.x-predmv.x;
    diff.y = mv.y-predmv.y;   

    return std::min( Norm1( diff ) , Norm1( mv ) );
}   

void BlockMatcher::FindBestMatchPel(const int xpos , const int ypos ,
                                    const CandidateList& cand_list,
                                    const MVector& mv_prediction,
                                    const int list_start)
{
    BlockDiffParams dparams;
    dparams.SetBlockLimits( m_bparams , m_pic_data , xpos , ypos);
    
    //now test against the offsets in the MV list to get the lowest cost//
    //////////////////////////////////////////////////////////////////////     

    float best_cost = m_cost_array[ypos][xpos].total;

    MVector best_mv = m_mv_array[ypos][xpos];

    for ( size_t lnum=list_start ; lnum<cand_list.size() ; ++lnum)
    {
        for (size_t i=0 ; i<cand_list[lnum].size() ; ++i)
        {
            m_peldiff.Diff( dparams , 
                            cand_list[lnum][i] , 
                            best_cost , 
                            best_mv);
        }// i
    }// num

    // Write the results in the arrays //
    /////////////////////////////////////

    m_mv_array[ypos][xpos] = best_mv;
    m_cost_array[ypos][xpos].SAD = best_cost;
    m_cost_array[ypos][xpos].mvcost = GetVar( mv_prediction , best_mv);
    m_cost_array[ypos][xpos].SetTotal( 0.0 );
}

void BlockMatcher::FindBestMatchSubp( const int xpos, const int ypos,
                                      const CandidateList& cand_list,
                                      const MVector& mv_prediction,
                                      const float lambda)
{

    BlockDiffParams dparams;
    dparams.SetBlockLimits( m_bparams , m_pic_data , xpos , ypos);    

    //now test against the offsets in the MV list to get the lowest cost//
    //////////////////////////////////////////////////////////////////////    

    // Numbers of the lists to do more searching in
    vector<int> list_nums; 

    // Costs of the initial vectors in each list
    OneDArray<float> list_costs( cand_list.size() );

    // First test the first in each of the lists to choose which lists to pursue
    MvCostData best_costs( m_cost_array[ypos][xpos] );
    best_costs.total = 100000000.0f;
    MVector best_mv( m_mv_array[ypos][xpos] );

    MvCostData cand_costs;
    MVector cand_mv;

    for (size_t list_num=0 ; list_num<cand_list.size() ; ++list_num )
    {
        for (size_t i=0 ; i<cand_list[list_num].size() ; ++i )
        {
            cand_mv = cand_list[list_num][i];
            cand_costs.mvcost = GetVarUp( mv_prediction , cand_mv );

            m_subpeldiff[m_precision-1]->Diff( dparams,
                                               cand_mv ,
                                               cand_costs.mvcost,
                                               lambda,
                                               best_costs ,
                                               best_mv);
        }//
    }// list_num


    // Write the results in the arrays //
    /////////////////////////////////////

     m_mv_array[ypos][xpos] = best_mv;
     m_cost_array[ypos][xpos] = best_costs;

}
void BlockMatcher::RefineMatchSubp(const int xpos, const int ypos,
                                   const MVector& mv_prediction,
                                   const float lambda)
{

    BlockDiffParams dparams;
    dparams.SetBlockLimits( m_bparams , m_pic_data , xpos , ypos);

    m_cost_array[ypos][xpos].mvcost = GetVarUp( mv_prediction,
                                                m_mv_array[ypos][xpos]<<m_precision );
    m_cost_array[ypos][xpos].SetTotal( lambda );

    // Initialise to the best pixel value
    MvCostData best_costs( m_cost_array[ypos][xpos] );
    MVector pel_mv( m_mv_array[ypos][xpos] );
    MVector best_mv( pel_mv );

    // If the integer value is good enough, bail out
    if ( best_costs.SAD < 2*dparams.Xl()*dparams.Yl() )
    {
        m_mv_array[ypos][xpos] = m_mv_array[ypos][xpos]<<m_precision;
        return;
    }

    // Next, test the predictor. If that's good enough, bail out
    MvCostData pred_costs;
    pred_costs.mvcost = 0;
    pred_costs.SAD = m_subpeldiff[m_precision-1]->Diff( dparams, mv_prediction);
    pred_costs.total = pred_costs.SAD;

    if (pred_costs.SAD<2*dparams.Xl()*dparams.Yl() )
    {
        m_mv_array[ypos][xpos] = mv_prediction;
        m_cost_array[ypos][xpos] = pred_costs;
        return;
    }

    // Now, let's see if we can do better than this

    MvCostData cand_costs;
    MVector cand_mv, old_best_mv;

    for (int i=1; i<=m_precision; ++i )
    {
        best_mv = best_mv<<1;
        MVector temp_best_mv = best_mv;

        // Do a neighbourhood of best_mv

        // Stage 1 - look at the 4 nearest points
        cand_mv.x = best_mv.x - 1;
        cand_mv.y = best_mv.y;
        m_subpeldiff[i-1]->Diff( dparams, cand_mv ,
                                 GetVarUp( mv_prediction, 
                                           cand_mv<<(m_precision-i) ) ,
                                 lambda , best_costs ,
                                 temp_best_mv);
        cand_mv.x = best_mv.x + 1;
        cand_mv.y = best_mv.y;
        m_subpeldiff[i-1]->Diff( dparams, cand_mv ,
                                 GetVarUp( mv_prediction, 
                                           cand_mv<<(m_precision-i) ) ,
                                 lambda , best_costs ,
                                 temp_best_mv);
        cand_mv.x = best_mv.x;
        cand_mv.y = best_mv.y - 1;
        m_subpeldiff[i-1]->Diff( dparams, cand_mv ,
                                 GetVarUp( mv_prediction, 
                                           cand_mv<<(m_precision-i) ) ,
                                 lambda , best_costs ,
                                 temp_best_mv);
        cand_mv.x = best_mv.x;
        cand_mv.y = best_mv.y + 1;
        m_subpeldiff[i-1]->Diff( dparams, cand_mv ,
                                 GetVarUp( mv_prediction, 
                                           cand_mv<<(m_precision-i) ) ,
                                 lambda , best_costs ,
                                 temp_best_mv);

        // Stage 2. If we've done better than the original value, 
        // look at the other two neighbours 
        if ( temp_best_mv.x != best_mv.x )
        {
            MVector new_best_mv = temp_best_mv;
            cand_mv.x = new_best_mv.x;
            cand_mv.y = new_best_mv.y - 1;
            m_subpeldiff[i-1]->Diff( dparams, cand_mv ,
                                     GetVarUp( mv_prediction, 
                                               cand_mv<<(m_precision-i) ) ,
                                     lambda , best_costs ,
                                     temp_best_mv);

            cand_mv.x = new_best_mv.x;
            cand_mv.y = new_best_mv.y + 1;
            m_subpeldiff[i-1]->Diff( dparams, cand_mv ,
                                     GetVarUp( mv_prediction, 
                                               cand_mv<<(m_precision-i) ) ,
                                     lambda , best_costs ,
                                     temp_best_mv);
        }
        else if ( temp_best_mv.y != best_mv.y )
        {
            MVector new_best_mv = temp_best_mv;
            cand_mv.x = new_best_mv.x - 1;
            cand_mv.y = new_best_mv.y;
            m_subpeldiff[i-1]->Diff( dparams, cand_mv ,
                                     GetVarUp( mv_prediction, 
                                               cand_mv<<(m_precision-i) ) ,
                                     lambda , best_costs ,
                                     temp_best_mv);

            cand_mv.x = new_best_mv.x + 1;
            cand_mv.y = new_best_mv.y;
            m_subpeldiff[i-1]->Diff( dparams, cand_mv ,
                                     GetVarUp( mv_prediction, 
                                               cand_mv<<(m_precision-i) ) ,
                                     lambda , best_costs ,
                                     temp_best_mv);
        } 

        best_mv = temp_best_mv;

        // Bail out if we can't do better than 10% worse than the predictor at
        // each stage
        if ( best_costs.total>1.1*pred_costs.total )
        {
            m_mv_array[ypos][xpos] = mv_prediction;
            m_cost_array[ypos][xpos] = pred_costs;
            return;   
        }

    }//i


    // Write the results in the arrays //
    /////////////////////////////////////

     m_mv_array[ypos][xpos] = best_mv;
     m_cost_array[ypos][xpos] = best_costs;   

}

} // namespace dirac
