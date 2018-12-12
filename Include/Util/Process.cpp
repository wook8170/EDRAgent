#include "StdAfx.h"
#include "CommonDef.h"
#include "Util/Process.h"
#include "Util/Util.h"
#include "ProcmonDll.h"
#include "EDRAgent.h"
#include "Util/FileSystem.h"
#include <TlHelp32.h>
#include "EDRWorkerThread.h"

#include "EDRAgentDll.h"

#define MAX_PI (10*1024)
#define MAX_TIMEOUT (10000)

class CEDRAgentApp;

extern CEDRAgentApp theApp;

CProcess::CProcess()
{
	LockGuard::Create(GetLock());
}

CProcess::~CProcess()
{
	LockGuard::Destroy(GetLock());
}

LOCK * CProcess::GetLock()
{
	return &m_lock;
}

CProcess* CProcess::Instance()
{
	static CProcess * pInstance = NULL;
	if (NULL == pInstance)
	{
		static CCriticalSection criticalSection; // multi-thread check
		criticalSection.Lock();
		if (NULL == pInstance)
		{
			static CProcess pRealInstance;
			pInstance = &pRealInstance;

			pRealInstance.m_pHashList = make_shared<CHashList>("RECENT_PROCESS_HASH", "RECENT_PROCESS_HASH");
		}
		criticalSection.Unlock();
	}

	return pInstance;
}

VOID CProcess::StartProcessMonitoring()
{
#ifndef NOT_USE_PROCESS_MONITOR
	// 프로세스 모니터링 시작
	ProcMon::Install();
	ProcMon::Start();
	ProcMon::Init(
		CProcess::ListCallbackForCreate
		, CProcess::ListCallbackForDelete
		, (VOID*)CProcess::Instance()
	);
#endif

	CProcess::m_bStartProcessMonitor = TRUE;

}

VOID CProcess::StopProcessMonitoring()
{
	CProcess::m_bStartProcessMonitor = FALSE;

	ProcMon::Deinit();
	ProcMon::Stop();
	ProcMon::Uninstall();

	Sleep(2 * 1000);

}

VOID CProcess::RegisterCallback(
	PPROCESSES_CALLBACK pCreateCallback
	, PPROCESSES_CALLBACK pDeleteCallback
	, VOID* pUserData
)
{
	if (!m_bStartProcessMonitor)
	{
		LOGW << "NicsProcMon.sys 가 초기화 되지 않았습니다.";
		return;
	}

	PROCESS_CALLBACK_CONTEXT_Ptr pProcessCallback(
		EDRNew PROCESS_CALLBACK_CONTEXT(
			pCreateCallback
			, pDeleteCallback
			, pUserData
		));

	BOOL exist = FALSE;

	for (
		auto it = CProcess::Instance()->m_vecProcessCallback.begin();
		it != CProcess::Instance()->m_vecProcessCallback.end();
		it++
		)
	{
		PROCESS_CALLBACK_CONTEXT_Ptr pProcessCallback = (*it);

		if (pProcessCallback->pUserData == pUserData)
		{
			LOGW << "동일 인스턴스에 대해 등록된 콜백이 이미 존재 합니다.";
			exist = TRUE;
			break;
		}
	}

	if (!exist)
	{
		CProcess::Instance()->m_vecProcessCallback.push_back(pProcessCallback);
	}
}

BOOL CProcess::IsInit()
{
	return CProcess::m_bStartProcessMonitor;
}

PROCESS_TREE_ITEM_Ptr CProcess::GetProcessInfo(
	DWORD pid
	, string sha256
)
{
	//LockGuard lockGuard(GetLock());

	PROCESS_TREE_ITEM_Ptr pProcessInfo;

	auto iterPair = m_processMapKernel.equal_range(sha256);

	for (auto it = iterPair.first; it != iterPair.second; )
	{
		pProcessInfo = it->second;

		if (pProcessInfo->dwPID == pid)
		{
			break;
		}
		else
		{
			it++;
		}
	}

	return pProcessInfo;
}

