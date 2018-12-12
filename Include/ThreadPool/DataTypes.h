#pragma once

#if WIN32
//#include "windows.h"
#else
typedef signed CHAR INT8;
typedef UCHAR UINT8;
typedef signed short INT16;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef int INT32;
typedef CHAR CHAR;
typedef short SHORT;
typedef long LONG;
typedef int INT;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef UCHAR BYTE;
typedef unsigned short WORD;
typedef float FLOAT;
typedef double DOUBLE;
typedef int BOOL;

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((VOID *)0)
#endif
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif
#endif
