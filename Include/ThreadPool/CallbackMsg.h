#pragma once

#include "DataTypes.h"
#include "Fault.h"

class Callback;
class AsyncCallbackBase;

class CallbackMsg
{
public:
	CallbackMsg(
		AsyncCallbackBase* asyncCallback
		, const Callback* callback
		, const VOID* callbackData
	) :
		m_asyncCallback(asyncCallback),
		m_callback(callback),
		m_callbackData(callbackData)
	{
		CHECK_NOTNULL(m_asyncCallback);
		CHECK_NOTNULL(m_callback);
		CHECK_NOTNULL(m_callbackData);
	}

	const AsyncCallbackBase* GetAsyncCallback() const
	{
		return m_asyncCallback;
	}

	const Callback* GetCallback() const
	{
		return m_callback;
	}

	const VOID* GetCallbackData() const
	{
		return m_callbackData;
	}

private:
	AsyncCallbackBase * m_asyncCallback;
	const Callback* m_callback;
	const VOID* m_callbackData;
};
