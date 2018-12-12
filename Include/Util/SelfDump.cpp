#include "stdafx.h"
#include "SelfDump.h"
#include <strsafe.h>

static TCHAR			g_szDumpPreFixName[MAX_PATH] = { 0, };
static TCHAR			g_szDumpPath[MAX_PATH_SELFDUMP] = { 0, };
static DWORD			g_dwDumpCapacityBytes = DEFAULT_DUMP_CAPACITY;
static MINIDUMP_TYPE	g_nMiniDumpType = MiniDumpNormal;
static pfnCbOnCrash		g_pfnCallback = NULL;
static LPVOID			g_pUserData = NULL;

CSelfDump::~CSelfDump()
{

}

CSelfDump::CSelfDump(
	IN LPCTSTR lpszDumpPreFixName/*=NULL*/
	, IN MINIDUMP_TYPE nMiniDumpType/*=MiniDumpNormal*/
	, IN DWORD dwDumpCapacityBytes/*=DEFAULT_DUMP_CAPACITY*/
	, IN LPCTSTR lpszDumpPath/*=NULL*/
	, IN pfnCbOnCrash pfnCallback/*=NULL*/
	, IN LPVOID lpUserData/*=NULL*/
)
{
	RegisterExceptionFilter(
		lpszDumpPreFixName
		, nMiniDumpType
		, dwDumpCapacityBytes
		, lpszDumpPath
		, pfnCallback
		, lpUserData
	);
}

LONG CSelfDump::CbTopLevelExceptionFilter(
	struct _EXCEPTION_POINTERS *pExceptionInfo
)
{
	LONG							lRtnValue = EXCEPTION_CONTINUE_SEARCH;
	HANDLE							hFile = INVALID_HANDLE_VALUE;
	BOOL							bRtnValue = FALSE;
	TCHAR							szPath[MAX_PATH_SELFDUMP] = { 0, };
	HMODULE							hDll = NULL;
	LPFN_MinuDumpWriteDump			pfn = NULL;
	SYSTEMTIME						stTime = { 0, };
	_MINIDUMP_EXCEPTION_INFORMATION	stExceptInfo = { 0, };

	if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributes(g_szDumpPath))
	{
		lRtnValue = EXCEPTION_CONTINUE_SEARCH;
		goto FINAL;
	}

	if (NULL != g_pfnCallback)
	{
		if (FALSE == (*g_pfnCallback)(g_pUserData))
		{
			LOGW << "2-2";
			WriteLogFile(g_szDumpPath, TEXT(StringNull), &stTime, g_szDumpPreFixName, FALSE);
			lRtnValue = EXCEPTION_CONTINUE_SEARCH;
			goto FINAL;
		}
	}

	if ((MINIDUMP_TYPE)-1 == g_nMiniDumpType)
	{
		WriteLogFile(g_szDumpPath, TEXT(StringNull), &stTime, g_szDumpPreFixName, FALSE);
		lRtnValue = EXCEPTION_CONTINUE_SEARCH;
		goto FINAL;
	}

	hDll = CSelfDump::LoadLibrary_DbgHelp();
	if (NULL == hDll)
	{
		WriteLogFile(g_szDumpPath, TEXT(StringNull), &stTime, g_szDumpPreFixName, FALSE);
		lRtnValue = EXCEPTION_CONTINUE_SEARCH;
		goto FINAL;
	}

	pfn = (LPFN_MinuDumpWriteDump)::GetProcAddress(hDll, "MiniDumpWriteDump");
	if (NULL == pfn)
	{
		WriteLogFile(g_szDumpPath, TEXT(StringNull), &stTime, g_szDumpPreFixName, FALSE);
		lRtnValue = EXCEPTION_CONTINUE_SEARCH;
		goto FINAL;
	}

	if (FALSE == CSelfDump::GetDumpFileFullPathName(g_szDumpPath,
		g_szDumpPreFixName,
		szPath,
		MAX_PATH_SELFDUMP,
		&stTime))
	{
		::GetLocalTime(&stTime);
		WriteLogFile(g_szDumpPath, TEXT(StringNull), &stTime, g_szDumpPreFixName, FALSE);
		lRtnValue = EXCEPTION_CONTINUE_SEARCH;
		goto FINAL;
	}

	hFile = ::CreateFile(szPath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		WriteLogFile(g_szDumpPath, szPath, &stTime, g_szDumpPreFixName, FALSE);
		lRtnValue = EXCEPTION_CONTINUE_SEARCH;
		goto FINAL;
	}

	stExceptInfo.ThreadId = ::GetCurrentThreadId();
	stExceptInfo.ExceptionPointers = pExceptionInfo;
	stExceptInfo.ClientPointers = NULL;

	if (FALSE == CSelfDump::ManageCapacity(g_szDumpPath,
		g_szDumpPreFixName,
		g_dwDumpCapacityBytes))
	{
		WriteLogFile(g_szDumpPath, szPath, &stTime, g_szDumpPreFixName, FALSE);
		lRtnValue = EXCEPTION_CONTINUE_SEARCH;
		goto FINAL;
	}

	bRtnValue = (*pfn)(::GetCurrentProcess(),
		::GetCurrentProcessId(),
		hFile,
		MiniDumpNormal,
		&stExceptInfo,
		NULL,
		NULL);
	if (TRUE == bRtnValue)
	{
		WriteLogFile(g_szDumpPath, szPath, &stTime, g_szDumpPreFixName, TRUE);

		lRtnValue = EXCEPTION_CONTINUE_SEARCH;
	}
	else
	{
		WriteLogFile(g_szDumpPath, szPath, &stTime, g_szDumpPreFixName, FALSE);
		lRtnValue = EXCEPTION_CONTINUE_SEARCH;
	}

