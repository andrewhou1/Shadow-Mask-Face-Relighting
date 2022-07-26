//
// Name :         GrRenderer.cpp
// Description :  Implementation of CGrRenderer, superclass for generic renderer.
//                This is a generic renderer that we can use for ray tracing
//                and other types of renderers.
// Author :       Charles B. Owen
//


#include "pch.h"
#include "GrRenderer.h"
#include "GrTexture.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CGrRenderer::CGrRenderer()
{
   m_angle = 25.;
   m_aspect = 1.;
   m_far = 1000.;
   m_near = 1.;
}

CGrRenderer::~CGrRenderer()
{

}


//
// Name :         CGrRenderer::Perspective()
// Description :  Configure the projection information for 
//                our renderer.
//

void CGrRenderer::Perspective(double p_angle, double p_aspect, double p_near, double p_far)
{
   m_angle = p_angle;
   m_aspect = p_aspect;
   m_far = p_far;
   m_near = p_near;
}


//
// Name :         CGrRenderer::LookAt()
// Description :  Set the look-at parameters for the view.
//
void CGrRenderer::LookAt(double ex, double ey, double ez, 
                         double cx, double cy, double cz, 
                         double ux, double uy, double uz)
{
   m_eye.Set(ex, ey, ez);
   m_center.Set(cx, cy, cz);
   m_up.Set(ux, uy, uz, 0.);
}


//
// Name :         CGrRenderer::Render() 
// Description :  Here's the workhorse function.  It causes the scene graph to be rendered.
//

bool CGrRenderer::Render(CGrPtr<CGrObject> &p_object)
{
    // Do anything we need to do before we render.
    RendererStart();

    // Do the actual rendering
    p_object->Render(this);

    // Any cleanup?
    RendererEnd();

    return true;
}

bool CGrRenderer::RendererStart()
{
   return true;
}

bool CGrRenderer::RendererEnd()
{
   return true;
}


//
// Name :         CGrRenderer::Light::Light()
// Description :  Set default light characteristics.
//

CGrRenderer::Light::Light()
{
   m_pos.Set(0, 0, 0);
   for(int i=0;  i<4;  i++)
   {
      m_ambient[i] = 0.f;
      m_diffuse[i] = 0.f;
      m_specular[i] = 0.f;
   }

}

//
// Name :         CGrRenderer::AddLight()
// Description :  Add a light to the list of lights.
// Note :         I've use a very basic set of light definitions.
//                You could add many more or override this function
//                with your own fancy lighting system.
//

void CGrRenderer::AddLight(const CGrPoint &p_loc, const float *p_ambient, 
                           const float *p_diffuse, const float *p_specular)
{
   Light light;
   light.m_pos = p_loc;
   
   for(int i=0;  i<4;  i++)
   {
      if(p_ambient)
         light.m_ambient[i] = p_ambient[i];
      if(p_diffuse)
         light.m_diffuse[i] = p_diffuse[i];
      if(p_specular)
         light.m_specular[i] = p_specular[i];
   }

   m_lights.push_back(light);
}


//
// Name :         CGrRenderer::AddLight()
// Description :  Alternative versions
//

void CGrRenderer::AddLight(double x, double y, double z, double w, const float *p_ambient, 
                           const float *p_diffuse, const float *p_specular)
{
   AddLight(CGrPoint(x, y, z, w), p_ambient, p_diffuse, p_specular);
}




//
// Name :         CGrRenderer::RendererBeginPolygon()
// Description :  Default behavior or beginning a polygon.
//

void CGrRenderer::RendererBeginPolygon()
{
   m_texture = NULL;
   m_polyvertex.clear();
   m_polynormal.clear();
   m_polytexture.clear();
}


void CGrRenderer::RendererEndPolygon()
{
}


void CGrRenderer::RendererTexture(CGrTexture *p_texture)
{
   m_texture = p_texture;
}

void CGrRenderer::RendererNormal(const CGrPoint &n)
{
   m_polynormal.push_back(n);
}

void CGrRenderer::RendererTexVertex(const CGrPoint &v)
{
   m_polytexture.push_back(v);
}

void CGrRenderer::RendererVertex(const CGrPoint &v)
{
   m_polyvertex.push_back(v);
}

void CGrRenderer::RendererColor(double *c)
{
}

void CGrRenderer::RendererPushMatrix()
{
}

void CGrRenderer::RendererPopMatrix()
{
}


void CGrRenderer::RendererRotate(double a, double x, double y, double z)
{
}

void CGrRenderer::RendererTranslate(double x, double y, double z)
{
}

void CGrRenderer::RendererMaterial(CGrMaterial *p_material)
{
}

void CGrRenderer::RendererTransform(const CGrTransform *p_transform)
{
}

void CGrRenderer::RendererSphere(const CGrPoint &center, double radius)
{
}

void CGrRenderer::RendererNormalize(bool)
{
}
