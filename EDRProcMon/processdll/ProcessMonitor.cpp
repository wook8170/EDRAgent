#include "ProcessMonitor.h"
#include "controls.h"
#include <exception>

int print_log(const char* format, ...)
{
	static char s_printf_buf[1024];
	va_list args;
	va_start(args, format);
	_vsnprintf(s_printf_buf, sizeof(s_printf_buf), format, args);
	va_end(args);
	OutputDebugStringA(s_printf_buf);
	return 0;
}

#define printf(format, ...) \
        print_log(format, __VA_ARGS__)

ProcessMonitor::ProcessMonitor(std::wstring driver_name)
	:   m_callback(NULL),
	m_callbackForDelete(NULL),
	m_hDriverHandle(INVALID_HANDLE_VALUE),
	m_wsServiceName(driver_name),
	m_TerminateMonitoring(false),
	m_TerminateMonitoringForDelete(false),
	m_pUserData(NULL)
{
}

ProcessMonitor::~ProcessMonitor(void)
{
	Reset();
}

UINT ProcessMonitor::Init(PPROCESSES_CALLBACK callback, PPROCESSES_CALLBACK callbackForDelete, VOID* pUserData)
{
	m_TerminateMonitoring = false;
	m_TerminateMonitoringForDelete = false;
	m_pUserData = pUserData;

	OpenDevice();

	AllowSelf();

	m_callback = callback;
	m_callbackForDelete = callbackForDelete;

	DWORD dispatchCount = 0;
	IoControl(
		IOCTL_GET_INIT_COUNT,
		NULL,
		0,
		&dispatchCount,
		sizeof(dispatchCount));
	
	std::string messge = "";
	
	printf("INTI COUNT #1: %d\n", dispatchCount);

	if (dispatchCount > 0)
	{
		DWORD bytesReturned = 0;
		// Get gray processes information.
		std::vector<PROCESS_INFO> gray(dispatchCount);
		IoControl(
			IOCTL_GET_INIT_PROCESS,
			NULL,
			0,
			&gray[0],
			static_cast<DWORD>(gray.size() * sizeof(gray[0])),
			&bytesReturned);

		// Adjust items count to actualy returned entries
		dispatchCount = bytesReturned / sizeof(gray[0]);

		std::vector<CHECK_LIST_ENTRY> checkList(dispatchCount);

		printf("INTI COUNT #2: %d\n", dispatchCount);

		for (unsigned i = 0; i < checkList.size(); i++)
		{
			checkList[i].ProcessInfo = &gray[i];
			checkList[i].AddToBlacklist = false;

			printf("INTI PROCESS #1: %d, %ws\n", i, checkList[i].ProcessInfo->ImagePath);

		}

		// Callback procedure expect array of pointers, prepare it.
		///*
		std::vector<PCHECK_LIST_ENTRY> ptrs(dispatchCount);
		for (unsigned i = 0; i < ptrs.size(); i++)
		{
			ptrs[i] = &checkList[i];
			printf("INTI PROCESS #2: %d, %ws\n", i, checkList[i].ProcessInfo->ImagePath);
		}
		//*/
		// Execute callback.
		printf("INTI COUNT #3: %d\n", ptrs.size());

		try 
		{
			if (m_callback)
			{
				printf("CALLBACK !!");
				m_callback(&ptrs[0], static_cast<ULONG>(ptrs.size()), m_pUserData);

			}
		}
		catch (...)
		{
			printf("CALLBACK EXCEPTION");

		}		
	}
	
	if(callbackForDelete)
	{
		HANDLE hEventForDelete = m_NotifyEventForDelete.GetHandle();
		IoControl(
			IOCTL_REGISTER_EVENT_FOR_DELETE,
			&hEventForDelete,
			sizeof(hEventForDelete),
			NULL,
			0);
		DWORD dwTidForDelete;
		m_hMonitoringThreadForDelete = CreateThread( 
			NULL,                   
			0,                      
			ProcessMonitor::MonitoringThreadProcForDelete,
			this,         
			0,            
			&dwTidForDelete); 

	}

	m_NotifyEvent.Reset();

	if(callback)
	{
		HANDLE hEvent = m_NotifyEvent.GetHandle();
		IoControl(
			IOCTL_REGISTER_EVENT,
			&hEvent,
			sizeof(hEvent),
			NULL,
			0);

		DWORD dwTid;
		m_hMonitoringThread = CreateThread( 
			NULL,					
			0,						
			ProcessMonitor::MonitoringThreadProc,
			this,		  
			0,			  
			&dwTid);   
	}


	return ERROR_SUCCESS;
}

