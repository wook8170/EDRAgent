#include "StdAfx.h"
#include "IOCPolicyHelperImpl.h"
#include "HttpClient/HttpData.h"
#include "HttpClient/HttpAttribute.h"
#include "Util/FileSystem.h"
#include "Util/Util.h"
#include "Util/HashList.h"
#include "Util/Process.h"
#include "Util/Registry.h"
#include "EDRShellNotifyDlg.h"
#include "EDRAgent.h"
#include "EDRPolicyData.h"
#include "EDRConfig.h"
#include "ProcmonDll.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#endif

class CEDRAgentApp;

extern CEDRAgentApp theApp;

BOOL CIOCPolicyHelperImpl::DoCommand(
	string type
	, vector<CommandData_Ptr> vecList
)
{
	LOGI << "콜백 호출 !";
	CEDRPolicyData	*pEDRPolicyData = GetPolicyData(type);

	return pEDRPolicyData->PerformCommand(vecList);
}

BOOL CIOCPolicyHelperImpl::UpdatePolicy(
	string updateStatus
	, CEDRPolicyData *pPolicyData
	, EDRJson json
	, BOOL isUpdate
)
{
#ifdef _DEBUG
	_CrtMemState s1;
	_CrtMemState s2;
	_CrtMemState s3;
	_CrtMemCheckpoint(&s1);
#endif
	BOOL ret = FALSE;
	string strKey = pPolicyData->GetList()->GetRealKey(json);
	EEDRType type = pPolicyData->GetEDRType();
	// 정책 상태 (추가/삭제/업데이트)
	string strUpdateStatus = json[JsonField_Status].asString();
	// 기존에 키에 대한 정책 항목이 있는 지 확인
	EDRJson policyItem = pPolicyData->FindNode(strKey);
	BOOL bExist = (!policyItem.empty()) ? TRUE : FALSE;

	if (CUtil::Compare(pPolicyData->GetTypeName(), Type_Registry) == 0)
	{
		string rawPath = StringNull;
		string userPath = StringNull;

		strKey = CRegistry::GetRawPath(strKey);
	}

	// 신규 정책 항목 추가
	if (CUtil::Compare(strUpdateStatus, StatusCode_Create) == 0)
	{
		LOGI << "정책 추가 요청";
		if (!bExist)
		{
			pPolicyData->InsertNode((EDRJson)json);
			if (isUpdate)
			{
				LOGW_IF(!SafePCKModule::Add(type, (CHAR*)strKey.append("\n").c_str(), (DWORD)strKey.length() + 1))
					<< "@@ 커널 정책 적용 실패 ( SafePCKModule::Add() ): " << updateStatus
					<< ", " << strKey
					<< ", " << json;
			}
		}
		else
		{
			if (policyItem.compare(json) != 0)
			{
				pPolicyData->DeleteNode(strKey);
				pPolicyData->InsertNode((EDRJson)json);

				if (isUpdate)
				{
					LOGW_IF(!SafePCKModule::Remove(type, (CHAR*)strKey.c_str(), (DWORD)strKey.length()))
						<< "@@ 커널 정책 적용 실패 ( SafePCKModule::Remove() ): " << updateStatus
						<< ", " << strKey
						<< ", " << json;

					LOGW_IF(!SafePCKModule::Add(type, (CHAR*)strKey.append("\n").c_str(), (DWORD)strKey.length() + 1))
						<< "@@ 커널 정책 적용 실패 ( SafePCKModule::Add() ): " << updateStatus
						<< ", " << strKey
						<< ", " << json;
				}

				ret = TRUE;
			}
		}
	}
	// 기존 정책 항목 업데이트
	else if (CUtil::Compare(strUpdateStatus, StatusCode_Update) == 0)
	{
		LOGI << "$ 정책 업데이트 요청";
		if (bExist)
		{
			pPolicyData->DeleteNode(strKey);
			pPolicyData->InsertNode((EDRJson)json);

			if (isUpdate)
			{
				LOGW_IF(!SafePCKModule::Remove(type, (CHAR*)strKey.c_str(), (DWORD)strKey.length()))
					<< "@@ 커널 정책 적용 실패 ( SafePCKModule::Remove() ): " << updateStatus
					<< ", " << strKey
					<< ", " << json;

				LOGW_IF(!SafePCKModule::Add(type, (CHAR*)strKey.append("\n").c_str(), (DWORD)strKey.length()+1))
					<< "@@ 커널 정책 적용 실패 ( SafePCKModule::Add() ): " << updateStatus
					<< ", " << strKey
					<< ", " << json;
			}


		}
		else
		{
			pPolicyData->InsertNode((EDRJson)json);

			if (isUpdate)
			{
				LOGW_IF(!SafePCKModule::Add(type, (CHAR*)strKey.append("\n").c_str(), (DWORD)strKey.length() + 1))
					<< "@@ 커널 정책 적용 실패 ( SafePCKModule::Add() ): " << updateStatus
					<< ", " << strKey
					<< ", " << json;
			}
		}

		ret = TRUE;
	}
	// 기존 정책 항목 삭제
	else if (CUtil::Compare(strUpdateStatus, StatusCode_Delete) == 0)
	{
		LOGI << "정책 삭제 요청";
		if (bExist)
		{
			pPolicyData->DeleteNode(strKey);
			if (isUpdate)
			{
				LOGW_IF(!SafePCKModule::Remove(type, (CHAR*)strKey.c_str(), strKey.length()))
					<< "@@ 커널 정책 적용 실패 ( SafePCKModule::Remove() ): " << updateStatus
					<< ", " << strKey
					<< ", " << json;
			}
		}

		ret = TRUE;
	}
	// Error
	else
	{
		LOGW
			<< "잘못된 status 값입니다. ("
			<< strUpdateStatus
			<< ")";
		LOGW << "\n" << json;

		ret = FALSE;
	}

	return FALSE;
}

