#pragma once

/****************************************
Http Json Data ( for Request, Response )
****************************************/
class CHttpData
{
public:
	typedef enum
	{
		RAW = 0,
		JSON,
	} EDataType;

private:
	size_t			m_len;
	CHAR *			m_pData;
	EDRJson			m_json;

	EDataType		m_eDataType;

	EDRJsonReader	m_jsonReader;
	EDRJsonWriter	m_jsonWriter;

public:
	CHttpData();

	CHttpData(
		EDRJson& json
	);

	CHttpData(
		VOID* pData
		, size_t size
		, size_t nmemb
	);

	~CHttpData();


	VOID Clear();


	size_t	GetLength();

	EDRJson	GetDataAsJson();

	CHAR *	GetDataAsRaw();

	size_t	AddData(
		EDRJson& json
	);

	size_t	AddData(
		VOID* pData
		, size_t size
		, size_t nmemb
	);
};