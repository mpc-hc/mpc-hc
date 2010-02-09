/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: arrays.h,v 1.21 2008/03/14 08:17:36 asuraparaju Exp $ $Name:  $
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
*                 Peter Meerwald (pmeerw@users.sourceforge.net)
*                 Mike Ferenduros (mike_ferenzduros@users.sourceforge.net)
*                 Anuradha Suraparaju
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

#ifndef _ARRAYS_H_
#define _ARRAYS_H_

//basic array types used for pictures etc

#include <memory>
#include <cstddef>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cstring>

namespace dirac
{
    //! Range type. 
    /*!
        Range type encapsulating a closed range of values [first,last]. 
        Used to initialies OneDArrays.
     */
    class Range
    {
    public:
        //! Constructor
        /*!
            Constructor taking a start and an end point for the range.
         */
        Range(int s, int e): m_fst(s), m_lst(e){}

        //! Returns the start of the range.
        int First() const {return m_fst;}

        //! Returns the end point of the range.
        int Last() const {return m_lst;}

    private:
        int m_fst ,m_lst;
    };

    //////////////////////////////
    //One-Dimensional Array type//
    //////////////////////////////

    //! A template class for one-dimensional arrays.
    /*!
        A template class for one-D arrays. Can be used wherever built-in 
        arrays are used, and eliminates the need for explicit memory 
        (de-)allocation. Also supports arrays not based at zero.
     */
    template <class T> class OneDArray
    {
    public:
        //! Default constructor.
        /*!
            Default constructor produces an empty array.
         */    
        OneDArray();

        //! 'Length' constructor.
        /*!
            Length constructor produces a zero-based array.
         */    
        OneDArray(const int len);

       //! Range constructor
        /*!
            Range constructor produces an array with values indexed within the 
            range parameters.
            \param    r    a range of indexing values.
         */        
        OneDArray(const Range& r);

        //! Destructor.
        /*!
            Destructor frees the data allocated in the constructors.
         */
        ~OneDArray()
        {
            FreePtr();
        }

        //! Copy constructor.
        /*!
            Copy constructor copies both data and metadata.
         */
        OneDArray(const OneDArray<T>& cpy);

        //! Assignment=
        /*!
            Assignment= assigns both data and metadata.
         */
        OneDArray<T>& operator=(const OneDArray<T>& rhs);    

        //! Resize the array, throwing away the current data.
        void Resize(int l);

        //! Element access.
        T& operator[](const int pos){return m_ptr[pos-m_first];}

        //! Element access.
        const T& operator[](const int pos) const {return m_ptr[pos-m_first];}

        //! Returns the length of the array.    
        int Length() const {return m_length;}

        //! Returns the index of the first element.    
        int First() const {return m_first;}

        //! Returns the index of the last element.    
        int Last() const {return m_last;}

    private:
        void Init(const int len);

        void Init(const Range& r);

        void FreePtr();    

        int m_first, m_last;
        int m_length;
        T* m_ptr;
    };

    //public member functions//
    ///////////////////////////

    template <class T>
    OneDArray<T>::OneDArray()
    {
        Init(0);
    }

    template <class T>
    OneDArray<T>::OneDArray(const int len)
    {
        Init(len);
    }

    template <class T>
    OneDArray<T>::OneDArray(const Range& r)
    {
        Init(r);
    }

    template <class T>
    OneDArray<T>::OneDArray(const OneDArray<T>& cpy)
    {
        m_first = cpy.m_first;
        m_last = cpy.m_last;
        m_length = m_last - m_first + 1;

        if (m_first==0)
            Init(m_length);
        else
            Init(Range(m_first , m_last));

        memcpy( m_ptr , cpy.m_ptr , m_length * sizeof( T ) );
    }

    template <class T>
    OneDArray<T>& OneDArray<T>::operator=(const OneDArray<T>& rhs)
    {
        if (&rhs != this)
        {
            FreePtr();
            m_first = rhs.m_first;
            m_last = rhs.m_last;
            m_length = rhs.m_length;

            if (m_first == 0)
                Init(m_length);
            else
                Init(Range(m_first , m_last));

            memcpy( m_ptr , rhs.m_ptr , m_length * sizeof( T ) );

        }
        return *this;
    }

    template <class T> 
    void OneDArray<T>::Resize(int l)
    {
        if (l != m_length)
        {
            FreePtr();
            Init(l);
        }
    }

    //private member functions//
    ////////////////////////////

    template <class T>
    void OneDArray<T>::Init(const int len)
    {
        Range r(0 , len-1);

        Init(r);

    }        

