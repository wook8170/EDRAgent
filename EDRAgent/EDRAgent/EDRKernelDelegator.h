#pragma once

#include "CommonDef.h"
#include "EDRWorkerThreadManager.h"
#include "EDRPolicyData.h"
#include "Util/SyncEvent.h"
#include "ThreadPool/LockGuard.h"

class CEDRKernelDelegator
{
protected:
	string									m_strName;	
	EEDRType								m_eType;

	CSyncEvent								m_logCollectEvent;
	AsyncCallback<ThreadCalllbackData_Ptr>	m_logCollectCallback;
	BOOL									m_logCollectThreadTerminating;
	THREAD_HANDLE							m_logCollectThreadHandle;

	CSyncEvent								m_logUploadEvent;
	AsyncCallback<ThreadCalllbackData_Ptr>	m_logUploadCallback;
	BOOL									m_logUploadThreadTerminating;
	THREAD_HANDLE							m_logUploadThreadHandle;

	LOCK									m_Lock;
	BOOL									m_bUsePollingMethod;

private:
	EEDRType GetType();

public:
	static VOID LogCollectFunc(
		const ThreadCalllbackData_Ptr& value
		, VOID* userData
	);

	static VOID LogUploadFunc(
		const ThreadCalllbackData_Ptr& value
		, VOID* userData
	);

	LOCK* GetLock();

	THREAD_HANDLE GetLogCollectThreadHandle();

	THREAD_HANDLE GetLogUploadThreadHandle();

	CSyncEvent& GetLogCollectEvent();

	CSyncEvent& GetLogUploadEvent();

	BOOL IsLogCollectThreadTerminated();

	BOOL IsLogUploadThreadTerminated();

	EDRJson ParseLogData(
		CHAR* pData
	);

	string GetName();

	BOOL UsePollingMethod();

	CEDRKernelDelegator(
		string name
		, EEDRType eEDRType
		, BOOL bUsePollingMethod = FALSE
	);

	~CEDRKernelDelegator();
};
