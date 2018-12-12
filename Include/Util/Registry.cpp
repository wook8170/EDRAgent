
#include "stdafx.h"
#include <Winreg.h>
#include "Util/Registry.h"
#include "Util/Util.h"

CRegistry::CRegistry()
{
	m_sRootKey = "HKEY_LOCAL_MACHINE";
	m_hKey = NULL;
	m_nKeyIndex = 0;
	m_nValueIndex = 0;
	m_sPath = StringNull;
	m_eOpt = EnumForNormal;

	m_nKeyIndex = 0;
	m_nValueIndex = 0;
	m_bPrepareEnumKey = FALSE;
	m_bPrepareEnumValue = FALSE;
}

CRegistry::~CRegistry()
{
	Close();
}

LONG CRegistry::Open(
	HKEY hKeyRoot
	, const string& pszPath
	, unsigned long nDesired
)
{
	m_sPath = pszPath;

	return RegOpenKeyEx(
		hKeyRoot
		, pszPath.c_str()
		, 0
		, nDesired
		, &m_hKey
	);
}

LONG CRegistry::Open(
	string sRoot
	, const string& pszPath
	, unsigned long nDesired
)
{
	HKEY rootKey = _GetRootKeyWithString(sRoot);

	return Open(
		rootKey
		, pszPath
		, nDesired
	);
}

LONG CRegistry::Create(
	HKEY hKeyRoot
	, const string& pszPath
)
{
	DWORD dw;
	m_sPath = pszPath;

	return RegCreateKeyEx(
		hKeyRoot
		, pszPath.c_str()
		, 0L
		, NULL
		, REG_OPTION_NON_VOLATILE
		, KEY_ALL_ACCESS
		, NULL
		, &m_hKey
		, &dw
	);
}

LONG CRegistry::Create(
	string sRoot
	, const string& pszPath
)
{
	HKEY rootKey = _GetRootKeyWithString(sRoot);

	DWORD dw;
	m_sPath = pszPath;

	return RegCreateKeyEx(
		rootKey
		, pszPath.c_str()
		, 0L
		, NULL
		, REG_OPTION_NON_VOLATILE
		, KEY_ALL_ACCESS
		, NULL
		, &m_hKey
		, &dw
	);
}

VOID CRegistry::Close()
{
	if (m_hKey)
	{
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}
}

LONG CRegistry::Write(
	const string& pszKey
	, DWORD dwVal
)
{
	CHECK(m_hKey);
	CHECK(!pszKey.empty());

	return RegSetValueEx(
		m_hKey
		, pszKey.c_str()
		, 0L, REG_DWORD
		, (CONST BYTE*) &dwVal
		, sizeof(DWORD)
	);
}

LONG CRegistry::Write(
	const string& pszKey
	, const string& pszData
)
{
	LONG	lReg;

	CHECK(m_hKey);
	CHECK(!pszKey.empty());
	CHECK(!pszData.empty());

	DWORD nCount = (DWORD)pszData.length();
	lReg = RegSetValueEx(
		m_hKey
		, pszKey.c_str()
		, 0L
		, REG_EXPAND_SZ
		, (CONST BYTE*) pszData.c_str()
		, nCount
	);
	return lReg;
}

LONG CRegistry::Write(
	const string& pszKey
	, const BYTE* pData
	, DWORD dwLength
)
{
	CHECK(m_hKey);
	CHECK(!pszKey.empty());
	CHECK(pData && dwLength > 0);

	return RegSetValueEx(
		m_hKey
		, pszKey.c_str()
		, 0L
		, REG_BINARY
		, pData
		, dwLength
	);
}

LONG CRegistry::Read(
	const string& pszKey
	, int  &dwVal
)
{
	return Read(pszKey, (DWORD&)dwVal);
}

LONG CRegistry::Read(
	const string& pszKey
	, DWORD &dwVal
)
{
	CHECK(m_hKey);
	CHECK(!pszKey.empty());

	DWORD dwType;
	DWORD dwSize = sizeof(DWORD);
	DWORD dwDest;

	LONG lRet = RegQueryValueEx(
		m_hKey
		, (LPTSTR)pszKey.c_str()
		, NULL
		, &dwType
		, (BYTE *)&dwDest
		, &dwSize
	);

	if (lRet == ERROR_SUCCESS)
		dwVal = dwDest;

	return lRet;
}

