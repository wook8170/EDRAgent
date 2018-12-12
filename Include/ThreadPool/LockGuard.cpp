#include "stdafx.h"

#include "LockGuard.h"
#include "Fault.h"

LockGuard::LockGuard(LOCK* lock)
{
	m_lock = lock;
	EnterCriticalSection(m_lock);
}

LockGuard::~LockGuard()
{
	LeaveCriticalSection(m_lock);
}

VOID LockGuard::Create(LOCK* lock)
{
	BOOL lockSuccess = InitializeCriticalSectionAndSpinCount(lock, 0x00000400);
	//BOOL lockSuccess = InitializeCriticalSectionAndSpinCount(lock, 0x0fffffff);
	CHECK(lockSuccess != 0);
}

VOID LockGuard::Destroy(LOCK* lock)
{
	DeleteCriticalSection(lock);
}