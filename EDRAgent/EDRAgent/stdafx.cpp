
// stdafx.cpp : source file that includes just the standard includes
// EDRAgent.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// LINK STATIC LIBARAY (.lib)
#pragma comment( lib, "lib_json.lib" )
#pragma comment( lib, "libcurl.lib" )
#pragma comment( lib, "libeay32.lib" )
#pragma comment( lib, "ssleay32.lib" )

// LINK DYNAMIC LIBARAY (.dll)
#ifdef _WIN64
#pragma comment( lib, "EDRAgentDll_64.lib" )
#pragma comment( lib, "EDRProcMonDll_64.lib" )
#else
#pragma comment( lib, "EDRAgentDll_32.lib" )	
#pragma comment( lib, "EDRProcMonDll_32.lib" )
#endif