vector<PROCESS_TREE_ITEM_Ptr> CProcess::FindProcessList(
	string key
)
{
	//LockGuard lockGuard(GetLock());

	BOOL found = FALSE;
	vector<PROCESS_TREE_ITEM_Ptr> vecProcessIdList;

	auto iterPair = m_processMapKernel.equal_range(key);
	for (auto it = iterPair.first; it != iterPair.second; )
	{
		PROCESS_TREE_ITEM_Ptr pProcessInfo = it->second;

		LOGD << "# 프로세스 리스트 발견 (F): "
			<< FILLSETWR(5) << pProcessInfo->dwPID
			<< ", " << FILLSETWR(5) << pProcessInfo->dwPPID
			<< ", " << pProcessInfo->szSHA256
			<< ", " << pProcessInfo->szMD5
			<< ", " << pProcessInfo->szImgPath;

		vecProcessIdList.push_back(pProcessInfo);
		it++;
		found = TRUE;
	}

	if (found)
	{
		return vecProcessIdList;
	}

	for (auto it = m_processMapKernel.begin(); it != m_processMapKernel.end(); )
	{
		PROCESS_TREE_ITEM_Ptr pProcessInfo = it->second;

		if (CUtil::Compare(pProcessInfo->szSHA256, key) == 0
			|| CUtil::Compare(pProcessInfo->szMD5, key) == 0
			|| CUtil::Compare(pProcessInfo->szImgPath, key) == 0
			)
		{
			LOGD << "# 프로세스 리스트 발견 (F): "
				<< FILLSETWR(5) << pProcessInfo->dwPID
				<< ", " << FILLSETWR(5) << pProcessInfo->dwPPID
				<< ", " << pProcessInfo->szSHA256
				<< ", " << pProcessInfo->szMD5
				<< ", " << pProcessInfo->szImgPath;

			vecProcessIdList.push_back(pProcessInfo);
		}

		it++;
	}

	return vecProcessIdList;
}

BOOL CProcess::AddProcessList(
	DWORD pid
	, PROCESS_TREE_ITEM_Ptr processInfo
)
{
	//LockGuard lockGuard(GetLock());

	CHECK_NOTNULL(processInfo);

	BOOL found = FALSE;
	string key = processInfo->szSHA256;

	auto iterPair = m_processMapKernel.equal_range(key);

	///*
	for (auto it = iterPair.first; it != iterPair.second; it++)
	{
		PROCESS_TREE_ITEM_Ptr pProcessInfo = it->second;

		if (processInfo->dwPID == pProcessInfo->dwPID
			&& processInfo->dwPPID == pProcessInfo->dwPPID
			&& CUtil::Compare(processInfo->szSHA256, pProcessInfo->szSHA256) == 0)
		{
			LOGD << "# 프로세스 리스트 중복 (X): "
				<< FILLSETWR(5) << processInfo->dwPID
				<< ", " << FILLSETWR(5) << processInfo->dwPPID
				<< ", " << processInfo->szSHA256
				<< ", " << processInfo->szMD5
				<< ", " << processInfo->szImgPath;

			found = TRUE;
			break;
		}
	}
	//*/
	if (!found)
	{
		LOGD << "# 프로세스 리스트 추가 (+): "
			<< FILLSETWR(5) << processInfo->dwPID
			<< ", " << FILLSETWR(5) << processInfo->dwPPID
			<< ", " << processInfo->szSHA256
			<< ", " << processInfo->szMD5
			<< ", " << processInfo->szImgPath;

		string key = processInfo->szSHA256;

		m_processMapKernel.insert(
			pair<string, PROCESS_TREE_ITEM_Ptr>(key, processInfo)
		);

		return TRUE;
	}

	return FALSE;
}

BOOL CProcess::RemoveProcessList(
	DWORD pid
	, PROCESS_TREE_ITEM_Ptr processInfo
)
{
	//LockGuard lockGuard(GetLock());

	CHECK_NOTNULL(processInfo);

	string key = processInfo->szSHA256;

	auto iterPair = m_processMapKernel.equal_range(key);

	for (auto it = iterPair.first; it != iterPair.second; )
	{
		PROCESS_TREE_ITEM_Ptr pProcessInfo = it->second;

		if (processInfo->dwPID == pProcessInfo->dwPID
			&& processInfo->dwPPID == pProcessInfo->dwPPID
			&& CUtil::Compare(processInfo->szSHA256, pProcessInfo->szSHA256) == 0)
		{
			LOGD << "# 프로세스 리스트 삭제 (-): "
				<< FILLSETWR(5) << pProcessInfo->dwPID
				<< ", " << FILLSETWR(5) << pProcessInfo->dwPPID
				<< ", " << pProcessInfo->szSHA256
				<< ", " << processInfo->szMD5
				<< ", " << pProcessInfo->szImgPath;

			pProcessInfo.reset();
			m_processMapKernel.erase(it++);
		}
		else
		{
			it++;
		}
	}

	return TRUE;
}

