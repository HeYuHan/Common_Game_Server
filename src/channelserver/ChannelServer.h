#pragma once
#ifndef __CHANNELSERVER_H__
#define __CHANNELSERVER_H__
#ifndef _WIN32
#include <tr1/unordered_map>
#else
#include <unordered_map>
#endif
#include <BaseServer.h>

#include <objectpool.h>
#include <UdpListener.h>
#include<vector>
using namespace RakNet;
class ChannelRoom;
class ChannelClient;
typedef std::vector<ChannelClient*>::iterator ClientIterator;
typedef std::vector<ChannelRoom*>::iterator RoomIterator;
typedef std::pair<uint64, ChannelClient*> UdpClientMapPair;
typedef std::tr1::unordered_map<uint64, ChannelClient*> UdpClientMap;
typedef std::tr1::unordered_map<uint64, ChannelClient*>::iterator UdpClientIterator;
struct ChannelConfig
{
	char ip[64];
	char pwd[64];
	int port;
	int max_client;
	int max_room;
public:
	ChannelConfig();
};
class ChannelServer:public BaseServer,public UdpListener
{
public:
	ChannelServer();
	~ChannelServer();
	virtual void OnUdpClientMessage(Packet* p);
	virtual void OnUdpClientDisconnected(Packet* p);
	virtual void OnUdpAccept(Packet* p);
	virtual void OnKeepAlive(Packet* p);
	virtual bool Init();
	virtual int Run();
	void Update(float time);
	ChannelRoom* GetRoom(int state,bool check_full=true);
	ChannelRoom* CreateNewRoom();
	void FreeRoom(ChannelRoom* room);
	void RemoveClient(ChannelClient* c);
public:
	ChannelConfig m_Config;
	ObjectPool<ChannelClient> m_ClientPool;
	ObjectPool<ChannelRoom> m_RoomPool;
	std::vector<ChannelRoom*> m_RoomList;
	
private:
	UdpClientMap m_UdpClientMap;
	
};
extern ChannelServer gChannelServer;


#endif // !__CHANNELSERVER_H__


