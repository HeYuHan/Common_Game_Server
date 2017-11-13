#include "ChannelServer.h"
#include <getopt.h>
#include <log.h>
#include <json/json.h>
using namespace Json;
enum
{
	channel_ip = 0x100,
	channel_port,
	password,
	max_client,
	max_room,
	log_path,
	log_name,
	weapon_path,
};
struct option long_options[]=
{
	{"log_name",1,0,log_path },
	{"log_path",1,0,log_name },
	{"ip",1,0,channel_ip },
	{"port",1,0,channel_port },
	{"password",1,0,password },
	{"max_client",1,0,max_client },
	{"max_room",1,0,max_room },
	{"weapon_path",1,0,weapon_path },
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
		case log_name:
			strcpy(gLogger.name, optarg);
		case log_path:
			strcpy(gLogger.fileName, optarg);
			break;
		case weapon_path:
			strcpy(gChannelServer.m_Config.weapon_config_path, optarg);
			break;
		case channel_ip:
			strcpy(gChannelServer.m_Config.ip, optarg);
			break;
		case channel_port:
			gChannelServer.m_Config.port = atoi(optarg);
			break;
		case max_client:
			gChannelServer.m_Config.max_client= atoi(optarg);
			break;
		case password:
			strcpy(gChannelServer.m_Config.pwd, optarg);
			break;
		case max_room:
			gChannelServer.m_Config.max_room = atoi(optarg);
			break;
		case '?':
			return 1;
			break;
		default:
			break;
		}
	}
	gChannelServer.Run();
	return 0;
}