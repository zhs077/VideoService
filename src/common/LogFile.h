
#ifndef LOGGER_H_
#define LOGGER_H_
#include "platform_config.h"

class LogFile
{
public:
	LogFile(void);
	~LogFile(void);
public:
	static bool SetLogFilePath(const string& file_path);
	static void LogInfo(const char* pFormat,...);
	static void LogError(const char* pFormat,...);
	static void LogDebug(const char* pFormat,...);
	
public:
	static Logger logger;
private:
	static bool open_log;//是否打开日志

	
};
#endif
#if defined(_MSC_VER) && _MSC_VER >1400
#define LOG_INFO(...) LogFile::LogInfo(__VA_ARGS__);
#define LOG_DEBUG(...) LogFile::LogDebug(__VA_ARGS__);
#define LOG_ERROR(...) LogFile::LogError(__VA_ARGS__);
#endif