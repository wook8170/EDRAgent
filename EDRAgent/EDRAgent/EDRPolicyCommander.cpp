#pragma once
#include "StdAfx.h"
#include "CommonDef.h"
#include "Util/Util.h"
#include "Util/FileSystem.h"
#include "Util/Process.h"
#include "Util/Registry.h"
#include "EDRPolicyCommander.h"

#include "EDRAgent.h"
extern CEDRAgentApp theApp;

CEDRPolicyCommander::CEDRPolicyCommander()
{

}

CEDRPolicyCommander::~CEDRPolicyCommander()
{

}

CEDRPolicyCommander::ECommand CEDRPolicyCommander::GetCommandCode(
	string cmd
)
{
	if (CUtil::Compare(cmd, Command_Delete) == 0)
	{
		return ECommandDelete;
	}
	else if (CUtil::Compare(cmd, Command_Block) == 0)
	{
		return ECommandBlock;
	}
	else if (CUtil::Compare(cmd, Command_BlockCompleted) == 0)
	{
		return ECommandBlockCompleted;
	}

	return ECommandInvalid;
};

string CEDRPolicyCommander::GetCommandString(
	CEDRPolicyCommander::ECommand eCmd
)
{
	if (eCmd == CEDRPolicyCommander::ECommandDelete)
	{
		return Command_Delete;
	}
	else if (eCmd == CEDRPolicyCommander::ECommandBlock)
	{
		return Command_Block;
	}
	else if (eCmd == CEDRPolicyCommander::ECommandBlockCompleted)
	{
		return Command_BlockCompleted;
	}

	return Command_Invalid;
};

BOOL CEDRPolicyCommander::DoCommandFile(
	vector<CommandData_Ptr> commandData
)
{
	BOOL ret = FALSE;

#ifndef NOT_USE_PROCESS_BLOCK
	vector<PROCESS_TREE_ITEM_Ptr> vecProcessList;

	string	log = "";
	string	message = "File 삭제를 위해 종료\n";
	INT		alertCount = 0;

	// 프로세스는 동일 해쉬로 동일 패스로 여러개의 이미지가 있을 수 있음
	for (auto it4PidList = commandData.begin(); it4PidList != commandData.end(); it4PidList++)
	{
		CommandData_Ptr pProcess = (*it4PidList);

		// 현재 실행중인 프로세스 리스트에서 삭제할 PID 추출
		// 강제 삭제를 위해서는 실행중인 프로세스가 종료되어야 하므로 일단 모두 강제 종료
		if (GetCommandCode(pProcess->szCommand) != ECommandBlockCompleted)
		{
			vector<PROCESS_TREE_ITEM_Ptr> vecPids = CProcess::Instance()->FindProcessList(pProcess->szData);
			vecProcessList.insert(vecProcessList.end(), vecPids.begin(), vecPids.end());
		}
	}
	// 위에서 추출한 종료 대상 프로세스 강제 종료
	for (auto it4Pid = vecProcessList.begin(); it4Pid != vecProcessList.end(); it4Pid++)
	{
		PROCESS_TREE_ITEM_Ptr pProcess = *it4Pid;
		ret = CProcess::Instance()->KillProcess(pProcess->dwPID);
		if (ret)
		{
			CProcess::Instance()->RemoveProcessList(pProcess->dwPID, pProcess);

			LOGI
				<< "* 프로세스 강제 종료 (X):"
				<< ", " << FILLSETWR(5) << pProcess->dwPID
				<< ", " << FILLSETWR(5) << pProcess->dwPPID
				<< ", " << pProcess->szSHA256
				<< ", " << pProcess->szMD5
				<< ", " << pProcess->szImgPath;
			
			// 사용자 알람
			message.append(" - ").append(pProcess->szImgPath).append("\n");
			alertCount++;

			if ((alertCount != 0 && alertCount % USER_ALERT_LIEN_PER_CELL == 0))
			{
				AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T(
					"Nictech NEDR"
					, (CHAR*)message.c_str());
				PostMessage(theApp.m_pMainWindow->m_hWnd, WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);

				message = "File 삭제를 위해 종료\n";
				alertCount = 0;
			}
		}
	}

	if (alertCount > 0 && alertCount % USER_ALERT_LIEN_PER_CELL > 0 && message.length())
	{
		AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T(
			"Nictech NEDR"
			, (CHAR*)message.c_str());
		PostMessage(theApp.m_pMainWindow->m_hWnd, WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);
	}

	message = "File 삭제 발생\n";
	alertCount = 0;

	// 삭제 명령에 해당하는 파일 삭제
	for (auto it4Proc = commandData.begin(); it4Proc != commandData.end(); it4Proc++)
	{
		CommandData_Ptr pProcess = (*it4Proc);

		if (GetCommandCode(pProcess->szCommand) == ECommandDelete)
		{
			if (CUtil::PatternMatch(pProcess->szData, "?:\\*")
				&& File::exists(pProcess->szData))
			{
				ret = File::Remove(pProcess->szData);

				// 사용자 알람
				if (ret)
				{
					LOGI << "% 파일 강제 삭제 (X):" << pProcess->szData;

					FileInfo file(pProcess->szCommand);

					string logItem = CUtil::Format(
						"type=file|command=delete|log_time=%s|name=%s|size=%d|path=%s|hash=%s;"
						, CUtil::CurrentDateTime()
						, file.Name()
						, file.Size()
						, pProcess->szCommand
						, CUtil::GetSha256(pProcess->szCommand).c_str()
					);
					
					log.append(logItem);
					message.append(" - ").append(pProcess->szData).append("\n");
					alertCount++;

					if ((alertCount != 0 && alertCount % USER_ALERT_LIEN_PER_CELL == 0))
					{
						AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T(
							"Nictech NEDR"
							, (CHAR*)message.c_str());
						PostMessage(theApp.m_pMainWindow->m_hWnd, WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);

						message = "File 삭제 발생\n";
						alertCount = 0;
					}
				}
			}
		}
	}

	if (alertCount > 0 && alertCount % USER_ALERT_LIEN_PER_CELL > 0 && message.length())
	{
		AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T(
			"Nictech NEDR"
			, (CHAR*)message.c_str());
		PostMessage(theApp.m_pMainWindow->m_hWnd, WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);
	}

	if (log.length() > 0)
	{
		THREAD_HANDLE logUploadThreadHandle = theApp.m_pMainWindow->GetUserModeLogUploadThreadHandle();
		ThreadCalllbackData_Ptr threadCallbackData = make_shared<ThreadCalllbackData>();
		threadCallbackData->wParam = (WPARAM)log.c_str();
		threadCallbackData->pContext = (VOID*)theApp.m_pMainWindow;

		CEDRWorkerThreadManager::Instance()->StartThread(logUploadThreadHandle);

		CEDRWorkerThreadManager::Instance()->InvokeThreadFunc(
			logUploadThreadHandle
			, threadCallbackData
		);
	}

#else
	ret = TRUE;
#endif

	return ret;
};

