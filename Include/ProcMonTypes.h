#pragma once

typedef struct _PROCESS_INFO
{
	ULONG ProcessId;
	ULONG ParentProcessId;
	WCHAR ImagePath[260];
} PROCESS_INFO, *PPROCESS_INFO;

typedef shared_ptr<PROCESS_INFO> PROCESS_INFO_Ptr;

typedef struct _CHECK_LIST_ENTRY
{
	PPROCESS_INFO ProcessInfo;
	BOOLEAN AddToBlacklist;
} CHECK_LIST_ENTRY, *PCHECK_LIST_ENTRY;

typedef shared_ptr<CHECK_LIST_ENTRY> CHECK_LIST_ENTRY_Ptr;