FINAL:

	if (INVALID_HANDLE_VALUE != hFile)
	{
		::CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
		pfn = NULL;
	}
	if (NULL != hDll)
	{
		::FreeLibrary(hDll);
		hDll = NULL;
		pfn = NULL;
	}

	PRINT_STACK_TRACE();

	exit(-1);
}

VOID CSelfDump::SetMiniDumpTypeWhenExist(
	IN LPCTSTR lpszRegKeyPath
	, IN LPCTSTR lpszValueName
)
{
	HKEY	hKey = NULL;
	LONG	lRtnValue = 0;
	DWORD	dwType = 0;
	DWORD	dwCbSize = 0;
	DWORD	dwValue = 0;

	if ((NULL == lpszRegKeyPath) || (NULL == lpszValueName))
	{
		goto FINAL;
	}

	lRtnValue = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		lpszRegKeyPath,
		0,
		KEY_READ,
		&hKey);
	if (ERROR_SUCCESS != lRtnValue)
	{
		goto FINAL;
	}

	dwType = REG_DWORD;
	dwCbSize = sizeof(dwValue);
	lRtnValue = ::RegQueryValueEx(hKey,
		lpszValueName,
		NULL,
		&dwType,
		(LPBYTE)&dwValue,
		&dwCbSize);
	if (ERROR_SUCCESS != lRtnValue)
	{
		goto FINAL;
	}

	g_nMiniDumpType = (MINIDUMP_TYPE)dwValue;

FINAL:
	if (NULL != hKey)
	{
		::RegCloseKey(hKey);
		hKey = NULL;
	}
}

VOID CSelfDump::WriteLogFile(
	IN LPCTSTR lpszPath
	, IN LPCTSTR lpszDumpFileFullPathName
	, IN PSYSTEMTIME pstTimeOccur
	, IN LPCTSTR lpszDumpPrefixName
	, IN BOOL bWriteDumpSuccess
)
{
	TCHAR szLogPath[MAX_PATH_SELFDUMP] = { 0, };
	TCHAR szSection[MAX_PATH] = { 0, };
	TCHAR szTmp[MAX_PATH] = { 0, };

	if ((NULL == lpszPath) || (NULL == lpszDumpFileFullPathName) || (NULL == pstTimeOccur) || (NULL == lpszDumpPrefixName))
	{
		return;
	}

	StringCchPrintf(
		szLogPath
		, MAX_PATH_SELFDUMP
		, TEXT("%s\\CrashDmp.log")
		, lpszPath
	);
	StringCchPrintf(
		szSection
		, MAX_PATH
		, TEXT("%.4d%.2d%.2d_%.2d%.2d%.2d%.3d")
		, pstTimeOccur->wYear
		, pstTimeOccur->wMonth
		, pstTimeOccur->wDay
		, pstTimeOccur->wHour
		, pstTimeOccur->wMinute
		, pstTimeOccur->wSecond
		, pstTimeOccur->wMilliseconds
	);

	::WritePrivateProfileString(szSection, TEXT("Prefix"), lpszDumpPrefixName, szLogPath);
	::WritePrivateProfileString(szSection, TEXT("DumpPath"), lpszDumpFileFullPathName, szLogPath);

	StringCchPrintf(szTmp, MAX_PATH, TEXT("%d"), ::GetCurrentProcessId());
	::WritePrivateProfileString(szSection, TEXT("ProcessId"), szTmp, szLogPath);

	::GetModuleFileName(NULL, szTmp, MAX_PATH);
	::WritePrivateProfileString(szSection, TEXT("CrashModulePath"), szTmp, szLogPath);

	if (TRUE == bWriteDumpSuccess)
	{
		::WritePrivateProfileString(szSection, TEXT("SuccessWriteDump"), TEXT("yes"), szLogPath);
	}
	else
	{
		::WritePrivateProfileString(szSection, TEXT("SuccessWriteDump"), TEXT("no"), szLogPath);
	}
}

