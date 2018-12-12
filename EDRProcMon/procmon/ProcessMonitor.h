#ifndef __PROCESS_MONITOR_H__
#define __PROCESS_MONITOR_H__

#include "ProcessHelper.h"


#include "utils.h"
#include "new.h"
#include "sync.h"

#include "types.h"

class ProcessMonitor
{
public:
	ProcessMonitor();

	NTSTATUS ProcessIrp(PIRP Irp);

	VOID OnCreateProcess(const HANDLE ParentPid, const HANDLE Pid);
	VOID OnLoadProcessImage(const HANDLE Pid, 
		const PUNICODE_STRING NativeImageName,
		const PVOID ImageBase, const SIZE_T ImageSize, PBOOLEAN TerminateProcess);
	VOID OnDeleteProcess(const HANDLE ParentPid, const HANDLE Pid);

	VOID Cleanup();
	VOID Reset(int type);
	VOID AddStartupProcess(HANDLE Pid, HANDLE ParentPID, const PUNICODE_STRING NativeImageName);
	VOID RemoveStartupProcess(HANDLE Pid);
	VOID NotifyStartupProcess();

private:

	ProcessHelperHolder m_InitProcesses;;
	ProcessHelperHolder m_ActiveProcesses;
	ProcessHelperHolder m_DeleteProcesses;

	ULONG QueryProcessesCount(int type);
	ULONG QueryInitProcessesCount();
	void CopyProcInfo(IN ProcessHelper* ProcHelper, IN OUT PPROCESS_INFO ProcInfo);
	void QueryProcesses(PROCESS_INFO ProcInfo[], ULONG Capacity, ULONG& ActualCount, int type);
	void QueryInitProcesses(PROCESS_INFO ProcInfo[], ULONG Capacity, ULONG& ActualCount);
	void TerminateBlackMarked();	
	void DeleteCompleted();	
	NTSTATUS AppendRule(PPROCESS_INFO pInfo, bool BlacklistRule);
	
	sync::SharedLock m_Lock;
	sync::SharedLock m_LockForDelete;

	sync::Event m_UserNotifyEvent;
	sync::Event m_UserNotifyEventDelete;
	sync::Event m_UserNotifyEventInitList;

	BOOLEAN m_Shutdown;
};

#endif //__PROCESS_MONITOR_H__
