#include "processdll.h"
#include "ServiceController.h"


BOOL Is64bitOS()
{
	BOOL bResult = FALSE;
	SYSTEM_INFO si;
	typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
	PGNSI pGNSI = NULL;
	
	pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
	if(NULL != pGNSI)
	{
		pGNSI(&si);
	}
	else
	{
		GetSystemInfo(&si);
	}

	//64비트를 지원하는 CPU라도 32비트 운영체제가 설치되면 32비트 CPU라고 나옵니다.
	if ( PROCESSOR_ARCHITECTURE_IA64 == si.wProcessorArchitecture )   
	{
		bResult = TRUE;
	}
	else if ( PROCESSOR_ARCHITECTURE_AMD64 == si.wProcessorArchitecture )
	{
			
		bResult = TRUE;
			
	}

	return bResult;
}
#ifdef _WIN64 
ProcessMonitor gProcessMonitor(L"PROCMON");
#else
ProcessMonitor gProcessMonitor(L"PROCMON");
#endif

namespace ProcMon
{
	UINT
		PROCESSDLL_EXPORTS_API 
		Init(PPROCESSES_CALLBACK callback, PPROCESSES_CALLBACK callbackForDelete, VOID* pUserData)
	{
		return gProcessMonitor.Init(callback, callbackForDelete, pUserData);
	}

	UINT
		PROCESSDLL_EXPORTS_API 
		Deinit()
	{
		return gProcessMonitor.Reset();
	}

	UINT
		PROCESSDLL_EXPORTS_API
		Install()
	{
		try
		{
			bool installed = false;
			bool running = false;
			ServiceController::QueryStatus(SERVICE_NAME, installed, running);
			if (installed)
				return ERROR_SUCCESS;
			if (!installed)
			{
				if(Is64bitOS())
				{
					std::wstring fullPath = utils::GetModulePath() + L"NicProcMon_64.sys";
					ServiceController::Install(SERVICE_NAME, fullPath, SERVICE_KERNEL_DRIVER, SERVICE_AUTO_START);
				}
				else
				{
					std::wstring fullPath = utils::GetModulePath() + L"NicProcMon_32.sys";
					ServiceController::Install(SERVICE_NAME, fullPath, SERVICE_KERNEL_DRIVER, SERVICE_AUTO_START);
				}
			}
			return ERROR_SUCCESS;
		}
		catch(const std::exception& )
		{
			return ERROR_SERVICE_NOT_FOUND;
		}
	}

	UINT
		PROCESSDLL_EXPORTS_API 
		GetStatus(__int32 * status)
	{
		try
		{
			bool installed = false;
			bool running = false;
			ServiceController::QueryStatus(SERVICE_NAME, installed, running);
			if (installed && running)
				*status = DriverStatus::Running;
			if (installed && !running)
				*status = DriverStatus::Installed;
			return ERROR_SUCCESS;
		}
		catch( const std::exception& )
		{
			return ERROR_SERVICE_DOES_NOT_EXIST;
		}
	}

	UINT
		PROCESSDLL_EXPORTS_API 
		Start()
	{
		try
		{
			bool installed = false;
			bool running = false;
			ServiceController::QueryStatus(SERVICE_NAME, installed, running);
			if (installed && running)
				return ERROR_SUCCESS;
			if (!installed)
			{
				return ERROR_SERVICE_NOT_FOUND;
			}
			if (!running)
			{
				ServiceController::Start(SERVICE_NAME);
			}
			return ERROR_SUCCESS;
		}
		catch(const std::exception& )
		{
			return ERROR_SERVICE_NOT_FOUND;
		}
	}

	UINT
		PROCESSDLL_EXPORTS_API 
		Stop()
	{
		try
		{
			bool installed = false;
			bool running = false;
			ServiceController::QueryStatus(SERVICE_NAME, installed, running);
			if (!installed)
			{
				return ERROR_SERVICE_NOT_FOUND;
			}
			if (!running)
			{
				return ERROR_SERVICE_NOT_ACTIVE;
			}

			ServiceController::Stop(SERVICE_NAME);

			return ERROR_SUCCESS;
		}
		catch(const std::exception& )
		{
			return ERROR_SERVICE_NOT_FOUND;
		}
	}

	UINT
		PROCESSDLL_EXPORTS_API 
		Uninstall()
	{
		try
		{
			bool installed = false;
			bool running = false;
			ServiceController::QueryStatus(SERVICE_NAME, installed, running);

			if (!installed)
			{
				return ERROR_SERVICE_NOT_FOUND;
			}

			if (running)
			{
				return ERROR_SERVICE_EXISTS;
			}

			ServiceController::Remove(SERVICE_NAME);

			return ERROR_SUCCESS;
		}
		catch(const std::exception& )
		{
			return ERROR_SERVICE_NOT_FOUND;
		}
	}
}
