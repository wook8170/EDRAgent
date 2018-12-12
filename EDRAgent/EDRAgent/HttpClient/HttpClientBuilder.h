#pragma once

#include "HttpContext.h"
#include "HttpClient.h"
#include "HttpAttribute.h"

/****************************************
Http Client Builder ( Must use bulder to create http client )
****************************************/

class CHttpClient;
class CHttpClientBuilder
{
private:
	CHttpClient * m_pPerformer;
	CHttpContext*				m_pContext;
	string 						m_strURL;
	string 						m_strContentType;
	CHttpAttribute::EMethod		m_eMethod;

public:
	CHttpClientBuilder(
		CHttpContext* pHttpContext
	);

	~CHttpClientBuilder();


	CHttpClientBuilder*	SetMethod(
		CHttpAttribute::EMethod method
	);

	CHttpClientBuilder*	SetContentType(
		string  contentType
	);

	CHttpClientBuilder*	SetURL(
		string  url
	);

	CHttpClient* Build();
};