BOOL CSelfDump::RegisterExceptionFilter(
	IN LPCTSTR lpszDumpPreFixName/*=NULL*/
	, IN MINIDUMP_TYPE nMiniDumpType/*=MiniDumpNormal*/
	, IN DWORD dwDumpCapacityBytes/*=DEFAULT_DUMP_CAPACITY*/
	, IN LPCTSTR lpszDumpPath/*=NULL*/
	, IN pfnCbOnCrash pfnCallback/*=NULL*/
	, IN LPVOID lpUserData/*=NULL*/
)
{
	BOOL	bRtnValue = TRUE;
	DWORD	dwRtnValue = 0;

	if (NULL != lpszDumpPreFixName)
	{
		StringCchCopy(g_szDumpPreFixName, MAX_PATH, lpszDumpPreFixName);
	}
	else
	{
		GetDefaultDumpPreFixName(g_szDumpPreFixName, MAX_PATH);
	}

	if (DEFAULT_DUMP_CAPACITY != dwDumpCapacityBytes)
	{
		g_dwDumpCapacityBytes = dwDumpCapacityBytes;
	}

	if (NULL != lpszDumpPath)
	{
		StringCchCopy(g_szDumpPath, MAX_PATH_SELFDUMP, lpszDumpPath);
	}
	else
	{
		dwRtnValue = CSelfDump::GetDefaultModuleTempPath(g_szDumpPath, MAX_PATH_SELFDUMP);
		if (ERROR_SUCCESS != dwRtnValue)
		{
			bRtnValue = FALSE;
			goto FINAL;
		}
	}

	g_nMiniDumpType = nMiniDumpType;
	g_pfnCallback = pfnCallback;
	g_pUserData = lpUserData;

	::SetUnhandledExceptionFilter(CSelfDump::CbTopLevelExceptionFilter);

	bRtnValue = TRUE;

FINAL:
	return bRtnValue;
}

BOOL CSelfDump::GetDumpFileFullPathName(
	IN LPCTSTR lpszPath
	, IN LPCTSTR lpszDumpPreFixName
	, OUT LPTSTR lpszDumpFileFullPathName
	, IN DWORD dwCchDumpFileFullPathName
	, OUT PSYSTEMTIME pstTime
)
{
	BOOL bRtnValue = TRUE;

	if ((NULL == lpszPath) ||
		(NULL == lpszDumpPreFixName) ||
		(NULL == lpszDumpFileFullPathName) ||
		(NULL == pstTime))
	{
		bRtnValue = FALSE;
		goto FINAL;
	}

	::GetLocalTime(pstTime);
	StringCchPrintf(lpszDumpFileFullPathName,
		dwCchDumpFileFullPathName,
		TEXT("%s\\%s_%.4d%.2d%.2d%.2d%.2d%.2d.dmp"),
		lpszPath,
		lpszDumpPreFixName,
		pstTime->wYear,
		pstTime->wMonth,
		pstTime->wDay,
		pstTime->wHour,
		pstTime->wMinute,
		pstTime->wSecond);
	bRtnValue = TRUE;
FINAL:
	return bRtnValue;
}

