// EDRAgentWnd.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"

#include "EDRAgent.h"
#include "EDRAgentWnd.h"
#include "EDRPolicyManager.h"
#include "EDRWorkerThread.h"
#include "EDRWorkerThreadManager.h"

#include "EDRShellNotifyDlg.h"
#include "EDRPolicyHashList.h"
#include "EDRPolicyHelperBase.h"

#include "HttpClient/HttpAttribute.h"
#include "ThreadPool/AsyncCallback.h"
#include "Util/Util.h"
#include "Util/Process.h"
#include "Util/URLParser.h"
#include "Util/Registry.h"
#include "Util/HashList.h"

#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>

#include "IOCPolicyHelperImpl.h"
#include "RESPolicyHelperImpl.h"

#include "EDRConfig.h"

// CEDRAgentWnd

IMPLEMENT_DYNAMIC(CEDRAgentWnd, CWnd)

extern CEDRAgentApp theApp;

CEDRAgentWnd::CEDRAgentWnd()
{
	m_pEDRShellNotifyDlg = NULL;
	m_pEDRWorkerThreadManager = NULL;
	m_pEDRPolicyManager = NULL;
	m_pIOCPolicyHelper = NULL;
	m_pRESPolicyHelper = NULL;
	m_pEDRConfig = NULL;

	LockGuard::Create(&m_lock);

	m_hPipe = CreateFile(
		"\\\\.\\pipe\\EDRNotificator1"
		, GENERIC_WRITE
		, 0
		, NULL
		, OPEN_EXISTING
		, FILE_FLAG_OVERLAPPED
		, NULL
	);
}

CEDRAgentWnd::~CEDRAgentWnd()
{
	LockGuard::Destroy(&m_lock);

	CloseHandle(m_hPipe);

	m_pEDRWorkerThreadManager->StopAllWorkerThread();

	// NicsProcMon.sys ����
	CProcess::Instance()->StopProcessMonitoring();

	EDRDelete(m_pEDRShellNotifyDlg);
	EDRDelete(m_pEDRWorkerThreadManager);
	EDRDelete(m_pEDRPolicyManager);
	EDRDelete(m_pIOCPolicyHelper);
	EDRDelete(m_pRESPolicyHelper);
	EDRDelete(m_pEDRConfig);

}

