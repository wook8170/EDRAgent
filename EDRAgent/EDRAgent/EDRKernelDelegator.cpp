#include "StdAfx.h"
#include "CommonDef.h"
#include "EDRAgent.h"
#include "EDRPolicyData.h"
#include "EDRKernelDelegator.h"
#include "EDRConfig.h"
#include "Util/FileSystem.h"
#include "Util/Util.h"

#include "HttpClient/HttpContext.h"
#include "HttpClient/HttpClientBuilder.h"
#include "HttpClient/HttpData.h"
#include "HttpClient/HttpClient.h"

extern CEDRAgentApp theApp;

EEDRType CEDRKernelDelegator::GetType()
{
	return m_eType;
}

VOID CEDRKernelDelegator::LogCollectFunc(
	const ThreadCalllbackData_Ptr& value
	, VOID* userData
)
{
	CEDRKernelDelegator* pThis = (CEDRKernelDelegator*)value->pContext;
	CHECK_NOTNULL(pThis);

	LOGI
		<< "~~ " << pThis->GetName() << " - 로그 쓰레드 시작 (>)"
		<< "( " << pThis->GetLogCollectThreadHandle()->pThread->GetThreadId()
		<< ", " << pThis->GetLogCollectThreadHandle()->pThread->GetThreadName()
		<< ") ";

	DWORD dwTimeOut = (pThis->UsePollingMethod()) ? 1000 : INFINITE;
	LOGI_IF(pThis->UsePollingMethod()) << "로그 업로드 쓰레드 인터벌: " << dwTimeOut;

	string logName = ".\\";
	logName.append(pThis->GetName()).append(".log");
	File logFile(logName);

	DWORD collectedLogCount = 0;
	DWORD maxLogCount = CEDRConfig::Instance()->GetLogBufferCount(pThis->GetType());

	do
	{
		ULONG logSize;
		DWORD size = sizeof(logSize);

		if (pThis->GetLogCollectEvent().Wait(dwTimeOut) && !pThis->IsLogCollectThreadTerminated())
		{
			LockGuard lock(&pThis->m_Lock);

			if (logFile.Open(File::out, File::text, File::append))
			{
				LOGW_IF(!SafePCKModule::GetLogSize(pThis->GetType(), &logSize, size)) << "@@ 실패 : SafePCKModule::GetLogSize(" << pThis->m_strName << ")";
				
				if (logSize > 0)
				{
					LOGI << "커널 이벤트 수신 ( " << pThis->GetName() << " )";
					LOGI << "로그 사이즈: " << logSize;
					
					CHAR* buffer = EDRNew CHAR[logSize + 1];
					CHECK(buffer);
					EDRZeroMemory(buffer, logSize + 1);

					LOGW_IF(!SafePCKModule::GetLog(pThis->GetType(), buffer, logSize)) << "@@ 실패 : SafePCKModule::GetLog(" << pThis->m_strName << ")";
					logFile.PutLine(buffer);
					EDRDeleteArray(buffer);
				}

				logFile.Close();
			}
			else
			{
				LOGW << "로그파일 ( " << logName << " )" << " 을 열 수 없습니다.";
			}
		}

		if (pThis->UsePollingMethod())
		{
			//if (++collectedLogCount >= CEDRConfig::Instance()->GetLogBufferCount(pThis->GetType()))
			{
				//LOGW << "Set Upload Thread event";
				collectedLogCount = 0;
				pThis->GetLogUploadEvent().Set();
			}
		}
		else
		{
			pThis->GetLogUploadEvent().Set();
			pThis->GetLogCollectEvent().Reset();
		}
	} while (!pThis->IsLogCollectThreadTerminated());
		
	LOGI
		<< "~~ " << pThis->GetName() << " - 로그 쓰레드 종료 (X) "
		<< "( " << pThis->GetLogCollectThreadHandle()->pThread->GetThreadId()
		<< ", " << pThis->GetLogCollectThreadHandle()->pThread->GetThreadName()
		<< ") ";
}