DWORD CSelfDump::GetFilesSize(
	IN LPCTSTR lpszFindFileName
	, OUT LPTSTR lpszEarlyFileName
	, IN DWORD dwCchEarlyFileName
)
{
	HANDLE			hFind = INVALID_HANDLE_VALUE;
	DWORD			dwTotalBytes = 0;
	WIN32_FIND_DATA	FindFileData = { 0, };

	dwTotalBytes = 0;

	if ((NULL == lpszFindFileName) || (NULL == lpszEarlyFileName))
	{
		goto FINAL;
	}

	ZeroMemory(lpszEarlyFileName, sizeof(TCHAR) * dwCchEarlyFileName);

	hFind = ::FindFirstFile(lpszFindFileName, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		dwTotalBytes = 0;
		goto FINAL;
	}

	if (TEXT('.') != FindFileData.cFileName[0])
	{
		dwTotalBytes += FindFileData.nFileSizeLow;
		StringCchCopy(lpszEarlyFileName, dwCchEarlyFileName, FindFileData.cFileName);
	}

	while (FindNextFile(hFind, &FindFileData) != 0)
	{
		if (TEXT('.') == FindFileData.cFileName[0])
		{
			continue;
		}

		dwTotalBytes += FindFileData.nFileSizeLow;

		if (TEXT(CharNull) == lpszEarlyFileName[0])
		{
			StringCchCopy(lpszEarlyFileName, dwCchEarlyFileName, FindFileData.cFileName);
			continue;
		}

		if (0 < _tcscmp(lpszEarlyFileName, FindFileData.cFileName))
		{
			StringCchCopy(lpszEarlyFileName, dwCchEarlyFileName, FindFileData.cFileName);
		}
	}

FINAL:

	if (INVALID_HANDLE_VALUE != hFind)
	{
		::FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
	}

	return dwTotalBytes;
}

BOOL CSelfDump::ManageCapacity(
	IN LPCTSTR lpszPath
	, IN LPCTSTR lpszDumpPreFixName
	, IN DWORD dwDumpCapacityBytes
)
{
	INT		i = 0;
	BOOL	bRtnValue = FALSE;
	DWORD	dwCbFileSize = 0;
	TCHAR	szFileName[MAX_PATH_SELFDUMP] = { 0, };
	TCHAR	szDelFileName[MAX_PATH_SELFDUMP] = { 0, };
	TCHAR	szEarlyFileName[MAX_PATH_SELFDUMP] = { 0, };

	if ((NULL == lpszPath) || (NULL == lpszDumpPreFixName))
	{
		bRtnValue = FALSE;
		goto FINAL;
	}

	StringCchPrintf(szFileName,
		MAX_PATH_SELFDUMP,
		TEXT("%s\\%s_*.dmp"),
		lpszPath,
		lpszDumpPreFixName);

	for (i = 0; i < MAX_DEL_TRY_MANAGE; i++)
	{
		dwCbFileSize = CSelfDump::GetFilesSize(szFileName,
			szEarlyFileName,
			MAX_PATH_SELFDUMP);

		if (dwCbFileSize < g_dwDumpCapacityBytes)
		{
			break;
		}

		StringCchPrintf(szDelFileName, MAX_PATH_SELFDUMP, TEXT("%s\\%s"), g_szDumpPath, szEarlyFileName);
		::DeleteFile(szDelFileName);
	}

	bRtnValue = TRUE;

FINAL:

	return bRtnValue;
}

HMODULE CSelfDump::LoadLibrary_DbgHelp()
{
	HMODULE	hDLL = NULL;
	TCHAR	szDbgHelp[MAX_PATH_SELFDUMP] = { 0, };
	LPTSTR	lpszFound = NULL;

	if (0 != ::GetModuleFileName(NULL, szDbgHelp, MAX_PATH_SELFDUMP))
	{
		lpszFound = _tcsrchr(szDbgHelp, TEXT(CharBackslash));
		if ((NULL != lpszFound) && (szDbgHelp < lpszFound))
		{
			*lpszFound = TEXT(CharNull);
			StringCchCat(szDbgHelp, MAX_PATH_SELFDUMP, TEXT("\\DbgHelp.dll"));

			hDLL = ::LoadLibrary(szDbgHelp);
			if (NULL != hDLL)
			{
				goto FINAL;
			}
		}
	}

	hDLL = CSelfDump::LoadLibraryFromSystem(TEXT("DbgHelp.dll"));

FINAL:
	return hDLL;
}

