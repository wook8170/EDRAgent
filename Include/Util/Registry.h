#pragma once

class CRegistry
{

public:
	typedef enum
	{
		EnumForDelete = 0,
		EnumForNormal,
	} EEnumOpt;

	CRegistry();

	~CRegistry();

public:
	LONG Open(
		HKEY hKeyRoot
		, const string& pszPath
		, unsigned long nDesired = KEY_ALL_ACCESS
	);

	LONG Open(
		string sRoot
		, const string& pszPath
		, unsigned long nDesired = KEY_ALL_ACCESS
	);

	LONG CRegistry::Create(
		string sRoot
		, const string& pszPath
	);

	LONG Create(
		HKEY hKeyRoot
		, const string& pszPath
	);

	VOID Close();

	LONG Write(
		const string& pszKey
		, DWORD dwVal
	);

	LONG Write(
		const string& pszKey
		, const string& pszVal
	);

	LONG Write(
		const string& pszKey
		, const BYTE* pData
		, DWORD dwLength
	);

	LONG Read(
		const string& pszKey
		, DWORD &dwVal
	);

	LONG Read(
		const string& pszKey
		, int &dwVal
	);

	LONG Read(
		const string& pszKey
		, string& sVal
	);

	LONG Read(
		const string& pszKey
		, BYTE* pData
		, DWORD dwLength
	);

	LONG FindFirstKey(
		string& sVal
	);

	LONG FindNextKey(
		string& sVal
	);

	LONG EnumKey(
		string& sVal
	);

	LONG FindFirstValue(
		string& sVal
	);

	LONG FindNextValue(
		string& sVal
	);

	LONG EnumValue(
		string& sVal
	);

	BOOL ExistSubkey(
		string& lpKey
		, string& subkey
	);

	BOOL ExistValue(
		string& lpKey
		, string& valueName
	);

	LONG DeleteValue(
		const string& pszValue
	);
	LONG DeleteValue(
		HKEY hKeyRoot
		, const string& pszPath
		, const string& pszValue
	);

	LONG DeleteKey(
		const string& pszKey
	);

	LONG DeleteKey(
		HKEY hKeyRoot
		, const string& pszPath
		, const string& pszKey
	);

	string& GetPath();

	LONG BeginEnumKey(
		CRegistry::EEnumOpt opt = CRegistry::EnumForNormal
	);

	LONG BeginEnumValue(
		CRegistry::EEnumOpt opt = CRegistry::EnumForNormal
	);

	LONG EndEnumKey();

	LONG EndEnumValue();

	LONG DeleteSubKey();

	LONG DeleteSubValue();

	string& GetRootKeyString();

	static string GetRawPath(
		string userPath
	);

	static string GetUserPath(
		string rawPath
	);

private:
	LONG _DeleteSubKey(
		CRegistry& reg
	);

	LONG _DeleteSubValue(
		CRegistry& reg
	);

	HKEY _GetRootKeyWithString(
		string rootKey
	);

protected:
	string	m_sRootKey;
	HKEY 	m_hKey;
	string	m_sPath;
	int		m_nKeyIndex;
	int		m_nValueIndex;
	BOOL	m_bPrepareEnumKey;
	BOOL	m_bPrepareEnumValue;

	EEnumOpt m_eOpt;
};

