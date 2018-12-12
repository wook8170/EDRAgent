#pragma once

#include "CommonDef.h"
#include "EDRPolicyHelper.h"
#include "EDRPolicyData.h"
#include "EDRPolicyCommander.h"
#include "EDRConfig.h"
#include "HttpClient/HttpData.h"
#include "Util/Util.h"
#include "Util/FileSystem.h"
#include <map>

/****************************************
Implementation of EDR Policy Helper Base
****************************************/
class CEDRPolicyHelperBase : public CEDRPolicyHelper
{
protected:
	map<string, CEDRPolicyData*>	m_map;
	string							m_strPolicyMetaName;
	string							m_strNormalUrl;
	string							m_strRefreshUrl;
	BOOL							m_bInit;
	CEDRPolicyCommander*			m_pPolicyCommander;

	CEDRPolicyData* GetPolicyData(
		string typeName
	);

	BOOL AddPolicyData(
		CEDRPolicyData	*pEDRPolicyData
	);

public:
	CEDRPolicyHelperBase();

	~CEDRPolicyHelperBase();

	void Init(
		CEDRConfig::EEDRConfig eConfig = CEDRConfig::EEDRConfigIOC
	);

	EEDRType GetEDRTypeFromTypeString(
		string type
	);

	BOOL IsInit();

	VOID PrintPolicyData();

	string GetName();

	/*
	string GetNormalUrl();

	string GetRefreshUrl();

	string GetSubUrl(
		string typeName
	);
	*/

	BOOL BackupFile(
		string filename
	);

	BOOL SavePolicyFile(
		EDRJson json
	);

	BOOL LoadPolicyFile();

	BOOL ApplyToKernelDriver();

	BOOL ClearPolicyFile();

	virtual EDRJson	GenerateAck(
		CHttpData* pHttpData
		, BOOL status
		, string keyName = JsonField_Data
	) override;

	virtual EDRJson	ExtractPolicy(
		CHttpData* pHttpData
		, string keyName = JsonField_Data
	) override;

	virtual BOOL ApplyPolicy(
		EDRJson jsonPolicy
		, BOOL isUpdated = TRUE
	) override;
};
