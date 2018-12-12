#pragma once

#include "EDRWorkerThread.h"
#include "EDRPolicyManager.h"
#include "EDRWorkerThreadManager.h"
#include "EDRAgentDll.h"
#include "EDRKernelDelegator.h"
#include "Util/SyncEvent.h"

// CEDRAgentWnd

class CEDRShellNotifyDlg;
class CEDRAgentWnd : public CWnd
{
	DECLARE_DYNAMIC(CEDRAgentWnd)
private:
	
	CEDRShellNotifyDlg			*m_pEDRShellNotifyDlg;
	CEDRWorkerThreadManager		*m_pEDRWorkerThreadManager;
	CEDRPolicyManager			*m_pEDRPolicyManager;
	CEDRPolicyHelperBase		*m_pIOCPolicyHelper;
	CEDRPolicyHelperBase		*m_pRESPolicyHelper;
	CEDRConfig					*m_pEDRConfig;

	THREAD_HANDLE				m_hThreadIOC;
	THREAD_HANDLE				m_hThreadRES;
	THREAD_HANDLE				m_hThreadUserModeLog;

	HANDLE						m_hPipe;

	LOCK						m_lock;

public:
	CEDRAgentWnd();

	virtual ~CEDRAgentWnd();
	
	VOID Init();

	THREAD_HANDLE GetUserModeLogUploadThreadHandle();

protected:

	static VOID CallbackFunc(
		const CallbackData_T& value
		, VOID* userData
	);

	static VOID ThreadFuncUserModeLogUpload(
		const ThreadCalllbackData_Ptr& value
		, VOID* userData
	);

	static VOID ThreadFuncIOC(
		const ThreadCalllbackData_Ptr& value
		, VOID* userData
	);

	static VOID ThreadFuncRES(
		const ThreadCalllbackData_Ptr& value
		, VOID* userData
	);

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnPushMessage(
		WPARAM wParam
		, LPARAM lParam
	);

	afx_msg LRESULT OnNotifyMessage(
		WPARAM wParam
		, LPARAM lParam
	);
};


