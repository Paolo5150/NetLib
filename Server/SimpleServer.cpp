#include <iostream>
#include <NetLib/TCPMessage.h>
#include <NetLib/TCPServer.h>

enum class MessageType : uint32_t
{
	Ping
};

class CustomServer : public TCPServer<MessageType>
{
public:
	CustomServer(uint16_t port) : TCPServer<MessageType>(port)
	{

	}

	bool OnClientConnection(std::shared_ptr <TCPConnection<MessageType>> client) override
	{
		std::cout << "Client ID " << client->GetID() << " Connected\n";
		return true;
	}

	void OnClientDisconnection(std::shared_ptr < TCPConnection<MessageType>> client) override
	{
		std::cout << "Client ID " << client->GetID() << " disconnected\n";

	}

	void OnMessage(std::shared_ptr<TCPConnection<MessageType>> client, const TCPMessage<MessageType>& msg)
	{
		switch (msg.Header.ID)
		{
		case MessageType::Ping:
		{
			std::cout << "Got a ping\n";
			client->Send(msg);
		}
		default:
			break;
		}
	}
};

void main()
{
	CustomServer s(60000);
	s.Start();
	while (1)
	{
		s.Update();
	}


}