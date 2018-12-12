#pragma once

//#include "ProcessMonitor.h"

#ifdef PROCESSDLL_EXPORTS
#define PROCESSDLL_EXPORTS_API __declspec(dllexport) __stdcall  
#else
#define PROCESSDLL_EXPORTS_API __declspec(dllimport) __stdcall 
#endif



#ifdef __cplusplus
extern "C" {
#endif

	namespace ProcMon
	{
		struct DriverStatus
		{
			enum Type
			{
				NotInstalled = 0,
				Installed = 1,
				Running = 2
			};
		};


		UINT
			PROCESSDLL_EXPORTS_API
			Init(PPROCESSES_CALLBACK callback, PPROCESSES_CALLBACK callbackForDelete, VOID* pUserData);

		UINT
			PROCESSDLL_EXPORTS_API
			Deinit();

		UINT
			PROCESSDLL_EXPORTS_API
			Install();

		UINT
			PROCESSDLL_EXPORTS_API
			GetStatus(__int32 * status);

		UINT
			PROCESSDLL_EXPORTS_API
			Start();

		UINT
			PROCESSDLL_EXPORTS_API
			Stop();

		UINT
			PROCESSDLL_EXPORTS_API
			Uninstall();
	}
#ifdef __cplusplus
}
#endif



