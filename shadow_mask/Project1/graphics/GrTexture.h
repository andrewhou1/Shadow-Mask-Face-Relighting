//
// Name :         GrTexture.h
// Description :  Header for CTexture, texture image class for OpenGL.
// Notice :       This is a modified version designed to work with the scene graph
//                system.  We can have an actual texture in a scene graph.
// Version :      See Texture.cpp
//

#if !defined(_GRTEXTURE_H)
#define _GRTEXTURE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GrObject.h"
#include <fstream>
#include <GL/gl.h>

class CGrTexture : public CGrObject
{
public:
    CGrTexture();
    CGrTexture(const CGrTexture &p_img);
    virtual ~CGrTexture();

    void glRender();
    virtual void Render(CGrRenderer *p_renderer);

    GLuint TexName();

    bool LoadFile(const _TCHAR *lpszPathName);
    bool LoadMemory(const BYTE *image, int width, int height, 
                    int colpitch, int rowpitch, bool repeatS, bool repeatT, bool transparency);

    void Fill(int r, int g, int b);
    void Set(int x, int y, int r, int g, int b);
    void SetSize(int p_x, int p_y);
    void SameSize(const CGrTexture &p_img);
    void Copy(const CGrTexture &p_img);
    bool Empty() const {return m_width <= 0 || m_height <= 0;}
    CGrTexture &operator=(const CGrTexture &p_img);

    BYTE *operator[](int i) {return m_image[i];}
    const BYTE *operator[](int i) const {return m_image[i];}
    BYTE *Row(int i) {return m_image[i];}
    const BYTE *Row(int i) const {return m_image[i];}

    int Width() const {return m_width;}
    int Height() const {return m_height;}
    BYTE *ImageBits() const {return m_image[0];}

private:
    bool ReadDIBFile(std::istream &file);
    bool ReadPPMFile(std::istream &file);

    bool    m_initialized;
    bool    m_mipmap;
    GLuint  m_texname;
    int     m_height;
    int     m_width;
    BYTE  **m_image;
};

#endif 
