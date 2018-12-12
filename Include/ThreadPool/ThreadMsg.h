#pragma once

#include "CommonDef.h"
#include "DataTypes.h"

class ThreadMsg
{
public:
	ThreadMsg(
		INT id
		, VOID* data
	) :
		m_id(id),
		m_data(data)
	{
	}

	INT GetId() const { return m_id; }

	VOID* GetData() const { return m_data; }

private:
	INT m_id;
	VOID* m_data;
};

