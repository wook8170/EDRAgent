#include <dbghelp.h>

typedef BOOL(WINAPI *LPFN_MinuDumpWriteDump)(
	HANDLE hProcess
	, DWORD dwPid
	, HANDLE hFile
	, MINIDUMP_TYPE DumpType
	, PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam
	, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam
	, PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);

typedef BOOL(*pfnCbOnCrash)(IN LPVOID lpUserData);

#define DEFAULT_DUMP_CAPACITY	1048576	// 1MB
#define MAX_DEL_TRY_MANAGE		32
#define MAX_PATH_SELFDUMP		1024

class CSelfDump
{
public:
	CSelfDump(
		IN LPCTSTR lpszDumpPreFixName = NULL
		, IN MINIDUMP_TYPE nMiniDumpType = MiniDumpNormal
		, IN DWORD dwDumpCapacityBytes = DEFAULT_DUMP_CAPACITY
		, IN LPCTSTR lpszDumpPath = NULL
		, IN pfnCbOnCrash pfnCallback = NULL
		, IN LPVOID lpUserData = NULL
	);

	virtual ~CSelfDump();

public:
	static BOOL RegisterExceptionFilter(
		IN LPCTSTR lpszDumpPreFixName = NULL
		, IN MINIDUMP_TYPE nMiniDumpType = MiniDumpNormal
		, IN DWORD dwDumpCapacityBytes = DEFAULT_DUMP_CAPACITY
		, IN LPCTSTR lpszDumpPath = NULL
		, IN pfnCbOnCrash pfnCallback = NULL
		, IN LPVOID lpUserData = NULL
	);

	static VOID SetMiniDumpTypeWhenExist(
		IN LPCTSTR lpszRegKeyPath
		, IN LPCTSTR lpszValueName
	);

private:
	static LONG WINAPI CbTopLevelExceptionFilter(
		struct _EXCEPTION_POINTERS *pExceptionInfo
	);

	static VOID WriteLogFile(
		IN LPCTSTR lpszPath
		, IN LPCTSTR lpszDumpFileFullPathName
		, IN PSYSTEMTIME pstTimeOccur
		, IN LPCTSTR lpszDumpPrefixName
		, IN BOOL bWriteDumpSuccess
	);

	// Utility
private:
	static DWORD	GetDefaultDumpPreFixName(
		OUT LPTSTR lpszDefaultDumpPreFixName
		, IN DWORD dwCchDefaultDumpPreFixName
	);

	static DWORD	GetDefaultModuleTempPath(
		OUT LPTSTR lpszPath
		, IN DWORD dwCchPath);

	static HMODULE	LoadLibrary_DbgHelp();

	static HMODULE	LoadLibraryFromSystem(
		IN LPCTSTR lpFileName
	);

	static BOOL		ManageCapacity(
		IN LPCTSTR lpszPath
		, IN LPCTSTR lpszDumpPreFixName
		, IN DWORD dwDumpCapacityBytes
	);

	static BOOL		GetDumpFileFullPathName(
		IN LPCTSTR lpszPath
		, IN LPCTSTR lpszDumpPreFixName
		, OUT LPTSTR lpszDumpFileFullPathName
		, IN DWORD dwCchDumpFileFullPathName
		, OUT PSYSTEMTIME pstTime
	);

	static DWORD	GetFilesSize(
		IN LPCTSTR lpszFindFileName
		, OUT LPTSTR lpszEarlyFileName
		, IN DWORD dwCchEarlyFileName
	);
};