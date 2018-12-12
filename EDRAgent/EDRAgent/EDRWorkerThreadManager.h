#pragma once

#include "CommonDef.h"
#include "EDRWorkerThread.h"
#include "EDRPolicyData.h"
#include "Util/SyncEvent.h"
#include "ThreadPool/AsyncCallback.h"
#include "Util/SyncEvent.h"

typedef struct _ThreadCalllbackData
{
	WPARAM	wParam;
	LPARAM	lParam;
	VOID*	pContext;

	_ThreadCalllbackData()
	{
		wParam = 0;
		lParam = 0;
		pContext = NULL;
	};

	_ThreadCalllbackData(
		WPARAM wparam
		, LPARAM lparam
		, VOID* context
	)
	{
		wParam = wparam;
		lParam = lParam;
		pContext = context;
	};
} ThreadCalllbackData;
typedef shared_ptr<CEDRWorkerThread>				CEDRWorkerThread_Ptr;
typedef shared_ptr<ThreadCalllbackData>				ThreadCalllbackData_Ptr;

typedef struct _ThreadStoppebleData_T
{
	CSyncEvent	*pSyncEvent;
	BOOL		*pTerminated;

	_ThreadStoppebleData_T()
	{
		pSyncEvent = NULL;
		pTerminated = NULL;
	}

	_ThreadStoppebleData_T(
		CSyncEvent *syncEvent
		, BOOL *terminated
	)
	{
		pSyncEvent = syncEvent;
		pTerminated = terminated;
	}

} ThreadStoppebleData;
typedef shared_ptr<ThreadStoppebleData>				ThreadStoppebleData_Ptr;

typedef struct _ThreadEntityData_T
{
	CHAR											szName[256];
	CEDRWorkerThread_Ptr							pThread;	
	ThreadCalllbackData_Ptr							pCallbackData;
	VOID*											pCallbackFunc;
	ThreadStoppebleData_Ptr							ptrStopperbleData;
	AsyncCallback<ThreadCalllbackData_Ptr>			callback;

	_ThreadEntityData_T()
	{
		ZeroMemory(szName, 256);
		CopyMemory(szName, "DEFAULT_NAME", min(strlen("DEFAULT_NAME"), 255));
		pThread = make_shared<CEDRWorkerThread>((const CHAR*)szName, TRUE);
		pCallbackFunc = NULL;
		ptrStopperbleData = NULL;
		pThread->CreateThread();
	}

	_ThreadEntityData_T(
		CHAR* name
		, BOOL syncStart = TRUE
		, VOID* callbackFunc = NULL
	)
	{
		ZeroMemory(szName, 256);
		CopyMemory(szName, name, min(strlen(name), 255));
		pThread = make_shared<CEDRWorkerThread>((const CHAR*)szName, syncStart);
		pCallbackFunc = callbackFunc;
		ptrStopperbleData = NULL;
		pThread->CreateThread();
	}

	~_ThreadEntityData_T()
	{
		if(ptrStopperbleData)
		if (ptrStopperbleData->pSyncEvent)
		{
			ptrStopperbleData->pSyncEvent->Set();
		}

		if(ptrStopperbleData->pTerminated)
		{
			*ptrStopperbleData->pTerminated = TRUE;
		}		

		pThread.reset();
		pCallbackData.reset();
		ptrStopperbleData.reset();
	}

} ThreadEntityData;
typedef shared_ptr<ThreadEntityData>				ThreadEntityData_Ptr;
typedef map<DWORD, ThreadEntityData_Ptr>			ThreadEntityDataMap;
typedef ThreadEntityData_Ptr						THREAD_HANDLE;

class CEDRWorkerThreadManager
{
private:
	ThreadEntityDataMap m_MapThreadPool;
public:
	static CEDRWorkerThreadManager * Instance();

	THREAD_HANDLE CreateWorkerThread(
		CHAR* name
		, BOOL syncStart = TRUE
		, VOID* callbackFunc = NULL
	);

	BOOL SetStopperbleEvent(
		THREAD_HANDLE threadID
		, CSyncEvent* event
		, BOOL* terminated
	);

	BOOL SetStopperbleEvent(
		THREAD_HANDLE threadID
		, ThreadStoppebleData_Ptr stopperbleData
	);

	BOOL StartThread(
		THREAD_HANDLE threadID
	);

	BOOL StopThread(
		THREAD_HANDLE threadID
	);

	BOOL InvokeThreadFunc(
		THREAD_HANDLE threadID
		, ThreadCalllbackData_Ptr data
	);

	BOOL SetThreadFunc(
		THREAD_HANDLE threadID
		, VOID* func
	);

	VOID StartAllWorkerThread();
	
	VOID StopAllWorkerThread();

};