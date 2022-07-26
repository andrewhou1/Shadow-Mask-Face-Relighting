
// Project1.h : main header file for the Project1 application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CProject1App:
// See Project1.cpp for the implementation of this class
//

class CProject1App : public CWinApp
{
public:
	CProject1App() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CProject1App theApp;
