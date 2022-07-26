//
//  Name :         GrTexture.cpp
//  Description :  Implementation of the CGrTexture class.
//                 This class allows for the simple manipulation of BITMAP images.
//                 This is a version that allows textures to be includes in a scene graph.
//  Version :      10-23-99 1.01 Declared version number
//                               JPEG loading support
//                  2-11-01 1.02 Scene graph version, no JPEG, PPM and BMP
//                  3-06-01 1.03 Changed to store image in native RGB format.
//                  2-25-03 1.04 Better error messages
//                  4-02-07 1.05 Unicode support (will work both ways, now)
//

#include "pch.h"
#include <sstream>
#include <iomanip>
#include <vector>

#include <gl/glu.h>

#include "GrTexture.h"

using namespace std;

// Unicode support for a basic STL string so this will work either way
#ifdef UNICODE
#define tostringstream wostringstream
#else
#define tostringstream ostringstream
#endif

/*
* Dib Header Marker - used in writing DIBs to files
*/
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
const int PADSIZE = 4;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGrTexture::CGrTexture()
{
    m_height = 0;
    m_width = 0;
    m_image = NULL;
    m_texname = 0;
    m_mipmap = true;

    m_initialized = false;
}

CGrTexture::CGrTexture(const CGrTexture &p_img)
{
    m_height = 0;
    m_width = 0;
    m_image = NULL;
    m_initialized = false;

    Copy(p_img);
}

CGrTexture::~CGrTexture()
{
    if(m_image)
    {
        delete [] m_image[0];
        delete [] m_image;
    }

}

// Textures do not render...
void CGrTexture::glRender()
{
}

void CGrTexture::Render(CGrRenderer *p_renderer)
{
}


//////////////////////////////////////////////////////////////////////
// Basic Manipulations
//////////////////////////////////////////////////////////////////////

//
// Name :         CGrTexture::Copy()
// Description :  Copy another image into this one.
//

void CGrTexture::Copy(const CGrTexture &p_img)
{
    SameSize(p_img);

    for(int i=0;  i<m_height;  i++)
    {
        for(int j=0;  j<m_width * 3;  j++)
        {
            m_image[i][j] = p_img.m_image[i][j];
        }

    }


}


CGrTexture & CGrTexture::operator =(const CGrTexture &p_img)
{
    Copy(p_img);
    return *this;
}

//
// Name :         CGrTexture::TexName()
// Description :  Obtain the texture name.  If the texture name has
//                not yet been assigned, do so, now.
//


GLuint CGrTexture::TexName()
{
    if(m_initialized)
        return m_texname;

    glGenTextures(1, &m_texname);
    glBindTexture(GL_TEXTURE_2D, m_texname);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if(m_mipmap)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, m_image[0]);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0,
        GL_RGB, GL_UNSIGNED_BYTE, m_image[0]);
    }

    m_initialized = true;
 
    return m_texname;
}



void CGrTexture::SameSize(const CGrTexture &p_img)
{
    SetSize(p_img.m_width, p_img.m_height);
}

//
// Name :         CGrTexture::SetSize()
// Description :  Sets the size of the image and allocates memory.
//                An image size of 0 is an empty empty.
//

void CGrTexture::SetSize(int p_x, int p_y)
{
    if(p_x == m_width || m_height == p_y)
        return;

    if(m_image)
    {
        delete [] m_image[0];
        delete [] m_image;
        m_image = NULL;
    }

    // Member variables
    m_height = p_y;
    m_width = p_x;
    m_initialized = false;

    if(p_x <= 0 || p_y <= 0)
        return;

    // Allocate memory for the image.  Note that storage for rows must
    // be on DWORD boundaries.  (or 16 word boundaries?)
    int usewidth = (m_width * 3 + (PADSIZE - 1)) / PADSIZE;
    usewidth *= PADSIZE;

    BYTE *image = new BYTE[usewidth * m_height];
    m_image = new BYTE *[m_height];
    for(int i=0;  i<m_height;  i++, image += usewidth)
    {
        m_image[i] = image;
    }
}

void CGrTexture::Set(int x, int y, int r, int g, int b)
{
    if(x >= 0 && x < m_width && y >= 0 && y < m_height)
    {
        BYTE *img = m_image[y] + x * 3;
        *img++ = r;
        *img++ = g;
        *img++ = b;
    }
}


void CGrTexture::Fill(int r, int g, int b)
{
    for(int i=0;  i<m_height;  i++)
    {
        BYTE *img = m_image[i];
        for(int j=0;  j<m_width * 3;  j+=3)
        {
            *img++ = r;
            *img++ = g;
            *img++ = b;
        }

    }

}

