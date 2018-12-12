#include "StdAfx.h"
#include "EDRPolicyManager.h"
#include "EDRConfig.h"


CEDRPolicyManager::CEDRPolicyManager()
{
	m_strName = "<defalut name> ";

	m_strUrl = "";
	m_pContext = NULL;
	m_pHttpClientBuilder = NULL;
	m_pResData = NULL;
	m_pReqData = NULL;
	m_pHttpClient = NULL;
	m_pCurrentPolicyHelper = NULL;

	m_pPolicyHelper[CHttpAttribute::ManagerIOC] = NULL;
	m_pPolicyHelper[CHttpAttribute::ManagerRES] = NULL;

	m_eCurrentPolicyHelper = CHttpAttribute::ManagerInvalid;
	m_bUsed = FALSE;

	LockGuard::Create(&m_lock);
}

CEDRPolicyManager::CEDRPolicyManager(string  name)
{
	m_strName = "<" + name + "> ";

	m_strUrl = "";
	m_pContext = NULL;
	m_pHttpClientBuilder = NULL;
	m_pResData = NULL;
	m_pReqData = NULL;
	m_pHttpClient = NULL;
	m_pCurrentPolicyHelper = NULL;

	m_pPolicyHelper[CHttpAttribute::ManagerIOC] = NULL;
	m_pPolicyHelper[CHttpAttribute::ManagerRES] = NULL;

	m_eCurrentPolicyHelper = CHttpAttribute::ManagerInvalid;
	m_bUsed = FALSE;

	LockGuard::Create(&m_lock);
}

CEDRPolicyManager::~CEDRPolicyManager()
{
	LockGuard::Destroy(&m_lock);

	EDRDelete(m_pHttpClientBuilder);
	EDRDelete(m_pHttpClient);
	EDRDelete(m_pReqData);
	EDRDelete(m_pContext);
}

string CEDRPolicyManager::GetUrlWithPolicyType(
	CHttpAttribute::EEDRPolicyType ePolicyType
)
{
	CHECK_NOTNULL(m_pCurrentPolicyHelper);

	LOGD << "EEDRPolicyType: " << ePolicyType;

	string url = StringNull;

	switch (ePolicyType)
	{
	case CHttpAttribute::PolicyInvalid:
		LOGD << "Non parameter: " << m_strUrl;
		break;
	case CHttpAttribute::PolicyIOCRefresh:
		SetCurrentPolicyHelper(CHttpAttribute::ManagerIOC);
		url.append(CEDRConfig::Instance()->GetRefreshURL(CEDRConfig::EEDRConfigIOC));
		url.append(StringSlash);
		url.append(CUtil::GetUserID());
		break;

	case CHttpAttribute::PolicyRESRefresh:
		SetCurrentPolicyHelper(CHttpAttribute::ManagerRES);
		url.append(CEDRConfig::Instance()->GetRefreshURL(CEDRConfig::EEDRConfigRES));
		url.append(StringSlash);
		url.append(CUtil::GetUserID());
		break;

	case CHttpAttribute::PolicyIOC:
		SetCurrentPolicyHelper(CHttpAttribute::ManagerIOC);
		url.append(CEDRConfig::Instance()->GetNormalURL(CEDRConfig::EEDRConfigIOC));
		url.append(StringSlash);
		url.append(CUtil::GetUserID());
		break;

	case CHttpAttribute::PolicyRESProcess:
		SetCurrentPolicyHelper(CHttpAttribute::ManagerRES);
		url.append(CEDRConfig::Instance()->GetNormalURL(CEDRConfig::EEDRConfigRES));
		url.append(StringSlash);
		url.append(CEDRConfig::Instance()->GetSubURL(Type_Process, CEDRConfig::EEDRConfigRES));
		url.append(StringSlash);
		url.append(CUtil::GetUserID());
		break;

	case CHttpAttribute::PolicyRESFile:
		SetCurrentPolicyHelper(CHttpAttribute::ManagerRES);
		url.append(CEDRConfig::Instance()->GetNormalURL(CEDRConfig::EEDRConfigRES));
		url.append(StringSlash);
		url.append(CEDRConfig::Instance()->GetSubURL(Type_File, CEDRConfig::EEDRConfigRES));
		url.append(StringSlash);
		url.append(CUtil::GetUserID());
		break;

	case CHttpAttribute::PolicyRESRegistry:
		SetCurrentPolicyHelper(CHttpAttribute::ManagerRES);
		url.append(CEDRConfig::Instance()->GetNormalURL(CEDRConfig::EEDRConfigRES));
		url.append(StringSlash);
		url.append(CEDRConfig::Instance()->GetSubURL(Type_Registry, CEDRConfig::EEDRConfigRES));
		url.append(StringSlash);
		url.append(CUtil::GetUserID());
		break;

	case CHttpAttribute::PolicyRESNetwork:
		SetCurrentPolicyHelper(CHttpAttribute::ManagerRES);
		url.append(CEDRConfig::Instance()->GetNormalURL(CEDRConfig::EEDRConfigRES));
		url.append(StringSlash);
		url.append(CEDRConfig::Instance()->GetSubURL(Type_IP, CEDRConfig::EEDRConfigRES));
		url.append(StringSlash);
		url.append(CUtil::GetUserID());
		break;

	default:
		break;

	}

	LOGD << "URL: " << url;
	return url;
}

