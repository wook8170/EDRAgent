// EasyAppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EDRShellNotifyDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static CHAR THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEDRShellNotifyDlg dialog
CEDRShellNotifyDlg::CEDRShellNotifyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEDRShellNotifyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEDRShellNotifyDlg)
	//}}AFX_DATA_INIT
	_bVisible = FALSE; // use this to hide the dialog on start
}

CEDRShellNotifyDlg::~CEDRShellNotifyDlg()
{
	Shell_NotifyIcon(NIM_DELETE, &_tnd); // delete from the status area
}

VOID CEDRShellNotifyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEDRShellNotifyDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEDRShellNotifyDlg, CDialog)
	//{{AFX_MSG_MAP(CEDRShellNotifyDlg)
	ON_BN_CLICKED(ID_SHELL_NOTIFY_MINIMIZE, OnHideApp)
	ON_WM_QUERYDRAGICON()
	ON_WM_WINDOWPOSCHANGING()
	//ON_MESSAGE( WM_TRAY_NOTIFY, OnTrayNotify )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEDRShellNotifyDlg message handlers
BOOL CEDRShellNotifyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	_ToolTipCtrl.Create(this,
		// the ToolTip control's style 
		TTS_NOPREFIX | // prevents the system from stripping the ampersand (&) 
					   // character from a string

		TTS_BALLOON | // the ToolTip control has the appearance of
		// 0x40        // a cartoon "balloon," with rounded corners 
					   // and a stem pointing to the item. 

		TTS_ALWAYSTIP  // the ToolTip will appear when the
					   // cursor is on a tool, regardless of 
					   // whether the ToolTip control's owner
					   // window is active or inactive
	);

	if (SetIconAndTitleForBalloonTip(&_ToolTipCtrl, TTI_INFO, _T("EDR Agent - (Block)")))
	{
		//register tools with the ToolTip control
		_ToolTipCtrl.AddTool(GetDlgItem(ID_SHELL_NOTIFY_MINIMIZE), _T("balloon on ID_SHELL_NOTIFY_MINIMIZE"));
		_ToolTipCtrl.AddTool(GetDlgItem(IDCANCEL), _T("balloon on IDCANCEL"));
		_ToolTipCtrl.AddTool(GetDlgItem(IDC_SHELL_NOTIFY_STATIC_TEXT), _T("balloon on STATIC\n(don't forget to add <Notify> to StaticCtrl Styles)"));
	}
	else
	{
		//......
	}

	//if (m_pAlarmMessage)
	{
		LoadToTray(this,
			WM_TRAY_NOTIFY, // user defined 
			m_AlarmMessage.szTitle,
			m_AlarmMessage.szInfo,
			m_AlarmMessage.szTip,
			10, //sec
			theApp.LoadIcon(IDR_SHELL_NOTIFY_MAINFRAME));
	}
	/*
	else
	{
		LoadToTray(this,
			WM_TRAY_NOTIFY, // user defined
			"EDR Agent - (block)",
			"string 1 - body text",
			"EDR Agent - (block)",
			10, //sec
			theApp.LoadIcon(IDR_SHELL_NOTIFY_MAINFRAME));
	}
	*/
	return TRUE;
}

BOOL CEDRShellNotifyDlg::SetIconAndTitleForBalloonTip(CToolTipCtrl *pToolTipCtrl, int tti_ICON, CString title)
{
	return ::PostMessage((HWND)pToolTipCtrl->m_hWnd,
		(UINT)TTM_SETTITLE, // Adds a standard icon and title string to a ToolTip    
		(WPARAM)tti_ICON,
		// TTI_NONE    = 0 - no icon 
		// TTI_INFO    = 1 - information icon 
		// TTI_WARNING = 2 - warning icon 
		// TTI_ERROR   = 3 - error icon 
		(LPARAM)(LPCTSTR)title);
}