multimap<string, PROCESS_TREE_ITEM_Ptr>& CProcess::GetProcessListFromKernel()
{
	//LockGuard lockGuard(GetLock());

	return m_processMapKernel;
}
BOOL CProcess::EnableTokenPrivilege(
	LPCTSTR pszPrivilege
)
{
	HANDLE hToken = 0;
	TOKEN_PRIVILEGES tkp = { 0 };

	if (!OpenProcessToken(
		GetCurrentProcess()
		, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY
		, &hToken
	))
	{
		return FALSE;
	}

	if (LookupPrivilegeValue(
		NULL
		, pszPrivilege
		, &tkp.Privileges[0].Luid
	))
	{
		tkp.PrivilegeCount = 1;
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		AdjustTokenPrivileges(
			hToken
			, FALSE
			, &tkp
			, 0
			, (PTOKEN_PRIVILEGES)NULL
			, 0
		);

		if (GetLastError() != ERROR_SUCCESS)
			return FALSE;

		return TRUE;
	}

	return FALSE;
}

HMODULE CProcess::LoadNTDLLFunctions()
{
	HMODULE hNtDll = LoadLibrary(_T("ntdll.dll"));
	if (hNtDll == NULL) return NULL;

	CProcess::m_pNtQueryInformationProcess =
		(pfnNtQueryInformationProcess)GetProcAddress(
			hNtDll
			, "NtQueryInformationProcess"
		);

	if (CProcess::m_pNtQueryInformationProcess == NULL) {
		FreeLibrary(hNtDll);
		return NULL;
	}
	return hNtDll;
}

VOID CProcess::FreeNTDLLFunctions(
	HMODULE hNtDll
)
{
	if (hNtDll)
	{
		FreeLibrary(hNtDll);
	}

	CProcess::m_pNtQueryInformationProcess = NULL;
}

