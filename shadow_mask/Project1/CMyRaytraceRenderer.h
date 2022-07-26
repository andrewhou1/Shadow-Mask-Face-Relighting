#pragma once
#include <cassert>
#include <fstream>
#include <vector>
#include <strstream>
#include <string>
#include "graphics/GrRenderer.h"
#include "graphics/RayIntersection.h"

class CMyRaytraceRenderer :
	public CGrRenderer
{
public:
    CMyRaytraceRenderer() { m_window = NULL; }
    int     m_rayimagewidth;
    int     m_rayimageheight;
    BYTE** m_rayimage;
    void SetImage(BYTE** image, int w, int h) { m_rayimage = image; m_rayimagewidth = w;  m_rayimageheight = h; }
    void SaveImage(const std::string& imagename);

    std::string imagename;
    void SetImageName(const std::string& filename) { imagename = filename; }

    CWnd* m_window;

    CRayIntersection m_intersection;

    std::list<CGrTransform> m_mstack;
    CGrMaterial* m_material;

    std::vector<CGrPoint> m_light;
    CGrPoint m_translation;
    
    void SetWindow(CWnd* p_window);
    void SetLighting(std::vector<CGrPoint>& lighting) { m_light.clear();  m_light = lighting; }
    void SetTranslationVector(CGrPoint& t) { m_translation = t; }
    bool RendererStart();
    bool RendererEnd();
    void RendererMaterial(CGrMaterial* p_material);

    virtual void RendererPushMatrix();
    virtual void RendererPopMatrix();
    virtual void RendererRotate(double a, double x, double y, double z);
    virtual void RendererTranslate(double x, double y, double z);
    void RendererEndPolygon();
};

