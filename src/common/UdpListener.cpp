#include "UdpListener.h"
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
UdpListener::UdpListener():
	m_Socket(NULL),
	m_MessagePacket(NULL)
{
}

UdpListener::~UdpListener()
{
}

bool UdpListener::CreateUdpServer(const char * ip, int port, int max_client)
{
	m_Socket = RakNet::RakPeerInterface::GetInstance();
	RakNet::RakNetStatistics *rss;
	m_Socket->SetIncomingPassword("", 0);
	m_Socket->SetTimeoutTime(30000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
	RakNet::SocketDescriptor socketDescriptors[2];
	socketDescriptors[0].port = port;
	socketDescriptors[0].socketFamily = AF_INET; // Test out IPV4
	bool b = m_Socket->Startup(max_client, socketDescriptors, 1) == RakNet::RAKNET_STARTED;
	if (b)
	{
		m_Socket->SetMaximumIncomingConnections(4);
		m_Socket->SetOccasionalPing(true);
		m_Socket->SetUnreliableTimeout(1000);
		return true;
	}
	else
	{
		m_Socket->Shutdown(300);
		// We're done with the network
		RakNet::RakPeerInterface::DestroyInstance(m_Socket);
		return false;
	}
	
}



void UdpListener::Update()
{
	if (NULL == m_Socket)return;
	for (m_MessagePacket = m_Socket->Receive(); m_MessagePacket; m_Socket->DeallocatePacket(m_MessagePacket), m_MessagePacket = m_Socket->Receive())
	{
		// We got a packet, get the identifier with our handy function
		unsigned char packetIdentifier = GetPacketIdentifier(m_MessagePacket);

		// Check if this is a network message packet
		switch (packetIdentifier)
		{
		case ID_DISCONNECTION_NOTIFICATION:
			// Connection lost normally
			OnUdpClientDisconnected(m_MessagePacket);
			printf("ID_DISCONNECTION_NOTIFICATION from %s\n", m_MessagePacket->systemAddress.ToString(true));;
			break;


		case ID_NEW_INCOMING_CONNECTION:
			// Somebody connected.  We have their IP now
			OnUdpAccept(m_MessagePacket);
			printf("ID_NEW_INCOMING_CONNECTION from %s with GUID %s\n", m_MessagePacket->systemAddress.ToString(true), m_MessagePacket->guid.ToString());
			//clientID = p->systemAddress; // Record the player ID of the client

			printf("Remote internal IDs:\n");
			for (int index = 0; index < MAXIMUM_NUMBER_OF_INTERNAL_IDS; index++)
			{
				RakNet::SystemAddress internalId = m_Socket->GetInternalID(m_MessagePacket->systemAddress, index);
				if (internalId != RakNet::UNASSIGNED_SYSTEM_ADDRESS)
				{
					printf("%i. %s\n", index + 1, internalId.ToString(true));
				}
			}

			break;

		case ID_INCOMPATIBLE_PROTOCOL_VERSION:
			printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
			break;

		case ID_CONNECTED_PING:
		case ID_UNCONNECTED_PING:
			printf("Ping from %s\n", m_MessagePacket->systemAddress.ToString(true));
			break;

		case ID_CONNECTION_LOST:
			// Couldn't deliver a reliable packet - i.e. the other system was abnormally
			// terminated
			OnUdpClientDisconnected(m_MessagePacket);
			printf("ID_CONNECTION_LOST from %s\n", m_MessagePacket->systemAddress.ToString(true));;
			break;

		default:
			// The server knows the static data of all clients, so we can prefix the message
			// With the name data
			//printf("%s\n", m_MessagePacket->data);

			// Relay the message.  We prefix the name for other clients.  This demonstrates
			// That messages can be changed on the server before being broadcast
			// Sending is the same as before
			//sprintf(message, "%s", p->data);
			//m_Socket->Send(message, (const int)strlen(message) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, true);
			OnUdpClientMessage(m_MessagePacket);
			break;
		}

	}
}
