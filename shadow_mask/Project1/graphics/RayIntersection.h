//
// Name :         RayIntersection.h
// Description :  Header for CRayIntersection
//                Polygon ray intersection support class/DLL.
//                Implements kdTree fast ray intersection algorithm.
// Author :       Charles B. Owen
// Version :       3-13-07 2.00 Moved to DLL
//                 4-11-07 2.01 Fixed problems related to coincident vertices
//

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _RAYINTERSECTION_H
#define _RAYINTERSECTION_H

#ifdef LIBRIDLL
#define LIBRIEXPORT  __declspec( dllexport )
#else
#define LIBRIEXPORT  __declspec( dllimport )
#endif

#include <list>
#include <vector>

#include "GrPoint.h"

#if !defined(GRPOINT_VERSION_MAJOR) || GRPOINT_VERSION_MAJOR != 1 || GRPOINT_VERSION_MINOR != 2
#error GrPoint.h version 1.02 is required
#endif

// 
// To use:
//
// 1.  Call Initialize() to clear the system.
// 2.  Call Material() to set a pointer to the current material property
// 3.  Add polygons to the system:
//     A.  Call PolygonBegin()
//     B.  Call Vertex() to add vertices for the polygon
//         Call Texture() to specify a texture for the polygon
//         Call Normal() to specify a normal for the polygon
//         Call TexVertex() to specify a vertex for the polygon
//     C.  Call PolygonEnd()
// 4.  Call LoadingComplete()
// 5.  Call Intersect() to test for intersections
// 6.  Call IntersectInfo() to get intersection information for rendering
//



// Anonymous references so you can make these anything you want.
class CGrTexture;
class CGrMaterial;

// Anonymous reference to the class that does all of the actual work
class CRayIntersectionD;

class CRay
{
public:
    CRay(const CGrPoint &o, const CGrPoint &d) {m_o=o;  m_d=d;}
    CRay() {}
    CRay(const CRay &r) {m_o = r.m_o; m_d = r.m_d;}

    const CGrPoint &Origin() const {return m_o;}
    const double Origin(int d) const {return m_o[d];}
    const CGrPoint &Direction() const {return m_d;}
    const double Direction(int d) const {return m_d[d];}
    CRay &operator=(const CRay &r) {m_o = r.m_o; m_d = r.m_d; return *this;}
    CGrPoint PointOnRay(double t) const {return m_o + m_d * t;}

private:
    CGrPoint    m_o;
    CGrPoint    m_d;
};

class LIBRIEXPORT CRayIntersection  
{
public:
	CRayIntersection();
	virtual ~CRayIntersection();

    void Initialize();
	void LoadingComplete();

    // Polygon insertion
	void PolygonBegin();
	void PolygonEnd();

    // Generic insertion routines
	void Material(CGrMaterial *p_material);
	void Vertex(const CGrPoint &p_vertex);
	void TexVertex(const CGrPoint &p_tvertex);
	void Normal(const CGrPoint &p_normal);
	void Texture(CGrTexture *p_texture);

    // Parameter routines
    double SetIntersectionCost(double c);
    double GetIntersectionCost() const;
    double SetTraverseCost(double c);
    double GetTraverseCost() const;
    int SetMaxDepth(int m);
    int GetMaxDepth() const;
    int SetMinLeaf(int m);
    int GetMinLeaf() const;

    enum ObjectType {POLYGON, OTHER};

    // This is a generic superclass for any type of 
    // object we may compute a ray intersection on.
    class Object
    {
    public:
        virtual ObjectType Type() const = 0;
    };

    bool Intersect(const CRay &p_ray, double p_maxt, const Object *p_ignore, 
       const Object *&p_object, double &p_t, CGrPoint &p_intersect);
    void IntersectInfo(const CRay &p_ray, const Object *p_object, double p_t, 
                      CGrPoint &p_normal, CGrMaterial *&p_material, 
                      CGrTexture *&p_texture, CGrPoint &p_texcoord) const; 

    void SaveStats();

private:
    CRayIntersectionD *ri;
};

#endif

#pragma comment(lib, "graphics/libRayIntersection.lib")