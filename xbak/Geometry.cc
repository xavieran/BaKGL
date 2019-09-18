/*
 * This file is part of xBaK.
 *
 * xBaK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * xBaK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xBaK.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) Guido de Jong <guidoj@users.sf.net>
 */

#include "Geometry.h"

bool  Angle::m_initialized = false;
float Angle::m_cosTbl[ANGLE_SIZE];
float Angle::m_sinTbl[ANGLE_SIZE];
float Angle::m_tanTbl[ANGLE_SIZE];

Angle::Angle ( const int a )
: m_angle(a & ANGLE_MASK)
{
    if (!m_initialized)
    {
        for (unsigned int i = 0; i < ANGLE_SIZE; i++)
        {
            m_cosTbl[i] = cos((float)i * PI2 / (float)ANGLE_SIZE);
            m_sinTbl[i] = sin((float)i * PI2 / (float)ANGLE_SIZE);
            m_tanTbl[i] = tan((float)i * PI2 / (float)ANGLE_SIZE);
        }
        m_initialized = true;
    }
}

Angle::~Angle()
{
}

int Angle::Get() const
{
    return m_angle;
}

void Angle::Set ( const int a )
{
    m_angle = a & ANGLE_MASK;
}

float Angle::GetCos() const
{
    return m_cosTbl[m_angle];
}

float Angle::GetSin() const
{
    return m_sinTbl[m_angle];
}

float Angle::GetTan() const
{
    return m_tanTbl[m_angle];
}

Angle& Angle::operator= ( const Angle &a )
{
    m_angle = a.m_angle;
    return *this;
}

Angle& Angle::operator+= ( const Angle &a )
{
    m_angle = (m_angle + a.m_angle) & ANGLE_MASK;
    return *this;
}

Angle& Angle::operator-= ( const Angle &a )
{
    m_angle = (m_angle - a.m_angle) & ANGLE_MASK;
    return *this;
}

Angle Angle::operator+ ( const Angle &a )
{
    return Angle(m_angle + a.m_angle);
}

Angle Angle::operator- ( const Angle &a )
{
    return Angle(m_angle - a.m_angle);
}

bool Angle::operator== ( const Angle &a ) const
{
    return m_angle == a.m_angle;
}

bool Angle::operator!= ( const Angle &a ) const
{
    return m_angle != a.m_angle;
}

bool Angle::operator< ( const Angle &a ) const
{
    return m_angle < a.m_angle;
}

bool Angle::operator> ( const Angle &a ) const
{
    return m_angle > a.m_angle;
}

bool Angle::operator<= ( const Angle &a ) const
{
    return m_angle <= a.m_angle;
}

bool Angle::operator>= ( const Angle &a ) const
{
    return m_angle >= a.m_angle;
}

Vector2D::Vector2D()
        : m_x(0)
        , m_y(0)
{
}

Vector2D::Vector2D(const int x, const int y)
        : m_x(x)
        , m_y(y)
{
}

Vector2D::Vector2D(const Vector2D &p)
        : m_x(p.m_x)
        , m_y(p.m_y)
{
}

Vector2D::~Vector2D()
{
}

Vector2D& Vector2D::operator=(const Vector2D &p)
{
    m_x = p.m_x;
    m_y = p.m_y;
    return *this;
}

Vector2D& Vector2D::operator+=(const Vector2D &p)
{
    m_x += p.m_x;
    m_y += p.m_y;
    return *this;
}

Vector2D& Vector2D::operator-=(const Vector2D &p)
{
    m_x -= p.m_x;
    m_y -= p.m_y;
    return *this;
}

Vector2D& Vector2D::operator*=(const int f)
{
    m_x *= f;
    m_y *= f;
    return *this;
}

Vector2D& Vector2D::operator/=(const int f)
{
    m_x /= f;
    m_y /= f;
    return *this;
}

Vector2D& Vector2D::operator*=(const float f)
{
    m_x = (int)((float)m_x * f);
    m_y = (int)((float)m_y * f);
    return *this;
}

Vector2D& Vector2D::operator/=(const float f)
{
    m_x = (int)((float)m_x / f);
    m_y = (int)((float)m_y / f);
    return *this;
}

