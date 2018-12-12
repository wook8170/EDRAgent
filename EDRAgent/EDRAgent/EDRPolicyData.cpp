#include "stdafx.h"
#include "EDRPolicyData.h"
#include "EDRAgentDll.h"
#include "EDRWorkerThreadManager.h"
#include "EDRPolicyCommander.h"
#include "Util/Util.h"
#include "Util/FileSystem.h"

CEDRPolicyData::CEDRPolicyData(
	EDRJson json
)
{
	m_strSubUrl		= json[JsonField_SubUrl].asString();
	m_strPolicyName	= json[JsonField_Name].asString();
	m_strTypeName	= json[JsonField_Type].asString();
	m_eType			= (EEDRType)json[JsonField_Code].asInt();

	m_pCommander = NULL;

	EDRJson jsonKeyNames = json[JsonField_Key];

	if (!json[JsonField_UsePolicy].empty() && JsonField_UsePolicy)
	{
		if (jsonKeyNames.isString())
		{
			m_strKeyItemNames.push_back(jsonKeyNames.asString());
			m_pHashList = EDRNew CHashList(jsonKeyNames.asString(), m_strTypeName);
		}
		else
		{
			for (auto it = jsonKeyNames.begin(); it != jsonKeyNames.end(); it++)
			{
				if (it->isString())
				{
					m_strKeyItemNames.push_back((*it).asString());
				}
			}

			m_pHashList = EDRNew CHashList(m_strKeyItemNames, m_strTypeName);
		}
	}
}

CEDRPolicyData::CEDRPolicyData(
	string TypeName
	, CEDRConfig::EEDRConfig eConfig
)
{
	m_strTypeName			= TypeName;
	m_strSubUrl				= CEDRConfig::Instance()->GetSubURL(TypeName, eConfig);
	m_strPolicyName			= CEDRConfig::Instance()->GetName(TypeName, eConfig);
	m_eType					= (EEDRType)CEDRConfig::Instance()->GetCode(TypeName, eConfig);
	vector<string> vecKeys	= CEDRConfig::Instance()->GetKeys(TypeName, eConfig);
	
	m_pCommander = NULL;

	m_strKeyItemNames = vecKeys;
	m_pHashList = EDRNew CHashList(vecKeys, m_strTypeName);

}
CEDRPolicyData::~CEDRPolicyData()
{
	EDRDelete(m_pHashList);
}

VOID CEDRPolicyData::SetCommander(
	CEDRPolicyCommander *commander
)
{
	m_pCommander = commander;
}

BOOL CEDRPolicyData::PerformCommand(
	vector<CommandData_Ptr> commandData
)
{
	if (m_pCommander)
	{
		return m_pCommander->DoCommand(m_strTypeName, commandData);
	}

	return TRUE;
}

BOOL CEDRPolicyData::AppyToKernel()
{
	string filename = ("KPOL.IOC.");
	string typeName = GetTypeName();

	filename = filename.append(typeName);

	if (WriteFile(filename))
	{
		File file(filename);
		if (!file.Open(File::in, File::text))
		{
			LOGW
				<< "파일 열기를 실패하였습니다. ("
				<< filename
				<< ")";

			return FALSE;
		}

		string strKenelPolicy = file.ReadAll(TRUE);
		INT type = (INT)GetEDRType();
		INT size = (INT)strKenelPolicy.length();

		LOGI << "** " << filename;
		LOGI << endl << strKenelPolicy;
		LOGW_IF(!SafePCKModule::Add(type, (CHAR*)strKenelPolicy.c_str(), size))
			<< "커널 정책 적용을 실패 하였습니다. ( SafePCKModule::Add() ) : ( "
			<< GetPolicyDataName() << ", " << GetTypeName()
			<< " )";
	}

	File::Remove(filename);

	return TRUE;
}

EEDRType CEDRPolicyData::GetEDRType()
{
	return m_eType;
}

string CEDRPolicyData::GetPolicyDataName()
{
	return m_strPolicyName;
}

string CEDRPolicyData::GetSubUrl()
{
	return m_strSubUrl;
}

CHashList* CEDRPolicyData::GetList()
{
	return m_pHashList;
}


DWORD CEDRPolicyData::Count()
{
	RETURN_FAIL_IF(m_pHashList == NULL, 0);

	return m_pHashList->Count();
}

BOOL CEDRPolicyData::InsertNode(
	EDRJson item
)
{
	RETURN_FAIL_IF(m_pHashList == NULL, FALSE);

	return m_pHashList->InsertNode(item);
}

EDRJson CEDRPolicyData::FindNode(
	string findKey
)
{
	EDRJson json;

	RETURN_FAIL_IF(m_pHashList == NULL, json);

	json = m_pHashList->FindNode(findKey);

	return json;
}

BOOL CEDRPolicyData::DeleteNode(
	string findKey
)
{
	RETURN_FAIL_IF(m_pHashList == NULL, FALSE);

	return m_pHashList->DeleteNode(findKey);
}

string CEDRPolicyData::GetTypeName()
{
	return m_strTypeName;
}

vector<string> CEDRPolicyData::GetKeyName()
{
	return m_strKeyItemNames;
}

VOID CEDRPolicyData::PrintList()
{
	RETURN_V_FAIL_IF(m_pHashList == NULL);

	m_pHashList->PrintList();
}

BOOL CEDRPolicyData::WriteFile(string filename)
{
	RETURN_FAIL_IF(m_pHashList == NULL, FALSE);

	return m_pHashList->WriteFile(filename);
}
