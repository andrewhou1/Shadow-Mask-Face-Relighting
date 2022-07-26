// GrCamera.h: interface for the CGrCamera class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <GL/glu.h>

//! Class that defines a simple camera model we can use with the mouse.
/*! The CGrCamera class can be used in application to allow the mouse to
change the viewpoint by changes to pitch, yaw, pan, tilt, and other
common camera operations. 

\version 02-01-2000 1.00 Initial implementation.
\version 02-03-2003 1.01 Version not dependent on CGrPoint and CGrTransform.
\version 02-02-2008 1.02 Changes to allow multiple mouse options.
\version 01-29-2011 1.03 More flexible system for multiple mouse support
\version 01-30-2012 1.04 Added documentation. A few new functions.

The documentation on this class is under development and is not yet complete */
class CGrCamera  
{
public:
    //! Default constructor
	CGrCamera();

    //! Destructor
	virtual ~CGrCamera();

    //! Number of mouse buttons supported
    static const int NumButtons = 3;

    //! Set the speed of the mouse wheel zoom.
    /*! This function determines the speed of the zoom when the 
        mouse wheel is moved. The default value if 0.01, which is chosen
        to given a reasonable rate of zoom. The zoom uses the MoveIn function,
        which is dependent on distance, zooming faster when farther away. */
    void SetWheelSpeed(double speed) {m_wheelSpeed = speed;}

    //! Get the mouse wheel zoom speed.
    double GetWheelSpeed() const {return m_wheelSpeed;}

    //! Set the speed of the mouse chord zoom.
    /*! This function determines the speed of the zoom when the 
        mouse is moved with both the left and right buttons pressed
        at the same time (chording). The default value if 0.01, which is chosen
        to given a reasonable rate of zoom. The zoom uses the MoveIn function,
        which is dependent on distance, zooming faster when farther away. */
    void SetChordSpeed(double speed) {m_chordSpeed = speed;}

    //! Get the mouse wheel chord speed.
    double GetChordSpeed() const {return m_chordSpeed;}

    //! Returns the distance between the camera center and the camera viewpoint
	double CameraDistance() const;

    //! Set the gravity flag. 
    /*! When the gravity flag is true, the up direction is forced to
        (0, 1, 0) at all times. This forces the up direction always to be
        up. 
        \param gravity The new value to set the gravity flag to */
	void SetGravity(bool gravity);

    //! Get the current setting of the gravity flag
    bool GetGravity() const {return m_gravity;}

    //! Move the camera in directions relative to the camera orientation
    /*! A camera dolly is a movement of the camera in space. This
        version of the Dolly functions moves the camera center and eye together, 
        so the camera appears to continue to face the same direction.
        \param x Dolly amount in the x direction. This is to the right relative to the camera
        \param y Dolly amount in the y direction. This is up relative to the camera
        \param z Dolly amount in the z direction. This is backwards relative to the camera */
	void Dolly(double x, double y, double z);

    //! Move the camera in directions relative to the camera orientation
    /*! A camera dolly is a movement of the camera in space. This
        version of the Dolly functions moves the camera center only, 
        so the camera stays at the same location, but the viewpoint moves.
        \param x Dolly amount in the x direction. This is to the right relative to the camera
        \param y Dolly amount in the y direction. This is up relative to the camera
        \param z Dolly amount in the z direction. This is backwards relative to the camera */
	void DollyCenter(double x, double y, double z);

    //! Move the camera in directions relative to the camera orientation
    /*! A camera dolly is a movement of the camera in space. This
        version of the Dolly functions moves the camera location only, 
        so the camera appears to continue to face the same location.
        \param x Dolly amount in the x direction. This is to the right relative to the camera
        \param y Dolly amount in the y direction. This is up relative to the camera
        \param z Dolly amount in the z direction. This is backwards relative to the camera */
    void DollyCamera(double x, double y, double z);

    //! Pitch the camera
    /*! This function rotates the camera around the center such that the
        viewpoint appears to pitch.
        \param d Pitch angle in degrees */
	void Pitch(double d);

    //! Yaw the camera
    /*! This function rotates the camera around the center such that the
        viewpoint appears to yaw.
        \param d Yaw angle in degrees */
	void Yaw(double d);

    //! Roll the camera
    /*! This function rotates the camera around the center such that the
        viewpoint appears to roll. THe function has no effect if the 
        gravity flag is set.
        \param d Roll angle in degrees */
	void Roll(double d);

