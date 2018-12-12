#pragma once

#include "HttpContext.h"
#include "HttpData.h"
#include "HttpAttribute.h"

/****************************************
Http Client
****************************************/

class CHttpClientBuilder;
class CHttpAckGenerator;
class CPolicyExtractor;

class CHttpClient
{
	friend CHttpClientBuilder;
protected:
	CHttpContext * m_pContext;
	BOOL						m_bUsed;
	string 						m_strURL;
	string						m_strContentType;
	CHttpAttribute::EMethod		m_eMethod;

	CHttpData *		m_pReqBody;
	CHttpData *		m_pResBody;

	static size_t	WriteCallback(
		VOID *ptr
		, size_t size
		, size_t nmemb
		, VOID *pHttpData
	);

	CHttpClient();

public:
	VOID ResetContext(
		CHttpContext* pContext
	);

	VOID ResetURL(
		string  url
	);

	VOID ResetMethod(
		CHttpAttribute::EMethod method
	);

	VOID ResetContentType(
		string  contentType
	);


	VOID SetRequestData(
		CHttpData* pData
	);

	VOID Reset();

	CHttpData * GetResponseData();
	HTTP_STATUS SendRequest();
	VOID Clear();

public:
	~CHttpClient();

};