//
// Name :         GrVector.h
// Description :  Classes and operators for homogeneous coordinates.
// Author :       Charles B. Owen
//
// Notice :       This class has no associated .cpp file.  All functions are inline.
//
#pragma once

#if !defined(LibGrafx)
#define LibGrafx
#endif

#if !defined(_GRVector_H)
#define _GRVector_H

#ifndef NOOPENGL
#include <GL/gl.h>
#endif

#include <cmath>

//! Class that contains a 4 element vector of doubles.

//! The CGrVector class is used to store a 3D homogeneous coordinate
//! or vector and contains functions to support normal vector
//! mathematical operations.
//! 
//! \par
//! The CGrTransform and CGrVector classes are available from the
//! <a href="https://www.cse.msu.edu/~cse472/secure/cgrtransform.html">
//! CGrTransform and CGrVector page.</a>
//! 
//! \par
//! See also \ref VecTranGlobals for global functions that operate
//! on CGrVector and CGrTransform objects.
//!
//! \version 1.00 02-03-2001 Declared version number
//! \version 1.01 03-23-2004 Visual Studio 2003 bug fixes, NOOPENGL option.
//! \version 1.02 03-11-2007 Allows for direct access to X, Y, Z, W.
//! Added MemberMultiply3. Added several additional set functions.
//! \version 1.03 01-22-2011 Change from CGrPoint to CGrVector
//! \version 1.04 01-28-2012 Documentation added.

class LibGrafx CGrVector
{
public:
    //! Default constructor. Initializes the vector to a zero vector.
    CGrVector() {m[0] = m[1] = m[2] = m[3] = 0;}

    //! Constructor that initializes the vector to x,y,z and optional w values.
    /*! \param x X value.
        \param y Y value.
        \param z Optional Z value. Defaults to 0 if not supplied.
        \param w Optional W value. Defaults to 1 is not supplied. */
    CGrVector(double x, double y, double z=0, double w=1.) {m[0] = x;  m[1] = y;  m[2] = z;  m[3] = w;}

    //! Constructor that initializes the vector from a 4 element array of floats.
    /*! This function will copy the values from a float array into the vector. It 
        copies all 4 values, not just 3, so the array must have 4 values and the
        w value should be reasonable for this coordinate or vector.
        \param p Pointer to a 4 element array of floats. */
    CGrVector(const float *p) {m[0] = p[0];  m[1] = p[1];  m[2] = p[2];  m[3] = p[3];}

    //! Constructor that initializes the vector from a 4 element array of doubles.
    /*! This function will copy the values from a double array into the vector. It 
        copies all 4 values, not just 3, so the array must have 4 values and the
        w value should be reasonable for this coordinate or vector.
        \param p Pointer to a 4 element array of doubles. */
    CGrVector(const double *p) {m[0] = p[0];  m[1] = p[1];  m[2] = p[2];  m[3] = p[3];}

    //! Copy constructor.
    CGrVector(const CGrVector &p) {m[0]=p.m[0];  m[1]=p.m[1];  m[2]=p.m[2];  m[3]=p.m[3];} 

    //! Assignment operators.
    CGrVector &operator=(const CGrVector &p) {m[0]=p.m[0];  m[1]=p.m[1];  m[2]=p.m[2];  m[3]=p.m[3]; return *this;}

    //! [in,out] Access the value of X.
    double &X() {return m[0];}

    //! [in,out] Access the value of Y.
    double &Y() {return m[1];}

    //! [in,out] Access the value of Z.
    double &Z() {return m[2];}

    //! [in,out] Access the value of W.
    double &W() {return m[3];}

    //! Access the value of X
    const double &X() const {return m[0];}

    //! Access the value of Y
    const double &Y() const {return m[1];}

    //! Access the value of Z
    const double &Z() const {return m[2];}

    //! Access the value of W
    const double &W() const {return m[3];}

    //! Set the value of X
    double X(double p) {return m[0] = p;}

    //! Set the value of Y
    double Y(double p) {return m[1] = p;}

    //! Set the value of Z
    double Z(double p) {return m[2] = p;}

    //! Set the value of W
    double W(double p) {return m[3] = p;}

    //! Set the value of the vector.
    /*! \param x X value.
        \param y Y value.
        \param z Optional Z value. Defaults to 0 if not supplied.
        \param w Optional W value. Defaults to 1 is not supplied. 
        \code 
            v.Set(12, 1, 9, 0);
        \endcode
    */
    void Set(double x, double y, double z=0.0, double w=1.) {m[0] = x;  m[1] = y;  m[2] = z;  m[3] = w;}

    //! Set the vector from a 4 element array of floats.
    /*! This function will copy the values from a float array into the vector. It 
        copies all 4 values, not just 3, so the array must have 4 values and the
        w value should be reasonable for this coordinate or vector.
        \param p Pointer to a 4 element array of floats. */
    void Set(const double *p) {m[0] = p[0];  m[1] = p[1];  m[2] = p[2];  m[3] = p[3];}

