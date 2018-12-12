#pragma once

#define _CRT_SECURE_NO_WARNINGS

//#include "stdafx.h"
#include "jsoncpp/json.h"
#include "curl/curl.h"
#include "curl/easy.h"
#include <iomanip>
#include <iostream>
#include "plog/Log.h"
#include "Util/Dbg.h"
#include <memory>  
#include <assert.h>

// COMMMON NAME SPACE
using namespace std;

//#define NOT_USE_PROCESS_BLOCK		(1)
//#define NOT_USE_PROCESS_MONITOR		(1)

// HTTP STATUS CODE
#define HTTP_ERROR_CLIENT	(0)
#define HTTP_ERROR_TIMEOUT	(408)
#define HTTP_OK				(200)
#define HTTP_ERROR_NOTFOUND	(404)
#define HTTP_ERROR_INTERNAL	(500)

// Memory
#define EDRAlloc(_size_) malloc(_size_)
#define EDRFree(_x_) {																				\
	if( _x_ ) free(_x_);																			\
	_x_ = NULL;																						\
}
#define EDRRealloc(_x_, _size_)realloc(_x_, _size_)
#define EDRNew new
#define EDRDelete(_x_) {																			\
	if( _x_ ) {																						\
		delete _x_;																					\
	}																								\
	_x_ = NULL;																						\
}
#define EDRDeleteArray(_x_) {																		\
	if( _x_ ) {																						\
		delete [] _x_;																				\
	}																								\
	_x_ = NULL;																						\
}
#define EDRZeroMemory( _dst_, _size_ ) ZeroMemory( _dst_, _size_ )
#define EDRCopyMemory( _dst_, _src_, _size_ ) memcpy( _dst_, _src_, _size_ )
#define EDRSetMemory(_dst_, _data_, _size_) memset(_dst_, _data_, _size_)
#define EDRException(_message_) throw std::runtime_error(_message_)

// LOG MACRO
#define FILLSETW(_count_)	std::setfill(' ') << std::setw(_count_)
#define FILLSETWL(_count_)	std::setfill(' ') << std::setw(_count_) << setiosflags( ios::left ) 
#define FILLSETWR(_count_)	std::setfill(' ') << std::setw(_count_) << setiosflags( ios::right ) 

#define RETURN_FAIL_IF(_condition_, _ret_) {														\
	if(_condition_) {																				\
		LOGW << "Fail !! \"" << #_condition_ << "\" is FALSE";										\
		return (## _ret_);																			\
	}																								\
}
#define RETURN_V_FAIL_IF(_condition_) {																\
	if(_condition_) {																				\
		LOGW << "Fail !! \"" << #_condition_ << "\" is FALSE";										\
		return;																						\
	}																								\
}
#define RETURN_FAIL_IF_MSG(_condition_, _ret_, _msg_) {												\
	if(_condition_) {																				\
		LOGW << _msg_;																				\
		LOGW << "Fail !! \"" << #_condition_ << "\" is FALSE";										\
		return (## _ret_);																			\
	}																								\
}
#define RETURN_V_FAIL_IF_MSG(_condition_, _ret_,  _msg_) {											\
	if(_condition_) {																				\
		LOGW << _msg_;																				\
		LOGW << "Fail !! \"" << #_condition_ << "\" is FALSE";										\
		return;																						\
	}																								\
}
#define PRINT_STACK_TRACE() {																		\
	std::vector<StackFrame> vecStackFrame = Dbg::stack_trace();										\
	std::vector<StackFrame>::iterator it;															\
	LOGF << "\n";																					\
	LOGF << std::string(205, '*');																	\
	for (it = vecStackFrame.begin(); it != vecStackFrame.end(); it++) {								\
		std::ostringstream ss;																		\
		std::ostringstream osname;																	\
		std::ostringstream osmodule;																\
		std::ostringstream osaddr;																	\
		std::ostringstream osfileline;																\
		osname << "**" << FILLSETWR(48) << (*it).name << "!";										\
		osmodule.width(16); osmodule << setiosflags( ios::left ) << (*it).module;					\
		osaddr.width(18); osaddr << setiosflags( ios::right ) << "Addr: 0x" << (*it).address << " ";\
		osfileline << (*it).file.c_str() << " (Line: " << (*it).line << ")";						\
		ss << osname.str() << osmodule.str() << osaddr.str() << osfileline.str();					\
		LOGF << ss.str();																			\
	}																								\
	LOGF << std::string(205, '*');																	\
	EDRException("예외상황 발생");																		\
}
#define CHECK(_condition_) {																		\
	if (_condition_ == FALSE) {																		\
		LOGF << "expression \"" << #_condition_ << "\" is FALSE";									\
		PRINT_STACK_TRACE();																		\
		assert(_condition_);																		\
	}																								\
}
#define CHECK_NOTNULL(_condition_){																	\
	if(_condition_==NULL) {																			\
		LOGF <<"expression \"" << #_condition_ << "\" is NULL";										\
		PRINT_STACK_TRACE();																		\
		assert(_condition_!=NULL);																	\
	}																								\
}
#define CHECK_GT(_a_, _b_) {																		\
	if(!(_a_>_b_)) {																				\
		LOGF << #_a_ << " is not greater than " << #_b_;											\
		PRINT_STACK_TRACE();																		\
		assert(_a_ > _b_);																			\
	}																								\
}																									\

// TEST
#define MAKE_CRASH() {																				\
	int *i = NULL;																					\
	*i = 0;																							\
}
#if 0//def _DEBUG
#define DPRINT_JSON(_json_) LOGD << #_json_ << " " << _json_.toStyledString()
#else
#define DPRINT_JSON(_json_)
#endif

