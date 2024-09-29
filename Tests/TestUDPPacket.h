#pragma once
#include "NetLib/UDPPacketAssembler.h"
#include "Common.h"

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