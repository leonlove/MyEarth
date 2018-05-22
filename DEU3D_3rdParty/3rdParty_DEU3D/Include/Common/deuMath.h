#ifndef DEU_MATH_H_8876D9A5_8788_4D1A_BD38_52DED9DEA7B8_INCLUDE
#define DEU_MATH_H_8876D9A5_8788_4D1A_BD38_52DED9DEA7B8_INCLUDE
#include "Export.h"
#include <float.h>
#include <math.h>
#include <vector>

#pragma warning(disable:4251)

namespace cmm{ namespace math
{

#ifdef max
    #undef max
#endif

#ifdef min
    #undef min
#endif

const double PI   = 3.1415926535897932384626533;
const double PI_2 = 1.57079632679489661923132665;

inline bool floatEqual(float flt1, float flt2, float fltEpsilon = 1e-6f)
{
    const float fltD = abs(flt1 - flt2);
    return (fltD <= fltEpsilon);
}

inline bool floatEqual(double flt1, double flt2, double fltEpsilon = 1e-9f)
{
    const double fltD = abs(flt1 - flt2);
    return (fltD <= fltEpsilon);
}

inline float Degrees2Radians(float angle) { return angle * (float)PI / 180.0f; }
inline double Degrees2Radians(double angle) { return angle * PI / 180.0; }

inline float Radians2Degrees(float angle) { return angle * 180.0f / (float)PI; }
inline double Radians2Degrees(double angle) { return angle * 180.0 / PI; }

inline double logsig(double dblX)
{
    const double dblExp = exp(-dblX);
    const double dblRes = 1.0 / (1.0 + dblExp);
    return dblRes;
}


inline double sinPress(double dblX, unsigned nTime)
{
    double dblRetVal = dblX;
    for(unsigned n = 0u; n < nTime; n++)
    {
        const double dbl1 = dblX * PI - PI_2;
        const double dbl2 = sin(dbl1);
        dblRetVal = (dbl2 + 1.0) * 0.5;
        dblX = dblRetVal;
    }
    return dblRetVal;
}


template<typename T>
inline T clampAbove(T v,T minimum) { return v<minimum?minimum:v; }

template<typename T>
inline T clampBelow(T v,T maximum) { return v>maximum?maximum:v; }

template<typename T>
inline T clampBetween(T v,T minimum, T maximum)
  { return clampBelow(clampAbove(v,minimum),maximum); }


template<typename T>
inline bool isValueInArea(const T &Left, const T &Right, const T &Value)
{
	if(Left < Right)
	{
		if(Value < Left)	return false;
		if(Value > Right)	return false;
	}
	else
	{
		if(Value < Right)	return false;
		if(Value > Left)	return false;
	}
	return true;
}


template<typename T>
class Point2
{
public:
    Point2(void)
    {
        _v[0] = 0;
        _v[1] = 0;
    }
    Point2(const Point2 &obj)
    {
        _v[0] = obj._v[0];
        _v[1] = obj._v[1];
    }
    Point2(T x, T y)
    {
        _v[0] = x;
        _v[1] = y;
    }

public:
    T  &x(void)         {   return _v[0];   }
    T  &y(void)         {   return _v[1];   }
    const T  &x(void) const   {   return _v[0];   }
    const T  &y(void) const   {   return _v[1];   }
    const Point2 &operator=(const Point2 &point)
    {
        if(this == &point)  return *this;
        _v[0] = point._v[0];
        _v[1] = point._v[1];
        return *this;
    }
    bool operator==(const Point2 &point) const
    {
        if(this == &point)  return true;

        if(!floatEqual(_v[0], point._v[0])) return false;
        if(!floatEqual(_v[1], point._v[1])) return false;

        return true;
    }
    bool operator!=(const Point2 &point) const
    {
        return !operator==(point);
    }

    double operator * (const Point2& rhs) const
    {
        return _v[0] * rhs._v[0] + _v[1] * rhs._v[1];
    }

    Point2 operator * (T rhs) const
    {
        return Point2(_v[0] * rhs, _v[1] * rhs);
    }

    const Point2& operator *= (T rhs)
    {
        _v[0] *= rhs;
        _v[1] *= rhs;
        return *this;
    }

    Point2 operator / (T rhs) const
    {
        return Point2(_v[0] / rhs, _v[1] / rhs);
    }

