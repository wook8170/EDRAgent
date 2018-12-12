#pragma once

#include "CommonDef.h"

/****************************************
Event class
****************************************/

class CSyncEvent
{
public:
	CSyncEvent(bool InitialState = false, bool AutoReset = false);

	~CSyncEvent();

	bool Valid();

	void Set();

	void Reset();

	bool IsSet();

	bool Wait(unsigned Timeout = INFINITE);

	HANDLE GetHandle();

private:
	HANDLE m_hEvent;
};