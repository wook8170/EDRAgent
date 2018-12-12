#pragma once

#include "DataTypes.h"
#include "CallbackMsg.h"

class CallbackThread
{
public:
	virtual ~CallbackThread() {}
	virtual VOID DispatchCallback(
		CallbackMsg* msg
	) = 0;
};


