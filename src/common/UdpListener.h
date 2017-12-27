#pragma once
#ifndef __UDPLISTENER_H__
#define __UDPLISTENER_H__
#include <MessageIdentifiers.h>
#include <RakPeerInterface.h>
#include <RakNetTypes.h>
#include <BitStream.h>
#include <PacketLogger.h>
#include <RakNetTypes.h>
using namespace RakNet;

class UdpListener
{
public:
	UdpListener();
	~UdpListener();
	bool CreateUdpServer(const char *addr, const char* pwd,int max_client);
	virtual void OnUdpAccept(Packet* p)=0;
	void Update();
	virtual void OnUdpClientMessage(Packet* p)=0;
	virtual void OnUdpClientDisconnected(Packet* p)=0;
	virtual void OnKeepAlive(Packet* p) = 0;
	void CloseClient(SystemAddress &address);
public:
	RakPeerInterface *m_Socket;
	Packet* m_MessagePacket;
};




#endif // !__UDPLISTENER_H__
