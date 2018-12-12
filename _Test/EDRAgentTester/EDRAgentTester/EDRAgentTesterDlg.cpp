
// EDRAgentTesterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EDRAgent.h"
#include "EDRAgentTester.h"
#include "EDRAgentTesterDlg.h"
#include "afxdialogex.h"

#ifdef _WIN64
#pragma comment( lib, "..\\..\\..\\bin\\release\\amd64\\EDRAgentDll_64.lib" )
#else
#pragma comment( lib, "..\\..\\..\\bin\\release\\i386\\EDRAgentDll_32.lib" )
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//HMODULE g_hDLL = NULL;


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CEDRAgentTesterDlg dialog



CEDRAgentTesterDlg::CEDRAgentTesterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEDRAgentTesterDlg::IDD, pParent)
	, m_nType(0)
	, m_strEdit(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEDRAgentTesterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_nType);
	DDX_Text(pDX, IDC_EDIT1, m_strEdit);
}

BEGIN_MESSAGE_MAP(CEDRAgentTesterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CEDRAgentTesterDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CEDRAgentTesterDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CEDRAgentTesterDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CEDRAgentTesterDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CEDRAgentTesterDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CEDRAgentTesterDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CEDRAgentTesterDlg::OnBnClickedButton6)
END_MESSAGE_MAP()


// CEDRAgentTesterDlg message handlers

BOOL CEDRAgentTesterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEDRAgentTesterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEDRAgentTesterDlg::OnPaint()
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
HCURSOR CEDRAgentTesterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CEDRAgentTesterDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//CDialogEx::OnOK();
	UpdateData();
	CString strMsg;
	strMsg.Format(_T("%d"), m_nType);
	AfxMessageBox(strMsg);
}


BOOL CEDRAgentTesterDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		if (GetDlgItem(IDC_EDIT1) == GetFocus())
		{
			CEdit* edit = (CEdit*)GetDlgItem(IDC_EDIT1);
			int nLen = edit->GetWindowTextLength();
			edit->SetSel(nLen, nLen);
			edit->ReplaceSel(_T("\r\n"));
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CEDRAgentTesterDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	if(InitDrv(m_nType + 1))
	{
		AfxMessageBox(_T("InitDrv() Success."));
	}
	else
	{
		AfxMessageBox(_T("InitDrv() Failed."));
	}	
}


void CEDRAgentTesterDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	if(DeInitDrv(m_nType + 1))
	{
		AfxMessageBox(_T("DeInitDrv() Success."));
	}
	else
	{
		AfxMessageBox(_T("DeInitDrv() Failed."));
	}	
}


void CEDRAgentTesterDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	if(SetFullList(m_nType + 1, (LPTSTR)(LPCTSTR)m_strEdit, m_strEdit.GetLength() + 1))
	{
		AfxMessageBox(_T("SetFullList() Success."));
	}
	else
	{
		AfxMessageBox(_T("SetFullList() Failed."));
	}	
}


void CEDRAgentTesterDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	if(Clear(m_nType + 1))
	{
		AfxMessageBox(_T("Clear() Success."));
	}
	else
	{
		AfxMessageBox(_T("Clear() Failed."));
	}	
}


void CEDRAgentTesterDlg::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	if(Add(m_nType + 1, (LPTSTR)(LPCTSTR)m_strEdit, m_strEdit.GetLength() + 1))
	{
		AfxMessageBox(_T("Add() Success."));
	}
	else
	{
		AfxMessageBox(_T("Add() Failed."));
	}	
}


void CEDRAgentTesterDlg::OnBnClickedButton6()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	
	if(Remove(m_nType + 1, (LPTSTR)(LPCTSTR)m_strEdit, m_strEdit.GetLength() + 1))
	{
		AfxMessageBox(_T("Remove() Success."));
	}
	else
	{
		AfxMessageBox(_T("Remove() Failed."));
	}	
}
