#pragma once

#include "CallbackThread.h"
#include "DataTypes.h"

class ThreadWin : public CallbackThread
{
public:
	ThreadWin(
		const CHAR* threadName
		, BOOL syncStart = TRUE
	);

	virtual ~ThreadWin();

	BOOL CreateThread();

	VOID ExitThread();

	BOOL IsCreated() { return (m_hThread != INVALID_HANDLE_VALUE); }

	HANDLE GetThreadHandle() { return m_hThread; }

	DWORD GetThreadId() { return m_threadId; }

	const CHAR* GetThreadName() { return THREAD_NAME; }

	VOID PostThreadMessage(
		UINT msg
		, VOID* data = NULL
	);

	static VOID StartAllThreads();

protected:
	virtual unsigned long Process(
		VOID* parameter
	) = 0;

private:
	ThreadWin(
		const ThreadWin&
	);

	ThreadWin& operator=(
		const ThreadWin&
		);

	virtual VOID DispatchCallback(
		CallbackMsg* msg
	);

	static int RunProcess(
		VOID* threadParam
	);

	const CHAR* THREAD_NAME;
	static const DWORD MAX_WAIT_TIME = INFINITE;//60000;
	DWORD m_threadId;
	HANDLE m_hThread;
	const BOOL SYNC_START;
	static HANDLE m_hStartAllThreads;
	static ThreadWin* m_allocatedStartAllThreads;
	HANDLE m_hThreadStarted;
	HANDLE m_hThreadExited;
};
