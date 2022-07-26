//
// Name :         OpenGLWnd.cpp
// Description :  Implementation for the COpenGLWnd class.  This is a
//                class derived from CWnd that will automatically set up
//                for OpenGL.  You can use this wherever you would use
//                CWnd and you'll automatically have OpenGL in this window.
// Author :       Charles B. Owen
// Version :       1-09-01 1.00 CBO Initial version number.
//                 3-03-03 1.01 CBO Improved error handing.
//                 3-16-03 1.02 CBO Fixed problem with saving image
//                 1-10-11 1.03 CBO Some cleanup prior to the semester
//				   1-13-14 YT setting double-buffering as default and a few minor changes
//				   1-12-20 YT stdafx.h is no longer the name of precompiled header, which is now just pch.h
//		


#include "pch.h"
#include "OpenGLWnd.h"

#include <cassert>
#include <fstream>
#include <vector>
#include <strstream>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// COpenGLWnd

COpenGLWnd::COpenGLWnd()
	: m_pPal(NULL), m_doublebuffer(true), m_created(false)
{
}

COpenGLWnd::~COpenGLWnd()
{
    if(m_pPal != NULL)
        delete m_pPal;
}


BEGIN_MESSAGE_MAP(COpenGLWnd, CWnd)
    //{{AFX_MSG_MAP(COpenGLWnd)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COpenGLWnd message handlers

BOOL COpenGLWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
    if (!CWnd::PreCreateWindow(cs))
        return FALSE;

    // Added for OpenGL
    cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN ;

    return true;
}

int COpenGLWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    CClientDC dc(this) ;
    //
    // Fill in the pixel format descriptor.
    //
    PIXELFORMATDESCRIPTOR pfd ;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR)) ;
    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR); 
    pfd.nVersion   = 1 ; 
    pfd.dwFlags    = PFD_SUPPORT_OPENGL |
        PFD_DRAW_TO_WINDOW ;

    if(m_doublebuffer)
        pfd.dwFlags |= PFD_DOUBLEBUFFER;

    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24 ;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32 ;
    pfd.iLayerType = PFD_MAIN_PLANE ;

    int nPixelFormat = ChoosePixelFormat(dc.m_hDC, &pfd);
    if (nPixelFormat == 0)
        return FormattedErrorAfxMsgBox("ChoosePixelFormat Failed");

    BOOL bResult = SetPixelFormat (dc.m_hDC, nPixelFormat, &pfd);
    if(!bResult)
        return FormattedErrorAfxMsgBox("SetPixelFormat  Failed");

    //
    // Create a rendering context.
    //
    m_hrc = wglCreateContext(dc.m_hDC);
    if (!m_hrc)
        return FormattedErrorAfxMsgBox("Unable to create an OpenGL rendering context");

    // Create the palette
    CreateRGBPalette(dc.m_hDC) ;

    m_created = true;

    return 0;
}


