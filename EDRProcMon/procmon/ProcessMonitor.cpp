#include "ProcessMonitor.h"
#include "utils.h"
#include "controls.h"

ProcessMonitor::ProcessMonitor() :  m_Shutdown(false)
{
}


VOID ProcessMonitor::OnCreateProcess(const HANDLE ParentPid, const HANDLE Pid)
{
	ProcessHelper* procHelper = new ProcessHelper(ParentPid, Pid);
	if (procHelper)
	{
		if (!m_ActiveProcesses.Add(procHelper))
			return;
	}
	else
	{
		ERR_FN(("Failed to allocate memory for ProcessHelper object\n"));
	}
}

VOID ProcessMonitor::OnLoadProcessImage(const HANDLE Pid, 
										const PUNICODE_STRING NativeImageName,
										const PVOID ImageBase,
										const SIZE_T ImageSize,
										PBOOLEAN TerminateProcess)
{
	// Don't do anyting during shutdown stage.
	{
		sync::AutoReadLock sharedLock(m_Lock);
		if (m_Shutdown)
			return;
	}

	ProcessHelper* procHelper = NULL;
	// Explicit scope to access and release list
	{
		ProcessHelperList procList = m_ActiveProcesses.AquireList();
		utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_ActiveProcesses);

		ProcessHelperList::iterator it = procList.find(Pid);
		if (it != procList.end())
			procHelper = it->second;

		if (!procHelper)
			return;
	}

	// Check if process already have ImageName assigned. If assigned, then, 
	// current callback for dll image which is mapped in process address space.
	// Ignore it in this case.
	if (!procHelper->ImageName.empty())
		return;

	// Convert native image path to DOS path. Lowercase final path to optimize 
	// path checking speed on rules processing.
	PUNICODE_STRING dosName = NULL;
	utils::KernelFileNameToDosName(NativeImageName, &dosName);

	VERBOSE_FN(("OnLoadProcessImage NativeImageName: %wZ\n", NativeImageName));
	VERBOSE_FN(("OnLoadProcessImage dosName: %wZ\n", dosName));

	if (dosName)
	{
		RtlDowncaseUnicodeString(dosName, dosName, FALSE);
		utils::nothrow_string_assign<std::wstring, WCHAR>(&procHelper->ImageName, dosName->Buffer, dosName->Length / sizeof(WCHAR));
		ExFreePool(dosName);
	}
	else
	{
		utils::nothrow_string_assign<std::wstring, WCHAR>(&procHelper->ImageName, NativeImageName->Buffer, NativeImageName->Length / sizeof(WCHAR));
	}

	if (procHelper->ImageName.size() == 0)
	{
		ERR_FN(("Failed to allocate memory for ImageName. Clean process record."));
		m_ActiveProcesses.Delete(Pid);
		return;
	}

	AddStartupProcess(Pid, procHelper->GetParentPid(), NativeImageName);
	// Artificail block to sync. access for mRulesLoaded & mInitialization flags.
	{
		sync::AutoReadLock sharedLock(m_Lock);

		if (!m_UserNotifyEvent.Valid())
		{
			ERR_FN(("Failded to !m_UserNotifyEvent.Valid() ProcessId: %d\n", procHelper->Pid));
			return;
		}

		if (!procHelper->ResumeEvent.Initialize(false, false))
		{
			ERR_FN(("Failded to initialize resume event for ProcessId: %d\n", procHelper->Pid));
			return;
		}

		// Notify user mode library counterpart (if core in Normal state)
		m_UserNotifyEvent.Set();
	}    

	NTSTATUS waitStatus = STATUS_SUCCESS;
	unsigned waitTimeout = 1*10*1000;

	VERBOSE_FN(("OnLoadProcessImage Wait NativeImageName: %wZ\n", NativeImageName));
	///*
	if (procHelper->ResumeEvent.Wait(waitTimeout, &waitStatus) && waitStatus == STATUS_TIMEOUT)
	{
		VERBOSE_FN(("OnLoadProcessImage Resume NativeImageName: %wZ\n", NativeImageName));
		Reset(EProcessCreate);
	}
	//*/
	//procHelper->ResumeEvent.Wait();

	VERBOSE_FN(("OnLoadProcessImage Disallowed NativeImageName: %wZ\n", NativeImageName));
	*TerminateProcess = procHelper->Marker == Disallowed;
}

