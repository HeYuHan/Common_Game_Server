#include "Client.h"
ChannelClient::ChannelClient():
	uid(0),
	m_NetState(NET_NONE)
{
	connection = this;
	stream = this;
}

ChannelClient::~ChannelClient()
{
}

void ChannelClient::OnConnected()
{
}

void ChannelClient::OnDisconnected()
{
}


void ChannelClient::OnMessage()
{
	byte b = 0;
	int i = 0;
	float f = 0;
	ReadByte(b);
	ReadInt(i);
	ReadFloat(f);
	ReadFloat(f);
}
