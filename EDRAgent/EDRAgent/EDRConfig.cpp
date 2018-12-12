#include "StdAfx.h"
#include "EDRConfig.h"
#include "Util/FileSystem.h"
#include "Util/Util.h"
#include "EDRAgentDll.h"

#ifdef EDRAGENT_EXPORTS
#include "../EDRAgentDll/EDRAgentKernelHandle.h"
#endif

CEDRConfig::CEDRConfig()
{
	File			fileIOCMeta(IOCMetaName);
	File			fileRESMeta(RESMetaName);
	EDRJsonReader	jsonReader;
	string			strMetaData;
	BOOL			bRet = FALSE;
	
	CHECK(fileIOCMeta.Open(File::in, File::text));
	CHECK(fileRESMeta.Open(File::in, File::text));

	strMetaData = fileIOCMeta.ReadAll(TRUE);
	bRet = jsonReader.parse(strMetaData.c_str(), m_configJson[EEDRConfigIOC]);
	if (!bRet)
	{
		LOGW << "* Json 파일 ( "
			<< IOCMetaName
			<< " ) 파싱 오류 :"
			<< jsonReader.getFormattedErrorMessages();;
	}

	m_JsonRoot[EEDRConfigIOC] = m_configJson[EEDRConfigIOC][JsonField_Root];

	strMetaData = fileRESMeta.ReadAll(TRUE);
	bRet = jsonReader.parse(strMetaData.c_str(), m_configJson[EEDRConfigRES]);
	if (!bRet)
	{
		LOGW << "* Json 파일 ( "
			<< RESMetaName
			<< " ) 파싱 오류 :"
			<< jsonReader.getFormattedErrorMessages();;
	}

	m_JsonRoot[EEDRConfigRES] = m_configJson[EEDRConfigRES][JsonField_Root];
}

vector<string> CEDRConfig::GetTypes(
	EEDRConfig eConfig
)
{
	vector<string> vecRet;
	EDRJson item;

	for (auto it = m_JsonRoot[eConfig].begin(); it != m_JsonRoot[eConfig].end(); it++)
	{
		item = (*it);

		DPRINT_JSON(item);

		if (item.isObject() && item[JsonField_Type].isString() )
		{
			//if (UsePolicy(item[JsonField_Type].asString(), eConfig))
			//{
				vecRet.push_back(item[JsonField_Type].asString());
			//}			
		}
	}

	return vecRet;
}

vector<string> CEDRConfig::GetAllAvailableTypes()
{
	map<string, string> mapTypes;

	vector<string> vecTypesIOC = GetTypes(EEDRConfigIOC);
	
	for (auto it = vecTypesIOC.begin(); it != vecTypesIOC.end(); it++)
	{
		if(mapTypes.find(*it) == mapTypes.end())
			mapTypes.insert(pair<string, string>((*it), (*it)));
	}	
	
	vector<string> vecTypesRES = GetTypes(EEDRConfigIOC);

	for (auto it = vecTypesIOC.begin(); it != vecTypesIOC.end(); it++)
	{
		if (mapTypes.find(*it) == mapTypes.end())
			mapTypes.insert(pair<string, string>((*it), (*it)));
	}

	vector<string> vecTypes;

	for (auto it = mapTypes.begin(); it != mapTypes.end(); it++)
	{
		vecTypes.push_back((*it).second);
	}

	return vecTypes;
}

vector<INT> CEDRConfig::GetAllAvailableCodes()
{
	INT codes[(INT)EDR_TYPE_MAX];

	for (INT i = 0; i<(INT)EDR_TYPE_MAX; i++)
	{
		codes[(INT)i] = (INT)EDR_TYPE_INVALID;
	}

	vector<string> vecTypesIOC = GetTypes(EEDRConfigIOC);

	for (auto it = vecTypesIOC.begin(); it != vecTypesIOC.end(); it++)
	{
		INT code = GetCode((*it), EEDRConfigIOC);

		if (codes[code] == EDR_TYPE_INVALID)
		{
			codes[code] = code;
		}
	}

	vector<string> vecTypesRES = GetTypes(EEDRConfigIOC);

	for (auto it = vecTypesIOC.begin(); it != vecTypesIOC.end(); it++)
	{
		INT code = GetCode((*it), EEDRConfigIOC);

		if (codes[code] == EDR_TYPE_INVALID)
		{
			codes[code] = code;
		}
	}

	vector<INT> vecTypes;

	for (auto i = 0; i<EDR_TYPE_MAX; i++)
	{
		vecTypes.push_back(codes[i]);
	}

	return vecTypes;
}

EDRJson CEDRConfig::GetRootItem(
	EEDRConfig eConfig
)
{
	return m_JsonRoot[eConfig];
}

