// Nurbs.cpp: implementation of the CNurbs class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Nurbs.h"

#include <cmath>
#include <cassert>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//
// Name :         CNurbs::CNurbs()
// Description :  Constructor.  Initialize this object.
//

CNurbs::CNurbs()
{
   m_controlpoints = true;    // Default is to draw the 
                              // control points with the curve
   m_firstdraw = true;        // So we call a function the first
                              // time we draw...
   m_texturemap = false;      // Default is to not texture map

   // m_texture.LoadFile("dive.bmp");

   // Initially set these values to defaults of nothing
   m_points = NULL;
   m_texpoints = NULL;
   m_usize = 0;
   m_vsize = 0;

}

CNurbs::~CNurbs()
{
   DeleteNurbsControlPoints();
}


//
// Name :         CNurbs::DrawSurface()
// Description :  Draw the surface.   This function 
//                is the main way we initiate the drawing process.
//

void CNurbs::DrawSurface()
{
   if(m_usize == 0 || m_vsize == 0)
      return;

   if(m_firstdraw)
      OnFirstDraw();

   glEnable(GL_AUTO_NORMAL);

   if(m_texturemap && !m_texture.Empty())
   {
      glEnable(GL_TEXTURE_2D);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, m_texture.MipTexName());
   }

   gluBeginSurface(m_nurbs);

   gluNurbsSurface(m_nurbs,
                   m_usize + 4, &m_uknots[0],
                   m_vsize + 4, &m_vknots[0],
                   m_vsize * 3,
                   3,
                   &m_points[0][0][0],
                   4, 4,
                   GL_MAP2_VERTEX_3);

   if(m_texturemap)
   {
      gluNurbsSurface(m_nurbs,
                      m_usize + 4, &m_texuknots[0],
                      m_vsize + 4, &m_texvknots[0],
                      m_vsize * 2,
                      2,
                      &m_texpoints[0][0][0],
                      4, 4,
                      GL_MAP2_TEXTURE_COORD_2);
   }

   gluEndSurface(m_nurbs);

   glDisable(GL_TEXTURE_2D);

   glDisable(GL_AUTO_NORMAL);
}


//
// Name :         CNurbs::DrawControlPoints()
// Description :  Draw the NURBS control points.  This is a
//                handy function for debugging problems.
//

void CNurbs::DrawControlPoints()
{
   if(m_firstdraw)
      OnFirstDraw();

   for(int iu=0;  iu<m_usize;  iu++)
   {
      for(int iv=0;  iv<m_vsize;  iv++)
      {
         GLfloat *p = m_points[iu][iv];

         const double PSIZE = 0.125;      // Size of my control point blocks

         glPushMatrix();
         glTranslated(p[0] - PSIZE / 2, p[1] - PSIZE / 2, p[2] - PSIZE / 2);

         Box(PSIZE, PSIZE, PSIZE);
         glPopMatrix();
      }
   }

}



//
// Name :         CNurbs::OnFirstDraw()
// Description :  Takes care of setting up for drawing...
//

void CNurbs::OnFirstDraw()
{
   m_firstdraw = false;

   // Allocate a NURBS renderer object
   m_nurbs = gluNewNurbsRenderer();
   gluNurbsProperty(m_nurbs, GLU_SAMPLING_TOLERANCE, 15.0);
   gluNurbsProperty(m_nurbs, GLU_DISPLAY_MODE, GLU_FILL);
}


//
// Name :         CNurbs::SetUV(int p_u, int p_v)
// Description :  Set the U and V control point counts.
//                This just calls AllocateNurbsControlPoints, but
//                I wanted a public portal in case I change the
//                functionality later.
//

void CNurbs::SetUV(int p_u, int p_v)
{
   AllocateNurbsControlPoints(p_u, p_v);
}


//
// Name :         CNurbs::CreateCylinder()
// Description :  Create a simple cylinder with open ends.
//