//
// CreateRGBPalette
//
BOOL COpenGLWnd::CreateRGBPalette(HDC hDC)
{
    //
    // Check to see if we need a palette
    //
    PIXELFORMATDESCRIPTOR pfd;
    int n = GetPixelFormat(hDC);
    DescribePixelFormat(hDC, n, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    if (!(pfd.dwFlags & PFD_NEED_PALETTE)) return FALSE ;

    // allocate a log pal and fill it with the color table info
    LOGPALETTE* pPal = (LOGPALETTE*) malloc(sizeof(LOGPALETTE) 
        + 256 * sizeof(PALETTEENTRY));
    if (!pPal) 
    {
        AfxMessageBox(TEXT("Out of memory for pallet"), MB_OK | MB_ICONSTOP);
        return FALSE;
    }
    pPal->palVersion = 0x300; // Windows 3.0
    pPal->palNumEntries = 256; // table size

    //
    // Create RGB Palette
    //
    ASSERT( pfd.cColorBits == 8) ;
    n = 1 << pfd.cColorBits;
    for (int i=0; i<n; i++)
    {
        pPal->palPalEntry[i].peRed =
            ComponentFromIndex(i, pfd.cRedBits, pfd.cRedShift);
        pPal->palPalEntry[i].peGreen =
            ComponentFromIndex(i, pfd.cGreenBits, pfd.cGreenShift);
        pPal->palPalEntry[i].peBlue =
            ComponentFromIndex(i, pfd.cBlueBits, pfd.cBlueShift);
        pPal->palPalEntry[i].peFlags = 0;
    }

    //
    // Fix up color table with system colors.
    //
    if ((pfd.cColorBits == 8)                           &&
        (pfd.cRedBits   == 3) && (pfd.cRedShift   == 0) &&
        (pfd.cGreenBits == 3) && (pfd.cGreenShift == 3) &&
        (pfd.cBlueBits  == 2) && (pfd.cBlueShift  == 6)
        )
    {
        for (int j = 1 ; j <= 12 ; j++)
            pPal->palPalEntry[m_defaultOverride[j]] = m_defaultPalEntry[j];
    }

    // Delete any existing GDI palette
    if (m_pPal) 
        delete m_pPal ;
    m_pPal = new CPalette ;

    BOOL bResult = m_pPal->CreatePalette(pPal);
    free (pPal);

    return bResult;
}


//////////////////////////////////////////////////////////////////////////////////////
//
// OpenGLpalette
//
unsigned char COpenGLWnd::m_threeto8[8] = {
    0, 0111>>1, 0222>>1, 0333>>1, 0444>>1, 0555>>1, 0666>>1, 0377
};
unsigned char COpenGLWnd::m_twoto8[4] = {
   0, 0x55, 0xaa, 0xff
};
unsigned char COpenGLWnd::m_oneto8[2] = {
    0, 255
};

int COpenGLWnd::m_defaultOverride[13] = {
    0, 3, 24, 27, 64, 67, 88, 173, 181, 236, 247, 164, 91
};

PALETTEENTRY COpenGLWnd::m_defaultPalEntry[20] = {
    { 0,   0,   0,    0 }, //0
    { 0x80,0,   0,    0 }, 
    { 0,   0x80,0,    0 }, 
    { 0x80,0x80,0,    0 }, 
    { 0,   0,   0x80, 0 },
    { 0x80,0,   0x80, 0 },
    { 0,   0x80,0x80, 0 },
    { 0xC0,0xC0,0xC0, 0 }, //7

    { 192, 220, 192,  0 }, //8
    { 166, 202, 240,  0 },
    { 255, 251, 240,  0 },
    { 160, 160, 164,  0 }, //11

    { 0x80,0x80,0x80, 0 }, //12
    { 0xFF,0,   0,    0 },
    { 0,   0xFF,0,    0 },
    { 0xFF,0xFF,0,    0 },
    { 0,   0,   0xFF, 0 },
    { 0xFF,0,   0xFF, 0 },
    { 0,   0xFF,0xFF, 0 },
    { 0xFF,0xFF,0xFF, 0 }  //19
  };



//
// ComponentFromIndex
//
unsigned char COpenGLWnd::ComponentFromIndex(int i, UINT nbits, UINT shift)
{
    unsigned char val;

    val = (unsigned char) (i >> shift);
    switch (nbits) {

    case 1:
        val &= 0x1;
        return m_oneto8[val];

    case 2:
        val &= 0x3;
        return m_twoto8[val];

    case 3:
        val &= 0x7;
        return m_threeto8[val];

    default:
        return 0;
    }
}


void COpenGLWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

    // Select the palette.
    CPalette* ppalOld = NULL;
	if (m_pPal)
	{
    	ppalOld = dc.SelectPalette(m_pPal, 0);
    	dc.RealizePalette();
	}
	
	// Make the HGLRC current
    BOOL bResult = wglMakeCurrent(dc.m_hDC, m_hrc);
    if (!bResult)
    {
        FormattedErrorAfxMsgBox("wglMakeCurrent Failed");
    }

    // Set the window:
    int width, height;
    GetSize(width, height);

    // Set the viewport
    glViewport(0, 0, width, height);

    //
    // Set up the mapping of 3-space to screen space
    //
    GLdouble gldAspect = GLdouble(width)/ GLdouble(height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, gldAspect, 1.0, 10.0);

	// Draw	
	OnGLDraw(&dc);

	//Swap Buffers
	if(m_doublebuffer)
      SwapBuffers(dc.m_hDC) ;

    // select old palette if we altered it
    if (ppalOld) 
        dc.SelectPalette(ppalOld, 0); 

	wglMakeCurrent(NULL, NULL) ;
}


