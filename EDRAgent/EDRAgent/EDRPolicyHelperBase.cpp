#include "StdAfx.h"
#include "CommonDef.h"
#include "EDRAgent.h"
#include "EDRPolicyHelperBase.h"
#include "EDRPolicyData.h"
#include "EDRConfig.h"
#include "EDRKernelHandler.h"

#include "HttpClient/HttpData.h"
#include "Util/Util.h"

using namespace FileSystem;

extern CEDRAgentApp theApp;

CEDRPolicyHelperBase::CEDRPolicyHelperBase()
{
	m_bInit = FALSE;
	m_pPolicyCommander = NULL;
}

CEDRPolicyHelperBase::~CEDRPolicyHelperBase()
{
	EDRDelete(m_pPolicyCommander);

	for (auto it = m_map.begin(); it != m_map.end(); it++)
	{
		CEDRPolicyData *pEDRPolicyData = (CEDRPolicyData*)((*it).second);

		EDRDelete(pEDRPolicyData);
	}
};

void CEDRPolicyHelperBase::Init(
	CEDRConfig::EEDRConfig eConfig
)
{
	m_strPolicyMetaName = CEDRConfig::Instance()->GetHelperName(eConfig);
	m_strNormalUrl = CEDRConfig::Instance()->GetNormalURL(eConfig);
	m_strRefreshUrl = CEDRConfig::Instance()->GetRefreshURL(eConfig);

	EDRJson items = CEDRConfig::Instance()->GetRootItem(eConfig);

	m_pPolicyCommander = EDRNew CEDRPolicyCommander();

	vector<string> types = CEDRConfig::Instance()->GetTypes(eConfig);

	for (auto it = types.begin(); it != types.end(); it++)
	{
		CEDRConfig *pEDRConfig = CEDRConfig::Instance();
		string typeName = (*it);
		INT	code = CEDRConfig::Instance()->GetCode(typeName, eConfig);

		if (!CEDRConfig::Instance()->UsePolicy(typeName, eConfig))
		{
			LOGI
				<< "!! (X) " << m_strPolicyMetaName
				<< "의 " << CEDRConfig::Instance()->GetName(typeName, eConfig)
				<< " - " << typeName
				<< " 은 설정에 의해 사용하지 않습니다.";

			continue;
		}
		else
		{
			LOGI
				<< "!! (O) " << m_strPolicyMetaName
				<< "의 " << CEDRConfig::Instance()->GetName(typeName, eConfig)
				<< " - " << typeName
				<< " 정책을 사용합니다.";

			CEDRPolicyData* pPolicyData = EDRNew CEDRPolicyData(typeName, eConfig);
			pPolicyData->SetCommander(m_pPolicyCommander);
			AddPolicyData(pPolicyData);

			BOOL bUsePollingMethod = CEDRConfig::Instance()->UsePollingMethod4Log((EEDRType)code);
			CEDRKernelHandler *pKernelHandler = EDRNew CEDRKernelHandler(typeName, (EEDRType)code, bUsePollingMethod);
		}		
	}

	m_bInit = TRUE;
}

EEDRType CEDRPolicyHelperBase::GetEDRTypeFromTypeString(string type)
{
	if (CUtil::Compare(type, Type_Process) == 0)
	{
		return EDR_TYPE_PROCESS;
	}
	else if (CUtil::Compare(type, Type_File) == 0)
	{
		return EDR_TYPE_FILE;
	}
	else if (CUtil::Compare(type, Type_Registry) == 0)
	{
		return EDR_TYPE_REG;
	}
	else if (CUtil::Compare(type, Type_IP) == 0)
	{
		return EDR_TYPE_IP;
	}
	else if (CUtil::Compare(type, Type_URL) == 0
		|| CUtil::Compare(type, Type_Domain) == 0)
	{
		return EDR_TYPE_URL;
	}

	return EDR_TYPE_INVALID;
}

BOOL CEDRPolicyHelperBase::IsInit()
{
	return m_bInit;
}

