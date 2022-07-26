//
// Name :        GrCamera.cpp
// Description : Implementation of the CGrCamera camera control class.  This is
//               an easy-to-use class for implementation of basic camera controls
//               such as pan/tilt/roll/dolly/etc.
// Author :      Charles B. Owen. Modified by Yiying Tong
//

#include "pch.h"
#include <cmath>
#include <cassert>
#include "GrCamera.h"

const double GR_PI = 3.1415926535897932384626433832795;
const double GR_PI2 = 2. * GR_PI;
const double GR_RTOD = 180. / GR_PI;      // Converts radians to degrees
const double GR_DTOR = GR_PI / 180.;      // Converts degrees to radians


// Some linear algebra helper routines
inline void _Subtract(const double *a, const double *b, double *c)
{
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    c[2] = a[2] - b[2];
}

inline double _Length(const double *a)
{
    return sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
}

inline void _Normalize(double *a)
{
    double len = _Length(a);
    a[0] /= len;        a[1] /= len;        a[2] /= len;
}

inline void _Cross(const double *a, const double *b, double *c)
{
    c[0] = a[1]*b[2] - a[2]*b[1];
    c[1] = a[2]*b[0] - a[0]*b[2];
    c[2] = a[0]*b[1] - a[1]*b[0];
}

inline void _Identity(double t[4][4])
{
    for(int i=0;  i<4;  i++)
        for(int j=0;  j<4;  j++)
            t[i][j] = i == j ? 1. : 0.;
}

inline void _Translate(double t[4][4], double x, double y, double z)
{
    _Identity(t);
    t[0][3] = x;        t[1][3] = y;        t[2][3] = z;
}

inline void _RotateX(double m[4][4], double r)
{
    double rr = r * GR_DTOR;
    double cr = cos(rr);
    double sr = sin(rr);

    m[0][0] = 1;  m[0][1] = 0;  m[0][2] = 0;  m[0][3] = 0;
    m[1][0] = 0;  m[1][1] = cr;  m[1][2] = -sr;  m[1][3] = 0;
    m[2][0] = 0;  m[2][1] = sr;  m[2][2] = cr;  m[2][3] = 0;
    m[3][0] = 0;  m[3][1] = 0;  m[3][2] = 0;  m[3][3] = 1;
}

inline void _RotateY(double m[4][4], double r)
{
    double rr = r * GR_DTOR;
    double cr = cos(rr);
    double sr = sin(rr);

    m[0][0] = cr;  m[0][1] = 0;  m[0][2] = sr;  m[0][3] = 0;
    m[1][0] = 0;  m[1][1] = 1;  m[1][2] = 0;  m[1][3] = 0;
    m[2][0] = -sr;  m[2][1] = 0;  m[2][2] = cr;  m[2][3] = 0;
    m[3][0] = 0;  m[3][1] = 0;  m[3][2] = 0;  m[3][3] = 1;
}

inline void _RotateZ(double m[4][4], double r)
{
    double rr = r * GR_DTOR;
    double cr = cos(rr);
    double sr = sin(rr);

    m[0][0] = cr;  m[0][1] = -sr;  m[0][2] = 0;  m[0][3] = 0;
    m[1][0] = sr;  m[1][1] = cr;  m[1][2] = 0;  m[1][3] = 0;
    m[2][0] = 0;  m[2][1] = 0;  m[2][2] = 1;  m[2][3] = 0;
    m[3][0] = 0;  m[3][1] = 0;  m[3][2] = 0;  m[3][3] = 1;
}

inline void _Multiply(double a[4][4], double b[4][4], double res[4][4])
{
   for(int r=0;  r<4;  r++)
      for(int c=0;  c<4;  c++)
      {
         res[r][c] = a[r][0] * b[0][c] + a[r][1] * b[1][c] + a[r][2] * b[2][c] + a[r][3] * b[3][c];
      }
}

inline void _Multiply(double a[4][4], double b[4][4], double c[4][4], double res[4][4])
{
    double i[4][4];
    _Multiply(a, b, i);
    _Multiply(i, c, res);
}

