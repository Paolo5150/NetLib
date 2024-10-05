#include "ServerTCP.h"

ServerTCP::ServerTCP(uint16_t port) : TCPServer<MessageType>(port)
{
}

bool ServerTCP::OnClientConnection(std::weak_ptr <TCPConnection<MessageType>> client, uint32_t assignedID)
{

	std::cout << "New client requesting connection. Approve? Y / N\n";
	m_clientMap[assignedID] = client;

	char r;
	std::cin >> r;
	if (r == 'Y' || r == 'y')
	{
		return true;
	}

	return false;
}

void ServerTCP::OnClientDisconnection(std::weak_ptr < TCPConnection<MessageType>> client)
{
	//std::cout << "Client ID " << client->GetID() << " disconnected\n";
	if (auto c = client.lock())
	{
		m_clientMap.erase(c->GetID());
	}
}

bool ServerTCP::OnIOError(std::weak_ptr<TCPConnection<MessageType>> client, std::error_code ec)
{
	//std::cout << "Client ID " << client->GetID() << " error" << ec.message() << "\n";
	return true;
}

void ServerTCP::Tick()
{
	Update(true);
}


void ServerTCP::OnMessage(std::weak_ptr<TCPConnection<MessageType>> client, const NetMessage<MessageType>& msg)
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
	switch (k)
	{
	case 1:
		std::cout << "Total connected clients: " << m_clientMap.size() << "\n";

		break;
	default:
		break;
	}
}
