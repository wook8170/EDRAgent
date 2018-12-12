#include "stdafx.h"
#include "CommonDef.h"
#include "Fault.h"
#include "DataTypes.h"

VOID FaultHandler(const CHAR* file, unsigned short line)
{
	DebugBreak();
	CHECK(0);
}