VOID CEDRKernelDelegator::LogUploadFunc(
	const ThreadCalllbackData_Ptr& value
	, VOID* userData
)
{
	CEDRKernelDelegator* pThis = (CEDRKernelDelegator*)value->pContext;
	CHECK_NOTNULL(pThis);

	LOGI
		<< "~~ " << pThis->GetName() << " - 로그 쓰레드 시작 (>)"
		<< "( " << pThis->GetLogUploadThreadHandle()->pThread->GetThreadId()
		<< ", " << pThis->GetLogUploadThreadHandle()->pThread->GetThreadName()
		<< ") ";

	string logName = ".\\";
	logName.append(pThis->GetName()).append(".log");
	File logFile(logName);
	FileInfo logFileInfo(logName);

	CHttpContext *pHttpContext = EDRNew CHttpContext();
	CHECK_NOTNULL(pHttpContext);

	CHttpClientBuilder *pHttpClientBuilder = EDRNew CHttpClientBuilder(pHttpContext);
	CHECK_NOTNULL(pHttpClientBuilder);

	do
	{
		if (pThis->GetLogUploadEvent().Wait() && !pThis->IsLogUploadThreadTerminated())
		{
			LockGuard lock(pThis->GetLock());

			CHttpData* pData = NULL;

			if (logFile.Open(File::in, File::text))
			{
				string logURL = CEDRConfig::Instance()->GetLogURL();
				logURL.append("/").append(CUtil::GetUserID());

				CHttpClient *pHttpClient = pHttpClientBuilder
					->SetMethod(CHttpAttribute::Post)
					->SetURL(logURL)
					->Build();

				CHECK_NOTNULL(pHttpClient);

				string buffer = logFile.ReadAll();
				EDRJson logJson = CUtil::ParseLogData((CHAR*)buffer.c_str());

				if (!logJson["data"].empty())
				{
					INT alertCount = 0;

					string message = pThis->GetName();
					message.append(" 차단 발생\n");
					CUtil::Upper(message);

					for (auto it = logJson["data"].begin(); it != logJson["data"].end(); it++ )
					{			
						if ((*it)["log"].isObject())
						{
							if (!(*it)["log_time"].empty()) message.append(" - ").append((*it)["log_time"].asString()).append(" / "); 
							if (!(*it)["log"]["path"].empty()) message.append((*it)["log"]["path"].asString()).append(" ");
							if (!(*it)["log"]["registry"].empty()) message.append((*it)["log"]["registry"].asString()).append(" ");
							if (!(*it)["log"]["ip"].empty()) message.append((*it)["log"]["ip"].asString()).append(" ");
							if (!(*it)["log"]["port"].empty()) message.append((*it)["log"]["port"].asString()).append(" ");
							if (!(*it)["log"]["url"].empty()) message.append((*it)["log"]["url"].asString()).append(" ");
							if (!(*it)["log"]["domain"].empty()) message.append((*it)["log"]["domain"].asString()).append(" ");

							message.append("\n");
						}
						else
						{
							if (!(*it)["log_time"].empty()) message.append(" - ").append((*it)["log_time"].asString()).append(" / ");
							if (!(*it)["log"].empty()) message.append((*it)["log"].asString()).append(" ");

							message.append("\n");
						}

						alertCount++;

						if ((alertCount != 0 && alertCount % USER_ALERT_LIEN_PER_CELL == 0) || logJson["data"].size() <= USER_ALERT_LIEN_PER_CELL)
						{
							AlarmMessage_T *pAlamrMessge = EDRNew AlarmMessage_T(
								"Nictech NEDR"
								, (CHAR*)message.c_str());
							PostMessage(theApp.m_pMainWindow->m_hWnd, WM_NOTIFY_MESSAGE, 0, (LPARAM)pAlamrMessge);

							message = pThis->GetName();
							message.append(" 차단 발생\n");
							CUtil::Upper(message);
						}
					}

					LOGD << "로그 전송 요청 :" << logJson.toStyledString();

					pData = EDRNew CHttpData(logJson);
					pHttpClient->SetRequestData(pData);
					HTTP_STATUS httpStatus = pHttpClient->SendRequest();
					CHttpData* pResponseData = pHttpClient->GetResponseData();

					LOGD_IF(pResponseData) << "로그 전송 응답: " << pResponseData->GetDataAsJson();
				}				

				EDRDelete(pData);
				EDRDelete(pHttpClient);
				EDRDelete(pData);

				logFile.Close();
				File::Remove(logName);
			}

			

		}

		pThis->GetLogUploadEvent().Reset();

	} while (!pThis->IsLogUploadThreadTerminated());


	EDRDelete(pHttpContext);
	EDRDelete(pHttpClientBuilder);

	LOGI
		<< "~~ " << pThis->GetName() << " - 로그 쓰레드 종료 (X) "
		<< "( " << pThis->GetLogUploadThreadHandle()->pThread->GetThreadId()
		<< ", " << pThis->GetLogUploadThreadHandle()->pThread->GetThreadName()
		<< ") ";
}

