//
// Name :         Nurbs.h
// Description :  Header file for the CNurbs class
//                This class is used to describe and draw a NURBS surface.
//

#if !defined(AFX_NURBS_H__C71DE3EB_FDE1_4D39_BB30_425A2C6BE7C7__INCLUDED_)
#define AFX_NURBS_H__C71DE3EB_FDE1_4D39_BB30_425A2C6BE7C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <GL/gl.h>
#include <GL/glu.h>

#include <vector>
#include "Texture.h"	// Added by ClassView

class CNurbs  
{
public:
	void SetKnotV(int v, double k);
	void SetKnotU(int u, double k);
	void SetControlPoint(int u, int v, double x, double y, double z);
	bool LoadTextureFile(const char *p_file);
	void DrawControlPoints();
	CNurbs();
	virtual ~CNurbs();

	void SetUV(int p_u, int p_v);
   void SetTextureMap(bool p) {m_texturemap = p;}

	void DrawSurface();

   // Example surfaces...
	void CreateCylinder(double p_radius, double p_height, bool p_seal=false);

private:
	void OnFirstDraw();
	void AllocateNurbsControlPoints(int p_usize, int p_vsize);
	void DeleteNurbsControlPoints();
   void Box(GLdouble p_x, GLdouble p_y, GLdouble p_z);

   void KnotUniform(std::vector<GLfloat> &p_knots);
   void KnotInterpolate(std::vector<GLfloat> &p_knots);

	GLUnurbsObj      *m_nurbs;

	bool              m_texturemap;
	CTexture          m_texture;
	bool              m_firstdraw;
   bool              m_controlpoints;

   // In this application we are dynamically allocating the
   // points array for our NURBS surface.  We can set various
   // different things depending on the size we make these
   // arrays.  Note that the allocation MUST be contiguous
   // for OpenGL to be able to deal with it.

   int               m_usize;    // What's the U dimension?
   int               m_vsize;    // What's the V dimension?

   // 3D point for our points array
   struct Point3
   {
      Point3() {}
      operator float *() {return m;}

   private:
      float m[3];
   };

   // 2D point for our texture points array
   struct Point2
   {
      Point2() {}
      operator float *() {return m;}

   private:
      float m[2];
   };

   // The points array and knots
   Point3         **m_points;
   std::vector<GLfloat>   m_uknots;
   std::vector<GLfloat>   m_vknots;

   // The texture points array and knots
   Point2         **m_texpoints;
   std::vector<GLfloat>   m_texuknots;
   std::vector<GLfloat>   m_texvknots;
};

#endif // !defined(AFX_NURBS_H__C71DE3EB_FDE1_4D39_BB30_425A2C6BE7C7__INCLUDED_)
