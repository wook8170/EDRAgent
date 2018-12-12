#include "stdafx.h"
#include "EDRAgentDll.h"
#include "EDRAgentKernelHandle.h"
#include "Util/Util.h"
#include "Util/FileSystem.h"
#include "ThreadPool/LockGuard.h"
#include "../../EDRAgent/EDRAgent/EDRAgent/EDRConfig.h"
#include "EDRFileHashMgr.h"

using namespace SafePCKModule;

CCriticalSection::CCriticalSection()
{
	InitializeCriticalSection(&m_cs);
}

CCriticalSection::~CCriticalSection()
{
	DeleteCriticalSection(&m_cs);
}

void CCriticalSection::Lock()
{
	EnterCriticalSection(&m_cs);
}

void CCriticalSection::Unlock()
{
	LeaveCriticalSection(&m_cs);
}

ULONG CKernelHandle::GetIOCTL(
	EIOCTLCode code
)
{
	return m_ioctl[code];
}

string CKernelHandle::GetIOCTL2String(
	EIOCTLCode code
)
{
	if (code == EIOCTL_INIT) return "EIOCTL_INIT";
	else if (code == EIOCTL_DEINIT) return "EIOCTL_DEINIT";
	else if (code == EIOCTL_SETFULLLIST) return "EIOCTL_SETFULLLIST";
	else if (code == EIOCTL_CLEAR) return "EIOCTL_CLEAR";
	else if (code == EIOCTL_ADD) return "EIOCTL_ADD";
	else if (code == EIOCTL_REMOVE) return "EIOCTL_REMOVE";
	else if (code == EIOCTL_SETEVENT) return "EIOCTL_SETEVENT";
	else if (code == EIOCTL_GETLOGSIZE) return "EIOCTL_GETLOGSIZE";
	else if (code == EIOCTL_GETLOG) return "EIOCTL_GETLOG";
	else return "EIOCTL_INVALID";

}

typedef struct _KERNEL_HANDLE_MAP_T
{
	string name;
	string handleName;
	DWORD *ioctls;
	BOOL needConvertUnicode;
} KERNEL_HANDLE_MAP_T;


CKernelHandle::CKernelHandle(
	EEDRType type
	, string name
	, string handleName
	, const ULONG *ioctls
)
{
	m_eType = type;
	m_strName = name;
	m_strHandleName = handleName;
	m_handle = CreateFile(
		m_strHandleName.c_str()
		, GENERIC_READ | GENERIC_WRITE
		, 0
		, NULL
		, OPEN_EXISTING
		, FILE_ATTRIBUTE_NORMAL
		, NULL
	);

	m_ioctl = (ULONG*)ioctls;

	for (auto i = 0; i < EIOCTL_MAX; i++)
	{
		LOGD << "IOCTL[" << i << "] : " << ioctls[i];
	}

	m_bInit = FALSE;
	m_bInitEvent = FALSE;
}

CKernelHandle::~CKernelHandle()
{
	if (m_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_handle);
	}
}

BOOL CKernelHandle::IsInit()
{
	return (m_bInit || m_bInitEvent);
}

BOOL CKernelHandle::SetOpt(
	EDROpt* opt
)
{
	m_EDROpt.dwLogBufferCount = opt->dwLogBufferCount;
	m_EDROpt.bNeedConvertUnicode = opt->bNeedConvertUnicode;
	m_EDROpt.dwMaxBuffSize = opt->dwMaxBuffSize;
	m_EDROpt.dwMaxTrhSize = opt->dwMaxTrhSize;
	m_EDROpt.dwUsePolling = opt->dwUsePolling;

	return TRUE;
}

EEDRType CKernelHandle::GetEDRType()
{
	return m_eType;
}

HANDLE CKernelHandle::GetEventHandle()
{
	return m_logEvent.GetHandle();
}

string CKernelHandle::GetKernelHandleName()
{
	return m_strHandleName;
}

string CKernelHandle::GetName()
{
	return m_strName;
}