    /** Unary divide by scalar. */
    const Point2& operator /= (T rhs)
    {
        _v[0] /= rhs;
        _v[1] /= rhs;
        return *this;
    }

    Point2 operator + (const Point2& rhs) const
    {
        return Point2(_v[0] + rhs._v[0], _v[1] + rhs._v[1]);
    }

    const Point2& operator += (const Point2& rhs)
    {
        _v[0] += rhs._v[0];
        _v[1] += rhs._v[1];
        return *this;
    }

    Point2 operator - (const Point2& rhs) const
    {
        return Point2(_v[0] - rhs._v[0], _v[1] - rhs._v[1]);
    }

    const Point2& operator -= (const Point2& rhs)
    {
        _v[0] -= rhs._v[0];
        _v[1] -= rhs._v[1];
        return *this;
    }

    Point2 operator - () const
    {
        return Point2 (-_v[0], -_v[1]);
    }

    T length() const
    {
        return (T)sqrt( _v[0] * _v[0] + _v[1]*_v[1] );
    }

    T normalize()
    {
        T norm = Point2::length();
        if (norm > 0.0)
        {
            T inv = 1.0 / norm;
            _v[0] *= inv;
            _v[1] *= inv;
        }
        return( norm );
    }

    void set(T x, T y)
    {
        _v[0] = x;
        _v[1] = y;
    }

public:
    T   _v[2];
};


template<typename T>
class Point3
{
public:
    Point3(void)
    {
        _v[0] = 0;
        _v[1] = 0;
        _v[2] = 0;
    }
    Point3(const Point3 &obj)
    {
        _v[0] = obj._v[0];
        _v[1] = obj._v[1];
        _v[2] = obj._v[2];
    }
    Point3(T x, T y, T z)
    {
        _v[0] = x;
        _v[1] = y;
        _v[2] = z;
    }

public:
    T  &x(void)         {   return _v[0];   }
    T  &y(void)         {   return _v[1];   }
    T  &z(void)         {   return _v[2];   }
    const T  &x(void) const   {   return _v[0];   }
    const T  &y(void) const   {   return _v[1];   }
    const T  &z(void) const   {   return _v[2];   }
    const Point3 &operator=(const Point3 &point)
    {
        if(this == &point)  return *this;
        _v[0] = point._v[0];
        _v[1] = point._v[1];
        _v[2] = point._v[2];
        return *this;
    }
    bool operator==(const Point3 &point) const
    {
        if(this == &point)  return true;

        if(!floatEqual(_v[0], point._v[0])) return false;
        if(!floatEqual(_v[1], point._v[1])) return false;
        if(!floatEqual(_v[2], point._v[2])) return false;

        return true;
    }
    bool operator!=(const Point3 &point) const
    {
        return !operator==(point);
    }
    Point3 operator - (const Point3& rhs) const
    {
        return Point3(_v[0] - rhs._v[0], _v[1] - rhs._v[1], _v[2] - rhs._v[2]);
    }
    Point3 operator + (const Point3& rhs) const
    {
        return Point3(_v[0] + rhs._v[0], _v[1] + rhs._v[1], _v[2] + rhs._v[2]);
    }
    Point3 operator * (T rhs) const
    {
        return Point3(_v[0] * rhs, _v[1] * rhs, _v[2] * rhs);
    }

    const Point3& operator *= (T rhs)
    {
        _v[0] *= rhs;
        _v[1] *= rhs;
        _v[2] *= rhs;
        return *this;
    }

    Point3 operator / (T rhs) const
    {
        return Point3(_v[0] / rhs, _v[1] / rhs, _v[2] / rhs);
    }

    /** Unary divide by scalar. */
    const Point3& operator /= (T rhs)
    {
        _v[0] /= rhs;
        _v[1] /= rhs;
        _v[2] /= rhs;
        return *this;
    }

    const Point3& operator += (const Point3& rhs)
    {
        _v[0] += rhs._v[0];
        _v[1] += rhs._v[1];
        _v[2] += rhs._v[2];
        return *this;
    }

    const Point3& operator -= (const Point3& rhs)
    {
        _v[0] -= rhs._v[0];
        _v[1] -= rhs._v[1];
        _v[2] -= rhs._v[2];
        return *this;
    }

