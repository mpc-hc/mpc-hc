/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <math.h>
#include <array>

#define EPSILON      (1e-7)
#define BIGNUMBER    (1e+9)
#define IsZero(d)    (fabs(d) < EPSILON)
#define Sgn(d)       (IsZero(d) ? 0 : (d) > 0 ? 1 : -1)
//#define SgnPow(d, p) (IsZero(d) ? 0 : (pow(fabs(d), p) * Sgn(d)))

class Vector
{
public:
    float x, y, z;

    Vector() { x = y = z = 0; }
    Vector(float x, float y, float z);
    void Set(float x, float y, float z);

    Vector Normal(Vector& a, Vector& b);
    float Angle(Vector& a, Vector& b);
    float Angle(const Vector& a);
    void Angle(float& u, float& v);     // returns spherical coords in radian, -M_PI_2 <= u <= M_PI_2, -M_PI <= v <= M_PI
    Vector Angle();                     // does like prev., returns 'u' in 'ret.x', and 'v' in 'ret.y'

    Vector Unit();
    Vector& Unitalize();
    float Length();
    float Sum();                        // x + y + z
    float CrossSum();                   // xy + xz + yz
    Vector Cross();                     // xy, xz, yz
    Vector Pow(float exp);

    Vector& Min(const Vector& a);
    Vector& Max(const Vector& a);
    Vector Abs();

    Vector Reflect(Vector& n);
    Vector Refract(Vector& n, float nFront, float nBack, float* nOut = nullptr);
    Vector Refract2(Vector& n, float nFrom, float nTo, float* nOut = nullptr);

    Vector operator - ();
    float& operator [](size_t i);

    float operator | (const Vector& v);     // dot
    Vector operator % (const Vector& v);    // cross

    bool operator == (const Vector& v) const;
    bool operator != (const Vector& v) const;

    Vector operator + (float d);
    Vector operator + (const Vector& v);
    Vector operator - (float d);
    Vector operator - (Vector& v);
    Vector operator * (float d);
    Vector operator * (const Vector& v);
    Vector operator / (float d);
    Vector operator / (const Vector& v);
    Vector& operator += (float d);
    Vector& operator += (const Vector& v);
    Vector& operator -= (float d);
    Vector& operator -= (Vector& v);
    Vector& operator *= (float d);
    Vector& operator *= (const Vector& v);
    Vector& operator /= (float d);
    Vector& operator /= (const Vector& v);

    template<typename T> static float DegToRad(T angle) { return (float)(angle * M_PI / 180); }
};

class Ray
{
public:
    Vector p, d;

    Ray() {}
    Ray(Vector& p, Vector& d);
    void Set(const Vector& p, const Vector& d);

    float GetDistanceFrom(Ray& r);          // r = plane
    float GetDistanceFrom(Vector& v);       // v = point

    Vector operator [](float t);
};

class XForm
{
    class Matrix
    {
    public:
        std::array<std::array<float, 4>, 4> mat;

        Matrix();

        Matrix operator * (const Matrix& m);
        Matrix& operator *= (Matrix& m);
        bool operator == (const Matrix& m) const;
    } m;

    bool m_isWorldToLocal;

public:
    XForm() : m_isWorldToLocal(false) {}
    XForm(Ray& r, Vector& s, bool isWorldToLocal = true);

    void operator *= (const Vector& s);      // scale
    void operator += (const Vector& t);      // translate
    void operator <<= (const Vector& r);     // rotate

    void operator /= (const Vector& s);      // scale
    void operator -= (Vector& t);            // translate
    void operator >>= (Vector& r);           // rotate

    bool operator == (const XForm& x) const; // compare
    bool operator != (const XForm& x) const;

    //  transformations
    Vector operator < (Vector& n);           // normal
    Vector operator << (const Vector& v);    // vector
    Ray operator << (Ray& r);                // ray
};
