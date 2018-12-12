#pragma once

#include "EDRAgent.h"
#include "resource.h"		// main symbols

#define WM_TRAY_NOTIFY WM_APP + 1000

class CEDRAgentApp;

extern CEDRAgentApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CEDRShellNotifyDlg dialog
class CEDRShellNotifyDlg : public CDialog
{
	// Construction
public:
	CEDRShellNotifyDlg(CWnd* pParent = NULL);	// standard constructor
	~CEDRShellNotifyDlg();

	// Dialog Data
		//{{AFX_DATA(CEDRShellNotifyDlg)
	enum { IDD = IDD_SHELLNOTIFY_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEDRShellNotifyDlg)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	VOID		 SetMessage(AlarmMessage* pAlarmMessage);
protected:
	virtual VOID DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	CToolTipCtrl   _ToolTipCtrl;
	NOTIFYICONDATA _tnd;
	BOOL           _bVisible;
	AlarmMessage_T m_AlarmMessage;

	BOOL SetIconAndTitleForBalloonTip(CToolTipCtrl *pToolTipCtrl,
		int          tti_ICON,
		CString      title);

	VOID LoadToTray(CWnd*	 pWnd,
		UINT	 uCallbackMessage,
		CString sInfoTitle,
		CString sInfo,
		CString sTip,
		int     uTimeout,
		HICON	 icon);

	// Generated message map functions
	//{{AFX_MSG(CEDRShellNotifyDlg)
	virtual BOOL OnInitDialog();
	afx_msg VOID OnHideApp();
	afx_msg VOID OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg LONG OnTrayNotify(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