void COpenGLWnd::SetDoubleBuffer(bool p_doublebuffer)
{
   // Double buffering must be set before the window is created.
   assert(!m_created);

   m_doublebuffer = p_doublebuffer;
}


//
// Name :         COpenGLWnd::OnGLDraw(CDC *pDC)
// Description :  Default OnGLDraw function.  This function clears the screen to a
//                strange color.
//

void COpenGLWnd::OnGLDraw(CDC *pDC)
{
   glClearColor(0.3f, 0.7f, 0.3f, 0.0f) ;
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glFlush();
}

//
// Name :         COpenGLWnd::GetSize()
// Description :  Convenience function to obtain the window size.
//

void COpenGLWnd::GetSize(int &p_width, int &p_height)
{
   RECT rect;
   GetClientRect(&rect);

   p_width = rect.right;
   p_height = rect.bottom;
}


BOOL COpenGLWnd::OnEraseBkgnd(CDC* pDC) 
{
	return true;
}

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
const int DIB_PADSIZE = 4;

//
// Name :         COpenGLWnd::OnSaveImage()
// Description :  This is called by a menu option.  It saves the current window image
//                as a DIB file.  It pops up a dialog box asking for the file name.
//

void COpenGLWnd::OnSaveImage()
{
   CWaitCursor wait;

   //
   // Obtain the image pixels
   //

   GLbyte **buffer;
   if(!ObtainPixels(buffer))
      return;

   //
   // Determine the output file
   //

	static _TCHAR BASED_CODE szFilter[] = TEXT("Image Files (*.bmp)|*.bmp|All Files (*.*)|*.*||");

	CFileDialog dlg(FALSE, TEXT(".bmp"), NULL, 0, szFilter, NULL);
	if(dlg.DoModal() != IDOK)
   {
      delete buffer[0];
      delete buffer;
      return;
   }

   UpdateWindow();

   // Open the file
   std::ofstream file(dlg.GetPathName(), std::ios::binary);

   //
   // Write the image file header
   //

   int width, height;
   GetSize(width, height);

   int usewidth3 = (width * 3 + (DIB_PADSIZE - 1)) / DIB_PADSIZE;     
   usewidth3 *= DIB_PADSIZE;

   BITMAPFILEHEADER bmfHdr;   // Header for Bitmap file

   // Fill in the bitmap file header
	bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM"
   bmfHdr.bfSize = sizeof(BITMAPFILEHEADER) + 
         sizeof(BITMAPINFOHEADER) + height * usewidth3;
   bmfHdr.bfReserved1 = 0;
   bmfHdr.bfReserved2 = 0;
   bmfHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

   // Fill in a BITMAPINFOHEADER
   BITMAPINFOHEADER bmi;

   bmi.biSize = sizeof(bmi);
   bmi.biWidth = width;
   bmi.biHeight = height;
   bmi.biPlanes = 1;
   bmi.biBitCount = 24;
   bmi.biCompression = BI_RGB;
   bmi.biSizeImage = 0;
   bmi.biXPelsPerMeter = 1;
   bmi.biYPelsPerMeter = 1;
   bmi.biClrUsed = 0;
   bmi.biClrImportant = 0;

   file.write((const char *)&bmfHdr, sizeof(BITMAPFILEHEADER));
   file.write((const char *)&bmi, sizeof(bmi));

   if(!file)
   {
      AfxMessageBox(TEXT("Unable to write output file file"), IDOK);
      delete buffer[0];
      delete buffer;

      return;
   }

   for(int i=0;  i<height;  i++)
   {
      file.write((const char *)&buffer[i][0], usewidth3);
   }

   delete buffer[0];
   delete buffer;

   if(!file)
   {
      AfxMessageBox(TEXT("Unable to write output file file"), IDOK);
      return;
   }

   file.close();

   wait.Restore();
}