VOID  CEDRPolicyHelperBase::PrintPolicyData()
{
	for (auto it = m_map.begin(); it != m_map.end(); it++)
	{
		CEDRPolicyData *pEDRPolicyData = (CEDRPolicyData*)((*it).second);

		LOGD << "==================";
		LOGD << "PolicyDataName: " << pEDRPolicyData->GetPolicyDataName();
		LOGD << "TypeName      : " << pEDRPolicyData->GetTypeName();

		vector<string>	keyNames = pEDRPolicyData->GetKeyName();
		for (auto it = keyNames.begin(); it != keyNames.end(); it++)
		{
			LOGD << "KeyName       : " << (*it);
		}
		pEDRPolicyData->PrintList();

		LOGD << " ";

	}
}

string CEDRPolicyHelperBase::GetName()
{
	return m_strPolicyMetaName;
}

/*
string CEDRPolicyHelperBase::GetNormalUrl()
{
	return m_strNormalUrl;
}

string CEDRPolicyHelperBase::GetRefreshUrl()
{
	return m_strRefreshUrl;
}

string CEDRPolicyHelperBase::GetSubUrl(string typeName)
{
	string subUrl = StringNull;

	for (auto it = m_map.begin(); it != m_map.end(); it++)
	{
		CEDRPolicyData *pEDRPolicyData = (CEDRPolicyData*)((*it).second);

		if (CUtil::Compare(pEDRPolicyData->GetTypeName(), typeName) == 0)
		{
			subUrl = pEDRPolicyData->GetSubUrl();
			break;
		}
	}

	LOGD << "서브 Url: " << subUrl;

	return subUrl;
}
*/

CEDRPolicyData* CEDRPolicyHelperBase::GetPolicyData(string typeName)
{
	CHECK(!typeName.empty());

	map<string, CEDRPolicyData*>::iterator it;
	it = m_map.find(typeName);

	if (it == m_map.end())
	{
		LOGW
			<< "해당 타입 값으로 리스트가 존재하지 않습니다.("
			<< typeName
			<< ")";
		return NULL;
	}

	return (CEDRPolicyData*)((*it).second);
}

BOOL CEDRPolicyHelperBase::AddPolicyData(
	CEDRPolicyData	*pEDRPolicyData
)
{
	CHECK_NOTNULL(pEDRPolicyData);

	LOGD << " ";
	LOGD << "=======================";
	LOGD << "NAME   : " << pEDRPolicyData->GetPolicyDataName();
	LOGD << "Type   : " << pEDRPolicyData->GetTypeName();

	vector<string>	keyNames = pEDRPolicyData->GetKeyName();
	for (auto it = keyNames.begin(); it != keyNames.end(); it++)
	{
		LOGD << "KeyName: " << (*it);
	}

	LOGD << "Code   : " << pEDRPolicyData->GetEDRType();

	map<string, CEDRPolicyData*>::iterator it;
	string typeName = pEDRPolicyData->GetTypeName();
	it = m_map.find(typeName);

	if (it != m_map.end())
	{
		LOGW
			<< "동일한 타입 값으로 리스트가 존재합니다 ("
			<< typeName
			<< ")";
		return FALSE;
	}

	m_map[pEDRPolicyData->GetTypeName()] = pEDRPolicyData;

	return TRUE;
}

EDRJson	CEDRPolicyHelperBase::GenerateAck(
	CHttpData* pHttpData
	, BOOL status
	, string keyName
)
{
	// m_map.size() 가 0 면 초기화가 되지 않은 상태
	RETURN_FAIL_IF_MSG(
		m_map.size() <= 0
		, Json::objectValue
		, "CDERPolicyData 가 없습니다."
	);

	EDRJson jsonRet;

	jsonRet[JsonField_Status] = CUtil::GetAckCode(status);
	jsonRet[JsonField_Message] = CUtil::GetAckMessage(status);
	jsonRet[JsonField_ApplyTime] = CUtil::CurrentDateTime();

	return jsonRet;
}

EDRJson	CEDRPolicyHelperBase::ExtractPolicy(
	CHttpData* pHttpData
	, string keyName
)
{
	// m_map.size() 가 0 면 초기화가 되지 않은 상태
	RETURN_FAIL_IF_MSG(
		m_map.size() <= 0
		, Json::objectValue
		, "CDERPolicyData 가 없습니다."
	);

	return pHttpData->GetDataAsJson()[keyName];
}