VOID CEDRPolicyManager::SetCurrentPolicyHelper(
	CHttpAttribute::EEDRManagerType eManagerType
)
{
	m_eCurrentPolicyHelper = eManagerType;
	m_pCurrentPolicyHelper = m_pPolicyHelper[m_eCurrentPolicyHelper];
}

VOID CEDRPolicyManager::SetCurrentPolicyHelperWithPolicyType(
	CHttpAttribute::EEDRPolicyType ePolicyType
)
{
	if (ePolicyType == CHttpAttribute::PolicyInvalid)
	{
		LOGW << "잘못된 PolicyManager 타입입니다.";
		return;
	}
	else if (ePolicyType >= CHttpAttribute::PolicyIOCRefresh
		&& ePolicyType < CHttpAttribute::PolicyRESRefresh)
	{
		SetCurrentPolicyHelper(CHttpAttribute::ManagerIOC);
	}
	else if (ePolicyType >= CHttpAttribute::PolicyRESRefresh
		&& ePolicyType < CHttpAttribute::PolicyMAX)
	{
		SetCurrentPolicyHelper(CHttpAttribute::ManagerRES);
	}
}

CEDRPolicyHelperBase * CEDRPolicyManager::GetCurrentPolicyHelper()
{
	return m_pCurrentPolicyHelper;
}

BOOL CEDRPolicyManager::Init(
	CHttpAttribute::EMethod eMethod
	, CHttpAttribute::EEDRPolicyType ePolicyType
)
{
	LockGuard lock(&m_lock);
	
	LOGI << GetCurrentName() << "초기화 시작";

	m_pContext = EDRNew CHttpContext();

	// Create a HttpClient builder
	m_pHttpClientBuilder = EDRNew CHttpClientBuilder(m_pContext);
	m_ePolicyType = ePolicyType;
	m_eMethod = eMethod;

	SetCurrentPolicyHelperWithPolicyType(m_ePolicyType);

	return TRUE;
}

string CEDRPolicyManager::GetCurrentName()
{

	string name = "[ ";
	name.append(m_strName)
		.append(" - ")
		.append((m_pCurrentPolicyHelper != NULL) ? m_pCurrentPolicyHelper->GetName() : StringNull)
		.append(" ] ");

	return name;
}

BOOL CEDRPolicyManager::Reset(
	CHttpAttribute::EMethod eMethod
	, CHttpAttribute::EEDRPolicyType ePolicyType
)
{
	{
		LockGuard lock(&m_lock);

		LOGI << GetCurrentName() << "리셋";

		EDRDelete(m_pHttpClientBuilder);
		EDRDelete(m_pHttpClient);
		EDRDelete(m_pReqData);
		EDRDelete(m_pContext);

		m_bUsed = FALSE;
		m_ePolicyType = ePolicyType;
		m_eMethod = eMethod;
	}

	return Init(eMethod, ePolicyType);
}

BOOL CEDRPolicyManager::Request(CHttpAttribute::EEDRPolicyType ePolicyType)
{
	string url = GetUrlWithPolicyType(ePolicyType);

	return Request(url);
}

