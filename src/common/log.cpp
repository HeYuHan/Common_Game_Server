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
	RollingFileAppender *fileAppender = new RollingFileAppender(name, fileName);
	OstreamAppender *consoleAppender = new OstreamAppender(name, &std::cout);
	PatternLayout *layout = new PatternLayout();
	layout->setConversionPattern("%d: %p %c %x: %m%n");
	fileAppender->setLayout(layout);
	consoleAppender->setLayout(layout);
	logger = &Category::getRoot().getInstance(name);
	logger->addAppender(fileAppender);
	logger->addAppender(consoleAppender);
	logger->setPriority(Priority::DEBUG);
}

Category & Logger::GetLogger()
{
	if (NULL == logger)Init();
	return *logger;
}
