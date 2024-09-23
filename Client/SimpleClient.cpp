#include <iostream>
#include <NetLib/TCPMessage.h>
#include <NetLib/TCPClient.h>
#include <NetLib/UDPSender.h>
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
	void Ping()
	{
		std::cout << "Pinging server\n";
		TCPMessage<MessageType> msg;
		msg.Header.ID = MessageType::Ping;

		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
		msg << timeNow;

		Send(msg);
	}

	void Message()
	{
		TCPMessage<MessageType> msg;
		msg.Header.ID = MessageType::Text;

		Send(msg);
	}

	void OnConnectionSuccessful() override
	{
		std::cout << "Custom client, connection ok\n";
	}

	void OnConnectionFail() override
	{

	}

};

class CustomUDPSender : UDPSender<MessageType>
{
public:
	CustomUDPSender(const std::string& sendToAddress, uint16_t port) :
		UDPSender(sendToAddress, port)
	{

	}

	void SendData(void* data, size_t size)
	{
		Send(data, size);
	}
};

void main()
{



	//Customclient c;
	//c.Connect("127.0.0.1", 60000);

	bool key[3] = { 0,0,0 };
	bool oldKey[3] = { 0,0,0 };

	CustomUDPSender sender("127.0.0.1", 50000);

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
			//std::cout << "Pressed 3\n";
			quittime = true;
		}
		if (key[0] && !oldKey[0])
		{
			//std::cout << "Pressed 1\n";
			std::stringstream m;
			for (int i = 0; i < 1490; i++)
			{
				m << 'a';
			}

			//c.Ping();
			sender.SendData(m.str().data(), m.str().size() * sizeof(char));
		}

		//if (key[1] && !oldKey[1])
		//	c.Message();

		for (int i = 0; i < 3; i++)
			oldKey[i] = key[i];

		//if (c.IsConnected())
		//{
		//	if (!c.GetMessages().Empty())
		//	{
		//		auto msg = c.GetMessages().PopFront();
		//		switch (msg.Header.ID)
		//		{
		//		case MessageType::Ping:
		//		{
		//			std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
		//			std::chrono::system_clock::time_point timeThen;
		//			msg >> timeThen;
		//			std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
		//		}
		//		break;
		//
		//		case MessageType::Text:
		//		{
		//			std::cout << "Text:\n";
		//
		//			std::string t;
		//			t.resize(msg.Body.size());
		//			std::memcpy(t.data(), msg.Body.data(), msg.Body.size());
		//
		//			std::cout << "Text: " << t << "\n";
		//		}
		//		break;
		//		default:
		//			break;
		//		}
		//	}
		//}
		//else
		//{
		//	std::cout << "Server down, bye\n";
		//	quittime = true;
		//}
	}

}