/*****************************************************************
|
|    AP4 - Lists
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

#ifndef _AP4_LIST_H_
#define _AP4_LIST_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Results.h"

/*----------------------------------------------------------------------
|   forward references
+---------------------------------------------------------------------*/
template <typename T> class AP4_List;

/*----------------------------------------------------------------------
|   AP4_List
+---------------------------------------------------------------------*/
template <typename T> 
class AP4_List 
{
public:
    // types
    class Item 
    {
    public:
        // types
        class Operator 
        {
        public:
            // methods
            virtual ~Operator() {}
            virtual AP4_Result Action(T* data) const = 0;
        };

        class Finder 
        {
        public:
            // methods
            virtual ~Finder() {}
            virtual AP4_Result Test(T* data) const = 0;
        };

        // methods
        Item(T* data) : m_Data(data), m_Next(0), m_Prev(0) {}
       ~Item() {}
        Item* GetNext() { return m_Next; }
        Item* GetPrev() { return m_Prev; }
        T*    GetData() { return m_Data; }

    private:
        // members
        T*    m_Data;
        Item* m_Next;
        Item* m_Prev;

        // friends
        friend class AP4_List;
    };

    // methods
                 AP4_List<T>(): m_ItemCount(0), m_Head(0), m_Tail(0) {}
    virtual     ~AP4_List<T>();
    AP4_Result   Clear();
    AP4_Result   Add(T* data);
    AP4_Result   Add(Item* item);
    AP4_Result   Remove(T* data);
    AP4_Result   Insert(Item* where, T* data);
    AP4_Result   Get(AP4_Ordinal idx, T*& data) const;
    AP4_Result   PopHead(T*& data);
    AP4_Result   Apply(const typename Item::Operator& op) const;
    AP4_Result   ApplyUntilFailure(const typename Item::Operator& op) const;
    AP4_Result   ApplyUntilSuccess(const typename Item::Operator& op) const ;
    AP4_Result   ReverseApply(const typename Item::Operator& op) const;
    AP4_Result   Find(const typename Item::Finder& finder, T*& data) const;
    AP4_Result   ReverseFind(const typename Item::Finder& finder, T*& data) const;
    AP4_Result   DeleteReferences();
    AP4_Cardinal ItemCount() const { return m_ItemCount; }
    Item*        FirstItem() const { return m_Head; }
    Item*        LastItem()  const { return m_Tail; }
 
protected:
    // members
    AP4_Cardinal m_ItemCount;
    Item*        m_Head;
    Item*        m_Tail;
    
private:
	// these cannot be used
    AP4_List<T>(const AP4_List<T>&);
	AP4_List<T>& operator=(const AP4_List<T>&);
};

/*----------------------------------------------------------------------
|   AP4_List<T>::~AP4_List<T>
+---------------------------------------------------------------------*/
template <typename T>
AP4_List<T>::~AP4_List()
{
    Clear();
}

/*----------------------------------------------------------------------
|   AP4_List<T>::Clear
+---------------------------------------------------------------------*/
template <typename T>
inline
AP4_Result
AP4_List<T>::Clear()
{
    Item* item = m_Head;
 
    while (item) {
        Item* next = item->m_Next;
        delete item;
        item = next;
    }
    m_ItemCount = 0;
    m_Head = m_Tail = NULL;
    
    return AP4_SUCCESS;
}
 
/*----------------------------------------------------------------------
|   AP4_List<T>::Add
+---------------------------------------------------------------------*/
template <typename T>
inline
AP4_Result
AP4_List<T>::Add(T* data)
{
    return Add(new Item(data));
}

