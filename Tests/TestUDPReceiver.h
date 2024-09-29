#pragma once
#include "NetLib/UDPReceiver.h"
#include "NetLib/UDPPacketAssembler.h"
#include "Common.h"
#include <string>
#include <random>

class MockUDPReceiver : public UDPReceiver<MessageType>
{
public:
	MockUDPReceiver(uint16_t port) :
		UDPReceiver(port, false)
	{
	}

	void OnMessage(OwnedUDPMessage<MessageType> msg) override
	{
		std::string s;
		s.resize(msg.TheMessage.GetPayloadSize());

		assert(msg.RemoteAddress == "127.0.0.1");
		assert(msg.RemotePort == 12345);

		//Retrieve string sent
		std::memcpy(s.data(), msg.TheMessage.GetPayload().data(), msg.TheMessage.GetPayloadSize());
		assert(s == ExpectedMessage);

	}

	void SetReceiveBuffer(uint8_t* data, size_t size, asio::ip::udp::endpoint endp)
	{
		std::memcpy(m_receiveBuffer, data, size);
		CheckIncompleteMessages();
		ProcessPacket(endp, size);
	}

	void AssertMapSize(int size)
	{
		assert(m_packetMap.size() == size);
	}

	void AssertInMessagesSize(int size)
	{
		assert(GetAvailableMessagesCount() == size);
	}

	void AssertHasPendingPacket(uint16_t id)
	{
		assert(!HasPendingPacketOfId(id));
	}

	std::string ExpectedMessage;
};

void TestUDPReceiver()
{
	MockUDPReceiver mr(90000);
	asio::ip::udp::endpoint ep(asio::ip::address::from_string("127.0.0.1"), 12345);

	{
		UDPPacket<MessageType> p1;
		p1.SetHeader(MessageType::Text, 0, 0, 1);

		std::string message = "Hello";
		p1.SetPayload((uint8_t*)message.data(), message.size());

		//This emulates the Receive function, when data comes in
		mr.SetReceiveBuffer(p1.DataBuffer.data(), p1.DataBuffer.size(), ep);
		//Single packet, message should have been processed
		mr.AssertInMessagesSize(1);
		//Map should be clear, no other expected packets
		mr.AssertMapSize(0);
		mr.ExpectedMessage = message;
		mr.Update(-1); //Will trigger OnMessage, more asserts there
	}

	//Dropped packets
	{
		//Two packets, but send one, verify incomplete messages are nuked
		UDPPacket<MessageType> p1;
		p1.SetHeader(MessageType::Text, 0, 0, 2);

		UDPPacket<MessageType> p2;
		p2.SetHeader(MessageType::Text, 0, 1, 2);

		//Unrelated packet
		UDPPacket<MessageType> p3;
		p3.SetHeader(MessageType::Text, 1, 0, 2);

		//Send 1st packet only
		mr.SetReceiveBuffer(p1.DataBuffer.data(), p1.DataBuffer.size(), ep);
		//Message incomplete, no message in queue
		mr.AssertInMessagesSize(0);
		//Map should have the one incomplete message, waiting for more packets
		mr.AssertMapSize(1);
		std::this_thread::sleep_for(std::chrono::milliseconds(200)); //Wait over threhsold limit, packets should be nuked (message in console will appear)
		//Need to trigger another receive
		mr.SetReceiveBuffer(p3.DataBuffer.data(), p3.DataBuffer.size(), ep);

		//Map will still be 1, because of the new unrelated packet we sent
		mr.AssertMapSize(1);
		mr.AssertHasPendingPacket(0); //Shouldn't have any packet of ID 0, as it was removed
	}

	//Out of order packets
	{
		UDPPacketAssembler<MessageType> assembler;
		std::stringstream ss;
		for (int i = 0; i < 20000; i++)
		{
			ss << (char)i;
		}

		NetMessage<MessageType> msg;
		msg.SetMessageID(MessageType::Text);
		msg.SetPayload(ss.str().data(), ss.str().size());
		auto packets = assembler.CreatePackets(msg);

		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::shuffle(packets.begin(), packets.end(), std::default_random_engine(seed));

		for (auto& p : packets)
			mr.SetReceiveBuffer(p.DataBuffer.data(), p.DataBuffer.size(), ep);

		mr.ExpectedMessage = ss.str();
		mr.Update(); //Will trigger OnMessage, more asserts there
	}
}