EDRJson CEDRConfig::GetItem(
	string type
	, EEDRConfig eConfig
)
{
	EDRJson json;

	for (auto it = m_JsonRoot[eConfig].begin(); it != m_JsonRoot[eConfig].end(); it++)
	{
		json = Json::objectValue;

		if (it->isObject())
		{
			if (CUtil::Compare((*it)[JsonField_Type].asString(), type) == 0)
			{
				DPRINT_JSON((*it));
				json = (*it);
				break;
			}
			else
			{
				continue;
			}
		}
	}

	return json;
}

CEDRConfig* CEDRConfig::Instance()
{
	static CEDRConfig * pInstance = NULL;
	if (NULL == pInstance)
	{
		static CCriticalSection criticalSection;
		criticalSection.Lock();
		if (NULL == pInstance)
		{
			static CEDRConfig RealInstance;
			pInstance = &RealInstance;
		}
		criticalSection.Unlock();
	}

	return pInstance;
}

VOID CEDRConfig::SetCurrentConfig(
	EEDRConfig eConfig
)
{
	if (eConfig == EEDRConfigInvalid)
	{
		m_eCurrentConfig = EEDRConfigIOC;
	}

	m_eCurrentConfig = eConfig;
}

string CEDRConfig::GetHelperName(
	EEDRConfig eConfig
)
{
	return m_configJson[eConfig][JsonField_HelperName].asString();
}

string CEDRConfig::GetNormalURL(
	EEDRConfig eConfig
)
{
	return m_configJson[eConfig][JsonField_NormalUrl].asString();
}

string CEDRConfig::GetRefreshURL(
	EEDRConfig eConfig
)
{
	return m_configJson[eConfig][JsonField_RefreshUrl].asString();
}

string CEDRConfig::GetLogURL(
	EEDRConfig eConfig
)
{
	return m_configJson[EEDRConfigIOC][JsonField_LogUrl].asString();
}

BOOL CEDRConfig::UsePolicy(
	string type
	, EEDRConfig eConfig
)
{
	string	value = "";
	EDRJson item = GetItem(type, eConfig);

	DPRINT_JSON(item);

	if (item.isObject())
	{
		if (!item[JsonField_UsePolicy].empty())
		{
			value = (!item[JsonField_UsePolicy].empty()) ? item[JsonField_UsePolicy].asString() : StringNull;
		}
	}

	return (CUtil::Compare(value, "true") == 0 ? TRUE : FALSE);
}

/*
string CEDRConfig::GetType(
	string type
	, EEDRConfig eConfig = EEDRConfigInvalid
)
{
	string	value;

	SetCurrentConfig(eConfig);

	EDRJson item = GetItem(type, m_eCurrentConfig);

	value = "";

	DPRINT_JSON(item);

	if (item.isObject())
	{
		if (!item[JsonField_Type].empty())
		{
			value = (!item[JsonField_Type].empty()) ? item[JsonField_Type].asString() : StringNull;
		}
	}

	return value;
}
*/

string CEDRConfig::GetSubURL(
	string type
	, EEDRConfig eConfig
)
{
	string	value;

	EDRJson item = GetItem(type, eConfig);

	value = "";

	DPRINT_JSON(item);

	if (item.isObject())
	{
		if (!item[JsonField_SubUrl].empty())
		{
			value = (!item[JsonField_SubUrl].empty()) ? item[JsonField_SubUrl].asString() : StringNull;
		}
	}

	return value;

}

string CEDRConfig::GetName(
	string type
	, EEDRConfig eConfig
)
{
	string	value;
	EDRJson item = GetItem(type, eConfig);

	value = "";

	DPRINT_JSON(item);

	if (item.isObject())
	{
		if (!item[JsonField_Name].empty())
		{
			value = (!item[JsonField_Name].empty()) ? item[JsonField_Name].asString() : StringNull;
		}
	}

	return value;
}

vector<string> CEDRConfig::GetKeys(
	string type
	, EEDRConfig eConfig
)
{
	vector<string>	 vecRet;
	EDRJson item = GetItem(type, eConfig)[JsonField_Key];
	EDRJson subItem = Json::objectValue;

	if (item.isArray())
	{
		subItem = Json::objectValue;

		for (auto it = item.begin(); it != item.end(); it++)
		{
			subItem = (*it);

			DPRINT_JSON(subItem);

			if (subItem.isString())
			{
				vecRet.push_back(subItem.asString());
			}
		}
	}
	else
	{
		if (item.isString())
		{
			vecRet.push_back(item.asString());
		}
	}

	return vecRet;
}