HMODULE CSelfDump::LoadLibraryFromSystem(
	IN LPCTSTR lpFileName
)
{
	if (NULL == lpFileName) {
		return NULL;
	}

	if (0 == lpFileName[0]) {
		return NULL;
	}

	const int BUF_SIZ = 4096;

	LPTSTR lpszSysDir = NULL;
	UINT nNeedSize = GetSystemDirectory(NULL, 0);
	lpszSysDir = (LPTSTR)malloc((nNeedSize + 1) * sizeof(TCHAR));
	if (NULL == lpszSysDir) {
		return NULL;
	}

	if (0 == GetSystemDirectory(lpszSysDir, nNeedSize + 1)) {
		free(lpszSysDir);
		lpszSysDir = NULL;
		return NULL;
	}

	TCHAR szFullPath[BUF_SIZ] = { 0, };
	//_tcsncpy(szFullPath, lpszSysDir, BUF_SIZ);
	//_tcsncat(szFullPath, _T(PathDelimeter), BUF_SIZ);
	//_tcsncat(szFullPath, lpFileName, BUF_SIZ);
	_tcsncpy_s(szFullPath, BUF_SIZ, lpszSysDir, _TRUNCATE);
	_tcsncat_s(szFullPath, BUF_SIZ, _T(PathDelimeter), _TRUNCATE);
	_tcsncat_s(szFullPath, BUF_SIZ, lpFileName, _TRUNCATE);

	HMODULE hModule = LoadLibraryEx(szFullPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

	free(lpszSysDir);
	lpszSysDir = NULL;

	if (hModule) {
		return hModule;
	}

	return NULL;
}

DWORD CSelfDump::GetDefaultModuleTempPath(
	OUT LPTSTR lpszPath
	, IN DWORD dwCchPath
)
{
	DWORD	dwRtnValue = ERROR_SUCCESS;
	LPTSTR	lpszFound = NULL;

	if (NULL == lpszPath)
	{
		dwRtnValue = ERROR_INVALID_PARAMETER;
		goto FINAL;
	}

	ZeroMemory(lpszPath, sizeof(TCHAR) * dwCchPath);

	if (0 == ::GetModuleFileName(NULL, lpszPath, dwCchPath))
	{
		dwRtnValue = ::GetLastError();
		goto FINAL;
	}

	lpszFound = _tcsrchr(lpszPath, TEXT(CharBackslash));
	if (NULL == lpszFound)
	{
		dwRtnValue = ERROR_PATH_NOT_FOUND;
		goto FINAL;
	}

	if (lpszFound <= lpszPath)
	{
		dwRtnValue = ERROR_PATH_NOT_FOUND;
		goto FINAL;
	}

	*lpszFound = TEXT(CharNull);

	StringCchCat(lpszPath, dwCchPath, TEXT("\\Temp"));

	dwRtnValue = ERROR_SUCCESS;

FINAL:
	return dwRtnValue;
}

DWORD CSelfDump::GetDefaultDumpPreFixName(
	OUT LPTSTR lpszDefaultDumpPreFixName
	, IN DWORD dwCchDefaultDumpPreFixName
)
{
	DWORD	dwRtnValue = ERROR_SUCCESS;
	TCHAR	szModulePath[MAX_PATH_SELFDUMP] = { 0, };
	LPTSTR	lpszFound = NULL;
	LPTSTR	lpszIter = NULL;
	BOOL	bFound = FALSE;
	INT		i = 0;

	if (NULL == lpszDefaultDumpPreFixName)
	{
		dwRtnValue = ERROR_INVALID_PARAMETER;
		goto FINAL;
	}

	ZeroMemory(lpszDefaultDumpPreFixName, sizeof(TCHAR)*dwCchDefaultDumpPreFixName);

	if (0 == ::GetModuleFileName(NULL, szModulePath, MAX_PATH_SELFDUMP))
	{
		dwRtnValue = ::GetLastError();
		StringCchPrintf(lpszDefaultDumpPreFixName, dwCchDefaultDumpPreFixName, TEXT("UnKnown"));
		goto FINAL;
	}

	lpszFound = _tcsrchr(szModulePath, TEXT(CharBackslash));
	if ((NULL == lpszFound) || (lpszFound <= szModulePath) || (TEXT(CharNull) == *(lpszFound + 1)))
	{
		dwRtnValue = ERROR_PATH_NOT_FOUND;
		StringCchPrintf(lpszDefaultDumpPreFixName, dwCchDefaultDumpPreFixName, TEXT("UnKnown"));
		goto FINAL;
	}

	bFound = FALSE;
	for (i = 0; i < MAX_PATH; i++)
	{
		if (TEXT(CharNull) == *(lpszFound + i))
		{
			break;
		}

		if (TEXT('.') == *(lpszFound + i))
		{
			bFound = TRUE;
			*(lpszFound + i) = TEXT(CharNull);
			break;
		}
	}

	if (FALSE == bFound)
	{
		dwRtnValue = ERROR_PATH_NOT_FOUND;
		StringCchPrintf(lpszDefaultDumpPreFixName, dwCchDefaultDumpPreFixName, TEXT("UnKnown"));
		goto FINAL;
	}

	dwRtnValue = ERROR_SUCCESS;
	StringCchCopy(lpszDefaultDumpPreFixName, dwCchDefaultDumpPreFixName, lpszFound + 1);

FINAL:
	return dwRtnValue;
}