//
// Name :         OpenGLRenderer.h
// Description :  Header for COpenGLRenderer
//                Plug-in renderer fro OpenGL
//                See OpenGLRenderer.cpp
// Author :       Charles B. Owen
//

#if !defined(AFX_OPENGLRENDERER_H__96078397_F350_4485_A87E_94051B49266B__INCLUDED_)
#define AFX_OPENGLRENDERER_H__96078397_F350_4485_A87E_94051B49266B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GrRenderer.h"

class COpenGLRenderer : public CGrRenderer  
{
public:
	COpenGLRenderer();
	virtual ~COpenGLRenderer();

    virtual bool RendererStart();
    virtual bool RendererEnd();
    virtual void RendererEndPolygon();
    virtual void RendererColor(double *c);
    virtual void RendererMaterial(CGrMaterial *p_material);
    virtual void RendererTranslate(double x, double y, double z);
    virtual void RendererTransform(const CGrTransform *p_transform);
    virtual void RendererRotate(double a, double x, double y, double z);
    virtual void RendererPopMatrix();
    virtual void RendererPushMatrix();

};

#endif // !defined(AFX_OPENGLRENDERER_H__96078397_F350_4485_A87E_94051B49266B__INCLUDED_)
