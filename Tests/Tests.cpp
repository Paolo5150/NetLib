#include "NetLib/UDPPacket.h"
#include <string>

enum class MessageType : uint32_t
{
	Ping,
	Text
};


void TestUDPPacket()
{
	UDPPacket<MessageType> packet;

	try
	{
		packet.SetPayload(nullptr, 0); //Should throw, as header was not set
		assert(false);
	}
	catch (std::exception& e)
	{
		assert(true);
	}

	packet.SetHeader(MessageType::Ping, 0, 2);

	auto newH = packet.ExtractHeader();

	assert(newH.ID == MessageType::Ping);
	assert(newH.PacketSequenceNumber == 0);
	assert(newH.PacketMaxSequenceNumbers == 2);

	std::string msg = "This should be size of 25";
	packet.SetPayload(msg.data(), sizeof(char) * msg.size());

	assert(packet.PacketSize == (sizeof(UDPPacketHeader<MessageType>) + (sizeof(char) * msg.size())));

	std::string retrieved;
	retrieved.resize(packet.PacketSize - sizeof(UDPPacketHeader<MessageType>));
	auto pl = packet.ExtractPayload();
	memcpy(retrieved.data(), pl.data(), pl.size());

	assert(retrieved == msg);
}

void main()
{
	TestUDPPacket();
	std::cout << "All tests passed";
}