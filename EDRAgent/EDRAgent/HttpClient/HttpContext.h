#pragma once

/****************************************
Http Context ( CURL..ETC )
****************************************/
class CHttpContext
{
private:
	Context * m_pContext;
public:
	CHttpContext();

	~CHttpContext();

	Context* GetContext();
};