BOOL CProcess::GetNtProcessInfo(
	const DWORD dwPID
	, PROCESSINFO_T *ppi
)
{
	if (!EnableTokenPrivilege(SE_DEBUG_NAME))
	{
		LOGW << "EnableTokenPrivilege 실패";
		return FALSE;
	}

	BOOL  bReturnStatus = TRUE;
	DWORD dwSize = 0;
	DWORD dwSizeNeeded = 0;
	SIZE_T dwBytesRead = 0;
	DWORD dwBufferSize = 0;
	HANDLE hHeap = 0;
	WCHAR *pwszBuffer = NULL;

	PROCESSINFO_T spi = { 0 };
	PPROCESS_BASIC_INFORMATION_T pbi = NULL;

	PEB_T peb = { 0 };
	PEB_LDR_DATA_T peb_ldr = { 0 };
	RTL_USER_PROCESS_PARAMETERS_T peb_upp = { 0 };

	ZeroMemory(&spi, sizeof(spi));
	ZeroMemory(&peb, sizeof(peb));
	ZeroMemory(&peb_ldr, sizeof(peb_ldr));
	ZeroMemory(&peb_upp, sizeof(peb_upp));

	spi.dwPID = dwPID;

	HANDLE hProcess = OpenProcess(
		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ
		, FALSE
		, dwPID
	);

	if (hProcess == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	hHeap = GetProcessHeap();

	dwSize = sizeof(PROCESS_BASIC_INFORMATION_T);

	pbi = (PPROCESS_BASIC_INFORMATION_T)HeapAlloc(
		hHeap
		, HEAP_ZERO_MEMORY
		, dwSize
	);

	if (!pbi)
	{
		CloseHandle(hProcess);
		return FALSE;
	}

	NTSTATUS dwStatus = CProcess::m_pNtQueryInformationProcess(
		hProcess
		, ProcessBasicInformation
		, pbi
		, dwSize
		, &dwSizeNeeded
	);
	DWORD err = GetLastError();

	if (dwStatus >= 0 && dwSize < dwSizeNeeded)
	{
		if (pbi)
		{
			HeapFree(hHeap, 0, pbi);
		}

		pbi = (PPROCESS_BASIC_INFORMATION_T)HeapAlloc(
			hHeap
			, HEAP_ZERO_MEMORY
			, dwSizeNeeded
		);

		if (!pbi)
		{
			CloseHandle(hProcess);
			return FALSE;
		}

		dwStatus = CProcess::m_pNtQueryInformationProcess(hProcess,
			ProcessBasicInformation,
			pbi, dwSizeNeeded, &dwSizeNeeded);
	}

	if (dwStatus >= 0)
	{
		spi.dwParentPID = (DWORD)pbi->InheritedFromUniqueProcessId;
		spi.dwBasePriority = (LONG)pbi->BasePriority;
		spi.dwExitStatus = (NTSTATUS)pbi->ExitStatus;
		spi.dwPEBBaseAddress = (DWORD)pbi->PebBaseAddress;
		spi.dwAffinityMask = (DWORD)pbi->AffinityMask;

		if (pbi->PebBaseAddress)
		{
			if (ReadProcessMemory(
				hProcess
				, pbi->PebBaseAddress
				, &peb
				, sizeof(peb)
				, &dwBytesRead
			))
			{
				spi.dwSessionID = (DWORD)peb.SessionId;
				spi.cBeingDebugged = (BYTE)peb.BeingDebugged;

				dwBytesRead = 0;
				if (ReadProcessMemory(
					hProcess
					, peb.ProcessParameters
					, &peb_upp
					, sizeof(RTL_USER_PROCESS_PARAMETERS_T)
					, &dwBytesRead
				))
				{
					if (peb_upp.CommandLine.Length > 0)
					{
						pwszBuffer = (WCHAR *)HeapAlloc(hHeap,
							HEAP_ZERO_MEMORY
							, peb_upp.CommandLine.Length
						);
						if (pwszBuffer)
						{
							if (ReadProcessMemory(
								hProcess
								, peb_upp.CommandLine.Buffer
								, pwszBuffer
								, peb_upp.CommandLine.Length
								, &dwBytesRead
							))
							{
								if (peb_upp.CommandLine.Length >= sizeof(spi.szCmdLine))
								{
									dwBufferSize = sizeof(spi.szCmdLine) - sizeof(TCHAR);
								}
								else
								{
									dwBufferSize = peb_upp.CommandLine.Length;
								}
#if defined(UNICODE) || (_UNICODE)
								StringCbCopyN(spi.szCmdLine, sizeof(spi.szCmdLine),
									pwszBuffer, dwBufferSize);
#else
								WideCharToMultiByte(
									CP_ACP
									, 0
									, pwszBuffer
									, (int)(dwBufferSize / sizeof(WCHAR))
									, spi.szCmdLine
									, sizeof(spi.szCmdLine)
									, NULL
									, NULL
								);
#endif
							}
							if (!HeapFree(hHeap, 0, pwszBuffer))
							{
								bReturnStatus = FALSE;
								goto gnpiFreeMemFailed;
							}
						}
					}

					if (peb_upp.ImagePathName.Length > 0)
					{
						dwBytesRead = 0;
						pwszBuffer = (WCHAR *)HeapAlloc(hHeap,
							HEAP_ZERO_MEMORY,
							peb_upp.ImagePathName.Length);
						if (pwszBuffer)
						{
							if (ReadProcessMemory(
								hProcess
								, peb_upp.ImagePathName.Buffer
								, pwszBuffer
								, peb_upp.ImagePathName.Length
								, &dwBytesRead
							))
							{
								if (peb_upp.ImagePathName.Length >= sizeof(spi.szImgPath))
									dwBufferSize = sizeof(spi.szImgPath) - sizeof(TCHAR);
								else
									dwBufferSize = peb_upp.ImagePathName.Length;

#if defined(UNICODE) || (_UNICODE)
								StringCbCopyN(spi.szImgPath, sizeof(spi.szImgPath),
									pwszBuffer, dwBufferSize);
#else
								WideCharToMultiByte(
									CP_ACP
									, 0
									, pwszBuffer
									, (int)(dwBufferSize / sizeof(WCHAR))
									, spi.szImgPath
									, sizeof(spi.szImgPath)
									, NULL
									, NULL
								);
#endif
							}
							if (!HeapFree(hHeap, 0, pwszBuffer))
							{
								bReturnStatus = FALSE;
								goto gnpiFreeMemFailed;
							}
						}
					}
				}
			}
		}

		if (spi.dwPID == 4)
		{
			ExpandEnvironmentStrings(
				_T("%SystemRoot%\\System32\\ntoskrnl.exe")
				, spi.szImgPath
				, sizeof(spi.szImgPath)
			);
		}
	}

gnpiFreeMemFailed:

	if (pbi != NULL)
	{
		if (!HeapFree(hHeap, 0, pbi))
		{
			__noop;
		}
	}

	CloseHandle(hProcess);
	*ppi = spi;

	return bReturnStatus;
}

BOOL CProcess::KillProcessTree(DWORD myprocID, DWORD dwTimeout)
{
	BOOL bRet = TRUE;
	HWND hWnd = NULL;
	PROCESSENTRY32 pe;

	memset(&pe, 0, sizeof(PROCESSENTRY32));
	pe.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Process32First(hSnap, &pe))
	{
		BOOL bContinue = TRUE;

		// kill child processes
		while (bContinue)
		{
			if (pe.th32ParentProcessID == myprocID)
			{
				LOGI << "자식 프로세스 강제 종료: " << pe.th32ProcessID;
				KillProcessTree(pe.th32ProcessID, dwTimeout);
				HANDLE hChildProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);

				if (hChildProc)
				{
					GetWindowThreadProcessId(hWnd, &myprocID);
					PostMessage(hWnd, WM_CLOSE, 0, 0);

					if (WaitForSingleObject(hChildProc, dwTimeout) == WAIT_OBJECT_0)
					{
						bRet = TRUE;
					}
					else
					{
						bRet = TerminateProcess(hChildProc, 0);
					}
					CloseHandle(hChildProc);
				}
			}
			bContinue = Process32Next(hSnap, &pe);
		}

		HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, myprocID);

		if (hProc)
		{
			TerminateProcess(hProc, 1);
			CloseHandle(hProc);
		}
	}
	return bRet;
}

