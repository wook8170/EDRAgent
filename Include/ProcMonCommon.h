#pragma once

#include <Windows.h>
#include "ProcMonTypes.h"
#include <vector>
#include <atlstr.h>

#define SERVICE_NAME L"NicsProcMon"

typedef
VOID
(CALLBACK *PPROCESSES_CALLBACK)(
	PCHECK_LIST_ENTRY CheckList[]
	, ULONG CheckListCount
	, VOID* UserData
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
	BOOL isDeny;
};

typedef std::vector<CheckProcessInfo> CheckedProcesses;

