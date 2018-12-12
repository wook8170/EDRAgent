#pragma once

#include "DataTypes.h"

class CallbackThread;

class Callback
{
public:
	typedef VOID(*CallbackFunc)(
		const VOID* cbData
		, VOID* userData
		);

	Callback(
		CallbackFunc func
		, CallbackThread* thread
		, VOID* userData = NULL
	) :
		m_thread(thread),
		m_func(func),
		m_userData(userData)
	{
	}

	CallbackThread* GetCallbackThread() const
	{
		return m_thread;
	}

	VOID* GetUserData() const
	{
		return m_userData;
	}

	CallbackFunc GetCallbackFunction() const
	{
		return m_func;
	}

	BOOL operator==(
		const Callback& rhs
		) const
	{
		return m_thread == rhs.m_thread &&
			m_func == rhs.m_func &&
			m_userData == rhs.m_userData;
	}

	BOOL operator!=(
		const Callback& rhs
		) const
	{
		return !(*this == rhs);
	}

private:
	CallbackThread * m_thread;

	VOID* m_userData;

	CallbackFunc m_func;
};

