#include <iostream>
#include <NetLib/TCPMessage.h>
#include <NetLib/TCPServer.h>

enum class MessageType : uint32_t
{
	Ping,
	Text
};

class CustomServer : public TCPServer<MessageType>
{
public:
	CustomServer(uint16_t port) : TCPServer<MessageType>(port)
	{

	}

	bool OnClientConnection(std::shared_ptr <TCPConnection<MessageType>> client, uint32_t assignedID) override
	{
		std::cout << "Client ID " << assignedID << " Connected\n";
		return false;
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
			break;
		}

		case MessageType::Text:
		{
			std::string t = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
			TCPMessage<MessageType> newMsg;
			newMsg.Header.ID = MessageType::Text;
			
			newMsg.SetData((void*)t.data(), sizeof(char) * t.size());
			client->Send(newMsg);
			break;
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