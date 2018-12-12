#pragma once
#include "EDRAgentDll.h"
#include "Util/SyncEvent.h"

#include <winioctl.h>

class CCriticalSection
{
public:
	CCriticalSection();

	~CCriticalSection();
	
	void Lock();
	
	void Unlock();

private:
	CRITICAL_SECTION m_cs;
};

class CKernelHandle
{
private:	
	string		m_strName;
	string		m_strHandleName;
	HANDLE		m_handle;
	CSyncEvent	m_logEvent;
	ULONG		*m_ioctl;
	BOOL		m_bInit;	
	BOOL		m_bInitEvent;
	EEDRType	m_eType;

	EDROpt		m_EDROpt;

protected:
	ULONG GetIOCTL(
		EIOCTLCode code
	);

	string GetIOCTL2String(
		EIOCTLCode code
	);

public:
	CKernelHandle(
		EEDRType type
		, string name
		, string handleName
		, const ULONG *ioctls
	);

	~CKernelHandle();

	BOOL IsInit();

	BOOL SetOpt(
		EDROpt* opt
	);

	EEDRType GetEDRType();

	HANDLE GetEventHandle();

	string GetKernelHandleName();

	string GetName();

	BOOL DoIOCTL(
		EIOCTLCode code
		, LPVOID InBuffer = NULL
		, DWORD InBufferSize = 0
		, LPVOID OutBuffer = NULL
		, DWORD OutBufferSize = 0
	);

	VOID Print();
};

class CEDRAgentKernelHandle
{
private:
	multimap<EEDRType, CKernelHandle*> m_KernelHandle;

protected:
	CEDRAgentKernelHandle()
	{
		
	};

public:
	static CEDRAgentKernelHandle * Instance();

	BOOL SetOpt(
		EEDRType eCode
		, EDROpt* opt
	);

	vector<CKernelHandle*> GetKernelHandle(
		EEDRType eCode
	);

	BOOL RegistKernelHandle(
		CKernelHandle* kenelHandle
	);

	BOOL IsRegistKernelHandle(
		CKernelHandle* kenelHandle
	);

	BOOL DoIOCTL(
		EEDRType handleType
		, EIOCTLCode ioctl
		, LPVOID InBuffer = NULL
		, DWORD InBufferSize = 0
		, LPVOID OutBuffer = NULL
		, DWORD OutBufferSize = 0
	);
};