    template <class T>
    void OneDArray<T>::Init(const Range& r)
    {

        m_first = r.First();
        m_last = r.Last();
        m_length = m_last - m_first + 1; 

        if ( m_length>0 ) 
        {
            m_ptr = new T[ m_length ];
        }
        else 
        {
            m_length = 0;
            m_first = 0;
            m_last = -1;
            m_ptr = NULL;
        }
    }

    template <class T>
    void OneDArray<T>::FreePtr()
    {
        if ( m_length>0 )
            delete[] m_ptr;
    }


    //////////////////////////////
    //Two-Dimensional Array type//
    //////////////////////////////

    //! A template class for two-dimensional arrays.
    /*!
        A template class to do two-d arrays, so that explicit memory 
        (de-)allocation is not required. Only zero-based arrays are 
        currently supported so that access is fast. Accessing elements along 
        a row is therefore much faster than accessing them along a column.
        Rows are contiguous in memory, so array[y][x] is equivalent to
        array[0][x+y*LengthX()].
     */
    template <class T> class TwoDArray
    {
        typedef T* element_type;

    public:

        //! Default constructor.
        /*!
            Default constructor creates an empty array.
         */    
        TwoDArray(){ Init(0,0); }

        //! Constructor.
        /*!
            The constructor creates an array of given width height.
         */    
        TwoDArray( const int height , const int width ){Init(height , width);}

        //! Constructor.
        /*!
            The constructor creates an array of given width and length height 
            and initialises it to a value
         */    
        TwoDArray( const int height , const int width , T val);

        //! Destructor
        /*!
            Destructor frees the data allocated in the constructor.
         */    
        virtual ~TwoDArray(){
            FreeData();    
        }

        //! Copy constructor.
        /*!
            Copy constructor copies data and metadata.
         */    
        TwoDArray(const TwoDArray<T>& Cpy);

        //! Assignment =
        /*!
            Assignement = assigns both data and metadata.
         */    
        TwoDArray<T>& operator=(const TwoDArray<T>& rhs);

        //! Copy Contents
        /*!
            Copy contents of array into output array retaining the dimensions
            of the output array. If output array is larger that array then
            pad with last true value.
            Return true is copy was successful
         */    
        bool CopyContents(TwoDArray<T>& out) const;

        //! Fill contents
        /*!
            Initialise the array with the val provided.
         */    
        void Fill(T val);

        //! Resizes the array, deleting the current data.    
        void Resize(const int height, const int width);    

        //! Element access.
        /*!
            Accesses the rows of the arrays, which are returned in the form 
            of pointers to the row data NOT OneDArray objects.
         */    
        inline element_type& operator[](const int pos){return m_array_of_rows[pos];}

        //! Element access.
        /*!
            Accesses the rows of the arrays, which are returned in the form of 
            pointers to the row data NOT OneDArray objects.
         */
        inline const element_type& operator[](const int pos) const {return m_array_of_rows[pos];}

        //! Returns the width
        int LengthX() const { return m_length_x; }

        //! Returns the height
        int LengthY() const { return m_length_y; }

        //! Returns the index of the first element of a row
        int FirstX() const { return m_first_x; } 

        //! Returns the index of the first element of a column
        int FirstY() const { return m_first_y; } 

        //! Returns the index of the last element of a row
        int LastX() const { return m_last_x; } 

        //! Returns the index of the first element of a column
        int LastY() const { return m_last_y; }

    private:
        //! Initialise the array
        void Init(const int height,const int width);

        //! Free all the allocated data
        void FreeData();    

        int m_first_x;
        int m_first_y;

        int m_last_x;
        int m_last_y;

        int m_length_x;
        int m_length_y;

        element_type* m_array_of_rows;
    };

    //public member functions//
    ///////////////////////////

    template <class T>
    TwoDArray<T>::TwoDArray( const int height , const int width , const T val)
    {
        Init( height , width );  
        std::fill_n( m_array_of_rows[0], m_length_x*m_length_y, val);
    }  

    template <class T>
    TwoDArray<T>::TwoDArray(const TwoDArray<T>& Cpy)
    {
        m_first_x = Cpy.m_first_x;
        m_first_y = Cpy.m_first_y;        
        m_last_x = Cpy.m_last_x;
        m_last_y = Cpy.m_last_y;

        m_length_x = m_last_x - m_first_x + 1;
        m_length_y = m_last_y - m_first_y + 1;        

        if (m_first_x == 0 && m_first_y == 0)        
            Init(m_length_y , m_length_x);
        else{
                //based 2D arrays not yet supported    
        }

        memcpy( m_array_of_rows[0] , (Cpy.m_array_of_rows)[0] , m_length_x * m_length_y * sizeof( T ) );

    }

