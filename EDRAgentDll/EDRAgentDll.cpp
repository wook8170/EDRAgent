// EDRAgentDll.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "stdio.h"
#include <string>
#include "EDRAgentDll.h"
#include "EDRAgentKernelHandle.h"
#include "../../EDRAgent/EDRAgent/EDRAgent/EDRConfig.h"

const ULONG REG_IOCTLS[] =
{
	IOCTL_EDR_REGISTRY_INIT
	,IOCTL_EDR_REGISTRY_DEINIT
	,IOCTL_EDR_REGISTRY_FULLLIST
	,IOCTL_EDR_REGISTRY_CLEAR
	,IOCTL_EDR_REGISTRY_ADD
	,IOCTL_EDR_REGISTRY_REMOVE
	,IOCTL_EDR_REGISTRY_INITEVENT
	,IOCTL_EDR_REGISTRY_GETLOGSIZE
	,IOCTL_EDR_REGISTRY_GETLOG
};

const ULONG FILE_IOCTLS[] =
{
	IOCTL_EDR_FILE_INIT
	,IOCTL_EDR_FILE_DEINIT
	,IOCTL_EDR_FILE_FULLLIST
	,IOCTL_EDR_FILE_CLEAR
	,IOCTL_EDR_FILE_ADD
	,IOCTL_EDR_FILE_REMOVE
	,IOCTL_EDR_FILE_INITEVENT
	,IOCTL_EDR_FILE_GETLOGSIZE
	,IOCTL_EDR_FILE_GETLOG
};

const ULONG URLTDI_IOCTLS[] =
{
	IOCTL_TDI_CMD_INIT
	,IOCTL_TDI_CMD_DEINIT
	,IOCTL_TDI_CMD_FULLLIST
	,IOCTL_TDI_CMD_CLEAR
	,IOCTL_TDI_CMD_ADD
	,IOCTL_TDI_CMD_REMOVE
	,IOCTL_TDI_CMD_INITEVENT
	,IOCTL_TDI_CMD_GETLOGSIZE
	,IOCTL_TDI_CMD_GETLOG
};

const ULONG IP_IOCTLS[] =
{
	IOCTL_SAFEFW_CMD_INIT
	,IOCTL_SAFEFW_CMD_DEINIT
	,IOCTL_SAFEFW_CMD_FULLLIST
	,IOCTL_SAFEFW_CMD_CLEAR
	,IOCTL_SAFEFW_CMD_ADD
	,IOCTL_SAFEFW_CMD_REMOVE
	,IOCTL_SAFEFW_CMD_INITEVENT
	,IOCTL_SAFEFW_CMD_GETLOGSIZE
	,IOCTL_SAFEFW_CMD_GETLOG
};

const ULONG URLLWF_IOCTLS[] =
{
	IOCTL_SAFEFW_CMD_INIT
	,IOCTL_SAFEFW_CMD_DEINIT
	,IOCTL_SAFEFW_URL_FULLLIST
	,IOCTL_SAFEFW_URL_CLEAR
	,IOCTL_SAFEFW_URL_ADD
	,IOCTL_SAFEFW_URL_REMOVE
	,IOCTL_SAFEFW_URL_INITEVENT
	,IOCTL_SAFEFW_URL_GETLOGSIZE
	,IOCTL_SAFEFW_URL_GETLOG
};

#define ADD_KERNEL_HANDLE(_pAgentKernelHandle_, _edrType_, _name_, _dosDeviceName_, _pIOCTLS_) {						\
	CKernelHandle* pKernelHandle = EDRNew CKernelHandle(_edrType_, _name_, _dosDeviceName_, _pIOCTLS_);					\
	if( !_pAgentKernelHandle_->IsRegistKernelHandle(pKernelHandle)) {													\
		if(pKernelHandle != NULL) {																						\
			_pAgentKernelHandle_->RegistKernelHandle(pKernelHandle);													\
		}																												\
	}																													\
	else {																												\
		EDRDelete(pKernelHandle);																						\
	}																													\
}

namespace SafePCKModule
{
	BOOL EDRAGENT_EXPORT_API InitDrv(INT nType)
	{
		CEDRAgentKernelHandle* pEDRAgentKernelHandle = CEDRAgentKernelHandle::Instance();

		CHECK_NOTNULL(pEDRAgentKernelHandle);

		EDROpt edrOpt;
		CEDRConfig *pEDRConfig = CEDRConfig::Instance();
		edrOpt.dwLogBufferCount = pEDRConfig->GetLogBufferCount(nType);
		edrOpt.bNeedConvertUnicode = pEDRConfig->IsUnicodeKernel(nType);
		edrOpt.dwUsePolling = pEDRConfig->UsePollingMethod4Log(nType);

		if (nType == (INT)EDR_TYPE_REG)
		{
			LOGI << "INIT RegistryHandler " << nType;
			ADD_KERNEL_HANDLE(pEDRAgentKernelHandle, EDR_TYPE_REG, "RegistryHandler", "\\\\.\\NICFSFD", &REG_IOCTLS[0]);
		}
		else if (nType == (INT)EDR_TYPE_FILE)
		{
			LOGI << "INIT FileHandler " << nType;
			ADD_KERNEL_HANDLE(pEDRAgentKernelHandle, EDR_TYPE_FILE, "FileHandler", "\\\\.\\NICFSFD", &FILE_IOCTLS[0]);
		}
		else if (nType == (INT)EDR_TYPE_IP)
		{
			LOGI << "INIT IPHandler " << nType;
			ADD_KERNEL_HANDLE(pEDRAgentKernelHandle, EDR_TYPE_IP, "IPHandler", "\\\\.\\SafeLwf", &IP_IOCTLS[0]);
		}
		else if (nType == (INT)EDR_TYPE_URL)
		{
			LOGI << "INIT URLLWFHandler " << nType;
			ADD_KERNEL_HANDLE(pEDRAgentKernelHandle, EDR_TYPE_URL, "URLLWFHandler", "\\\\.\\SafeLwf", &URLLWF_IOCTLS[0]);

			LOGI << "INIT URLTDIHandler " << nType;
			ADD_KERNEL_HANDLE(pEDRAgentKernelHandle, EDR_TYPE_URL, "URLTDIHandler", "\\\\.\\safetdi_nfo", &URLTDI_IOCTLS[0]);
		}
		else
		{
			return FALSE;
		}

		pEDRAgentKernelHandle->SetOpt((EEDRType)nType, &edrOpt);
		ULONG size = sizeof(EDROpt);
		BOOL bRet = pEDRAgentKernelHandle->DoIOCTL(
			(EEDRType)nType
			, EIOCTL_INIT
			, (LPVOID)&edrOpt
			, (ULONG)sizeof(EDROpt)
		);

		return bRet;
	}

