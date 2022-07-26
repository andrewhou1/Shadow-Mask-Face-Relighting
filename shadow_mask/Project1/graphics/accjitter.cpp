//
// Name :         accjitter.h
// Description :  Jittering functions from the OpenGL Red book.
//

#include <stdafx.h>
#include <cmath>
#include "accjitter.h"

GLdouble PI_ = 3.14159265358979323846;

void accFrustrum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
                 GLdouble zNear, GLdouble zFar, GLdouble pixdx, GLdouble pixdy,
                 GLdouble eyedx, GLdouble eyedy, GLdouble focus)
{
   GLint viewport[4];

   // Get the current viewport
   glGetIntegerv(GL_VIEWPORT, viewport);

   GLdouble xwsize = right - left;
   GLdouble ywsize = top - bottom;

   GLdouble dx = -(pixdx * xwsize / (GLdouble)viewport[2] + eyedx * zNear / focus);
   GLdouble dy = -(pixdy * ywsize / (GLdouble)viewport[3] + eyedy * zNear / focus);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(left + dx, right + dx, bottom + dy, top + dy, zNear, zFar);
   glMatrixMode(GL_MODELVIEW);

   glLoadIdentity();
   glTranslated(-eyedx, -eyedy, 0.0);
}

void accPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar,
                    GLdouble pixdx, GLdouble pixdy, GLdouble eyedx, GLdouble eyedy,
                    GLdouble focus)
{
   GLdouble fov2 = ((fovy * PI_) / 180.) / 2.0;

   GLdouble top = zNear * tan(fov2);
   GLdouble bottom = -top;
   GLdouble right = top * aspect;
   GLdouble left = - right;

   accFrustrum(left, right, bottom, top, zNear, zFar, pixdx, pixdy, eyedx, eyedy, focus);
}