VOID ProcessMonitor::OnDeleteProcess(const HANDLE ParentPid, const HANDLE Pid)
{
	{
		sync::AutoReadLock sharedLock(m_LockForDelete);
		if (m_Shutdown)
			return;
	}

	ProcessHelper* procHelper = NULL;
	
	// Explicit scope to access and release list
	{
		ProcessHelperList procList = m_ActiveProcesses.AquireList();
		utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_ActiveProcesses);

		ProcessHelperList::iterator it = procList.find(Pid);
		if (it != procList.end())
			procHelper = it->second;		
	}

	ProcessHelper* procHelperForDelete = new ProcessHelper(ParentPid, Pid);

	if( procHelperForDelete != NULL && procHelper!= NULL && procHelper->ImageName.size() != 0 )
	{
		utils::nothrow_string_assign<std::wstring, WCHAR>
			(&procHelperForDelete->ImageName
			, (WCHAR*)procHelper->ImageName.c_str(),
			procHelper->ImageName.size()
			);
		
	}

	if (procHelperForDelete)
	{
		if (!m_DeleteProcesses.Add(procHelperForDelete))
		{		
			ERR_FN(("OnDeleteProcess Fail m_DeleteProcesses.Add()\n"));
			return;
		}
	}
	else
	{
		ERR_FN(("Failed to allocate memory for ProcessHelper object\n"));
	}

	// Artificail block to sync. access for mRulesLoaded & mInitialization flags.
	{
		sync::AutoReadLock sharedLock(m_LockForDelete);

		if (!m_UserNotifyEventDelete.Valid())
		{
			ERR_FN(("OnDeleteProcess Fail m_UserNotifyEventDelete.Valid()\n"));
			return;
		}

		if (!procHelperForDelete->ResumeEventForDelete.Initialize(false, false))
		{
			ERR_FN(("Failded to initialize resume event for ParentPid: %d, Pid: %d, %ws\n", ParentPid, Pid, procHelperForDelete->ImageName.c_str()));
			return;
		}
		// Notify user mode library counterpart (if core in Normal state)
		m_UserNotifyEventDelete.Set();
	}    

	NTSTATUS waitStatus = STATUS_SUCCESS;
	unsigned waitTimeout = 1*10*1000;

	VERBOSE_FN(("OnDeleteProcess Wait NativeImageName: ParentPid: %d, Pid: %d, %ws\n", ParentPid, Pid, procHelperForDelete->ImageName.c_str()));

	if (procHelperForDelete->ResumeEventForDelete.Wait(waitTimeout, &waitStatus) && waitStatus == STATUS_TIMEOUT)
	{
		VERBOSE_FN(("OnDeleteProcess Resume NativeImageName: ParentPid: %d, Pid: %d, %ws\n", ParentPid, Pid, procHelperForDelete->ImageName.c_str()));
		Reset(EProcessDelete);
	}
	
	VERBOSE_FN(("OnDeleteProcess Disallowed NativeImageName: ParentPid: %d, Pid: %d, %ws\n", ParentPid, Pid, procHelperForDelete->ImageName.c_str()));

	m_DeleteProcesses.Delete(Pid);
	m_ActiveProcesses.Delete(Pid);
	RemoveStartupProcess(Pid);
}