LONG CRegistry::Read(
	const string& pszKey
	, string& sVal
)
{
	CHECK(m_hKey);
	CHECK(!pszKey.empty());

	DWORD	dwType;
	DWORD	dwSize = 200;
	CHAR	szVal[200];

	memset(szVal, 0, sizeof(szVal));

	LONG lReturn = RegQueryValueEx(
		m_hKey
		, (LPTSTR)pszKey.c_str()
		, NULL
		, &dwType
		, (BYTE *)szVal
		, &dwSize
	);

	if (lReturn == ERROR_SUCCESS)
		sVal = szVal;

	return lReturn;
}

LONG CRegistry::Read(
	const string& pszKey
	, BYTE* pData
	, DWORD dwLen
)
{
	CHECK(m_hKey);
	CHECK(!pszKey.empty());

	DWORD	dwType;

	return RegQueryValueEx(
		m_hKey
		, (LPTSTR)pszKey.c_str()
		, NULL
		, &dwType
		, pData
		, &dwLen
	);

}

LONG CRegistry::DeleteKey(
	const string& pszKey
)
{
	return RegDeleteKey(m_hKey, pszKey.c_str());
}

LONG CRegistry::DeleteKey(
	HKEY hKeyRoot
	, const string& pszPath
	, const string& pszKey
)
{
	LONG lReg = Open(hKeyRoot, pszPath);
	if (lReg == ERROR_SUCCESS)
	{
		lReg = DeleteKey(pszKey);
	}
	return lReg;
}

LONG CRegistry::DeleteValue(
	HKEY hKeyRoot
	, const string& pszPath
	, const string& pszValue
)
{
	LONG lReg = Open(hKeyRoot, pszPath);
	if (lReg == ERROR_SUCCESS)
	{
		lReg = DeleteValue(pszValue);
	}
	return lReg;
}

LONG CRegistry::DeleteValue(
	const string& pszValue
)
{
	return RegDeleteValue(m_hKey, pszValue.c_str());
}

LONG CRegistry::FindFirstKey(
	string& sVal
)
{
	m_nKeyIndex = 0;

	DWORD	dwSize = 256;
	CHAR	szVal[256] = { 0, };

	LONG lReturn = RegEnumKeyEx(
		m_hKey
		, (m_eOpt == CRegistry::EnumForNormal) ? m_nKeyIndex++ : 0
		, szVal
		, &dwSize
		, 0
		, NULL
		, NULL
		, NULL
	);

	if (lReturn == ERROR_SUCCESS)
	{
		szVal[dwSize] = NULL;
		sVal = szVal;
	}

	return lReturn;
}

LONG CRegistry::FindNextKey(
	string& sVal
)
{
	DWORD	dwSize = 256;
	CHAR	szVal[256] = { 0, };

	LONG lReturn = RegEnumKeyEx(
		m_hKey
		, (m_eOpt == CRegistry::EnumForNormal) ? m_nKeyIndex++ : 0
		, szVal
		, &dwSize
		, 0
		, NULL
		, NULL
		, NULL
	);

	if (lReturn == ERROR_SUCCESS)
	{
		szVal[dwSize] = NULL;
		sVal = szVal;
	}

	return lReturn;
}

LONG CRegistry::EnumKey(
	string& sVal
)
{
	if (!m_bPrepareEnumKey)
	{
		LOGW << "BeginEnumKey() 호출이 필요 합니다.";
		return ERROR_NOT_READY;
	}

	if (m_nKeyIndex == 0)
	{
		return FindFirstKey(sVal);
	}
	else
	{
		return FindNextKey(sVal);
	}
}


LONG CRegistry::FindFirstValue(
	string& sVal
)
{
	m_nValueIndex = 0;
	CHAR	szVal[256] = { 0, };
	DWORD	cbValueName = 256;
	DWORD	Type = REG_EXPAND_SZ;
	LONG	lReturn = RegEnumValue(
		m_hKey
		, (m_eOpt == CRegistry::EnumForNormal) ? m_nValueIndex++ : 0
		, szVal
		, &cbValueName
		, NULL
		, &Type
		, NULL
		, NULL
	);

	if (lReturn == ERROR_SUCCESS)
	{
		sVal = szVal;
	}
	return lReturn;
}