BOOL CEDRPolicyCommander::DoCommandRegistry(
	vector<CommandData_Ptr> commandData
)
{
	BOOL	ret = FALSE;
	string	log = "";
	string	message = "Registry 삭제 발생\n";
	INT		alertCount = 0;

	for (auto it4RegistryList = commandData.begin(); it4RegistryList != commandData.end(); it4RegistryList++)
	{
		CommandData_Ptr pRegistry = (*it4RegistryList);
		string registry = pRegistry->szData;
		ECommand eCmd = GetCommandCode(pRegistry->szCommand);

		string splitChar = PathDelimeter;
		vector<string> vecRegistryEntity = CUtil::Split(registry, splitChar);
		string strRegistryKey = StringNull;
		string strRegistryParentKey = StringNull;
		string strValue = StringNull;

		for (auto i = 1; i < vecRegistryEntity.size(); i++)
		{
			if (i > 1)
			{
				strRegistryKey.append(PathDelimeter);
				strRegistryParentKey.append(PathDelimeter);
			}
			strRegistryKey.append(vecRegistryEntity[i]);
			if (i < vecRegistryEntity.size() - 1)
			{
				strRegistryParentKey.append(vecRegistryEntity[i]);
			}
			else
			{
				strValue = vecRegistryEntity[i];
			}
		}

		if (vecRegistryEntity.size() <= 1)
		{
			LOGE << "잘못된 레지스트리 경로 입니다. ( " << registry << " )";
			pRegistry.reset();
			continue;
		}

		if (eCmd == ECommandDelete)
		{
			CRegistry registryKey;
			CRegistry registryParentKey;
			BOOL deleted = FALSE;
			// 항목이 Value 일 경우
			if (ERROR_SUCCESS == registryParentKey.Open(vecRegistryEntity[0], strRegistryParentKey))
			{
				// 해당 value 삭제
				ret |= (ERROR_SUCCESS == registryParentKey.DeleteValue(strValue));
			}
			// 항목이 Key 일 경우
			if (ERROR_SUCCESS == registryKey.Open(vecRegistryEntity[0], strRegistryKey))
			{
				// 해당 key 하위 value 삭제
				ret |= (ERROR_SUCCESS == registryKey.DeleteSubValue());
				// 해당 key 하위 key 삭제
				ret |= (ERROR_SUCCESS == registryKey.DeleteSubKey());
				// 항목 key 삭제
				if (ERROR_SUCCESS == registryParentKey.Open(vecRegistryEntity[0], strRegistryParentKey))
				{
					ret |= (ERROR_SUCCESS == registryParentKey.DeleteKey(strValue));
				}
			}

			if (ret)
			{
				LOGI << "% 레지스트리 강제 삭제 (X):" << pRegistry->szData;

				string logItem = CUtil::Format(
					"type=registry|command=delete|log_time=%s|log=%s;"
					, CUtil::CurrentDateTime().c_str()
					, pRegistry->szData
				);
				
				log.append(logItem);
				message.append(" - ").append(pRegistry->szData).append("\n");
				alertCount++;

				if ((alertCount != 0 && alertCount % USER_ALERT_LIEN_PER_CELL == 0))
				{
					message.append(" - ").append(pRegistry->szData);
					AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T("Nictech NEDR", (CHAR*)message.c_str());
					PostMessage(theApp.m_pMainWindow->m_hWnd, WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);

					message = "Registry 삭제 발생\n";
					alertCount = 0;
				}
			}
			else
			{
				LOGI
					<< "* 레지스트리 ( "
					<< registry
					<< " ) 를 찾지 못했습니다.";
			}
		}

		pRegistry.reset();
	}

	if (alertCount > 0 && alertCount % USER_ALERT_LIEN_PER_CELL > 0)
	{
		AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T(
			"Nictech NEDR"
			, (CHAR*)message.c_str());
		PostMessage(theApp.m_pMainWindow->m_hWnd, WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);
	}

	if (log.length() > 0)
	{
		THREAD_HANDLE logUploadThreadHandle = theApp.m_pMainWindow->GetUserModeLogUploadThreadHandle();
		ThreadCalllbackData_Ptr threadCallbackData = make_shared<ThreadCalllbackData>();
		threadCallbackData->wParam = (WPARAM)log.c_str();
		threadCallbackData->pContext = (VOID*)theApp.m_pMainWindow;

		CEDRWorkerThreadManager::Instance()->StartThread(logUploadThreadHandle);

		CEDRWorkerThreadManager::Instance()->InvokeThreadFunc(
			logUploadThreadHandle
			, threadCallbackData
		);
	}

	return ret;
};