ULONG ProcessMonitor::QueryProcessesCount(int type)
{
	if(type==EProcessCreate)
	{
		VERBOSE_FN(("QueryProcessesCount: EProcessCreate"));
		ProcessHelperList procList = m_ActiveProcesses.AquireList();
		utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_ActiveProcesses);

		ULONG count = 0;
		ProcessHelperList::iterator it;
		for (it = procList.begin(); it != procList.end(); it++)
			if (it->second->Marker == ProcessMarker::Clean)
				count++;

		return count;
	}
	else if(type==EProcessDelete)
	{
		VERBOSE_FN(("QueryProcessesCount: EProcessDelete"));
		ProcessHelperList procList = m_DeleteProcesses.AquireList();
		utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_DeleteProcesses);

		ULONG count = 0;
		ProcessHelperList::iterator it;
		for (it = procList.begin(); it != procList.end(); it++)			
			count++;

		return count;

	}
	else 
	{
		VERBOSE_FN(("QueryProcessesCount: 0"));
		return 0;
	}
}

ULONG ProcessMonitor::QueryInitProcessesCount()
{
	ProcessHelperList procList = m_InitProcesses.AquireList();
	utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_InitProcesses);

	ULONG count = 0;
	ProcessHelperList::iterator it;
	for (it = procList.begin(); it != procList.end(); it++)
		count++;

	VERBOSE_FN(("QueryInitProcessesCount: %d", count));

	return count;
}

void ProcessMonitor::CopyProcInfo(IN ProcessHelper* ProcHelper, IN OUT PPROCESS_INFO ProcInfo)
{
	ProcInfo->ProcessId = (ULONG)ProcHelper->GetPid();
	ProcInfo->ParentProcessId = (ULONG)ProcHelper->GetParentPid();

	VERBOSE_FN(("CopyProcInfo: %ws", ProcHelper->ImageName.c_str()));

	memcpy(ProcInfo->ImagePath, ProcHelper->ImageName.c_str(), 
		(ProcHelper->ImageName.size() + 1) * sizeof(wchar_t));
}

void ProcessMonitor::QueryProcesses(PROCESS_INFO ProcInfo[], ULONG Capacity, ULONG& ActualCount, int type )
{
	if(type==EProcessCreate)
	{
		ProcessHelperList procList = m_ActiveProcesses.AquireList();
		utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_ActiveProcesses);

		ActualCount = 0;
		ProcessHelperList::iterator it;
		for (it = procList.begin(); it != procList.end(); it++)
		{
			ProcessHelper* procHelper = it->second;

			if (procHelper->Marker == ProcessMarker::Clean)
			{
				CopyProcInfo(procHelper, &ProcInfo[ActualCount]);

				ActualCount++;
				if (ActualCount == Capacity)
					break;
			}
		}
	}
	else if(type==EProcessDelete)
	{
		ProcessHelperList procList = m_DeleteProcesses.AquireList();
		utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_DeleteProcesses);

		ActualCount = 0;
		ProcessHelperList::iterator it;
		for (it = procList.begin(); it != procList.end(); it++)
		{
			ProcessHelper* procHelper = it->second;

			CopyProcInfo(procHelper, &ProcInfo[ActualCount]);

			ActualCount++;

			if (ActualCount == Capacity)
				break;

		}
	}
}

void ProcessMonitor::QueryInitProcesses(PROCESS_INFO ProcInfo[], ULONG Capacity, ULONG& ActualCount)
{

	ProcessHelperList procList = m_InitProcesses.AquireList();
	utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_InitProcesses);

	ActualCount = 0;
	ProcessHelperList::iterator it;
	for (it = procList.begin(); it != procList.end(); it++)
	{
		ProcessHelper* procHelper = it->second;

		CopyProcInfo(procHelper, &ProcInfo[ActualCount]);

		ActualCount++;
		if (ActualCount == Capacity)
			break;
	}
}

void ProcessMonitor::TerminateBlackMarked()
{
	// Terminate all black processes.
	ProcessHelperList procList = m_ActiveProcesses.AquireList();
	utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_ActiveProcesses);
	ProcessHelperList::iterator it;
	for (it = procList.begin(); it != procList.end(); it++)
	{
		ProcessHelper* procHelper = it->second;
		if (procHelper->Marker == ProcessMarker::Disallowed)
		{
			if (procHelper->IsSuspended())
				procHelper->ResumeEvent.Set();
			else
				utils::ScheduleProcessTerminate(procHelper->Pid);
		}
	}
}

