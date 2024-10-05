#pragma once
#include "NetLib/UDPMessager.h"
#include "NetLib/UDPPacketAssembler.h"
#include "Common.h"
#include <string>
#include <random>

class MockUDPMessager : public UDPMessager<MessageType>
{
public:
	MockUDPMessager() : UDPMessager()
	{
	}

	void OnMessage(OwnedUDPMessage<MessageType> msg) override
	{
		std::string s;
		s.resize(msg.TheMessage.GetPayloadSize());

		assert(msg.RemoteAddress == ExpectedSenderID);
		assert(msg.RemotePort == ExpectedPort);

		//Retrieve string sent
		std::memcpy(s.data(), msg.TheMessage.GetPayload().data(), msg.TheMessage.GetPayloadSize());
		assert(s == ExpectedMessage);

	}

	void OnDisconnection(const std::string& addressPort)
	{
		assert(true);
	}

	void SetReceiveBuffer(uint8_t* data, uint32_t size, asio::ip::udp::endpoint& endp)
	{
		std::memcpy(m_receiveBuffer, data, size);
		CheckInactiveEndpoints();
		CheckIncompleteMessages();
		ProcessPacket(endp, size);
	}

	bool OnIOError(std::error_code ec)
	{
		return true;
	}

	void AssertMapSize(int size, asio::ip::udp::endpoint& endp)
	{
		std::string senderKey = endp.address().to_string() + ":" + std::to_string(endp.port());
		assert(m_packetMap[senderKey].size() == size);
	}

	void AssertInMessagesSize(int size)
	{
		assert(GetAvailableMessagesCount() == size);
	}

	void AssertHasPendingPacket(uint16_t id, asio::ip::udp::endpoint& endp)
	{
		assert(!HasPendingPacketOfId(id, endp));
	}

	std::string ExpectedMessage;
	std::string ExpectedSenderID;
	uint32_t ExpectedPort;
};

