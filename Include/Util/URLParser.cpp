
#include "stdafx.h"
#include "util/URLParser.h"

#define CHECK_LEN_END(POS, LEN) if(POS>=LEN) {_url_errorno=100;goto __PARSE_END;}
#define WALK_SP(POS, LEN, BUF) for(;POS<LEN && BUF[POS]==' ';POS++)
#define WALK_UNTIL(POS, LEN, BUF, DELC) for(;POS<LEN && BUF[POS]!=DELC;POS++)
#define WALK_UNTIL2(POS, LEN, BUF, DELI1, DELI2) for(;POS<LEN && BUF[POS]!=DELI1 && BUF[POS]!=DELI2 ;POS++)
#define WALK_UNTIL3(POS, LEN, BUF, DELI1, DELI2, DELI3) for(;POS<LEN && BUF[POS]!=DELI1 && BUF[POS]!=DELI2 && BUF[POS]!=DELI3;POS++)
#define CHECK_REMAIN_END(POS, LEN, REQ_LEN) if(LEN-POS < REQ_LEN) {_url_errorno=100; goto __PARSE_END; }
#define WALK_CHAR(POS, BUF, DELI) if(BUF[POS++] != DELI) goto __PARSE_END

int __kv_callback_map(
	VOID* list
	, string k
	, string v
);
int __kv_callback_vec(
	VOID* list
	, string k
	, string v
);

CURLParse::CURLParse()
{
}

CURLParse::~CURLParse()
{
}



string CURLParse::UrlDecode(
	string str
)
{
	int _url_errorno = 0;
	size_t pos = 0, per = 0;
	size_t len = str.size();
	const CHAR* buf = str.c_str();
	string decstr;
	_url_errorno = 0;
	for (per = pos = 0;;)
	{
		WALK_UNTIL2(pos, len, buf, '%', '+');
		decstr.append(buf, per, pos - per);
		if (pos >= len)
			goto __PARSE_END;
		if (buf[pos] == '%')
		{
			CHECK_REMAIN_END(pos, len, 3);
			try
			{
				CHAR c = CURLParse::ToChar(buf + pos + 1);
				decstr.push_back(c);
				pos += 3;
				per = pos;
			}
			catch (int err)
			{
				_url_errorno = err;
				goto __PARSE_END;
			}
			if (pos >= len)
				goto __PARSE_END;
		}
		else if (buf[pos] == '+')
		{
			decstr.push_back(' ');
			pos++;
			per = pos;
		}
	}
__PARSE_END: if (_url_errorno != 0)
	return StringNull;
			 return decstr;

}