DWORD ProcessMonitor::MonitoringThreadProc( LPVOID lpParam )
{
	ProcessMonitor* pThis = (ProcessMonitor*)lpParam;

	do
	{
		if (pThis->m_NotifyEvent.Wait() && !pThis->m_TerminateMonitoring)
		{
			pThis->Dispatch();
		}
	}
	while (!pThis->m_TerminateMonitoring);
	return 0;
}

DWORD ProcessMonitor::MonitoringThreadProcForDelete( LPVOID lpParam )
{
	ProcessMonitor* pThis = (ProcessMonitor*)lpParam;
	do
	{
		if (pThis->m_NotifyEventForDelete.Wait() && !pThis->m_TerminateMonitoringForDelete)
			pThis->DispatchForDelete();
	}
	while (!pThis->m_TerminateMonitoringForDelete);
	return 0;
}

std::vector<PPROCESS_INFO> processList;
void ProcessMonitor::Dispatch()
{
	int type = EProcessCreate;
	DWORD dispatchCount = 0;
	IoControl(
		IOCTL_GET_PROCESS_COUNT,
		(LPVOID)&type,
		sizeof(type),
		&dispatchCount,
		sizeof(dispatchCount));

	if (dispatchCount > 0)
	{
		DWORD bytesReturned = 0;
		// Get gray processes information.
		std::vector<PROCESS_INFO> gray(dispatchCount);
		IoControl(
			IOCTL_GET_PROCESS_LIST,
			(LPVOID)&type,
			sizeof(type),
			&gray[0],
			static_cast<DWORD>(gray.size() * sizeof(gray[0])),
			&bytesReturned);

		// Adjust items count to actualy returned entries
		dispatchCount = bytesReturned/sizeof(gray[0]);

		std::vector<CHECK_LIST_ENTRY> checkList(dispatchCount);
		for (unsigned i = 0; i < checkList.size(); i++)
		{
			checkList[i].ProcessInfo = &gray[i];
			checkList[i].AddToBlacklist = false;
		}

		// Callback procedure expect array of pointers, prepare it.
		std::vector<PCHECK_LIST_ENTRY> ptrs(dispatchCount);
		for (unsigned i = 0; i < ptrs.size(); i++)
			ptrs[i] = &checkList[i];

		// Execute callback.
		try 
		{
			if (m_callback)
				m_callback(&ptrs[0], static_cast<ULONG>(ptrs.size()), m_pUserData);
		}
		catch (...)
		{
		}

		for (unsigned i = 0; i < checkList.size(); i++)
		{
			if (wcslen(checkList[i].ProcessInfo->ImagePath) == 0)
				continue;//throw std::exception("internal error. Process info don't have ImagePath");

			if (checkList[i].AddToBlacklist)
				Block(checkList[i].ProcessInfo);
			else
				Allow(checkList[i].ProcessInfo);
		}

		IoControl(
			IOCTL_TERMINATE_ALL,
			NULL,
			0,
			NULL,
			0);
	}
	else
	{
		m_NotifyEvent.Reset();
	}
}