inline void _MultiplyPoint(double m[4][4], double p[3])
{
    double x=p[0];      double y=p[1];      double z=p[2];
    p[0] = m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3];
    p[1] = m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3];
    p[2] = m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3];

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGrCamera::CGrCamera()
{
    m_mousemode[0] = PITCHYAW;      // Button 1
    m_mousemode[1] = PANTILT;       // Button 2
    m_mousemode[2] = ROLLMOVE;      // Button 3
    m_mouseButton = 0;              // Nothing currently pressed
    m_wheelSpeed = 0.01;            // Speed of the mouse wheel
    m_chordSpeed = 0.01;            // Speed of the mouse chord zoom
    m_gravity = true;
	m_flip = true;
    m_zNearRange = 0.25f;
    m_zFarRange = 4;
    Set(0, 0, 100, 0, 0, 0, 0, 1, 0);
    FieldOfView(25.);
}

CGrCamera::~CGrCamera()
{

}

void CGrCamera::Apply(int width, int height, bool noidentity)
{
    // Configure the projection matrix
    glMatrixMode(GL_PROJECTION);
    if(!noidentity)
        glLoadIdentity();

    ::gluPerspective(m_fieldofview, 
        GLdouble(width) / GLdouble(height),
        ZNear(), ZFar());

    // Configure the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    if(!noidentity)
        glLoadIdentity();

    ::gluLookAt(m_eye[0], m_eye[1], m_eye[2], 
        m_center[0], m_center[1], m_center[2],
        m_up[0], m_up[1], m_up[2]);
}


void CGrCamera::Set(double p_eyex, double p_eyey, double p_eyez, double p_centerx, double p_centery, double p_centerz, double p_upx, double p_upy, double p_upz)
{
    m_eye[0] = p_eyex;              m_eye[1] = p_eyey;              m_eye[2] = p_eyez;      m_eye[3] = 1;
    m_center[0] = p_centerx;        m_center[1] = p_centery;        m_center[2] = p_centerz; m_center[3] = 1;
    m_up[0] = p_upx;                m_up[1] = p_upy;                m_up[2] = p_upz;    m_up[3] = 0;

    ComputeFrame();
}

void CGrCamera::SetEye(double p_eyex, double p_eyey, double p_eyez)
{
    m_eye[0] = p_eyex;              m_eye[1] = p_eyey;              m_eye[2] = p_eyez;

    ComputeFrame();
}

void CGrCamera::SetCenter(double p_centerx, double p_centery, double p_centerz)
{
    m_center[0] = p_centerx;        m_center[1] = p_centery;        m_center[2] = p_centerz;

    ComputeFrame();
}

void CGrCamera::SetUp(double p_upx, double p_upy, double p_upz)
{
    m_up[0] = p_upx;                m_up[1] = p_upy;                m_up[2] = p_upz;

    ComputeFrame();
}

void CGrCamera::Set3dv(const double *p_eye, const double *p_center, const double *p_up)
{
    m_eye[0] = p_eye[0];            m_eye[1] = p_eye[1];        m_eye[2] = p_eye[2];
    m_center[0] = p_center[0];            m_center[1] = p_center[1];        m_center[2] = p_center[2];
    m_up[0] = p_up[0];            m_up[1] = p_up[1];        m_up[2] = p_up[2];

    ComputeFrame();
}

//
// Name :         CGrCamera::ComputeFrame()
// Description :  We maintain variables that describe the X,Y,Z axis of 
//                the camera frame.  This function computes those values.
//

void CGrCamera::ComputeFrame()
{
    if(m_gravity)
    {
        m_up[0] = 0;        m_up[1] = 1;        m_up[2] = 0;
    }

    _Subtract(m_eye, m_center, m_cameraz);
    _Normalize(m_cameraz);
    _Cross(m_cameraz, m_up, m_camerax);
    _Normalize(m_camerax);
    _Cross(m_cameraz, m_camerax, m_cameray);
}


//
// Camera rotation operations.  These function rotate the camera
// around the eye position.
//

void CGrCamera::Pan(double d)
{
    double ucen[4][4];
    _Translate(ucen, m_eye[0], m_eye[1], m_eye[2]);

    double rot[4][4];
    RotCameraY(rot, d);

    double cen[4][4];
    _Translate(cen, -m_eye[0], -m_eye[1], -m_eye[2]);

    double t[4][4];

    _Multiply(ucen, rot, cen, t);

    _MultiplyPoint(t, m_center);
    _MultiplyPoint(t, m_up);

    ComputeFrame();
}

