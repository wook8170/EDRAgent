#include "stdafx.h"
#include "Pipe.h"

CPipe::CPipe(VOID)
{
	// Init

	m_LastError = PIPE_ERROR_SUCCESS;
	m_GUID = "{E91885F1-84E4-11d5-898D-0008C725AC74}";		// { 0xe91885f1, 0x84e4, 0x11d5, { 0x89, 0x8d, 0x0, 0x8, 0xc7, 0x25, 0xac, 0x74 } }
	m_SubKey = StringNull;

	m_bCreateCalled = FALSE;
	m_bCloseCalled = FALSE;
	m_bPipeRegistered = FALSE;

	m_hReadPipe = NULL;
	m_hWritePipe = NULL;
	m_hClientReadPipe = NULL;
	m_hClientWritePipe = NULL;
	m_hMutex = NULL;

	m_bVaildThread = FALSE;
	m_bRunThread = TRUE;
}

CPipe::~CPipe(VOID)
{
	Close();
}

BOOL CPipe::Create(
	std::string PipeName
	, DWORD dwDesiredAccess
	, BOOL bServerSide
	, PIPE_CALLBACK CallBackFunc
	, LPVOID pCallBackParam
)
{
	BOOL					bCreationSucceeded;
	SECURITY_ATTRIBUTES		SecAttrib;

	if (m_bCreateCalled)
	{
		SetLastError(PIPE_CREATE_ALREADY_CALLED);

		return FALSE;
	}

	m_PipeName = PipeName;
	m_dwDesiredAccess = dwDesiredAccess;
	m_bServerSide = bServerSide;
	m_CallBackFunc = CallBackFunc;
	m_pCallBackParam = pCallBackParam;
	m_bCreateCalled = TRUE;

	if (m_bServerSide)
	{
		if (RegisterPipe())
		{
			bCreationSucceeded = FALSE;

			SecAttrib.nLength = sizeof(SECURITY_ATTRIBUTES);
			SecAttrib.lpSecurityDescriptor = NULL;
			SecAttrib.bInheritHandle = TRUE;

			if (dwDesiredAccess & PIPE_READ)
			{
				bCreationSucceeded = (
					CreatePipe(
						&m_hClientReadPipe
						, &m_hWritePipe
						, &SecAttrib
						, 0
					)
					) ? TRUE : FALSE;
			}

			if (dwDesiredAccess & PIPE_WRITE)
			{
				bCreationSucceeded = (
					CreatePipe(
						&m_hReadPipe
						, &m_hClientWritePipe
						, &SecAttrib
						, 0)
					) ? TRUE : FALSE;
			}

			if (bCreationSucceeded && UpdateRegistration() && CallBack())
			{
				return TRUE;
			}
			else
			{
				SetLastError(PIPE_ERROR_CREATION_FAILED);
				UnRegisterPipe();
			}
		}
	}
	else
	{
		if (RetrievePipe() && CallBack())
		{
			return TRUE;
		}
	}

	return FALSE;
}

VOID CPipe::Close()
{
	if (m_bCloseCalled) return;

	m_bCloseCalled = TRUE;
	m_bRunThread = FALSE;

	UnRegisterPipe();

	CloseHandle(m_hClientWritePipe);
	CloseHandle(m_hClientReadPipe);
	CloseHandle(m_hWritePipe);
	CloseHandle(m_hReadPipe);
	CloseHandle(m_hMutex);

	if (m_bVaildThread) *m_pbVaildPtr = FALSE;
}


BOOL CPipe::ReadPipe(
	LPVOID Buffer
	, DWORD nNumberOfBytesToRead
	, DWORD* lpNumberOfBytesRead
)
{
	if (ReadFile(m_hReadPipe, Buffer, nNumberOfBytesToRead, lpNumberOfBytesRead, NULL))
	{
		return TRUE;
	}

	SetLastError(PIPE_ERROR_READ_FAILED);

	return FALSE;
}

BOOL CPipe::WritePipe(
	LPVOID Buffer
	, DWORD nNumberOfBytesToWrite
)
{
	DWORD	nNumberOfBytesWritten;

	if (WriteFile(m_hWritePipe, Buffer, nNumberOfBytesToWrite, &nNumberOfBytesWritten, NULL))
	{
		if (nNumberOfBytesWritten == nNumberOfBytesToWrite)
		{
			return TRUE;
		}
	}

	SetLastError(PIPE_ERROR_WRITE_FAILED);

	return FALSE;
}

