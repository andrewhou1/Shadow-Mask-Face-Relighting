
// ChildView.h : interface of the CChildView class
//


#pragma once
#include "graphics/OpenGLWnd.h"
#include "graphics/GrCamera.h"
#include "graphics/GrObject.h"

// CChildView window

class CChildView : public COpenGLWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	CGrCamera m_camera;

	CGrPtr<CGrObject> m_scene;

	bool m_raytrace;

private:
	BYTE** m_rayimage;
	int         m_rayimagewidth;
	int         m_rayimageheight;

// Operations
public:
	void OnGLDraw(CDC* pDC);
	void ConfigureRenderer(CGrRenderer* p_renderer);
	
// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRenderRaytrace();
	afx_msg void OnUpdateRenderRaytrace(CCmdUI* pCmdUI);
	afx_msg void OnRenderRunexperiment();
	afx_msg void OnRenderDpr();
};