void ProcessMonitor::DeleteCompleted()
{
	ProcessHelperList procList = m_DeleteProcesses.AquireList();
	utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_DeleteProcesses);
	ProcessHelperList::iterator it;

	VERBOSE_FN(("DeleteCompleted"));
	for (it = procList.begin(); it != procList.end(); it++)
	{
		ProcessHelper* procHelper = it->second;

		if (procHelper->IsSuspendedForDelete())
		{			
			VERBOSE_FN(("DeleteCompleted: ResumeEventForDelete.Set()"));
			procHelper->ResumeEventForDelete.Set();			
		}
	}
}

NTSTATUS ProcessMonitor::AppendRule(PPROCESS_INFO pInfo, bool BlacklistRule)
{
	sync::AutoReadLock sharedLock(m_Lock);

	// Find process which match rule.
	ProcessHelperList procList = m_ActiveProcesses.AquireList();
	utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_ActiveProcesses);
	ProcessHelperList::iterator it;
	for (it = procList.begin(); it != procList.end(); it++)
	{
		ProcessHelper* procHelper = it->second;
		if (procHelper->GetPid() == (HANDLE)(pInfo->ProcessId))
		{
			procHelper->Marker = BlacklistRule ? ProcessMarker::Disallowed : ProcessMarker::Allowed;
			// Any rule added after Initialization interpreted as user controlled action
			if (procHelper->IsSuspended())
			{
				// Resume suspended process 
				// (action will be applied to it depending of marker)
				procHelper->ResumeEvent.Set();
			}
		}
	}

	return STATUS_SUCCESS;
}

