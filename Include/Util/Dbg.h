#pragma once

//namespace dbg
typedef struct StackFrame
{
	DWORD64 address;
	std::string name;
	std::string module;
	unsigned int line;
	std::string file;
} StackFrame;

class Dbg
{
public:
	static	void trace(
		const char* msg
		, ...
	);

	static	std::string basename(
		const std::string& file
	);

	static	std::vector<StackFrame> stack_trace();

	static	void handle_assert(
		const char* func
		, const char* cond
	);

	static	void fail(
		const char* func
		, const char* msg
	);

};
