#include <Adp.h>

#include <stdarg.h>
#include <stdio.h>
#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "Log.h"

using namespace std;

namespace adp {

static vector<string>* messages = nullptr;
static LogLevel LOGLEVEL = LogLevel::ERROR;

void ToStdOutput(LogLevel level, const char* message)
{
	if (level < LOGLEVEL)
		return;

	cout << message << endl;
}

void Log::Init()
{
	messages = new vector<string>();
}

void Log::SetLogLevel(LogLevel level)
{
	LOGLEVEL = level;
}

void Log::Shutdown()
{
	delete messages;
	messages = nullptr;
}

void Log::Write(const char* message)
{
	Write(LogLevel::DEBUG, message);
}

void Log::Write(LogLevel level ,const char* message)
{
	messages->emplace_back(message);

	ToStdOutput(level, message);
}

void Log::Writef(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	Log::Writef(LogLevel::DEBUG, format, args);
	va_end (args);
}

void Log::Writef(LogLevel level, const char* format, ...)
{
	char buffer[10000];
	va_list args;
	va_start(args, format);

	#if defined(_MSC_VER)
	size_t len = vsprintf_s(buffer, 10000, format, args);
	#else
	size_t len = vsnprintf(buffer, 10000, format, args);
	#endif

	messages->emplace_back((const char*)buffer, len);
	
	va_end (args);


	if (level < LOGLEVEL)
		return;

	for(int i=0 ; i<len ; ++i)
	{
		cout << buffer[i];
	}

	cout << endl;
}

int Log::NumMessages()
{
	return (int)messages->size();
}

const string& Log::Message(int index)
{
	return messages->at(index);
}

}; // namespace adp.