LONG CRegistry::FindNextValue(
	string& sVal
)
{
	CHAR	szVal[200] = { 0, };
	DWORD	cbValueName = 200;
	DWORD	Type;
	LONG	lReturn = RegEnumValue(
		m_hKey
		, (m_eOpt == CRegistry::EnumForNormal) ? m_nValueIndex++ : 0
		, szVal
		, &cbValueName
		, NULL
		, &Type
		, NULL
		, NULL
	);

	if (lReturn == ERROR_SUCCESS)
	{
		sVal = szVal;
	}
	return lReturn;
}

LONG CRegistry::EnumValue(
	string& sVal
)
{
	if (!m_bPrepareEnumValue)
	{
		LOGW << "BeginEnumValue() 호출이 필요 합니다.";
		return ERROR_NOT_READY;
	}

	if (m_nValueIndex == 0)
	{
		return FindFirstValue(sVal);
	}
	else
	{
		return FindNextValue(sVal);
	}
}

BOOL CRegistry::ExistSubkey(
	string& lpKey
	, string& subkey
)
{
	HKEY	key = NULL;
	BOOL	bResult = FALSE;
	DWORD	rc = ERROR_SUCCESS;
	CHAR	szSubKey[1024] = { 0, };
	DWORD	dwLength = sizeof(szSubKey);
	DWORD	dwDisp = 0;

	CHECK(!lpKey.empty());

	int iIndex = 0;
	rc = RegEnumKeyEx(
		m_hKey
		, iIndex
		, szSubKey
		, &dwLength
		, NULL
		, NULL
		, NULL
		, NULL
	);

	if (rc == ERROR_NO_MORE_ITEMS)
	{
		bResult = FALSE;
	}
	else
	{
		bResult = TRUE;
	}

	RegCloseKey(key);
	return bResult;
}

