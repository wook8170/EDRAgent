#include "StdAfx.h"
#include "Util.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <tchar.h>
#include <string.h>
#include <cstring>
#include <cstdarg>

#include <WinSock.h>
#include <IPHlpApi.h>                       
#pragma comment(lib, "iphlpapi.lib" )

#include "openssl/sha.h"
#include "openssl/md5.h"

#define MAX_CHECK_SUM_FILE_SIZE			5242880

const string  CUtil::GetUserIP()
{
	WSADATA wsa_Data;
	int wsa_ReturnCode = WSAStartup(0x101, &wsa_Data);

	CHAR szHostName[255];
	gethostname(szHostName, 255);
	struct hostent *host_entry;
	host_entry = gethostbyname(szHostName);
	CHAR * szLocalIP;
	szLocalIP = inet_ntoa(
		*(struct in_addr *)*host_entry->h_addr_list
	);
	WSACleanup();

	return szLocalIP;
}

const string  CUtil::GetUserMAC()
{
	CHAR strMac[256] = { NULL, };
	DWORD size = sizeof(PIP_ADAPTER_INFO);

	PIP_ADAPTER_INFO Info;
	ZeroMemory(&Info, size);
	int result = GetAdaptersInfo(Info, &size);        
	if (result == ERROR_BUFFER_OVERFLOW)
	{
		Info = (PIP_ADAPTER_INFO)malloc(size);
		GetAdaptersInfo(Info, &size);
	}
	if (!Info)
	{
		return strMac;
	}

	sprintf_s(
		strMac
		, sizeof(strMac)
		, "%0.2X-%0.2X-%0.2X-%0.2X-%0.2X-%0.2X"
		, Info->Address[0]
		, Info->Address[1]
		, Info->Address[2]
		, Info->Address[3]
		, Info->Address[4]
		, Info->Address[5]
	);

	return strMac;
}

const string  CUtil::GetUserID()
{
	return "dev01";
}

const string  CUtil::GetAckMessage(
	BOOL status
)
{
	return (status) ?
		"Successfully Processed" :
		"Failure Processed";
}

const INT CUtil::GetAckCode(
	BOOL status
)
{
	return (status) ? 200 : 404;
}

const string  CUtil::CurrentDateTime()
{
	time_t     now = time(0);
	struct tm  tstruct;
	CHAR       buf[80];
	localtime_s(&tstruct, &now);
	strftime(
		buf
		, sizeof(buf)
		, "%Y.%m.%d %X:51",
		&tstruct
	);

	return buf;
}

const string  CUtil::GetSha256(
	string  filepath
)
{
	FILE *file;
	fopen_s(&file, filepath.c_str(), "rb");

	errno_t err = 0;

	if (err != 0 || !file) return StringNull;

	UCHAR	hash[SHA256_DIGEST_LENGTH * 2] = { 0x00, };
	CHAR	mdtstring[SHA256_DIGEST_LENGTH * 2 + 1] = { 0x00, };

	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	const int bufSize = 32768;
	byte *buffer = (byte*)malloc(bufSize);
	size_t bytesRead = 0;
	if (!buffer) return StringNull;
	while ((bytesRead = fread(buffer, 1, bufSize, file)))
	{
		SHA256_Update(&sha256, buffer, bytesRead);
	}
	SHA256_Final(hash, &sha256);

	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		//sprintf(mdtstring + (i * 2), "%02X", hash[i]);
		sprintf_s(mdtstring + (i * 2), sizeof(mdtstring) - (i * 2), "%02X", hash[i]);
	}

	mdtstring[SHA256_DIGEST_LENGTH * 2] = 0;

	fclose(file);
	free(buffer);

	return mdtstring;
}

const string  CUtil::GetMD5(
	string  filepath
)
{
	FILE *file;
	fopen_s(&file, filepath.c_str(), "rb");

	errno_t err = 0;

	if (err != 0 || !file) return StringNull;

	UCHAR	hash[MD5_DIGEST_LENGTH * 2] = { 0x00, };
	CHAR	mdtstring[MD5_DIGEST_LENGTH * 2 + 1] = { 0x00, };

	MD5_CTX sha256;
	MD5_Init(&sha256);
	const int bufSize = 32768;
	byte *buffer = (byte*)malloc(bufSize);
	size_t bytesRead = 0;
	if (!buffer) return StringNull;
	while ((bytesRead = fread(buffer, 1, bufSize, file)))
	{
		MD5_Update(&sha256, buffer, bytesRead);
	}
	MD5_Final(hash, &sha256);

	for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
	{
		//sprintf(mdtstring + (i * 2), "%02X", hash[i]);
		sprintf_s(mdtstring + (i * 2), sizeof(mdtstring) - (i * 2), "%02X", hash[i]);

	}

	mdtstring[MD5_DIGEST_LENGTH * 2] = 0;

	fclose(file);
	free(buffer);

	return mdtstring;
}