    Point3 operator - () const
    {
        return Point3 (-_v[0], -_v[1], -_v[2]);
    }

    double length() const
    {
        return sqrt( _v[0] * _v[0] + _v[1]*_v[1] + _v[2]*_v[2]);
    }

    T normalize()
    {
        T norm = Point3::length();
        if (norm > 0.0)
        {
            T inv = 1.0 / norm;
            _v[0] *= inv;
            _v[1] *= inv;
        }
        return( norm );
    }

    void set(T x, T y, T z)
    {
        _v[0] = x;
        _v[1] = y;
        _v[2] = z;
    }
public:
    T   _v[3];
};


template<typename T>
class Box2
{
public:
    Box2(void)  {   m_bValid = false;   }
    Box2(const Box2 &obj)
    {
        m_bValid = obj.m_bValid;
        m_points[0] = obj.m_points[0];
        m_points[1] = obj.m_points[1];
    }
    Box2(const Point2<T> &point1, const Point2<T> &point2)
    {
        set(point1, point2);
    }

public:
    enum CornerType
    {
        LeftBottom      = 0,
        RightBottom     = 1,
        LeftTop         = 2,
        RightTop        = 3
    };
    bool contain(const Point2<T> &point)
    {
        if(!m_bValid)   return false;

        if(point._v[0] < m_points[0]._v[0] || point._v[0] > m_points[1]._v[0])
        {
            return true;
        }

        if(point._v[1] < m_points[0]._v[1] || point._v[1] > m_points[1]._v[1])
        {
            return true;
        }

        return false;
    }

    bool contain(const Box2 &bb) const
    {
        if(!m_bValid)   return false;

        if(m_points[0].x() > bb.m_points[1].x() || m_points[0].y() > bb.m_points[1].y() ||
           m_points[1].x() < bb.m_points[0].x() || m_points[1].y() < bb.m_points[0].y())
        {
            return false;
        }

        return true;
    }

    Point2<T> center(void) const
    {
        Point2<T>   point;
        if(!m_bValid)   return point;

        const T x = m_points[0].x() + m_points[1].x();
        const T y = m_points[0].y() + m_points[1].y();
        point.set(x * 0.5, y * 0.5);
        return point;
    }

    Point2<T> corner(CornerType eType) const
    {
        Point2<T>   point;
        if(!m_bValid)   return point;

        if(eType & 1)
        {
            point.x() = m_points[1].x();
        }
        else
        {
            point.x() = m_points[0].x();
        }

        if(eType & 2)
        {
            point.y() = m_points[1].y();
        }
        else
        {
            point.y() = m_points[0].y();
        }

        return point;
    }
    void set(const Point2<T> &point0, const Point2<T> &point1)
    {
        m_bValid = true;
        m_points[0].set(std::min(point0.x(), point1.x()), std::min(point0.y(), point1.y()));
        m_points[1].set(std::max(point0.x(), point1.x()), std::max(point0.y(), point1.y()));
    }
    const Point2<T> &point0(void) const {   return m_points[0]; }
    const Point2<T> &point1(void) const {   return m_points[1]; }
    const T width(void) const           {   return m_points[1].x() - m_points[0].x();       }
    const T height(void) const          {   return m_points[1].y() - m_points[0].y();       }
    const T left(void) const            {   return m_points[0].x();                         }
    const T right(void) const           {   return m_points[1].x();                         }
    const T bottom(void) const          {   return m_points[0].y();                         }
    const T top(void) const             {   return m_points[1].y();                         }