void CGrCamera::Tilt(double d)
{
    double ucen[4][4];
    _Translate(ucen, m_eye[0], m_eye[1], m_eye[2]);

    double rot[4][4];
    RotCameraX(rot, d);

    double cen[4][4];
    _Translate(cen, -m_eye[0], -m_eye[1], -m_eye[2]);

    double t[4][4];

    _Multiply(ucen, rot, cen, t);

    _MultiplyPoint(t, m_center);
    _MultiplyPoint(t, m_up);

    ComputeFrame();
}

void CGrCamera::Roll(double d)
{
    double ucen[4][4];
    _Translate(ucen, m_eye[0], m_eye[1], m_eye[2]);

    double rot[4][4];
    RotCameraZ(rot, d);

    double cen[4][4];
    _Translate(cen, -m_eye[0], -m_eye[1], -m_eye[2]);

    double t[4][4];

    _Multiply(ucen, rot, cen, t);

    _MultiplyPoint(t, m_center);
    _MultiplyPoint(t, m_up);

    ComputeFrame();
}


//
// Center rotation operations.  These function rotate the camera around 
// the center location.  Note that camera roll and center roll would
// be the same thing.  So, we only need Yaw and Pitch.
//

void CGrCamera::Yaw(double d)
{
    double ucen[4][4];
    _Translate(ucen, m_center[0], m_center[1], m_center[2]);

    double rot[4][4];
    RotCameraY(rot, d);

    double cen[4][4];
    _Translate(cen, -m_center[0], -m_center[1], -m_center[2]);

    double b[4][4];

    _Multiply(ucen, rot, cen, b);

    _MultiplyPoint(b, m_eye);
    _MultiplyPoint(b, m_up);

    ComputeFrame();
}

void CGrCamera::Pitch(double d)
{
    double ucen[4][4];
    _Translate(ucen, m_center[0], m_center[1], m_center[2]);

    double rot[4][4];
    RotCameraX(rot, d);

    double cen[4][4];
    _Translate(cen, -m_center[0], -m_center[1], -m_center[2]);

    double a[4][4];

    _Multiply(ucen, rot, cen, a);

    _MultiplyPoint(a, m_eye);
    _MultiplyPoint(a, m_up);

    ComputeFrame();
}

//
// Name :         CGrCamera::Dolly()
// Description :  A camera dolly operation moves the camera in space.
//                This function moves the camera and center together.
//

void CGrCamera::Dolly(double x, double y, double z)
{
    double t[4][4];
    DollyHelper(t, x, y, z);

    _MultiplyPoint(t, m_center);
    _MultiplyPoint(t, m_eye);

    // Frame does not change...
}



void CGrCamera::DollyCamera(double x, double y, double z)
{
    double t[4][4];
    DollyHelper(t, x, y, z);

    _MultiplyPoint(t, m_eye);
   ComputeFrame();
}

void CGrCamera::DollyCenter(double x, double y, double z)
{
    double t[4][4];
    DollyHelper(t, x, y, z);

    _MultiplyPoint(t, m_center);
   ComputeFrame();
}

void CGrCamera::DollyHelper(double m[4][4], double x, double y, double z)
{
    double uncam[4][4];
    UnRotCamera(uncam);
    double tran[4][4];
    _Translate(tran, x, y, z);
    double tocam[4][4];
    RotCamera(tocam);

    _Multiply(uncam, tran, tocam, m);
}

void CGrCamera::MoveIn(double z)
{
    double view[3];
    _Subtract(m_eye, m_center, view);
    double dist = _Length(view);
    z *= dist;
    if(z < -dist/2)
        z = -dist/2;

    for(int i=0;  i<3;  i++)
        m_eye[i] += view[i] * (z/dist);
    ComputeFrame();
}

//
// Name :         CGrCamera::MouseWheel()
// Description :  Handle mouse wheel movements
//
void CGrCamera::MouseWheel(short zDelta)
{
    MoveIn(-zDelta * m_wheelSpeed);
}



//
// Name :         CGrCamera::SetMouseMode()
// Description :  Set the mode for a mouse button
//
void CGrCamera::SetMouseMode(eMouseMode m, int b)
{
    assert(b > 0 && b <= NumButtons);

    m_mousemode[b-1] = m;
}