const wstring CUtil::ConvM2U(
	string multibyte
)
{
	string message_a = multibyte;
	wstring message_w;

	USES_CONVERSION;

	message_w = wstring(A2W(multibyte.c_str()));
	
	return message_w;
}

const string CUtil::ConvU2M(
	wstring unicode
)
{
	wstring message_w = unicode;
	string message_a;

	USES_CONVERSION;

	message_a = string(W2A(unicode.c_str()));

	return message_a;
}

VOID CUtil::Upper(
	string&  str
)
{
	string::iterator it;
	int i;
	for (i = 0;i < str.size();++i) {
		((CHAR *)(VOID *)str.data())[i] = toupper(((CHAR *)str.data())[i]);
	}
}

VOID CUtil::Lower(
	string&  str
)
{
	string::iterator it;
	int i;
	for (i = 0;i < str.size();++i) {
		((CHAR *)(VOID *)str.data())[i] = tolower(((CHAR *)str.data())[i]);
	}
}

INT CUtil::Compare(
	string  str
	, string str2
)
{
	string src = str.c_str();
	string dst = str2.c_str();

	CUtil::Lower(src);
	CUtil::Lower(dst);

	return src.compare(dst);
}

string CUtil::UTF8ToAnsi(
	string str
)
{
	int length = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size() + 1, 0, 0);
	if (0 < length) {
		std::wstring wtemp(length, (wchar_t)0);
		MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size() + 1, &wtemp[0], length);
		length = WideCharToMultiByte(CP_ACP, 0, &wtemp[0], -1, 0, 0, 0, 0);
		std::string temp(length, (CHAR)0);
		WideCharToMultiByte(CP_ACP, 0, &wtemp[0], -1, &temp[0], length, 0, 0);
		return temp;
	}
	return StringNull;
}

CHAR* CUtil::AnsiToUTF8(
	const CHAR * pszCode
)
{
	int     nLength, nLength2;
	BSTR    bstrCode;
	char*   pszUTFCode = NULL;

	nLength = MultiByteToWideChar(CP_ACP, 0, pszCode, lstrlen(pszCode), NULL, NULL);
	bstrCode = SysAllocStringLen(NULL, nLength);
	MultiByteToWideChar(CP_ACP, 0, pszCode, lstrlen(pszCode), bstrCode, nLength);

	nLength2 = WideCharToMultiByte(CP_UTF8, 0, bstrCode, -1, pszUTFCode, 0, NULL, NULL);
	pszUTFCode = (char*)malloc(nLength2 + 1);
	WideCharToMultiByte(CP_UTF8, 0, bstrCode, -1, pszUTFCode, nLength2, NULL, NULL);
	SysFreeString(bstrCode);

	return pszUTFCode;
}

vector<string> CUtil::Split(
	const string& s
	, string& seperator
)
{
	std::vector<std::string> output;
	std::string::size_type prev_pos = 0, pos = 0;

	while ((pos = s.find(seperator, pos)) != std::string::npos)
	{
		std::string substring(s.substr(prev_pos, pos - prev_pos));
		output.push_back(substring);
		prev_pos = ++pos;
	}

	output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word

	return output;
}