BOOL CRegistry::ExistValue(
	string& lpKey
	, string& valueName
)
{
	HKEY	key;
	BOOL	bResult = FALSE;
	DWORD	rc = ERROR_SUCCESS;
	CHAR	szValueName[1024] = _T("\0");
	DWORD	dwLength = sizeof(szValueName);

	DWORD dwDisp;
	if (RegCreateKeyEx(
		m_hKey
		, lpKey.c_str()
		, 0
		, NULL
		, REG_OPTION_NON_VOLATILE
		, KEY_READ
		, NULL
		, &key
		, &dwDisp
	) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	int iIndex = 0;
	rc = RegEnumValue(
		key
		, iIndex
		, szValueName
		, &dwLength
		, NULL
		, NULL
		, NULL
		, NULL
	);

	if (rc == ERROR_NO_MORE_ITEMS)
	{
		bResult = FALSE;
	}
	else
	{
		bResult = TRUE;
	}

	RegCloseKey(key);
	return bResult;
}

string& CRegistry::GetPath()
{
	return m_sPath;
}

LONG CRegistry::BeginEnumKey(CRegistry::EEnumOpt opt)
{
	if (m_bPrepareEnumKey)
	{
		LOGW
			<< "이전 BeginEnumKey() 호출 후 EndEnumKey() 가 호출 되지 않았습니다.";
		return ERROR_NOT_READY;
	}

	m_nKeyIndex = 0;
	m_bPrepareEnumKey = TRUE;
	m_eOpt = opt;

	return ERROR_SUCCESS;
}

LONG CRegistry::BeginEnumValue(CRegistry::EEnumOpt opt)
{
	if (m_bPrepareEnumValue)
	{
		LOGW
			<< "이전 BeginEnumValue() 호출 후 EndEnumValue() 가 호출 되지 않았습니다.";
		return ERROR_NOT_READY;
	}

	m_nValueIndex = 0;
	m_bPrepareEnumValue = TRUE;
	m_eOpt = opt;

	return ERROR_SUCCESS;
}

LONG CRegistry::EndEnumKey()
{
	if (!m_bPrepareEnumKey)
	{
		LOGW
			<< "BeginEnumKey() 호출 되지 않았습니다.";
		return ERROR_NOT_READY;
	}
	m_nKeyIndex = 0;
	m_bPrepareEnumKey = FALSE;

	return ERROR_SUCCESS;
}

LONG CRegistry::EndEnumValue()
{
	if (!m_bPrepareEnumValue)
	{
		LOGW
			<< "BeginEnumValue() 호출 되지 않았습니다.";
		return ERROR_NOT_READY;
	}

	m_nValueIndex = 0;
	m_bPrepareEnumValue = FALSE;

	return ERROR_SUCCESS;
}

LONG CRegistry::DeleteSubKey()
{
	return _DeleteSubKey(*this);
}

LONG CRegistry::_DeleteSubKey(CRegistry& reg)
{
	string strKey;

	reg.BeginEnumKey(CRegistry::EnumForDelete);

	while (reg.EnumKey(strKey) == ERROR_SUCCESS && !strKey.empty())
	{
		if (reg.ExistSubkey(reg.GetPath(), strKey))
		{
			CRegistry regSub;

			if (regSub.Open(reg.GetRootKeyString(), reg.GetPath() + PathDelimeter + strKey) != ERROR_SUCCESS)
			{
				continue;
			}

			regSub.DeleteSubValue();

			LOGI
				<< "- Delete Key  : "
				<< regSub.GetPath()
				<< " ("
				<< _DeleteSubKey(regSub)
				<< ") Err: "
				<< GetLastError();

			regSub.Close();

			reg.DeleteKey(strKey);
		}


	}

	reg.EndEnumKey();


	return ERROR_SUCCESS;
}

LONG CRegistry::DeleteSubValue()
{
	return _DeleteSubValue(*this);
}

LONG CRegistry::_DeleteSubValue(CRegistry& reg)
{
	string strValue;
	string strData;

	reg.BeginEnumValue(CRegistry::EnumForDelete);

	while (reg.EnumValue(strValue) == ERROR_SUCCESS && !strValue.empty())
	{
		string path = reg.GetPath();

		LOGI
			<< "- Delete Value: "
			<< reg.GetPath()
			<< PathDelimeter
			<< strValue
			<< " ("
			<< reg.DeleteValue(strValue)
			<< ") Err: "
			<< GetLastError();
	}

	reg.EndEnumValue();

	return ERROR_SUCCESS;
}

HKEY CRegistry::_GetRootKeyWithString(string rootKey)
{
	HKEY key;

	if (CUtil::Compare(rootKey, "HKCR") == 0
		|| CUtil::Compare(rootKey, "HKEY_CLASSES_ROOT") == 0
		)
	{
		key = HKEY_CLASSES_ROOT;
		m_sRootKey = rootKey;
	}
	else if (CUtil::Compare(rootKey, "HKCU") == 0
		|| CUtil::Compare(rootKey, "HKEY_CURRENT_USER") == 0
		)
	{
		key = HKEY_CURRENT_USER;
		m_sRootKey = rootKey;
	}
	else if (CUtil::Compare(rootKey, "HKLM") == 0
		|| CUtil::Compare(rootKey, "HKEY_LOCAL_MACHINE") == 0
		)
	{
		key = HKEY_LOCAL_MACHINE;
		m_sRootKey = rootKey;
	}
	else if (CUtil::Compare(rootKey, "HKU") == 0
		|| CUtil::Compare(rootKey, "HKEY_USERS") == 0
		)
	{
		key = HKEY_USERS;
		m_sRootKey = rootKey;
	}
	else if (CUtil::Compare(rootKey, "HKCC") == 0
		|| CUtil::Compare(rootKey, "HKEY_CURRENT_CONFIG") == 0
		)
	{
		key = HKEY_CURRENT_CONFIG;
		m_sRootKey = rootKey;
	}
	else
	{
		EDRException("Invalid Root key");
	}

	return key;
}

string& CRegistry::GetRootKeyString()
{
	return m_sRootKey;
}

string CRegistry::GetRawPath(string userPath)
{
	string delimeterStr = "\\";
	string strRawPath;
	vector<string> regItem = CUtil::Split(userPath, delimeterStr);

	if (CUtil::Compare(regItem.at(0), "HKCR") == 0
		|| CUtil::Compare(regItem.at(0), "HKEY_CLASSES_ROOT") == 0
		)
	{
		regItem.at(0) = "\\REGISTRY\\MACHINE\\SOFTWARE\\CLASSES";
	}
	else if (CUtil::Compare(regItem.at(0), "HKCU") == 0
		|| CUtil::Compare(regItem.at(0), "HKEY_CURRENT_USER") == 0
		)
	{
		string item = "\\REGISTRY\\USER";
		item.append(CUtil::GetCurrentSID());
		regItem.at(0) = item;
	}
	else if (CUtil::Compare(regItem.at(0), "HKLM") == 0
		|| CUtil::Compare(regItem.at(0), "HKEY_LOCAL_MACHINE") == 0
		)
	{
		regItem.at(0) = "\\REGISTRY\\MACHINE";
	}
	else if (CUtil::Compare(regItem.at(0), "HKU") == 0
		|| CUtil::Compare(regItem.at(0), "HKEY_USERS") == 0
		)
	{
		regItem.at(0) = "\\REGISTRY\\USER";
	}
	/*
	(
	else if (CUtil::Compare(regItem.at(0), "HKCC") == 0
	|| CUtil::Compare(regItem.at(0), "HKEY_CURRENT_CONFIG") == 0
	)
	{
	}
	*/
	strRawPath = StringNull;

	for (auto it = regItem.begin(); it != regItem.end(); it++)
	{
		if (it != regItem.begin())
		{
			strRawPath.append(StringBackslash);
		}

		strRawPath.append(*it);
	}

	LOGI << "* 커널 경로: " << strRawPath;

	return strRawPath;
}

string CRegistry::GetUserPath(string rawPath)
{
	string delimeterStr = StringBackslash;
	string strUserPath;
	vector<string> regItem = CUtil::Split(rawPath, delimeterStr);

	int startIndex = 0;

	if (CUtil::Compare(regItem.at(1), "REGISTRY") == 0
		&& CUtil::Compare(regItem.at(2), "MACHINE") == 0
		&& CUtil::Compare(regItem.at(3), "SOFTWARE") == 0
		&& CUtil::Compare(regItem.at(4), "CLASSES") == 0
		)
	{
		regItem.at(0) = "\\HKCR";
		startIndex = 5;
	}
	else if (CUtil::Compare(regItem.at(1), "REGISTRY") == 0
		&& CUtil::Compare(regItem.at(2), "USER") == 0
		&& CUtil::Compare(regItem.at(3), CUtil::GetCurrentSID()) == 0
		)
	{
		regItem.at(0) = "\\HKCU";
		startIndex = 4;
	}
	else if (CUtil::Compare(regItem.at(1), "REGISTRY") == 0
		&& CUtil::Compare(regItem.at(2), "MACHINE") == 0
		)
	{
		regItem.at(0) = "\\HKLM";
		startIndex = 3;
	}
	else if (CUtil::Compare(regItem.at(1), "HKU") == 0
		&& CUtil::Compare(regItem.at(2), "HKEY_USERS") == 0
		&& CUtil::Compare(regItem.at(3), "SOFTWARE") != 0
		&& CUtil::Compare(regItem.at(4), "CLASSES") != 0
		)
	{
		regItem.at(0) = "\\HKU";
		startIndex = 3;
	}
	/*
	(
	else if (CUtil::Compare(regItem.at(0), "HKCC") == 0
	|| CUtil::Compare(regItem.at(0), "HKEY_CURRENT_CONFIG") == 0
	)
	{
	}
	*/
	strUserPath = regItem.at(0);

	INT currentIndex = 0;
	for (auto it = regItem.begin(); it != regItem.end(); it++, currentIndex++)
	{
		if (currentIndex >= startIndex)
		{
			if (currentIndex > startIndex)
			{
				strUserPath.append(StringBackslash);
			}

			strUserPath.append(*it);
		}
	}

	LOGI << "* 유저 경로: " << strUserPath;

	return strUserPath;
}
