#ifndef __DEBUG_H__
#define __DEBUG_H__

#if 1
//
// Debug levels: higher values indicate higher urgency
//
#define DBG_LEVEL_VERBOSE 1
#define DBG_LEVEL_TERSE   2
#define DBG_LEVEL_WARNING 3
#define DBG_LEVEL_ERROR   4

static int DriverDbgLevel = 2;

#define DBGPRintEX(Level, Prefix, Type, Fmt)                        \
{                                                                   \
{                                                                   \
	DbgPrint("[NICS_PROCMON]%s%s %s (%d): ", Prefix, Type, __FILE__, __LINE__);   \
	DbgPrint Fmt;                                                   \
}                                                                   \
}

#define DBGPRint(Level, Prefix, Fmt)                                \
{                                                                   \
{                                                                   \
	DbgPrint("[NICS_PROCMON]%s", Prefix);                                   \
	DbgPrint Fmt;                                                   \
}                                                                   \
}


#define DbgMsg(_msg, ...) do{ DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_INFO_LEVEL, "[NICS_PROCMON]: "##_msg, ##__VA_ARGS__);} while(FALSE)

#if 0
///*
#define VERBOSE_FN(Fmt) DBGPRintEX(DBG_LEVEL_VERBOSE, __FUNCTION__"(): ", Fmt)
#define TERSE_FN(Fmt) DBGPRintEX(DBG_LEVEL_TERSE, __FUNCTION__"(): ", Fmt)
#define WARNING_FN(Fmt) DBGPRintEX(DBG_LEVEL_WARNING, __FUNCTION__"(): ", "WRN", Fmt)
#define ERR_FN(Fmt) DBGPRintEX(DBG_LEVEL_ERROR, __FUNCTION__"(): ", "ERR", Fmt)
//*/
#else
#define VERBOSE_FN(Fmt) DbgMsg Fmt
#define TERSE_FN(Fmt) 	DbgMsg Fmt
#define WARNING_FN(Fmt) DbgMsg Fmt
#define ERR_FN(Fmt) 	DbgMsg Fmt
#endif
#else

#define VERBOSE_FN(Fmt)
#define TERSE_FN(Fmt)
#define WARNING_FN(Fmt)
#define ERR_FN(Fmt)

#define DBGPRintEX(Level, Fmt)
#define DBGPRint(Fmt)
#endif // DBG

#endif //__DEBUG_H__
