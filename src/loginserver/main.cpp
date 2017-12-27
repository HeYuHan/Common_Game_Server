#include "LoginServer.h"
#include <getopt.h>
#include <tools.h>
#include <log.h>
#include <HttpConnection4.h>
enum
{
	addr = 0x100,
	data_path
};
struct option long_options[] =
{
	{"addr",1,0,addr},
	{"data_path",1,0,data_path }
};

int main(int argc, char **argv)
{

	memset(&gServer.m_Config, 0, sizeof(Config));
	strcpy(gServer.m_Config.addr, "127.0.0.1:9300");
	strcpy(gLogger.fileName, "./../log/loginserver.log");
	strcpy(gLogger.logName, "login");
	gServer.m_Config.thread_count = 10;
	strcpy(gServer.m_Config.data_path, "./loginserver.json");
	while (1)
	{
		int option_index = 0;
		int option = getopt_long(argc, argv, "p:", long_options, &option_index);
		if (option <= 0)break;
		switch (option)
		{
		case addr:
			strcpy(gServer.m_Config.addr, optarg);
			break;
		case data_path:
			strcpy(gServer.m_Config.data_path, optarg);
			break;
		case '?':
			return 1;
			break;
		default:
			break;
		}
	}

	if (!RunAsDaemon())
	{
		console_error("%s","run as daemon error!");
		return -1;
	}
	return gServer.Run();
}