INT CEDRConfig::GetCode(
	string type
	, EEDRConfig eConfig
)
{
	INT		value = EDR_TYPE_INVALID;
	EDRJson item = GetItem(type, eConfig);

	if (item.isObject())
	{
		if (!item[JsonField_Code].empty())
		{
			value = (!item[JsonField_Code].empty()) ? item[JsonField_Code].asInt() : EDR_TYPE_INVALID;
		}
	}

	return value;
}

INT CEDRConfig::GetLogBufferCount(
	string type
	, EEDRConfig eConfig
)
{
	INT		value = 10;
	EDRJson item = GetItem(type, eConfig);

	if (item.isObject())
	{
		if (!item[JsonField_LogBufferCount].empty())
		{
			value = (!item[JsonField_LogBufferCount].empty()) ? item[JsonField_LogBufferCount].asInt() : 10;
		}
	}

	return value;
}

INT CEDRConfig::GetLogBufferCount(
	INT code
)
{
	EDRJson itemsIOC = GetRootItem(EEDRConfigIOC);
	EDRJson itemsRES = GetRootItem(EEDRConfigRES);

	INT iocbufferSize = 10; // 기본
	INT resBufferSize = 10;	// 기본

	for (auto it = itemsIOC.begin(); it != itemsIOC.end(); it++)
	{
		EDRJson item = (*it);

		if(item[JsonField_Code].asInt() == code)		
		{
			iocbufferSize = item[JsonField_LogBufferCount].asInt();
			break;
		}		
	}

	for (auto it = itemsRES.begin(); it != itemsRES.end(); it++)
	{
		EDRJson item = (*it);

		if(item[JsonField_Code].asInt() == code)
		{
			resBufferSize = item[JsonField_LogBufferCount].asInt();
			break;
		}		
	}

	return max(iocbufferSize, resBufferSize);
}

BOOL CEDRConfig::IsUnicodeKernel(
	string type
	, EEDRConfig eConfig
)
{
	BOOL	bRet = FALSE;
	string	value = "";
	EDRJson item = GetItem(type, eConfig);

	if (item.isObject())
	{
		if (!item[JsonField_UnicodeKernel].empty())
		{
			value = (!item[JsonField_UnicodeKernel].empty()) ? item[JsonField_UnicodeKernel].asString() : "FALSE";
		}
	}

	return (CUtil::Compare(value, "true") == 0 ? TRUE : FALSE);

}

BOOL CEDRConfig::IsUnicodeKernel(
	INT code
)
{
	EDRJson itemsIOC = GetRootItem(EEDRConfigIOC);
	EDRJson itemsRES = GetRootItem(EEDRConfigRES);

	string iocUnicode = "false";	// 기본
	string resUnicode = "false";	// 기본

	for (auto it = itemsIOC.begin(); it != itemsIOC.end(); it++)
	{
		EDRJson item = (*it);

		if (!item[JsonField_Code].empty() && item[JsonField_Code].isInt() && item[JsonField_Code].asInt() == code)
		{
			iocUnicode = item[JsonField_UnicodeKernel].asString();
			break;
		}		
	}

	for (auto it = itemsRES.begin(); it != itemsRES.end(); it++)
	{
		EDRJson item = (*it);

		if (!item[JsonField_Code].empty() && item[JsonField_Code].isInt() && item[JsonField_Code].asInt() == code)
		{
			resUnicode = item[JsonField_UnicodeKernel].asString();
			break;
		}
	}

	return (CUtil::Compare(iocUnicode, "true") == 0 || CUtil::Compare(resUnicode, "true") == 0);
}

BOOL CEDRConfig::UsePollingMethod4Log(
	INT code
)
{
	EDRJson itemsIOC = GetRootItem(EEDRConfigIOC);
	EDRJson itemsRES = GetRootItem(EEDRConfigRES);

	string iocUnicode = "false";	// 기본
	string resUnicode = "false";	// 기본

	for (auto it = itemsIOC.begin(); it != itemsIOC.end(); it++)
	{
		EDRJson item = (*it);

		if (!item[JsonField_Code].empty() && item[JsonField_Code].isInt() && item[JsonField_Code].asInt() == code)
		{
			iocUnicode = item[JsonField_LogMethod].asString();
			break;
		}
	}

	for (auto it = itemsRES.begin(); it != itemsRES.end(); it++)
	{
		EDRJson item = (*it);

		if (!item[JsonField_Code].empty() && item[JsonField_Code].isInt() && item[JsonField_Code].asInt() == code)
		{
			resUnicode = item[JsonField_LogMethod].asString();
			break;
		}
	}

	return (CUtil::Compare(iocUnicode, "polling") == 0 || CUtil::Compare(resUnicode, "polling") == 0);
}