// JSON FIELD NAME
#define IOCMetaName						(".\\IOCMeta.json")
#define RESMetaName						(".\\RESMeta.json")

#define Command_Block					("block")
#define Command_Delete					("delete")
#define Command_BlockCompleted			("deleteCompleted")
#define Command_Invalid					("Invalid Cmd")

#define KEY		(0)
#define VALUE	(1)

#define Type_Process					("process")
#define Type_Registry					("registry")
#define Type_File						("file")
#define Type_IP							("ip")
#define Type_URL						("url")
#define Type_Domain						("domain")

#define JsonFiled_ID					("id")
#define JsonFiled_ApplyList				("apply_list")
#define JsonField_ApplyTime				("apply_time")
#define JsonField_Message				("message")

#define JsonField_HelperName			("helper_name")
#define JsonField_NormalUrl				("normal_url")
#define JsonField_RefreshUrl			("refresh_url")
#define JsonField_LogUrl				("log_url")
#define JsonField_SubUrl				("sub_url")
#define JsonField_Root					("items")
#define JsonField_Name					("name")
#define JsonField_Type					("type")
#define JsonField_Key					("key")
#define JsonField_Code					("code")
#define JsonField_UsePolicy				("use_policy")
#define JsonField_Command				("command")
#define JsonField_LogBufferCount		("log_buffer_count")
#define JsonField_UnicodeKernel			("unicode_kernel")
#define JsonField_LogMethod				("log_method")
#define JsonField_LogTime				("log_time")
#define JsonField_Log					("log")

#define JsonField_Data					("data")
#define JsonField_PolicyData			("policy_data")
#define JsonField_Status				("status")

#define JsonField_Data					("data")
#define JsonField_UserID				("user_id")
#define JsonField_UserIP				("ip")
#define JsonField_UserMac				("mac")

#define StatusCode_Create				("C")
#define StatusCode_Update				("U")
#define StatusCode_Delete				("D")

#define StringSlash						"/"
#define StringBackslash					"\\"
#define StringDot						"."
#define StringNull						""

#define CharSemicolon					':'
#define CharDot							'.'
#define CharQuestion					'?'
#define CharSharp						'#'
#define	CharSlash						'/'
#define CharBackslash					'\\'
#define CharNull						'\0'

#define PathDelimeter					StringBackslash

// DATA TYPE
typedef CURL					Context;
typedef ULONG					HTTP_STATUS;
typedef Json::Value				EDRJson;
typedef Json::Reader			EDRJsonReader;
typedef Json::FastWriter		EDRJsonWriter;

// CALLBACK DATA STRUCT
typedef struct CallbackData_T
{
	INT		eRestType;
	CHAR	szURL[MAX_PATH];
} CallbackData;
typedef shared_ptr<CallbackData> CallbackData_Ptr;

// ALARM DATA STRUCT
typedef struct AlarmMessage_T
{
	CHAR	szTitle[512];
	CHAR	szInfo[1024];
	CHAR	szTip[512];

	AlarmMessage_T()
	{
		EDRZeroMemory(szTitle, 64);
		EDRZeroMemory(szInfo, 1024);
		EDRZeroMemory(szTip, 512);
	}
	AlarmMessage_T(CHAR* pszTitle, CHAR* pszInfo)
	{
		AlarmMessage_T();

		EDRCopyMemory(szTitle, pszTitle, min(512 - 1, strlen(pszTitle)));
		EDRCopyMemory(szInfo, pszInfo, min(1024 - 1, strlen(pszInfo)));
		EDRCopyMemory(szTip, pszTitle, min(512 - 1, strlen(pszTitle)));

		szTitle[min(512 - 1, strlen(pszTitle))] = NULL;
		szInfo[min(1024 - 1, strlen(pszInfo))] = NULL;
		szTip[min(512 - 1, strlen(pszTitle))] = NULL;
	}
} AlarmMessage;
typedef shared_ptr<AlarmMessage> AlarmMessage_Ptr;

#define MAX_DATA_LEN	(1024)
#define MAX_CMD_LEN		(64)

typedef struct CommandData_T
{
	INT			eType;
	CHAR		szCommand[MAX_CMD_LEN];
	CHAR		szData[MAX_DATA_LEN];
	CHAR		szData2[MAX_DATA_LEN];
	EDRJson		json;

	CommandData_T(
		INT type
		, string command
		, string data
		, EDRJson jsonData
		, string data2 = StringNull)
	{
		EDRZeroMemory(szCommand, MAX_CMD_LEN);
		EDRZeroMemory(szData, MAX_DATA_LEN);
		EDRZeroMemory(szData2, MAX_DATA_LEN);

		eType = type;
		EDRCopyMemory(
			szCommand
			, command.c_str()
			, command.length()
		);
		EDRCopyMemory(
			szData
			, data.c_str()
			, data.length()
		);
		if (!data2.empty())
		{
			EDRCopyMemory(
				szData2
				, data.c_str()
				, data.length()
			);
		}
		json = jsonData;
	}

	CommandData_T()
	{
		EDRZeroMemory(szCommand, MAX_CMD_LEN);
		EDRZeroMemory(szData, MAX_DATA_LEN);
		EDRZeroMemory(szData2, MAX_DATA_LEN);
	}

	~CommandData_T()
	{
	}
} CommandData;

typedef shared_ptr<CommandData>				CommandData_Ptr;
typedef shared_ptr<vector<CommandData_Ptr>>	CommandDataVector_Ptr;

#define USER_ALERT_LIEN_PER_CELL	(5)