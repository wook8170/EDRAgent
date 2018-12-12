#pragma once

#include "uthash/uthash.h"
#include "jsoncpp/json.h"
//#include "HttpClient/HttpAttribute.h"

typedef struct
{
	CHAR*			key;
	EDRJson			json;
	UT_hash_handle	hh;
} IOCNRESList_T;

class CHashList
{
private:
	IOCNRESList_T * m_hashList;
	string			m_strTypeName;
	vector<string>	m_strKeyNames;
	BOOL			m_bInit;

public:
	CHashList();

	CHashList(
		string keyName
		, string typeFieldName
	);

	CHashList(
		vector<string> keyNames
		, string typeFieldName
	);

	~CHashList();

	BOOL Init(
		string keyName
		, string typeFieldName
	);

	BOOL Init(
		vector<string> keyNames
		, string typeFieldName
	);

	DWORD Count();

	BOOL InsertNode(
		EDRJson item
	);

	EDRJson FindNode(
		string findKey
	);

	BOOL DeleteNode(
		string findKey
	);

	VOID PrintList(
	);

	BOOL WriteFile(
		string filename
	);

	string GetRealKey(
		EDRJson json
		, string defaultKey = "fileKey"
	);
};