Vector2D Vector2D::operator+(const Vector2D &p)
{
    return Vector2D(m_x + p.m_x, m_y + p.m_y);
}

Vector2D Vector2D::operator-(const Vector2D &p)
{
    return Vector2D(m_x - p.m_x, m_y - p.m_y);
}

Vector2D Vector2D::operator*(const int f)
{
    return Vector2D(m_x * f, m_y * f);
}

Vector2D Vector2D::operator/(const int f)
{
    return Vector2D(m_x / f, m_y / f);
}

Vector2D Vector2D::operator*(const float f)
{
    return Vector2D((int)((float)m_x * f), (int)((float)m_y * f));
}

Vector2D Vector2D::operator/(const float f)
{
    return Vector2D((int)((float)m_x / f), (int)((float)m_y / f));
}

bool Vector2D::operator==(const Vector2D &p) const
{
    return (m_x == p.m_x) && (m_y == p.m_y);
}

bool Vector2D::operator!=(const Vector2D &p) const
{
    return (m_x != p.m_x) || (m_y != p.m_y);
}

bool Vector2D::operator<(const Vector2D &p) const
{
    return ((m_x < p.m_x) && (m_y <= p.m_y)) ||
           ((m_y < p.m_y) && (m_x <= p.m_x));
}

bool Vector2D::operator>(const Vector2D &p) const
{
    return ((m_x > p.m_x) && (m_y >= p.m_y)) ||
           ((m_y > p.m_y) && (m_x >= p.m_x));
}

bool Vector2D::operator<=(const Vector2D &p) const
{
    return (m_x <= p.m_x) && (m_y <= p.m_y);
}

bool Vector2D::operator>=(const Vector2D &p) const
{
    return (m_x >= p.m_x) && (m_y >= p.m_y);
}

int Vector2D::GetX() const
{
    return m_x;
}

int Vector2D::GetY() const
{
    return m_y;
}

void Vector2D::SetX(const int x)
{
    m_x = x;
}

void Vector2D::SetY(const int y)
{
    m_y = y;
}

unsigned int Vector2D::GetRho() const
{
    return (unsigned int)sqrt(((float)m_x * (float)m_x) + ((float)m_y * (float)m_y));
}

int Vector2D::GetTheta() const
{
    if (m_x == 0)
    {
        if (m_y >= 0)
        {
            return ANGLE_SIZE / 4;
        }
        else
        {
            return - ANGLE_SIZE / 4;
        }
    }
    else
    {
        int angle = (int)((atan((float)m_y / (float)m_x) / PI2) * (float)ANGLE_SIZE);
        if (m_x > 0)
        {
            return angle & ANGLE_MASK;
        }
        else
        {
            return (angle + ANGLE_SIZE / 2) & ANGLE_MASK;
        }
    }
}


Vector3D::Vector3D()
        : m_x(0)
        , m_y(0)
        , m_z(0)
{
}

Vector3D::Vector3D(const int x, const int y, const int z)
        : m_x(x)
        , m_y(y)
        , m_z(z)
{
}

Vector3D::Vector3D(const Vector3D &p)
        : m_x(p.m_x)
        , m_y(p.m_y)
        , m_z(p.m_z)
{
}

Vector3D::Vector3D(const Vector2D &p)
    : m_x(p.GetX())
    , m_y(p.GetY())
    , m_z(0)
{
}

Vector3D::~Vector3D()
{
}

Vector3D& Vector3D::operator=(const Vector3D &p)
{
    m_x = p.m_x;
    m_y = p.m_y;
    m_z = p.m_z;
    return *this;
}

Vector3D& Vector3D::operator+=(const Vector3D &p)
{
    m_x += p.m_x;
    m_y += p.m_y;
    m_z += p.m_z;
    return *this;
}

Vector3D& Vector3D::operator-=(const Vector3D &p)
{
    m_x -= p.m_x;
    m_y -= p.m_y;
    m_z -= p.m_z;
    return *this;
}

Vector3D& Vector3D::operator=(const Vector2D &p)
{
    m_x = p.GetX();
    m_y = p.GetY();
    m_z = 0;
    return *this;
}

