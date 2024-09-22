#include <iostream>
#include <NetLib/Message.h>
#include <NetLib/NetServer.h>

enum class MessageType : uint32_t
{
	Ping
};

class CustomServer : public Server<MessageType>
{
public:
	CustomServer(uint16_t port) : Server<MessageType>(port)
	{

	}

	bool OnClientConnection(std::shared_ptr <Connection<MessageType>> client) override
	{
		return true;
	}

	void OnClientDisconnection(std::shared_ptr < Connection<MessageType>> client) override
	{
	}

	void OnMessage(std::shared_ptr<Connection<MessageType>> client, const Message<MessageType>& msg)
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