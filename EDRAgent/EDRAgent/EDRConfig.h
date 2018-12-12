#pragma once

#include "CommonDef.h"

class CEDRConfig
{
public:
	typedef enum
	{
		EEDRConfigInvalid = -1,
		EEDRConfigIOC = 0,
		EEDRConfigRES,
		EEDRConfigMAX,
	} EEDRConfig;


private:
	EDRJson			m_configJson[EEDRConfigMAX];
	EDRJson			m_JsonRoot[EEDRConfigMAX];

	EEDRConfig		m_eCurrentConfig;

protected:
	CEDRConfig();

public:
	static CEDRConfig* Instance();

	vector<string> GetTypes(
		EEDRConfig eConfig = EEDRConfigInvalid
	);

	vector<string> GetAllAvailableTypes(
	);

	vector<INT> GetAllAvailableCodes(
	);

	EDRJson GetRootItem(
		EEDRConfig eConfig = EEDRConfigInvalid
	);

	EDRJson GetItem(
		string type
		, EEDRConfig eConfig = EEDRConfigInvalid
	);

	VOID SetCurrentConfig(
		EEDRConfig eConfig = EEDRConfigInvalid
	);

	string GetHelperName(
		EEDRConfig eConfig = EEDRConfigInvalid
	);

	string GetNormalURL(
		EEDRConfig eConfig = EEDRConfigInvalid
	);

	string GetRefreshURL(
		EEDRConfig eConfig = EEDRConfigInvalid
	);

	string GetLogURL(
		EEDRConfig eConfig = EEDRConfigInvalid
	);


	BOOL UsePolicy(
		string type
		, EEDRConfig eConfig = EEDRConfigInvalid
	);

	/*
	string GetType(
		string type
		, EEDRConfig eConfig = EEDRConfigInvalid
	);
	*/

	string GetSubURL(
		string type
		, EEDRConfig eConfig = EEDRConfigInvalid
	);

	string GetName(
		string type
		, EEDRConfig eConfig = EEDRConfigInvalid
	);

	vector<string> GetKeys(
		string type
		, EEDRConfig eConfig = EEDRConfigInvalid
	);

	INT GetCode(
		string type
		, EEDRConfig eConfig = EEDRConfigInvalid
	);

	INT GetLogBufferCount(
		string type
		, EEDRConfig eConfig = EEDRConfigInvalid
	);

	INT GetLogBufferCount(
		INT code
	);

	BOOL IsUnicodeKernel(
		string type
		, EEDRConfig eConfig = EEDRConfigInvalid
	);

	BOOL IsUnicodeKernel(
		INT code
	);

	BOOL UsePollingMethod4Log(
		INT code
	);
};