void CNurbs::CreateCylinder(double p_radius, double p_height, bool p_seal)
{
   // Make sure we are allocated first...
   if(m_usize == 0 || m_vsize == 0)
      return;

   int u, v;

   //
   // Allocate the surface points...
   //

   // v is along the length of the cylinder
   for(v=0;  v<m_vsize;  v++)
   {
      // u is around the radius...
      for(u=0;  u<m_usize;  u++)
      {
         m_points[u][v][0] = -p_radius * sin(double(u) / double(m_usize-3) * 6.2830);
         m_points[u][v][1] = double(v) / double(m_vsize - 1) * p_height;
         m_points[u][v][2] = -p_radius * cos(double(u) / double(m_usize-3) * 6.2830);

         if(p_seal && (v == 0 || v == m_vsize - 1))
         {
            m_points[u][v][0] = 0;
            m_points[u][v][2] = 0;
         }

      }

   }

   // We want uniform sampling in the u dimension
   KnotUniform(m_uknots);

   // We want to interpolate in the v dimension
   KnotInterpolate(m_vknots);

   //
   // Fill in the texture data points
   //

   for(v=0;  v<m_vsize;  v++)
   {
      // u is around the radius...
      for(u=0;  u<m_usize;  u++)
      {
         // Texture points
         m_texpoints[u][v][0] = double(u) / double(m_usize - 1);
         m_texpoints[u][v][1] = double(v) / double(m_vsize - 1);
      }
   }

   // We want uniform sampling in the u dimension
   KnotUniform(m_texuknots);

   // We want to interpolate in the v dimension
   KnotInterpolate(m_texvknots);
}


//
// Name :         CNurbs::AllocateNurbsControlPoints()
// Description :  In this application we have a variable-sized NURBS surfaces.
//                This function allocates the control point arrays for 
//                the texture and surface to the right size and
//                sets the size of the knots arrays.
// Parameters :   p_usize - Number of control points in the U dimension
//                p_vsize - Number of control points in the V dimension.
//

void CNurbs::AllocateNurbsControlPoints(int p_usize, int p_vsize)
{
   // Don't allocate if we are already allocated...
   if(m_usize == p_usize && m_vsize == p_vsize)
      return;

   // Delete any existing allocation
   DeleteNurbsControlPoints();

   if(p_usize == 0 || p_vsize == 0)
      return;

   m_usize = p_usize;
   m_vsize = p_vsize;

   // Allocate the points and texture points arrays
   // This is a simple 2D allocation...
   m_points = new Point3 *[m_usize];
   m_points[0] = new Point3[m_usize * m_vsize];
   m_texpoints = new Point2 *[m_usize];
   m_texpoints[0] = new Point2[m_usize * m_vsize];

   for(int i=1;  i<m_usize;  i++)
   {
      m_points[i] = m_points[0] + i * m_vsize;
      m_texpoints[i] = m_texpoints[0] + i * m_vsize;
   }

   // Set the knot array sizes
   m_uknots.resize(m_usize + 4);
   m_vknots.resize(m_vsize + 4);
   m_texuknots.resize(m_usize + 4);
   m_texvknots.resize(m_vsize + 4);

   KnotUniform(m_uknots);
   KnotUniform(m_vknots);
   KnotUniform(m_texuknots);
   KnotUniform(m_texvknots);
}


//
// Name :         CNurbs::DeleteNurbsControlPoints()
// Description :  We have a dynamic array that stores the NURBS control
//                points.  This function deletes that allocation.
//

void CNurbs::DeleteNurbsControlPoints()
{
   if(m_points)
   {
      delete [] m_points[0];
      delete [] m_points;

      m_points = NULL;
   }

   if(m_texpoints)
   {
      delete [] m_texpoints[0];
      delete [] m_texpoints;

      m_texpoints = NULL;
   }

   m_usize = 0;
   m_vsize = 0;
}

//
// Name :         CNurbs::SetControlPoint()
// Description :  Sets the value of a control point.
//

