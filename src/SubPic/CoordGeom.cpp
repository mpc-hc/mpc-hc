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

#include "stdafx.h"
#include "CoordGeom.h"
#include "../DSUtil/DSUtil.h"
#include <cmath>

static bool IsZero(float d)
{
    return IsEqual(d, 0.0f);
}

//
// Vector
//

Vector::Vector(float _x, float _y, float _z)
    : x(_x), y(_y), z(_z)
{
}

void Vector::Set(float _x, float _y, float _z)
{
    x = _x;
    y = _y;
    z = _z;
}

float Vector::Length() const
{
    return sqrt(x * x + y * y + z * z);
}

float Vector::Sum() const
{
    return (x + y + z);
}

float Vector::CrossSum() const
{
    return (x * y + x * z + y * z);
}

Vector Vector::Cross() const
{
    return Vector(x * y, x * z, y * z);
}

Vector Vector::Pow(float exp) const
{
    return (exp == 0.0f ? Vector(1.0f, 1.0f, 1.0f) : exp == 1.0f ? *this : Vector(pow(x, exp), pow(y, exp), pow(z, exp)));
}

Vector Vector::Unit() const
{
    float l = Length();
    if (!l || l == 1.0f) {
        return *this;
    }
    return (*this * (1.0f / l));
}

Vector& Vector::Unitalize()
{
    return (*this = Unit());
}

Vector Vector::Normal(const Vector& a, const Vector& b) const
{
    return ((a - *this) % (b - a));
}

float Vector::Angle(const Vector& a, const Vector& b) const
{
    return (((a - *this).Unit()).Angle((b - *this).Unit()));
}

float Vector::Angle(const Vector& a) const
{
    float angle = *this | a;
    return ((angle > 1.0f) ? 0.0f : (angle < -1.0f) ? (float)M_PI : acos(angle));
}

void Vector::Angle(float& u, float& v) const
{
    Vector n = Unit();

    u = asin(n.y);

    if (IsZero(n.z)) {
        v = (float)M_PI_2 * SGN(n.x);
    } else if (n.z > 0) {
        v = atan(n.x / n.z);
    } else if (n.z < 0) {
        v = IsZero(n.x) ? (float)M_PI : ((float)M_PI * SGN(n.x) + atan(n.x / n.z));
    }
}

Vector Vector::Angle() const
{
    Vector ret;
    Angle(ret.x, ret.y);
    ret.z = 0;
    return ret;
}

Vector& Vector::Min(const Vector& a)
{
    x = (x < a.x) ? x : a.x;
    y = (y < a.y) ? y : a.y;
    z = (z < a.z) ? z : a.z;
    return *this;
}

Vector& Vector::Max(const Vector& a)
{
    x = (x > a.x) ? x : a.x;
    y = (y > a.y) ? y : a.y;
    z = (z > a.z) ? z : a.z;
    return *this;
}

Vector Vector::Abs() const
{
    return Vector(fabs(x), fabs(y), fabs(z));
}

Vector Vector::Reflect(const Vector& n) const
{
    return (n * ((-*this) | n) * 2 - (-*this));
}

Vector Vector::Refract(const Vector& N, float nFront, float nBack, float* nOut /*= nullptr*/) const
{
    Vector D = -*this;

    float N_dot_D = (N | D);
    float n = N_dot_D >= 0.0f ? (nFront / nBack) : (nBack / nFront);

    Vector cos_D = N * N_dot_D;
    Vector sin_T = (cos_D - D) * n;

    float len_sin_T = sin_T | sin_T;

    if (len_sin_T > 1.0f) {
        if (nOut) {
            *nOut = N_dot_D >= 0.0f ? nFront : nBack;
        }
        return this->Reflect(N);
    }

    float N_dot_T = (float)sqrt(1.0f - len_sin_T);
    if (N_dot_D < 0.0f) {
        N_dot_T = -N_dot_T;
    }

    if (nOut) {
        *nOut = N_dot_D >= 0.0f ? nBack : nFront;
    }

    return (sin_T - (N * N_dot_T));
}