VOID CEDRAgentWnd::Init()
{
	LOGI << "Current Machin Username: " << CUtil::GetMachineUserName();
	LOGI << "Current SID            : " << CUtil::GetCurrentSID();
	LOGI << " ";

	CProcess::Instance();

	// NicsProcMon.sys ����
	CProcess::Instance()->StopProcessMonitoring();
	CProcess::Instance()->StartProcessMonitoring();

	// ��� ���� �ʱ�ȭ
	m_pEDRConfig = CEDRConfig::Instance();
	m_pEDRPolicyManager = EDRNew CEDRPolicyManager();
	m_pEDRWorkerThreadManager = CEDRWorkerThreadManager::Instance();
	m_pEDRShellNotifyDlg = NULL;

	AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T(
		"Nictech NEDR"
		, "EDR Agent �� �����մϴ�..\n");
	SendMessage(WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);

	// ������ ����
	m_hThreadIOC = m_pEDRWorkerThreadManager->CreateWorkerThread("Thread-IOCThread");
	m_hThreadRES = m_pEDRWorkerThreadManager->CreateWorkerThread("Thread-RESThread");
	m_hThreadUserModeLog = m_pEDRWorkerThreadManager->CreateWorkerThread("Thread-UserModeLog");

	m_pEDRWorkerThreadManager->SetThreadFunc(m_hThreadIOC, CEDRAgentWnd::ThreadFuncIOC);
	m_pEDRWorkerThreadManager->SetThreadFunc(m_hThreadRES, CEDRAgentWnd::ThreadFuncRES);
	m_pEDRWorkerThreadManager->SetThreadFunc(m_hThreadUserModeLog, CEDRAgentWnd::ThreadFuncUserModeLogUpload);

	// Regist IOC Helper to Policy manger
	m_pIOCPolicyHelper =
		(CEDRPolicyHelperBase*)EDRNew CIOCPolicyHelperImpl("IOC Policy Helper");
	((CIOCPolicyHelperImpl*)(m_pIOCPolicyHelper))->Init(IOCMetaName);
	m_pEDRPolicyManager->SetPolicyHelper(
		CHttpAttribute::ManagerIOC
		, (CEDRPolicyHelperBase*)m_pIOCPolicyHelper
	);

	// Regist RES Helper to Policy manger
	m_pRESPolicyHelper =
		(CEDRPolicyHelperBase*)EDRNew CRESPolicyHelperImpl("RES Policy Helper");
	((CRESPolicyHelperImpl*)(m_pRESPolicyHelper))->Init(RESMetaName);
	m_pEDRPolicyManager->SetPolicyHelper(
		CHttpAttribute::ManagerRES,
		(CEDRPolicyHelperBase*)m_pRESPolicyHelper
	);

#ifdef _DEBUG
	m_pIOCPolicyHelper->ClearPolicyFile();
	m_pRESPolicyHelper->ClearPolicyFile();
#endif

	string message = "�ʱ�ȭ �� ����͸��� �����մϴ�.\n - ";

	vector<string> vecTypes = CEDRConfig::Instance()->GetAllAvailableTypes();

	for (auto it = vecTypes.begin(); it != vecTypes.end(); it++)
	{
		message.append((*it)).append(" ");
	}

	pAlamrMessge = EDRNew AlarmMessage_T(
		"Nictech NEDR"
		, (CHAR*)message.c_str());
	SendMessage(WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);

	if (!m_pIOCPolicyHelper->LoadPolicyFile())
	{
		AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T(
			"Nictech NEDR"
			, "��å�� �������� �ʽ��ϴ�..\n���ο� ��å�� �޾� �ɴϴ�.");

		SendMessage(WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);

		LOGI << "IOC ��å ���� ��û";
		m_pEDRPolicyManager->Reset(
			CHttpAttribute::Delete
			, CHttpAttribute::PolicyIOCRefresh
		);
		m_pEDRPolicyManager->Request();
		m_pIOCPolicyHelper->PrintPolicyData();
	}

	if (!m_pRESPolicyHelper->LoadPolicyFile())
	{
		AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T(
			"Nictech NEDR"
			, "��å�� �������� �ʽ��ϴ�..\n���ο� ��å�� �޾� �ɴϴ�.");

		SendMessage(WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);

		LOGI << "RES ��å ���� ��û";
		m_pEDRPolicyManager->Reset(
			CHttpAttribute::Delete
			, CHttpAttribute::PolicyRESRefresh
		);
		m_pEDRPolicyManager->Request();
		m_pRESPolicyHelper->PrintPolicyData();
	}

	// ������ ����
	m_pEDRWorkerThreadManager->StartAllWorkerThread();
}

THREAD_HANDLE CEDRAgentWnd::GetUserModeLogUploadThreadHandle()
{
	return m_hThreadUserModeLog;
}

#define BEGIN_EDR_MESSAGE_MAP(_className_, _instance_)
#define ON_EDR_MESSAGE(_evtid_, _callback_, _handler_)
#define END_EDR_MESSAGE_MAP()


BEGIN_MESSAGE_MAP(CEDRAgentWnd, CWnd)
	ON_MESSAGE(WM_IOC_MESSAGE, &CEDRAgentWnd::OnPushMessage)
	ON_MESSAGE(WM_NOTIFY_MESSAGE, &CEDRAgentWnd::OnNotifyMessage)
END_MESSAGE_MAP()

// CEDRAgentWnd message handlers
VOID CEDRAgentWnd::CallbackFunc(
	const CallbackData_T& value
	, VOID* userData
)
{
	CEDRAgentWnd* pThis = (CEDRAgentWnd*)userData;
	if (!pThis)
	{
		return;
	}

	CEDRPolicyManager* pPolicyManager = (CEDRPolicyManager*)pThis->m_pEDRPolicyManager;
	if (!pPolicyManager)
	{
		return;
	}

	pPolicyManager->Reset(CHttpAttribute::Get, (CHttpAttribute::EEDRPolicyType)value.eRestType);
	LOGW_IF(!pPolicyManager->Request()) << "HTTP ��û ����.";

	AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T(
		"Nictech NEDR"
		, "����: ��å ��û�� ���� �Ͽ����ϴ�.\n");

	pThis->SendMessage(WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);


	return;
}

