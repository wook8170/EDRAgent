
// EDRFileHashMgr.cpp : 구현 파일
//

#include "stdafx.h"
#include "EDRFileHashMgr.h"
//#include "KISA_SHA256.h"
#include <stdio.h>
#include "Util\Util.h"


HANDLE	g_hNicFsfd = INVALID_HANDLE_VALUE;
BOOL	g_bStartEDRFileMon = FALSE;

HANDLE OpenNicfsfd()
{
	return CreateFileA("\\\\.\\NICFSFD", GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
	   				 NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}


HANDLE	g_hR0EDRFileInterfaceEvent = NULL;
HANDLE	g_hEDRFileThread = NULL;
BOOL	g_bCheckEDRFile = FALSE;

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS)(HANDLE hProcess,PBOOL Wow64Process);

BOOL IsWow64()
{
	BOOL bIsWow64=FALSE;
	HMODULE hKernel=GetModuleHandleA("kernel32");
	if(hKernel!=NULL)
	{
		LPFN_ISWOW64PROCESS fnIsWow64Process=(LPFN_ISWOW64PROCESS)GetProcAddress(hKernel,"IsWow64Process");
		if(fnIsWow64Process!=NULL)
		{
			fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
		}
		FreeLibrary(hKernel);
	}

	return bIsWow64;
}



DWORD WINAPI ProcessKernelDataEDRFile(LPVOID lpParameter)
{	
	DWORD dwBufLen = 0;
	DWORD dwListEmpty;
	
	PVOID	pDataBuf = NULL;
	int		nDataSize = 0;
	BOOL	bIsWow64 = IsWow64();
	
	if(g_hNicFsfd == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	if( bIsWow64 )
	{
		nDataSize = sizeof(CHECKEDRFILE_DATA64);
	}
	else
	{
		nDataSize = sizeof(CHECKEDRFILE_DATA);
	}

	pDataBuf = (PVOID)calloc(1, nDataSize);
		
	if (!pDataBuf)
	{
		return 0;
	}
	
	while(g_bStartEDRFileMon)
	{
		dwBufLen = 0;
		dwListEmpty = 1;

		memset(pDataBuf, 0x00, nDataSize);
		
		if( !DeviceIoControl(g_hNicFsfd, IOCTL_EDR_FILE_GET_HASH, NULL, 0, pDataBuf, nDataSize, &dwBufLen, NULL) ) 
		{
			DWORD dwError = GetLastError();
			dwListEmpty = dwBufLen = 0;
		}

		if( dwBufLen == nDataSize )
		{
			string filepath, hash;
			wstring wfilepath, whash;

			if (bIsWow64)
				wfilepath = (PWCHAR)((PCHECKEDRFILE_DATA64)pDataBuf)->FileName;
			else
				wfilepath = (PWCHAR)((PCHECKEDRFILE_DATA)pDataBuf)->FileName;

			filepath = CW2A(wfilepath.c_str());

			hash = CUtil::GetSha256(filepath);
			whash.assign(hash.begin(), hash.end());
			wcscpy_s((PWCHAR)(((PCHECKEDRFILE_DATA64)pDataBuf)->Hash), sizeof((PCHECKEDRFILE_DATA64)pDataBuf)->Hash / sizeof(WCHAR), whash.c_str());

			if( !DeviceIoControl(g_hNicFsfd, IOCTL_EDR_FILE_RESULT_HASH, pDataBuf, nDataSize, &dwListEmpty, 4, &dwBufLen, NULL) ) 
			{
				DWORD dwError = GetLastError();
				StopEDRFileHashMgr();
			}

			memset(pDataBuf, 0x00, nDataSize);
		}	
		else
		{
			dwListEmpty = 0;
		}

		if ( g_bStartEDRFileMon && !dwListEmpty) 
		{
			WaitForSingleObject(g_hR0EDRFileInterfaceEvent, 3000);
		}
	}

	if (g_hEDRFileThread) 
	{
		CloseHandle(g_hEDRFileThread);
	}

	if ( pDataBuf )
		free(pDataBuf);

	return 1;
}

BOOL StartEDRFileHashMgr( ) 
{
	if (g_hNicFsfd == INVALID_HANDLE_VALUE)
	{
		g_hNicFsfd = OpenNicfsfd();
	}

	// Open event for comunication R0                                        
	g_hR0EDRFileInterfaceEvent = OpenEvent(SYNCHRONIZE, FALSE, TEXT("Global\\NICCHECKEDRFILEEVT"));
	
	if(g_hR0EDRFileInterfaceEvent == NULL)
	{
		DWORD dwError = GetLastError();
		if (g_hNicFsfd != INVALID_HANDLE_VALUE)
			CloseHandle(g_hNicFsfd); 
		
		g_hNicFsfd = INVALID_HANDLE_VALUE;
		return FALSE;
	}
	
	g_bStartEDRFileMon = TRUE;
	g_hEDRFileThread = ::CreateThread(NULL, 0, ProcessKernelDataEDRFile, NULL, 0, NULL); 
	
	if (!g_hEDRFileThread) 
	{
		if (g_hNicFsfd != INVALID_HANDLE_VALUE) 
			CloseHandle(g_hNicFsfd); 

		if (g_hR0EDRFileInterfaceEvent) 
			CloseHandle(g_hR0EDRFileInterfaceEvent); 
		
		g_hR0EDRFileInterfaceEvent = NULL;
		g_hNicFsfd = INVALID_HANDLE_VALUE;

		return FALSE;
	}
	
	return TRUE;
}

BOOL StopEDRFileHashMgr( ) 
{
	g_bStartEDRFileMon = FALSE;

	if ( g_hR0EDRFileInterfaceEvent )
	{
		SetEvent(g_hR0EDRFileInterfaceEvent);
		
		CloseHandle(g_hR0EDRFileInterfaceEvent);
		g_hR0EDRFileInterfaceEvent = NULL;
	}
	
	return TRUE;
}

