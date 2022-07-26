//
// Name :         GrRenderer.h
// Description :  Header file for CGrRenderer, superclass for generic renderer.
//                See GrRenderer.cpp
// Author :       Charles B. Owen
//


#ifndef _GRRENDERER_H
#define _GRRENDERER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GrObject.h"
#include <vector>

class CGrRenderer  
{
public:
    virtual void RendererVertex(const CGrPoint &v);
    CGrRenderer();
    virtual ~CGrRenderer();

    // Standard renderer configuration functions
    void Perspective(double p_angle, double p_aspect, double p_near, double p_far);
    virtual void AddLight(double x, double y, double z, double w, 
        const float *ambient, const float *diffuse, const float *specular);
    virtual void AddLight(const CGrPoint &p_loc, 
        const float *p_ambient, const float *p_diffuse, const float *p_specular);
    virtual void LookAt(double ex, double ey, double ez, double cx, double cy, double cz, double ux, double uy, double uz);

    virtual void Clear() {m_lights.clear();}

    // The call to invoke the renderer
    bool Render(CGrPtr<CGrObject> &p_object);

    // The functions that make up the renderer
    // Some are abstract, others have default values
    virtual bool RendererStart();
    virtual bool RendererEnd();
    virtual void RendererBeginPolygon();
    virtual void RendererEndPolygon();
    virtual void RendererTexture(CGrTexture *p_texture);
    virtual void RendererNormal(const CGrPoint &n);
    virtual void RendererTexVertex(const CGrPoint &v);
    virtual void RendererPushMatrix();
    virtual void RendererPopMatrix();
    virtual void RendererRotate(double a, double x, double y, double z);
    virtual void RendererTranslate(double x, double y, double z);
    virtual void RendererTransform(const CGrTransform *p_transform);
    virtual void RendererMaterial(CGrMaterial *p_material);
    virtual void RendererColor(double *c);
    virtual void RendererSphere(const CGrPoint &center, double radius);
    virtual void RendererNormalize(bool);

    // Information necessary to describe a light
    struct Light
    {
        Light();

        CGrPoint m_pos;      // Where be the light?
        float    m_ambient[4];
        float    m_diffuse[4];
        float    m_specular[4];
    };

    // Parameter access functions
    double ProjectionAngle() const {return m_angle;}
    double ProjectionAspect() const {return m_aspect;}
    double NearClip() const {return m_near;}
    double FarClip() const {return m_far;}
    const CGrPoint &Eye() const {return m_eye;}
    const CGrPoint &Center() const {return m_center;}
    const CGrPoint &Up() const {return m_up;}
    int LightCnt() const {return int(m_lights.size());}
    const Light &GetLight(int n) const {return m_lights[n];}
    CGrTexture *PolyTexture() {return m_texture;}
    const std::list<CGrPoint> &PolyVertices() const {return m_polyvertex;}
    const std::list<CGrPoint> &PolyNormals() const {return m_polynormal;}
    const std::list<CGrPoint> &PolyTexVertices() const {return m_polytexture;}

private:
    double   m_angle;       // Projection angle (vertical)
    double   m_aspect;      // Aspect ratio
    double   m_near, m_far; // Clip planes

    // Lookat parameters
    CGrPoint m_eye;
    CGrPoint m_center;
    CGrPoint m_up;

    // Lights
    std::vector<Light> m_lights;

    // Current polygon definition information.
    CGrPtr<CGrTexture>   m_texture;
    std::list<CGrPoint>  m_polyvertex;
    std::list<CGrPoint>  m_polynormal;
    std::list<CGrPoint>  m_polytexture;

};

#endif // !defined(AFX_GRRENDERER_H__CFA4660A_883B_405D_B8D2_8DA9D471E66C__INCLUDED_)
