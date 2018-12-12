#include "stdafx.h"

#include "EDRWorkerThread.h"
#include "ThreadPool/AsyncCallback.h"
#include "ThreadPool/UserMsgs.h"
#include "ThreadPool/ThreadMsg.h"

CEDRWorkerThread::CEDRWorkerThread(const CHAR* threadName, BOOL syncStart) : ThreadWin(threadName, syncStart)
{
}

unsigned long CEDRWorkerThread::Process(VOID* parameter)
{
	MSG msg;
	BOOL bRet;

	while ((bRet = GetMessage(&msg, NULL, WM_USER_BEGIN, WM_USER_END)) != 0)
	{
		switch (msg.message)
		{
		case WM_DISPATCH_CALLBACK:
		{
			CHECK_NOTNULL((VOID*)msg.wParam);

			ThreadMsg* threadMsg = reinterpret_cast<ThreadMsg*>(msg.wParam);
			CallbackMsg* callbackMsg = static_cast<CallbackMsg*>(threadMsg->GetData());
			callbackMsg->GetAsyncCallback()->TargetInvoke(&callbackMsg);
			EDRDelete(threadMsg);
			//delete threadMsg;

			break;
		}

		case WM_EXIT_THREAD:
			return 0;

		default:
			CHECK(TRUE);
		}
	}
	return 0;
}