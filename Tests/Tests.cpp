#include "NetLib/UDPPacketAssembler.h"
#include <string>

enum class MessageType : uint32_t
{
	Ping,
	Text
};


void TestUDPPacket()
{
	UDPPacket<MessageType> packet;
	std::string msg = "This should be size of 25";

	{
		try
		{
			packet.SetPayload(nullptr, 0); //Should throw, as header was not set
			assert(false);
		}
		catch (std::exception& e)
		{
			assert(true);
		}

		packet.SetHeader(MessageType::Text, 0, 0, 2);

		auto newH = packet.ExtractHeader();

		assert(newH.PacketSequenceNumber == 0);
		assert(newH.PacketMaxSequenceNumbers == 2);

		packet.SetPayload((uint8_t*)msg.data(), sizeof(char) * msg.size());

		assert(packet.PacketSize == (sizeof(UDPPacketHeader<MessageType>) + (sizeof(char) * msg.size())));

		std::string retrieved;
		retrieved.resize(packet.PacketSize - sizeof(UDPPacketHeader<MessageType>));
		auto pl = packet.ExtractPayload();
		memcpy(retrieved.data(), pl.data(), pl.size());

		assert(retrieved == msg);
	}
	

	//Packet from full buffer
	UDPPacket<MessageType> packet2(packet.DataBuffer);
	{
		auto newH = packet2.ExtractHeader();

		assert(newH.PacketSequenceNumber == 0);
		assert(newH.PacketMaxSequenceNumbers == 2);

		assert(packet2.PacketSize == (sizeof(UDPPacketHeader<MessageType>) + (sizeof(char) * msg.size())));

		std::string retrieved;
		retrieved.resize(packet2.PacketSize - sizeof(UDPPacketHeader<MessageType>));
		auto pl = packet2.ExtractPayload();
		memcpy(retrieved.data(), pl.data(), pl.size());

		assert(retrieved == msg);
	}

}

void TestUDPPacketAssembler()
{
	UDPPacketAssembler<MessageType> assembler;
	auto maxSize = UDPPacket<MessageType>::GetMaxBodySize();

	std::stringstream ss;
	for (int i = 0; i < maxSize ; i++)
	{
		ss << "a";
	}

	ss << "b";

	auto packets = assembler.CreatePackets(MessageType::Text, (uint8_t*)ss.str().data(), ss.str().length());

	assert(packets.size() == 2);

	{
		auto h = packets[0].ExtractHeader();
		assert(h.PacketID == 0);
		assert(h.PacketMaxSequenceNumbers == 2);
		assert(h.PacketSequenceNumber == 0);
	}

	{
		auto h = packets[1].ExtractHeader();
		assert(h.MessageID == MessageType::Text);
		assert(h.PacketID == 0);
		assert(h.PacketMaxSequenceNumbers == 2);
		assert(h.PacketSequenceNumber == 1);
	}

	//Recreate message
	auto message = assembler.AssembleMessageFromPackets(packets);
	std::string toStr;
	toStr.resize(message.size());
	std::memcpy(toStr.data(), message.data(), message.size());

	auto orig = ss.str();
	assert(toStr == orig);

}

void main()
{
	TestUDPPacket();
	TestUDPPacketAssembler();
	std::cout << "All tests passed";
}