    //! Tilt the camera
    /*! This function rotates the center around the camera such that the
        camera appears to tilt up or down. 
        \param d Tilt angle in degrees */
	void Tilt(double d);

    //! Pan the camera
    /*! This function rotates the center around the camera such that the
        camera appears to pan left and right. 
        \param d Pan angle in degrees */
    void Pan(double d);

    //! Set many of the camera parameters in one function
    /*! This function sets several camera parameters in a single function call.
        \param eyex Camera eye X coordinate
        \param eyey Camera eye Y coordinate
        \param eyez Camera eye Z coordinate
        \param cenx Camera center X coordinate
        \param ceny Camera center Y coordinate
        \param cenz Camera center Z coordinate
        \param upx Camera up X coordinate
        \param upy Camera up Y coordinate
        \param upz Camera up Z coordinate */
	void Set(double eyex, double eyey, double eyez, double cenx, double ceny, double cenz, double upx=0, double upy=1, double upz=0);

    //! Set many of the camera parameters in one function
    /*! This function sets several camera parameters in a single function call.
        \param eye Camera eye position as an array of 3 values
        \param center Camera center position as an array of 3 values
        \param up Camera up direction as an array of 3 values */
    void Set3dv(const double *eye, const double *center, const double *up);

    //! Set the camera eye position. 
    /*! This is the location of the camera.
        \param eyex Camera eye X coordinate
        \param eyey Camera eye Y coordinate
        \param eyez Camera eye Z coordinate */
	void SetEye(double eyex, double eyey, double eyez);

    //! Set the camera eye position. 
    /*! This is the location of the camera.
        \param eye Camera eye as a three element array */
    void SetEye(const double *eye) {SetEye(eye[0], eye[1], eye[2]);}


    //! Set the camera center. 
    /*! This is the location the camera is looking at.
        \param cenx Camera center X coordinate
        \param ceny Camera center Y coordinate
        \param cenz Camera center Z coordinate */
	void SetCenter(double cenx, double ceny, double cenz);

    //! Set the camera center. 
    /*! This is the location the camera is looking at.
        \param cen Camera center coordinate as a three element array */
    void SetCenter(const double *cen) {SetCenter(cen[0], cen[1], cen[2]);}

    //! Set the camera up direction. 
    /*! A vector that specifies the direction considered up for the camera.
        \param upx Camera up X coordinate
        \param upy Camera up Y coordinate
        \param upz Camera up Z coordinate */
	void SetUp(double upx, double upy, double upz);

    //! Set the camera up direction. 
    /*! A vector that specifies the direction considered up for the camera.
        \param up Camera up direction as a three element array */
    void SetUp(const double *up) {SetUp(up[0], up[1], up[2]);}

    //! Proportional move of the camera closer to or or farther from the center.
    /*! This function moves the camera towards the viewpoint. The rate of move
        depends on the distance to the viewpoint, slowing as the viewpoint comes 
        closer and never actually reaching it. This as the advantage of an apparent
        zoom without the posibility of moving past the camera center.
        \param z Amount of the move. A positive value moves towards the camera. */
    void MoveIn(double z);

    //! Set the range for ZNear relative to the camera center
    /*! This value sets a range from 0 <= r < 1 that will determine
        the value of ZNear. For example, setting this value to 0.5 would set 
        ZNear at a point half way from the camera to the center. Smaller values move the 
        ZNear plane closer to the camera. The default value if 0.25.
        \param r New ZNear range value */
    void SetZNearRange(double r) {m_zNearRange = r;}

    //! Set the range for ZFar relative to the camera center
    /*! This value sets a range 1 < r that will determine
        the value of ZFar. For example, setting this value to 2 would set 
        ZFar at a point twice as far from the camera as the center. Larger values move the 
        ZFar plane farther from the camera. The default value if 4.
        \param r New ZFar range value */
    void SetZFarRange(double r) {m_zFarRange = r;}

    void FieldOfView(double f) {m_fieldofview = f;}
    double FieldOfView() const {return m_fieldofview;}

    const double *Eye() const {return m_eye;}
    const double *Center() const {return m_center;}
    const double *Up() const {return m_up;}

    double EyeX() const {return m_eye[0];}
    double EyeY() const {return m_eye[1];}
    double EyeZ() const {return m_eye[2];}
    double CenterX() const {return m_center[0];}
    double CenterY() const {return m_center[1];}
    double CenterZ() const {return m_center[2];}
    double UpX() const {return m_up[0];}
    double UpY() const {return m_up[1];}
    double UpZ() const {return m_up[2];}
    double ZNear() const {return CameraDistance() * m_zNearRange;}
    double ZFar() const {return CameraDistance() * m_zFarRange;}