BOOL CEDRPolicyManager::Request(string url)
{
	LockGuard lock(&m_lock);

	LOGW_IF(m_bUsed) << "객체 리셋이 필요합니다. Reset() 함수룰 호출해야 합니다.";

	if (!m_pCurrentPolicyHelper)
	{
		LOGW << "CEDRPolicyHelperBase 인스턴스가 유효하지 않습니다.";
		return FALSE;
	}

	m_strUrl = GetUrlWithPolicyType(m_ePolicyType);

	LOGI << "HTTP Start >> ( " << m_strUrl << " )";

	if (url.empty() && m_strUrl.empty())
	{
		LOGW << "요청 URL 이 없습니다 !";
		return FALSE;
	}

	string reqUrl = (!m_strUrl.empty()) ? m_strUrl : url;

	CHECK_NOTNULL(m_pCurrentPolicyHelper);

	// CHttpClient 객체 생성
	m_pHttpClient = m_pHttpClientBuilder
		->SetMethod(m_eMethod)
		->SetURL(reqUrl)
		->Build();

	CHECK_NOTNULL(m_pHttpClient);

	// HTTP Request 전송 ( CHttpClient )
	LOGI << GetCurrentName() << "HTTP URL: " << reqUrl;
	HTTP_STATUS httpStatus = m_pHttpClient->SendRequest();
	LOGI << GetCurrentName() << "HTTP Send (REQ) Status: " << httpStatus;

	// HTTP Request 결과 확인
	if (httpStatus != HTTP_OK)
	{
		/*
		LOGW_IF(m_pHttpClient->GetResponseData() != NULL)
			<< GetCurrentName()
			<< "HTTP Response (REQ): \n"
			<< m_pHttpClient->GetResponseData()->GetDataAsRaw()
			<< "\n";
		//*/
		LOGW << GetCurrentName() << "HTTP Error (REQ) Status: " << httpStatus;
		LOGW << GetCurrentName() << "HTTP Finish <<";
		return FALSE;
	}

	// HTTP Response 수신
	m_pResData = m_pHttpClient->GetResponseData();
	//LOGI << GetCurrentName() << "HTTP Response (REQ): \n" << m_pResData->GetDataAsJson() << "\n";

	// 수신된 HTTP Data 로 부터 정책 적용	
	EDRJson		jsonPolicy;
	EDRJson		jsonAck;
	EDRJson		tempJson;
	string		key = JsonField_Data;

	// Refresh 정책은 포맷이 달라서 따로 처리
	if (m_ePolicyType == CHttpAttribute::PolicyIOCRefresh
		|| m_ePolicyType == CHttpAttribute::PolicyRESRefresh)
	{
		tempJson = m_pResData->GetDataAsJson();
		CHttpData tempHttpData(tempJson[JsonField_Data]);
		key = JsonField_PolicyData;
		jsonPolicy = PerformPolicyExtrator(&tempHttpData, key);

		DPRINT_JSON(jsonPolicy);

		BOOL bRet = PerformPolicyApplier(jsonPolicy);

		if (!bRet)
		{
			LOGW << "HTTP Policy (REQ): 정책을 적용할 수 없습니다.";
			return FALSE;
		}
		// 정책 적용 결과 (ACK) 생성
		jsonAck = PerformAckGenerate(&tempHttpData, bRet, key);
	}
	else
	{
		jsonPolicy = PerformPolicyExtrator(m_pResData, key);

		DPRINT_JSON(jsonPolicy);

		BOOL bRet = PerformPolicyApplier(jsonPolicy);

		if (!bRet)
		{
			LOGW << "HTTP Policy (REQ): 정책을 적용할 수 없습니다.";
			return FALSE;
		}

		// 정책 적용 결과 (ACK) 생성
		jsonAck = PerformAckGenerate(m_pResData, bRet, key);
	}

	DPRINT_JSON(jsonAck);

	// CHttpClient 객체 리셋
	EDRDelete(m_pReqData);
	m_pReqData = EDRNew CHttpData(jsonAck);
	m_pHttpClient->Reset();
	m_pHttpClient->SetRequestData(m_pReqData);
	m_pHttpClient->ResetMethod(CHttpAttribute::Post);

	LOGI << GetCurrentName() << "HTTP URL: " << reqUrl;
	//LOGI << GetCurrentName() << "HTTP Generate ACK: \n" << jsonAck << "\n";

	// HTTP Request (ACK) 전송
	httpStatus = m_pHttpClient->SendRequest();
	m_pResData = m_pHttpClient->GetResponseData();

	// HTTP Request 결과 확인
	if (httpStatus != HTTP_OK)
	{
		/*
		LOGW_IF(m_pResData != NULL)
			<< GetCurrentName()
			<< "HTTP Response (ACK): \n"
			<< m_pResData->GetDataAsJson()
			<< "\n";

		*/
		LOGW << GetCurrentName() << "HTTP Error (ACK) Status: " << httpStatus;
		LOGW << GetCurrentName() << "HTTP Finish <<";

		m_bUsed = TRUE;

		return FALSE;
	}

	LOGI << GetCurrentName() << "HTTP Send (ACK) Status: " << httpStatus;

	// ACK 에 대한 HTTP Response 확인	
	/*
	LOGI_IF(m_pResData!=NULL)
		<< GetCurrentName()
		<< "HTTP Response (ACK): \n"
		<< m_pResData->GetDataAsJson()
		<< "\n";
	*/
	LOGI << GetCurrentName() << "HTTP Finish <<";

	m_bUsed = TRUE;

	return TRUE;
}

VOID CEDRPolicyManager::SetPolicyHelper(
	CHttpAttribute::EEDRManagerType eMangerType
	, CEDRPolicyHelperBase* pPolicyHelper
)
{
	m_pPolicyHelper[eMangerType] = pPolicyHelper;
}

EDRJson CEDRPolicyManager::PerformAckGenerate(
	CHttpData* pHttpData
	, BOOL status
	, string keyName
)
{
	EDRJson jsonRet;

	if (m_pCurrentPolicyHelper)
	{
		jsonRet = m_pCurrentPolicyHelper->GenerateAck(
			pHttpData
			, status
			, keyName
		);
	}

	return jsonRet;
}

EDRJson CEDRPolicyManager::PerformPolicyExtrator(
	CHttpData* pHttpData
	, string keyName
)
{
	EDRJson jsonRet;

	if (m_pCurrentPolicyHelper)
	{
		jsonRet = m_pCurrentPolicyHelper->ExtractPolicy(
			pHttpData
			, keyName
		);
	}

	return jsonRet;
}

BOOL CEDRPolicyManager::PerformPolicyApplier(
	EDRJson jsonPolicy
)
{
	if (m_pCurrentPolicyHelper)
	{
		return m_pCurrentPolicyHelper->ApplyPolicy(
			jsonPolicy
		);
	}

	return TRUE;
}