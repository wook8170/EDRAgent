#include "StdAfx.h"
#include "HttpClientBuilder.h"

CHttpClientBuilder::CHttpClientBuilder(
	CHttpContext* pHttpContext
)
{
	m_pPerformer = NULL;

	CHECK_NOTNULL(pHttpContext);

	m_pContext = pHttpContext;
}

CHttpClientBuilder::~CHttpClientBuilder()
{
}

CHttpClientBuilder* CHttpClientBuilder::SetMethod(
	CHttpAttribute::EMethod method
)
{
	m_eMethod = method;

	return this;
}

CHttpClientBuilder* CHttpClientBuilder::SetContentType(
	string  contentType
)
{
	m_strContentType = contentType.c_str();

	return this;
}

CHttpClientBuilder* CHttpClientBuilder::SetURL(
	string  url
)
{
	m_strURL = url.c_str();

	return this;
}

CHttpClient* CHttpClientBuilder::Build()
{
	LOGE_IF(!m_pContext) << "객체 생성 실패";
	LOGE_IF(m_strURL.empty()) << "객체 생성 실패";

	m_pPerformer = EDRNew CHttpClient();

	CHECK_NOTNULL(m_pPerformer);

	m_pPerformer->ResetContext(m_pContext);
	m_pPerformer->ResetMethod(m_eMethod);
	m_pPerformer->ResetURL(m_strURL);
	m_pPerformer->ResetContentType(m_strContentType);

	return m_pPerformer;
}