multimap<string, PROCESS_TREE_ITEM_Ptr>& CProcess::GetProcessList()
{
	if (!EnableTokenPrivilege(SE_DEBUG_NAME))
	{
		LOGW << "EnableTokenPrivilege 실패";
		return m_processMap;
	}

	for (auto it = m_processMap.begin(); it != m_processMap.end(); ++it)
	{
		auto item = (*it).second;
		item.reset();
		m_processMap.erase(it);
	}

	PROCESSINFO_T			processInfo;

	BOOL					bRet = FALSE;
	BOOL					bFound = FALSE;
	DWORD					dwPIDs[MAX_PI] = { 0 };
	DWORD					dwArraySize = MAX_PI * sizeof(DWORD);
	DWORD					dwSizeNeeded = 0;
	DWORD					dwPIDCount = 0;

	if (EnumProcesses((DWORD*)&dwPIDs, dwArraySize, &dwSizeNeeded))
	{
		HMODULE hNtDll = LoadNTDLLFunctions();

		CHECK_NOTNULL(hNtDll);

		dwPIDCount = dwSizeNeeded / sizeof(DWORD);

		LOGI << "전체 프로세스 : " << dwPIDCount;

		for (DWORD p = 0; p < MAX_PI && p < dwPIDCount; p++)
		{
			if (GetNtProcessInfo(dwPIDs[p], &processInfo))
			{
				string  imagePath = processInfo.szImgPath;
				EDRJson recentFile = CProcess::Instance()->GetRecentProcess(imagePath);
				string  imageHashMD5 = recentFile["md5"].asString();
				string  imageHashSHA256 = recentFile["sha256"].asString();
				string	commandLine = processInfo.szCmdLine;

				if (imagePath.empty() || imageHashMD5.empty() || imageHashSHA256.empty())
				{
					LOGW << "** 프로세스를 정보를 가져올 수 없습니다. ( PID: " << dwPIDs[p] << " )";
					continue;
				}

				PROCESS_TREE_ITEM_Ptr pProcessTreeItem(EDRNew PROCESS_TREE_ITEM());

				if (pProcessTreeItem == NULL)
				{
					LOGW << "메모리 할당 실패";
					continue;
				}

				LOGD << "==============";
				LOGD << "프로세스 발견 : " << imagePath;
				LOGD << "- PID         : " << processInfo.dwPID;
				LOGD << "- PPID        : " << processInfo.dwParentPID;
				LOGD << "- name        : " << imagePath;
				LOGD << "- SHA256      : " << imageHashSHA256;
				LOGD << "- MD5         : " << imageHashMD5;

				pProcessTreeItem->dwPID = processInfo.dwPID;
				EDRCopyMemory(pProcessTreeItem->szImgPath, imagePath.c_str(), imagePath.length());
				EDRCopyMemory(pProcessTreeItem->szMD5, imageHashMD5.c_str(), imageHashMD5.length());
				EDRCopyMemory(pProcessTreeItem->szSHA256, imageHashSHA256.c_str(), imageHashSHA256.length());

				m_processMap.insert(pair<string, PROCESS_TREE_ITEM_Ptr>(imageHashSHA256, pProcessTreeItem));
			}
		}

		FreeNTDLLFunctions(hNtDll);
	}

	return m_processMap;
}

