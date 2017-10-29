#pragma once
#ifndef __CHANNELSERVER_H__
#define __CHANNELSERVER_H__
#include <BaseServer.h>
#include <unordered_map>
#include <objectpool.h>
#include <UdpListener.h>
using namespace RakNet;
class ChannelClient;
typedef std::tr1::unordered_map<uint64, ChannelClient*> UdpClientMap;
typedef std::pair<uint64, ChannelClient*> UdpClientMapPair;
typedef std::tr1::unordered_map<uint64, ChannelClient*>::iterator UdpClientIterator;
class ChannelServer:public BaseServer,public UdpListener
{
public:
	ChannelServer();
	~ChannelServer();
	virtual void OnUdpClientMessage(Packet* p);
	virtual void OnUdpClientDisconnected(Packet* p);
	virtual void OnUdpAccept(Packet* p);
	virtual void Init();
	void Update(float time);

private:
	UdpClientMap m_UdpClientMap;
	ObjectPool<ChannelClient> m_ClientPool;
};
extern ChannelServer gChannelServer;


#endif // !__CHANNELSERVER_H__