Vector Vector::Refract2(const Vector& N, float nFrom, float nTo, float* nOut /*= nullptr*/) const
{
    Vector D = -*this;

    float N_dot_D = (N | D);
    float n = nFrom / nTo;

    Vector cos_D = N * N_dot_D;
    Vector sin_T = (cos_D - D) * n;

    float len_sin_T = sin_T | sin_T;

    if (len_sin_T > 1.0f) {
        if (nOut) {
            *nOut = nFrom;
        }
        return this->Reflect(N);
    }

    float N_dot_T = (float)sqrt(1.0f - len_sin_T);
    if (N_dot_D < 0.0f) {
        N_dot_T = -N_dot_T;
    }

    if (nOut) {
        *nOut = nTo;
    }

    return (sin_T - (N * N_dot_T));
}

float Vector::operator | (const Vector& v) const
{
    return (x * v.x + y * v.y + z * v.z);
}

Vector Vector::operator % (const Vector& v) const
{
    return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

float& Vector::operator [](size_t i)
{
    return (!i ? x : (i == 1) ? y : z);
}

Vector Vector::operator - () const
{
    return Vector(-x, -y, -z);
}

bool Vector::operator == (const Vector& v) const
{
    return (IsZero(x - v.x) && IsZero(y - v.y) && IsZero(z - v.z));
}

bool Vector::operator != (const Vector& v) const
{
    return !(*this == v);
}

Vector Vector::operator + (float d) const
{
    return Vector(x + d, y + d, z + d);
}

Vector Vector::operator + (const Vector& v) const
{
    return Vector(x + v.x, y + v.y, z + v.z);
}

Vector Vector::operator - (float d) const
{
    return Vector(x - d, y - d, z - d);
}

Vector Vector::operator - (const Vector& v) const
{
    return Vector(x - v.x, y - v.y, z - v.z);
}

Vector Vector::operator * (float d) const
{
    return Vector(x * d, y * d, z * d);
}

Vector Vector::operator * (const Vector& v) const
{
    return Vector(x * v.x, y * v.y, z * v.z);
}

Vector Vector::operator / (float d) const
{
    return Vector(x / d, y / d, z / d);
}

Vector Vector::operator / (const Vector& v) const
{
    return Vector(x / v.x, y / v.y, z / v.z);
}

Vector& Vector::operator += (float d)
{
    x += d;
    y += d;
    z += d;
    return *this;
}

Vector& Vector::operator += (const Vector& v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Vector& Vector::operator -= (float d)
{
    x -= d;
    y -= d;
    z -= d;
    return *this;
}

Vector& Vector::operator -= (Vector& v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

Vector& Vector::operator *= (float d)
{
    x *= d;
    y *= d;
    z *= d;
    return *this;
}

Vector& Vector::operator *= (const Vector& v)
{
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
}

Vector& Vector::operator /= (float d)
{
    x /= d;
    y /= d;
    z /= d;
    return *this;
}

Vector& Vector::operator /= (const Vector& v)
{
    x /= v.x;
    y /= v.y;
    z /= v.z;
    return *this;
}

//
// Ray
//

Ray::Ray(const Vector& _p, const Vector& _d)
    : p(_p)
    , d(_d)
{
}

void Ray::Set(const Vector& _p, const Vector& _d)
{
    p = _p;
    d = _d;
}

float Ray::GetDistanceFrom(const Ray& r) const
{
    float t = (d | r.d);
    if (IsZero(t)) {
        return -std::numeric_limits<float>::infinity();    // plane is parallel to the ray, return -infinite
    }
    return (((r.p - p) | r.d) / t);
}

float Ray::GetDistanceFrom(const Vector& v) const
{
    float t = ((v - p) | d) / (d | d);
    return ((p + d * t) - v).Length();
}

Vector Ray::operator [](float t) const
{
    return (p + d * t);
}

//
// XForm
//


XForm::XForm(const Ray& r, const Vector& s, bool isWorldToLocal /*= true*/)
{
    m_isWorldToLocal = isWorldToLocal;
    if (isWorldToLocal) {
        *this -= r.p;
        *this >>= r.d;
        *this /= s;

    } else {
        *this *= s;
        *this <<= r.d;
        *this += r.p;
    }
}

void XForm::operator *= (const Vector& v)
{
    Matrix s;
    s.mat[0][0] = v.x;
    s.mat[1][1] = v.y;
    s.mat[2][2] = v.z;
    m *= s;
}

void XForm::operator += (const Vector& v)
{
    Matrix t;
    t.mat[3][0] = v.x;
    t.mat[3][1] = v.y;
    t.mat[3][2] = v.z;
    m *= t;
}

void XForm::operator <<= (const Vector& v)
{
    Matrix x;
    x.mat[1][1] = cos(v.x);
    x.mat[1][2] = -sin(v.x);
    x.mat[2][1] = sin(v.x);
    x.mat[2][2] = cos(v.x);

    Matrix y;
    y.mat[0][0] = cos(v.y);
    y.mat[0][2] = -sin(v.y);
    y.mat[2][0] = sin(v.y);
    y.mat[2][2] = cos(v.y);

    Matrix z;
    z.mat[0][0] = cos(v.z);
    z.mat[0][1] = -sin(v.z);
    z.mat[1][0] = sin(v.z);
    z.mat[1][1] = cos(v.z);

    m = m_isWorldToLocal ? (m * y * x * z) : (m * z * x * y);
}

void XForm::operator /= (const Vector& v)
{
    Vector s;
    s.x = IsZero(v.x) ? 0.0f : 1.0f / v.x;
    s.y = IsZero(v.y) ? 0.0f : 1.0f / v.y;
    s.z = IsZero(v.z) ? 0.0f : 1.0f / v.z;
    *this *= s;
}

void XForm::operator -= (const Vector& v)
{
    *this += -v;
}

void XForm::operator >>= (const Vector& v)
{
    *this <<= -v;
}

Vector XForm::operator < (const Vector& n) const
{
    Vector ret;

    ret.x = n.x * m.mat[0][0] +
            n.y * m.mat[1][0] +
            n.z * m.mat[2][0];
    ret.y = n.x * m.mat[0][1] +
            n.y * m.mat[1][1] +
            n.z * m.mat[2][1];
    ret.z = n.x * m.mat[0][2] +
            n.y * m.mat[1][2] +
            n.z * m.mat[2][2];

    return ret;
}

Vector XForm::operator << (const Vector& v) const
{
    Vector ret;

    ret.x = v.x * m.mat[0][0] +
            v.y * m.mat[1][0] +
            v.z * m.mat[2][0] +
            m.mat[3][0];
    ret.y = v.x * m.mat[0][1] +
            v.y * m.mat[1][1] +
            v.z * m.mat[2][1] +
            m.mat[3][1];
    ret.z = v.x * m.mat[0][2] +
            v.y * m.mat[1][2] +
            v.z * m.mat[2][2] +
            m.mat[3][2];

    return ret;
}

Ray XForm::operator << (const Ray& r) const
{
    return Ray(*this << r.p, *this < r.d);
}

bool XForm::operator == (const XForm& x) const
{
    return m_isWorldToLocal == x.m_isWorldToLocal && m == x.m;
}

bool XForm::operator != (const XForm& x) const
{
    return !(*this == x);
}

//
// XForm::Matrix
//

XForm::Matrix::Matrix()
{
    mat[0][0] = 1.0f;
    mat[0][1] = 0.0f;
    mat[0][2] = 0.0f;
    mat[0][3] = 0.0f;
    mat[1][0] = 0.0f;
    mat[1][1] = 1.0f;
    mat[1][2] = 0.0f;
    mat[1][3] = 0.0f;
    mat[2][0] = 0.0f;
    mat[2][1] = 0.0f;
    mat[2][2] = 1.0f;
    mat[2][3] = 0.0f;
    mat[3][0] = 0.0f;
    mat[3][1] = 0.0f;
    mat[3][2] = 0.0f;
    mat[3][3] = 1.0f;
}

XForm::Matrix XForm::Matrix::operator * (const Matrix& m) const
{
    Matrix ret;

    for (ptrdiff_t i = 0; i < 4; i++) {
        for (ptrdiff_t j = 0; j < 4; j++) {
            ret.mat[i][j] = mat[i][0] * m.mat[0][j] +
                            mat[i][1] * m.mat[1][j] +
                            mat[i][2] * m.mat[2][j] +
                            mat[i][3] * m.mat[3][j];

            if (IsZero(ret.mat[i][j])) {
                ret.mat[i][j] = 0.0f;
            }
        }
    }

    return ret;
}

XForm::Matrix& XForm::Matrix::operator *= (XForm::Matrix& m)
{
    return (*this = *this * m);
}

bool XForm::Matrix::operator == (const XForm::Matrix& m) const
{
    return mat == m.mat;
}