int CPipe::GetLastError()
{
	return m_LastError;
}

BOOL CPipe::RegisterPipe()
{
	HKEY		hRegKey;
	DWORD		dwDisposition;
	DWORD		dwValuePId;
	DWORD		dwValueDef;
	DWORD		dwDataLen;
	HANDLE		hProcess;
	HANDLE		hMutex;
	string		MutexName;

	MutexName = "Global\\";
	MutexName.append(m_GUID).append(StringSlash).append(m_PipeName);
	m_SubKey = "SOFTWARE\\";
	m_SubKey.append(m_GUID).append(PathDelimeter).append(m_PipeName);

	if (RegCreateKeyExA(
		HKEY_LOCAL_MACHINE
		, m_SubKey.c_str()
		, 0
		, StringNull
		, 0
		, KEY_ALL_ACCESS | KEY_WOW64_64KEY
		, NULL
		, &hRegKey
		, &dwDisposition) == ERROR_SUCCESS
		)
	{
		if (dwDisposition == REG_OPENED_EXISTING_KEY)
		{
			dwDataLen = sizeof(DWORD);

			if (RegQueryValueExA(
				hRegKey
				, "PID"
				, 0
				, NULL
				, (BYTE*)&dwValuePId
				, &dwDataLen) == ERROR_SUCCESS
				)
			{
				if ((hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwValuePId)) != NULL)
				{
					CloseHandle(hProcess);

					if ((hMutex = OpenMutexA(
						MUTEX_ALL_ACCESS
						, FALSE
						, MutexName.c_str())) != NULL
						)
					{
						CloseHandle(hMutex);

						SetLastError(PIPE_ERROR_NAME_ALREADY_EXISTS);
						RegCloseKey(hRegKey);

						return FALSE;
					}
				}
			}
		}

		if ((m_hMutex = CreateMutexA(NULL, TRUE, MutexName.c_str())) != NULL)
		{
			dwValuePId = GetCurrentProcessId();
			dwValueDef = (DWORD)INVALID_HANDLE_VALUE;

			if ((RegSetValueExA(hRegKey, "PID", 0, REG_DWORD, (BYTE*)&dwValuePId, sizeof(DWORD)) &
				RegSetValueExA(hRegKey, "HRP", 0, REG_DWORD, (BYTE*)&dwValueDef, sizeof(DWORD)) &
				RegSetValueExA(hRegKey, "HWP", 0, REG_DWORD, (BYTE*)&dwValueDef, sizeof(DWORD))) == ERROR_SUCCESS)
			{
				RegCloseKey(hRegKey);
				m_bPipeRegistered = TRUE;

				return TRUE;
			}
		}

		RegCloseKey(hRegKey);
	}

	SetLastError(PIPE_ERROR_NAME_REGISTRATION_FAILED);

	// Pipe registration failed 

	return FALSE;
}

BOOL CPipe::UnRegisterPipe()
{
	if (m_bPipeRegistered && (RegDeleteKeyA(HKEY_LOCAL_MACHINE, m_SubKey.c_str()) == ERROR_SUCCESS))
	{
		m_bPipeRegistered = FALSE;
		return TRUE;
	}

	SetLastError(PIPE_ERROR_NAME_UNREGISTRATIONS_FAILED);

	return FALSE;
}

BOOL CPipe::UpdateRegistration()
{
	HKEY	hRegKey;
	DWORD	dwDisposition;

	if (RegCreateKeyExA(
		HKEY_LOCAL_MACHINE
		, m_SubKey.c_str()
		, 0
		, StringNull
		, 0
		, KEY_ALL_ACCESS | KEY_WOW64_64KEY
		, NULL
		, &hRegKey
		, &dwDisposition) == ERROR_SUCCESS
		)
	{
		if ((RegSetValueExA(hRegKey, "HRP", 0, REG_DWORD, (BYTE*)&m_hClientReadPipe, sizeof(DWORD)) &
			RegSetValueExA(hRegKey, "HWP", 0, REG_DWORD, (BYTE*)&m_hClientWritePipe, sizeof(DWORD))) == ERROR_SUCCESS)
		{
			RegCloseKey(hRegKey);

			return TRUE;
		}

		RegCloseKey(hRegKey);
	}

	return FALSE;
}

