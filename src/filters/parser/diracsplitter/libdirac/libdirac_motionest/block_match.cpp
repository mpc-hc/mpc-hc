/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: block_match.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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

ValueType GetVar( const MVector& predmv , const MVector& mv )
{
    MVector diff;
    diff.x = mv.x-predmv.x;
    diff.y = mv.y-predmv.y;    

    return std::max( Norm1( diff ) , 48 );
}

ValueType GetVar( const std::vector<MVector>& pred_list , const MVector& mv)
{
    ValueType sum=0;
    MVector diff;
    for (size_t i=0 ; i<pred_list.size() ; ++i)
    {
        diff.x = mv.x - pred_list[i].x;
        diff.y = mv.y - pred_list[i].y;
        sum += Norm1( diff );
    }

    return sum;
}

void AddNewVlist( CandidateList& vect_list, const MVector& mv, const int xr , const int yr , const int step )
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

BlockMatcher::BlockMatcher( const PicArray& pic_data , const PicArray& ref_data , const OLBParams& bparams ,
                            const MvArray& mv_array , const TwoDArray< MvCostData >& cost_array):
    m_pic_data(pic_data), 
    m_ref_data(ref_data),
    m_mv_array(mv_array),
    m_cost_array(cost_array),
    m_simplediff( ref_data , pic_data ), //NB: ORDER!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    m_checkdiff( ref_data , pic_data ),
    m_simplediffup( ref_data , pic_data ),
    m_checkdiffup( ref_data , pic_data ),
    m_bparams(bparams)
{}
    
void BlockMatcher::FindBestMatch(int xpos , int ypos ,
                                 const CandidateList& cand_list,
                                 const MVector& mv_prediction,
                                 float lambda)
{
    BlockDiffParams dparams;
    dparams.SetBlockLimits( m_bparams , m_pic_data , xpos , ypos);
    lambda /= m_bparams.Xblen()*m_bparams.Yblen();
    lambda *= dparams.Xl()*dparams.Yl();
    
    // Pointer to either a simple block diff object, or a bounds-checking one
    BlockDiff* mydiff;

       //now test against the offsets in the MV list to get the lowest cost//
      //////////////////////////////////////////////////////////////////////    

    // Numbers of the lists to do more searching in
    vector<int> list_nums; 

    // Costs of the initial vectors in each list
    OneDArray<float> list_costs( cand_list.size() );

    // The minimum cost so far
    float min_cost;    

    // First test the first in each of the lists to choose which lists to pursue

    MvCostData best_costs;
    // Initialise so that we choose a valid vector to start with!
    best_costs.total=100000000.0f;
    MVector best_mv( cand_list[0][0] );

    MVector cand_mv;
    MvCostData cand_costs;

    for (size_t lnum=0 ; lnum<cand_list.size() ; ++lnum )
    {

        cand_mv = cand_list[lnum][0];
        cand_costs.mvcost = GetVar( mv_prediction , cand_mv );

        // See whether we need to do bounds checking or not
        if (( dparams.Xp()+cand_mv.x )<0 || ( dparams.Xp()+dparams.Xl()+cand_mv.x) >= m_ref_data.LengthX() ||
            (dparams.Yp()+cand_mv.y)<0 || (dparams.Yp()+dparams.Yl()+cand_mv.y) >= m_ref_data.LengthY() )
            mydiff = &m_checkdiff;
        else
            mydiff = &m_simplediff;    

        cand_costs.SAD = mydiff->Diff( dparams , cand_mv );
        cand_costs.SetTotal( lambda );

        if ( cand_costs.total < best_costs.total)
        {
            best_costs = cand_costs;
            best_mv = cand_mv ;

        }

        list_costs[lnum] = cand_costs.total;

    }// lnum


    // Select which lists we're going to use //
    ///////////////////////////////////////////

    min_cost = list_costs[0];

    for ( int lnum=1 ; lnum<list_costs.Length() ; ++lnum)
    {
        if ( list_costs[lnum]<min_cost )
            min_cost = list_costs[lnum];
    }// lnum

    for ( int lnum=0 ; lnum<list_costs.Length() ; ++lnum)
    {
        // Only do lists whose 1st element isn't too far off best
        if ( list_costs[lnum] < 1.5*min_cost ) // (value of 1.5 TBD) 
            list_nums.push_back( lnum );
    }// lnum


    // Ok, now we know which lists to pursue. Just go through all of them //
    ////////////////////////////////////////////////////////////////////////
    int list_num;

    for ( size_t num=0 ; num<list_nums.size() ; ++num)
    {
        list_num = list_nums[num];

        for (size_t i=1 ; i<cand_list[list_num].size() ; ++i)
        {//start at 1 since did 0 above

            cand_mv = cand_list[list_num][i];
            cand_costs.mvcost =  GetVar( mv_prediction , cand_mv);
            
            if ((dparams.Xp()+cand_mv.x)<0 || (dparams.Xp()+dparams.Xl()+cand_mv.x) > m_ref_data.LengthX() ||
                (dparams.Yp()+cand_mv.y)<0 || (dparams.Yp()+dparams.Yl()+cand_mv.y) > m_ref_data.LengthY() )
                mydiff = &m_checkdiff;
            else
                mydiff = &m_simplediff;

            cand_costs.SAD = mydiff->Diff( dparams , cand_mv );
            cand_costs.SetTotal( lambda );
            
            if ( cand_costs.total < best_costs.total)
            {
                best_costs = cand_costs;
                best_mv = cand_mv;

            }
        }// i
    }// num

    // Write the results in the arrays //
    /////////////////////////////////////

    m_mv_array[ypos][xpos] = best_mv;
    m_cost_array[ypos][xpos] = best_costs;
}




