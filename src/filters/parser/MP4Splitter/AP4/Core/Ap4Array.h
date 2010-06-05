/*****************************************************************
|
|    AP4 - Arrays
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/
/**
 * @file 
 * @brief Arrays
 */

#ifndef _AP4_ARRAY_H_
#define _AP4_ARRAY_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Config.h"
#if defined(APT_CONFIG_HAVE_NEW_H)
#include <new>
#endif
#include "Ap4Types.h"
#include "Ap4Results.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const int AP4_ARRAY_INITIAL_COUNT = 64;

/*----------------------------------------------------------------------
|   AP4_Array
+---------------------------------------------------------------------*/
template <typename T> 
class AP4_Array 
{
public:
    // methods
             AP4_Array(): m_AllocatedCount(0), m_ItemCount(0), m_Items(0) {}
             AP4_Array(const T* items, AP4_Size count);
    virtual ~AP4_Array();
    AP4_Cardinal ItemCount() const { return m_ItemCount; }
    AP4_Result   Append(const T& item);
    AP4_Result   RemoveLast();
    T& operator[](unsigned long idx) { return m_Items[idx]; }
    const T& operator[](unsigned long idx) const { return m_Items[idx]; }
    AP4_Result Clear();
    AP4_Result EnsureCapacity(AP4_Cardinal count);
    AP4_Result SetItemCount(AP4_Cardinal item_count);

protected:
    // members
    AP4_Cardinal m_AllocatedCount;
    AP4_Cardinal m_ItemCount;
    T*           m_Items;
};

/*----------------------------------------------------------------------
|   AP4_Array<T>::AP4_Array<T>
+---------------------------------------------------------------------*/
template <typename T>
AP4_Array<T>::AP4_Array(const T* items, AP4_Size count) :
    m_AllocatedCount(count),
    m_ItemCount(count),
    m_Items((T*)::operator new(count*sizeof(T)))
{
    for (unsigned int i=0; i<count; i++) {
        new ((void*)&m_Items[i]) T(items[i]);
    }
}

/*----------------------------------------------------------------------
|   AP4_Array<T>::~AP4_Array<T>
+---------------------------------------------------------------------*/
template <typename T>
AP4_Array<T>::~AP4_Array()
{
    Clear();
    ::operator delete((void*)m_Items);
}

/*----------------------------------------------------------------------
|   NPT_Array<T>::Clear
+---------------------------------------------------------------------*/
template <typename T>
AP4_Result
AP4_Array<T>::Clear()
{
    // destroy all items
    for (AP4_Ordinal i=0; i<m_ItemCount; i++) {
        m_Items[i].~T();
    }

    m_ItemCount = 0;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Array<T>::EnsureCapacity
+---------------------------------------------------------------------*/
template <typename T>
AP4_Result
AP4_Array<T>::EnsureCapacity(AP4_Cardinal count)
{
    // check if we already have enough
    if (count <= m_AllocatedCount) return AP4_SUCCESS;

    // (re)allocate the items
    T* new_items = (T*) ::operator new (count*sizeof(T));
    if (new_items == NULL) {
        return AP4_ERROR_OUT_OF_MEMORY;
    }
    if (m_ItemCount && m_Items) {
        for (unsigned int i=0; i<m_ItemCount; i++) {
            new ((void*)&new_items[i]) T(m_Items[i]);
            m_Items[i].~T();
        }
        ::operator delete((void*)m_Items);
    }
    m_Items = new_items;
    m_AllocatedCount = count;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Array<T>::SetItemCount
+---------------------------------------------------------------------*/
template <typename T>
AP4_Result 
AP4_Array<T>::SetItemCount(AP4_Cardinal item_count)
{
    // shortcut
    if (item_count == m_ItemCount) return AP4_SUCCESS;
    
    // check for a reduction in the number of items
    if (item_count < m_ItemCount) {
        // destruct the items that are no longer needed
        for (unsigned int i=item_count; i<m_ItemCount; i++) {
            m_Items[i].~T();
        }
        m_ItemCount = item_count;
        return AP4_SUCCESS;
    }
    
    // grow the list
    AP4_Result result = EnsureCapacity(item_count);
    if (AP4_FAILED(result)) return result;
    
    // construct the new items
    for (unsigned int i=m_ItemCount; i<item_count; i++) {
        new ((void*)&m_Items[i]) T();
    }
    m_ItemCount = item_count;
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Array<T>::RemoveLast
+---------------------------------------------------------------------*/
template <typename T>
AP4_Result
AP4_Array<T>::RemoveLast()
{
    if (m_ItemCount) {
        m_Items[--m_ItemCount].~T();
        return AP4_SUCCESS;
    } else {
        return AP4_ERROR_OUT_OF_RANGE;
    }
}

/*----------------------------------------------------------------------
|   AP4_Array<T>::Append
+---------------------------------------------------------------------*/
template <typename T>
AP4_Result
AP4_Array<T>::Append(const T& item)
{
    // ensure that we have enough space
    if (m_AllocatedCount < m_ItemCount+1) {
        // try double the size, with a minimum
        unsigned long new_count = m_AllocatedCount?2*m_AllocatedCount:AP4_ARRAY_INITIAL_COUNT;

        // if that's still not enough, just ask for what we need
        if (new_count < m_ItemCount+1) new_count = m_ItemCount+1;
    
        // reserve the space
        AP4_Result result = EnsureCapacity(new_count);
        if (result != AP4_SUCCESS) return result;
    }
    
    // store the item
    new ((void*)&m_Items[m_ItemCount++]) T(item);

    return AP4_SUCCESS;
}

#endif // _AP4_ARRAY_H_













