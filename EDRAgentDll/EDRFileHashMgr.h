
// EDRFileHashMgr.h : 헤더 파일
//

#pragma once

#include "winioctl.h"
#include "winnt.h"

#define FILE_DEVICE_FILEMON	0x00008300

#define IOCTL_EDR_FILE_GET_HASH		(ULONG) CTL_CODE( FILE_DEVICE_FILEMON, 0xC9, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_EDR_FILE_RESULT_HASH	(ULONG) CTL_CODE( FILE_DEVICE_FILEMON, 0xCA, METHOD_BUFFERED, FILE_ANY_ACCESS )

//#pragma pack(push, 1 )
typedef struct _CHECKEDRFILE_DATA
{
	LIST_ENTRY		IoListEntry;
    BYTE			FileName[2048];
    BYTE			Hash[130];
    void*			pWaitSema;
    struct _CHECKEDRFILE_DATA *pMyAddr;
}CHECKEDRFILE_DATA, *PCHECKEDRFILE_DATA;

typedef struct _CHECKEDRFILE_DATA64
{
	LIST_ENTRY64	IoListEntry;
    BYTE			FileName[2048];
    BYTE			Hash[130];
    LONGLONG		pWaitSema;
    LONGLONG		pMyAddr;
}CHECKEDRFILE_DATA64, *PCHECKEDRFILE_DATA64;
//#pragma pack(pop)

BOOL StartEDRFileHashMgr( );
BOOL StopEDRFileHashMgr( );
