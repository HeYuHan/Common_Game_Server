#pragma once
#ifndef __UDPCONNECTION_H__
#define __UDPCONNECTION_H__
#include "NetworkConnection.h"
#include <MessageIdentifiers.h>
#include <RakPeerInterface.h>
#include <RakNetTypes.h>
#include <BitStream.h>
#include <PacketLogger.h>
#include <RakNetTypes.h>
#define KEEP_ALIVE_MSG 253
using namespace RakNet;
class UdpConnection :public NetworkConnection
{
public:
	UdpConnection();
	~UdpConnection();
	virtual void Update(float time);
	virtual int Read(void* data, int size);
	virtual int Send(void* data, int size);
	bool Connect(const char* ip, int port);
	void InitServerSocket(RakPeerInterface* server, SystemAddress address);
	void OnServerMessage(Packet* p);
	void DisConnect();
	void KeepAlive();
private:
	bool m_Connected;
	int m_KeepAliveLostCount;
	float m_KeepAliveTime;
public:
	
	bool m_IsServer;
	RakPeerInterface *m_Socket;
	SystemAddress m_SystemAddress;
	Packet* m_MessagePacket;
	

};



#endif // !__UDPCONNECTION_H__
