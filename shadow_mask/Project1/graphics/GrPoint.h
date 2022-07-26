//
// Name :         GrPoint.h
// Description :  Classes and operators for homogeneous coordinates.
// Author :       Charles B. Owen
// Version :      1.00  2-03-01 Declared version number
//                1.01  3-23-04 Visual Studio 2003 bug fixes
//                              NOOPENGL option.
//                1.02  3-11-07 CGrPoint allows for direct access to X, Y, Z, W
//                              Added CGrPoint::MemberMultiply3
//                              Added several additional set functions
//
// Notice :       This class has no associated .cpp file.  All functions are inline.
//

#if !defined(_GRPOINT_H)
#define _GRPOINT_H

#define GRPOINT_VERSION_MAJOR 1
#define GRPOINT_VERSION_MINOR 2

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef NOOPENGL
#include <GL/gl.h>
#endif

#include <cmath>

// class CGrPoint
// Class for a graphics point or vector.  
// I'm going to make this 4D since we'll need
// that later, anyway.

class CGrPoint
{
public:
    CGrPoint() {}
    CGrPoint(double x, double y, double z=0, double w=1.) {m[0] = x;  m[1] = y;  m[2] = z;  m[3] = w;}
    CGrPoint(const float *p) {m[0] = p[0];  m[1] = p[1];  m[2] = p[2];  m[3] = p[3];}
    CGrPoint(const double *p) {m[0] = p[0];  m[1] = p[1];  m[2] = p[2];  m[3] = p[3];}
    CGrPoint(const CGrPoint &p) {m[0]=p.m[0];  m[1]=p.m[1];  m[2]=p.m[2];  m[3]=p.m[3];} 

    CGrPoint &operator=(const CGrPoint &p) {m[0]=p.m[0];  m[1]=p.m[1];  m[2]=p.m[2];  m[3]=p.m[3]; return *this;}

    double &X() {return m[0];}
    double &Y() {return m[1];}
    double &Z() {return m[2];}
    double &W() {return m[3];}
    const double &X() const {return m[0];}
    const double &Y() const {return m[1];}
    const double &Z() const {return m[2];}
    const double &W() const {return m[3];}

    double X(double p) {return m[0] = p;}
    double Y(double p) {return m[1] = p;}
    double Z(double p) {return m[2] = p;}
    double W(double p) {return m[3] = p;}

    void Set(double x, double y, double z, double w=1.) {m[0] = x;  m[1] = y;  m[2] = z;  m[3] = w;}
    void Set(const double *p) {m[0] = p[0];  m[1] = p[1];  m[2] = p[2];  m[3] = p[3];}
    void Set(const float *p) {m[0] = p[0];  m[1] = p[1];  m[2] = p[2];  m[3] = p[3];}

    CGrPoint Perp2() const {return CGrPoint(-m[1], m[0], 0);}

#ifndef NOOPENGL
    void glVertex() const {glVertex4dv(m);}
    void glNormal() const {glNormal3dv(m);}
    void glTexVertex() const {glTexCoord2dv(m);}
#endif

    CGrPoint operator -(const CGrPoint &b) const {return CGrPoint(m[0]-b.m[0], m[1]-b.m[1], m[2]-b.m[2], m[3]-b.m[3]);}
    CGrPoint operator -() const {return CGrPoint(-m[0], -m[1], -m[2], -m[3]);}
    CGrPoint operator +(const CGrPoint &b) const {return CGrPoint(m[0]+b.m[0], m[1]+b.m[1], m[2]+b.m[2], m[3]+b.m[3]);}
    CGrPoint &operator -=(const CGrPoint &b) {m[0]-=b.m[0]; m[1]-=b.m[1]; m[2]-=b.m[2]; m[3]-=b.m[3]; return *this;}
    CGrPoint &operator +=(const CGrPoint &b) {m[0]+=b.m[0]; m[1]+=b.m[1]; m[2]+=b.m[2]; m[3]+=b.m[3]; return *this;}
    CGrPoint operator *(double n) const {return CGrPoint(m[0]*n, m[1]*n, m[2]*n, m[3]*n);}
    CGrPoint operator /(double n) const {return CGrPoint(m[0]/n, m[1]/n, m[2]/n, m[3]/n);}
    operator const double *() const {return m;}
    operator double *() {return m;}
    void Normalize3() {double l = Length3();  m[0] /= l;  m[1] /= l;  m[2] /= l;}
    double Length3() const {return sqrt(m[0]*m[0] + m[1]*m[1] + m[2]*m[2]);}
    double LengthSquared3() const {return m[0]*m[0] + m[1]*m[1] + m[2]*m[2];}

    void Minimize(const CGrPoint &p) {for(int i=0;  i<4;  i++) m[i] = p.m[i] < m[i] ? p.m[i] : m[i];}
    void Maximize(const CGrPoint &p) {for(int i=0;  i<4;  i++) m[i] = p.m[i] > m[i] ? p.m[i] : m[i];}

    void WeightedAdd3(const CGrPoint &p, double w) {m[0] += p.m[0] * w;  m[1] += p.m[1] * w;  m[2] += p.m[2] * w;} 
    CGrPoint &MemberMultiply3(const CGrPoint &p) {m[0] *= p.m[0];  m[1] *= p.m[1];  m[2] *= p.m[2];  return *this;}

private:
    double m[4];
};

// Normalize a vector. 
inline CGrPoint Normalize3(const CGrPoint &p)
{
   CGrPoint a(p);
   a.Normalize3();
   return a;
}

// Cross product
inline CGrPoint Cross3(const CGrPoint &a, const CGrPoint &b)
{
   return CGrPoint(a.Y()*b.Z() - a.Z()*b.Y(), a.Z()*b.X() - a.X()*b.Z(), a.X()*b.Y() - a.Y()*b.X(), 0);
}

// Dot product
inline double Dot3(const CGrPoint &a, const CGrPoint &b)
{
   return a.X() * b.X() + a.Y() * b.Y() + a.Z() * b.Z();
}

// Dot product
inline double Dot2(const CGrPoint &a, const CGrPoint &b)
{
   return a.X() * b.X() + a.Y() * b.Y();
}

// Distance between two points
inline double Distance(const CGrPoint& a, const CGrPoint& b)
{
    return sqrt((a.X() - b.X()) * (a.X() - b.X()) +
        (a.Y() - b.Y()) * (a.Y() - b.Y()) +
        (a.Z() - b.Z()) * (a.Z() - b.Z()));
};

#endif