    const Box2 &operator=(const Box2 &obj)
    {
        if(this == &obj)    return *this;
        m_points[0] = obj.m_points[0];
        m_points[1] = obj.m_points[1];
        m_bValid    = obj.m_bValid;
        return *this;
    }
    bool operator==(const Box2 &obj) const
    {
        if(this == *obj)    return true;
        if(m_points[0] != obj.m_points[0])  return false;
        if(m_points[1] != obj.m_points[1])  return false;
        if(m_bValid    != obj.m_bValid)     return false;
        return true;
    }
    bool operator!=(const Box2 &obj) const
    {
        return !operator==(obj);
    }
    void expandBy(const Point2<T> &point)
    {
        if(!m_bValid)
        {
            m_points[0] = m_points[1] = point;
            m_bValid = true;
            return;
        }

        if(point.x() < m_points[0].x())
        {
            m_points[0]._v[0] = point.x();
        }
        else if(point.x() > m_points[1].x())
        {
            m_points[1]._v[0] = point.x();
        }

        if(point.y() < m_points[0].y())
        {
            m_points[0]._v[1] = point.y();
        }
        else if(point.y() > m_points[1].y())
        {
            m_points[1]._v[1] = point.y();
        }
    }
    void expandBy(const Box2 &obj)
    {
        if(!obj.m_bValid)   return;
        expandBy(obj.point0());
        expandBy(obj.point1());
    }
private:
    Point2<T>   m_points[2];
    bool        m_bValid;
};


template<typename T>
class Box3
{
public:
    Box3(void)  {   }
    Box3(const Box3 &obj)
    {
        m_points[0] = obj.m_points[0];
        m_points[1] = obj.m_points[1];
    }
    Box3(const Point3<T> &point1, const Point3<T> &point2)
    {
        m_points[0] = point1;
        m_points[1] = point2;
    }

public:
    enum CornerType
    {
        NearLeftBottom      = 0,
        NearRightBottom     = 1,
        NearLeftTop         = 2,
        NearRightTop        = 3,
        FarLeftBottom       = 4,
        FarRightBottom      = 5,
        FarLeftTop          = 6,
        FarRightTop         = 7
    };
    Point3<T> corner(CornerType eType) const
    {
        Point3<T>   point;
        if(eType & 1)
        {
            point.x() = std::max(m_points[0].x(), m_points[1].x());
        }
        else
        {
            point.x() = std::min(m_points[0].x(), m_points[1].x());
        }

        if(eType & 2)
        {
            point.y() = std::max(m_points[0].y(), m_points[1].y());
        }
        else
        {
            point.y() = std::min(m_points[0].y(), m_points[1].y());
        }

        if(eType & 4)
        {
            point.z() = std::max(m_points[0].z(), m_points[1].z());
        }
        else
        {
            point.z() = std::min(m_points[0].z(), m_points[1].z());
        }

        return point;
    }
    const T width(void) const           {   return abs(m_points[0].x() - m_points[1].x());   }
    const T height(void) const          {   return abs(m_points[0].y() - m_points[1].y());   }
    const T depth(void) const           {   return abs(m_points[0].z() - m_points[1].z());   }
    const Box3 &operator=(const Box3 &obj)
    {
        if(this == &obj)    return *this;
        m_points[0] = obj.m_points[0];
        m_points[1] = obj.m_points[1];
        return *this;
    }
    bool operator==(const Box3 &obj) const
    {
        if(this == *obj)    return true;
        if(m_points[0] != obj.m_points[0])  return false;
        if(m_points[1] != obj.m_points[1])  return false;
        if(m_points[2] != obj.m_points[2])  return false;
        return true;
    }
    bool operator!=(const Box3 &obj) const
    {
        return !operator==(obj);
    }

private:
    Point3<T>   m_points[2];
};


template<typename T>
class Circle
{
public:
    Circle(void){ m_radius = -1.0;}
    Circle(const Circle &obj) : m_ptCenter(obj.m_ptCenter), m_radius(obj.m_radius)
    {
    }
    Circle(const Point2<T> &center, T radius) : m_ptCenter(center), m_radius(radius)
    {
    }

    const T          getRadius(void) const      {   return m_radius;    }
    void             setRadius(T radius)        {   m_radius = radius;  }
    const Point2<T> &getCenter(void) const      {   return m_ptCenter;  }
    void             setCenter(const Point2<T> &center) {   m_ptCenter = center;    }
    bool             isValid(void) const        {   return m_radius >= 0.0;         }
    void             expandBy(const Point2<T> &point);
    void             expandBy(const Circle &obj);

    const Circle &operator=(const Circle &obj)  {   m_ptCenter = obj.m_ptCenter;    m_radius = obj.m_radius;    }
    bool  operator==(const Circle &obj) const
    {
        if(m_ptCenter != obj.m_ptCenter)            return false;
        if(!floatEqual(m_radius, obj.m_radius))     return false;
        return true;
    }
    bool  operator!=(const Circle &obj) const
    {
        return !operator==(obj);
    }

private:
    Point2<T>   m_ptCenter;
    T           m_radius;
};


template<typename T>
class Sphere
{
public:
    Sphere(void){ m_radius = -1.0;}
    Sphere(const Sphere &obj) : m_ptCenter(obj.m_ptCenter), m_radius(obj.m_radius)
    {
    }
    Sphere(const Point3<T> &center, T radius) : m_ptCenter(center), m_radius(radius)
    {
    }

