#include "ChannelServer.h"
int main()
{
	gChannelServer.Init();
	gChannelServer.CreateUdpServer("192.168.2.103", 9500, 10);
	gChannelServer.Run();
	return 0;
}