VOID CEDRAgentWnd::ThreadFuncUserModeLogUpload(
	const ThreadCalllbackData_Ptr & value
	, VOID * userData
)
{
	CEDRAgentWnd* pThis = (CEDRAgentWnd*)value->pContext;
	RETURN_V_FAIL_IF(!pThis);
	
	LockGuard(&pThis->m_lock);

	CHAR* rawData = (CHAR*)value->wParam;
	EDRJson logJson = CUtil::ParseLogData((CHAR*)rawData);

	//LOGD << "LOG JSON: " << logJson;

	RETURN_V_FAIL_IF(logJson.empty());
	string logURL = CEDRConfig::Instance()->GetLogURL();

	CHttpContext *pHttpContext = EDRNew CHttpContext();
	CHECK_NOTNULL(pHttpContext);

	CHttpClientBuilder *pHttpClientBuilder = EDRNew CHttpClientBuilder(pHttpContext);
	CHECK_NOTNULL(pHttpClientBuilder);

	logURL.append("/").append(CUtil::GetUserID());

	CHttpClient *pHttpClient = pHttpClientBuilder
		->SetMethod(CHttpAttribute::Post)
		->SetURL(logURL)
		->Build();
	CHECK_NOTNULL(pHttpClient);

	CHttpData* pData = EDRNew CHttpData(logJson);

	LOGD << "�α� ���� ��û: " << logJson;

	pHttpClient->SetRequestData(pData);
	HTTP_STATUS httpStatus = pHttpClient->SendRequest();
	CHttpData* pResponseData = pHttpClient->GetResponseData();

	LOGD_IF(httpStatus == HTTP_OK && pResponseData) << "�α� ���� ����: " << pResponseData->GetDataAsJson();

	EDRDelete(pHttpClient);
	EDRDelete(pHttpContext);
	EDRDelete(pHttpClientBuilder);

	return;
}

VOID CEDRAgentWnd::ThreadFuncIOC(
	const ThreadCalllbackData_Ptr& value
	, VOID* userData
)
{
	
	CEDRAgentWnd* pThis = (CEDRAgentWnd*)value->pContext;
	RETURN_V_FAIL_IF(!pThis);

	CEDRPolicyManager* pPolicyManager = (CEDRPolicyManager*)pThis->m_pEDRPolicyManager;
	RETURN_V_FAIL_IF(!pPolicyManager);

	CHttpAttribute::EEDRPolicyType ePolicyType = (CHttpAttribute::EEDRPolicyType)value->lParam;
	CHttpAttribute::EMethod eMethod;

	LockGuard(&pThis->m_lock);

	if (ePolicyType == CHttpAttribute::PolicyIOCRefresh
		|| ePolicyType == CHttpAttribute::PolicyRESRefresh)
	{
		eMethod = CHttpAttribute::Delete;
	}
	else
	{
		eMethod = CHttpAttribute::Get;
	}

	pPolicyManager->Reset(eMethod, ePolicyType);

	LOGW_IF(!pPolicyManager->Request()) << "HTTP ��û ����.";

	return;
}

VOID CEDRAgentWnd::ThreadFuncRES(
	const ThreadCalllbackData_Ptr& value
	, VOID* userData
)
{
	CEDRAgentWnd* pThis = (CEDRAgentWnd*)value->pContext;
	RETURN_V_FAIL_IF(!pThis);

	CEDRPolicyManager* pPolicyManager = (CEDRPolicyManager*)pThis->m_pEDRPolicyManager;
	RETURN_V_FAIL_IF(!pPolicyManager);

	CHttpAttribute::EEDRPolicyType ePolicyType = (CHttpAttribute::EEDRPolicyType)value->lParam;
	CHttpAttribute::EMethod eMethod;

	LockGuard(&pThis->m_lock);

	if (ePolicyType == CHttpAttribute::PolicyIOCRefresh
		|| ePolicyType == CHttpAttribute::PolicyRESRefresh)
	{
		eMethod = CHttpAttribute::Delete;
	}
	else
	{
		eMethod = CHttpAttribute::Get;
	}

	pPolicyManager->Reset(eMethod, ePolicyType);

	LOGW_IF(!pPolicyManager->Request()) << "HTTP ��û ����.";

	return;
}



