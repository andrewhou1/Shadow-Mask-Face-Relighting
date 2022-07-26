//
//  Name :         Texture.cpp
//  Description :  Implementation of the CTexture class.
//                 This class allows for the simple manipulation of BITMAP images.
//  Version :      10-23-99 1.01 Declared version number
//                  2-23-03 1.02 Fixed bug where one constructor did not inititalize m_mipinitialized.
//
//

#include "stdafx.h"
#include "Texture.h"
#include <cassert>

using namespace std;

#include <GL/glu.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef UNICODE
#define tstring wstring
#else
#define tstring string
#endif

/*
 * Dib Header Marker - used in writing DIBs to files
 */
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
const int PADSIZE = 4;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTexture::CTexture()
{
   m_height = 0;
   m_width = 0;
   m_image = NULL;
   m_texname = 0;

   m_initialized = false;
   m_mipinitialized = false;
   m_autobmp = false;
}

CTexture::CTexture(const CTexture &p_img)
{
   m_height = 0;
   m_width = 0;
   m_image = NULL;
   m_initialized = false;
   m_mipinitialized = false;

   Copy(p_img);
}

CTexture::~CTexture()
{
   if(m_image)
   {
      delete [] m_image[0];
      delete [] m_image;
   }

}

//////////////////////////////////////////////////////////////////////
// Basic Manipulations
//////////////////////////////////////////////////////////////////////

//
// Name :         CTexture::Copy()
// Description :  Copy another image into this one.
//

void CTexture::Copy(const CTexture &p_img)
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


CTexture & CTexture::operator =(const CTexture &p_img)
{
   Copy(p_img);
   return *this;
}

//
// Name :         CTexture::TexName()
// Description :  Obtain the texture name.  If the texture name has
//                not yet been assigned, do so, now.
//


GLuint CTexture::TexName()
{
   if(m_initialized)
      return m_texname;

   if(m_image == NULL)
       return 0;

   glGenTextures(1, &m_texname);
   glBindTexture(GL_TEXTURE_2D, m_texname);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0,
     GL_BGR_EXT, GL_UNSIGNED_BYTE, m_image[0]);

   m_initialized = true;

   return m_texname;
}


GLuint CTexture::MipTexName()
{
    if(m_mipinitialized)
        return m_miptexname;

    if(m_image == NULL)
        return 0;

    glGenTextures(1, &m_miptexname);
    glBindTexture(GL_TEXTURE_2D, m_miptexname);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, m_width, m_height, GL_BGR_EXT, GL_UNSIGNED_BYTE, m_image[0]);

    m_mipinitialized = true;

    return m_miptexname;

}

void CTexture::SameSize(const CTexture &p_img)
{
	SetSize(p_img.m_width, p_img.m_height);
}

//
// Name :         CTexture::SetSize()
// Description :  Sets the size of the image and allocates memory.
//                An image size of 0 is an empty empty.
//

void CTexture::SetSize(int p_x, int p_y)
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
   m_mipinitialized = false;

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

void CTexture::Set(int x, int y, int r, int g, int b)
{
   if(x >= 0 && x < m_width && y >= 0 && y < m_height)
   {
      BYTE *img = m_image[y] + x * 3;
      *img++ = b;
      *img++ = g;
      *img++ = r;
   }
}


void CTexture::Fill(int r, int g, int b)
{
   for(int i=0;  i<m_height;  i++)
   {
      BYTE *img = m_image[i];
      for(int j=0;  j<m_width * 3;  j+=3)
      {
         *img++ = b;
         *img++ = g;
         *img++ = r;
      }

   }

}

//////////////////////////////////////////////////////////////////////
// Generic file and memory reading operations
//////////////////////////////////////////////////////////////////////



//
//  Name :         CTexture::LoadFile()
//  Description :  Load this image from a file of type BMP or PPM
//

bool CTexture::LoadFile(LPCTSTR pathName)
{
    tstring filename = pathName;
    if(m_autobmp)
    {
        // Automatically set the suffix to .bmp, no matter
        // what it currently is.
        for(int i=(int)filename.length() - 1;  i>= 0;  i--)
        {
            if(filename[i] == TEXT('.'))
            {
                filename.resize(i);
                filename += TEXT(".bmp");
                break;
            }
            else if(filename[i] == TEXT('/') || filename[i] == TEXT('\\'))
            {
                filename += TEXT(".bmp");
                break;
            }
        }
    }

    // Open the file we are reading from.  Note that
    // I'm opening in binary.

    ifstream file(filename.c_str(), ios::binary);
    if(!file)
    {
        tstring msg = TEXT("Unable to open image file: ");
        msg += filename;
        AfxMessageBox(msg.c_str());
        return false;
    }

    BYTE begin[20];
    file.read((char *)begin, sizeof(begin));
    if(!file)
    {
        tstring msg = TEXT("Unsupported texture file type: ");
        msg += filename;
        AfxMessageBox(msg.c_str());
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
        tstring msg = TEXT("Unsupported texture file type: ");
        msg += filename;
        AfxMessageBox(msg.c_str());
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// BMP file operations
//////////////////////////////////////////////////////////////////////


//
//  Name :         CTexture::ReadDIBFile()
//  Description :  Load a BMP file.
//

bool CTexture::ReadDIBFile(istream &file)
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
      AfxMessageBox(TEXT("Unsupported image file type"));
      return false;
   }

	if (bmfHeader.bfType != DIB_HEADER_MARKER)
   {
      AfxMessageBox(TEXT("Note a BMP file"));
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
      AfxMessageBox(TEXT("Premature end of file in image file"));
      return false;
   }

   if(pBMI->biHeight < 0 || pBMI->biWidth < 0 || pBMI->biCompression != BI_RGB)
   {
      delete (BYTE *)pBMI;
      AfxMessageBox(TEXT("Unsupported file type"));
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
            *row++ = bmiColors[*img].rgbBlue;
            *row++ = bmiColors[*img].rgbGreen;
            *row++ = bmiColors[*img++].rgbRed;
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
         for(c=0;  c<m_width*3;  c++)
         {
            // Guess what:  Microsoft stores images as BGR, not RGB
            *row++ = *img++;
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
            *row++ = *img++;
            *row++ = *img++;
            *row++ = *img++;
            img++;
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

bool CTexture::ReadPPMFile(istream &file)
{
   char c1, c2;
   file >> c1 >> c2;
   if(c1 != 'P' || c2 != '6')
   {
      AfxMessageBox(TEXT("Invalid file type!"));
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
         file.read((char *)&m_image[r][c * 3 + 2], 1);
         file.read((char *)&m_image[r][c * 3 + 1], 1);
         file.read((char *)&m_image[r][c * 3 + 0], 1);
      }
   }
   
   return true;
}



