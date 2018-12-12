#pragma once

#include "DataTypes.h"

#define LOCK CRITICAL_SECTION

class LockGuard
{
public:
	LockGuard(
		LOCK* lock
	);

	~LockGuard();

	static VOID Create(
		LOCK* lock
	);

	static VOID Destroy(
		LOCK* lock
	);

private:
	LOCK * m_lock;
};