afx_msg LRESULT CEDRAgentWnd::OnPushMessage(WPARAM wParam, LPARAM lParam)
{
	string url = (CHAR*)lParam;
	string message;

	CHttpAttribute::EEDRPolicyType	ePolicyType = (CHttpAttribute::EEDRPolicyType)wParam;
	CHttpAttribute::EEDRManagerType	eManagerType;

	if (ePolicyType == CHttpAttribute::PolicyIOCRefresh)
	{
		message = "IOC ��å�� ���� �޾� �ɴϴ�.";
	}
	else if (ePolicyType == CHttpAttribute::PolicyIOC)
	{
		message = "IOC ��å�� �޾� �ɴϴ�.";
	}
	else if (ePolicyType == CHttpAttribute::PolicyRESRefresh)
	{
		message = "���� ��å�� ���� �޾� �ɴϴ�.";
	}
	else if (ePolicyType == CHttpAttribute::PolicyRESProcess)
	{
		message = "���μ��� ���� ��å�� ���� �Ǿ����ϴ�.";
	}
	else if (ePolicyType == CHttpAttribute::PolicyRESFile)
	{
		message = "���� ���� ��å�� ���� �Ǿ����ϴ�.";
	}
	else if (ePolicyType == CHttpAttribute::PolicyRESRegistry)
	{
		message = "������Ʈ�� ���� ��å�� ���� �Ǿ����ϴ�.";
	}
	else if (ePolicyType == CHttpAttribute::PolicyRESNetwork)
	{
		message = "��Ʈ��ũ ���� ��å�� ���� �Ǿ����ϴ�.";
	}

	AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T(
		"Nictech NEDR"
		, (CHAR*)message.c_str());

	SendMessage(WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);

	if (ePolicyType == CHttpAttribute::PolicyIOCRefresh || ePolicyType == CHttpAttribute::PolicyIOC)
	{
		eManagerType = CHttpAttribute::ManagerIOC;
	}
	else if (ePolicyType >= CHttpAttribute::PolicyRESRefresh || ePolicyType <= CHttpAttribute::PolicyRESNetwork)
	{
		eManagerType = CHttpAttribute::ManagerRES;
	}
	else
	{
		eManagerType = CHttpAttribute::ManagerInvalid;
	}

	CEDRWorkerThreadManager* pEDRWorkerThreadManager = CEDRWorkerThreadManager::Instance();
	RETURN_FAIL_IF(!pEDRWorkerThreadManager, 0);

	ThreadCalllbackData_Ptr threadCallbackData = make_shared<ThreadCalllbackData>();

	threadCallbackData->wParam = eManagerType;
	threadCallbackData->lParam = ePolicyType;
	threadCallbackData->pContext = (VOID*)this;

	LOGI << "ManagerType: " << threadCallbackData->wParam
		<< ", PolicyType: " << threadCallbackData->lParam
		<< ", Context   : " << this;

	if (eManagerType == CHttpAttribute::ManagerIOC)
	{
		pEDRWorkerThreadManager->InvokeThreadFunc(m_hThreadIOC, threadCallbackData);
	}
	else if (eManagerType == CHttpAttribute::ManagerRES)
	{
		pEDRWorkerThreadManager->InvokeThreadFunc(m_hThreadRES, threadCallbackData);
	}

	return 0;
}


afx_msg LRESULT CEDRAgentWnd::OnNotifyMessage(WPARAM wParam, LPARAM lParam)
{
	AlarmMessage* pAlarmMessage = (AlarmMessage*)lParam;
	string prarameter = pAlarmMessage->szTitle;
	prarameter.append("|").append(pAlarmMessage->szInfo);
	DWORD cbWritten;

	CHAR buff[2048];

	EDRSetMemory(buff, 0xCC, 2048);
	EDRCopyMemory(buff, prarameter.c_str(), prarameter.length());

	WriteFile(m_hPipe, prarameter.c_str(), (DWORD)prarameter.length(), &cbWritten, NULL);

	EDRDelete(pAlarmMessage);

	Sleep(500);

	return 0;
}