    //! Set the vector from a 4 element array of doubles.
    /*! This function will copy the values from a double array into the vector. It 
        copies all 4 values, not just 3, so the array must have 4 values and the
        w value should be reasonable for this coordinate or vector.
        \param p Pointer to a 4 element array of doubles. */
    void Set(const float *p) {m[0] = p[0];  m[1] = p[1];  m[2] = p[2];  m[3] = p[3];}

    //! Compute the 2D Perp operator on X, Y.
    /*! Z is set to 0 and W is set to 1. */
    CGrVector Perp2() const {return CGrVector(-m[1], m[0], 0);}

#ifndef NOOPENGL
    //! Executes a glVertex4dv call based on the CGrVector values.
    void glVertex() const {glVertex4dv(m);}

    //! Executes a glNormal call based on the CGrVector values.
    void glNormal() const {glNormal3dv(m);}

    //! Executes a glTexVertex call based on the CGrVector values.
    void glTexVertex() const {glTexCoord2dv(m);}
#endif

    //! Subtraction operator. Returns this - b (4D).
    CGrVector operator -(const CGrVector &b) const {return CGrVector(m[0]-b.m[0], m[1]-b.m[1], m[2]-b.m[2], m[3]-b.m[3]);}

    //! Unary minus operator. Negates all values.
    CGrVector operator -() const {return CGrVector(-m[0], -m[1], -m[2], -m[3]);}

    //! Addition operator. Returns this+b (4D).
    CGrVector operator +(const CGrVector &b) const {return CGrVector(m[0]+b.m[0], m[1]+b.m[1], m[2]+b.m[2], m[3]+b.m[3]);}

    //! -= operator. Subtracts b from this vector (4D).
    CGrVector &operator -=(const CGrVector &b) {m[0]-=b.m[0]; m[1]-=b.m[1]; m[2]-=b.m[2]; m[3]-=b.m[3]; return *this;}

    //! += operator. Adds b to this vector (4D) .
    CGrVector &operator +=(const CGrVector &b) {m[0]+=b.m[0]; m[1]+=b.m[1]; m[2]+=b.m[2]; m[3]+=b.m[3]; return *this;}

    //! /= operator. Divides values by a scalar (4D).
    CGrVector &operator /=(double n) {m[0]/=n; m[1]/=n; m[2]/=n; m[3]/=n; return *this;}

    //! *= operator. Multiplies values by a scalar (4D).
    CGrVector operator *(double n) const {return CGrVector(m[0]*n, m[1]*n, m[2]*n, m[3]*n);}

    //! / operator. Returns this/n, where n is a scalar (4D).
    CGrVector operator /(double n) const {return CGrVector(m[0]/n, m[1]/n, m[2]/n, m[3]/n);}

    //! Operator that allows a CGrVector object to be treated as a const array
    /*! This operator allows the values in const CGrVector object to be accessed
        using the normal array notation and to be passed to functions as a 
        const array.
        \code
        const CGrVector v(1, 0, 0);
        double x = v[0];        // Accesses X
        glVertex4dv(v);
        \endcode */
    operator const double *() const {return m;}

    //! Operator that allows a CGrVector object to be treated as an array
    /*! This operator allows the values in CGrVector object to be accessed
        using the normal array notation and to be passed to functions as an         
        array.
        \code
        CGrVector v(1, 0, 0);
        double x = v[0];        // Accesses X
        v[2] = 7;               // Sets Y
        \endcode */
    operator double *() {return m;}

    //! Returns the length of the vector as a 4D vector. Only uses X,Y,Z, W.
    double Length() const {return sqrt(m[0]*m[0] + m[1]*m[1] + m[2]*m[2] + m[3]*m[3]);}

    //! Returns the squared length of the vector as a 4D vector. Only uses X,Y,Z, W.
    double LengthSquared() const {return (m[0]*m[0] + m[1]*m[1] + m[2]*m[2] + m[3]*m[3]);}

    //! Normalize as a 4D vector. Affects X, Y, Z, W.
    void Normalize() {double l = Length();  m[0] /= l;  m[1] /= l;  m[2] /= l;  m[3] /= l;}

    //! Compute this += p * w, where p is a vector and w is a scalar (4D).
    /*! This function is used to create weighted sums of other vector objects in 4D.
        \param p Vector to add to this one.
        \param w Scalar weight that the vector p is multiplied by prior to the addition. */
    void WeightedAdd(const CGrVector &p, double w) {m[0] += p.m[0] * w;  m[1] += p.m[1] * w;  m[2] += p.m[2] * w;  m[3] += p.m[3] * w;} 