    const T          getRadius(void) const      {   return m_radius;    }
    void             setRadius(T radius)        {   m_radius = radius;  }
    const Point3<T> &getCenter(void) const      {   return m_ptCenter;    }
    void             setCenter(const Point3<T> &center) {   m_ptCenter = center;    }
    bool             isValid(void) const        {   return m_radius >= 0.0;         }
    void             expandBy(const Point3<T> &point);
    void             expandBy(const Sphere &obj);

    const Sphere &operator=(const Sphere &obj)
    {
        if(this == &obj)    return *this;
        m_ptCenter = obj.m_ptCenter;
        m_radius = obj.m_radius;
        return *this;
    }
    bool  operator==(const Sphere &obj) const
    {
        if(m_ptCenter != obj.m_ptCenter)            return false;
        if(!floatEqual(m_radius, obj.m_radius))     return false;
        return true;
    }
    bool  operator!=(const Sphere &obj) const
    {
        return !operator==(obj);
    }

protected:
    Point3<T>       m_ptCenter;
    T               m_radius;
};


template<typename T>
void Circle<T>::expandBy(const Point2<T> &point)
{
    if (isValid())
    {
        Point2<T> dv;

        dv.x() = point.x() - m_ptCenter.x();
        dv.y() = point.y() - m_ptCenter.y();

        const T r = sqrt(dv.x() * dv.x() + dv.y() * dv.y());

        if (r > m_radius)
        {
            const T dr = (r - m_radius) * 0.5;
            const T ratio = dr / r;

            m_ptCenter.x() += dv.x() * ratio;
            m_ptCenter.y() += dv.y() * ratio;

            m_radius += dr;
        }
    }
    else
    {
        m_ptCenter = point;
        m_radius = 0.0;
    }
}


template<typename T>
void Circle<T>::expandBy(const Circle &obj)
{
    // ignore operation if incomming BoundingSphere is invalid.
    if (!obj.isValid()) return;

    // This sphere is not set so use the inbound sphere
    if (!isValid())
    {
        m_ptCenter  = obj.m_ptCenter;
        m_radius    = obj.m_radius;
        return;
    }

    // Calculate d == The distance between the sphere centers   
    const Point3<T> vec(m_ptCenter.x() - obj.x(), m_ptCenter.y() - obj.y());
    const T d = sqrt(vec.x() * vec.x() + vec.y() * vec.y());

    // New sphere is already inside this one
    if ( d + obj.m_radius <= m_radius)
    {
        return;
    }

    //  New sphere completely contains this one 
    if ( d + m_radius <= obj.m_radius)
    {
        m_ptCenter = obj.m_ptCenter;
        m_radius = obj.m_radius;
        return;
    }


    // Build a new sphere that completely contains the other two:
    //
    // The center point lies halfway along the line between the furthest
    // points on the edges of the two spheres.
    //
    // Computing those two points is ugly - so we'll use similar triangles
    const T new_radius = (m_radius + d + sh.m_radius ) * 0.5;
    const T ratio = ( new_radius - m_radius ) / d ;

    m_ptCenter.x() += ( obj.m_ptCenter.x() - m_ptCenter.x() ) * ratio;
    m_ptCenter.y() += ( obj.m_ptCenter.y() - m_ptCenter.y() ) * ratio;

    m_radius = new_radius;
}


template<typename T>
void Sphere<T>::expandBy(const Point3<T> &point)
{
    if (isValid())
    {
        Point3<T> dv;

        dv.x() = point.x() - m_ptCenter.x();
        dv.y() = point.y() - m_ptCenter.y();
        dv.z() = point.z() - m_ptCenter.z();

        const T r = sqrt(dv.x() * dv.x() + dv.y() * dv.y() + dv.z() * dv.z());

        if (r > m_radius)
        {
            const T dr = (r - m_radius) * 0.5;
            const T ratio = dr / r;

            m_ptCenter.x() += dv.x() * ratio;
            m_ptCenter.y() += dv.y() * ratio;
            m_ptCenter.z() += dv.z() * ratio;

            m_radius += dr;
        }
    }
    else
    {
        m_ptCenter = point;
        m_radius = 0.0;
    }
}


template<typename T>
void Sphere<T>::expandBy(const Sphere &obj)
{
    // ignore operation if incomming BoundingSphere is invalid.
    if (!obj.isValid()) return;

    // This sphere is not set so use the inbound sphere
    if (!isValid())
    {
        m_ptCenter  = obj.m_ptCenter;
        m_radius    = obj.m_radius;
        return;
    }

    // Calculate d == The distance between the sphere centers   
    const Point3<T> vec(m_ptCenter.x() - obj.m_ptCenter.x(), m_ptCenter.y() - obj.m_ptCenter.y(), m_ptCenter.z() - obj.m_ptCenter.z());
    const T d = sqrt(vec.x() * vec.x() + vec.y() * vec.y() + vec.z() * vec.z());

    // New sphere is already inside this one
    if ( d + obj.m_radius <= m_radius)
    {
        return;
    }

    //  New sphere completely contains this one 
    if ( d + m_radius <= obj.m_radius)
    {
        m_ptCenter = obj.m_ptCenter;
        m_radius = obj.m_radius;
        return;
    }


    // Build a new sphere that completely contains the other two:
    //
    // The center point lies halfway along the line between the furthest
    // points on the edges of the two spheres.
    //
    // Computing those two points is ugly - so we'll use similar triangles
    const T new_radius = (m_radius + d + obj.m_radius ) * 0.5;
    const T ratio = ( new_radius - m_radius ) / d ;

    m_ptCenter.x() += ( obj.m_ptCenter.x() - m_ptCenter.x() ) * ratio;
    m_ptCenter.y() += ( obj.m_ptCenter.y() - m_ptCenter.y() ) * ratio;
    m_ptCenter.z() += ( obj.m_ptCenter.z() - m_ptCenter.z() ) * ratio;

    m_radius = new_radius;
}

typedef Point2<int>         Point2i;
typedef Point2<double>      Point2d;
typedef Point2<double>      Vector2d;

typedef Point3<int>         Point3i;
typedef Point3<double>      Point3d;
typedef Point3<double>      Vector3d;

typedef Box2<int>           Box2i;
typedef Box2<double>        Box2d;

typedef Box3<int>           Box3i;
typedef Box3<double>        Box3d;

typedef Circle<double>      Circled;
typedef Sphere<double>      Sphered;

class CM_EXPORT Polygon2
{
public:
    explicit Polygon2(void);
    Polygon2(const Polygon2 &val);
    explicit Polygon2(const std::vector<Point2d> &vertexs);
    virtual ~Polygon2(void);

public:
    const Polygon2 &operator=(const Polygon2 &val);

public:
    void        setVertexs(const std::vector<Point2d> &vertices);
    void        addVertex(const Point2d &point);
    void        clear(void);

    unsigned    getVerticesCount(void) const    {   return (unsigned)m_vecVertices.size(); }

    bool        containsPoint(const Point2d &point) const;
    double      area(void) const;
    Point2d     centroid(void) const;

    void        enlarge(double dblDist);

    // Modify
    void        insertVertexAfter(unsigned iInsertAfter, const Point2d &point);
    void        removeVertex(unsigned i);
    void        reverseOrder(void);
    unsigned    removeDegenerateVertexs(double dEpsilon = 0.0001f);

    bool        isValid(void) const;
    bool        isIntersectional(const Polygon2 &plg) const;
    bool        isConvex(void) const;
    const Point2d &getSafeVertex(unsigned index) const;
    void        setSafeVertex(unsigned index, const Point2d &point);

    double      pointDistance(const Point2d &ptTest) const;
    double      findNearestSegment(const Point2d &ptTest, Point2d &vtx0, Point2d &vtx1) const;

    Box2d getBound(void) const;

protected:
    std::vector<Point2d>        m_vecVertices;
};


double CM_EXPORT Point2LineDistance(const Point2d &ptTest, const Point2d &pointA, const Point2d &pointB, bool bSegment);


}}

#endif