VOID CALLBACK CIOCPolicyHelperImpl::ListCallbackForCreate(
	PCHECK_LIST_ENTRY CheckList[]
	, ULONG CheckListCount
	, VOID* UserData
)
{
	CHECK_NOTNULL(UserData);

	CIOCPolicyHelperImpl* pThis = (CIOCPolicyHelperImpl*)UserData;

	if (!pThis->IsInit())
	{
		LOGW << "객체가 초기화 되지 않았습니다.";
		return;
	}

	CEDRPolicyData	*pEDRPolicyData = pThis->GetPolicyData(Type_Process);
	BOOL found = FALSE;

	if (!pEDRPolicyData)
	{
		LOGW << "유효한 리스트를 가져오지 못했습니다.";
		return;
	}

	vector<CommandData_Ptr> vecProcessList;

	for (auto i = 0; i < (INT)CheckListCount; i++)
	{
		string processPath = CUtil::ConvU2M(CheckList[i]->ProcessInfo->ImagePath);	

		if (CUtil::PatternMatch(processPath, "SVCHOST.EXE"))
		{
			continue;
		}
		EDRJson recentFile = CProcess::Instance()->GetRecentProcess(processPath);		
		string processHashSha256 = recentFile["sha256"].asString();
		string processHashMD5 = recentFile["md5"].asString();
		string cmd = "blockcompleted";

		if (processPath.empty() || processHashSha256.empty() || processHashMD5.empty())
		{
			LOGW << "프로세스 정보를 가져올 수 없습니다.";
			LOGW << "PATH  :" << processPath;
			LOGW << "SHA256:" << processHashSha256;
			LOGW << "MD5   :" << processHashMD5;

			continue;
		}

		EDRJson jsonProcessViaSha256 = pEDRPolicyData->FindNode(processHashSha256);
		EDRJson jsonProcessViaMD5 = pEDRPolicyData->FindNode(processHashMD5);
		EDRJson jsonProcess = (!jsonProcessViaSha256.empty()) ? jsonProcessViaSha256 : jsonProcessViaMD5;

		LOGI << "jsonProcessViaSha256: " << jsonProcessViaSha256;
		LOGI << "jsonProcessViaMD5: " << jsonProcessViaMD5;

		if (!jsonProcess.empty())
		{
#ifdef NOT_USE_PROCESS_BLOCK
			CheckList[i]->AddToBlacklist = FALSE;
#else
			CheckList[i]->AddToBlacklist = TRUE;
#endif			
			CommandData_Ptr data(EDRNew CommandData(
				EDR_TYPE_PROCESS
				, cmd
				, processHashSha256
				, jsonProcess
			));

			vecProcessList.push_back(data);
		}		
		else
		{
			EDRJson jsonProcess = pEDRPolicyData->FindNode(processPath.c_str());
			if (!jsonProcess.empty())
			{
#ifdef NOT_USE_PROCESS_BLOCK
				CheckList[i]->AddToBlacklist = FALSE;
#else
				CheckList[i]->AddToBlacklist = TRUE;
#endif				
				CommandData_Ptr data(EDRNew CommandData(
					EDR_TYPE_PROCESS
					, cmd
					, jsonProcess["path"].asString()
					, jsonProcess
				));
			}
			else
			{
				LOGD
					<< "* 프로세스 리스트 허용 (O): " << FILLSETWR(5) << CheckList[i]->ProcessInfo->ProcessId
					<< ", " << FILLSETWL(5) << CheckList[i]->ProcessInfo->ParentProcessId
					<< ", " << processHashSha256
					<< ", " << processHashMD5
					<< ", " << processPath;

				CheckList[i]->AddToBlacklist = FALSE;
			}
		}
	}

	pEDRPolicyData->PerformCommand(vecProcessList);

}