BOOL CEDRPolicyHelperBase::ApplyPolicy(
	EDRJson jsonPolicy
	, BOOL isUpdated
)
{
	// m_map.size() 가 0 면 초기화가 되지 않은 상태
	RETURN_FAIL_IF_MSG(
		m_map.size() <= 0
		, Json::objectValue
		, "CDERPolicyData 가 없습니다."
	);

	return TRUE;
}

BOOL CEDRPolicyHelperBase::BackupFile(string filename)
{
	string bakName = filename;
	bakName.append(".old");
	BOOL bRet = FALSE;

	LOGW_IF(!FileSystem::File::Remove(bakName))
		<< bakName
		<< " 파일을 삭제 하지 못했습니다.";
	LOGW_IF(!FileSystem::File::copy(filename, bakName))
		<< "파일 "
		<< filename
		<< " 을 "
		<< bakName
		<< " 파일로 백업 하지 못했습니다.";
	LOGW_IF(!FileSystem::File::Remove(filename))
		<< filename
		<< " 파일을 삭제 하지 못했습니다.";

	return TRUE;
}

BOOL CEDRPolicyHelperBase::SavePolicyFile(EDRJson json)
{
	string strFilename = m_strPolicyMetaName;
	BackupFile(strFilename.append(".POLICY"));
	File filePolicy(strFilename);
	CHECK(filePolicy.Open(File::out, File::text));

	filePolicy.PutLine(json.toStyledString());
	filePolicy.flush();
	filePolicy.Close();

	return TRUE;
}

BOOL CEDRPolicyHelperBase::LoadPolicyFile()
{
	string strFilename = m_strPolicyMetaName;
	File filePolicy(strFilename.append(".POLICY"));

	if (!filePolicy.Open(File::in, File::text))
	{
		LOGW << "정책 파일을 읽을 수 없습니다.";
		return FALSE;
	}

	string strPolicy = filePolicy.ReadAll();
	filePolicy.Close();

	if (strPolicy.empty())
	{
		LOGW << "정책 파일이 비어 있습니다.";
		return FALSE;
	}

	EDRJson			json;
	EDRJsonReader	reader;
	BOOL bRet = reader.parse(strPolicy, json);

	// 실시간 디버깅 용 
	/*
	string strValue = json.toStyledString();
	*/

	if (!bRet)
	{
		LOGW << "정책 파일을 읽을 수 없습니다.";
		return FALSE;
	}

	if (ApplyPolicy(json, FALSE) == FALSE)
	{
		LOGW << "정책을 적용할 수 없습니다.";
		return FALSE;
	}

	LOGI << "정책 파일을 정상적으로 읽었습니다.";
	return TRUE;
}

BOOL CEDRPolicyHelperBase::ApplyToKernelDriver()
{
	for (auto it = m_map.begin(); it != m_map.end(); it++)
	{
		CEDRPolicyData *pEDRPolicyData = (CEDRPolicyData*)((*it).second);

		string filename = ("KPOL.IOC.");
		string typeName = pEDRPolicyData->GetTypeName();
		string from = "\n";
		string to = ";";

		filename = filename.append(typeName);

		if (pEDRPolicyData->WriteFile(filename))
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
			INT type = (int)pEDRPolicyData->GetEDRType();
			INT size = strKenelPolicy.length();

			LOGI << "** " << filename;
			LOGI << endl << strKenelPolicy;

			strKenelPolicy = CUtil::Replace(strKenelPolicy, from, to);

			LOGW_IF(!SafePCKModule::Add(type, (CHAR*)strKenelPolicy.c_str(), size))
				<< "커널 정책 적용을 실패 하였습니다. ( "
				<< pEDRPolicyData->GetPolicyDataName() << ", " << pEDRPolicyData->GetTypeName()
				<< " )";
		}

		File::Remove(filename);
	}

	return TRUE;
}

BOOL CEDRPolicyHelperBase::ClearPolicyFile()
{
	string strFilename = m_strPolicyMetaName;

	strFilename.append(".POLICY");
	LOGI_IF(File::Remove(strFilename))
		<< "정책 파일을 삭제 하였습니다. ( "
		<< strFilename << " )";

	strFilename.append(".old");
	LOGI_IF(File::Remove(strFilename))
		<< "정책 파일을 삭제 하였습니다. ( "
		<< strFilename << " )";

	return TRUE;
}