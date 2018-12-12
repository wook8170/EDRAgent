#include "StdAfx.h"
#include "EDRPolicyData.h"
#include "EDRPolicyHelperBase.h"
#include "EDRPolicyCommander.h"
#include "Util/Process.h"

class CRESPolicyHelperImpl : public CEDRPolicyHelperBase
{
private:
	string	m_strName;

	BOOL DoCommand(
		string type
		, vector<CommandData_Ptr> vecList
	);

	BOOL UpdatePolicy(
		string updateStatus
		, CEDRPolicyData *pPolicyData
		, EDRJson json
		, BOOL isUpdate = FALSE
	);

	static VOID CALLBACK ListCallbackForCreate(
		PCHECK_LIST_ENTRY CheckList[]
		, ULONG CheckListCount
		, VOID* UserData
	);

public:
	CRESPolicyHelperImpl(
		string name
	);

	~CRESPolicyHelperImpl();

	void Init(string name);

	EDRJson	GenerateAck(
		CHttpData* pHttpData
		, BOOL status
		, string keyName
	);

	BOOL ApplyPolicy(
		EDRJson jsonPolicy
		, BOOL isUpdated
	);
};
