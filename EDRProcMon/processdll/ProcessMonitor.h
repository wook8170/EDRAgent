#ifndef __PROCESS_MONITOR_H__
#define __PROCESS_MONITOR_H__

#include <string>
#include "common.h"
#include "utils.h"
//#include <boost/thread.hpp>

class ProcessMonitor
{
	HANDLE m_hDriverHandle;
	std::wstring m_wsServiceName;

	PPROCESSES_CALLBACK m_callback;
	PPROCESSES_CALLBACK m_callbackForDelete;
	utils::Event m_NotifyEvent;
	utils::Event m_NotifyEventForDelete;
	HANDLE m_hMonitoringThread;
	HANDLE m_hMonitoringThreadForDelete;
	bool m_TerminateMonitoring;
	bool m_TerminateMonitoringForDelete;

	utils::Event m_NotifyEventForInitList;

	VOID*	m_pUserData;

	static DWORD WINAPI MonitoringThreadProc( LPVOID lpParam );

	static DWORD WINAPI MonitoringThreadProcForDelete( LPVOID lpParam );

	void OpenDevice();

	void Dispatch();

	void DispatchForDelete();

	void AllowSelf();

	UINT Block(PPROCESS_INFO ProcessInfo);

	UINT Allow(PPROCESS_INFO ProcessInfo);

	void IoControl(DWORD IoControlCode, LPVOID InBuffer, DWORD InBufferSize,
		LPVOID OutBuffer, DWORD OutBufferSize, LPDWORD BytesReturned = NULL);

public:
	ProcessMonitor(std::wstring driver_name);
	UINT Init(PPROCESSES_CALLBACK callback, PPROCESSES_CALLBACK callbackForDelete, VOID* pUserData);
	UINT Reset();

	virtual ~ProcessMonitor(void);
};

#endif // __PROCESS_MONITOR_H__
