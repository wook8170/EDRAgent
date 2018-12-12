#pragma once

#include "CommonDef.h"
#include "HttpClient/HttpData.h"
#include "EDRPolicyData.h"

/****************************************
Http Policy Helper ( Generator Ack, Extract policy, Apply Policy )
****************************************/
class CEDRPolicyHelper abstract
{
	virtual EDRJson	GenerateAck(
		CHttpData* pHttpData
		, BOOL status
		, string keyName = JsonField_Data
	) = 0;

	virtual EDRJson	ExtractPolicy(
		CHttpData* pHttpData
		, string keyName = JsonField_Data
	) = 0;

	virtual BOOL ApplyPolicy(
		EDRJson jsonPolicy
		, BOOL isUpdated = TRUE
	) = 0;
};