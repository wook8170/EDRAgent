#include "stdafx.h"

#include "AsyncCallbackBase.h"

AsyncCallbackBase::AsyncCallbackBase() :
	m_invocationHead(NULL)
{
	LockGuard::Create(&m_lock);
}

AsyncCallbackBase::~AsyncCallbackBase()
{
	LockGuard::Destroy(&m_lock);
}

VOID AsyncCallbackBase::Register(
	Callback::CallbackFunc func
	, CallbackThread* thread
	, VOID* userData
)
{
	LockGuard lockGuard(&m_lock);

	InvocationNode* node = EDRNew InvocationNode();
	node->CallbackElement = EDRNew Callback(func, thread, userData);

	if (m_invocationHead == NULL)
	{
		m_invocationHead = node;
	}
	else
	{
		InvocationNode* curr = m_invocationHead;
		while (curr->Next != NULL)
			curr = curr->Next;

		curr->Next = node;
	}
}

VOID AsyncCallbackBase::Unregister(
	Callback::CallbackFunc func
	, CallbackThread* thread
	, VOID* userData
)
{
	LockGuard lockGuard(&m_lock);

	InvocationNode* curr = m_invocationHead;
	InvocationNode* prev = NULL;

	const Callback callback(func, thread, userData);
	while (curr != NULL)
	{
		if (*curr->CallbackElement == callback)
		{
			if (curr == m_invocationHead)
				m_invocationHead = curr->Next;
			else
				prev->Next = curr->Next;

			EDRDelete(curr->CallbackElement);
			EDRDelete(curr);
			break;
		}
		prev = curr;
		curr = curr->Next;
	}
}

VOID AsyncCallbackBase::Clear()
{
	LockGuard lockGuard(&m_lock);
	while (m_invocationHead)
	{
		InvocationNode* curr = m_invocationHead;
		m_invocationHead = curr->Next;
		EDRDelete(curr->CallbackElement);
		EDRDelete(curr);
	}
}

