#include "ClientTCP.h"

ClientTCP::ClientTCP()
{
	std::cout << "Client on " << GetCurrentThreadId() << "\n";
}


void ClientTCP::OnConnectionSuccessful() 
{
	std::cout << "Custom client, connection ok " << GetCurrentThreadId() << "\n";
}

void ClientTCP::OnConnectionFail() 
{
	std::cout << "Custom client, connection failed " << GetCurrentThreadId() << "\n";

}

void ClientTCP::OnKeyPressed(int n)
{
	std::cout << "Tcp guy pressed " << n << "\n";
	switch (n)
	{
	case 1:
		ConnectAsync("127.0.0.1", 60000);
		break;
	case 2:
		Disconnect();
		break;

	default:
		break;
	}
}

void ClientTCP::Tick()
{
	if (IsConnected())
	{
		if (!GetMessages().Empty())
		{
			auto msg = GetMessages().PopFront();
			switch (msg.GetMessageID())
			{
			case MessageType::Ping:
			{			
			}
			break;
	
			case MessageType::Text:
			{
			
			}
			break;
			default:
				break;
			}
		}
	}
	else
	{
		//CallbackToClient cc;
		//if (GetLatestConnectionCallback( cc))
		//{
		//	if (cc.CType == CallbackType::ConnectionFail)
		//	{
		//		std::cout << "Failed to connect " << cc.Message << std::endl;
		//		static int attempts = 3;
		//		if (attempts > 0)
		//		{
		//			attempts--;
		//			ConnectAsync("127.0.0.1", 60000);
		//
		//		}
		//	}
		//	else if (cc.CType == CallbackType::IOError)
		//	{
		//		std::cout << "IOError " << cc.Message << std::endl;
		//		Destroy();
		//	}
		//	
		//}
	}
}

