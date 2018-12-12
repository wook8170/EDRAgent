#pragma once

#include <winternl.h>
#include <psapi.h>

#define STRSAFE_LIB
#include <strsafe.h>

#include "ProcMonCommon.h"
#include "ProcMonDll.h"
#include "ProcMonControl.h"
#include "ProcMonTypes.h"
#include "Util/Util.h"
#include "Util/HashList.h"
#include "ThreadPool/LockGuard.h"

#include <vector>

#include "openssl/sha.h"
#include "openssl/md5.h"

#pragma comment(lib, "strsafe.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "psapi.lib")


#ifndef NTSTATUS
#define LONG NTSTATUS
#endif

// Unicode path usually prefix with '\\?\'
#define MAX_UNICODE_PATH	32767L
// Used in PEB struct
typedef ULONG PPS_POST_PROCESS_INIT_ROUTINE_T;

// Used in PEB struct
typedef struct _PEB_LDR_DATA_T {
	BYTE Reserved1[8];
	PVOID Reserved2[3];
	LIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATA_T, *PPEB_LDR_DATA_T;

// Used in PEB struct
typedef struct _RTL_USER_PROCESS_PARAMETERS_T {
	BYTE Reserved1[16];
	PVOID Reserved2[10];
	UNICODE_STRING ImagePathName;
	UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS_T, *PRTL_USER_PROCESS_PARAMETERS_T;

// Used in PROCESS_BASIC_INFORMATION struct
typedef struct _PEB_T {
	BYTE Reserved1[2];
	BYTE BeingDebugged;
	BYTE Reserved2[1];
	PVOID Reserved3[2];
	PPEB_LDR_DATA_T Ldr;
	PRTL_USER_PROCESS_PARAMETERS_T ProcessParameters;
	BYTE Reserved4[104];
	PVOID Reserved5[52];
	PPS_POST_PROCESS_INIT_ROUTINE_T PostProcessInitRoutine;
	BYTE Reserved6[128];
	PVOID Reserved7[1];
	ULONG SessionId;
} PEB_T, *PPEB_T;

// Used with NtQueryInformationProcess
typedef struct _PROCESS_BASIC_INFORMATION_T {
	LONG ExitStatus;
	PPEB_T PebBaseAddress;
	ULONG_PTR AffinityMask;
	LONG BasePriority;
	ULONG_PTR UniqueProcessId;
	ULONG_PTR InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION_T, *PPROCESS_BASIC_INFORMATION_T;

// NtQueryInformationProcess in NTDLL.DLL
typedef NTSTATUS(NTAPI *pfnNtQueryInformationProcess)(
	IN	HANDLE ProcessHandle,
	IN	PROCESSINFOCLASS ProcessInformationClass,
	OUT	PVOID ProcessInformation,
	IN	ULONG ProcessInformationLength,
	OUT	PULONG ReturnLength	OPTIONAL
	);

typedef struct _PROCESSINFO_T
{
	DWORD	dwPID;
	DWORD	dwParentPID;
	DWORD	dwSessionID;
	DWORD	dwPEBBaseAddress;
	DWORD	dwAffinityMask;
	LONG	dwBasePriority;
	LONG	dwExitStatus;
	BYTE	cBeingDebugged;
	TCHAR	szImgPath[MAX_UNICODE_PATH];
	TCHAR	szCmdLine[MAX_UNICODE_PATH];
} PROCESSINFO_T;

typedef struct _PROCESS_TREE_ITEM_T				PROCESS_TREE_ITEM;
typedef struct _PROCESS_CALLBACK_CONTEXT_T		PROCESS_CALLBACK_CONTEXT;
typedef shared_ptr<PROCESS_TREE_ITEM>			PROCESS_TREE_ITEM_Ptr;
typedef shared_ptr<PROCESS_CALLBACK_CONTEXT>	PROCESS_CALLBACK_CONTEXT_Ptr;
typedef shared_ptr<CHashList>					CHashList_Ptr;

class CProcess
{
public:
	multimap<string, PROCESS_TREE_ITEM_Ptr>		m_processMap;
	multimap<string, PROCESS_TREE_ITEM_Ptr>		m_processMapKernel;
	BOOL										m_bStartProcessMonitor;
	vector<PROCESS_CALLBACK_CONTEXT_Ptr>		m_vecProcessCallback;
	CHashList_Ptr								m_pHashList;

private:
	LOCK										m_lock;


protected:
	CProcess();

	pfnNtQueryInformationProcess				m_pNtQueryInformationProcess;

	HMODULE LoadNTDLLFunctions();

	VOID FreeNTDLLFunctions(
		IN HMODULE hNtDll
	);

	BOOL EnableTokenPrivilege(
		IN LPCTSTR pszPrivilege
	);

	BOOL KillProcessTree(
		DWORD myprocID, DWORD dwTimeout
	);

public:
	~CProcess();

	LOCK * GetLock();

	static CProcess* Instance();

	VOID StartProcessMonitoring();

	VOID StopProcessMonitoring();

	VOID CProcess::RegisterCallback(
		PPROCESSES_CALLBACK pCreateCallback
		, PPROCESSES_CALLBACK pDeleteCallback
		, VOID* pUserData
	);

	BOOL IsInit();

	PROCESS_TREE_ITEM_Ptr GetProcessInfo(
		DWORD pid
		, string sha256
	);

	vector<PROCESS_TREE_ITEM_Ptr> FindProcessList(
		string sha256
	);

	BOOL AddProcessList(
		DWORD pid
		, PROCESS_TREE_ITEM_Ptr processInfo
	);

	BOOL RemoveProcessList(
		DWORD pid
		, PROCESS_TREE_ITEM_Ptr processInfo
	);

	multimap<string, PROCESS_TREE_ITEM_Ptr>& GetProcessList();

	multimap<string, PROCESS_TREE_ITEM_Ptr>& GetProcessListFromKernel();

	BOOL GetNtProcessInfo(
		IN const DWORD dwPID
		, OUT PROCESSINFO_T *ppi
	);

	BOOL KillProcess(
		DWORD PID
	);

	BOOL KillProcess(
		string  hashOrPath
		, BOOL bIsHash = TRUE
	);

	static VOID CALLBACK ListCallbackForCreate(
		PCHECK_LIST_ENTRY CheckList[]
		, ULONG CheckListCount
		, VOID* UserData
	);

	static VOID CALLBACK ListCallbackForDelete(
		PCHECK_LIST_ENTRY CheckList[]
		, ULONG CheckListCount
		, VOID* UserData
	);

	EDRJson GetRecentProcess(
		string path
	);
};

typedef struct _PROCESS_TREE_ITEM_T
{
	DWORD	dwPID;
	DWORD	dwPPID;

	CHAR	szImgPath[260];
	CHAR	szMD5[SHA256_DIGEST_LENGTH * 2 + 65];
	CHAR	szSHA256[SHA256_DIGEST_LENGTH * 2 + 65];

	_PROCESS_TREE_ITEM_T()
	{
		EDRZeroMemory(szImgPath, 260);
		EDRZeroMemory(szMD5, SHA256_DIGEST_LENGTH * 2 + 65);
		EDRZeroMemory(szSHA256, SHA256_DIGEST_LENGTH * 2 + 65);
	}

	_PROCESS_TREE_ITEM_T(PPROCESS_INFO pInfo)
	{
		string path = CUtil::ConvU2M(pInfo->ImagePath);
		EDRJson recentFile = CProcess::Instance()->GetRecentProcess(path);

		string	imagePath = CUtil::ConvU2M(pInfo->ImagePath);
		string  imageHashMD5 = recentFile["md5"].asString();
		string  imageHashSHA256 = recentFile["sha256"].asString();


		DWORD	pid = (DWORD)pInfo->ProcessId;
		DWORD	ppid = (DWORD)pInfo->ParentProcessId;

		dwPID = pid;
		dwPPID = ppid;

		EDRZeroMemory(szImgPath, 260);
		EDRZeroMemory(szMD5, SHA256_DIGEST_LENGTH * 2 + 65);
		EDRZeroMemory(szSHA256, SHA256_DIGEST_LENGTH * 2 + 65);

		EDRCopyMemory(szImgPath, imagePath.c_str(), imagePath.length());
		EDRCopyMemory(szMD5, imageHashMD5.c_str(), imageHashMD5.length());
		EDRCopyMemory(szSHA256, imageHashSHA256.c_str(), imageHashSHA256.length());
	}

	~_PROCESS_TREE_ITEM_T()
	{
	}

} PROCESS_TREE_ITEM;

typedef struct _PROCESS_CALLBACK_CONTEXT_T
{
	PPROCESSES_CALLBACK pCreateCallback;
	PPROCESSES_CALLBACK pDeleteCallback;
	VOID*				pUserData;

	_PROCESS_CALLBACK_CONTEXT_T()
	{
		pCreateCallback = NULL;
		pDeleteCallback = NULL;
		pUserData = NULL;
	};

	_PROCESS_CALLBACK_CONTEXT_T(
		PPROCESSES_CALLBACK createCallback
		, PPROCESSES_CALLBACK deleteCallback
		, VOID* userData
	)
	{
		pCreateCallback = createCallback;
		pDeleteCallback = deleteCallback;
		pUserData = userData;
	};


} PROCESS_CALLBACK_CONTEXT, *PPROCESS_CALLBACK_CONTEXT;