BOOL CPipe::RetrievePipe()
{
	HKEY	hRegKey;
	DWORD	dwValuePId;
	DWORD	dwReadPipe;
	DWORD	dwWritePipe;
	HANDLE	hProcess;
	DWORD	dwDataLen;
	HANDLE	hMutex;
	string	MutexName;

	MutexName = "Global\\";
	MutexName.append(m_GUID).append(StringSlash).append(m_PipeName);
	m_SubKey = "SOFTWARE\\";
	m_SubKey.append(m_GUID).append(PathDelimeter).append(m_PipeName);

	if (RegOpenKeyExA(
		HKEY_LOCAL_MACHINE
		, m_SubKey.c_str()
		, 0
		, KEY_ALL_ACCESS | KEY_WOW64_64KEY
		, &hRegKey) == ERROR_SUCCESS
		)
	{
		dwDataLen = sizeof(DWORD);

		if (RegQueryValueExA(hRegKey, "PID", 0, NULL, (BYTE*)&dwValuePId, &dwDataLen) == ERROR_SUCCESS)
		{
			if ((hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwValuePId)) != NULL)
			{
				if ((hMutex = OpenMutexA(
					MUTEX_ALL_ACCESS
					, FALSE
					, MutexName.c_str()))
					)
				{
					if ((RegQueryValueExA(hRegKey, "HRP", 0, NULL, (BYTE*)&dwReadPipe, &dwDataLen) &
						RegQueryValueExA(hRegKey, "HWP", 0, NULL, (BYTE*)&dwWritePipe, &dwDataLen)) == ERROR_SUCCESS)
					{
						if (dwReadPipe != NULL)
						{
							if (!DuplicateHandle(
								hProcess
								, (HANDLE)dwReadPipe
								, GetCurrentProcess()
								, &m_hReadPipe
								, 0
								, FALSE, DUPLICATE_SAME_ACCESS)
								)
							{
								m_hReadPipe = INVALID_HANDLE_VALUE;
							}
						}

						if (dwWritePipe != NULL)
						{
							if (!DuplicateHandle(
								hProcess
								, (HANDLE)dwWritePipe
								, GetCurrentProcess()
								, &m_hWritePipe
								, 0
								, FALSE, DUPLICATE_SAME_ACCESS)
								)
							{
								m_hWritePipe = INVALID_HANDLE_VALUE;
							}
						}

						if ((m_hReadPipe != INVALID_HANDLE_VALUE) && (m_hWritePipe != INVALID_HANDLE_VALUE))
						{
							CloseHandle(hProcess);
							CloseHandle(hMutex);
							RegCloseKey(hRegKey);

							return TRUE;
						}
					}

					SetLastError(PIPE_ERROR_CREATION_FAILED);
					CloseHandle(hProcess);
					CloseHandle(hMutex);
					RegCloseKey(hRegKey);

					return FALSE;
				}
			}

			CloseHandle(hProcess);
		}

		RegCloseKey(hRegKey);
	}

	SetLastError(PIPE_ERROR_NAME_DOES_NOT_EXISTS);

	return FALSE;
}

UINT CPipe::CallBackThread(
	LPVOID pParam
)
{
	CPipe	*pThis = (CPipe*)pParam;
	BOOL	bValidPtr = TRUE;
	BYTE	Buffer[PIPE_MAX_READ_BUFFER_SIZE];
	DWORD	dwBytesRead;

	pThis->m_bVaildThread = TRUE;
	pThis->m_pbVaildPtr = &bValidPtr;

	while (pThis->m_bRunThread && pThis->ReadPipe(&Buffer[0], PIPE_MAX_READ_BUFFER_SIZE, &dwBytesRead))
	{
		if (bValidPtr) pThis->m_CallBackFunc(pThis->m_pCallBackParam, &Buffer[0], dwBytesRead);
	}

	if (bValidPtr) pThis->m_bVaildThread = FALSE;

	return 0;
}

BOOL CPipe::CallBack()
{
	if (!m_CallBackFunc) return TRUE;

	if (AfxBeginThread((AFX_THREADPROC)CallBackThread, (VOID*)this))
	{
		return TRUE;
	}

	SetLastError(PIPE_ERROR_CALLBACK);

	return FALSE;
}

VOID CPipe::SetLastError(
	int nErrorCode
)
{
	m_LastError = nErrorCode;
}