void CNurbs::SetControlPoint(int u, int v, double x, double y, double z)
{
   assert(u >= 0);
   assert(u < m_usize);
   assert(v >= 0);
   assert(v < m_vsize);

   m_points[u][v][0] = x;
   m_points[u][v][1] = y;
   m_points[u][v][2] = z;
}

//
// Name :         CNurbs::SetKnotU()
// Description :  Set a U knot value.
//

void CNurbs::SetKnotU(int u, double k)
{
   assert(u >= 0);
   assert(u < m_usize + 4);

   m_uknots[u] = k;
}


//
// Name :         CNurbs::SetKnotV()
// Description :  Set a V knot value.
//

void CNurbs::SetKnotV(int v, double k)
{
   assert(v >= 0);
   assert(v < m_vsize + 4);

   m_vknots[v] = k;
}


//
// Name :         CNurbs::KnotUniform()
// Description :  Fill in a knots vector with uniformly space knots.
//

void CNurbs::KnotUniform(std::vector<GLfloat> &p_knots)
{
   for(int i=0;  i<p_knots.size();  i++)
      p_knots[i] = i;
}


//
// Name :         CNurbs::KnotInterplote()
// Description :  Fill in a knots vector with knots that will
//                interpolate the end points.
//

void CNurbs::KnotInterpolate(std::vector<GLfloat> &p_knots)
{
   int i;
   int k = 0;

   for(i=0;  i<4;  i++)
      p_knots[i] = 0;

   k++;
   for( ; i<p_knots.size() - 4;  i++)
      p_knots[i] = k++;

   for( ; i<p_knots.size();  i++)
      p_knots[i] = k;
}




//
//        Name : CNurbs::Box()
// Description : Draw an arbitrary size box. p_x, p_y, and 
//               p_z are the height of the box. We'll use this is a 
//               common primitive.
//      Origin : The back corner is at 0, 0, 0, and the box 
//               is entirely in the positive octant.
//

void CNurbs::Box(GLdouble p_x, GLdouble p_y, GLdouble p_z)
{
   GLdouble a[] = {0., 0., p_z};
   GLdouble b[] = {p_x, 0., p_z};
   GLdouble c[] = {p_x, p_y, p_z};
   GLdouble d[] = {0., p_y, p_z};
   GLdouble e[] = {0., 0., 0.};
   GLdouble f[] = {p_x, 0., 0.};
   GLdouble g[] = {p_x, p_y, 0.};
   GLdouble h[] = {0., p_y, 0.};

   // Front
   glBegin(GL_QUADS);
      glNormal3d(0, 0, 1);
      glVertex3dv(a);
      glVertex3dv(b);
      glVertex3dv(c);
      glVertex3dv(d);
   glEnd();

   // Right
   glBegin(GL_QUADS);
      glNormal3d(1, 0, 0);
      glVertex3dv(c);
      glVertex3dv(b);
      glVertex3dv(f);
      glVertex3dv(g);
   glEnd();

   // Back
   glBegin(GL_QUADS);
      glNormal3d(0, 0, -1);
      glVertex3dv(h);
      glVertex3dv(g);
      glVertex3dv(f);
      glVertex3dv(e);
   glEnd();

   // Left
   glBegin(GL_QUADS);
      glNormal3d(-1, 0, 0);
      glVertex3dv(d);
      glVertex3dv(h);
      glVertex3dv(e);
      glVertex3dv(a);
   glEnd();

   // Top
   glBegin(GL_QUADS);
      glNormal3d(0, 1, 0);
      glVertex3dv(d);
      glVertex3dv(c);
      glVertex3dv(g);
      glVertex3dv(h);
   glEnd();

   // Bottom
   glBegin(GL_QUADS);
      glNormal3d(0, -1, 0);
      glVertex3dv(e);
      glVertex3dv(f);
      glVertex3dv(b);
      glVertex3dv(a);
   glEnd();
}







bool CNurbs::LoadTextureFile(const char *p_file)
{
   return m_texture.LoadFile(p_file);
}