//////////////////////////////////////////////////////////////////////
// Generic file and memory reading operations
//////////////////////////////////////////////////////////////////////

//
// Name :         CGrTexture::LoadMemory()
// Description :  Load a texture image from an existing memory image.
//                It is assumed the image is RGB order.  Might add something
//                to change this later.
//
//                repeatS, repeatT, transparency are ignored at this time
//                and reserved for future expansion.
//

bool CGrTexture::LoadMemory(const BYTE *image, 
                            int width, int height, 
                            int colpitch, int rowpitch, 
                            bool repeatS, bool repeatT, 
                            bool transparency)
{
    // Extract information from the header
    SetSize(width, height);

    int r, c;

    switch(colpitch)
    {
    default:
        break;

    case 1:
        for(r=0;  r<m_height;  r++)
        {
            const BYTE *img = image + r * rowpitch;
            BYTE *row = m_image[r];
            for(c=0;  c<m_width;  c++)
            {
                *row++ = *img;
                *row++ = *img;
                *row++ = *img++;
            }
        }
        break;

    case 3:
        for(r=0;  r<m_height;  r++)
        {
            const BYTE *img = image + r * rowpitch;
            BYTE *row = m_image[r];
            for(c=0;  c<m_width;  c++)
            {
                *row++ = *img++;
                *row++ = *img++;
                *row++ = *img++;
            }
        }
        break;

    case 4:
        for(r=0;  r<m_height;  r++)
        {
            const BYTE *img = image + r * rowpitch;
            BYTE *row = m_image[r];
            for(c=0;  c<m_width;  c++)
            {
                *row++ = *img++;
                *row++ = *img++;
                *row++ = *img++;
                img++;
            }
        }
        break;


    }

    return true;
}



//
//  Name :         CGrTexture::LoadFile()
//  Description :  Load this image from a file of type BMP or PPM
//