void BlockMatcher::FindBestMatchSubp(int xpos, int ypos,
                                      const CandidateList& cand_list,
                                      const MVector& mv_prediction,
                                      float lambda)
{

    BlockDiffParams dparams;
    dparams.SetBlockLimits( m_bparams , m_pic_data , xpos , ypos);    

    // Pointer to either a simple block diff object, or a bounds-checking one
    BlockDiff* mydiff;

       //now test against the offsets in the MV list to get the lowest cost//
      //////////////////////////////////////////////////////////////////////    

    // Numbers of the lists to do more searching in
    vector<int> list_nums; 

    // Costs of the initial vectors in each list
    OneDArray<float> list_costs( cand_list.size() );

    // The minimum cost so far
    float min_cost;    

    // First test the first in each of the lists to choose which lists to pursue
    MvCostData best_costs( m_cost_array[ypos][xpos] );
    MVector best_mv( m_mv_array[ypos][xpos] );

    MvCostData cand_costs;
    MVector cand_mv;

    for (size_t list_num=0 ; list_num<cand_list.size() ; ++list_num )
    {

        cand_mv = cand_list[list_num][0];
        cand_costs.mvcost = GetVar( mv_prediction , cand_mv );

        // See whether we need to do bounds checking or not
        if (   (( dparams.Xp()<<1 )+(cand_mv.x>>2))<0 
            || ((( dparams.Xp()+dparams.Xl() )<<1)+(cand_mv.x>>2)) >= m_ref_data.LengthX()
            || (( dparams.Yp()<<1)+(cand_mv.y>>2))<0 
            || (((dparams.Yp()+dparams.Yl())<<1)+(cand_mv.y>>2)) >= m_ref_data.LengthY() )
            mydiff = &m_checkdiffup;
        else
            mydiff = &m_simplediffup;    

        cand_costs.SAD = mydiff->Diff( dparams , cand_mv );
        cand_costs.SetTotal( lambda );
 
       if (cand_costs.total< best_costs.total)
        {
            best_costs = cand_costs;
            best_mv = cand_mv;
        }
        
        list_costs[list_num] = cand_costs.total;
    }// list_num


    // Select which lists we're going to use //
    ///////////////////////////////////////////

    min_cost = list_costs[0];

    for ( int lnum=1 ; lnum<list_costs.Length() ; ++lnum)
    {
        if ( list_costs[lnum]<min_cost )
            min_cost = list_costs[lnum];
    }// lnum

    for ( int lnum=0 ; lnum<list_costs.Length() ; ++lnum )
    {
        // Only do lists whose 1st element isn't too far off best
        if ( list_costs[lnum] < 1.5*min_cost ) // (value of 1.5 TBD) 
            list_nums.push_back( lnum );
    }// lnum

    // Ok, now we know which lists to pursue. Just go through all of them //
    ////////////////////////////////////////////////////////////////////////
    int list_num;

    for ( size_t num=0 ; num<list_nums.size() ; ++num)
    {
        list_num = list_nums[num];

        for (size_t i=1 ; i<cand_list[list_num].size() ; ++i)
        {//start at 1 since did 0 above

            cand_mv = cand_list[list_num][i];
            cand_costs.mvcost = GetVar( mv_prediction , cand_mv );

            if (   (( dparams.Xp()<<1 )+( cand_mv.x>>2 ))<0 
                || ((( dparams.Xp()+dparams.Xl() )<<1)+( cand_mv.x>>2 )) >= m_ref_data.LengthX()
                || (( dparams.Yp()<<1 )+( cand_mv.y>>2 ))<0 
                || ((( dparams.Yp()+dparams.Yl() )<<1)+(cand_mv.y>>2)) >= m_ref_data.LengthY() )
                 mydiff = &m_checkdiffup;
            else
                mydiff = &m_simplediffup;

            cand_costs.SAD = mydiff->Diff( dparams , cand_mv );
            cand_costs.SetTotal( lambda );

            if (cand_costs.total< best_costs.total)
            {
                best_costs = cand_costs;
                best_mv = cand_mv;
            }

        }// i
    }// num

    // Write the results in the arrays //
    /////////////////////////////////////

     m_mv_array[ypos][xpos] = best_mv;
     m_cost_array[ypos][xpos] = best_costs;   

}
} // namespace dirac
