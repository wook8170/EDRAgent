#pragma once
#include <plog/Record.h>
#include <plog/Util.h>
#include <iomanip>
#include <vector>



namespace plog
{
    class TxtFormatter
    {
    public:
        static util::nstring header()
        {
            return util::nstring();
        }

		static std::vector<util::nstring> _split(const util::nstring& s, util::nstring& seperator)
		{
			std::vector<util::nstring> output;

			util::nstring::size_type prev_pos = 0, pos = 0;

			while ((pos = s.find(seperator, pos)) != util::nstring::npos)
			{
				util::nstring substring(s.substr(prev_pos, pos - prev_pos));

				output.push_back(substring);

				prev_pos = ++pos;
			}

			output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word

			return output;
		}

		static util::nstring format(const Record& record)
		{
			tm t;
			util::localtime_s(&t, &record.getTime().time);
#define FUNC_LEN 48
			char funcName[FUNC_LEN + 1];
			memset(funcName, '.', FUNC_LEN + 1);
			std::string func = record.getFunc();
			memcpy(funcName + (FUNC_LEN - min(FUNC_LEN, func.length())), func.c_str(), min(FUNC_LEN, func.length()));

			if (func.length() >= FUNC_LEN)
			{
				funcName[FUNC_LEN - 2] = '.';
				funcName[FUNC_LEN - 1] = '.';
			}

			funcName[FUNC_LEN] = NULL;
			func = funcName;

			util::nostringstream ss;
#if 1            
			util::nstring message = record.getMessage();

			ss << t.tm_year + 1900 << "-" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mon + 1 << PLOG_NSTR("-") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mday << PLOG_NSTR(" ");
			ss << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_hour << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_min << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_sec << PLOG_NSTR(".") << std::setfill(PLOG_NSTR('0')) << std::setw(3) << record.getTime().millitm << PLOG_NSTR(" ");
			ss << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::left << severityToString(record.getSeverity()) << PLOG_NSTR(" ");
			ss << PLOG_NSTR("[TID:") << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::left << record.getTid() << PLOG_NSTR("] ");
			ss << PLOG_NSTR("[") << func.c_str() << PLOG_NSTR("()@") << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::right << record.getLine() << PLOG_NSTR("] ");
			//ss << *it << PLOG_NSTR("\n");
			ss << message << PLOG_NSTR("\n");
#else
			ss << t.tm_year + 1900 << "-" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mon + 1 << PLOG_NSTR("-") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mday << PLOG_NSTR(" ");
			ss << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_hour << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_min << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_sec << PLOG_NSTR(".") << std::setfill(PLOG_NSTR('0')) << std::setw(3) << record.getTime().millitm << PLOG_NSTR(" ");
			ss << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::left << severityToString(record.getSeverity()) << PLOG_NSTR(" ");
			ss << PLOG_NSTR("[TID:") << record.getTid() << PLOG_NSTR("] ");			
			ss << std::setfill(PLOG_NSTR(' ')) << std::setw(200) << record.getMessage();
			ss << PLOG_NSTR("[FNC:") << record.getFunc() << PLOG_NSTR("() @LINE: ") << record.getLine() << PLOG_NSTR("] ");

			if (record.getSeverity() < fatal)
			{
				ss << "\n********************************************************************************************************\n";
				exit(-1);
			}
#endif

            return ss.str();
        }
    };
}
