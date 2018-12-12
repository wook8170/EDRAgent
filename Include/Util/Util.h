#pragma once

#include "CommonDef.h"

/****************************************
Utility class
****************************************/

class CUtil
{
protected:
	static string _format_arg_list(
		const CHAR *fmt
		, va_list args
	);

public:

	const static string GetUserIP();

	const static string GetUserMAC();

	const static string GetUserID();

	const static string GetAckMessage(
		BOOL status
	);

	const static INT GetAckCode(
		BOOL status
	);

	const static string CurrentDateTime();

	const static string GetSha256(
		string  filepath
	);

	const static string GetMD5(
		string  filepath
	);

	const static wstring ConvM2U(
		string multibyte
	);

	const static string ConvU2M(
		wstring unicode
	);

	static VOID Upper(
		string&  str
	);

	static VOID Lower(
		string&  str
	);

	static INT Compare(
		string  str
		, string str2
	);

	static string UTF8ToAnsi(
		string str
	);

	static CHAR* AnsiToUTF8(
		const CHAR * pszCode
	);

	/*
	static CHAR* ANSIToUTF8(
		const CHAR * pszCode
	);
	*/

	static vector<string> Split(
		const string& s
		, string& seperator
	);

	static string Replace(
		string& src
		, string& from
		, string& to
	);

	static BOOL PatternMatch(
		string str
		, string pattern
	);

	static string Format(
		const CHAR* fmt
		, ...);

	static string GetMachineUserName();

	static string GetCurrentSID();

	static BOOL Is64BitOS();

	static EDRJson ParseLogData(
		CHAR* pData
	);

};

