#include "HttpConnection4.h"
#include<HTTPConnection2.h>
#include<TCPInterface.h>
#include <RakPeerInterface.h>
#include <GetTime.h>
using namespace RakNet;
int HttpRequet(const char* url, RakString &request,std::string &content)
{
	int ret = 500;
	HTTPConnection2 *httpConnection2;
	httpConnection2 = HTTPConnection2::GetInstance();
	TCPInterface *tcp = TCPInterface::GetInstance();
	tcp->Start(0, 64);
	tcp->AttachPlugin(httpConnection2);
	SystemAddress address;
	UriParser parser;
	if (!parser.Parse(url))return ret;
	httpConnection2->TransmitRequest(request, parser.host, parser.port);
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

void DecodeUrl(const char* uri, std::string &result)
{
	
	RakString rak=uri;
	RakString decode;
	decode =rak.URLDecode();
	result = decode.C_String();
}

UriParser::UriParser():
	port(80),
	m_DecodeData(NULL)
{
}

UriParser::~UriParser()
{
	Clean();
}


bool UriParser::Parse(const char * url)
{
	memset(host, 0, 64);
	struct evhttp_uri *uri = evhttp_uri_parse(url);
	const char* h = evhttp_uri_get_host(uri);
	strcpy(host, h);
	int url_port = evhttp_uri_get_port(uri);
	port = 80;
	if (url_port > 0)port = url_port;
	if (NULL != uri)
	{
		evhttp_uri_free(uri);
		uri = NULL;
	}
	return strlen(host)>0;
}

char * UriParser::Decode(const char * url)
{
	Clean();
	m_DecodeData = evhttp_decode_uri(url);
	return m_DecodeData;
}

void UriParser::Clean()
{
	if (m_DecodeData)
	{
		delete[] m_DecodeData;
		m_DecodeData = NULL;
	}
}
