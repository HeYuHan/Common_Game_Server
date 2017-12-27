#include "HttpConnection4.h"
#include<HTTPConnection2.h>
#include<TCPInterface.h>
#include <RakPeerInterface.h>
#include <event2/http.h>
#include <GetTime.h>
#include "log.h"
using namespace RakNet;
int HttpRequet(const char* url, RakString &request,std::string &content)
{
	int ret = 500;
	HTTPConnection2 *httpConnection2;
	httpConnection2 = HTTPConnection2::GetInstance();
	TCPInterface *tcp = TCPInterface::GetInstance();
	tcp->Start(0, 64);
	tcp->AttachPlugin(httpConnection2);
	
	int port = 80;
	struct evhttp_uri *uri = evhttp_uri_parse(url);
	const char* host = evhttp_uri_get_host(uri);
	int url_port = evhttp_uri_get_port(uri);
	if (url_port > 0)port = url_port;
	httpConnection2->TransmitRequest(request, host, 9300);
	evhttp_uri_free(uri);
	RakNet::Time timeout = RakNet::GetTime() + 2000;
	while (RakNet::GetTime() < timeout)
	{
		SystemAddress sa;
		Packet *packet;
		// This is kind of crappy, but for TCP plugins, always do HasCompletedConnectionAttempt, then Receive(), then HasFailedConnectionAttempt(),HasLostConnection()
		sa = tcp->HasCompletedConnectionAttempt();
		for (packet = tcp->Receive(); packet; tcp->DeallocatePacket(packet), packet = tcp->Receive())
			;
		sa = tcp->HasFailedConnectionAttempt();
		if (sa != UNASSIGNED_SYSTEM_ADDRESS)
		{
			break;
		}
		sa = tcp->HasLostConnection();
		if (sa != UNASSIGNED_SYSTEM_ADDRESS)
		{
			break;
		}

		RakString stringTransmitted;
		RakString hostTransmitted;
		RakString responseReceived;
		SystemAddress hostReceived;
		int contentOffset;
		if (httpConnection2->GetResponse(stringTransmitted, hostTransmitted, responseReceived, hostReceived, contentOffset))
		{

			if (responseReceived.IsEmpty() == false)
			{
				if (contentOffset == -1)
				{
					ret = 404;
				}
				else
				{
					content = responseReceived.SubStr(contentOffset, responseReceived.GetLength() - contentOffset).C_String();
					ret = 200;
				}

			}
			break;

		}

	}
	TCPInterface::DestroyInstance(tcp);
	HTTPConnection2::DestroyInstance(httpConnection2);
	return ret;
}
int HttpGet(const char * url, std::string & content)
{
	RakString rsRequest = RakString::FormatForGET(url);
	return HttpRequet(url, rsRequest, content);
}
int HttpPost(const char * url, const char* data,std::string & content)
{
	RakString rsRequest = RakString::FormatForPOST(url, "text/plain", data);
	return HttpRequet(url, rsRequest, content);
	
}