BOOL CKernelHandle::DoIOCTL(
	EIOCTLCode code
	, LPVOID InBuffer
	, DWORD InBufferSize
	, LPVOID OutBuffer
	, DWORD OutBufferSize
)
{
	BOOL	bRet = FALSE;
	DWORD	dwBytesReturned = 0;
	LPVOID	pInBuffer;
	DWORD	dwInBufferSize;

	if ((m_bInit && code == EIOCTL_INIT) || (m_bInitEvent && code == EIOCTL_SETEVENT))
	{
		Print();
		LOGD << "@@ - CODE            :" << GetIOCTL2String(code);
		LOGD << "@@ - IOCTL           :" << GetIOCTL(code);		
		LOGI << "@@ - " << GetName() << " 은 이미 초기화가 되었습니다.";
		return FALSE;
	}

	if (m_handle == INVALID_HANDLE_VALUE)
	{
		m_handle = CreateFile(
			m_strHandleName.c_str()
			, GENERIC_READ | GENERIC_WRITE
			, 0
			, NULL
			, OPEN_EXISTING
			, FILE_ATTRIBUTE_NORMAL
			, NULL
		);
	}

	if (m_handle != INVALID_HANDLE_VALUE)
	{
		wstring uniszList;
		DWORD ioctl = GetIOCTL(code);
		
		Print();
		LOGD << "@@ - CODE            :" << GetIOCTL2String(code);
		LOGD << "@@ - IOCTL           :" << ioctl;

		if (InBuffer != NULL)
		{
			if (( code == EIOCTL_SETFULLLIST || code == EIOCTL_ADD || code == EIOCTL_REMOVE ) && m_EDROpt.bNeedConvertUnicode)
			{
				uniszList = CUtil::ConvM2U((CHAR*)InBuffer);
				pInBuffer = (LPVOID)uniszList.c_str();
				dwInBufferSize = (DWORD)uniszList.length() * sizeof(WCHAR);	
				LOGD << "@@ - pInBuffer U     :" << uniszList;
			}
			else
			{
				pInBuffer = InBuffer;
				dwInBufferSize = InBufferSize;
				LOGD << "@@ - pInBuffer A     :" << (CHAR*)pInBuffer;
			}
		}

		LOGD << "@@ - dwInBufferSize  :" << dwInBufferSize;
		LOGD << "@@ - OutBuffer       :" << OutBuffer;
		LOGD << "@@ - OutBufferSize   :" << OutBufferSize;

		if (ioctl != IOCTL_INVALID)
		{
			bRet = DeviceIoControl(
				m_handle
				, ioctl
				, pInBuffer
				, dwInBufferSize
				, OutBuffer
				, OutBufferSize
				, &dwBytesReturned
				, NULL);
		}	

		LOGD << "@@ - Ret             :" << bRet;

	}

	if (code == EIOCTL_INIT )//&& bRet == TRUE)
	{
		m_bInit = TRUE;
		if (GetEDRType() == EDR_TYPE_FILE)
			StartEDRFileHashMgr();
	}

	if (code == EIOCTL_SETEVENT)//&& bRet == TRUE)
	{
		m_bInitEvent = TRUE;
	}

	if (code == EIOCTL_DEINIT )//&& bRet == TRUE)
	{
		m_bInit = FALSE;
		m_bInitEvent = FALSE;
		if (GetEDRType() == EDR_TYPE_FILE)
			StopEDRFileHashMgr();
	}

	return bRet;
}

VOID CKernelHandle::Print()
{
	LOGI << "----------------------";
	LOGI << "@@ Name              : " << m_strName;
	LOGI << "@@ Kernel Handle name: " << m_strHandleName;
	LOGI << "@@ HANDLE            : " << m_handle;
	LOGI << "@@ Type              : " << m_eType;
	LOGI << "@@ OPT logCount      : " << m_EDROpt.dwLogBufferCount;
	LOGI << "@@ OPT Unicode       : " << m_EDROpt.bNeedConvertUnicode;
}