BOOL CProcess::KillProcess(DWORD PID)
{
	if (!EnableTokenPrivilege(SE_DEBUG_NAME))
	{
		LOGW << "EnableTokenPrivilege 실패";
		return FALSE;
	}

	return KillProcessTree(PID, MAX_TIMEOUT);
}

BOOL CProcess::KillProcess(string  hashOrPath, BOOL bIsHash)
{
	if (!EnableTokenPrivilege(SE_DEBUG_NAME))
	{
		LOGW << "EnableTokenPrivilege 실패";
		return FALSE;
	}

	PROCESSINFO_T processInfo;
	BOOL bRet = FALSE;
	BOOL bFound = FALSE;
	DWORD dwPIDs[MAX_PI] = { 0 };
	DWORD dwArraySize = MAX_PI * sizeof(DWORD);
	DWORD dwSizeNeeded = 0;
	DWORD dwPIDCount = 0;

	if (EnumProcesses((DWORD*)&dwPIDs, dwArraySize, &dwSizeNeeded))
	{
		HMODULE hNtDll = LoadNTDLLFunctions();

		CHECK_NOTNULL(hNtDll);

		dwPIDCount = dwSizeNeeded / sizeof(DWORD);

		LOGI << "전체 프로세스 : " << dwPIDCount;

		for (DWORD p = 0; p < MAX_PI && p < dwPIDCount; p++)
		{
			ZeroMemory(&processInfo, sizeof(PROCESSINFO_T));

			if (GetNtProcessInfo(dwPIDs[p], &processInfo))
			{
				string  imagePath = processInfo.szImgPath;
				EDRJson recentFile = CProcess::Instance()->GetRecentProcess(imagePath);
				string  imageHash = recentFile["sha256"].asString();

				if (imagePath.empty() || imageHash.empty())
				{
					LOGW << "** 프로세스를 정보를 가져올 수 없습니다. ( PID: " << dwPIDs[p] << " )";
					continue;
				}

				INT isSame = (bIsHash) ? imageHash.compare(hashOrPath) : imagePath.compare(hashOrPath);

				if (isSame == 0)
				{
					LOGD << "==============";
					LOGD << "프로세스 발견 : " << imagePath;
					LOGD << "- PID         : " << processInfo.dwPID;
					LOGD << "- PPID        : " << processInfo.dwParentPID;
					LOGD << "- name        : " << imagePath;
					LOGD << "- hash        : " << imageHash;

					bFound = TRUE;
					bRet = KillProcessTree(processInfo.dwPID, MAX_TIMEOUT);
				}
				// 디버깅 용
				else
				{
					/*
					LOGD << "==============";
					LOGD << "프로세스 : " << imagePath;
					LOGD << "- PID    : " << processInfo.dwPID;
					LOGD << "- PPID   : " << processInfo.dwParentPID;
					LOGD << "- name   : " << imagePath;
					LOGD << "- hash   : " << imageHash;
					*/
				}
			}
		}
		LOGE_IF(!bFound) << "** 프로세스를 찾을 수 없습니다. ( Hash or Path: " << hashOrPath << " )";
		FreeNTDLLFunctions(hNtDll);
	}

	return bFound;
}

