#pragma once

#include "ThreadPool/ThreadWin.h"

class CEDRWorkerThread : public ThreadWin
{
public:
	CEDRWorkerThread(
		const CHAR* threadName
		, BOOL syncStart = TRUE
	);

private:
	virtual unsigned long Process(
		VOID* parameter
	);
};

