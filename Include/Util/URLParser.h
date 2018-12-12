#pragma once

#include "CommonDef.h"

#include <unordered_map>
#include <tuple>
#include <vector>
#include <string>

using namespace std;

typedef struct {
	string key;
	string val;
} query_kv_t;

typedef int(*__kv_callback)(VOID* list, string k, string v);

class CURLParse
{
private:
	CURLParse();
public:
	virtual ~CURLParse();
	static CURLParse* ParseUrl(
		string urlstr
	);

	static int ParsePath(
		vector<string> *pdirlist
		, string pathstr
	);

	static string UrlDecode(
		string str
	);

	static CHAR ToChar(
		const CHAR* hex
	);

	static string UrlEncode(
		string s
	);

	static VOID ToHex(
		CHAR *desthex
		, CHAR c
	);

	static size_t ParseKeyValueMap(
		unordered_map<string, string> *kvmap
		, string str
		, BOOL strict = TRUE
	);

	static size_t ParseKeyValueList(
		vector< query_kv_t > *kvmap
		, string rawstr
		, BOOL strict = TRUE
	);

	static size_t ParseKeyValue(
		string rawstr
		, __kv_callback kvcb
		, VOID* obj
		, BOOL strict
	);

private:
	VOID Parse();

	string mRawUrl;
public:
	string scheme;
	string hostName;
	string port;
	string path;
	string query;
	string fragment;
};