CEDRAgentKernelHandle * CEDRAgentKernelHandle::Instance()
{
	static CEDRAgentKernelHandle * pInstance = NULL;
	if (NULL == pInstance)
	{
#if 1//def _DEBUG
		File::Remove(".\\Log\\EDRAgentDll.log");;
#endif
		plog::init(plog::verbose, ".\\Log\\EDRAgentDll.log", 1024 * 1024 * 10, 10);

		LOGI << "====================================================================================================================================================";
		LOGI << "##########  #########    ########         ##       ########   ##########  ###     ##  ##########           #########     ##            ##           ";
		LOGI << "##          ##      ##   ##      ##     ######    ##      ##  ##          ####    ##      ##               ##      ##    ##            ##           ";
		LOGI << "##          ##       ##  ##      ##    ##    ##   ##          ##          ## ##   ##      ##               ##       ##   ##            ##           ";
		LOGI << "########    ##       ##  ########     ##########  ##    ####  #########   ##  ##  ##      ##               ##       ##   ##            ##           ";
		LOGI << "##          ##       ##  ##    ##     ##      ##  ##      ##  ##          ##   ## ##      ##               ##       ##   ##            ##           ";
		LOGI << "##          ##      ##   ##     ##    ##      ##  ##      ##  ##          ##    ####      ##       ####	##      ##    ##            ##           ";
		LOGI << "##########  ########     ##      ##   ##      ##   ########   ##########  ##     ###      ##       ####	########      ##########    ##########   ";
		LOGI << "----------------------------------------------------------------------------------------------------------------------------------------------------";
		LOGI << "EDRAgent DLL START !!";
		LOGI << "DATE: " << CUtil::CurrentDateTime();
		LOGI << "====================================================================================================================================================";
		LOGI << StringNull;
		LOGI << "EDRAgentDll.dll loaded !!";

		static CCriticalSection criticalSection;
		criticalSection.Lock();
		if (NULL == pInstance)
		{
			static CEDRAgentKernelHandle pRealInstance;
			pInstance = &pRealInstance;
		}

		criticalSection.Unlock();
	}

	return pInstance;
}

BOOL CEDRAgentKernelHandle::SetOpt(
	EEDRType handleType
	, EDROpt* opt
)
{
	BOOL bRet = FALSE;

	vector<CKernelHandle*> pKernelHandles = GetKernelHandle((EEDRType)handleType);

	for (auto it = pKernelHandles.begin(); it != pKernelHandles.end(); it++)
	{
		CKernelHandle* pKernelHandle = (*it);

		if (pKernelHandle)
		{
			bRet = pKernelHandle->SetOpt(opt);

			if (!bRet)
			{
				break;
			}
		}
	}

	return bRet;
}

vector<CKernelHandle*> CEDRAgentKernelHandle::GetKernelHandle(
	EEDRType eCode
)
{
	vector<CKernelHandle*> kernelHandles;

	auto iterPair = m_KernelHandle.equal_range(eCode);

	for (auto it = iterPair.first; it != iterPair.second; it++)
	{
		CKernelHandle* pKernelHandle = it->second;
		kernelHandles.push_back(pKernelHandle);
	}

	return kernelHandles;
}

BOOL CEDRAgentKernelHandle::RegistKernelHandle(
	CKernelHandle* kenelHandle
)
{
	m_KernelHandle.insert(pair<EEDRType, CKernelHandle*>(kenelHandle->GetEDRType(), kenelHandle));

	return TRUE;
}

BOOL CEDRAgentKernelHandle::IsRegistKernelHandle(
	CKernelHandle* kenelHandle
)
{
	BOOL bRet = FALSE;
	CKernelHandle* pKenelHandle;

	auto iterPair = m_KernelHandle.equal_range(kenelHandle->GetEDRType());

	for (auto it = iterPair.first; it != iterPair.second; )
	{
		pKenelHandle = it->second;

		if (CUtil::Compare(pKenelHandle->GetName(), kenelHandle->GetName()) == 0
			&& CUtil::Compare(pKenelHandle->GetKernelHandleName(), kenelHandle->GetKernelHandleName()) == 0)
		{
			bRet = TRUE;
			break;
		}
		else
		{
			it++;
		}
	}

	return bRet;
}

BOOL CEDRAgentKernelHandle::DoIOCTL(
	EEDRType handleType
	, EIOCTLCode ioctl
	, LPVOID InBuffer
	, DWORD InBufferSize
	, LPVOID OutBuffer
	, DWORD OutBufferSize
)
{
	BOOL bRet = TRUE;

	vector<CKernelHandle*> pKernelHandles = GetKernelHandle((EEDRType)handleType);

	for (auto it = pKernelHandles.begin(); it != pKernelHandles.end(); it++)
	{
		CKernelHandle* pKernelHandle = (*it);

		if (pKernelHandle)
		{			
			if (!pKernelHandle->DoIOCTL(
				ioctl
				, InBuffer
				, InBufferSize
				, OutBuffer
				, OutBufferSize
			))
			{
				bRet = FALSE;
			}
		}
	}

	return bRet;
}

