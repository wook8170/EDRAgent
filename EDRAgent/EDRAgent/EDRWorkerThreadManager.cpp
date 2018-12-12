#include "StdAfx.h"
#include "EDRPolicyData.h"
#include "EDRWorkerThreadManager.h"

CEDRWorkerThreadManager * CEDRWorkerThreadManager::Instance()
{
	static CEDRWorkerThreadManager * pInstance = NULL;
	if (NULL == pInstance)
	{
		static CCriticalSection criticalSection;
		criticalSection.Lock();
		if (NULL == pInstance)
		{
			static CEDRWorkerThreadManager RealInstance;
			pInstance = &RealInstance;
		}
		criticalSection.Unlock();
	}

	return pInstance;
}

THREAD_HANDLE CEDRWorkerThreadManager::CreateWorkerThread(
	CHAR* name
	, BOOL syncStart
	, VOID* callbackFunc
)
{
	THREAD_HANDLE pThreadPtr = make_shared<ThreadEntityData>(
		name
		, syncStart
		, callbackFunc
	);

	if (pThreadPtr)
	{
		m_MapThreadPool[(DWORD)pThreadPtr.get()] = pThreadPtr;

		/*
		LOGI
			<< "~~ 쓰레드 생성 (+): "
			<< pThreadPtr->pThread
			<< ", " << pThreadPtr->pThread->GetThreadName();
		*/
		return pThreadPtr;
	}

	return NULL;
}
BOOL CEDRWorkerThreadManager::SetStopperbleEvent(
	THREAD_HANDLE threadID
	, CSyncEvent* event
	, BOOL* terminated
)
{
	ThreadStoppebleData_Ptr ptrStopperbleData = make_shared<ThreadStoppebleData>(event, terminated);
	//threadID->pSyncEvent = event;
	//threadID->pTerminated = terminated;

	SetStopperbleEvent(threadID, ptrStopperbleData);

	return TRUE;
}

BOOL CEDRWorkerThreadManager::SetStopperbleEvent(
	THREAD_HANDLE threadID
	, ThreadStoppebleData_Ptr stopperbleData
)
{
	threadID->ptrStopperbleData = stopperbleData;

	return TRUE;
}

BOOL CEDRWorkerThreadManager::StartThread(
	THREAD_HANDLE threadID
)
{
	if (!threadID->pCallbackFunc)
	{
		LOGW << "~~ 쓰레드 워커 함수가 등록되지 않았습니다.";
		return FALSE;
	}

	threadID->callback.Register(
		(AsyncCallback<ThreadCalllbackData_Ptr>::CallbackFunc)threadID->pCallbackFunc
		, (CallbackThread*)(threadID->pThread.get())
		, (VOID*)this
	);

	ThreadWin::StartAllThreads();

	/*
	LOGI
		<< "~~ 쓰레드 시작 (~): "
		<< threadID->pThread
		<< ", " << threadID->pThread->GetThreadName();
	*/
	return TRUE;
}

BOOL CEDRWorkerThreadManager::StopThread(
	THREAD_HANDLE threadID
)
{
	CHECK_NOTNULL(threadID->pThread);
	
	if (threadID->ptrStopperbleData)
	{
		if (threadID->ptrStopperbleData->pTerminated)
		{
			*threadID->ptrStopperbleData->pTerminated = TRUE;
		}

		if (threadID->ptrStopperbleData->pSyncEvent)
		{
			threadID->ptrStopperbleData->pSyncEvent->Set();
		}
	}
	threadID->callback.Unregister(
		(AsyncCallback<ThreadCalllbackData_Ptr>::CallbackFunc)threadID->pCallbackFunc
		, (CallbackThread*)(threadID->pThread.get())
		, (VOID*)this
	);
	threadID->pThread->ExitThread();

	/*
	LOGI
		<< "~~ 쓰레드 중지 (/): "
		<< threadID->pThread
		<< ", " << threadID->pThread->GetThreadName();
	*/

	return TRUE;
}

BOOL CEDRWorkerThreadManager::InvokeThreadFunc(
	THREAD_HANDLE threadID
	, ThreadCalllbackData_Ptr data
)
{
	threadID->callback.Invoke(data);

	return TRUE;
}

BOOL CEDRWorkerThreadManager::SetThreadFunc(
	THREAD_HANDLE threadID
	, VOID* func
)
{
	if (!threadID->pCallbackFunc)
	{
		threadID->pCallbackFunc = func;
		return TRUE;
	}

	return FALSE;
}

VOID CEDRWorkerThreadManager::StartAllWorkerThread()
{
	ThreadWin::StartAllThreads();

	for (auto it = m_MapThreadPool.begin(); it != m_MapThreadPool.end(); it++)
	{
		ThreadEntityData_Ptr pThreadPtr = (*it).second;
		StartThread(pThreadPtr);
	}
}

VOID CEDRWorkerThreadManager::StopAllWorkerThread()
{
	for (auto it = m_MapThreadPool.begin(); it != m_MapThreadPool.end(); it++)
	{
		ThreadEntityData_Ptr pThreadPtr = (*it).second;
		StopThread(pThreadPtr);
	}
}