void TestUDPMessager()
{
	{
		MockUDPMessager mr;
		mr.StartListening(90000);
		asio::ip::udp::endpoint ep(asio::ip::address::from_string("127.0.0.1"), 12345);
		mr.ExpectedSenderID = "127.0.0.1";
		mr.ExpectedPort = 12345;
		UDPPacket<MessageType> p1;
		p1.SetHeader(MessageType::Text, 0, 0, 1);
	
		std::string message = "Hello";
		p1.SetPayload((uint8_t*)message.data(), message.size());
	
		//This emulates the Receive function, when data comes in
		mr.SetReceiveBuffer(p1.DataBuffer.data(), p1.DataBuffer.size(), ep);
		//Single packet, message should have been processed
		mr.AssertInMessagesSize(1);
		//Map should be clear, no other expected packets
		mr.AssertMapSize(0, ep);
		mr.ExpectedMessage = message;
		mr.Update(); //Will trigger OnMessage, more asserts there
	}
	
	//Dropped packets
	{
		MockUDPMessager mr;
		mr.StartListening(90000);

		asio::ip::udp::endpoint ep(asio::ip::address::from_string("127.0.0.1"), 12345);
		mr.ExpectedSenderID = "127.0.0.1";
		mr.ExpectedPort = 12345;
	
	
		//Two packets, but send one, verify incomplete messages are nuked
		UDPPacket<MessageType> p1;
		p1.SetHeader(MessageType::Text, 0, 0, 2);
	
		UDPPacket<MessageType> p2;
		p2.SetHeader(MessageType::Text, 0, 1, 2);
	
		//Unrelated packet
		UDPPacket<MessageType> p3;
		p3.SetHeader(MessageType::Text, 1, 0, 2);
	
		//Send 1st packet only
		mr.SetReceiveBuffer(p1.DataBuffer.data(), (uint32_t)p1.DataBuffer.size(), ep);
		//Message incomplete, no message in queue
		mr.AssertInMessagesSize(0);
		//Map should have the one incomplete message, waiting for more packets
		mr.AssertMapSize(1, ep);
		std::this_thread::sleep_for(std::chrono::milliseconds(600)); //Wait over threhsold limit, packets should be nuked (message in console will appear)
		//Need to trigger another receive
		mr.SetReceiveBuffer(p3.DataBuffer.data(), (uint32_t)p3.DataBuffer.size(), ep);
	
	
		//Map will still be 1, because of the new unrelated packet we sent
		mr.AssertMapSize(1, ep);
		mr.AssertHasPendingPacket(0, ep); //Shouldn't have any packet of ID 0, as it was removed
	}
	
	//Out of order packets
	{
		MockUDPMessager mr;
		mr.StartListening(90000);
		asio::ip::udp::endpoint ep(asio::ip::address::from_string("127.0.0.1"), 12345);
		mr.ExpectedSenderID = "127.0.0.1";
		mr.ExpectedPort = 12345;
	
		UDPPacketAssembler<MessageType> assembler;
		std::stringstream ss;
		for (int i = 0; i < 20000; i++)
		{
			ss << (char)i;
		}
	
		NetMessage<MessageType> msg;
		msg.SetMessageID(MessageType::Text);
		msg.SetPayload(ss.str().data(), (uint32_t)ss.str().size());
	
		auto packets = assembler.CreatePackets(msg);
	
		//Fill with random duplicates
		packets.push_back(packets[0]);
		packets.push_back(packets[3]);
		packets.push_back(packets[2]);
		packets.push_back(packets[2]);
		packets.push_back(packets[1]);
		packets.push_back(packets[6]);
	
		auto seed = static_cast<uint16_t>(std::chrono::system_clock::now().time_since_epoch().count());
		std::shuffle(packets.begin(), packets.end(), std::default_random_engine(seed));
	
		for (auto& p : packets)
			mr.SetReceiveBuffer(p.DataBuffer.data(), (uint32_t)p.DataBuffer.size(), ep);
	
		mr.ExpectedMessage = ss.str();
		mr.Update(); //Will trigger OnMessage, more asserts there
	}
	
	{
		MockUDPMessager mr;
		mr.StartListening(90000);
		asio::ip::udp::endpoint ep1(asio::ip::address::from_string("192.168.1.1"), 12345);
		asio::ip::udp::endpoint ep2(asio::ip::address::from_string("10.0.0.1"), 12346);
		asio::ip::udp::endpoint ep3(asio::ip::address::from_string("172.16.0.1"), 12347);
		
		UDPPacket<MessageType> p1, p2, p3;
		p1.SetHeader(MessageType::Text, 0, 0, 1);
		p2.SetHeader(MessageType::Text, 1, 0, 1);
		p3.SetHeader(MessageType::Text, 2, 0, 1);
		
		std::string message1 = "Hello from sender 1";
		std::string message2 = "Greetings from sender 2";
		std::string message3 = "Hi from sender 3";
		
		p1.SetPayload((uint8_t*)message1.data(),(uint32_t) message1.size());
		p2.SetPayload((uint8_t*)message2.data(),(uint32_t) message2.size());
		p3.SetPayload((uint8_t*)message3.data(),(uint32_t) message3.size());
		
		mr.SetReceiveBuffer(p1.DataBuffer.data(), (uint32_t)p1.DataBuffer.size(), ep1);
		mr.SetReceiveBuffer(p2.DataBuffer.data(), (uint32_t)p2.DataBuffer.size(), ep2);
		mr.SetReceiveBuffer(p3.DataBuffer.data(), (uint32_t)p3.DataBuffer.size(), ep3);
		
		mr.AssertInMessagesSize(3);
		mr.AssertMapSize(0, ep1);
		
		// Verify messages from different senders
		mr.ExpectedMessage = message1;
		mr.ExpectedSenderID = "192.168.1.1";
		mr.ExpectedPort = 12345;
	
		mr.Update(false, 1);
		mr.ExpectedMessage = message2;
		mr.ExpectedSenderID = "10.0.0.1";
		mr.ExpectedPort = 12346;
	
		mr.Update(false, 1);
		mr.ExpectedMessage = message3;
		mr.ExpectedSenderID = "172.16.0.1";
		mr.ExpectedPort = 12347;
	
		mr.Update(false, 1);
	
		asio::ip::udp::endpoint ep4(asio::ip::address::from_string("122.16.0.1"), 12347);
	
		//test saved endpoints
		assert(mr.HasEndpoint(ep1));
		assert(mr.HasEndpoint(ep2));
		assert(mr.HasEndpoint(ep3));
		assert(!mr.HasEndpoint(ep4));
	
		mr.DisconnectEndpoint(ep1);
		assert(!mr.HasEndpoint(ep1));
	
	}

	{
		MockUDPMessager mr;
		mr.StartListening(90000);
		asio::ip::udp::endpoint ep1(asio::ip::address::from_string("192.168.1.1"), 12345);
		asio::ip::udp::endpoint ep2(asio::ip::address::from_string("10.0.0.1"), 12346);
		asio::ip::udp::endpoint ep3(asio::ip::address::from_string("172.16.0.1"), 12347);
		mr.SetDisconnectEndpointThreshold(5000);
		// Create packets for different senders
		UDPPacket<MessageType> p1, p2, p3;

		// Sender 1: Message split across multiple packets
		std::string message1 = "Hello from sender 1. This is a longer message to test multiple packet assembly.";
		size_t payloadSize = UDPPacket<MessageType>::GetMaxPayloadSize();

		// Fragment the message if necessary
		for (size_t i = 0; i < message1.size(); i += payloadSize)
		{
			size_t chunkSize = std::min(payloadSize, message1.size() - i);
			std::string chunk = message1.substr(i, chunkSize);

			// Create a packet for each fragment
			p1.SetHeader(MessageType::Text, (uint16_t)0, (uint16_t)(i / payloadSize), (uint16_t)((message1.size() + payloadSize - 1) / payloadSize));
			p1.SetPayload((uint8_t*)chunk.data(), (uint32_t)chunk.size());

			// Send each packet
			mr.SetReceiveBuffer(p1.DataBuffer.data(), (uint32_t)p1.DataBuffer.size(), ep1);
		}

		// Sender 2: Single packet
		std::string message2 = "Greetings from sender 2";
		p2.SetHeader(MessageType::Text, 1, 0, 1); // Single packet message
		p2.SetPayload((uint8_t*)message2.data(), (uint32_t)message2.size());
		mr.SetReceiveBuffer(p2.DataBuffer.data(), (uint32_t)p2.DataBuffer.size(), ep2);

		// Sender 3: Single packet
		std::string message3 = "Hi from sender 3";
		p3.SetHeader(MessageType::Text, 2, 0, 1); // Single packet message
		p3.SetPayload((uint8_t*)message3.data(), (uint32_t)message3.size());
		mr.SetReceiveBuffer(p3.DataBuffer.data(), (uint32_t)p3.DataBuffer.size(), ep3);

		// Validate the received messages
		mr.AssertInMessagesSize(3);  // Expecting three complete messages
		mr.AssertMapSize(0, ep1);    // Map should be clear (all fragments assembled)

		// Verify messages from different senders
		mr.ExpectedMessage = message1;
		mr.ExpectedSenderID = "192.168.1.1";
		mr.ExpectedPort = 12345;
		mr.Update(true, 1);  // Process message from sender 1

		mr.ExpectedMessage = message2;
		mr.ExpectedSenderID = "10.0.0.1";
		mr.ExpectedPort = 12346;
		mr.Update(true, 1);  // Process message from sender 2

		mr.ExpectedMessage = message3;
		mr.ExpectedSenderID = "172.16.0.1";
		mr.ExpectedPort = 12347;
		mr.Update(true, 1);  // Process message from sender 3

		//Test automatic disconnection
		//Send from ep1, but not ep1 and ep3, which should be removed after time threshold
		mr.SetReceiveBuffer(p2.DataBuffer.data(), (uint32_t)p2.DataBuffer.size(), ep1);
		mr.SetReceiveBuffer(p2.DataBuffer.data(), (uint32_t)p2.DataBuffer.size(), ep2);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		mr.SetReceiveBuffer(p2.DataBuffer.data(), (uint32_t)p2.DataBuffer.size(), ep1);
		mr.SetReceiveBuffer(p2.DataBuffer.data(), (uint32_t)p2.DataBuffer.size(), ep2);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		mr.SetReceiveBuffer(p2.DataBuffer.data(), (uint32_t)p2.DataBuffer.size(), ep1);
		mr.SetReceiveBuffer(p2.DataBuffer.data(), (uint32_t)p2.DataBuffer.size(), ep2);
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		mr.SetReceiveBuffer(p2.DataBuffer.data(), (uint32_t)p2.DataBuffer.size(), ep1);
		mr.SetReceiveBuffer(p2.DataBuffer.data(), (uint32_t)p2.DataBuffer.size(), ep2);
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));
		mr.SetReceiveBuffer(p2.DataBuffer.data(), (uint32_t)p2.DataBuffer.size(), ep1);
		mr.SetReceiveBuffer(p2.DataBuffer.data(), (uint32_t)p2.DataBuffer.size(), ep2);
		mr.Update(true, 0); //Pass 0, so we don't actually receive a message callback, we only want to trigger the disconnection callback

		assert(mr.HasEndpoint(ep1));
		assert(mr.HasEndpoint(ep2));
		assert(!mr.HasEndpoint(ep3));

	}
}