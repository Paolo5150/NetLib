#include <iostream>
#include <NetLib/Message.h>
#include <NetLib/NetClient.h>
#include <chrono>

enum class MessageType : uint32_t
{
	Ping
};

class Customclient : public Client<MessageType>
{
public: 
	void Ping()
	{
		std::cout << "Pinging server\n";
		Message<MessageType> msg;
		msg.Header.ID = MessageType::Ping;

		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
		msg << timeNow;

		Send(msg);
	}
};

void main()
{
	Customclient c;
	c.Connect("127.0.0.1", 60000);

	bool key[3] = { 0,0,0 };
	bool oldKey[3] = { 0,0,0 };



	bool quittime = false;
	while (!quittime)
	{

		if (GetForegroundWindow() == GetConsoleWindow())
		{
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
		}

		if (key[2] && !oldKey[2])
		{
			std::cout << "Pressed 3\n";
			quittime = true;
		}
		if (key[0] && !oldKey[0])
			c.Ping();

		for (int i = 0; i < 3; i++)
			oldKey[i] = key[i];

		if (c.IsConnected())
		{
			if (!c.GetMessages().Empty())
			{
				auto msg = c.GetMessages().PopFront().TheMessage;
				switch (msg.Header.ID)
				{
				case MessageType::Ping:
				{
					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					msg >> timeThen;
					std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
				}
					break;
				default:
					break;
				}
			}
		}
		else
		{
			std::cout << "Server down, bye\n";
			quittime = true;
		}
	}

}