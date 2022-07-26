//
// Name :         GrVRMLFactory.h
// Description :  Header for CGrVRMLFactory
//                Object File Format (VRML) file loader.
// Author :       Charles B. Owen
//

#if !defined(GRVRMLFACTOR_H)
#define GRVRMLFACTOR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>
#include "GrObject.h"
#include "libvrml.h"

// The scene graph node for VRML objects

// class CGrVRML
// Class for a single VRML object

class CGrVRML : public CGrObject, public CVRML::Renderer
{
public:
    CGrVRML();
    virtual ~CGrVRML();

    virtual void glRender();
    void Render(CGrRenderer *p_renderer);

    bool Load(const char *p_file);

private:
    // These are the renderer callback functions from the VRML library
    virtual void PolygonBegin();
    virtual void PolygonEnd();
    virtual void Vertex(float x, float y, float z);
    virtual void Normal(float x, float y, float z);
    virtual void TexCoord(float s, float t);
    virtual void PushMatrix();
    virtual void PopMatrix();
    virtual void Translate(float x, float y, float z);
    virtual void Rotate(float a, float x, float y, float z);
    virtual void Scale(float x, float y, float z);
    virtual void MultMatrix(const double *m);
    virtual void Material(const float *ambient, const float *diffuse, const float *specular, const float *emissive, float shininess);
    virtual void Texture(int index);

    CVRML        m_vrml;        // The underlying actual VRML object
    CGrRenderer *m_renderer;    // Current renderer
    int          m_texture;     // Current texture
    std::vector<CGrPtr<CGrTexture> > m_textureCache;
    std::vector<CGrPtr<CGrMaterial> > m_materials;
};



class CGrVRMLFactory  
{
public:
	CGrVRMLFactory();
	virtual ~CGrVRMLFactory();

	bool Load(const char *p_file);


    // Results return
    CGrVRML *SceneGraph() {return m_vrml;}

private:
    // Pointer to the created object
    CGrPtr<CGrVRML>    m_vrml;
};

#endif