string CURLParse::UrlEncode(
	string s
)
{
	const CHAR *ptr = s.c_str();
	string enc;
	CHAR c;
	CHAR phex[3] = { '%' };
	for (size_t i = 0; i < s.size(); i++)
	{
		c = ptr[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
			|| (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '*'
			|| c == '.')
		{
			enc.push_back(c);
		}
		else if (c == ' ')
		{
			enc.push_back('+');
		}
		else
		{
			ToHex(phex + 1, c);
			enc.append(phex, 0, 3);
		}
	}
	return enc;
}

VOID CURLParse::ToHex(
	CHAR* desthex
	, CHAR c
)
{
	static CHAR hextable[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8',
			'9', 'A', 'B', 'C', 'D', 'E', 'F' };
	desthex[0] = hextable[c >> 4];
	desthex[1] = hextable[c & 0x0f];
}

int CURLParse::ParsePath(
	vector<string>* folders
	, string pathstr
)
{
	int _url_errorno = 0;
	size_t path_pos = 0;
	size_t pos = 0;
	size_t len = pathstr.size();
	const CHAR* str = pathstr.c_str();
	string name;
	for (pos = 0;;)
	{
		WALK_CHAR(pos, str, '/');
		path_pos = pos;
		CHECK_LEN_END(pos, len);
		WALK_UNTIL(pos, len, str, '/');
		name = pathstr.substr(path_pos, pos - path_pos);
		folders->push_back(name);
	}
__PARSE_END: return (int)folders->size();
}

VOID CURLParse::Parse()
{
	int _url_errorno = 0;
	const CHAR *str = mRawUrl.c_str();

	size_t pos, len;
	size_t scheme_pos, host_pos, port_pos, path_pos, param_pos, tag_pos;
	pos = 0;
	len = mRawUrl.size();
	WALK_SP(pos, len, str); // remove preceding spaces.
	if (str[pos] == '/')
	{
		goto __PARSE_HOST;
	}

	// start protocol scheme
	scheme_pos = pos;
	WALK_UNTIL(pos, len, str, ':');
	CHECK_LEN_END(pos, len);
	scheme = mRawUrl.substr(scheme_pos, pos - scheme_pos);
	CHECK_REMAIN_END(pos, len, 3);
	WALK_CHAR(pos, str, ':');
	WALK_CHAR(pos, str, '/');

	// start host address
__PARSE_HOST:
	WALK_CHAR(pos, str, '/');
	host_pos = pos;
	WALK_UNTIL3(pos, len, str, ':', '/', '?');
	if (pos < len)
	{
		hostName = mRawUrl.substr(host_pos, pos - host_pos);
		if (str[pos] == ':')
			goto __PARSE_PORT;
		if (str[pos] == '/')
			goto __PARSE_PATH;
		if (str[pos] == '?')
			goto __PARSE_PARAM;
	}
	else
	{
		hostName = mRawUrl.substr(host_pos, pos - host_pos);
	}

__PARSE_PORT:
	WALK_CHAR(pos, str, ':');
	port_pos = pos;
	WALK_UNTIL2(pos, len, str, '/', '?');
	port = mRawUrl.substr(port_pos, pos - port_pos);
	CHECK_LEN_END(pos, len);
	if (str[pos] == '?')
		goto __PARSE_PARAM;
__PARSE_PATH: path_pos = pos;
	WALK_UNTIL(pos, len, str, '?');
	path = mRawUrl.substr(path_pos, pos - path_pos);
	CHECK_LEN_END(pos, len);
__PARSE_PARAM:
	WALK_CHAR(pos, str, '?');
	param_pos = pos;
	WALK_UNTIL(pos, len, str, '#');
	query = mRawUrl.substr(param_pos, pos - param_pos);
	CHECK_LEN_END(pos, len);

	// start parsing fragment
	WALK_CHAR(pos, str, '#');
	tag_pos = pos;
	fragment = mRawUrl.substr(tag_pos, len - tag_pos);
__PARSE_END: return;
}

CURLParse* CURLParse::ParseUrl(
	string urlstr
)
{
	CURLParse *url = EDRNew CURLParse;
	url->mRawUrl = urlstr;
	url->Parse();
	return url;
}

CHAR CURLParse::ToChar(
	const CHAR* hex
)
{
	UCHAR nible[2];
	UCHAR c, base;
	for (int i = 0; i < 2; i++) {
		c = hex[i];
		if (c >= '0' && c <= '9') {
			base = '0';
		}
		else if (c >= 'A' && c <= 'F') {
			base = 'A' - 10;
		}
		else if (c >= 'a' && c <= 'f') {
			base = 'a' - 10;
		}
		else {
			throw 200;
		}
		nible[i] = c - base;
	}
	return ((nible[0] << 4) | nible[1]);
}

size_t CURLParse::ParseKeyValueMap(
	unordered_map<string, string> *kvmap
	, string rawstr
	, BOOL strict
)
{
	return ParseKeyValue(
		rawstr
		, __kv_callback_map
		, kvmap
		, strict
	);
}

size_t CURLParse::ParseKeyValueList(
	vector< query_kv_t > *kvvec
	, string rawstr
	, BOOL strict
)
{
	return ParseKeyValue(
		rawstr
		, __kv_callback_vec
		, kvvec
		, strict
	);
}

size_t CURLParse::ParseKeyValue(
	string rawstr
	, __kv_callback kvcb
	, VOID* obj
	, BOOL strict
)
{

	int _url_errorno = 0;
	const CHAR *str = rawstr.c_str();
	size_t pos, len, item_len;
	pos = 0;
	len = rawstr.size();

	string key, val;
	size_t key_pos;
	WALK_SP(pos, len, str);
	CHECK_LEN_END(pos, len);
	key_pos = pos;
	item_len = 0;
	for (;;)
	{
		WALK_UNTIL2(pos, len, str, '=', '&');
		if (pos >= len || str[pos] == '&')
		{
			// Be careful for boundary check error to be caused. !!!
			// *** Do not access str[] any more in this block. !!!

			val = rawstr.substr(key_pos, pos - key_pos);

			if (strict == TRUE)
			{
				if (key.empty() == FALSE && val.empty() == FALSE)
				{
					kvcb(obj, key, val);
					item_len++;
				}
			}
			else if (!(key.empty() == TRUE && val.empty() == TRUE))
			{
				kvcb(obj, key, val);
				item_len++;
			}

			key.clear();val.clear();
			if (pos >= len) goto __PARSE_END;
			pos++;
			key_pos = pos;
		}
		else if (str[pos] == '=')
		{
			key = rawstr.substr(key_pos, pos - key_pos);
			pos++;
			key_pos = pos;
		}
	}
__PARSE_END:
	if (_url_errorno != 0)
		return -1;
	return item_len;
}


int __kv_callback_map(
	VOID* list
	, string k
	, string v
)
{
	auto *map = (unordered_map<string, string>*)list;
	(*map)[k] = v;
	return (int)map->size();
}

int __kv_callback_vec(
	VOID* list
	, string k
	, string v
)
{
	auto *vec = (vector<query_kv_t>*)list;
	query_kv_t t = { k, v };
	vec->push_back(t);
	return (int)vec->size();
}
