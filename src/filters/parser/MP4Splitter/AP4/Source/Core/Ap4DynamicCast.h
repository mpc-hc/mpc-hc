/*****************************************************************
|
|    AP4 - Target Platform and Compiler Configuration
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
 * @brief Dynamic Cast Support
 */
#ifndef _AP4_DYNAMIC_CAST_H_
#define _AP4_DYNAMIC_CAST_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Config.h"

/*----------------------------------------------------------------------
|   macros
+---------------------------------------------------------------------*/
#if defined(AP4_CONFIG_NO_RTTI)
#define AP4_DYNAMIC_CAST(_class,_object) \
( ((_object)==0) ? 0 : reinterpret_cast<_class*>((_object)->DynamicCast(&_class::_class_##_class)) )
#define AP4_IMPLEMENT_DYNAMIC_CAST(_class)              \
static int _class_##_class;                             \
virtual void* DynamicCast(const void* class_anchor) {   \
    if (class_anchor ==  &_class::_class_##_class) {    \
        return static_cast<_class*>(this);              \
    }                                                   \
    return NULL;                                        \
}
#define AP4_IMPLEMENT_DYNAMIC_CAST_D(_class,_superclass)\
static int _class_##_class;                             \
virtual void* DynamicCast(const void* class_anchor) {   \
    if (class_anchor ==  &_class::_class_##_class) {    \
        return static_cast<_class*>(this);              \
    } else {                                            \
        return _superclass::DynamicCast(class_anchor);  \
    }                                                   \
}
#define AP4_IMPLEMENT_DYNAMIC_CAST_D2(_class,_superclass,_mixin)\
static int _class_##_class;                                 \
virtual void* DynamicCast(const void* class_anchor) {       \
    if (class_anchor ==  &_class::_class_##_class) {        \
        return static_cast<_class*>(this);                  \
    } else {                                                \
        void* sup = _superclass::DynamicCast(class_anchor); \
        if (sup) return sup;                                \
        return _mixin::DynamicCast(class_anchor);           \
    }                                                       \
}
#define AP4_DEFINE_DYNAMIC_CAST_ANCHOR(_class) int _class::_class_##_class = 0;

#else

#define AP4_DYNAMIC_CAST(_class,_object) dynamic_cast<_class*>(_object)
#define AP4_IMPLEMENT_DYNAMIC_CAST(_class)
#define AP4_IMPLEMENT_DYNAMIC_CAST_D(_class,_superclass)
#define AP4_IMPLEMENT_DYNAMIC_CAST_D2(_class,_superclass,_mixin)
#define AP4_DEFINE_DYNAMIC_CAST_ANCHOR(_class)

#endif

#endif // _AP4_DYNAMIC_CAST_H_
