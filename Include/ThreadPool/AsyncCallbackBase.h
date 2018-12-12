#pragma once

#include "Fault.h"
#include "Callback.h"
#include "LockGuard.h"
#include "CallbackMsg.h"

class AsyncCallbackBase
{
public:
	AsyncCallbackBase();
	virtual ~AsyncCallbackBase();
	virtual VOID TargetInvoke(
		CallbackMsg** msg
	) const = 0;

protected:
	struct InvocationNode
	{
		InvocationNode() : Next(NULL), CallbackElement(NULL) { }
		InvocationNode* Next;
		const Callback* CallbackElement;
	};

	VOID Register(
		Callback::CallbackFunc func
		, CallbackThread* thread
		, VOID* userData = NULL
	);

	VOID Unregister(
		Callback::CallbackFunc func
		, CallbackThread* thread
		, VOID* userData = NULL
	);

	InvocationNode* GetInvocationHead() { return m_invocationHead; }

	LOCK* GetLock() { return &m_lock; }

private:
	typedef VOID(AsyncCallbackBase::*bool_type)() const;
	VOID this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty() ? 0 : &AsyncCallbackBase::this_type_does_not_support_comparisons;
	}
	BOOL operator !() const { return !Empty(); }
	BOOL Empty() const { return !m_invocationHead; }
	VOID Clear();

private:
	InvocationNode * m_invocationHead;

	LOCK m_lock;
};
