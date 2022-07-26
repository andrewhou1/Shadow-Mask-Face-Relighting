//
// Name :          OpenGLWnd.h
// Description :   Header file for COpenGLWnd OpenGL superclass.
// Documentation : See OpenGLWnd.cpp
// Author :        Charles B. Owen
// Modification:   Jan 12, 19 - Y.T.
//

#pragma once

#include <gl/gl.h>
#include <gl/glu.h>

/////////////////////////////////////////////////////////////////////////////
// COpenGLWnd window

class COpenGLWnd : public CWnd
{
public:
    COpenGLWnd();

    // Operations
    virtual void OnGLDraw(CDC *pDC);

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(COpenGLWnd)
protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    //}}AFX_VIRTUAL

    // Implementation
public:
    void OnSaveImage();
    void GetSize(int &p_width, int &p_height);
    bool ObtainPixels(GLbyte **&p_pixels);
    virtual ~COpenGLWnd();

    HGLRC HGLRc() {return m_hrc;}

    // Generated message map functions
protected:
    void SetDoubleBuffer(bool p_doublebuffer);
    bool GetDoubleBuffer() const {return m_doublebuffer;}

    //{{AFX_MSG(COpenGLWnd)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    int FormattedErrorAfxMsgBox(const char *p_msg);

    bool m_created;
    bool m_doublebuffer;
    BOOL CreateRGBPalette(HDC hDC);

    HGLRC m_hrc;
    CPalette* m_pPal ;		//Palette

   	//
    // Support for generating RGB color palette
    //
    unsigned char ComponentFromIndex(int i, UINT nbits, UINT shift) ;

    static unsigned char   m_oneto8[2];
    static unsigned char   m_twoto8[4];
    static unsigned char   m_threeto8[8];
    static int             m_defaultOverride[13];
    static PALETTEENTRY    m_defaultPalEntry[20];
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "winmm.lib")