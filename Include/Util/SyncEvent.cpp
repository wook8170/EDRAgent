#include "StdAfx.h"
#include "SyncEvent.h"

CSyncEvent::CSyncEvent(bool InitialState, bool AutoReset)
{
	m_hEvent = CreateEvent(NULL, !AutoReset, InitialState, NULL);
	if (m_hEvent == NULL)
		EDRException("Unable to create event");
}

CSyncEvent::~CSyncEvent()
{
	CloseHandle(m_hEvent);
}

bool CSyncEvent::Valid()
{
	return m_hEvent != NULL;
}

void CSyncEvent::Set()
{
	SetEvent(m_hEvent);
}

void CSyncEvent::Reset()
{
	ResetEvent(m_hEvent);
}

bool CSyncEvent::IsSet()
{
	return (WaitForSingleObject(m_hEvent, 0) == WAIT_OBJECT_0);
}

bool CSyncEvent::Wait(unsigned Timeout)
{
	return (WaitForSingleObject(m_hEvent, Timeout) == WAIT_OBJECT_0 || WaitForSingleObject(m_hEvent, Timeout) == WAIT_TIMEOUT);
}

HANDLE CSyncEvent::GetHandle()
{
	return m_hEvent;
}