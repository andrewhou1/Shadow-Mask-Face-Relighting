// GrVRMLFactory.cpp: implementation of the CGrVRMLFactory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <string>
#include <fstream>
#include <strstream>
#include <vector>
#include <cassert>

#include "GrPoint.h"
#include "GrVRMLFactory.h"
#include "GrRenderer.h"
#include "GrTexture.h"

using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef UNICODE
// This function converts the C-style strings to 
// unicode strings so we can send them to AfxMessageBox
inline void AfxMessageBox(const char *str, UINT nType = 0, UINT nIDHelp = 0)
{
    size_t len = strlen(str);
    LPWSTR dest = new WCHAR[len + 1];
    int cnt = MultiByteToWideChar(CP_UTF8, 0, str, -1, dest, int(len) + 1);
    AfxMessageBox(dest, nType, nIDHelp);
    delete [] dest;
}
#endif


CGrVRMLFactory::CGrVRMLFactory()
{
}

CGrVRMLFactory::~CGrVRMLFactory()
{

}

bool CGrVRMLFactory::Load(const char *p_file)
{
    // Create a scene graph node
    m_vrml = new CGrVRML;

    // And load into it
    return m_vrml->Load(p_file);
}


//////////////////////////////////////////////////////////////////////
// CGrVRML:  VRML scene graph node class
//////////////////////////////////////////////////////////////////////

CGrVRML::CGrVRML()
{
    m_texture = -1;
}

CGrVRML::~CGrVRML() {}

bool CGrVRML::Load(const char *p_file)
{
    return m_vrml.FileLoad(p_file);
}


//
// Name :         CGrVRML::glRender()
// Description :  Render this VRML object via OpenGL
//

void CGrVRML::glRender()
{
    m_vrml.glRender();
}


void CGrVRML::Render(CGrRenderer *p_renderer)
{
    m_renderer = p_renderer;        // Save this off so we have it for the
                                    // callbacks from the VRML renderer.
    m_texture = -1;                 // No current texture

    // Clear the list of materials
    m_materials.clear();

    // Before we render, we create a texture cache that 
    // makes a local texture object from those in the 
    // VRML object.
    m_textureCache.clear();
    for(int i=0;  i<m_vrml.GetTextureCount();  i++)
    {
        // Obtain information about the texture
        const BYTE *image;
        int width, height;
        int colpitch, rowpitch;
        bool repeatS, repeatT, transparency;
        m_vrml.GetTexture(i, image, width, height, colpitch, rowpitch, repeatS, repeatT, transparency);

        // Create the local scene graph node for the texture
        CGrPtr<CGrTexture> texture = new CGrTexture;
        texture->LoadMemory(image, width, height, colpitch, rowpitch, repeatS, repeatT, transparency);

        m_textureCache.push_back(texture);
    }

    m_vrml.Render(this);
}


void CGrVRML::Texture(int index)
{
    m_texture = index;

}


void CGrVRML::PolygonBegin()
{
    m_renderer->RendererNormalize(true);        // Have to autonormalize, since VRML objects often scale
    m_renderer->RendererBeginPolygon();

    if(m_texture >= 0)
        m_renderer->RendererTexture(m_textureCache[m_texture]);
}

void CGrVRML::PolygonEnd()
{
    m_renderer->RendererEndPolygon();
    m_renderer->RendererNormalize(false);

    m_renderer->RendererTexture(NULL);
}

void CGrVRML::Vertex(float x, float y, float z)
{
    m_renderer->RendererVertex(CGrPoint(x, y, z));
}


void CGrVRML::Normal(float x, float y, float z)
{
    m_renderer->RendererNormal(CGrPoint(x, y, z, 0));
}


void CGrVRML::TexCoord(float s, float t)
{
    m_renderer->RendererTexVertex(CGrPoint(s, t, 0));
}


void CGrVRML::PushMatrix()
{
    m_renderer->RendererPushMatrix();
}

void CGrVRML::PopMatrix()
{
    m_renderer->RendererPopMatrix();
}

void CGrVRML::Translate(float x, float y, float z)
{
    m_renderer->RendererTranslate(x, y, z);
}

void CGrVRML::Rotate(float a, float x, float y, float z)
{
    m_renderer->RendererRotate(a, x, y, z);
}

void CGrVRML::Scale(float x, float y, float z)
{
    CGrTransform s;
    s.SetScale(x, y, z);
    m_renderer->RendererTransform(&s);
}

void CGrVRML::MultMatrix(const double *m)
{
    CGrTransform t;
    for(int c=0;  c<4;  c++)
    {
        for(int r=0;  r<4;  r++)
        {
            t[r][c] = *m++;
        }
    }

    m_renderer->RendererTransform(&t);
}


void CGrVRML::Material(const float *ambient, const float *diffuse, const float *specular, 
              const float *emissive, float shininess)
{
    // Create a material node
    CGrPtr<CGrMaterial> mat = new CGrMaterial;

    // The renderer system only keeps a pointer to the material object
    // and does not put a reference count onto it.  It's actually blind
    // to what the material object actually contains.  So, we keep a pointer to it
    // in this scene graph node as well.
    m_materials.push_back(mat);

    mat->AmbientDiffuseSpecularShininess(ambient, diffuse, specular, shininess);


    // Not doing anything with transparency for now...

    m_renderer->RendererMaterial(mat);
}


