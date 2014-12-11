#include "LogFile.h"

Logger LogFile::logger ;
bool LogFile::open_log= false;

LogFile::LogFile(void)
{

}


LogFile::~LogFile(void)
{
}

bool LogFile::SetLogFilePath(const string& log_path)
{
	try
	{
		PropertyConfigurator::doConfigure(log_path);
		logger = Logger::getRoot();
	}
	catch(std::exception const &e) 
	{
		cout << "Log4cpp configure problem: " << e.what() << endl;
		return false;
	}
	open_log = true;
	return true;

}


#define DO_LOGGER(logLevel,pFormat, bufSize)\
	\
	if(LogFile::logger.isEnabledFor(logLevel))\
{ \
	va_list args; \
	va_start(args, pFormat); \
	char buf[bufSize] = {0}; \
	_vsnprintf(buf, sizeof(buf), pFormat, args); \
	va_end(args); \
	LogFile::logger.forcedLog(logLevel, buf); \
	 \
}
void LogFile::LogInfo(const char* pFormat,...)
{
	if (open_log)
	{
		DO_LOGGER(log4cplus::INFO_LOG_LEVEL,pFormat,1024);
	}

}

void LogFile::LogError(const char* pFormat,...)
{
	if (open_log)
	{	
		DO_LOGGER(log4cplus::ERROR_LOG_LEVEL,pFormat,1024);
	}
}
void LogFile::LogDebug(const char* pFormat,...)
{
	if (open_log)
	{
		DO_LOGGER(log4cplus::DEBUG_LOG_LEVEL,pFormat,512);
	}
}

