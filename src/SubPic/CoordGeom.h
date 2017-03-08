/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2016 see Authors.txt
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

#include <cmath>
#include <array>

class Vector
{
public:
    float x, y, z;

    Vector() { x = y = z = 0.0f; }
    Vector(float x, float y, float z);
    void Set(float x, float y, float z);

    Vector Normal(const Vector& a, const Vector& b) const;
    float Angle(const Vector& a, const Vector& b) const;
    float Angle(const Vector& a) const;
    void Angle(float& u, float& v) const;   // returns spherical coords in radian, -M_PI_2 <= u <= M_PI_2, -M_PI <= v <= M_PI
    Vector Angle() const;                   // does like prev., returns 'u' in 'ret.x', and 'v' in 'ret.y'

    Vector Unit() const;
    Vector& Unitalize();
    float Length() const;
    float Sum() const;                      // x + y + z
    float CrossSum() const;                 // xy + xz + yz
    Vector Cross() const;                   // xy, xz, yz
    Vector Pow(float exp) const;

    Vector& Min(const Vector& a);
    Vector& Max(const Vector& a);
    Vector Abs() const;

    Vector Reflect(const Vector& n) const;
    Vector Refract(const Vector& n, float nFront, float nBack, float* nOut = nullptr) const;
    Vector Refract2(const Vector& n, float nFrom, float nTo, float* nOut = nullptr) const;

    Vector operator - () const;
    float& operator [](size_t i);

    float operator | (const Vector& v) const;   // dot
    Vector operator % (const Vector& v) const;  // cross

    bool operator == (const Vector& v) const;
    bool operator != (const Vector& v) const;

    Vector operator + (float d) const;
    Vector operator + (const Vector& v) const;
    Vector operator - (float d) const;
    Vector operator - (const Vector& v) const;
    Vector operator * (float d) const;
    Vector operator * (const Vector& v) const;
    Vector operator / (float d) const;
    Vector operator / (const Vector& v) const;
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
    Ray(const Vector& p, const Vector& d);
    void Set(const Vector& p, const Vector& d);

    float GetDistanceFrom(const Ray& r) const;      // r = plane
    float GetDistanceFrom(const Vector& v) const;   // v = point

    Vector operator [](float t) const;
};

class XForm
{
    class Matrix
    {
    public:
        std::array<std::array<float, 4>, 4> mat;

        Matrix();

        Matrix operator * (const Matrix& m) const;
        Matrix& operator *= (Matrix& m);
        bool operator == (const Matrix& m) const;
    } m;

    bool m_isWorldToLocal;

public:
    XForm() : m_isWorldToLocal(false) {}
    XForm(const Ray& r, const Vector& s, bool isWorldToLocal = true);

    void operator *= (const Vector& s);         // scale
    void operator += (const Vector& t);         // translate
    void operator <<= (const Vector& r);        // rotate

    void operator /= (const Vector& s);         // scale
    void operator -= (const Vector& t);         // translate
    void operator >>= (const Vector& r);        // rotate

    bool operator == (const XForm& x) const;    // compare
    bool operator != (const XForm& x) const;

    //  transformations
    Vector operator < (const Vector& n) const;  // normal
    Vector operator << (const Vector& v) const; // vector
    Ray operator << (const Ray& r) const;       // ray
};
