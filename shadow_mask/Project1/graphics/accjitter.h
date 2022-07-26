//
// Name :         accjitter.h
// Description :  Jittering functions from the OpenGL Red book.
//

#ifndef _ACCJITTER_H
#define _ACCJITTER_H

#include <GL/gl.h>

void accFrustrum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
                 GLdouble zNear, GLdouble zFar, GLdouble pixdx, GLdouble pixdy,
                 GLdouble eyedx, GLdouble eyedy, GLdouble focus);

void accPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar,
                    GLdouble pixdx, GLdouble pixdy, GLdouble eyedx, GLdouble eyedy,
                    GLdouble focus);

#endif