void ProcessMonitor::DispatchForDelete()
{
	int type = EProcessDelete;

	DWORD dispatchCount = 0;
	IoControl(
		IOCTL_GET_PROCESS_COUNT,
		(LPVOID)&type,
		sizeof(type),
		&dispatchCount,
		sizeof(dispatchCount));

	if (dispatchCount > 0)
	{
		DWORD bytesReturned = 0;
		// Get gray processes information.
		std::vector<PROCESS_INFO> gray(dispatchCount);
		IoControl(
			IOCTL_GET_PROCESS_LIST,
			(LPVOID)&type,
			sizeof(type),
			&gray[0],
			static_cast<DWORD>(gray.size() * sizeof(gray[0])),
			&bytesReturned);

		// Adjust items count to actualy returned entries
		dispatchCount = bytesReturned/sizeof(gray[0]);

		std::vector<CHECK_LIST_ENTRY> checkList(dispatchCount);
		for (unsigned i = 0; i < checkList.size(); i++)
		{
			checkList[i].ProcessInfo = &gray[i];
			checkList[i].AddToBlacklist = false;
		}

		// Callback procedure expect array of pointers, prepare it.
		std::vector<PCHECK_LIST_ENTRY> ptrs(dispatchCount);
		for (unsigned i = 0; i < ptrs.size(); i++)
			ptrs[i] = &checkList[i];

		// Execute callback.
		try 
		{
			if (m_callbackForDelete)
				m_callbackForDelete(&ptrs[0], static_cast<ULONG>(ptrs.size()), m_pUserData);
		}
		catch (...)
		{
		}

		IoControl(
			IOCTL_DELETE_COMPLETED,
			NULL,
			0,
			NULL,
			0);
		m_NotifyEventForDelete.Reset();
	}
	else
	{
		IoControl(
			IOCTL_DELETE_COMPLETED,
			NULL,
			0,
			NULL,
			0);

		m_NotifyEventForDelete.Reset();
	}
}

UINT ProcessMonitor::Allow(PPROCESS_INFO ProcessInfo)
{
	try
	{
		IoControl(
			IOCTL_ALLOW,
			ProcessInfo,
			sizeof(ProcessInfo),
			NULL,
			0);
	}
	catch (const std::runtime_error &ex)
	{
		return GetLastError();

		ex;
	}
	return 0;
}

UINT ProcessMonitor::Block(PPROCESS_INFO ProcessInfo)
{
	try
	{
		IoControl(
			IOCTL_BLOCK,
			ProcessInfo,
			sizeof(ProcessInfo),
			NULL,
			0);
	}
	catch (const std::runtime_error &ex)
	{
		DWORD lastError = GetLastError();

		return lastError;

		ex;
	}

	return 0;
}


UINT ProcessMonitor::Reset()
{

	m_TerminateMonitoring = true;
	m_TerminateMonitoringForDelete = true;
	m_NotifyEvent.Set();
	m_NotifyEventForDelete.Set();
	m_NotifyEventForInitList.Set();

	if (m_hMonitoringThread)
		::WaitForSingleObject(m_hMonitoringThread, INFINITE);

	if (m_hMonitoringThreadForDelete)
		::WaitForSingleObject(m_hMonitoringThreadForDelete, INFINITE);

	if (m_hDriverHandle != INVALID_HANDLE_VALUE)
	{
		IoControl(
			IOCTL_RESET,
			NULL,
			0,
			NULL,
			0);
		CloseHandle(m_hDriverHandle);
		m_hDriverHandle = INVALID_HANDLE_VALUE;
	}

	return ERROR_SUCCESS;
}

void ProcessMonitor::OpenDevice()
{
	if (m_hDriverHandle != INVALID_HANDLE_VALUE)
		return;

	m_hDriverHandle = CreateFile(
		(L"\\\\.\\" + m_wsServiceName).c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);

	if (m_hDriverHandle == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Can't open driver device");
}

void ProcessMonitor::IoControl(DWORD IoControlCode, LPVOID InBuffer, DWORD InBufferSize,
							   LPVOID OutBuffer, DWORD OutBufferSize, LPDWORD BytesReturned)
{
	DWORD tmp;
	if (!DeviceIoControl(
		m_hDriverHandle,
		IoControlCode,
		InBuffer,
		InBufferSize,
		OutBuffer,
		OutBufferSize,
		BytesReturned != NULL ? BytesReturned : &tmp,
		NULL))
	{        
		throw std::runtime_error("DeviceIoControl() failed");
	}
}

void ProcessMonitor::AllowSelf()
{
	PROCESS_INFO info;
	SecureZeroMemory(&info, sizeof(info));
	info.ProcessId = GetCurrentProcessId();
	std::wstring exePath = utils::GetModuleFileNameEx();
	::memcpy(&info.ImagePath, exePath.c_str(), exePath.size() * 2);
	Allow(&info);
}
