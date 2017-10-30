#pragma once
#ifndef __CHANNELCLIENT_H__
#define __CHANNELCLIENT_H__
#include <UdpConnection.h>
class ChannelClient:public NetworkStream,public UdpConnection
{
public:
	ChannelClient();
	~ChannelClient();
	virtual void OnConnected();
	virtual void OnDisconnected();
	virtual void DisConnect();
	virtual void OnMessage();
public:
	unsigned int uid;
	NetState m_NetState;
private:

};


#endif // !__CHANNELCLIENT_H__