    template <class T>
    TwoDArray<T>& TwoDArray<T>::operator=(const TwoDArray<T>& rhs)
    {
        if (&rhs != this)
        {
            FreeData();

            m_first_x = rhs.m_first_x;
            m_first_y = rhs.m_first_y;            

            m_last_x = rhs.m_last_x;
            m_last_y = rhs.m_last_y;

            m_length_x = m_last_x - m_first_x + 1;
            m_length_y = m_last_y - m_first_y + 1;        

            if (m_first_x == 0 && m_first_y == 0)
                Init(m_length_y , m_length_x);
            else
            {
                    //based 2D arrays not yet supported
            }

            memcpy( m_array_of_rows[0], (rhs.m_array_of_rows)[0], m_length_x * m_length_y * sizeof( T ) );

        }

        return *this;

    }
    
    template <class T>
    bool TwoDArray<T>::CopyContents(TwoDArray<T>& out) const
    {
        if (&out != this)
        {
            int rows = std::min (m_length_y, out.m_length_y);
            int cols = std::min (m_length_x, out.m_length_x);
            for (int j = 0; j < rows; ++j)
            {
                memcpy( out.m_array_of_rows[j], m_array_of_rows[j], cols * sizeof( T )) ;
                for (int i = cols; i <out.m_length_x; ++i)
                    out.m_array_of_rows[j][i] = out.m_array_of_rows[j][cols-1];
            }
            for (int j = rows; j < out.m_length_y; ++j)
            {
                memcpy( out.m_array_of_rows[j], out.m_array_of_rows[rows-1], out.m_length_x * sizeof( T )) ;
            }
        }
        return true;
    }
    
    template <class T>
    void TwoDArray<T>::Fill( T val)
    {
        if (m_length_x && m_length_y)
            std::fill_n( m_array_of_rows[0], m_length_x*m_length_y, val);
    }  

    template <class T>
    void TwoDArray<T>::Resize(const int height, const int width)
    {
        if (height != m_length_y || width != m_length_x)
        {
            FreeData();
            Init(height , width);
        }
    }

    //private member functions//
    ////////////////////////////

    template <class T>
    void TwoDArray<T>::Init(const int height , const int width)
    {
        m_length_x = width; 
        m_length_y = height;
        m_first_x = 0;
        m_first_y = 0;

        m_last_x = m_length_x-1;
        m_last_y = m_length_y-1;

        if (m_length_y>0)
        {
            // allocate the array containing ptrs to all the rows
            m_array_of_rows = new element_type[ m_length_y ];

            if ( m_length_x>0 )
            {
                // Allocate the whole thing as a single big block
                m_array_of_rows[0] = new T[ m_length_x * m_length_y ];

                // Point the pointers
                for (int j=1 ; j<m_length_y ; ++j)
                    m_array_of_rows[j] = m_array_of_rows[0] + j * m_length_x;
            }
            else
            {
                m_length_x = 0;
                m_first_x = 0;
                m_last_x = -1;
            }
        }
        else 
        {
            m_length_x = 0;
            m_length_y = 0;
            m_first_x = 0;
            m_first_y = 0;
            m_last_x = -1;
            m_last_y = -1;
            m_array_of_rows = NULL;
        }
    }

    template <class T>
    void TwoDArray<T>::FreeData()
    {
        if (m_length_y>0)
        {
            if (m_length_x>0) 
            {
                delete[] m_array_of_rows[0];
            }

            m_length_y = m_length_x = 0;
            // deallocate the array of rows
            delete[] m_array_of_rows;
        }    
    }

    // Related functions

    //! A function for extracting array data
    template <class T >
    std::ostream & operator<< (std::ostream & stream, TwoDArray<T> & array)
    {
        for (int j=0 ; j<array.LengthY() ; ++j)
        {
            for (int i=0 ; i<array.LengthX() ; ++i)
            {
                stream << array[j][i] << " ";
            }// i
            stream << std::endl;
        }// j

        return stream;
    }

    //! A function for inserting array data
    template <class T >
    std::istream & operator>> (std::istream & stream, TwoDArray<T> & array)
    {
        for (int j=0 ; j<array.LengthY() ; ++j)
        {
            for (int i=0 ; i<array.LengthX() ; ++i)
            {
                stream >> array[j][i];
            }// i
        }// j

        return stream;
    }

} //namespace dirac
#endif
