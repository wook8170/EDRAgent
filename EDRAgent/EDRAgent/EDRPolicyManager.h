#pragma once

#include "CommonDef.h"
#include "HttpClient/HttpContext.h"
#include "HttpClient/HttpClientBuilder.h"
#include "HttpClient/HttpData.h"
#include "HttpClient/HttpClient.h"
#include "EDRPolicyHelperBase.h"
#include "ThreadPool/LockGuard.h"

/****************************************
Implementation of DER Policy Manager
****************************************/
class CEDRPolicyManager
{
private:
	string 							m_strName;
	string							m_strUrl;
	CHttpContext *					m_pContext;
	CHttpClientBuilder *			m_pHttpClientBuilder;
	CHttpData*						m_pResData;
	CHttpData*						m_pReqData;
	CHttpClient *					m_pHttpClient;
	CHttpAttribute::EMethod			m_eMethod;
	CHttpAttribute::EEDRPolicyType	m_ePolicyType;
	CHttpAttribute::EEDRManagerType	m_eCurrentPolicyHelper;
	CEDRPolicyHelperBase *			m_pCurrentPolicyHelper;
	CEDRPolicyHelperBase *			m_pPolicyHelper[CHttpAttribute::ManagerMAX];
	BOOL							m_bUsed;
	LOCK							m_lock;

	string GetUrlWithPolicyType(
		CHttpAttribute::EEDRPolicyType ePolicyType = CHttpAttribute::PolicyInvalid
	);

	VOID SetCurrentPolicyHelper(
		CHttpAttribute::EEDRManagerType eManagerType
	);

	VOID SetCurrentPolicyHelperWithPolicyType(
		CHttpAttribute::EEDRPolicyType ePolicyType
	);

	CEDRPolicyHelperBase * GetCurrentPolicyHelper();

	string GetCurrentName();

public:
	CEDRPolicyManager();

	CEDRPolicyManager(
		string  name
	);

	~CEDRPolicyManager();

	BOOL Init(
		CHttpAttribute::EMethod eMethod = CHttpAttribute::Get
		, CHttpAttribute::EEDRPolicyType ePolicyType = CHttpAttribute::PolicyIOC
	);

	BOOL Reset(
		CHttpAttribute::EMethod eMethod = CHttpAttribute::Get
		, CHttpAttribute::EEDRPolicyType ePolicyType = CHttpAttribute::PolicyIOC
	);

	BOOL Request(
		CHttpAttribute::EEDRPolicyType ePolicyType
	);

	BOOL Request(
		string url = StringNull
	);

	VOID SetPolicyHelper(
		CHttpAttribute::EEDRManagerType eMangerType
		, CEDRPolicyHelperBase* pPolicyHelper
	);

	EDRJson PerformAckGenerate(
		CHttpData* pHttpData
		, BOOL status
		, string keyName = JsonField_Data
	);

	EDRJson PerformPolicyExtrator(
		CHttpData* pHttpData
		, string keyName = JsonField_Data
	);

	BOOL PerformPolicyApplier(
		EDRJson
	);

};

