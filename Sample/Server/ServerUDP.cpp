#include "ServerUDP.h"

ServerUDP::ServerUDP() : UDPMessager()
{

}

void ServerUDP::OnDisconnection(const std::string& addressPort)
{
}

bool ServerUDP::OnIOError(std::error_code ec)
{
	return true;
}

void ServerUDP::OnMessage(OwnedUDPMessage<MessageType> msg) 
{
	switch (msg.TheMessage.GetMessageID())
	{
	case MessageType_Ping:
		std::cout << "Got ping from " << msg.RemoteAddress << " " << msg.RemotePort << std::endl;
		Send(msg.TheMessage, msg.RemoteAddress, msg.RemotePort);
		break;
	case MessageType_Position:
		auto pl = msg.TheMessage.GetPayload();
		auto root = flatbuffers::GetRoot<Message>(pl.data());
		auto pos = root->payload_as_TransformPosition();
		std::cout << pos->x() << " " << pos->y() << " " << pos->z() << std::endl;
		//Just bounce back the message to test send/receive on both ends
		Send(msg.TheMessage, msg.RemoteAddress, msg.RemotePort);

		break;
	}
}

void ServerUDP::Tick()
{
	Update(true);
}

void ServerUDP::OnKeyPressed(int k)
{
	std::cout << "Key pressed " << k << "\n";
	switch (k)
	{
	case 1:
		std::cout << "Which port? (ef. 50000)\n";
		uint32_t port;
		std::cin >> port;
		StartListening(port);
		break;
	default:
		break;
	}
}
