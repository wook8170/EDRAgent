#pragma once

#define		PIPE_MAX_READ_BUFFER_SIZE		0xFFF
#define		MESSAGE_BUFFER_SIZE				1024

enum
{
	PIPE_ERROR_SUCCESS = 0x40000000,		// Success
	PIPE_ERROR_NAME_ALREADY_EXISTS,			// The specified pipe name already exists
	PIPE_ERROR_NAME_REGISTRATION_FAILED,	// The pipe name registration failed 
	PIPE_ERROR_NAME_DOES_NOT_EXISTS,		// The specified pipe name doesn´t exists
	PIPE_ERROR_NAME_UNREGISTRATIONS_FAILED, // The pipe name unregistration failed 
	PIPE_ERROR_CREATION_FAILED,				// Pipe creation failed
	PIPE_ERROR_CALLBACK,					// Error related to callback
	PIPE_ERROR_READ_FAILED,					// Error while reading from pipe
	PIPE_ERROR_WRITE_FAILED,				// Error while Wrinting to pipe
	PIPE_CREATE_ALREADY_CALLED				// Create was already called
};

enum
{
	PIPE_READ = 0x01,			// Grant client read access
	PIPE_WRITE													// Grant client write access
};

typedef	UINT(*PIPE_CALLBACK)	(LPVOID pParam, LPVOID Buffer, DWORD dwLength);

typedef struct IPCMessage_T
{
	DWORD	type;
	CHAR	message[MESSAGE_BUFFER_SIZE];
} IPCMessage_T, *PIPCMessage_T;

#define IPCMessage_SIZE	sizeof(IPCMessage_T)

class CPipe
{
private:

	string				m_GUID;
	string				m_SubKey;
	int					m_LastError;

	std::string				m_PipeName;
	DWORD				m_dwDesiredAccess;
	BOOL				m_bServerSide;
	PIPE_CALLBACK		m_CallBackFunc;
	LPVOID				m_pCallBackParam;

	BOOL				m_bCreateCalled;
	BOOL				m_bCloseCalled;
	BOOL				m_bPipeRegistered;

	HANDLE				m_hReadPipe;
	HANDLE				m_hWritePipe;
	HANDLE				m_hClientReadPipe;
	HANDLE				m_hClientWritePipe;
	HANDLE				m_hMutex;

	BOOL				*m_pbVaildPtr;
	BOOL				m_bVaildThread;
	BOOL				m_bRunThread;

	BOOL RegisterPipe();

	BOOL UnRegisterPipe();

	BOOL UpdateRegistration();

	BOOL RetrievePipe();

	BOOL CallBack();

	VOID SetLastError(
		int nErrorCode
	);

	static UINT CallBackThread(
		LPVOID pParam
	);

public:

	CPipe();
	~CPipe();
	BOOL Create(
		string PipeName
		, DWORD dwDesiredAccess = PIPE_READ | PIPE_WRITE
		, BOOL bServerSide = TRUE
		, PIPE_CALLBACK CallBackFunc = NULL
		, LPVOID pCallBackParam = NULL
	);

	BOOL ReadPipe(
		LPVOID Buffer
		, DWORD nNumberOfBytesToRead
		, DWORD* lpNumberOfBytesRead
	);

	BOOL WritePipe(
		LPVOID Buffer
		, DWORD nNumberOfBytesToWrite
	);

	VOID Close();

	int GetLastError();
};