#include "ServerTCP.h"

ServerTCP::ServerTCP(uint16_t port) : TCPServer<MessageType>(port)
{
}

bool ServerTCP::OnClientConnection(std::shared_ptr <TCPConnection<MessageType>> client, uint32_t assignedID) 
{

	std::cout << "New client requesting connection. Approve? Y / N\n";
	char r;
	std::cin >> r;
	if (r == 'Y' || r == 'y')
	{
		m_clientMap[assignedID] = client;
		return true;
	}

	return false;
}

void ServerTCP::OnClientDisconnection(std::shared_ptr < TCPConnection<MessageType>> client) 
{
	//std::cout << "Client ID " << client->GetID() << " disconnected\n";
	m_clientMap.erase(client->GetID());
}

bool ServerTCP::OnIOError(std::shared_ptr<TCPConnection<MessageType>> client, std::error_code ec) 
{
	//std::cout << "Client ID " << client->GetID() << " error" << ec.message() << "\n";
	return true;
}

void ServerTCP::Tick()
{
	Update(true);
}


void ServerTCP::OnMessage(std::shared_ptr<TCPConnection<MessageType>> client, const NetMessage<MessageType>& msg)
{
	switch (msg.GetMessageID())
	{
	case MessageType::Ping:
	{

		break;
	}

	case MessageType::Text:
	{

		break;
	}

	default:
		break;
	}
}

void ServerTCP::OnKeyPressed(int k)
{
	std::cout << "Key pressed " << k << "\n";
}