//
// Name :         CGrCamera::MouseDown()
// Description :  Indicate that a mouse button has been pressed.
// Parameters :   x, y : Mouse coordinates
//                button : 1 to NumButtons
//
void CGrCamera::MouseDown(int x, int y, int button)
{
    assert(button > 0 && button <= NumButtons);

    m_mousex = x;  
    m_mousey = y;  
    m_mouseButton = button;
}

bool CGrCamera::MouseMove(int x, int y, UINT nFlags)
{
    if((nFlags & MK_LBUTTON) && (nFlags & MK_RBUTTON))
    {
        // We have a chording of the two buttons
        MoveIn((y - m_mousey) * m_chordSpeed);

        m_mousex = x;
        m_mousey = y;
        return true;
    }
    else if((nFlags & MK_LBUTTON) || (nFlags & MK_RBUTTON) || (nFlags & MK_MBUTTON))
    {
        // We have a single button press
        // m_mouseButton == 0 indicates no currently pressed button
        if(m_mouseButton >= 1)
        {
            switch(m_mousemode[m_mouseButton - 1])
            {
            case PANTILT:
                Pan((x - m_mousex) * -0.1);
                Tilt((y - m_mousey) * -0.1);
                break;

            case ROLLMOVE:
                Roll((x - m_mousex) * 0.3);
                MoveIn((y - m_mousey) * 0.002);
                break;

            case DOLLYXY:
                DollyCamera(x - m_mousex, y - m_mousey, 0);
                break;

            case PITCHYAW:
				if (m_flip) {
					Yaw((x - m_mousex) * 0.1);
					Pitch((y - m_mousey) * 0.1);
				} else {
					Yaw((x - m_mousex) * -0.1);
					Pitch((y - m_mousey) * -0.1);

				}
				break;

            case MOVE:
                MoveIn((y - m_mousey) * 0.002);
                break;
            }
        }

        m_mousex = x;
        m_mousey = y;
        return true;
    }

    return false;
}


//
// Name :         CGrCamera::SetGravity()
// Description :  Turn on or off gravity.  Gravity simply 
//                forces the up direction to stay up.
//

void CGrCamera::SetGravity(bool p_gravity)
{
    if(m_gravity == p_gravity)
        return;

    m_gravity = p_gravity;
    if(m_gravity)
    {
        m_up[0] = 0;        m_up[1] = 1;        m_up[2] = 0;
        ComputeFrame();
    }
}

//
// Name :         CGrCamera::CameraDistance()
// Description :  Returns the distance from the camera to the center
//

double CGrCamera::CameraDistance() const
{
    double view[3];
    _Subtract(m_eye, m_center, view);
    return _Length(view);
}

inline void CGrCamera::RotCamera(double m[4][4])
{
    _Identity(m);
    m[0][0] = m_camerax[0];     m[0][1] = m_camerax[1];     m[0][2] = m_camerax[2];
    m[1][0] = m_cameray[0];     m[1][1] = m_cameray[1];     m[1][2] = m_cameray[2];
    m[2][0] = m_cameraz[0];     m[2][1] = m_cameraz[1];     m[2][2] = m_cameraz[2];
}

inline void CGrCamera::UnRotCamera(double m[4][4])
{
    _Identity(m);
    m[0][0] = m_camerax[0];     m[1][0] = m_camerax[1];     m[2][0] = m_camerax[2];
    m[0][1] = m_cameray[0];     m[1][1] = m_cameray[1];     m[2][1] = m_cameray[2];
    m[0][2] = m_cameraz[0];     m[1][2] = m_cameraz[1];     m[2][2] = m_cameraz[2];
}

void CGrCamera::RotCameraX(double m[4][4], double a)
{
    double uncam[4][4];
    UnRotCamera(uncam);
    double rot[4][4];
    _RotateX(rot, a);
    double tocam[4][4];
    RotCamera(tocam);

    _Multiply(uncam, rot, tocam, m);
}


void CGrCamera::RotCameraY(double m[4][4], double a)
{
    double uncam[4][4];
    UnRotCamera(uncam);
    double rot[4][4];
    _RotateY(rot, a);
    double tocam[4][4];
    RotCamera(tocam);

    _Multiply(uncam, rot, tocam, m);
}


void CGrCamera::RotCameraZ(double m[4][4], double a)
{
    double uncam[4][4];
    UnRotCamera(uncam);
    double rot[4][4];
    _RotateZ(rot, a);
    double tocam[4][4];
    RotCamera(tocam);

    _Multiply(uncam, rot, tocam, m);
}