Vector3D& Vector3D::operator+=(const Vector2D &p)
{
    m_x += p.GetX();
    m_y += p.GetY();
    return *this;
}

Vector3D& Vector3D::operator-=(const Vector2D &p)
{
    m_x -= p.GetX();
    m_y -= p.GetY();
    return *this;
}

Vector3D& Vector3D::operator*=(const int f)
{
    m_x *= f;
    m_y *= f;
    m_z *= f;
    return *this;
}

Vector3D& Vector3D::operator/=(const int f)
{
    m_x /= f;
    m_y /= f;
    m_z /= f;
    return *this;
}

Vector3D& Vector3D::operator*=(const float f)
{
    m_x = (int)((float)m_x * f);
    m_y = (int)((float)m_y * f);
    m_z = (int)((float)m_z * f);
    return *this;
}

Vector3D& Vector3D::operator/=(const float f)
{
    m_x = (int)((float)m_x / f);
    m_y = (int)((float)m_y / f);
    m_z = (int)((float)m_z / f);
    return *this;
}

Vector3D Vector3D::operator+(const Vector3D &p)
{
    return Vector3D(m_x + p.m_x, m_y + p.m_y, m_z + p.m_z);
}

Vector3D Vector3D::operator-(const Vector3D &p)
{
    return Vector3D(m_x - p.m_x, m_y - p.m_y, m_z - p.m_z);
}

Vector3D Vector3D::operator+(const Vector2D &p)
{
    return Vector3D(m_x + p.GetX(), m_y + p.GetY(), m_z);
}

Vector3D Vector3D::operator-(const Vector2D &p)
{
    return Vector3D(m_x - p.GetX(), m_y - p.GetY(), m_z);
}

Vector3D Vector3D::operator*(const int f)
{
    return Vector3D(m_x * f, m_y * f, m_z * f);
}

Vector3D Vector3D::operator/(const int f)
{
    return Vector3D(m_x / f, m_y / f, m_z / f);
}

Vector3D Vector3D::operator*(const float f)
{
    return Vector3D((int)((float)m_x * f), (int)((float)m_y * f), (int)((float)m_z * f));
}

Vector3D Vector3D::operator/(const float f)
{
    return Vector3D((int)((float)m_x / f), (int)((float)m_y / f), (int)((float)m_z / f));
}

bool Vector3D::operator==(const Vector3D &p) const
{
    return (m_x == p.m_x) && (m_y == p.m_y) && (m_z == p.m_z);
}

bool Vector3D::operator!=(const Vector3D &p) const
{
    return (m_x != p.m_x) || (m_y != p.m_y) || (m_z != p.m_z);
}

bool Vector3D::operator<(const Vector3D &p) const
{
    return ((m_x < p.m_x) && (m_y <= p.m_y) && (m_z <= p.m_z)) ||
           ((m_y < p.m_y) && (m_z <= p.m_z) && (m_x <= p.m_x)) ||
           ((m_z < p.m_z) && (m_x <= p.m_x) && (m_y <= p.m_y));
}

bool Vector3D::operator>(const Vector3D &p) const
{
    return ((m_x > p.m_x) && (m_y >= p.m_y) && (m_z >= p.m_z)) ||
           ((m_y > p.m_y) && (m_z >= p.m_z) && (m_x >= p.m_x)) ||
           ((m_z > p.m_z) && (m_x >= p.m_x) && (m_y >= p.m_y));
}

bool Vector3D::operator<=(const Vector3D &p) const
{
    return (m_x <= p.m_x) && (m_y <= p.m_y) && (m_z <= p.m_z);
}

bool Vector3D::operator>=(const Vector3D &p) const
{
    return (m_x >= p.m_x) && (m_y >= p.m_y) && (m_z >= p.m_z);
}

int Vector3D::GetX() const
{
    return m_x;
}

int Vector3D::GetY() const
{
    return m_y;
}

int Vector3D::GetZ() const
{
    return m_z;
}

void Vector3D::SetX(const int x)
{
    m_x = x;
}

void Vector3D::SetY(const int y)
{
    m_y = y;
}