LOCK* CEDRKernelDelegator::GetLock()
{
	return &m_Lock;
}

THREAD_HANDLE CEDRKernelDelegator::GetLogCollectThreadHandle()
{
	return m_logCollectThreadHandle;
}

THREAD_HANDLE CEDRKernelDelegator::GetLogUploadThreadHandle()
{
	return m_logCollectThreadHandle;
}

CSyncEvent& CEDRKernelDelegator::GetLogCollectEvent()
{
	return m_logCollectEvent;
}

CSyncEvent& CEDRKernelDelegator::GetLogUploadEvent()
{
	return m_logUploadEvent;
}

BOOL CEDRKernelDelegator::IsLogCollectThreadTerminated()
{
	return m_logCollectThreadTerminating;
}

BOOL CEDRKernelDelegator::IsLogUploadThreadTerminated()
{
	return m_logUploadThreadTerminating;
}

EDRJson CEDRKernelDelegator::ParseLogData(
	CHAR* pData
)
{
	EDRJson logData;

	string DelimeterItem = ";";
	string DelimeterField = "|";
	string DelimeterKeyValue = "=";

	vector<string> vecItems = CUtil::Split(pData, DelimeterItem);
	EDRJson jsonRoot = Json::objectValue;
	EDRJson jsonLogs = Json::arrayValue;

	for (auto itItems = vecItems.begin(); itItems != vecItems.end(); itItems++)
	{
		if ((*itItems).empty())
		{
			continue;
		}

		vector<string> vecSingleItem = CUtil::Split((*itItems), DelimeterField);
		EDRJson	jsonItem = Json::objectValue;
		EDRJson jsonItemLog = Json::objectValue;

		for (auto itSingleItem = vecSingleItem.begin(); itSingleItem != vecSingleItem.end(); itSingleItem++)
		{
			vector<string>	vecData = CUtil::Split((*itSingleItem), DelimeterKeyValue);;

			if (vecData.empty())
			{
				continue;
			}

			if (CUtil::PatternMatch("type", vecData[KEY])
				|| CUtil::PatternMatch("command", vecData[KEY])
				|| CUtil::PatternMatch("log_time", vecData[KEY])
				|| CUtil::PatternMatch("log", vecData[KEY])
				)
			{
				jsonItem[vecData[KEY]] = vecData[VALUE];
				//LOGD << "ITEM #1: " << jsonItem.toStyledString();
			}
			else
			{
				jsonItem["log"][vecData[KEY]] = vecData[VALUE];
				//LOGD << "ITEM #2: " << jsonItem.toStyledString();
			}
		}

		jsonLogs.append(jsonItem);
	}

	jsonRoot["data"] = jsonLogs;

	//LOGD << "LOG DATA :" << jsonRoot.toStyledString();

	return jsonRoot;
}

string CEDRKernelDelegator::GetName()
{
	return m_strName;
}

