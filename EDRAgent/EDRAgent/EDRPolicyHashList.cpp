#include "stdafx.h"
#include "EDRPolicyHashList.h"
#include "Util/Util.h"

BOOL CEDRPolicyHashList::CreateList(
	string keyName
	, string typeName
)
{
	if (GetList(typeName) == NULL)
	{
		CHashList *pHashList = EDRNew CHashList(keyName, typeName);
		m_map[typeName] = pHashList;
		return TRUE;
	}
	else
	{
		LOGW 
			<< "동일한 타입 값으로 리스트가 존재합니다 ("
			<< typeName
			<< ")";
		return FALSE;
	}
}

BOOL CEDRPolicyHashList::RemoveList(
	string typeName
)
{
	map<string, CHashList*>::iterator it;
	it = m_map.find(typeName);

	if (it == m_map.end())
	{
		LOGW 
			<< "동일한 타입 값으로 리스트가 존재합니다 ("
			<< typeName
			<< ")";
		return FALSE;
	}
	m_map.erase(it);

	return TRUE;
}

CHashList* CEDRPolicyHashList::GetList(
	string typeName
)
{
	map<string, CHashList*>::iterator it;
	it = m_map.find(typeName);

	if (it == m_map.end())
	{
		LOGW 
			<< "해당 타입 값으로 리스트가 존재하지 않습니다 ("
			<< typeName
			<< ")";
		return NULL;
	}
	return (CHashList*)it->second;
}

CEDRPolicyHashList::CEDRPolicyHashList()
{
}

CEDRPolicyHashList* CEDRPolicyHashList::GetInstance()
{
	static CEDRPolicyHashList instance;
	return &instance;
}

DWORD CEDRPolicyHashList::Count(
	string typeName
)
{
	CHashList* pHashList = GetList(typeName);
	
	if (pHashList)
	{
		return pHashList->Count();
	}
	return 0;
}

BOOL CEDRPolicyHashList::InsertNode(
	string typeName
	, EDRJson item
)
{
	CHashList* pHashList = GetList(typeName);

	if (pHashList)
	{
		return pHashList->InsertNode(item);;
	}

	return FALSE;
}

EDRJson CEDRPolicyHashList::FindNode(
	string keyName
	, string findKey
)
{
	EDRJson json;

	CHashList* pHashList = GetList(keyName);
	if (pHashList)
	{
		json  = pHashList->FindNode(findKey);
	}
	
	return json;
}

BOOL CEDRPolicyHashList::DeleteNode(
	string keyName
	, string findKey
)
{
	CHashList* pHashList = GetList(keyName);
	
	if (pHashList)
	{
		return pHashList->DeleteNode(findKey);
	}

	return FALSE;
}

string CEDRPolicyHashList::GetTypeName(
	string typeName
)
{
	CHashList* pHashList = GetList(typeName);
	
	if (pHashList)
	{
		return "";//pHashList->GetTypeName();
	}

	return "";
}

string CEDRPolicyHashList::GetKeyName(
	string typeName
)
{
	CHashList* pHashList = GetList(typeName);	

	if (pHashList)
	{
		return "";//pHashList->GetKeyName();
	}

	return "";
}

BOOL CEDRPolicyHashList::Exist(
	string typeName
)
{
	CHashList* pHashList = GetList(typeName);

	return (pHashList) ? TRUE : FALSE;
}

VOID CEDRPolicyHashList::PrintList(
	string typeName
)
{
	CHashList* pHashList = GetList(typeName);
	
	if (pHashList)
	{
		pHashList->PrintList();
	}	
}

BOOL CEDRPolicyHashList::WriteFile(string typeName, string filename)
{
	CHashList* pHashList = GetList(typeName);
	
	if (pHashList)
	{
		return pHashList->WriteFile(filename);
	}

	return FALSE;	
}

string CEDRPolicyHashList::GetRealKey(EDRJson json, string typeName, string keyName)
{
	// 실시간 디버깅 용 
	string strVal = json.toStyledString();

	if (CUtil::Compare(typeName, "file") == 0 
		|| CUtil::Compare(typeName, "process") == 0)
	{
		if (json[keyName.c_str()].isObject())
		{
			string Upperkey = json[keyName.c_str()]["SHA256"].asString().c_str();
			CUtil::Upper(Upperkey);
			return Upperkey;
		}
	}

	string Upperkey = json[keyName.c_str()].asString().c_str();
	CUtil::Upper(Upperkey);

	return Upperkey;
}
