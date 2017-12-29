#include "tools.h"
#include <time.h>
#include <stdlib.h>
#include "log.h"
#include <string.h>
#include "common.h"
#ifdef LINUX
#include <arpa/inet.h>
#include <sys/signal.h>
#endif // LINUX

static bool m_RandomFirst = true;
int RandomRange(int a, int b)
{
	if (m_RandomFirst) {
		m_RandomFirst = false;
		srand((unsigned int)time(NULL));
	}
	return (int)((double)rand() / ((RAND_MAX + 1.0) / (b - a + 1.0)) + a);
}
void ParseJsonValue(Json::Value json, const char* key, int &value)
{
	if (!json[key].isNull())value = json[key].asInt();
	else
	{

		log_warn("%s config miss use default:%d", key, value);
	}
}
void ParseJsonValue(Json::Value json, const char* key, float &value)
{
	if (!json[key].isNull())value = json[key].asDouble();
	else
	{

		log_warn("%s config miss use default:%f", key, value);
	}
}
void ParseJsonValue(Json::Value json, const char* key, bool &value)
{
	if (!json[key].isNull())value = json[key].asBool();
	else
	{

		log_warn("%s config miss use default:%s", key, value ? "true" : "false");
	}
}
void ParseJsonValue(Json::Value json, const char* key, char* str, int len)
{
	if (!json[key].isNull())
	{
		memset(str, 0, len);
		int str_len = json[key].asString().size();
		if (str_len < len)
		{
			strcpy(str, json[key].asString().c_str());
		}
	}
	else
	{

		log_warn("%s config miss use default:%s", key, str);
	}
}
bool ParseSockAddr(sockaddr_in & addr, const char * str, bool by_name)
{
	if (str)
	{
		char address[256];
		uint port;

		const char * port_start = strchr(str, ':');
		if (port_start)
		{
			memcpy(address, str, port_start - str);
			address[port_start - str] = 0;
			port = atoi(port_start + 1);

			memset(&addr, 0,sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);

			struct hostent* host = NULL;
			if (by_name && (host = gethostbyname(address)))
			{
				addr.sin_addr.s_addr =   inet_addr(inet_ntoa(*((struct in_addr*)host->h_addr)));
			}
			else
			{
				if (address[0])
					addr.sin_addr.s_addr = inet_addr(address);
			}

			return true;
		}
	}

	return false;
}
bool RunAsDaemon()
{
#ifdef LINUX

	// ignore signals
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	return daemon(1, 0) == 0;
#endif // LINUX
	return true;
}