NTSTATUS ProcessMonitor::ProcessIrp(PIRP Irp)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	ULONG_PTR BytesWritten = 0;

	PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG ControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

	ULONG method = ControlCode & 0x03;
	if (method != METHOD_BUFFERED)
		return utils::CompleteIrp(Irp, STATUS_INVALID_PARAMETER, BytesWritten);

	ULONG InputBufferSize = irpStack->Parameters.DeviceIoControl.InputBufferLength;

	ULONG OutputBufferSize = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

	PUCHAR Buffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;

	switch (ControlCode) 
	{
	case IOCTL_GET_PROCESS_COUNT:
		{
			int type = 0;
			VERBOSE_FN(("ProcessIrp: IOCTL_GET_PROCESS_COUNT"));
			if (InputBufferSize == sizeof(int))
			{
				type = *((int*)Buffer);
			}
			else
			{
				ERR_FN(("ProcessIrp: IOCTL_GET_PROCESS_COUNT STATUS_INVALID_PARAMETER"));
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			VERBOSE_FN(("ProcessIrp: IOCTL_GET_PROCESS_COUNT Type: %d", type));

			if (OutputBufferSize >= sizeof(ULONG))
			{
				*(PULONG)Buffer = QueryProcessesCount(type);
				BytesWritten = sizeof(ULONG);
				status = STATUS_SUCCESS;
			}
			else
				status = STATUS_INVALID_PARAMETER;
			break;
		}
	case IOCTL_RESET:
		{
			VERBOSE_FN(("ProcessIrp: IOCTL_RESET"));
			Reset(EProcessBoth);
			status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_GET_PROCESS_LIST:
		{
			int type = 0;
			VERBOSE_FN(("ProcessIrp: IOCTL_GET_PROCESS_LIST"));
			if (InputBufferSize == sizeof(int))
			{
				type = *((int*)Buffer);
			}
			else
			{
				ERR_FN(("ProcessIrp: IOCTL_GET_PROCESS_LIST STATUS_INVALID_PARAMETER"));
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			VERBOSE_FN(("ProcessIrp: IOCTL_GET_PROCESS_LIST Type: %d", type));

			if (OutputBufferSize >= sizeof(PROCESS_INFO))
			{
				ULONG entriesProcessed = 0;
				QueryProcesses((PPROCESS_INFO)Buffer, OutputBufferSize/sizeof(PROCESS_INFO), entriesProcessed, type);
				BytesWritten = entriesProcessed * sizeof(PROCESS_INFO);
				status = STATUS_SUCCESS;
			}
			else
				status = STATUS_INVALID_PARAMETER;

			
			if (m_UserNotifyEventInitList.Valid())
			{
				VERBOSE_FN(("IOCTL_GET_PROCESS_LIST m_UserNotifyEventInitList.Set()"));
				m_UserNotifyEventInitList.Set();
			}	
			
			break;
		}
	case IOCTL_TERMINATE_ALL:
		{
			VERBOSE_FN(("ProcessIrp: IOCTL_TERMINATE_ALL"));
			TerminateBlackMarked();
			status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_ALLOW:
		{	
			VERBOSE_FN(("ProcessIrp: IOCTL_ALLOW"));
			if (InputBufferSize >= sizeof(PPROCESS_INFO))
				status = AppendRule((PPROCESS_INFO)Buffer, false);
			else
				status = STATUS_INVALID_PARAMETER;
			break;
		}
	case IOCTL_BLOCK:
		{
			VERBOSE_FN(("ProcessIrp: IOCTL_BLOCK"));
			if (InputBufferSize >= sizeof(PPROCESS_INFO))
				status = AppendRule((PPROCESS_INFO)Buffer, true);
			else
				status = STATUS_INVALID_PARAMETER;
			break;
		}
	case IOCTL_REGISTER_EVENT:
		{
			VERBOSE_FN(("ProcessIrp: IOCTL_REGISTER_EVENT"));
			HANDLE  hEvent = NULL;
#if defined(_WIN64)
			if (IoIs32bitProcess(Irp))
			{
				if (InputBufferSize == sizeof(UINT32))
				{
					UINT32  Handle32;
					Handle32 = *((PUINT32)Buffer);
					hEvent = LongToHandle(Handle32);
				}
				else
				{
					status = STATUS_INVALID_PARAMETER;
					break;
				}
			}
			else                        
#endif
			{
				if (InputBufferSize == sizeof(HANDLE))
				{
					hEvent = *((PHANDLE)Buffer);
				}
				else
				{
					status = STATUS_INVALID_PARAMETER;
					break;
				}
			}

			sync::AutoWriteLock autoWriteLock(m_Lock);
			m_UserNotifyEvent.Cleanup();
			status = m_UserNotifyEvent.Initialize(hEvent) ? STATUS_SUCCESS : STATUS_INVALID_HANDLE;

			// Set control process flag
			if (m_UserNotifyEvent.Valid())
			{
				ProcessHelperList procList = m_ActiveProcesses.AquireList();
				utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_ActiveProcesses);

				ProcessHelperList::const_iterator proc = procList.find(PsGetCurrentProcessId());
				ASSERT(proc != procList.end());

				// Look for suspended processes. If just one find. Signal event.
				ProcessHelperList::iterator it2;
				for (it2 = procList.begin(); it2 != procList.end(); it2++)
				{
					ProcessHelper* procHelper = it2->second;
					if (procHelper->IsSuspended())
					{
						m_UserNotifyEvent.Set();
						break;
					}
				}
			}

			break;
		}
	case IOCTL_REGISTER_EVENT_FOR_DELETE:
		{
			VERBOSE_FN(("ProcessIrp: IOCTL_REGISTER_EVENT_FOR_DELETE"));
			HANDLE	hEvent = NULL;
#if defined(_WIN64)
			if (IoIs32bitProcess(Irp))
			{
				if (InputBufferSize == sizeof(UINT32))
				{
					UINT32	Handle32;
					Handle32 = *((PUINT32)Buffer);
					hEvent = LongToHandle(Handle32);
				}
				else
				{
					status = STATUS_INVALID_PARAMETER;
					break;
				}
			}
			else						
#endif
			{
				if (InputBufferSize == sizeof(HANDLE))
				{
					hEvent = *((PHANDLE)Buffer);
				}
				else
				{
					status = STATUS_INVALID_PARAMETER;
					break;
				}
			}

			sync::AutoWriteLock autoWriteLock(m_LockForDelete);
			m_UserNotifyEventDelete.Cleanup();
			status = m_UserNotifyEventDelete.Initialize(hEvent) ? STATUS_SUCCESS : STATUS_INVALID_HANDLE;

			// Set control process flag
			if (m_UserNotifyEventDelete.Valid())
			{
				ProcessHelperList procList = m_DeleteProcesses.AquireList();
				utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_DeleteProcesses);

				ProcessHelperList::const_iterator proc = procList.find(PsGetCurrentProcessId());
				ASSERT(proc != procList.end());

				// Look for suspended processes. If just one find. Signal event.
				ProcessHelperList::iterator it2;
				for (it2 = procList.begin(); it2 != procList.end(); it2++)
				{
					ProcessHelper* procHelper = it2->second;
					if (procHelper->IsSuspendedForDelete())
					{
						m_UserNotifyEventDelete.Set();
						break;
					}
				}
			}

			break;
		}
	case IOCTL_DELETE_COMPLETED:
		{
			VERBOSE_FN(("ProcessIrp: IOCTL_DELETE_COMPLETED"));
			DeleteCompleted();
			status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_GET_INIT_COUNT:
		{
			if (OutputBufferSize >= sizeof(ULONG))
			{
				*(PULONG)Buffer = QueryInitProcessesCount();
				BytesWritten = sizeof(ULONG);
				status = STATUS_SUCCESS;
			}
			else
				status = STATUS_INVALID_PARAMETER;
			break;
		}
	
	case IOCTL_GET_INIT_PROCESS:
		{			
			VERBOSE_FN(("ProcessIrp: IOCTL_GET_INIT_PROCESS"));
			
			if (OutputBufferSize >= sizeof(PROCESS_INFO))
			{
				ULONG entriesProcessed = 0;
				QueryInitProcesses((PPROCESS_INFO)Buffer, OutputBufferSize/sizeof(PROCESS_INFO), entriesProcessed);
				BytesWritten = entriesProcessed * sizeof(PROCESS_INFO);
				status = STATUS_SUCCESS;
			}
			else
				status = STATUS_INVALID_PARAMETER;
			
			break;
		}
	default:
		status = STATUS_NOT_IMPLEMENTED;
	}

	return utils::CompleteIrp(Irp, status, BytesWritten);
}

VOID ProcessMonitor::Cleanup()
{
	{
		sync::AutoReadLock exclusiveLock(m_Lock);
		sync::AutoReadLock exclusiveLockForDelete(m_LockForDelete);
		m_Shutdown = TRUE;
	}

	Reset(EProcessBoth);

	m_InitProcesses.Clean();
	m_ActiveProcesses.Clean();	
	m_DeleteProcesses.Clean();
}

VOID ProcessMonitor::Reset(int op)
{
	TERSE_FN(("Reset\n"));

	{
		sync::AutoWriteLock autoWriteLock(m_Lock);
		
		if(op & EProcessCreate)
		{
			if (m_UserNotifyEvent.Valid())
			{
				m_UserNotifyEvent.Set();
				m_UserNotifyEvent.Cleanup();
			}
		}
		

		ProcessHelperList procList = m_ActiveProcesses.AquireList();
		utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_ActiveProcesses);
		ProcessHelperList::iterator it;
		for (it = procList.begin(); it != procList.end(); it++)
		{        
			ProcessHelper* procHelper = it->second;
			if (procHelper->IsSuspended())
			{
				procHelper->ResumeEvent.Set();
			}
		}
	}

	{
		sync::AutoWriteLock autoWriteLockForDelete(m_LockForDelete);

		if(op & EProcessDelete)
		{
			if (m_UserNotifyEventDelete.Valid())
			{
				m_UserNotifyEventDelete.Set();
				m_UserNotifyEventDelete.Cleanup();
			}
		}

		ProcessHelperList procListForDelete = m_DeleteProcesses.AquireList();
		utils::ScopedListReleaser<ProcessHelperHolder> procListGaurdForDelete(m_DeleteProcesses);
		ProcessHelperList::iterator itForDelete;
		for (itForDelete = procListForDelete.begin(); itForDelete != procListForDelete.end(); itForDelete++)
		{        
			ProcessHelper* procHelperForDelete = itForDelete->second;
			if (procHelperForDelete->IsSuspendedForDelete())
			{
				procHelperForDelete->ResumeEventForDelete.Set();
			}
		}
	}

	if (m_UserNotifyEventInitList.Valid())
	{
		m_UserNotifyEventInitList.Set();
		m_UserNotifyEventInitList.Cleanup();
	}
}

VOID ProcessMonitor::AddStartupProcess(HANDLE Pid, HANDLE ParentPID, const PUNICODE_STRING NativeImageName)
{
	
	{
		sync::AutoReadLock sharedLock(m_Lock);
		if (m_Shutdown)
			return;
	}

	ProcessHelper* newProcHelper = new ProcessHelper(ParentPID, Pid);

	if (newProcHelper)
	{
		if (!m_InitProcesses.Add(newProcHelper))
			return;
	}
	else
	{
		ERR_FN(("Failed to allocate memory for ProcessHelper object\n"));
	}


	ProcessHelper* procHelper = NULL;
	// Explicit scope to access and release list
	{
		ProcessHelperList procList = m_InitProcesses.AquireList();
		utils::ScopedListReleaser<ProcessHelperHolder> procListGaurd(m_InitProcesses);

		ProcessHelperList::iterator it = procList.find(Pid);
		if (it != procList.end())
			procHelper = it->second;

		if (!procHelper)
			return;
	}

	// Check if process already have ImageName assigned. If assigned, then, 
	// current callback for dll image which is mapped in process address space.
	// Ignore it in this case.
	if (!procHelper->ImageName.empty())
		return;

	// Convert native image path to DOS path. Lowercase final path to optimize 
	// path checking speed on rules processing.
	PUNICODE_STRING dosName = NULL;
	utils::KernelFileNameToDosName(NativeImageName, &dosName);

	VERBOSE_FN(("AddStartupProcess NativeImageName: %d, %d, %wZ\n", Pid, ParentPID, NativeImageName));
	VERBOSE_FN(("AddStartupProcess dosName: %wZ\n", dosName));

	if (dosName)
	{
		RtlDowncaseUnicodeString(dosName, dosName, FALSE);
		utils::nothrow_string_assign<std::wstring, WCHAR>(&procHelper->ImageName, dosName->Buffer, dosName->Length / sizeof(WCHAR));
		ExFreePool(dosName);
	}
	else
	{
		utils::nothrow_string_assign<std::wstring, WCHAR>(&procHelper->ImageName, NativeImageName->Buffer, NativeImageName->Length / sizeof(WCHAR));
	}

	if (procHelper->ImageName.size() == 0)
	{
		ERR_FN(("AddStartupProcess Failed to allocate memory for ImageName. Clean process record."));
		m_InitProcesses.Delete(Pid);
		return;
	}

}

VOID ProcessMonitor::RemoveStartupProcess(HANDLE Pid)
{
	{
		sync::AutoReadLock sharedLock(m_Lock);
		if (m_Shutdown)
			return;
		
		VERBOSE_FN(("RemoveStartupProcess NativeImageName: %d\n", Pid));
		m_InitProcesses.Delete(Pid);
	}
}

VOID ProcessMonitor::NotifyStartupProcess()
{
	VERBOSE_FN(("NotifyStartupProcess"));

	{
		sync::AutoReadLock sharedLock(m_Lock);

		if (!m_UserNotifyEvent.Valid())
		{
			ERR_FN(("NotifyStartupProcess Failded to !m_UserNotifyEvent.Valid() "));
			return;
		}

		m_UserNotifyEvent.Set();

	}  
}

