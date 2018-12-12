#include "stdafx.h"

#include "ThreadWin.h"
#include "UserMsgs.h"
#include "ThreadMsg.h"
#include "Fault.h"

HANDLE ThreadWin::m_hStartAllThreads = INVALID_HANDLE_VALUE;
ThreadWin* ThreadWin::m_allocatedStartAllThreads = NULL;

struct ThreadParam
{
	ThreadWin* pThread;
};

ThreadWin::ThreadWin(const CHAR* threadName, BOOL syncStart) :
	THREAD_NAME(threadName),
	SYNC_START(syncStart),
	m_hThreadStarted(INVALID_HANDLE_VALUE),
	m_hThreadExited(INVALID_HANDLE_VALUE),
	m_hThread(INVALID_HANDLE_VALUE),
	m_threadId(0)
{
	if (m_hStartAllThreads == INVALID_HANDLE_VALUE)
	{
		m_hStartAllThreads = CreateEvent(
			NULL
			, TRUE
			, FALSE
			, TEXT("StartAllThreadsEvent")
		);
		m_allocatedStartAllThreads = this;
	}
}

ThreadWin::~ThreadWin()
{
	if (m_allocatedStartAllThreads == this && m_hStartAllThreads != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hStartAllThreads);
		m_hStartAllThreads = INVALID_HANDLE_VALUE;
	}

	if (m_hThread != INVALID_HANDLE_VALUE)
		ExitThread();
}

VOID ThreadWin::PostThreadMessage(UINT msg, VOID* data)
{
	BOOL success = ::PostThreadMessage(GetThreadId(), msg, (WPARAM)data, 0);
	CHECK(success != 0);
}

VOID ThreadWin::DispatchCallback(CallbackMsg* msg)
{
	ThreadMsg* threadMsg = EDRNew ThreadMsg(WM_DISPATCH_CALLBACK, msg);

	PostThreadMessage(WM_DISPATCH_CALLBACK, threadMsg);
}

BOOL ThreadWin::CreateThread()
{
	if (!IsCreated())
	{
		m_hThreadStarted = CreateEvent(
			NULL
			, TRUE
			, FALSE
			, TEXT("ThreadCreatedEvent")
		);

		ThreadParam threadParam;
		threadParam.pThread = this;
		m_hThread = ::CreateThread(
			NULL
			, 0
			, (unsigned long(__stdcall *)(VOID *))RunProcess
			, (VOID *)(&threadParam)
			, 0
			, &m_threadId
		);
		CHECK_NOTNULL(m_hThread);

		DWORD err = WaitForSingleObject(m_hThreadStarted, MAX_WAIT_TIME);
		CHECK(err == WAIT_OBJECT_0);

		CloseHandle(m_hThreadStarted);
		m_hThreadStarted = INVALID_HANDLE_VALUE;

		if (m_hThread)
		{
			/*
			LOGV
				<< "[ NEW THREAD - 신규 쓰레드 생성 NAME: ( "
				<< GetThreadName()
				<< " ), TID ( "
				<< GetThreadId()
				<< " ) ]";
			*/
			LOGI
				<< "[ 쓰레드 생성 (+): TID ("
				<< FILLSETWR(5) << GetThreadId()
				<< " ), TName ( "
				<< GetThreadName()
				<< " ) ]";

		}
		return m_hThread ? TRUE : FALSE;
	}
	return FALSE;
}

VOID ThreadWin::ExitThread()
{
	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		m_hThreadExited = CreateEvent(NULL, TRUE, FALSE, TEXT("ThreadExitedEvent"));

		PostThreadMessage(WM_EXIT_THREAD);

		if (::WaitForSingleObject(m_hThreadExited, MAX_WAIT_TIME) == WAIT_TIMEOUT)
			::TerminateThread(m_hThread, 1);

		LOGI
			<< "[ 쓰레드 중지 (/): TID ("
			<< FILLSETWR(5) << GetThreadId()
			<< " ), TName ( "
			<< GetThreadName()
			<< " ) ]";

		::CloseHandle(m_hThread);
		m_hThread = INVALID_HANDLE_VALUE;

		::CloseHandle(m_hThreadExited);
		m_hThreadExited = INVALID_HANDLE_VALUE;
	}
}

int ThreadWin::RunProcess(VOID* threadParam)
{
	ThreadWin* thread;
	thread = (ThreadWin*)(static_cast<ThreadParam*>(threadParam))->pThread;

	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	BOOL err = SetEvent(thread->m_hThreadStarted);
	CHECK(err != 0);

	if (thread->SYNC_START == TRUE)
	{
		DWORD err = WaitForSingleObject(m_hStartAllThreads, MAX_WAIT_TIME);
		CHECK(err == WAIT_OBJECT_0);
	}

	LOGI
		<< "[ 쓰레드 시작 (>): TID ("
		<< FILLSETWR(5) << thread->GetThreadId()
		<< " ), TName ( "
		<< thread->GetThreadName()
		<< " ) ]";

	int retVal = thread->Process(NULL);

	err = SetEvent(thread->m_hThreadExited);
	CHECK(err != 0);

	return retVal;
}

VOID ThreadWin::StartAllThreads()
{
	BOOL err = SetEvent(m_hStartAllThreads);
	CHECK(err != 0);
}

