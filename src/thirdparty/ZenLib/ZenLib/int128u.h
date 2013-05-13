/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// based on http://Tringi.Mx-3.cz
// Only adapted for ZenLib:
// - .hpp --> .h
// - Namespace
// - int128u alias
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef UINT128_HPP
#define UINT128_HPP

/*
  Name: uint128.hpp
  Copyright: Copyright (C) 2005, Jan Ringos
  Author: Jan Ringos, http://Tringi.Mx-3.cz

  Version: 1.1
*/


#include <exception>
#include <cstdlib>
#include <cstdio>
#include "ZenLib/Conf.h"

// CLASS

namespace ZenLib
{

class uint128 {
    public://private:
        // Binary correct representation of signed 128bit integer
        int64u lo;
        int64u hi;

    protected:
        // Some global operator functions must be friends
        friend bool operator <  (const uint128 &, const uint128 &) throw ();
        friend bool operator == (const uint128 &, const uint128 &) throw ();
        friend bool operator || (const uint128 &, const uint128 &) throw ();
        friend bool operator && (const uint128 &, const uint128 &) throw ();

    public:
        // Constructors
        inline uint128 () throw () {};
        inline uint128 (const uint128 & a) throw () : lo (a.lo), hi (a.hi) {};

        inline uint128 (const int & a) throw () : lo (a), hi (0ull) {};
        inline uint128 (const unsigned int & a) throw () : lo (a), hi (0ull) {};
        inline uint128 (const int64u & a) throw () : lo (a), hi (0ull) {};

        uint128 (const float a) throw ();
        uint128 (const double & a) throw ();
        uint128 (const long double & a) throw ();

        uint128 (const char * sz) throw ();

        // TODO: Consider creation of operator= to eliminate
        //       the need of intermediate objects during assignments.

    private:
        // Special internal constructors
        uint128 (const int64u & a, const int64u & b) throw ()
            : lo (a), hi (b) {};

    public:
        // Operators
        bool operator ! () const throw ();

        uint128 operator - () const throw ();
        uint128 operator ~ () const throw ();

        uint128 & operator ++ ();
        uint128 & operator -- ();
        uint128 operator ++ (int);
        uint128 operator -- (int);

        uint128 & operator += (const uint128 & b) throw ();
        uint128 & operator *= (const uint128 & b) throw ();

        uint128 & operator >>= (unsigned int n) throw ();
        uint128 & operator <<= (unsigned int n) throw ();

        uint128 & operator |= (const uint128 & b) throw ();
        uint128 & operator &= (const uint128 & b) throw ();
        uint128 & operator ^= (const uint128 & b) throw ();

        // Inline simple operators
        inline const uint128 & operator + () const throw () { return *this; };

        // Rest of inline operators
        inline uint128 & operator -= (const uint128 & b) throw () {
            return *this += (-b);
        };
        inline uint128 & operator /= (const uint128 & b) throw () {
            uint128 dummy;
            *this = this->div (b, dummy);
            return *this;
        };
        inline uint128 & operator %= (const uint128 & b) throw () {
            this->div (b, *this);
            return *this;
        };

        // Common methods
        unsigned int toUint () const throw () {
            return (unsigned int) this->lo; };
        int64u toUint64 () const throw () {
            return (int64u) this->lo; };
        const char * toString (unsigned int radix = 10) const throw ();
        float toFloat () const throw ();
        double toDouble () const throw ();
        long double toLongDouble () const throw ();

        // Arithmetic methods
        uint128  div (const uint128 &, uint128 &) const throw ();

        // Bit operations
        bool    bit (unsigned int n) const throw ();
        void    bit (unsigned int n, bool val) throw ();
}
#ifdef __GNUC__
    __attribute__ ((__aligned__ (16), __packed__))
#endif
;


// GLOBAL OPERATORS

bool operator <  (const uint128 & a, const uint128 & b) throw ();
bool operator == (const uint128 & a, const uint128 & b) throw ();
bool operator || (const uint128 & a, const uint128 & b) throw ();
bool operator && (const uint128 & a, const uint128 & b) throw ();

// GLOBAL OPERATOR INLINES

inline uint128 operator + (const uint128 & a, const uint128 & b) throw () {
    return uint128 (a) += b; };
inline uint128 operator - (const uint128 & a, const uint128 & b) throw () {
    return uint128 (a) -= b; };
inline uint128 operator * (const uint128 & a, const uint128 & b) throw () {
    return uint128 (a) *= b; };
inline uint128 operator / (const uint128 & a, const uint128 & b) throw () {
    return uint128 (a) /= b; };
inline uint128 operator % (const uint128 & a, const uint128 & b) throw () {
    return uint128 (a) %= b; };

inline uint128 operator >> (const uint128 & a, unsigned int n) throw () {
    return uint128 (a) >>= n; };
inline uint128 operator << (const uint128 & a, unsigned int n) throw () {
    return uint128 (a) <<= n; };

inline uint128 operator & (const uint128 & a, const uint128 & b) throw () {
    return uint128 (a) &= b; };
inline uint128 operator | (const uint128 & a, const uint128 & b) throw () {
    return uint128 (a) |= b; };
inline uint128 operator ^ (const uint128 & a, const uint128 & b) throw () {
    return uint128 (a) ^= b; };

inline bool operator >  (const uint128 & a, const uint128 & b) throw () {
    return   b < a; };
inline bool operator <= (const uint128 & a, const uint128 & b) throw () {
    return !(b < a); };
inline bool operator >= (const uint128 & a, const uint128 & b) throw () {
    return !(a < b); };
inline bool operator != (const uint128 & a, const uint128 & b) throw () {
    return !(a == b); };


// MISC

typedef uint128 __uint128;

typedef uint128 int128u;
} //NameSpace

#endif