VOID CALLBACK CProcess::ListCallbackForCreate(
	PCHECK_LIST_ENTRY CheckList[]
	, ULONG CheckListCount
	, VOID* UserData
)
{
	LOGD << "** 전체 프로세스 개수: " << CheckListCount;

	for (auto checkListIndex = 0; checkListIndex < (INT)CheckListCount; checkListIndex++)
	{		
		PROCESS_TREE_ITEM_Ptr processInfo(EDRNew PROCESS_TREE_ITEM(CheckList[checkListIndex]->ProcessInfo));
		CProcess::Instance()->AddProcessList(CheckList[checkListIndex]->ProcessInfo->ProcessId, processInfo);
	}

	for (
		auto it = CProcess::Instance()->m_vecProcessCallback.begin();
		it != CProcess::Instance()->m_vecProcessCallback.end();
		it++
		)
	{
		PROCESS_CALLBACK_CONTEXT_Ptr pProcessCallback = (*it);

		if (pProcessCallback->pCreateCallback)
		{
			pProcessCallback->pCreateCallback(
				CheckList
				, CheckListCount
				, pProcessCallback->pUserData
			);
		}
	}
}

VOID CALLBACK CProcess::ListCallbackForDelete(
	PCHECK_LIST_ENTRY CheckList[]
	, ULONG CheckListCount
	, VOID* UserData
)
{
	for (auto i = 0; i < (INT)CheckListCount; i++)
	{
		PROCESS_TREE_ITEM_Ptr processInfo(EDRNew PROCESS_TREE_ITEM(CheckList[i]->ProcessInfo));
		CProcess::Instance()->RemoveProcessList(CheckList[i]->ProcessInfo->ProcessId, processInfo);
	}

	for (
		auto it = CProcess::Instance()->m_vecProcessCallback.begin();
		it != CProcess::Instance()->m_vecProcessCallback.end();
		it++
		)
	{
		PROCESS_CALLBACK_CONTEXT_Ptr pProcessCallback = (*it);

		if (pProcessCallback->pDeleteCallback)
		{
			pProcessCallback->pDeleteCallback(
				CheckList
				, CheckListCount
				, pProcessCallback->pUserData
			);
		}
	}

}

#include "openssl/sha.h"
#include "openssl/md5.h"

#define PROCESS_INFO_FMT	("PATH:%s_SIZE:%ld_DATE:%s")
EDRJson CProcess::GetRecentProcess(string path)
{
	EDRJson emptyJson = Json::objectValue;

	FileInfo processImage(path);

	if (!processImage.Good())
	{
		return emptyJson;
	}

	string rawKey = CUtil::Format(PROCESS_INFO_FMT, path.c_str(), processImage.Size(), processImage.Date().c_str());

	errno_t err = 0;

	// SHA256
	UCHAR	hashSHA256[SHA256_DIGEST_LENGTH * 2] = { 0x00, };
	CHAR	sha256String[SHA256_DIGEST_LENGTH * 2 + 1] = { 0x00, };

	SHA256_CTX sha256;
	SHA256_Init(&sha256);

	SHA256_Update(&sha256, (CHAR*)rawKey.c_str(), rawKey.length());
	SHA256_Final(hashSHA256, &sha256);

	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		//sprintf(sha256String + (i * 2), "%02X", hashSHA256[i]);
		sprintf_s(sha256String + (i * 2), sizeof(sha256String) - (i*2), "%02X", hashSHA256[i]);
	}

	sha256String[SHA256_DIGEST_LENGTH * 2] = 0;

	string key = sha256String;
	EDRJson recentItem = m_pHashList->FindNode(key);

	if (recentItem.empty())
	{
		EDRJson json;
		json["fileKey"] = key;
		json["sha256"] = CUtil::GetSha256(path);
		json["md5"] = CUtil::GetMD5(path);
		json["path"] = path;
		json["size"] = processImage.Size();
		json["date"] = processImage.Date();
		
		m_pHashList->InsertNode(json);

		return json;
	}
	return recentItem;
}