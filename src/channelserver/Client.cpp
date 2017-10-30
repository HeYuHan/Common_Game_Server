#include "pch.h"
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

void ChannelClient::DisConnect()
{
}

void ChannelClient::OnMessage()
{

}
