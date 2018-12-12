
// EDRTesterDlg.h : header file
//

#pragma once


// CEDRTesterDlg dialog
class CEDRTesterDlg : public CDialogEx
{
// Construction
public:
	CEDRTesterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EDRTESTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonIocRefresh();
	afx_msg void OnBnClickedButtonIocUpdate();
	afx_msg void OnBnClickedButtonResRefresh();
	afx_msg void OnBnClickedButtonResProcess();
	afx_msg void OnBnClickedButtonResFile();
	afx_msg void OnBnClickedButtonResRegistry();
	afx_msg void OnBnClickedButtonResNetwork();
};
