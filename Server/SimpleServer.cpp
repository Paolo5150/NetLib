#include <iostream>
#include <NetLib/NetMessage.h>
#include <NetLib/TCPServer.h>
#include <NetLib/UDPReceiver.h>

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

	bool OnClientConnection(std::shared_ptr <TCPServerClientConnection<MessageType>> client, uint32_t assignedID) override
	{
		std::cout << "Client ID " << assignedID << " Connected\n";
		return true;
	}

	void OnClientDisconnection(std::shared_ptr < TCPServerClientConnection<MessageType>> client) override
	{
		std::cout << "Client ID " << client->GetID() << " disconnected\n";

	}

	void OnMessage(std::shared_ptr<TCPServerClientConnection<MessageType>> client, const NetMessage<MessageType>& msg)
	{
		switch (msg.GetMessageID())
		{
		case MessageType::Ping:
		{
			std::cout << "Got a ping\n";
			client->Send(msg);
			break;
		}

		case MessageType::Text:
		{
			std::string t = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n";
			NetMessage<MessageType> newMsg;
			newMsg.SetMessageID(MessageType::Text);
			
			newMsg.SetPayload((void*)t.data(), sizeof(char) * t.size());
			client->Send(newMsg);
			break;
		}
		
		default:
			break;
		}
	}
};


class CustomUDPReceiver : UDPReceiver<MessageType>
{
public:
	CustomUDPReceiver( uint16_t port) :
		UDPReceiver( port)
	{

	}


};

void main()
{
	//CustomServer s(60000);
	//s.Start();

	CustomUDPReceiver c(50000);
	while (1)
	{
		//s.Update();
	}



}