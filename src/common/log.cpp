#include "log.h"
Logger gLogger;
Logger::Logger():
	logger(NULL)
{
	memset(name, 0, sizeof(name));
	memset(fileName, 0, sizeof(fileName));
}
Logger::~Logger()
{
	logger->shutdown();
	logger = NULL;
}

void Logger::Init()
{
	if (strlen(name) == 0)
	{
		strcpy(name, "Log");
		
	}
	if (strlen(fileName) == 0)
	{
		strcpy(fileName, "../log/log_file.log");
	}
	PatternLayout *layout = new PatternLayout();
	layout->setConversionPattern("%d: %p %c %x: %m%n");
	logger = &Category::getRoot().getInstance(name);
#ifndef DEBUG
	RollingFileAppender *fileAppender = new RollingFileAppender(name, fileName);
	fileAppender->setLayout(layout);
	logger->addAppender(fileAppender);
#endif
	OstreamAppender *consoleAppender = new OstreamAppender(name, &std::cout);
	consoleAppender->setLayout(layout);
	logger->addAppender(consoleAppender);

	logger->setPriority(Priority::DEBUG);
}

Category & Logger::GetLogger()
{
	if (NULL == logger)Init();
	return *logger;
}