VOID CEDRShellNotifyDlg::LoadToTray(CWnd    *pWnd,
	UINT	  uCallbackMessage,
	CString sInfoTitle, // title for a balloon ToolTip.
						// This title appears in boldface above the text.
						// It can have a maximum of 63 characters
	CString sInfo, // the text for a balloon ToolTip, it can have
				   // a maximum of 255 characters
	CString sTip, // the text for a standard ToolTip. 
				  // It can have a maximum of 128 characters, 
				  // including the terminating NULL.
	int     uTimeout, // in sec.
	HICON	  icon)
{
	//NOTIFYICONDATA contains information that the system needs to process taskbar status area messages

	ZeroMemory(&_tnd, sizeof(NOTIFYICONDATA));

	_tnd.cbSize = sizeof(NOTIFYICONDATA);
	_tnd.hWnd = pWnd->GetSafeHwnd();
	_tnd.uID = 0;

	_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_INFO | NIF_REALTIME;
	// Flag Description:
	// - NIF_ICON	 The hIcon member is valid.  
	// - NIF_MESSAGE The uCallbackMessage member is valid. 
	// - NIF_TIP	 The szTip member is valid. 
	// - NIF_STATE	 The dwState and dwStateMask members are valid. 
	// - NIF_INFO	 Use a balloon ToolTip instead of a standard ToolTip. The szInfo, uTimeout, szInfoTitle, and dwInfoFlags members are valid. 
	// - NIF_GUID	 Reserved. 

	_tnd.dwInfoFlags = NIIF_WARNING | NIIF_RESPECT_QUIET_TIME; // add an icon to a balloon ToolTip
			// Flag Description 
			// - NIIF_ERROR     An error icon. 
			// - NIIF_INFO      An information icon. 
			// - NIIF_NONE      No icon. 
			// - NIIF_WARNING   A warning icon. 
			// - NIIF_ICON_MASK Version 6.0. Reserved. 
			// - NIIF_NOSOUND   Version 6.0. Do not play the associated sound. Applies only to balloon ToolTips 

	_tnd.uCallbackMessage = uCallbackMessage;
	_tnd.uTimeout = uTimeout * 1000;
	_tnd.hIcon = icon;

	strcpy_s(_tnd.szInfoTitle, sizeof(_tnd.szInfoTitle), sInfoTitle);
	strcpy_s(_tnd.szInfo, sizeof(_tnd.szInfo), sInfo);
	strcpy_s(_tnd.szTip, sizeof(_tnd.szTip), sTip);

	Shell_NotifyIcon(NIM_ADD, &_tnd); // add to the taskbar's status area
}

VOID CEDRShellNotifyDlg::OnHideApp()
{
	theApp.HideApplication();
}

LONG CEDRShellNotifyDlg::OnTrayNotify(WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
	case WM_LBUTTONDBLCLK: //on double-click the left mouse button restore the dialog
		_bVisible = TRUE;
		this->ShowWindow(SW_RESTORE);
		break;
	}

	return (0);
}

BOOL CEDRShellNotifyDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_MOUSEMOVE)
	{
		_ToolTipCtrl.RelayEvent(pMsg); // pass the mouse message to the ToolTip control for processing
	}

	// NO ESCAPE or RETURN key	
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN ||
			pMsg->wParam == VK_ESCAPE)
		{
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);

			return TRUE;				// DO NOT process further
		}
	}

	// NO ALT+ key
	if (pMsg->message == WM_SYSCOMMAND)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

VOID CEDRShellNotifyDlg::SetMessage(AlarmMessage* pAlarmMessage)
{
	EDRZeroMemory(&m_AlarmMessage, sizeof(AlarmMessage));

	EDRCopyMemory(
		&m_AlarmMessage.szTitle
		, pAlarmMessage->szTitle
		, strlen(pAlarmMessage->szTitle)
	);
	EDRCopyMemory(
		&m_AlarmMessage.szInfo
		, pAlarmMessage->szInfo
		, strlen(pAlarmMessage->szInfo)
	);
	EDRCopyMemory(
		&m_AlarmMessage.szTip
		, pAlarmMessage->szTitle
		, min(128, strlen(pAlarmMessage->szTitle))
	);
}

// (MSDN) The framework calls this member function when the size, position, 
// or Z-order is about to change as a result of a call to theSetWindowPos member
// function or another window-management function
VOID CEDRShellNotifyDlg::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos)
{
	if (!_bVisible) //do this only once to hide the dialog on start
	{
		lpwndpos->flags &= ~SWP_SHOWWINDOW;
	}

	CDialog::OnWindowPosChanging(lpwndpos);
}