BOOL CEDRPolicyCommander::DoCommandProcess(
	vector<CommandData_Ptr> commandData
)
{
	BOOL ret = FALSE;
#ifndef NOT_USE_PROCESS_BLOCK
	vector<PROCESS_TREE_ITEM_Ptr> vecProcessList;

	string	log = "";
	string	message = "Process 차단 발생\n";
	INT		alertCount = 0;

	// 프로세스는 동일 해쉬로 동일 패스로 여러개의 이미지가 있을 수 있음
	for (auto it4PidList = commandData.begin(); it4PidList != commandData.end(); it4PidList++)
	{
		CommandData_Ptr pProcess = (*it4PidList);
		// 현재 실행중인 프로세스 리스트에서 삭제할 PID 추출
		// 강제 삭제를 위해서는 실행중인 프로세스가 종료되어야 하므로 일단 모두 강제 종료
		if (GetCommandCode(pProcess->szCommand) != ECommandBlockCompleted)
		{
			vector<PROCESS_TREE_ITEM_Ptr> vecPids = CProcess::Instance()->FindProcessList(pProcess->szData);
			vecProcessList.insert(vecProcessList.end(), vecPids.begin(), vecPids.end());
		}
	}

	// 위에서 추출한 종료 대상 프로세스 강제 종료
	for (auto it4Pid = vecProcessList.begin(); it4Pid != vecProcessList.end(); it4Pid++)
	{
		PROCESS_TREE_ITEM_Ptr pProcess = *it4Pid;
		ret = CProcess::Instance()->KillProcess(pProcess->dwPID);
		if (ret)
		{
			CProcess::Instance()->RemoveProcessList(pProcess->dwPID, pProcess);

			LOGI
				<< "* 프로세스 강제 종료 (X):"
				<< ", " << FILLSETWR(5) << pProcess->dwPID
				<< ", " << FILLSETWR(5) << pProcess->dwPPID
				<< ", " << pProcess->szSHA256
				<< ", " << pProcess->szMD5
				<< ", " << pProcess->szImgPath;

			// 사용자 알람

			FileInfo file(pProcess->szImgPath);

			string logItem = CUtil::Format(
				"type=process|command=kill|log_time=%s|pid=%ld|ppid=%ld|hash=%s|path=%s;"
				, CUtil::CurrentDateTime().c_str()
				, pProcess->dwPID
				, pProcess->dwPPID
				, CUtil::GetSha256(pProcess->szImgPath).c_str()
				, pProcess->szImgPath
			);

			log.append(logItem);
			message.append(" - ").append(pProcess->szImgPath).append("\n");
			alertCount++;

			if ((alertCount != 0 && alertCount % USER_ALERT_LIEN_PER_CELL == 0))
			{
				AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T(
					"Nictech NEDR"
					, (CHAR*)message.c_str());
				PostMessage(theApp.m_pMainWindow->m_hWnd, WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);

				message = "Process 차단 발생\n";
				alertCount = 0;
			}			
		}
		else
		{
			LOGI
				<< "* 프로세스를 찾지 못했습니다: "
				<< ", " << FILLSETWR(5) << pProcess->dwPID
				<< ", " << FILLSETWR(5) << pProcess->dwPPID
				<< ", " << pProcess->szSHA256
				<< ", " << pProcess->szMD5
				<< ", " << pProcess->szImgPath;
		}
	}

	if (alertCount > 0 && alertCount % USER_ALERT_LIEN_PER_CELL > 0)
	{
		AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T(
			"Nictech NEDR"
			, (CHAR*)message.c_str());
		PostMessage(theApp.m_pMainWindow->m_hWnd, WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);
	}

	if (log.length() > 0)
	{
		CHAR* buff = EDRNew CHAR[log.length() + 1];
		EDRZeroMemory(buff, log.length() + 1);
		EDRCopyMemory(buff, log.c_str(), log.length());

		LOGD << "**** LOG : " << log;
		LOGD << "**** LOG : " << buff;

		THREAD_HANDLE logUploadThreadHandle = theApp.m_pMainWindow->GetUserModeLogUploadThreadHandle();
		ThreadCalllbackData_Ptr threadCallbackData = make_shared<ThreadCalllbackData>();
		threadCallbackData->wParam = (WPARAM)buff;
		threadCallbackData->pContext = (VOID*)theApp.m_pMainWindow;

		CEDRWorkerThreadManager::Instance()->InvokeThreadFunc(
			logUploadThreadHandle
			, threadCallbackData
		);
	}	

	// 삭제 명령에 해당하는 프로세스 삭제
	/*
	for (auto it4Proc = commandData.begin(); it4Proc != commandData.end(); it4Proc++)
	{
		CommandData_Ptr pCommandData = (*it4Proc);

		if (GetCommandCode(pCommandData->szCommand) != ECommandDelete)
		{
			if (CUtil::PatternMatch(pCommandData->szData, "?:\\*")
				&& File::exists(pCommandData->szData))
			{
				ret = File::Remove(pCommandData->szData);
				LOGI_IF(ret) << "% 프로세스 강제 삭제 (X):" << pCommandData->szData;
				// 사용자 알람
				string message = "Process 삭제 발생\n";
				message.append(" - ").append(pCommandData->szData);
				AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T("Nictech NEDR", (CHAR*)message.c_str());
				PostMessage(theApp.m_pMainWindow->m_hWnd, WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);
			}
		}

		pCommandData.reset();
	}
	*/

#else
	ret = TRUE;
#endif

	return ret;
};

BOOL CEDRPolicyCommander::DoCommandIp(
	vector<CommandData_Ptr> commandData
)
{
	return FALSE;
};

BOOL CEDRPolicyCommander::DoCommandURL(
	vector<CommandData_Ptr> commandData
)
{
	return FALSE;
};

BOOL CEDRPolicyCommander::DoCommand(
	string type
	, vector<CommandData_Ptr> commandData
)
{
	if (CUtil::Compare(type, Type_Process) == 0)
	{
		return DoCommandProcess(commandData);
	}
	else if (CUtil::Compare(type, Type_Registry) == 0)
	{
		return DoCommandRegistry(commandData);
	}
	else if (CUtil::Compare(type, Type_File) == 0)
	{
		return DoCommandFile(commandData);
	}
	else if (CUtil::Compare(type, Type_IP) == 0)
	{
		return DoCommandIp(commandData);
	}
	else if (CUtil::Compare(type, Type_URL) == 0
		|| CUtil::Compare(type, Type_Domain) == 0)
	{
		return DoCommandURL(commandData);
	}

	return FALSE;
}

