#pragma once

#include "CommonDef.h"
#include "EDRAgentDll.h"
#include "Util\SyncEvent.h"
#include "EDRWorkerThreadManager.h"
#include "Util\SyncEvent.h"
#include "EDRWorkerThreadManager.h"
#include "EDRKernelDelegator.h"

class CEDRPolicyCommander 
{
public:
	typedef enum
	{
		ECommandInvalid = -1,
		ECommandBlock = 0,
		ECommandBlockCompleted,
		ECommandDelete,
		ECommandMax
	} ECommand;

private:

	ECommand CEDRPolicyCommander::GetCommandCode(
		string cmd
	);

	string GetCommandString(
		CEDRPolicyCommander::ECommand eCmd
	);

protected:
	BOOL DoCommandFile(
		vector<CommandData_Ptr> commandData
	);

	BOOL DoCommandRegistry(
		vector<CommandData_Ptr> commandData
	);

	BOOL DoCommandProcess(
		vector<CommandData_Ptr> commandData
	);

	BOOL DoCommandIp(
		vector<CommandData_Ptr> commandData
	);

	BOOL DoCommandURL(
		vector<CommandData_Ptr> commandData
	);

public:
	CEDRPolicyCommander();

	~CEDRPolicyCommander();

	BOOL DoCommand(
		string type
		, vector<CommandData_Ptr> commandData
	);
};