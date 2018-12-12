#ifndef __COMMON_H__
#define __COMMON_H__

#include <Windows.h>
#include "types.h"
#include <vector>
#include <atlstr.h>
#ifdef _WIN64 
#define SERVICE_NAME L"NicProcMon_64"
#else
#define SERVICE_NAME L"NicProcMon_32"
#endif

typedef
	VOID
	(CALLBACK *PPROCESSES_CALLBACK)(
	PCHECK_LIST_ENTRY CheckList[],
	ULONG CheckListCount,
	VOID* UserData
	);

struct ProcessInfo
{
	unsigned int pid;
	CString imageName;
};

typedef std::vector<ProcessInfo> RunningProcesses;

struct CheckProcessInfo
{
	ProcessInfo process;
	bool isDeny;
};

typedef std::vector<CheckProcessInfo> CheckedProcesses;

#endif // __COMMON_H__