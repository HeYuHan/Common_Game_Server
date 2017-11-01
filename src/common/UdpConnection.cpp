#include "UdpConnection.h"
static unsigned char GetPacketIdentifier(RakNet::Packet *p)
{
	if (p == 0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		RakAssert(p->length > sizeof(RakNet::MessageID) + sizeof(RakNet::Time));
		return (unsigned char)p->data[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)];
	}
	else
		return (unsigned char)p->data[0];
}
UdpConnection::UdpConnection():
	m_IsServer(false),
	m_Socket(NULL),
	m_SystemAddress(UNASSIGNED_SYSTEM_ADDRESS),
	m_MessagePacket(NULL)
{
	m_Type = UDP_SOCKET;
}

UdpConnection::~UdpConnection()
{
}

void UdpConnection::Update()
{
	if (NULL == m_Socket || m_IsServer)return;
	for (m_MessagePacket = m_Socket->Receive(); m_MessagePacket; m_Socket->DeallocatePacket(m_MessagePacket), m_MessagePacket = m_Socket->Receive())
	{
		// We got a packet, get the identifier with our handy function
		unsigned char packetIdentifier = GetPacketIdentifier(m_MessagePacket);

		// Check if this is a network message packet
		switch (packetIdentifier)
		{
		case ID_DISCONNECTION_NOTIFICATION:
			// Connection lost normally
			printf("ID_DISCONNECTION_NOTIFICATION\n");
			OnDisconnected();
			break;
		case ID_ALREADY_CONNECTED:
			// Connection lost normally
			//printf("ID_ALREADY_CONNECTED with guid %" PRINTF_64_BIT_MODIFIER "u\n", m_MessagePacket->guid);
			break;
		case ID_INCOMPATIBLE_PROTOCOL_VERSION:
			printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
			break;
		case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
			printf("ID_REMOTE_DISCONNECTION_NOTIFICATION\n");
			break;
		case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
			printf("ID_REMOTE_CONNECTION_LOST\n");
			OnDisconnected();
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
			printf("ID_REMOTE_NEW_INCOMING_CONNECTION\n");
			break;
		case ID_CONNECTION_BANNED: // Banned from this server
			printf("We are banned from this server.\n");
			break;
		case ID_CONNECTION_ATTEMPT_FAILED:
			printf("Connection attempt failed\n");
			OnDisconnected();
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			// Sorry, the server is full.  I don't do anything here but
			// A real app should tell the user
			printf("ID_NO_FREE_INCOMING_CONNECTIONS\n");
			OnDisconnected();
			break;

		case ID_INVALID_PASSWORD:
			printf("ID_INVALID_PASSWORD\n");
			OnDisconnected();
			break;

		case ID_CONNECTION_LOST:
			// Couldn't deliver a reliable packet - i.e. the other system was abnormally
			// terminated
			printf("ID_CONNECTION_LOST\n");
			//OnDisconnected();
			break;

		case ID_CONNECTION_REQUEST_ACCEPTED:
			// This tells the client they have connected
			printf("ID_CONNECTION_REQUEST_ACCEPTED to %s with GUID %s\n", m_MessagePacket->systemAddress.ToString(true), m_MessagePacket->guid.ToString());
			printf("My external address is %s\n", m_Socket->GetExternalID(m_MessagePacket->systemAddress).ToString(true));
			m_SystemAddress = m_MessagePacket->systemAddress;
			OnConnected();
			break;
		case ID_CONNECTED_PING:
		case ID_UNCONNECTED_PING:
			printf("Ping from %s\n", m_MessagePacket->systemAddress.ToString(true));
			break;
		default:
			// It's a client, so just show the message
			//printf("%s\n", p->data);
			if (stream)stream->OnRevcMessage();
			break;
		}
	}
}

int UdpConnection::Read(void * data, int size)
{
	if (m_MessagePacket)
	{
		int data_size = m_MessagePacket->length - 1;
		if (size < data_size)
		{
			throw "data size too less";
		}
		memcpy(data, m_MessagePacket->data+1, data_size);
		return data_size;
	}
	return 0;
}

int UdpConnection::Send(void * data, int size)
{
	if (0 == m_Socket->Send((char*)data, size, HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_IsServer?m_SystemAddress:RakNet::UNASSIGNED_SYSTEM_ADDRESS, false))
	{
		return 0;
	}
	return size;
}



bool UdpConnection::Connect(const char * ip, int port)
{
	m_IsServer = false;
	m_Socket= RakPeerInterface::GetInstance();
	m_Socket->AllowConnectionResponseIPMigration(false);
	RakNet::SocketDescriptor socketDescriptor(0, 0);
	socketDescriptor.socketFamily = AF_INET;
	m_Socket->Startup(8, &socketDescriptor, 1);
	m_Socket->SetOccasionalPing(true);
	RakNet::ConnectionAttemptResult car = m_Socket->Connect(ip, port, "", 0);
	if (car == RakNet::CONNECTION_ATTEMPT_STARTED)
	{
		return true;
	}
	else
	{
		m_Socket->Shutdown(300);
		RakPeerInterface::DestroyInstance(m_Socket);
		return false;
	}
	
}

void UdpConnection::InitServerSocket(RakPeerInterface * server, SystemAddress address)
{
	m_IsServer = true;
	m_SystemAddress = address;
	m_Socket = server;
}


void UdpConnection::OnServerMessage(Packet* p)
{
	m_MessagePacket = p;
	if (stream)stream->OnRevcMessage();
}

void UdpConnection::DisConnect()
{
	m_Socket->CloseConnection(m_SystemAddress, false);
	if (!m_IsServer)
	{
		m_Socket->Shutdown(300);
		RakPeerInterface::DestroyInstance(m_Socket);
		m_SystemAddress = UNASSIGNED_SYSTEM_ADDRESS;
		m_Socket = NULL;
	}
	OnDisconnected();
	
}
