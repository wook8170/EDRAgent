
// EDRAgent.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#include "EDRAgent.h"
#include "EDRPolicyManager.h"
#include "EDRWorkerThread.h"
#include "EDRShellNotifyDlg.h"
#include "EDRPolicyHashList.h"

#include "IOCPolicyHelperImpl.h"
#include "RESPolicyHelperImpl.h"
#include "EDRPolicyHelperBase.h"
#include "ThreadPool/AsyncCallback.h"
#include "Util/Util.h"
#include "Util/Process.h"
#include "Util/URLParser.h"
#include "Util/Registry.h"
#include "Util/Pipe.h"

#include <stdio.h>
#include <stdlib.h>  
#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>

#include <Util\SelfDump.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CEDRAgentApp theApp;
CPipe	pipe;

//CEDRWorkerThread ThreadIOCProcess("PolicyIOC Process");

struct CharData { CHAR* data; };


// CEDRAgentApp

BEGIN_MESSAGE_MAP(CEDRAgentApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CEDRAgentApp construction

//CSelfDump g_cSelfDump();

CEDRAgentApp::CEDRAgentApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	FileSystem::Directory::Create("Log");
	FileSystem::Directory::Create("Dump");
#ifdef _DEBUG
	File::Remove(".\\Log\\EDRAgent.log");;
#endif
	plog::init(plog::verbose, ".\\Log\\EDRAgent.log", 1024 * 1024 * 10, 10);
	CSelfDump::RegisterExceptionFilter(NULL, MiniDumpValidTypeFlags, 1024 * 1024 * 100, ".\\Dump");

	LOGI << "================================================================================================";
	LOGI << "##########  #########    ########         ##       ########   ##########  ###     ##  ##########";
	LOGI << "##          ##      ##   ##      ##     ######    ##      ##  ##          ####    ##      ##    ";
	LOGI << "##          ##       ##  ##      ##    ##    ##   ##          ##          ## ##   ##      ##    ";
	LOGI << "########    ##       ##  ########     ##########  ##    ####  #########   ##  ##  ##      ##    ";
	LOGI << "##          ##       ##  ##    ##     ##      ##  ##      ##  ##          ##   ## ##      ##    ";
	LOGI << "##          ##      ##   ##     ##    ##      ##  ##      ##  ##          ##    ####      ##    ";
	LOGI << "##########  ########     ##      ##   ##      ##   ########   ##########  ##     ###      ##    ";
	LOGI << "------------------------------------------------------------------------------------------------";
	LOGI << "EDRAgent START !!";
	LOGI << "DATE: " << CUtil::CurrentDateTime();
	LOGI << "================================================================================================";
	LOGI << StringNull;
}

CEDRAgentApp::~CEDRAgentApp()
{
	pipe.Close();
}


// The one and only CEDRAgentApp object

PIPE_CALLBACK PipeCallback(LPVOID pParam, LPVOID Buffer, DWORD dwLength)
{
	LOGI << "IPC 서버 콜백 호출 !";

	CPipe *pPipe = (CPipe*)pParam;

	PIPCMessage_T message = (IPCMessage_T*)Buffer;

	LOGI << "Type   : " << message->type;
	LOGI << "Message: " << message->message;

	theApp.m_pMainWnd->PostMessage(
		WM_IOC_MESSAGE
		, (WPARAM)message->type
		, (LPARAM)message->message
	);

	return 0;
}
// CEDRAgentApp initialization

BOOL CEDRAgentApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = EDRNew CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	DWORD dwStyle = WS_EX_TOOLWINDOW;
	m_pMainWindow = EDRNew CEDRAgentWnd;
	m_pMainWindow->CreateEx(dwStyle, AfxRegisterWndClass(NULL, NULL, NULL, NULL), _T("EDRAgentWnd"), WS_POPUP, 0, 0, 0, 0, NULL, NULL);

	//CIOCPolicyHelperImpl IOCPolicyHelperImpl("IOC Policy Helper");
	//CRESPolicyHelperImpl RESPolicyHelperImpl("RES Policy Helper");

	m_pMainWindow->Init();
	m_pMainWnd = m_pMainWindow;

	CRegistry reg;
	reg.Open(HKEY_LOCAL_MACHINE, "Software\\{E91885F1-84E4-11d5-898D-0008C725AC74}");
	string subkey;

	reg.BeginEnumKey(CRegistry::EnumForDelete);
	while (reg.EnumKey(subkey) == ERROR_SUCCESS)
	{
		reg.DeleteKey(subkey);
	}
	reg.EndEnumKey();

	if (pipe.Create(
		"PIPE_EDRAGENT"
		, PIPE_READ | PIPE_WRITE
		, TRUE
		, (PIPE_CALLBACK)PipeCallback
		, &pipe)
		)
	{
		LOGI << "IPC 서버 시작 !";
	}
	else
	{
		LOGW << "IPC 서버 초기화 실패 ( 0x" << hex << pipe.GetLastError() << " ) !";
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		EDRDelete(pShellManager);
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.

	return TRUE;
}