//
// Name :         COpenGLWnd::ObtainPixels()
// Description :  Allocates the 2D array for p_pixels and obtains all of the 
//                image pixels into it from OpenGL.
// Returns :      true if successful.
//

bool COpenGLWnd::ObtainPixels(GLbyte **&p_pixels)
{
    // Force window to be up-to-date
    UpdateWindow();

    //
    // Make the OpenGL rendering context current
    //

    CClientDC dc(this) ;
    BOOL bResult = wglMakeCurrent(dc.m_hDC, m_hrc);
	if (!bResult)
	{
      AfxMessageBox(TEXT("Unable to select OpenGL rendering context, could not obtain pixel data."));
      p_pixels = NULL;
      return false;
	}

   //
   // Allocate space for the image
   //

   int width, height;
   GetSize(width, height);

   int usewidth3 = (width * 3 + (DIB_PADSIZE - 1)) / DIB_PADSIZE;     
   usewidth3 *= DIB_PADSIZE;

   p_pixels = new GLbyte *[height];
   p_pixels[0] = new GLbyte[height * usewidth3];
   for(int j=1;  j<height;  j++)
      p_pixels[j] = p_pixels[0] + j * usewidth3; 

   //
   // Configure OpenGL for obtaining the rows.
   //

   GLint swapbytes, lsbfirst, rowlength;
   GLint skiprows, skippixels, alignment;

   /* Save current modes. */
   glGetIntegerv(GL_PACK_SWAP_BYTES, &swapbytes);
   glGetIntegerv(GL_PACK_LSB_FIRST, &lsbfirst);
   glGetIntegerv(GL_PACK_ROW_LENGTH, &rowlength);
   glGetIntegerv(GL_PACK_SKIP_ROWS, &skiprows);
   glGetIntegerv(GL_PACK_SKIP_PIXELS, &skippixels);
   glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);

   glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);
   glPixelStorei(GL_PACK_LSB_FIRST, GL_FALSE);
   glPixelStorei(GL_PACK_ROW_LENGTH, 0);
   glPixelStorei(GL_PACK_SKIP_ROWS, 0);
   glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
   glPixelStorei(GL_PACK_ALIGNMENT, DIB_PADSIZE);

   //
   // Actually read the pixels.
   //

   glReadBuffer(GL_FRONT);
   glReadPixels(0, 0, width, height, GL_BGR_EXT,
                GL_UNSIGNED_BYTE, (GLvoid *)&p_pixels[0][0]);

   /* Restore saved modes. */
   glPixelStorei(GL_PACK_SWAP_BYTES, swapbytes);
   glPixelStorei(GL_PACK_LSB_FIRST, lsbfirst);
   glPixelStorei(GL_PACK_ROW_LENGTH, rowlength);
   glPixelStorei(GL_PACK_SKIP_ROWS, skiprows);
   glPixelStorei(GL_PACK_SKIP_PIXELS, skippixels);
   glPixelStorei(GL_PACK_ALIGNMENT, alignment);

   wglMakeCurrent(NULL, NULL) ;

   return true;
}


//
// Name :        COpenGLWnd::FormattedErrorAfxMsgBox()
// Description : Output an error message to a message box based on
//               the GetLastError value.
//

int COpenGLWnd::FormattedErrorAfxMsgBox(const char *p_msg)
{
    LPVOID lpMsgBuf;
    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        0, // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL 
        );

    ostrstream str;
    str << p_msg << " " << (char *)lpMsgBuf << ends;
    
    // Free the buffer.
    LocalFree( lpMsgBuf );

#ifdef UNICODE
	CComBSTR bstr;
	bstr = str.str();
    AfxMessageBox(bstr, MB_OK | MB_ICONSTOP);
#else
    AfxMessageBox(str.str(), MB_OK | MB_ICONSTOP);
#endif
    delete str.str();

    return -1;
}



