#pragma once

#include "CommonDef.h"
#include "EDRKernelDelegator.h"

class CEDRKernelHandler
{
private:
	CEDRKernelDelegator					* m_pLogProcess;
	BOOL							m_bInit;
	string							m_strName;
	EEDRType						m_eType;
	BOOL							m_bUsePollingMethod;
public:
	CEDRKernelHandler(
		string name
		, EEDRType eType
		, BOOL usePollingMethod = FALSE
	)
	{
		m_strName = name;
		m_eType = eType;
		m_bUsePollingMethod = usePollingMethod;

		LOGI 
			<< "CEDRKernelHandler: name (" 
			<< m_strName
			<< "), type (" 
			<< m_eType 
			<< "), m_bUsePollingMethod (" 
			<< m_bUsePollingMethod 
			<< ")";

		m_pLogProcess = EDRNew CEDRKernelDelegator(
			m_strName
			, m_eType
			, m_bUsePollingMethod
		);

	};

	CEDRKernelHandler() 
	{
		EDRDelete(m_pLogProcess);
	};

};

