﻿
// EDRAgent.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "EDRAgentWnd.h"		// main symbols

// CEDRAgentApp:
// See EDRAgent.cpp for the implementation of this class
//

class CEDRAgentApp : public CWinApp
{
public:
	CEDRAgentWnd * m_pMainWindow;
	
public:
	CEDRAgentApp();

	~CEDRAgentApp();

	// Overrides
public:
	virtual BOOL InitInstance();

	// Implementation

	DECLARE_MESSAGE_MAP()
};

//#include "EDRAgent.h"
//extern CEDRAgentApp theApp;