	BOOL EDRAGENT_EXPORT_API DeInitDrv(INT nType)
	{
		CEDRAgentKernelHandle* pEDRAgentKernelHandle = CEDRAgentKernelHandle::Instance();

		CHECK_NOTNULL(pEDRAgentKernelHandle);

		return pEDRAgentKernelHandle->DoIOCTL(
			(EEDRType)nType
			, EIOCTL_DEINIT
		);
	}

	BOOL EDRAGENT_EXPORT_API SetFullList(INT nType, CHAR* inBuf, DWORD inLen)
	{
		CEDRAgentKernelHandle* pEDRAgentKernelHandle = CEDRAgentKernelHandle::Instance();

		CHECK_NOTNULL(pEDRAgentKernelHandle);

		return pEDRAgentKernelHandle->DoIOCTL(
			(EEDRType)nType
			, EIOCTL_SETFULLLIST
			, (LPVOID)inBuf
			, (ULONG)inLen
		);
	}

	BOOL EDRAGENT_EXPORT_API Clear(INT nType)
	{
		CEDRAgentKernelHandle* pEDRAgentKernelHandle = CEDRAgentKernelHandle::Instance();

		CHECK_NOTNULL(pEDRAgentKernelHandle);

		return pEDRAgentKernelHandle->DoIOCTL(
			(EEDRType)nType
			, EIOCTL_CLEAR
		);
	}

	BOOL EDRAGENT_EXPORT_API Add(INT nType, CHAR* inBuf, DWORD inLen)
	{
		CEDRAgentKernelHandle* pEDRAgentKernelHandle = CEDRAgentKernelHandle::Instance();

		CHECK_NOTNULL(pEDRAgentKernelHandle);

		return pEDRAgentKernelHandle->DoIOCTL(
			(EEDRType)nType
			, EIOCTL_ADD
			, (LPVOID)inBuf
			, (DWORD)inLen
		);
	}

	BOOL EDRAGENT_EXPORT_API Remove(INT nType, CHAR* inBuf, DWORD inLen)
	{
		CEDRAgentKernelHandle* pEDRAgentKernelHandle = CEDRAgentKernelHandle::Instance();

		CHECK_NOTNULL(pEDRAgentKernelHandle);

		return pEDRAgentKernelHandle->DoIOCTL(
			(EEDRType)nType
			, EIOCTL_REMOVE
			, (LPVOID)inBuf
			, (DWORD)inLen
		);
	}

	BOOL EDRAGENT_EXPORT_API InitEvent(INT nType, HANDLE* inBuf, DWORD inLen)
	{
		CEDRAgentKernelHandle* pEDRAgentKernelHandle = CEDRAgentKernelHandle::Instance();

		CHECK_NOTNULL(pEDRAgentKernelHandle);

		return pEDRAgentKernelHandle->DoIOCTL(
			(EEDRType)nType
			, EIOCTL_SETEVENT
			, (LPVOID)inBuf
			, (DWORD)inLen
		);
	}

	BOOL EDRAGENT_EXPORT_API GetLogSize(INT nType, ULONG* outBuf, DWORD outLen)
	{
		CEDRAgentKernelHandle* pEDRAgentKernelHandle = CEDRAgentKernelHandle::Instance();

		CHECK_NOTNULL(pEDRAgentKernelHandle);

		return pEDRAgentKernelHandle->DoIOCTL(
			(EEDRType)nType
			, EIOCTL_GETLOGSIZE
			, NULL
			, 0
			, (LPVOID)outBuf
			, (DWORD)outLen
		);
	}

	BOOL EDRAGENT_EXPORT_API GetLog(INT nType, CHAR* outBuf, DWORD outLen)
	{
		CEDRAgentKernelHandle* pEDRAgentKernelHandle = CEDRAgentKernelHandle::Instance();

		CHECK_NOTNULL(pEDRAgentKernelHandle);

		return pEDRAgentKernelHandle->DoIOCTL(
			(EEDRType)nType
			, EIOCTL_GETLOG
			, NULL
			, 0
			, (LPVOID)outBuf
			, (DWORD)outLen
		);
	}
}
