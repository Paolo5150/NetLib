#include "ClientUDP.h"

ClientUDP::ClientUDP() : UDPMessager()
{
	std::cout << "Press 1 to open port\n";
}

void ClientUDP::OnDisconnection(const std::string& addressPort)
{
}

bool ClientUDP::OnIOError(std::error_code ec)
{
	return true;
}
void ClientUDP::OnMessage(OwnedUDPMessage<MessageType> msg) 
{
	switch (msg.TheMessage.GetMessageID())
	{
		case MessageType::MessageType_Ping:
		{
			auto pl = msg.TheMessage.GetPayload();
			auto root = flatbuffers::GetRoot<Message>(pl.data());
			auto pingMsg = root->payload_as_PingMsg();

			auto now = std::chrono::high_resolution_clock::now();
			auto duration = now.time_since_epoch();
			// Convert duration to nanoseconds and to int64_t (or uint64_t)
			double millsNow = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
			double timeThen = pingMsg->time();
			double diff = (millsNow - timeThen) / 1000.0;
			std::cout << "Ping time mills: " << diff << std::endl;
		}
		break;
		case MessageType_Position:
		{
			auto pl = msg.TheMessage.GetPayload();
			auto root = flatbuffers::GetRoot<Message>(pl.data());
			auto pos = root->payload_as_TransformPosition();
			std::cout << pos->x() << " " << pos->y() << " " << pos->z() << std::endl;
		}
		break;
	}
}

void ClientUDP::OnKeyPressed(int n)
{
	m_fbBuilder.Clear();
	switch (n)
	{
	case 1:
	{
		std::cout << "Which port? (ef. 50000)\n";
		uint32_t port;
		std::cin >> port;
		StartListening(port);
	}
		break;
	case 2:
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
		Send(mymsg, SERVER_ADDRESS, SERVER_PORT);
	}
		break;

	case 3:
	{
		for (float i = 0; i < 500; i += 0.25f)
		{
			m_fbBuilder.Clear();
			auto p = CreateTransformPosition(m_fbBuilder, i, i, i);
			auto msg = CreateMessage(m_fbBuilder, MessageUnion_TransformPosition, p.o);
			m_fbBuilder.Finish(msg);

			NetMessage<MessageType> mymsg;
			mymsg.SetMessageID(MessageType_Position);
			mymsg.SetPayload(m_fbBuilder.GetBufferPointer(), m_fbBuilder.GetSize());
			Send(mymsg, SERVER_ADDRESS, SERVER_PORT);
		}
	}
		break;
	default:
		break;
	}
}

void ClientUDP::Tick()
{
	Update(true);
}

