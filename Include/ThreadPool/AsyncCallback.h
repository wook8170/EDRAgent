#pragma once

#include "AsyncCallbackBase.h"
#include "Callback.h"
#include "CallbackThread.h"

template<typename TData = NoData>

class AsyncCallback : public AsyncCallbackBase
{
public:
	typedef VOID(*CallbackFunc)(
		const TData& cbData
		, VOID* userData
		);

	VOID Register(
		CallbackFunc func
		, CallbackThread* thread
		, VOID* userData = NULL
	)
	{
		AsyncCallbackBase::Register(
			reinterpret_cast<Callback::CallbackFunc>(func)
			, thread
			, userData
		);
	}
	VOID Unregister(
		CallbackFunc func
		, CallbackThread* thread
		, VOID* userData = NULL
	)
	{
		AsyncCallbackBase::Unregister(
			reinterpret_cast<Callback::CallbackFunc>(func)
			, thread
			, userData
		);
	}
	VOID operator()(
		const TData& data
		)
	{
		Invoke(data);
	}

	VOID Invoke(
		const TData& data
	)
	{
		LockGuard lockGuard(GetLock());
		InvocationNode* node = GetInvocationHead();
		while (node != NULL)
		{
			const Callback* callback = EDRNew Callback(*node->CallbackElement);
			const TData* callbackData = EDRNew TData(data);
			CallbackMsg* msg = EDRNew CallbackMsg(this, callback, callbackData);

			callback->GetCallbackThread()->DispatchCallback(msg);

			node = node->Next;
		}
	}

	virtual VOID TargetInvoke(
		CallbackMsg** msg
	) const
	{
		const Callback* callback = (*msg)->GetCallback();

		const TData* callbackData = static_cast<const TData*>((*msg)->GetCallbackData());

		CallbackFunc func = reinterpret_cast<CallbackFunc>(callback->GetCallbackFunction());

		(*func)(*callbackData, callback->GetUserData());

		//delete callbackData;
		//delete callback;
		//delete *msg;
		EDRDelete(callbackData);
		EDRDelete(callback);
		EDRDelete(*msg);
		*msg = NULL;
	}
};
