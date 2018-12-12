#pragma once

/****************************************
Http Attribute ( definition )
****************************************/
class CHttpAttribute
{
public:

	typedef enum
	{
		MethodInvalid = -1,
		Get = 0,
		Post,
		Delete,
	} EMethod;

	typedef enum
	{
		PolicyInvalid = -1,
		PolicyIOCRefresh,
		PolicyIOC,
		PolicyRESRefresh,
		PolicyRESProcess,
		PolicyRESFile,
		PolicyRESRegistry,
		PolicyRESNetwork,
		PolicyMAX,
	} EEDRPolicyType;

	typedef enum
	{
		ManagerInvalid = -1,
		ManagerIOC,
		ManagerRES,
		ManagerMAX,
	} EEDRManagerType;

};
