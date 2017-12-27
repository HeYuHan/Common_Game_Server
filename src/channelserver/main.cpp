#include "ChannelServer.h"
#include <getopt.h>
#include <log.h>
#include <json/json.h>
#include <HttpConnection3.h>
#include <Timer.h>
#include <tools.h>
using namespace Json;
enum
{
	addr = 0x100,
	password,
	data_path,
};
struct option long_options[]=
{
	{"addr",1,0,addr },
	{"password",1,0,password },
	{"data_path",1,0,data_path },
};
int main(int argc,char **argv)
{
	
	while (1)
	{
		int option_index = 0;
		int option = getopt_long(argc, argv, "i:p:", long_options, &option_index);
		if (option <= 0)break;
		switch (option)
		{
		case data_path:
			strcpy(gChannelServer.m_Config.data_config_path, optarg);
			break;
		case addr:
			strcpy(gChannelServer.m_Config.addr, optarg);
			break;
		case password:
			strcpy(gChannelServer.m_Config.pwd, optarg);
			break;
		case '?':
			return 1;
			break;
		default:
			break;
		}
	}
	if (!RunAsDaemon())return -1;

	return 	gChannelServer.Run();
}