bool CGrTexture::LoadFile(const _TCHAR *pFilename)
{
    string filename;

#ifdef UNICODE
    size_t len = _tcslen(pFilename);
    vector<char> vfilename;
    vfilename.resize(len + 1);
    int cnt = WideCharToMultiByte(CP_UTF8, 0, pFilename, -1, &vfilename[0], int(len) + 1, NULL, NULL);
    filename = &vfilename[0];
#else
   filename = pFilename;
#endif
    // Open the file we are reading from.  Note that
    // I'm opening in binary.  Must use the 8-bit function.

    ifstream file(filename.c_str(), ios::binary);
    if(!file)
    {
        tostringstream str;
        str << _T("Unable to open image file: ") << pFilename << ends;
        AfxMessageBox(str.str().c_str());
        return false;
    }

    BYTE begin[20];
    file.read((char *)begin, sizeof(begin));
    if(!file)
    {
        tostringstream str;
        str << _T("Unsupported read file type: ") << pFilename << ends;
        AfxMessageBox(str.str().c_str());
        return false;
    }

    // Rewind the file so load can start at the beginning
    file.seekg(0);


    if(begin[0] == 'B' && begin[1] == 'M')
    {
        // We have a Windows BITMAP file
        return ReadDIBFile(file);
    }
    else if(begin[0] == 'P' && begin[1] == '6')
    {
        // We have a PPM file
        return ReadPPMFile(file);
    }
    else
    {
        tostringstream str;
        str << _T("Unsupported read file type: ") << pFilename << ends;
        AfxMessageBox(str.str().c_str());
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// BMP file operations
//////////////////////////////////////////////////////////////////////


//
//  Name :         CGrTexture::ReadDIBFile()
//  Description :  Load a BMP file.
//

bool CGrTexture::ReadDIBFile(istream &file)
{
    // Variables for loading of BITMAP files
    BITMAPFILEHEADER bmfHeader;

    int nBMISize;
    BITMAPINFOHEADER * pBMI;

    /*
    * Go read the DIB file header and check if it's valid.
    */
    file.read((char *)&bmfHeader, sizeof(bmfHeader));
    if(!file)
    {
        AfxMessageBox(_T("Unsupported image file type"));
        return false;
    }

    if (bmfHeader.bfType != DIB_HEADER_MARKER)
    {
        AfxMessageBox(_T("Note a BMP file"));
        return false;
    }

    // We allocate memory for the bitmapinfo header, which varies in size
    // depending on the palette and/or the version.  We assume it goes from 
    // the current location to the start of the data.
    nBMISize = bmfHeader.bfOffBits - sizeof(bmfHeader);
    pBMI = (BITMAPINFOHEADER *)(new BYTE[nBMISize]);
    file.read((char *)pBMI, nBMISize);
    if(!file)
    {
        delete (BYTE *)pBMI;
        AfxMessageBox(_T("Premature end of file in image file"));
        return false;
    }

    if(pBMI->biHeight < 0 || pBMI->biWidth < 0 || pBMI->biCompression != BI_RGB)
    {
        delete (BYTE *)pBMI;
        AfxMessageBox(_T("Unsupported file type"));
        return false;
    }

    // Extract information from the header
    SetSize(pBMI->biWidth, pBMI->biHeight);

    // We'll need a pointer to the colormap if any
    // It's right after the BITMAPINFOHEADER in memory.
    RGBQUAD *bmiColors = (RGBQUAD *)(((BYTE *)pBMI) + pBMI->biSize);

    int r, c;
    int usewidth1 = (m_width + (PADSIZE - 1)) / PADSIZE;         usewidth1 *= PADSIZE;
    int usewidth3 = (m_width * 3 + (PADSIZE - 1)) / PADSIZE;     usewidth3 *= PADSIZE;
    int usewidth4 = (m_width * 4 + (PADSIZE - 1)) / PADSIZE;     usewidth4 *= PADSIZE;

    // Allocate memory for one row of data from the file.
    // This is worst case allocation, we'll often need less.
    BYTE *rowbuf = new BYTE[usewidth4];
    bool err = false;

    switch(pBMI->biBitCount)
    {
    default:
        err = true;
        break;

    case 8:
        for(r=0;  r<m_height;  r++)
        {
            file.read((char *)rowbuf, usewidth1);
            if(!file)
            {
                err = true;
                break;
            }

            BYTE *img = rowbuf;
            BYTE *row = m_image[r];
            for(c=0;  c<m_width;  c++)
            {
                *row++ = bmiColors[*img].rgbRed;
                *row++ = bmiColors[*img].rgbGreen;
                *row++ = bmiColors[*img++].rgbBlue;
            }
        }
        break;

    case 24:
        for(r=0;  r<m_height;  r++)
        {
            file.read((char *)rowbuf, usewidth3);
            if(!file)
            {
                err = true;
                break;
            }

            BYTE *img = rowbuf;
            BYTE *row = m_image[r];
            for(c=0;  c<m_width;  c++)
            {
                // Guess what:  Microsoft stores images as BGR, not RGB
                *row++ = img[2];
                *row++ = img[1];
                *row++ = img[0];
                img += 3;
            }
        }
        break;

    case 32:
        for(r=0;  r<m_height;  r++)
        {
            file.read((char *)rowbuf, usewidth4);
            if(!file)
            {
                err = true;
                break;
            }

            BYTE *img = rowbuf;
            BYTE *row = m_image[r];
            for(c=0;  c<m_width;  c++)
            {
                // Guess what:  Microsoft stores images as BGR, not RGB
                *row++ = img[2];
                *row++ = img[1];
                *row++ = img[0];
                img+=4;
            }
        }
        break;


    }

    // Free all of the temporary allocations
    delete (BYTE *)pBMI;
    delete [] rowbuf;
    if(err)
    {
        delete m_image[0];
        delete m_image;
        m_image = NULL;
        m_width = 0;
        m_height = 0;
        return false;
    }

    return true;
}


//
// Name :         _ReadSkip()
// Description :  Simple function to read an integer, skipping
//                any PPM comments.
//

static int _ReadSkip(istream &str)
{
    char c;

    // Grab the first non-whitespace character...
    str >> c;

    while(c == '#')
    {
        // We know this is enough because lines can't be larger than 
        // 76 characters by the rules of the game.
        str.ignore(80, '\n');
        str >> c;
    }

    // Put the character back so we can read as an integer...
    str.putback(c);
    int i;
    str >> i;
    return i;
}

bool CGrTexture::ReadPPMFile(istream &file)
{
    char c1, c2;
    file >> c1 >> c2;
    if(c1 != 'P' || c2 != '6')
    {
        AfxMessageBox(_T("Invalid file type!"));
        return false;
    }

    int w = _ReadSkip(file);
    int h = _ReadSkip(file);
    int maxval = _ReadSkip(file);

    // Read over the newline character after
    // the last integer.
    char c;
    file.read(&c, 1);

    SetSize(w, h);

    for(int r=h-1; r>=0; r--)
    {
        // Byte reversal
        for(int c=0;  c<w;  c++)
        {
            file.read((char *)&m_image[r][c * 3 + 0], 1);
            file.read((char *)&m_image[r][c * 3 + 1], 1);
            file.read((char *)&m_image[r][c * 3 + 2], 1);
        }
    }

    return true;
}




