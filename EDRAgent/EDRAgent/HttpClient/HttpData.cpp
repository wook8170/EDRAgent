#include "StdAfx.h"
#include "HttpData.h"
#include "Util/Util.h"

CHttpData::CHttpData()
{
	m_pData = NULL;
	m_len = 0;
	m_json = Json::objectValue;
	Clear();
}

CHttpData::CHttpData(
	EDRJson& json
)
{
	m_pData = NULL;
	m_len = 0;
	m_json = Json::objectValue;

	Clear();
	AddData(json);
}

CHttpData::CHttpData(
	VOID* pData
	, size_t size
	, size_t nmemb
)
{
	m_pData = NULL;
	m_len = 0;
	m_json = Json::objectValue;

	Clear();
	AddData(pData, size, nmemb);
}

CHttpData::~CHttpData()
{
	EDRFree(m_pData);
	m_json.clear();
}

VOID CHttpData::Clear()
{
	if (!m_json.empty())
	{
		m_json.clear();
	}

	EDRFree(m_pData);

	m_len = 0;
	m_pData = (CHAR*)EDRAlloc(m_len + 1);

	CHECK_NOTNULL(m_pData);

	m_pData[0] = CharNull;
}

size_t CHttpData::GetLength()
{
	return m_len;
}

EDRJson CHttpData::GetDataAsJson()
{
	if (!m_json.empty())
	{
		m_json.clear();
	}

	if (!m_pData)
	{
		return m_json;
	}

	BOOL parsingRet = m_jsonReader.parse(m_pData, m_json);

	LOGE_IF(!parsingRet)
		<< "Json 파일 파싱 실패: "
		<< m_jsonReader.getFormattedErrorMessages();

	return m_json;
}

CHAR* CHttpData::GetDataAsRaw()
{
	return m_pData;
}

size_t CHttpData::AddData(
	EDRJson& json
)
{
	string  outData = m_jsonWriter.write(json);

	return AddData((VOID*)outData.c_str(), 1, outData.length());
}

size_t CHttpData::AddData(
	VOID* pData
	, size_t size
	, size_t nmemb
)
{
	size_t new_len = m_len + (size * nmemb);
	m_pData = (CHAR*)EDRRealloc((VOID*)m_pData, new_len + 1);

	CHECK_NOTNULL(m_pData);

	EDRCopyMemory(m_pData + m_len, pData, size * nmemb);

	m_pData[new_len] = CharNull;
	m_len = new_len;

	m_json.clear();

	BOOL parsingRet = m_jsonReader.parse(m_pData, m_json);

	LOGE_IF(!parsingRet)
		<< "Json 파일 파싱 실패: \n"
		<< m_pData << "\n"
		<< m_jsonReader.getFormattedErrorMessages();

	//LOGI << "데이터: \n" << m_json;

	return size * nmemb;
}