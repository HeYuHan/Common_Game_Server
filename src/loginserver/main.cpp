#include "LoginServer.h"
#include <getopt.h>
enum
{
	ip = 0x100,
	port,
	config_path
};
struct option long_options[] =
{
	{"ip",1,0,ip},
	{ "port",1,0,port },
	{"data_path",1,0,config_path}
};

int main(int argc, char **argv)
{
	memset(&gServer.m_Config, 0, sizeof(Config));
	strcpy(gServer.m_Config.ip, "127.0.0.1");
	gServer.m_Config.port = 9300;
	strcpy(gServer.m_Config.data_path, "./loginserver.json");
	while (1)
	{
		int option_index = 0;
		int option = getopt_long(argc, argv, "p:", long_options, &option_index);
		if (option <= 0)break;
		switch (option)
		{
		case ip:
			strcpy(gServer.m_Config.ip, optarg);
			break;
		case port:
			gServer.m_Config.port= atoi(optarg);
			break;
		case config_path:
			strcpy(gServer.m_Config.data_path, optarg);
			break;
		case '?':
			return 1;
			break;
		default:
			break;
		}
	}
	return gServer.Run();
}