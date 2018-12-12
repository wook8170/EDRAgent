
// EDRTesterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EDRTester.h"
#include "EDRTesterDlg.h"
#include "afxdialogex.h"

#include "Util\Pipe.h"

typedef enum
{
	MethodInvalid = -1,
	Get = 0,
	Post,
	Delete,
} EMethod;

typedef enum
{
	PolicyInvalid = -1,
	PolicyIOCRefresh,
	PolicyIOC,
	PolicyRESRefresh,
	PolicyRESProcess,
	PolicyRESFile,
	PolicyRESRegistry,
	PolicyRESNetwork,
	PolicyMAX,
} EEDRPolicyType;

typedef enum
{
	ManagerInvalid = -1,
	ManagerIOC,
	ManagerRES,
	ManagerMAX,
} EEDRManagerType;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEDRTesterDlg dialog

CPipe	pipe;

CEDRTesterDlg::CEDRTesterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEDRTesterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	if (pipe.Create(
		"PIPE_EDRAGENT"
		, PIPE_READ | PIPE_WRITE
		, FALSE
		)
	)
	{
		LOGI << "IPC 서버 시작 !";
	}
	else
	{
		LOGW << "IPC 서버 초기화 실패 ( 0x" << hex << pipe.GetLastError() << " ) !";
	}
}

void CEDRTesterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEDRTesterDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_IOC_REFRESH, &CEDRTesterDlg::OnBnClickedButtonIocRefresh)
	ON_BN_CLICKED(IDC_BUTTON_IOC_UPDATE, &CEDRTesterDlg::OnBnClickedButtonIocUpdate)
	ON_BN_CLICKED(IDC_BUTTON_RES_REFRESH, &CEDRTesterDlg::OnBnClickedButtonResRefresh)
	ON_BN_CLICKED(IDC_BUTTON_RES_PROCESS, &CEDRTesterDlg::OnBnClickedButtonResProcess)
	ON_BN_CLICKED(IDC_BUTTON_RES_FILE, &CEDRTesterDlg::OnBnClickedButtonResFile)
	ON_BN_CLICKED(IDC_BUTTON_RES_REGISTRY, &CEDRTesterDlg::OnBnClickedButtonResRegistry)
	ON_BN_CLICKED(IDC_BUTTON_RES_NETWORK, &CEDRTesterDlg::OnBnClickedButtonResNetwork)
END_MESSAGE_MAP()


// CEDRTesterDlg message handlers

BOOL CEDRTesterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEDRTesterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEDRTesterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CEDRTesterDlg::OnBnClickedButtonIocRefresh()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	IPCMessage_T message;

	message.type = PolicyIOCRefresh;

	sprintf_s(message.message, MESSAGE_BUFFER_SIZE, "%s", "IOC Refresh");

	BOOL bRet = pipe.WritePipe(&message, IPCMessage_SIZE);
}


void CEDRTesterDlg::OnBnClickedButtonIocUpdate()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	IPCMessage_T message;

	message.type = PolicyIOC;

	sprintf_s(message.message, MESSAGE_BUFFER_SIZE, "%s", "IOC Update");

	BOOL bRet = pipe.WritePipe(&message, IPCMessage_SIZE);
}


void CEDRTesterDlg::OnBnClickedButtonResRefresh()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	IPCMessage_T message;
	
	message.type = PolicyRESRefresh;

	sprintf_s(message.message, MESSAGE_BUFFER_SIZE, "%s", "RES Refresh");

	BOOL bRet = pipe.WritePipe(&message, IPCMessage_SIZE);
}


void CEDRTesterDlg::OnBnClickedButtonResProcess()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	IPCMessage_T message;

	message.type = PolicyRESProcess;

	sprintf_s(message.message, MESSAGE_BUFFER_SIZE, "%s", "RES Process");

	BOOL bRet = pipe.WritePipe(&message, IPCMessage_SIZE);
}


void CEDRTesterDlg::OnBnClickedButtonResFile()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	IPCMessage_T message;

	message.type = PolicyRESFile;

	sprintf_s(message.message, MESSAGE_BUFFER_SIZE, "%s", "RES File");

	BOOL bRet = pipe.WritePipe(&message, IPCMessage_SIZE);
}


void CEDRTesterDlg::OnBnClickedButtonResRegistry()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	IPCMessage_T message;

	message.type = PolicyRESRegistry;

	sprintf_s(message.message, MESSAGE_BUFFER_SIZE, "%s", "RES Registry");

	BOOL bRet = pipe.WritePipe(&message, IPCMessage_SIZE);
}


void CEDRTesterDlg::OnBnClickedButtonResNetwork()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	IPCMessage_T message;

	message.type = PolicyRESNetwork;

	sprintf_s(message.message, MESSAGE_BUFFER_SIZE, "%s", "RES Network");

	BOOL bRet = pipe.WritePipe(&message, IPCMessage_SIZE);
}
