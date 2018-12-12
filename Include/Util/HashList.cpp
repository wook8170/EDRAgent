#include "StdAfx.h"
#include "Util/HashList.h"
#include "Util/Util.h"
#include "EDRPolicyData.h"
#include "FileSystem.h"
#include "Util/Registry.h"

using namespace FileSystem;


CHashList::CHashList()
{
	m_bInit = FALSE;
}

CHashList::CHashList(
	string keyName
	, string typeFieldName
)
{
	m_bInit = FALSE;
	Init(keyName, typeFieldName);
}

CHashList::CHashList(
	vector<string> keyName
	, string typeFieldName
)
{
	m_bInit = FALSE;
	Init(keyName, typeFieldName);
}
CHashList::~CHashList()
{
	IOCNRESList_T *j = NULL;
	for (j = m_hashList; j != NULL; j = (IOCNRESList_T*)j->hh.next)
	{
		HASH_DEL(m_hashList, j);
	}
	/*
	LOGD
		<< "리스트가 삭제 되었습니다. ("
		<< m_strTypeName
		<< ")";
	*/
}

BOOL CHashList::Init(string keyName, string typeFieldName)
{
	if (m_bInit)
	{
		LOGW << "이미 초기화가 되었습니다.";
		return FALSE;
	}
	m_strKeyNames.push_back(keyName);
	m_strTypeName = typeFieldName;
	m_hashList = NULL;
	m_bInit = TRUE;

	return TRUE;
}

BOOL CHashList::Init(vector<string> keyNames, string typeFieldName)
{
	if (m_bInit)
	{
		LOGW << "이미 초기화가 되었습니다.";
		return FALSE;
	}
	m_strKeyNames = keyNames;
	m_strTypeName = typeFieldName;
	m_hashList = NULL;
	m_bInit = TRUE;

	return TRUE;
}

DWORD CHashList::Count()
{
	CHECK(!m_strTypeName.empty());
	CHECK(!m_strKeyNames.empty());

	return HASH_COUNT(m_hashList);
}

BOOL CHashList::InsertNode(
	EDRJson item
)
{
	CHECK(!m_strTypeName.empty());
	CHECK(!m_strKeyNames.empty());

	string Upperkey = GetRealKey(item);

	IOCNRESList_T *newItem = (IOCNRESList_T *)EDRAlloc(sizeof(IOCNRESList_T));
	CHECK_NOTNULL(newItem);
	EDRZeroMemory(newItem, sizeof(IOCNRESList_T));

	size_t keyLen = Upperkey.length();
	newItem->key = (CHAR *)EDRAlloc(keyLen + 1);
	CHECK_NOTNULL(newItem->key);
	EDRZeroMemory(newItem->key, (keyLen + 1));
	EDRCopyMemory(newItem->key, Upperkey.c_str(), keyLen);
	newItem->json = item;

	HASH_ADD_STR(m_hashList, key, newItem);
	/*
	LOGD
		<< "추가 : "
		<< "( "
		<< m_strTypeName
		<< " ) "
		<< newItem->key;
	*/

	return TRUE;
}

EDRJson CHashList::FindNode(
	string findKey
)
{
	CHECK(!m_strTypeName.empty());
	CHECK(!m_strKeyNames.empty());

	EDRJson json;
	string Upperkey = findKey.c_str();
	CUtil::Upper(Upperkey);

	if (!m_hashList)
	{
		return json;
	}

	IOCNRESList_T *foundItem = NULL;

	HASH_FIND_STR(m_hashList, Upperkey.c_str(), foundItem);

	if (foundItem != NULL)
	{
		json = foundItem->json;
		/*
		LOGD
			<< "발견 : \n"
			<< Upperkey;//foundItem->json;
		*/
	}

	return json;
}

BOOL CHashList::DeleteNode(
	string findKey
)
{
	CHECK(!m_strTypeName.empty());
	CHECK(!m_strKeyNames.empty());

	string Upperkey = findKey.c_str();
	CUtil::Upper(Upperkey);
	IOCNRESList_T *foundItem = NULL;

	HASH_FIND_STR(m_hashList, Upperkey.c_str(), foundItem);

	if (foundItem != NULL)
	{
		/*
		LOGD
			<< "삭제 : \n"
			<< Upperkey;//foundItem->json;
		*/
		HASH_DEL(m_hashList, foundItem);
		EDRFree(foundItem);
		return TRUE;
	}

	LOGD
		<< "삭제 실패: "
		<< findKey;

	return FALSE;
}

VOID CHashList::PrintList()
{
	CHECK(!m_strTypeName.empty());
	CHECK(!m_strKeyNames.empty());

	IOCNRESList_T *pList, *pItem = NULL;
	pList = m_hashList;

	INT i = 0;
	for (pItem = pList; pItem != NULL; pItem = (IOCNRESList_T*)(pItem->hh.next))
	{
		///*
		LOGD
			<< " - [" << i++ << "] "
			<< pItem->key;
		//*/
	}
}

BOOL CHashList::WriteFile(string filename)
{
	CHECK(!m_strTypeName.empty());
	CHECK(!m_strKeyNames.empty());

	IOCNRESList_T *pList, *pItem = NULL;
	pList = m_hashList;

	FileSystem::File file(filename);

	CHECK(file.Open(File::out, File::text));

	for (pItem = pList; pItem != NULL; pItem = (IOCNRESList_T*)(pItem->hh.next))
	{
		// 타입이 레지스트리일 경우, 커널에서 인식 가능한 RAW 포맷으로 변경해 주어야 함
		if (CUtil::Compare(m_strTypeName, Type_Registry) == 0)
		{
			file.PutLine(CRegistry::GetRawPath(pItem->key), TRUE);
		}
		else
		{
			file.PutLine(pItem->key, TRUE);
		}		
	}

	file.flush();
	file.Close();

	return TRUE;
}

string CHashList::GetRealKey(
	EDRJson json
	, string defaultKey

)
{
	DPRINT_JSON(json);

	string key = StringNull;
	string val = json.toStyledString();

	for (auto it = m_strKeyNames.begin(); it != m_strKeyNames.end(); it++)
	{
		/*
		LOGD << "필드 명: " << (*it);
		*/
		if (CUtil::PatternMatch((*it), "*/*"))
		{
			string delimeter = "/";
			vector<string> strItem = CUtil::Split((*it), delimeter);
			key = json[strItem[0]][strItem[1]].asString();
		}
		else
		{
			key = json[(*it)].asString();
		}

		if (!key.empty())
		{
			break;
		}
	}

	if (key.empty())
	{
		key = json[defaultKey].asString();
	}

	CUtil::Upper(key);

	return key;
}
