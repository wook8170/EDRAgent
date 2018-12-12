#include "StdAfx.h"
#include "HttpContext.h"

CHttpContext::CHttpContext()
{
	curl_global_init(CURL_GLOBAL_ALL);
	m_pContext = curl_easy_init();

	CHECK_NOTNULL(m_pContext);
}

CHttpContext::~CHttpContext()
{
	if (m_pContext)
	{
		curl_easy_cleanup(m_pContext);
		curl_global_cleanup();

		m_pContext = NULL;
	}
}

Context* CHttpContext::GetContext()
{
	return m_pContext;
}