CIOCPolicyHelperImpl::CIOCPolicyHelperImpl(
	string name
)
{
	m_strName = name;
}

CIOCPolicyHelperImpl::~CIOCPolicyHelperImpl()
{

}

void CIOCPolicyHelperImpl::Init(
	string name
)
{
	CEDRPolicyHelperBase::Init(CEDRConfig::EEDRConfigIOC);

	// 해당 타입에 대한 리스트 획득
	map<string, CEDRPolicyData*>::iterator listItem = m_map.find(Type_Process);

	if (listItem != m_map.end())
	{
		CProcess::Instance()->RegisterCallback(
			CIOCPolicyHelperImpl::ListCallbackForCreate
			, NULL
			, (VOID*)this
		);
	}
}

EDRJson	CIOCPolicyHelperImpl::GenerateAck(
	CHttpData* pHttpData
	, BOOL status
	, string keyName
)
{
	EDRJson jsonRoot = CEDRPolicyHelperBase::GenerateAck(
		pHttpData
		, status
		, keyName
	);

	EDRJson jsonApplyList;
	EDRJson jsonData = pHttpData->GetDataAsJson()[keyName];
	for (auto it = jsonData.begin(); it != jsonData.end(); it++)
	{
		if (it->isObject())
		{
			EDRJson item;
			item[JsonFiled_ID] = (*it)[JsonFiled_ID];
			item[JsonField_Type] = (*it)[JsonField_Type];

			jsonApplyList.append(item);
		}
	}

	jsonRoot[JsonFiled_ApplyList] = jsonApplyList;

	return jsonRoot;
}