string CUtil::Replace(
	string& src
	, string& from
	, string& to
)
{
	size_t start_pos = 0;
	while ((start_pos = src.find(from, start_pos)) != std::string::npos)
	{
		src.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return src;
}

BOOL CUtil::PatternMatch(string str, string pattern)
{
	enum State
	{
		Exact,      	// exact match
		Any,        	// ?
		AnyRepeat    	// *
	};

	const CHAR *s = str.c_str();
	const CHAR *p = pattern.c_str();
	const CHAR *q = 0;
	INT state = 0;
	BOOLEAN match = TRUE;

	if (std::strchr(p, '*') == NULL && strrchr(p, CharQuestion) == NULL)
	{
		return (std::strstr(s, p) != NULL);
	}

	while (match && *p)
	{
		if (*p == '*')
		{
			state = AnyRepeat;
			q = p + 1;
		}
		else if (*p == CharQuestion)
		{
			state = Any;
		}
		else
		{
			state = Exact;
		}

		if (*s == 0)
		{
			break;
		}

		switch (state)
		{
		case Exact:
			match = *s == *p;
			s++;
			p++;
			break;

		case Any:
			match = TRUE;
			s++;
			p++;
			break;

		case AnyRepeat:
			match = TRUE;
			s++;

			if (*s == *q) p++;
			break;
		}
	}

	if (state == AnyRepeat)
	{
		return (BOOLEAN)(*s == *q);
	}
	else if (state == Any)
	{
		return (BOOLEAN)(*s == *p);
	}
	else
	{
		return match && (BOOLEAN)(*s == *p);
	}
}

string CUtil::_format_arg_list(const CHAR *fmt, va_list args)
{
	if (!fmt) return "";
	int   result = -1, length = 256;
	char *buffer = 0;
	while (result == -1)
	{
		if (buffer) delete[] buffer;
		buffer = new char[length + 1];
		memset(buffer, 0, length + 1);
		//result = _vsnprintf(buffer, length, fmt, args);
		result = vsnprintf_s(buffer, length, _TRUNCATE, fmt, args);
		length *= 2;
	}
	string s(buffer);
	delete[] buffer;
	return s;
}


string CUtil::Format(const CHAR* fmt, ...)
{
#if 0
	INT size = 512;
	CHAR* buffer = 0;
	buffer = EDRNew CHAR[size];
	va_list vl;
	va_start(vl, fmt);
	INT nsize = vsnprintf(buffer, size, fmt, vl);

	if (size <= nsize)
	{
		delete[] buffer;
		buffer = 0;
		buffer = EDRNew CHAR[nsize + 1];
		nsize = vsnprintf(buffer, size, fmt, vl);
	}

	std::string ret(buffer);
	va_end(vl);
	delete[] buffer;
	return ret;
#endif
	va_list args;
	va_start(args, fmt);
	string s = CUtil::_format_arg_list(fmt, args);
	va_end(args);
	return s;


}

#include <Sddl.h>
#include <stdio.h>
string CUtil::GetMachineUserName()
{
	HANDLE hTok = NULL;
	string machineUsername = StringNull;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTok))
	{
		LPBYTE buf = NULL;
		DWORD  dwSize = 0;
		GetTokenInformation(hTok, TokenUser, NULL, 0, &dwSize);
		if (dwSize)
			buf = (LPBYTE)LocalAlloc(LPTR, dwSize);

		if (GetTokenInformation(hTok, TokenUser, buf, dwSize, &dwSize))
		{
			PSID pSid = ((PTOKEN_USER)buf)->User.Sid;

			CHAR user[20], domain[20];
			DWORD cbUser = 20, cbDomain = 20;
			SID_NAME_USE nu;
			LookupAccountSid(NULL, pSid, user, &cbUser, domain, &cbDomain, &nu);
			machineUsername.append(domain).append("\\").append(user);
		}
		LocalFree(buf);
		CloseHandle(hTok);
	}

	return machineUsername;
}

string CUtil::GetCurrentSID()
{
	string machineUsername = CUtil::GetMachineUserName();
	LPTSTR sidstring;
	string strSID;

	LPCTSTR wszAccName = machineUsername.c_str();
	LPTSTR wszDomainName = (LPTSTR)GlobalAlloc(GPTR, sizeof(CHAR) * 1024);
	DWORD cchDomainName = 1024;
	SID_NAME_USE eSidType;
	char sid_buffer[1024];
	DWORD cbSid = 1024;
	SID * sid = (SID *)sid_buffer;

	if (!LookupAccountName(NULL, wszAccName, sid_buffer, &cbSid, wszDomainName, &cchDomainName, &eSidType)) {
		return StringNull;
	}

	if (!ConvertSidToStringSid(sid, &sidstring)) {
		return StringNull;
	}

	strSID = sidstring;

	return strSID;
}

BOOL CUtil::Is64BitOS()
{
	BOOL bResult = FALSE;
	SYSTEM_INFO si;
	typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
	PGNSI pGNSI = NULL;

	pGNSI = (PGNSI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
	if (NULL != pGNSI)
	{
		pGNSI(&si);
	}
	else
	{
		GetSystemInfo(&si);
	}

	if (PROCESSOR_ARCHITECTURE_IA64 == si.wProcessorArchitecture)
	{
		bResult = TRUE;
	}
	else if (PROCESSOR_ARCHITECTURE_AMD64 == si.wProcessorArchitecture)
	{

		bResult = TRUE;

	}

	return bResult;
}

#include "Util/Process.h"

EDRJson CUtil::ParseLogData(
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
			}
			else
			{
				if (CUtil::PatternMatch("hash", vecData[KEY]))
				{
					string hash = "";
					if (vecData[VALUE].length() == 32)
					{
						hash.append("MD5=");
					}
					else if (vecData[VALUE].length() == 40)
					{
						hash.append("SHA1=");
					}
					else
					{
						hash.append("SHA256=");
					}

					hash.append(vecData[VALUE]);

					jsonItem["log"][vecData[KEY]] = hash;
				}
				else if (CUtil::PatternMatch("pid", vecData[KEY])
					|| CUtil::PatternMatch("ppid", vecData[KEY])
					|| CUtil::PatternMatch("size", vecData[KEY])
				)
				{
					jsonItem["log"][vecData[KEY]] = atoi(vecData[VALUE].c_str());
				}
				else
				{				
					jsonItem["log"][vecData[KEY]] = vecData[VALUE];
					//LOGD << "ITEM #2: " << jsonItem.toStyledString();
				}
			}
		}

		jsonLogs.append(jsonItem);
	}

	jsonRoot[JsonField_Data] = jsonLogs;
	jsonRoot[JsonField_UserIP] = CUtil::GetUserIP();
	jsonRoot[JsonField_UserMac] = CUtil::GetUserMAC();

	return jsonRoot;
}
