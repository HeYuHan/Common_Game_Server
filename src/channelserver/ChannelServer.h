#pragma once
#ifndef __CHANNELSERVER_H__
#define __CHANNELSERVER_H__
#include <BaseServer.h>
#include <unordered_map>
#include <objectpool.h>
#include <UdpListener.h>
#include "Room.h"
using namespace RakNet;
class ChannelClient;
typedef std::tr1::unordered_map<uint64, ChannelClient*> UdpClientMap;
typedef std::pair<uint64, ChannelClient*> UdpClientMapPair;
typedef std::tr1::unordered_map<uint64, ChannelClient*>::iterator UdpClientIterator;
struct ChannelConfig
{
	char ip[64] = "127.0.0.1";
	char pwd[64] = "channel";
	int port=9500;
	int max_client=512;
	int max_room = 100;
};
class ChannelServer:public BaseServer,public UdpListener
{
public:
	ChannelServer();
	~ChannelServer();
	virtual void OnUdpClientMessage(Packet* p);
	virtual void OnUdpClientDisconnected(Packet* p);
	virtual void OnUdpAccept(Packet* p);
	virtual bool Init();
	virtual int Run();
	void Update(float time);
public:
	ChannelConfig m_Config;
	ObjectPool<ChannelClient> m_ClientPool;
	ObjectPool<ChannelRoom> m_RoomPool;
private:
	UdpClientMap m_UdpClientMap;
	
};
extern ChannelServer gChannelServer;


#endif // !__CHANNELSERVER_H__