BOOL CIOCPolicyHelperImpl::ApplyPolicy(
	EDRJson jsonPolicy
	, BOOL isUpdated
)
{
	// m_vecTypeNames.size() 가 0 면 초기화가 되지 않은 상태
	RETURN_FAIL_IF_MSG(
		m_map.size() <= 0
		, Json::objectValue
		, "CDERPolicyData 가 없습니다."
	);

	if (jsonPolicy.size() == 0)
	{
		LOGW << "적용 가능한 신규 정책이 없습니다.. (" << jsonPolicy << ")";
		return FALSE;
	}

	DPRINT_JSON(jsonPolicy);

	if (isUpdated)
	{
		SavePolicyFile(jsonPolicy);
	}

	map<string, CommandDataVector_Ptr>	mapVecList;
	CommandDataVector_Ptr				vecCurrentList;

	for (auto it = jsonPolicy.begin(); it != jsonPolicy.end(); it++)
	{
		string typeName = (*it)[JsonField_Type].asString();

		// 해당 타입에 대한 리스트 획득
		map<string, CEDRPolicyData*>::iterator listItem = m_map.find(typeName);

		if (listItem == m_map.end())
		{
			continue;
		}

		CEDRPolicyData	*pPolicyData = (CEDRPolicyData*)listItem->second;

		if (listItem == m_map.end() || pPolicyData == NULL)
		{
			LOGW << "리스트를 찾을 수 없습니다. (" << typeName << ")";
			continue;
		}

		if (it->isObject() && !typeName.empty())
		{
			DPRINT_JSON((*it));

			// 리스트의 실제 키값 획득
			string strKey = pPolicyData->GetList()->GetRealKey((*it));
			// 커맨드 종류
			string strCmd = (*it)[JsonField_Command].asString();

			LOGD << "키: ( " << strKey << " )";

#ifdef _DEBUG
			// 타입이 레지스트리일 경우, 커널에서 인식 가능한 RAW 포맷으로 변경해 주어야 함
			if (CUtil::Compare(typeName, Type_Registry) == 0)
			{
				string rawPath = StringNull;
				string userPath = StringNull;

				rawPath = CRegistry::GetRawPath(strKey);
				userPath = CRegistry::GetUserPath(rawPath);
				//LOGD << "RAW VALUEL	: " << rawPath;
				//LOGD << "USER VALUEL: " << userPath;
			}
#endif
			// 실시간 차단 정책 적용 ///////////////////////////////////////
			if (mapVecList.find(typeName) == mapVecList.end())
			{
				CommandDataVector_Ptr vecCommandDataList(EDRNew vector<CommandData_Ptr>());
				mapVecList[typeName] = vecCommandDataList;
			}

			// 키에 해당하는 command data 추가			
			map<string, CommandDataVector_Ptr>::iterator itVecList = mapVecList.find(typeName);
			CommandData_Ptr commandData(EDRNew CommandData(GetEDRTypeFromTypeString(typeName), strCmd, strKey, (*it)));
			vecCurrentList = (*itVecList).second;
			vecCurrentList->push_back(commandData);
			
			// 정책 업데이트 //////////////////////////////////////////
			BOOL bApplied = UpdatePolicy((*it)[JsonField_Status].asString(), pPolicyData, (*it), isUpdated);
		}
	}

	// 정책 적용 ( 차단, 삭제 또는 커널 적용 )
	for (auto itVecList = mapVecList.begin(); itVecList != mapVecList.end(); )
	{
		CommandDataVector_Ptr vecCurrentList = (*itVecList).second;
		
		// 유저모드 
		LOGW_IF(DoCommand((*itVecList).first, *vecCurrentList))
			<< "정책 실행을 실패 하였습니다. ( "
			<< (*itVecList).first <<
			" )";

		if (!isUpdated)
		{
			map<string, CEDRPolicyData*>::iterator listItem = m_map.find((*itVecList).first);
			if (listItem == m_map.end())
			{
				continue;
			}

			CEDRPolicyData	*pPolicyData = (CEDRPolicyData*)listItem->second;
			if (pPolicyData == NULL)
			{
				continue;
			}

			// 커널모드
			LOGW_IF(!pPolicyData->AppyToKernel())
				<< "정책 적용 실패 하였습니다. ( "
				<< (*itVecList).first <<
				" )";
		}
		itVecList++;
	}

	return TRUE;
}