BOOL CEDRKernelDelegator::UsePollingMethod()
{
	return m_bUsePollingMethod;
}

CEDRKernelDelegator::CEDRKernelDelegator(
	string name
	, EEDRType eEDRType
	, BOOL bUsePollingMethod
)
{
	m_strName = name;
	m_eType = eEDRType;
	m_logCollectThreadTerminating = FALSE;
	m_logUploadThreadTerminating = FALSE;
	m_bUsePollingMethod = bUsePollingMethod;

	LockGuard::Create(&m_Lock);

	HANDLE hEvent = m_logCollectEvent.GetHandle(); 
	
	BOOL bInit = SafePCKModule::InitDrv(eEDRType);
	LOGW_IF(!bInit) << "@@ 실패 : SafePCKModule::InitDrv(" << m_strName << ")";
	
	BOOL bClear = SafePCKModule::Clear(eEDRType);
	LOGW_IF(!bClear) << "@@ 실패 : SafePCKModule::Clear(" << m_strName << ")";
	
	BOOL bInitClear = SafePCKModule::InitEvent(eEDRType, &hEvent, sizeof(hEvent));
	LOGW_IF(!bInitClear) << "@@ 실패 : SafePCKModule::InitEvent(" << m_strName << ")";

	if (!bInit)
	{
		LOGI << m_strName << " 로그 쓰레드를 초기화 할 수 없습니다.";

		return;
	}

	CEDRWorkerThreadManager *pEDRWorkerThreadManager = CEDRWorkerThreadManager::Instance();

	m_logCollectThreadHandle =
		pEDRWorkerThreadManager->CreateWorkerThread(
		(CHAR*)(string("LogCollect-").append(m_strName).c_str())
		);
	m_logUploadThreadHandle =
		pEDRWorkerThreadManager->CreateWorkerThread(
		(CHAR*)(string("LogUpload-").append(m_strName).c_str())
		);

	pEDRWorkerThreadManager->SetThreadFunc(
		m_logCollectThreadHandle,
		LogCollectFunc
	);

	pEDRWorkerThreadManager->SetThreadFunc(
		m_logUploadThreadHandle,
		LogUploadFunc
	);

	// 더미 데이터
	ThreadCalllbackData_Ptr data = make_shared<ThreadCalllbackData>();
	data->pContext = this;

	ThreadStoppebleData_Ptr ptrThreadLogCollectThreadStoppebleData = 
		make_shared<ThreadStoppebleData>(&m_logCollectEvent, &m_logCollectThreadTerminating);
	ThreadStoppebleData_Ptr ptrThreadLogUploadThreadStoppebleData = 
		make_shared<ThreadStoppebleData>(&m_logUploadEvent, &m_logUploadThreadTerminating);

	pEDRWorkerThreadManager->SetStopperbleEvent(
		m_logCollectThreadHandle
		, ptrThreadLogCollectThreadStoppebleData
	);
	pEDRWorkerThreadManager->SetStopperbleEvent(
		m_logUploadThreadHandle
		, ptrThreadLogUploadThreadStoppebleData
	);

	pEDRWorkerThreadManager->StartThread(m_logCollectThreadHandle);
	pEDRWorkerThreadManager->StartThread(m_logUploadThreadHandle);
	pEDRWorkerThreadManager->InvokeThreadFunc(m_logCollectThreadHandle, data);
	pEDRWorkerThreadManager->InvokeThreadFunc(m_logUploadThreadHandle, data);

	m_logCollectEvent.Reset();
	m_logUploadEvent.Reset();
}

CEDRKernelDelegator::~CEDRKernelDelegator()
{
	LockGuard::Destroy(&m_Lock);

	CEDRWorkerThreadManager *pEDRWorkerThreadManager = CEDRWorkerThreadManager::Instance();
	pEDRWorkerThreadManager->StopAllWorkerThread();

	m_logCollectThreadHandle.reset();
	m_logUploadThreadHandle.reset();

}