    //! Member-wise multiply by another vector (4D).
    /*! Computes this[i] *= p[i] for each vector value. 
        \param p Vector to multiply by */
    CGrVector &MemberMultiply(const CGrVector &p) {m[0] *= p.m[0];  m[1] *= p.m[1];  m[2] *= p.m[2];  m[3] *= p.m[3]; return *this;}

    // 3D-only variations

    //! Normalize as a 3D vector. Only affects X, Y, Z.
    void Normalize3() {double l = Length3();  m[0] /= l;  m[1] /= l;  m[2] /= l;}

    //! Returns the length of the vector as a 3D vector. Only uses X,Y,Z.
    double Length3() const {return sqrt(m[0]*m[0] + m[1]*m[1] + m[2]*m[2]);}

    //! Returns the square of the length of the vector as a 3D vector. Only uses X,Y,Z.
    double LengthSquared3() const {return m[0]*m[0] + m[1]*m[1] + m[2]*m[2];}

    //! For each value in the vector, set to the minimum of this[i] or p[i].
    /*! \param p Vector of values to compare with this vector. */
    void Minimize(const CGrVector &p) {for(int i=0;  i<4;  i++) m[i] = p.m[i] < m[i] ? p.m[i] : m[i];}

    //! For each value in the vector, set to the maximum of this[i] or p[i].
    /*! \param p Vector of values to compare with this vector. */
    void Maximize(const CGrVector &p) {for(int i=0;  i<4;  i++) m[i] = p.m[i] > m[i] ? p.m[i] : m[i];}

    //! Compute this += p * w, where p is a vector and w is a scalar (3D).
    /*! This function is used to create weighted sums of other vector objects in 3D.
        \param p Vector to add to this one.
        \param w Scalar weight that the vector p is multiplied by prior to the addition. */
    void WeightedAdd3(const CGrVector &p, double w) {m[0] += p.m[0] * w;  m[1] += p.m[1] * w;  m[2] += p.m[2] * w;} 

    //! Member-wise multiply by another vector.
    /*! Computes this[i] *= p[i] for each vector value. 
        \param p Vector to multiply by */
    CGrVector &MemberMultiply3(const CGrVector &p) {m[0] *= p.m[0];  m[1] *= p.m[1];  m[2] *= p.m[2];  return *this;}

private:
    double m[4];
};

/*! \defgroup VecTranGlobals CGrVector and CGrTransform global functions
*/


//! Compute a normalized version of a vector (4D).
/*! \ingroup VecTranGlobals
    \param p Vector to be normalized */
inline CGrVector Normalize(const CGrVector &p)
{
   CGrVector a(p);
   a.Normalize();
   return a;
}

//! Compute the dot product of two vectors (4D).
/*! \ingroup VecTranGlobals
    The result is a dot b.
    \param a Vector a.
    \param b Vector b.*/
inline double Dot(const CGrVector &a, const CGrVector &b)
{
   return a.X() * b.X() + a.Y() * b.Y() + a.Z() * b.Z() + a.W() * b.W();
}

//! Compute a normalized version of a vector (3D).
/*! \ingroup VecTranGlobals
    \param p Vector to be normalized */
inline CGrVector Normalize3(const CGrVector &p)
{
   CGrVector a(p);
   a.Normalize3();
   return a;
}

//! Compute the cross product of two vectors (3D).
/*! \ingroup VecTranGlobals
    The result is a cross b.
    \param a Vector a.
    \param b Vector b.*/
inline CGrVector Cross(const CGrVector &a, const CGrVector &b)
{
   return CGrVector(a.Y()*b.Z() - a.Z()*b.Y(), a.Z()*b.X() - a.X()*b.Z(), a.X()*b.Y() - a.Y()*b.X(), 0);
}

//! Compute the dot product of two vectors (2D).
/*! \ingroup VecTranGlobals
    The result is a dot b.
    \param a Vector a.
    \param b Vector b.*/
inline double Dot2(const CGrVector &a, const CGrVector &b)
{
   return a.X() * b.X() + a.Y() * b.Y();
}

//! Compute the dot product of two vectors (3D).
/*! \ingroup VecTranGlobals
    The result is a dot b.
    \param a Vector a.
    \param b Vector b.*/
inline double Dot3(const CGrVector &a, const CGrVector &b)
{
   return a.X() * b.X() + a.Y() * b.Y() + a.Z() * b.Z();
}

//! Compute the distance betwen two vectors (3D).
/*! \ingroup VecTranGlobals
    The scalar result is |a - b|.
    \param a Vector a.
    \param b Vector b.*/
inline double Distance(const CGrVector &a, const CGrVector &b)
{
    return sqrt( (a.X() - b.X()) * (a.X() - b.X()) +
                (a.Y() - b.Y()) * (a.Y() - b.Y()) +
                (a.Z() - b.Z()) * (a.Z() - b.Z()));
}

#endif
