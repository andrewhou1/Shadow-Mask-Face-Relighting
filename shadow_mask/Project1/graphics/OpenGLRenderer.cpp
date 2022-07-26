//
// Name :         OpenGLRenderer.cpp
// Description :  Implementation for COpenGLRenderer
//                This class is a plug-in renderer for OpenGL.  It is 
//                basically a mapping of the generic renderer calls to the
//                matching OpenGL calls.
// Author :       Charles B. Owen
//

#include "pch.h"
#include "OpenGLRenderer.h"

#include "GrTexture.h"

#include <GL/glu.h>
#include <list>

using namespace std;


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COpenGLRenderer::COpenGLRenderer()
{

}

COpenGLRenderer::~COpenGLRenderer()
{

}


//
// Name :         COpenGLRenderer::RendererStart()
// Description :  Perform actions we must do before we render the model.
//

bool COpenGLRenderer::RendererStart()
{
   // Clear the screen
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f) ;
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //
   // Set up the camera
   //

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   // Set the camera parameters
   gluPerspective(ProjectionAngle(),    // Vertical field of view in degrees.
                  ProjectionAspect(),   // The aspect ratio.
                  NearClip(),           // Near clipping
                  FarClip());        // Far clipping

   //
   // Some standard parameters
   //

   // Enable depth test
   glEnable(GL_DEPTH_TEST);

   // Cull backfacing polygons
   glCullFace(GL_BACK);
   glEnable(GL_CULL_FACE);

   // Set the camera location
   glMatrixMode(GL_MODELVIEW);

   glLoadIdentity();
   gluLookAt(Eye().X(), Eye().Y(), Eye().Z(), 
             Center().X(), Center().Y(), Center().Z(), 
             Up().X(), Up().Y(), Up().Z());

   // Enable lighting
   glEnable(GL_LIGHTING);

   // Configure any lights
   for(int i=0;  i<LightCnt();  i++)
   {
      glEnable(GL_LIGHT0+i);

      GLfloat lightpos[4];
      for(int j=0;  j<4;  j++)
         lightpos[j] = GLfloat(GetLight(i).m_pos[j]);

      glLightfv(GL_LIGHT0+i, GL_POSITION, lightpos);
      glLightfv(GL_LIGHT0+i, GL_DIFFUSE, GetLight(i).m_diffuse);
      glLightfv(GL_LIGHT0+i, GL_SPECULAR, GetLight(i).m_specular);
      glLightfv(GL_LIGHT0+i, GL_AMBIENT, GetLight(i).m_ambient);
   }

   return true;
}


//
// Name :         COpenGLRenderer::RendererEnd()
// Description :  Perform actions that must be done after everything is rendered.
//

bool COpenGLRenderer::RendererEnd()
{
   glFlush();
   return true;
}


//
// Name :         COpenGLRenderer::RendererEndPolygon()
// Description :  End definition of a polygon.  The superclass has
//                already collected the polygon information
//
void COpenGLRenderer::RendererEndPolygon()
{
   const list<CGrPoint> &vertices = PolyVertices();
   const list<CGrPoint> &normals = PolyNormals();
   const list<CGrPoint> &tvertices = PolyTexVertices();

   if(PolyTexture())
   {
      glEnable(GL_TEXTURE_2D);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, PolyTexture()->TexName());
   }

   list<CGrPoint>::const_iterator normal = normals.begin();
   list<CGrPoint>::const_iterator tvertex = tvertices.begin();

   glBegin(GL_POLYGON);
   for(list<CGrPoint>::const_iterator i=vertices.begin();  i!=vertices.end();  i++)
   {
      if(normal != normals.end())
      {
         normal->glNormal();
         normal++;
      }

      if(tvertex != tvertices.end())
      {
         tvertex->glTexVertex();
         tvertex++;
      }
      
      i->glVertex();
   }

   glEnd();

   if(PolyTexture())
   {
      glDisable(GL_TEXTURE_2D);
   }


}


void COpenGLRenderer::RendererPushMatrix()
{
   glPushMatrix();
}

void COpenGLRenderer::RendererPopMatrix()
{
   glPopMatrix();
}

void COpenGLRenderer::RendererRotate(double a, double x, double y, double z)
{
   glRotated(a, x, y, z);
}

void COpenGLRenderer::RendererTranslate(double x, double y, double z)
{
    glTranslated(x, y, z);
}


void COpenGLRenderer::RendererTransform(const CGrTransform *p_transform)
{
   p_transform->glMultMatrix();
}

void COpenGLRenderer::RendererMaterial(CGrMaterial *p_material)
{
   p_material->glMaterial();
}

void COpenGLRenderer::RendererColor(double *c)
{
   glColor4dv(c);
}