void Vector3D::SetZ(const int z)
{
    m_z = z;
}

unsigned int Vector3D::GetRho() const
{
    return (unsigned int)sqrt(((float)m_x * (float)m_x) + ((float)m_y * (float)m_y) + ((float)m_z * (float)m_z));
}

int Vector3D::GetTheta() const
{
    if (m_x == 0)
    {
        if (m_y >= 0)
        {
            return ANGLE_SIZE / 4;
        }
        else
        {
            return - ANGLE_SIZE / 4;
        }
    }
    else
    {
        int angle = (int)((atan((float)m_y / (float)m_x) / PI2) * (float)ANGLE_SIZE);
        if (m_x > 0)
        {
            return angle & ANGLE_MASK;
        }
        else
        {
            return (angle + ANGLE_SIZE / 2) & ANGLE_MASK;
        }
    }
}


Rectangle::Rectangle(const int x, const int y, const int w, const int h)
        : m_x(x)
        , m_y(y)
        , m_width(w)
        , m_height(h)
{
}

Rectangle::Rectangle(const Rectangle &r)
        : m_x(r.m_x)
        , m_y(r.m_y)
        , m_width(r.m_width)
        , m_height(r.m_height)
{
}

Rectangle::~Rectangle()
{
}

Rectangle& Rectangle::operator=(const Rectangle &r)
{
    m_x = r.m_x;
    m_y = r.m_y;
    m_width = r.m_width;
    m_height = r.m_height;
    return *this;
}

bool Rectangle::operator==(const Rectangle &r)
{
    return (m_x == r.m_x) && (m_y == r.m_y) && (m_width == r.m_width) && (m_height == r.m_height);
}

bool Rectangle::operator!=(const Rectangle &r)
{
    return (m_x != r.m_x) || (m_y != r.m_y) || (m_width != r.m_width) || (m_height != r.m_height);
}

bool Rectangle::operator<(const Rectangle &r)
{
    return (m_x < r.m_x) ||
           ((m_x == r.m_x) && (m_y < r.m_y)) ||
            ((m_x == r.m_x) && (m_y == r.m_y) && ((m_width * m_height) < (r.m_width * r.m_height)));
}

bool Rectangle::operator>(const Rectangle &r)
{
    return (m_x > r.m_x) ||
            ((m_x == r.m_x) && (m_y > r.m_y)) ||
            ((m_x == r.m_x) && (m_y == r.m_y) && ((m_width * m_height) > (r.m_width * r.m_height)));
}

bool Rectangle::operator<=(const Rectangle &r)
{
    return (m_x <= r.m_x) ||
            ((m_x == r.m_x) && (m_y <= r.m_y)) ||
            ((m_x == r.m_x) && (m_y == r.m_y) && ((m_width * m_height) <= (r.m_width * r.m_height)));
}

bool Rectangle::operator>=(const Rectangle &r)
{
    return (m_x >= r.m_x) ||
            ((m_x == r.m_x) && (m_y >= r.m_y)) ||
            ((m_x == r.m_x) && (m_y == r.m_y) && ((m_width * m_height) >= (r.m_width * r.m_height)));
}

int Rectangle::GetXPos() const
{
    return m_x;
}

int Rectangle::GetYPos() const
{
    return m_y;
}

int Rectangle::GetXCenter() const
{
    return m_x + m_width / 2;
}

int Rectangle::GetYCenter() const
{
    return m_y + m_height / 2;
}

int Rectangle::GetWidth() const
{
    return m_width;
}

int Rectangle::GetHeight() const
{
    return m_height;
}

void Rectangle::SetXPos(const int x)
{
    m_x = x;
}

void Rectangle::SetYPos(const int y)
{
    m_y = y;
}

void Rectangle::SetWidth(const int w)
{
    m_width = w;
}

void Rectangle::SetHeight(const int h)
{
    m_height = h;
}

bool Rectangle::IsInside(const Vector2D &p) const
{
    return ((m_x <= p.GetX()) && (p.GetX() < m_x + m_width) &&
            (m_y <= p.GetY()) && (p.GetY() < m_y + m_height));
}