    //! Posible modes for each mouse button
    enum eMouseMode {PANTILT=10, ROLLMOVE=11, PITCHYAW=12, DOLLYXY=13, MOVE=14};

    //! Set the mode for each mouse button
    /*! This function sets the mouse button modes. This is the camera
        operation assigned to each mouse button.
        The default values for the buttons are PITCHYAW on button 1,
        PANTILT on button 2, and ROLLMOVE on button 3.
        \param m The mode to set for the button
        \param button The button to set (1 to 3) */
    void SetMouseMode(eMouseMode m, int button=1);

    //! Get the current mouse mode
    eMouseMode GetMouseMode(int b=1) const {return m_mousemode[b-1];}

    //! Handle a mouse press
    /*! This function should be called when the mouse button is
        pressed. Here is example code for handling the left button press: \code
void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_camera.MouseDown(point.x, point.y);

    COpenGLWnd ::OnLButtonDown(nFlags, point);
}
\endcode
       
        This is example code for a right button press:\code
void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
    m_camera.MouseDown(point.x, point.y, 2);

    COpenGLWnd::OnRButtonDown(nFlags, point);
}
\endcode
        Note that MouseMove must also be implemented.
        \param x Mouse press x location
        \param y Mouse press y location
        \param button Button number 1 to 3. */
    void MouseDown(int x, int y, int button=1);

    //! Handle a mouse move
    /*! Handle the movement of the mouse. This implements the actual
        camera manipulations. An example of how this should be used 
        is here: \code
void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
    if(m_camera.MouseMove(point.x, point.y, nFlags))
         Invalidate();

    COpenGLWnd ::OnMouseMove(nFlags, point);
}
\endcode 
        \param x Mouse x position
        \param y Mouse y position
        \param nFlags The mouse flags as provided by the Windows mouse move event
        \return True if the viewpoint was changed and the display should be invalidated
*/
	bool MouseMove(int x, int y, UINT nFlags = (MK_LBUTTON | MK_RBUTTON));

    //! Handle a mouse wheel zoom
    /*! Handle the mouse wheel, implementing a zoom. The zoom speed 
        is determined by the values provided to the function SetWheelSpeed().
    Example usage: \code
BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    m_camera.MouseWheel(zDelta);
    Invalidate();

    return COpenGLWnd::OnMouseWheel(nFlags, zDelta, pt);
}
\endcode
        \param zDelta THe zDelta parameter provided by the mouse wheel event.
    */
    void MouseWheel(short zDelta);

    //! This function implements the gluLookAt() function using the current camera parameters
    inline void gluLookAt() {::gluLookAt(m_eye[0], m_eye[1], m_eye[2], 
                                  m_center[0], m_center[1], m_center[2],
                                  m_up[0], m_up[1], m_up[2]);}

    //! Apply the camera settings to OpenGL
    /*! This function sets up the camera projection and model view matrices
        in one step.
        \param width The window width
        \param height The window height
        \param noidentity If this flag is set, the matrices are not cleared to
        an identity matrix when set.

        An example of how this function is used, assuming GetSize if a function
        that returns the window size:\code
    //
    // Set up the camera
    //

    int width, height;
    GetSize(width, height);
    m_camera.Apply(width, height);
\endcode
    The implementation of this function is as follows:\code
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
\endcode
    */
    void Apply(int width, int height, bool noidentity=false);

private:
	void DollyHelper(double m[4][4], double x, double y, double z);
	void ComputeFrame();
    
    double          m_up[4];
	double          m_center[4];
	double          m_eye[4];

	double          m_fieldofview;

    eMouseMode      m_mousemode[NumButtons];

    bool            m_gravity;
	bool			m_flip;
    double          m_zNearRange;
    double          m_zFarRange;
    double          m_wheelSpeed;
    double          m_chordSpeed;

    // Last mouse press
	int             m_mousey;
	int             m_mousex;
    int             m_mouseButton;      // 0 for non pressed, 1-NumButtons otherwise

    // The camera frame.
    double          m_camerax[3];
    double          m_cameray[3];
    double          m_cameraz[3];

    void RotCamera(double m[4][4]);
    void UnRotCamera(double m[4][4]);
    void RotCameraX(double m[4][4], double a);
    void RotCameraY(double m[4][4], double a);
    void RotCameraZ(double m[4][4], double a);
};

