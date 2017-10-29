#include "pch.h"
ChannelClient::ChannelClient()
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
