#pragma once

#include "CommonDef.h"
#include "EDRAgentDll.h"
#include "EDRPolicyCommander.h"
#include "EDRWorkerThreadManager.h"
#include "EDRConfig.h"

#include "HttpClient/HttpAttribute.h"
#include "Util/HashList.h"

class CEDRPolicyCommander;
class CEDRPolicyData
{
private:
	string				m_strPolicyName;
	string				m_strTypeName;
	string				m_strSubUrl;
	EEDRType			m_eType;
	vector<string>		m_strKeyItemNames;
	CHashList			*m_pHashList;
	CEDRPolicyCommander	*m_pCommander;
	BOOL				m_bInit;

public:
	CEDRPolicyData(
		EDRJson json = Json::objectValue
	);

	CEDRPolicyData(
		string TypeName
		, CEDRConfig::EEDRConfig eConfig = CEDRConfig::EEDRConfigIOC
	);

	~CEDRPolicyData();

	VOID SetCommander(
		CEDRPolicyCommander *commander
	);

	BOOL PerformCommand(
		vector<CommandData_Ptr> commandData
	);

	BOOL AppyToKernel();

	EEDRType GetEDRType();

	string GetPolicyDataName();

	string GetSubUrl();

	CHashList* GetList(
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

	string GetTypeName();

	vector<string> GetKeyName();

	VOID PrintList();

	BOOL WriteFile(
		string filename
	);
};

