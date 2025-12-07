#pragma once

#include <string>

namespace adp {

enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class Log
{
public:
	static void Init();

	static void Shutdown();

	// If LogLevel is not specified, LogLevel::DEBUG is used
	static void Write(LogLevel level, const char* message);
	static void Write(const char* message);

	static void Writef(LogLevel level, const char* format, ...);
	static void Writef(const char* format, ...);

	static int NumMessages();

	static const std::string& Message(int index);

	static void SetLogLevel(LogLevel level);
};

}; // namespace adp.
