#include "StdAfx.h"
#include "HttpClient.h"
#include "HttpAttribute.h"
#include "HttpData.h"
#include "Util/Util.h"

CHttpClient::CHttpClient()
{
	m_pContext = NULL;
	m_pReqBody = NULL;
	m_pResBody = NULL;

	m_bUsed = FALSE;
}

CHttpClient::~CHttpClient()
{
	EDRDelete(m_pResBody);
}

size_t CHttpClient::WriteCallback(
	VOID *ptr
	, size_t size
	, size_t nmemb
	, VOID *pHttpData
)
{
	CHttpData* responseData = (CHttpData*)pHttpData;

	CHECK_NOTNULL(responseData);

	responseData->AddData(ptr, size, nmemb);

	LOGI << size * nmemb << " bytes 수신";

	return size * nmemb;
}

VOID CHttpClient::ResetContext(
	CHttpContext* pContext
)
{
	LOGW_IF(m_bUsed) << "객체 리셋이 필요합니다. Reset() 함수룰 호출해야 합니다.";
	m_pContext = pContext;
}

VOID CHttpClient::ResetURL(
	string  url
)
{
	LOGW_IF(m_bUsed) << "객체 리셋이 필요합니다. Reset() 함수룰 호출해야 합니다.";
	m_strURL = url;
}

VOID CHttpClient::ResetMethod(
	CHttpAttribute::EMethod method
)
{
	LOGW_IF(m_bUsed) << "객체 리셋이 필요합니다. Reset() 함수룰 호출해야 합니다.";
	m_eMethod = method;
}

VOID CHttpClient::ResetContentType(
	string  contentType
)
{
	LOGW_IF(m_bUsed) << "객체 리셋이 필요합니다. Reset() 함수룰 호출해야 합니다.";
	m_strContentType = contentType;
}

VOID CHttpClient::SetRequestData(
	CHttpData* pData
)
{
	LOGW_IF(m_bUsed) << "객체 리셋이 필요합니다. Reset() 함수룰 호출해야 합니다.";
	m_pReqBody = pData;
}

VOID CHttpClient::Reset()
{
	EDRDelete(m_pResBody);

	m_bUsed = FALSE;
}

CHttpData* CHttpClient::GetResponseData()
{
	return m_pResBody;
}


HTTP_STATUS CHttpClient::SendRequest()
{
	CHECK_NOTNULL(m_pContext);
	CHECK(m_eMethod != CHttpAttribute::MethodInvalid);
	CHECK(!m_strURL.empty());

	if (m_bUsed)
	{
		LOGW_IF(m_bUsed)
			<< "객체 리셋이 필요합니다. Reset() 함수룰 호출해야 합니다.";
		return HTTP_ERROR_CLIENT;
	}

	CURLcode	res;
	CString		strError = _T(StringNull);
	CHAR		szError[CURL_ERROR_SIZE] = { NULL, };
	Context*	pContext = m_pContext->GetContext();

	CHECK_NOTNULL(pContext);

	// Reset Response data
	EDRDelete(m_pResBody);
	m_pResBody = EDRNew CHttpData();
	CHECK_NOTNULL(m_pResBody);

	CHAR*	utf8Data = NULL;

	LOGD << "Request Url: " << m_strURL;
	LOGD << "Request Method: " << ((m_eMethod == CHttpAttribute::Post) ?
		"Post" :
		(m_eMethod == CHttpAttribute::Get) ? "Get" :
		(m_eMethod == CHttpAttribute::Delete) ? "Delete" :
		"Invalid");

	curl_easy_reset(pContext);

	if (m_eMethod == CHttpAttribute::Post)
	{
		struct curl_slist *headerlist = NULL;
		headerlist = curl_slist_append(headerlist, "Content-Type: application/json");

		CHECK_NOTNULL(headerlist);
		CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_POST, 1L));
		CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_HTTPHEADER, headerlist));

		if (m_pReqBody)
		{
			LOGD << "Request Body: \n" << m_pReqBody->GetDataAsJson().toStyledString();
			utf8Data = CUtil::AnsiToUTF8(m_pReqBody->GetDataAsRaw());
			CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_POSTFIELDS, (VOID*)utf8Data));
		}
	}
	else if (m_eMethod == CHttpAttribute::Delete)
	{
		struct curl_slist *headerlist = NULL;
		headerlist = curl_slist_append(headerlist, "Content-Type: application/json");

		EDRJson			jsonResetData;
		EDRJson			jsonApplyList;
		EDRJsonWriter	jsonWriter;

		jsonResetData[JsonField_ApplyTime] = CUtil::CurrentDateTime();
		jsonResetData[JsonFiled_ApplyList] = Json::arrayValue;

		CHttpData httpData(jsonResetData);
	
		LOGD << "Request Body: \n" << httpData.GetDataAsJson().toStyledString();
		
		utf8Data = CUtil::AnsiToUTF8(httpData.GetDataAsRaw());
		CHECK_NOTNULL(headerlist);
		CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_CUSTOMREQUEST, "DELETE"));
		CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_POSTFIELDS, (CHAR*)utf8Data));
		CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_HTTPHEADER, headerlist));
	}

	CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_TIMEOUT, 10));
	CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_URL, m_strURL.c_str()));
	CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0));
	CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_FOLLOWLOCATION, 1L));
	CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_NOSIGNAL, 1));
	CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_ERRORBUFFER, szError));
	CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_SSL_VERIFYPEER, FALSE));
	CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_WRITEFUNCTION, CHttpClient::WriteCallback));
	CHECK(CURLE_OK == curl_easy_setopt(pContext, CURLOPT_WRITEDATA, m_pResBody));

	res = curl_easy_perform(pContext);

	m_bUsed = TRUE;

	//LOGD_IF(m_pResBody != NULL) << "Response Body: \n" << m_pResBody->GetDataAsJson();

	EDRDelete(utf8Data);

	if (res != CURLE_OK)
	{
		EDRDelete(m_pResBody);
		LOGW
			<< "Request() 실패 - curl_easy_perform() : "
			<< curl_easy_strerror(res);

		return HTTP_ERROR_TIMEOUT;
	}

	HTTP_STATUS httpCode = HTTP_OK;
	curl_easy_getinfo(pContext, CURLINFO_RESPONSE_CODE, &httpCode);

	return httpCode;
}

VOID CHttpClient::Clear()
{
	m_pContext;

	m_strURL.clear();
	m_strURL.clear();
	m_strContentType.clear();

	EDRDelete(m_pResBody);
}
