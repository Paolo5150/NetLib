#include "ClientTCP.h"

ClientTCP::ClientTCP()
{
	std::cout << "Client on " << GetCurrentThreadId() << "\n";
}

void ClientTCP::OnKeyPressed(int n)
{
	m_fbBuilder.Clear();

	switch (n)
	{
	case 1:
		ConnectAsync(SERVER_ADDRESS, SERVER_PORT);
		break;
	case 2:
		Disconnect();
		break;

	case 3: //Send ping
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto duration = now.time_since_epoch();

		// Convert duration to nanoseconds and to int64_t (or uint64_t)
		double timeNow = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		auto pingmsg = CreatePingMsg(m_fbBuilder, timeNow);
		auto msg = CreateMessage(m_fbBuilder, MessageUnion_PingMsg, pingmsg.o);
		m_fbBuilder.Finish(msg);

		NetMessage<MessageType> mymsg;
		mymsg.SetMessageID(MessageType_Ping);
		mymsg.SetPayload(m_fbBuilder.GetBufferPointer(), m_fbBuilder.GetSize());
		Send(mymsg);
		
	}
		break;

	case 4: //Send hello to all connected clients.
	{
		std::string hi = "Hi everyone!";
		auto multi = CreateMulticastTextMsg(m_fbBuilder, m_fbBuilder.CreateString(hi));
		auto msg = CreateMessage(m_fbBuilder, MessageUnion_MulticastTextMsg, multi.o);
		m_fbBuilder.Finish(msg);

		NetMessage<MessageType> mymsg;
		mymsg.SetMessageID(MessageType_MulticastText);
		mymsg.SetPayload(m_fbBuilder.GetBufferPointer(), m_fbBuilder.GetSize());
		Send(mymsg);
	}
		break;

	default:
		break;
	}
}

void ClientTCP::Tick()
{
	CallbackToClient cc;
	if (GetLatestCallback(cc))
	{
		if (cc.CType == CallbackType::ConnectionFail)
		{
			std::cout << "Failed to connect " << cc.Message << std::endl;
		}
		else if (cc.CType == CallbackType::ConnectionSuccess)
		{
			std::cout << "Connection OK!\n";

		}
		else if (cc.CType == CallbackType::IOError)
		{
			std::cout << "IOError " << cc.Message << std::endl;
			Destroy();
		}		
	}

	if (IsConnected())
	{
		if (!GetMessages().Empty())
		{
			auto msg = GetMessages().PopFront();
			switch (msg.GetMessageID())
			{
			case MessageType::MessageType_Ping:
			{			
				auto pl = msg.GetPayload();
				auto root = flatbuffers::GetRoot<Message>(pl.data());
				auto pingMsg = root->payload_as_PingMsg();

				auto now = std::chrono::high_resolution_clock::now();
				auto duration = now.time_since_epoch();
				double millsNow = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
				double timeThen = pingMsg->time();
				double diff = (millsNow - timeThen) / 1000.0;
				std::cout << "Ping time mills: " << diff << std::endl;
			}
			break;
	
			case MessageType::MessageType_MulticastText:
			{
				auto pl = msg.GetPayload();
				auto root = flatbuffers::GetRoot<Message>(pl.data());
				auto multiMsg = root->payload_as_MulticastTextMsg();
				auto msg = multiMsg->msg();
				std::cout << "received multicast " << msg->c_str() << std::endl;
			}
			break;
			default:
				break;
			}
		}
	}

}

