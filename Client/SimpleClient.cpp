#include <iostream>
#include <NetLib/NetMessage.h>
#include <NetLib/TCPClient.h>
#include <NetLib/UDPMessager.h>
#include <chrono>
#include <sstream>

enum class MessageType : uint32_t
{
	Ping,
	Text
};

class Customclient : public TCPClient<MessageType>
{
public:
	Customclient()
	{
		std::cout << "Client on " << GetCurrentThreadId() << "\n";
	}
	void Ping()
	{
		std::cout << "Pinging server\n";
		NetMessage<MessageType> msg;
		msg.SetMessageID(MessageType::Ping);

		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
		msg.SetPayload(&timeNow, sizeof(timeNow));

		Send(msg);
	}

	void Message()
	{
		NetMessage<MessageType> msg;
		msg.SetMessageID(MessageType::Text);

		Send(msg);
	}

	void OnConnectionSuccessful() override
	{
		std::cout << "Custom client, connection ok " << GetCurrentThreadId() << "\n";
	}

	void OnConnectionFail() override
	{
		std::cout << "Custom client, connection failed " << GetCurrentThreadId() << "\n";
		
	}

};

class CustomUDPSender : public UDPMessager<MessageType>
{
public:
	CustomUDPSender() :	UDPMessager()
	{
	}

	void SendData(MessageType id, uint8_t* data, uint32_t size, const std::string& sendToAddress, uint32_t port)
	{
		NetMessage<MessageType> msg;
		msg.SetMessageID(id);
		msg.SetPayload(data, size);
		Send(msg, sendToAddress, port);
	}

	void OnDisconnection(const std::string& addressPort)
	{
	}


	void OnMessage(OwnedUDPMessage<MessageType> msg) override
	{
		auto& pl = msg.TheMessage.GetPayload();
		//Test, i know it's a string
		std::string s;
		s.resize(pl.size());
		std::memcpy((void*)s.data(), pl.data(), pl.size());

		std::cout << "Received " << s << "\n";
	}
};

void main()
{
	Customclient c;
	c.ConnectAsync("127.0.0.1", 60000);

	bool key[3] = { 0,0,0 };
	bool oldKey[3] = { 0,0,0 };

	//CustomUDPSender sender;
	//FILE* f = fopen("netconfig.txt", "r");
	//uint32_t port;
	//fscanf(f, "%d", &port);
	//std::cout << "Listening to " << port << "\n";
	//sender.StartListening(port);

	bool quittime = false;
	while (!quittime)
	{
		//sender.Update(true);
		if (GetForegroundWindow() == GetConsoleWindow())
		{
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
		}

		if (key[2] && !oldKey[2])
		{
			//std::cout << "Pressed 3\n";
			quittime = true;
		}
		if (key[0] && !oldKey[0])
		{
			//std::cout << "Pressed 1\n";
			std::stringstream m;
			std::string t = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n";
			m << "Ciao\n";

			c.Ping();
		//	sender.SendData(MessageType::Text, (uint8_t*)m.str().data(), m.str().size() * sizeof(char), "127.0.0.1", 50000);
		}

		if (key[1] && !oldKey[1])
			c.Message();

		for (int i = 0; i < 3; i++)
			oldKey[i] = key[i];

		if (c.IsConnected())
		{
			if (!c.GetMessages().Empty())
			{
				auto msg = c.GetMessages().PopFront();
				switch (msg.GetMessageID())
				{
				case MessageType::Ping:
				{
					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					auto p = msg.GetPayload();
					std::memcpy(&timeThen, p.data(), p.size());
					//msg >> timeThen;
					std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
				}
				break;
		
				case MessageType::Text:
				{
					std::cout << "Text:\n";
		
					std::string t;
					t.resize(msg.GetPayloadSize());
					std::memcpy(t.data(), msg.GetPayload().data(), msg.GetPayloadSize());
		
					std::cout << "Text: " << t << "\n";
				}
				break;
				default:
					break;
				}
			}
		}
		else
		{
			CallbackToClient cc;
			if (c.GetLatestConnectionCallback( cc))
			{
				if (cc.CType == CallbackType::ConnectionFail)
				{
					std::cout << "Failed to connect " << cc.Message << std::endl;
					static int attempts = 3;
					if (attempts > 0)
					{
						attempts--;
						c.ConnectAsync("127.0.0.1", 60000);

					}
				}
				else if (cc.CType == CallbackType::IOError)
				{
					std::cout << "IOError " << cc.Message << std::endl;
					c.Destroy();
					quittime = 1;
				}
				
			}
		}
	}

}