// libRayIntersection.h : main header file for the libRayIntersection DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// ClibRayIntersectionApp
// See libRayIntersection.cpp for the implementation of this class
//

class ClibRayIntersectionApp : public CWinApp
{
public:
	ClibRayIntersectionApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
