#include "LoginServer.h"
#include <getopt.h>
#include <tools.h>
#include <log.h>
#include <HttpConnection4.h>
enum
{
	addr = 0x100,
	data_path,
	log_path,
	flag_daemon
};
struct option long_options[] =
{
	{"addr",1,0,addr},
	{"data_path",1,0,data_path },
	{"daemon",0,0,flag_daemon },
	{"log_path",1,0,log_path }
};

int main(int argc, char **argv)
{

	memset(&gServer.m_Config, 0, sizeof(Config));
	strcpy(gServer.m_Config.addr, "127.0.0.1:9300");
	strcpy(gLogger.logName, "loginserver");
	gServer.m_Config.thread_count = 10;
	strcpy(gServer.m_Config.data_path, "./loginserver.json");
	bool as_daemon = false;
	while (1)
	{
		int option_index = 0;
		int option = getopt_long(argc, argv, "", long_options, &option_index);
		if (option <= 0)break;
		switch (option)
		{
		case addr:
			strcpy(gServer.m_Config.addr, optarg);
			break;
		case data_path:
			strcpy(gServer.m_Config.data_path, optarg);
			break;
		case flag_daemon:
			as_daemon = true;
			break;
		case log_path:
			gLogger.m_LogToFile = true;
			strcpy(gLogger.filePath, optarg);
			break;
		case '?':
			return 1;
			break;
		default:
			break;
		}
	}
	if (as_daemon &&!RunAsDaemon())
	{
		log_error("%s","run as daemon error!");
		return -1;
	}
	return gServer.Run();
}