/*----------------------------------------------------------------------
|   AP4_List<T>::Add
+---------------------------------------------------------------------*/
template <typename T>
AP4_Result
AP4_List<T>::Add(Item* item)
{
    // add element at the tail
    if (m_Tail) {
        item->m_Prev = m_Tail;
        item->m_Next = NULL;
        m_Tail->m_Next = item;
        m_Tail = item;
    } else {
        m_Head = item;
        m_Tail = item;
        item->m_Next = NULL;
        item->m_Prev = NULL;
    }

    // one more item in the list now
    m_ItemCount++;
 
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_List<T>::Remove
+---------------------------------------------------------------------*/
template <typename T>
AP4_Result
AP4_List<T>::Remove(T* data)
{
    Item* item = m_Head;

    while (item) {
        if (item->m_Data == data) {
            // delete item
            if (item->m_Prev) {
                // item is not the head
                if (item->m_Next) {
                    // item is not the tail
                    item->m_Next->m_Prev = item->m_Prev;
                    item->m_Prev->m_Next = item->m_Next;
                } else {
                    // item is the tail
                    m_Tail = item->m_Prev;
                    m_Tail->m_Next = NULL;
                }
            } else {
                // item is the head
                m_Head = item->m_Next;
                if (m_Head) {
                    // item is not the tail
                    m_Head->m_Prev = NULL;
                } else {
                    // item is also the tail
                    m_Tail = NULL;
                }
            }

            // delete the item
            delete item;

            // one less item in the list now
            m_ItemCount--;

            return AP4_SUCCESS;
        }
        item = item->m_Next;
    }
 
    return AP4_ERROR_NO_SUCH_ITEM;
}

/*----------------------------------------------------------------------
|   AP4_List<T>::Insert
+---------------------------------------------------------------------*/
template <typename T>
AP4_Result
AP4_List<T>::Insert(Item* where, T* data)
{
    Item* item = new Item(data);

    if (where == NULL) {
        // insert as the head
        if (m_Head) {
            // replace the current head
            item->m_Prev = NULL;
            item->m_Next = m_Head;
            m_Head->m_Prev = item;
            m_Head = item;
        } else {
            // this item becomes the head and tail
            m_Head = item;
            m_Tail = item;
            item->m_Next = NULL;
            item->m_Prev = NULL;
        }
    } else {
        // insert after the 'where' item
        if (where == m_Tail) {
            // add the item at the end
            return Add(item);
        } else {
            // update the links
            item->m_Prev = where;
            item->m_Next = where->m_Next;
            where->m_Next->m_Prev = item;
            where->m_Next = item;
        }
    }

    // one more item in the list now
    ++m_ItemCount;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_List<T>::Get
+---------------------------------------------------------------------*/
template <typename T>
AP4_Result
AP4_List<T>::Get(AP4_Ordinal idx, T*& data) const
{
    Item* item = m_Head;

    if (idx < m_ItemCount) {
        while (idx--) item = item->m_Next;
        data = item->m_Data;
        return AP4_SUCCESS;
    } else {
        data = NULL;
        return AP4_ERROR_NO_SUCH_ITEM;
    }
}

/*----------------------------------------------------------------------
|   AP4_List<T>::PopHead
+---------------------------------------------------------------------*/
template <typename T>
AP4_Result
AP4_List<T>::PopHead(T*& data)
{
    // check that we have at least one item
    if (m_Head == NULL) {
        return AP4_ERROR_LIST_EMPTY;
    }

    // remove the item and return it
    data = m_Head->m_Data;
    Item* head = m_Head;
    m_Head = m_Head->m_Next;
    if (m_Head) {
        m_Head->m_Prev = NULL;
    } else {
        m_Tail = NULL;
    }

    // delete item
    delete head;

    // one less item in the list now
    m_ItemCount--;
 
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_List<T>::Apply
+---------------------------------------------------------------------*/
template <typename T>
inline 
AP4_Result
AP4_List<T>::Apply(const typename Item::Operator& op) const
{
    Item* item = m_Head;
 
    while (item) {
        op.Action(item->m_Data);
        item = item->m_Next;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_List<T>::ApplyUntilFailure
+---------------------------------------------------------------------*/
template <typename T>
inline 
AP4_Result
AP4_List<T>::ApplyUntilFailure(const typename Item::Operator& op) const
{
    Item* item = m_Head;
 
    while (item) {
        AP4_Result result;
        result = op.Action(item->m_Data);
        if (result != AP4_SUCCESS) return result;
        item = item->m_Next;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_List<T>::ApplyUntilSuccess
+---------------------------------------------------------------------*/
template <typename T>
inline 
AP4_Result
AP4_List<T>::ApplyUntilSuccess(const typename Item::Operator& op) const
{
    Item* item = m_Head;
 
    while (item) {
        AP4_Result result;
        result = op.Action(item->m_Data);
        if (result == AP4_SUCCESS) return AP4_SUCCESS;
        item = item->m_Next;
    }

    return AP4_FAILURE;
}

/*----------------------------------------------------------------------
|   AP4_List<T>::ReverseApply
+---------------------------------------------------------------------*/
template <typename T>
inline 
AP4_Result
AP4_List<T>::ReverseApply(const typename Item::Operator& op) const
{
    Item* item = m_Tail;
 
    while (item) {
        if (op.Action(item->m_Data) != AP4_SUCCESS) {
            return AP4_ERROR_LIST_OPERATION_ABORTED;
        }
        item = item->m_Prev;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_List<T>::Find
+---------------------------------------------------------------------*/
template <typename T>
inline 
AP4_Result
AP4_List<T>::Find(const typename Item::Finder& finder, T*& data) const
{
    Item* item = m_Head;
 
    while (item) {
        if (finder.Test(item->m_Data) == AP4_SUCCESS) {
            data = item->m_Data;
            return AP4_SUCCESS;
        }
        item = item->m_Next;
    }

    data = NULL;
    return AP4_ERROR_NO_SUCH_ITEM;
}

/*----------------------------------------------------------------------
|   AP4_List<T>::ReverseFind
+---------------------------------------------------------------------*/
template <typename T>
inline 
AP4_Result
AP4_List<T>::ReverseFind(const typename Item::Finder& finder, T*& data) const
{
    Item* item = m_Tail;
 
    while (item) {
        if (finder.Test(item->m_Data) == AP4_SUCCESS) {
            data = item->m_Data;
            return AP4_SUCCESS;
        }
        item = item->m_Prev;
    }

    data = NULL;
    return AP4_ERROR_NO_SUCH_ITEM;
}

/*----------------------------------------------------------------------
|   AP4_List<T>::DeleteReferences
+---------------------------------------------------------------------*/
template <typename T>
inline 
AP4_Result
AP4_List<T>::DeleteReferences()
{
    Item* item = m_Head;
 
    while (item) {
        Item* next = item->m_Next;
        delete item->m_Data;
        delete item;
        item = next;
    }

    // no more items
    m_Head = m_Tail = NULL;
    m_ItemCount = 0;

    return AP4_SUCCESS;
}

#endif // _AP4_LIST_H_













