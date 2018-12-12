#pragma once

#include "CommonDef.h"

#include "jsoncpp/json.h"
#include "HttpClient/HttpAttribute.h"
#include "Util/HashList.h"
#include <map>

class CEDRPolicyHashList
{
private:
	map<string, CHashList*>	m_map;
	CEDRPolicyHashList();

public:
	static CEDRPolicyHashList* GetInstance();

	BOOL CreateList(
		string keyName
		, string typeName
	);

	BOOL RemoveList(
		string typeName
	);

	CHashList* GetList(
		string typeName
	);

	DWORD Count(
		string typeName
	);

	BOOL InsertNode(
		string typeName
		, EDRJson item
	);

	EDRJson FindNode(
		string keytypeName
		, string findKey
	);

	BOOL DeleteNode(
		string typeName
		, string findKey
	);

	string GetTypeName(
		string typeName
	);

	string GetKeyName(
		string typeName
	);

	BOOL Exist(
		string typeName
	);

	VOID PrintList(
		string typeName
	);

	BOOL WriteFile(
		string typeName
		, string filename
	);

	string GetRealKey(
		EDRJson json
		, string typeName
		, string keyName
	);
};

