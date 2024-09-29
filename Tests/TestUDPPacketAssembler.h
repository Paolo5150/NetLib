#pragma once
#include "NetLib/UDPPacketAssembler.h"
#include "Common.h"
#include <string>

//Text packets
void TestUDPPacketAssembler()
{
	UDPPacketAssembler<MessageType> assembler;
	auto maxSize = UDPPacket<MessageType>::GetMaxPayloadSize();

	std::stringstream ss;
	for (int i = 0; i < maxSize; i++)
	{
		ss << "a";
	}

	ss << "b";

	NetMessage<MessageType> msg;
	msg.SetPayload(ss.str().data(), ss.str().length());
	msg.SetMessageID(MessageType::Text);

	auto packets = assembler.CreatePackets(msg);

	assert(packets.size() == 2);

	{
		auto h = packets[0].ExtractHeader();
		assert(h.PacketID == 0);
		assert(h.MessageID == MessageType::Text);
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
	auto message = assembler.AssemblePayloadFromPackets(packets);
	std::string toStr;
	toStr.resize(message.size());
	std::memcpy(toStr.data(), message.data(), message.size());

	auto orig = ss.str();
	assert(toStr == orig);

}

//Data packets
void TestUDPPacketAssembler2()
{
	UDPPacketAssembler<MessageType> assembler;
	auto maxSize = UDPPacket<MessageType>::GetMaxPayloadSize();

	struct TestData
	{
		float x;
		float y;
		float z;

	};

	TestData td = { 23.45f, 22.532f, 11.90f };

	NetMessage<MessageType> msg;
	msg.SetPayload(&td, sizeof(td));
	msg.SetMessageID(MessageType::Data);

	auto packets = assembler.CreatePackets(msg);

	assert(packets.size() == 1);

	{
		auto h = packets[0].ExtractHeader();
		assert(h.PacketID == 0);
		assert(h.MessageID == MessageType::Data);
		assert(h.PacketMaxSequenceNumbers == 1);
		assert(h.PacketSequenceNumber == 0);
	}


	//Recreate message
	auto message = assembler.AssemblePayloadFromPackets(packets);
	TestData retrievedData;
	std::memcpy(&retrievedData, message.data(), message.size());

	assert(td.x == retrievedData.x);
	assert(td.y == retrievedData.y);
	assert(td.z == retrievedData.z);

}

//Larger data 
void TestUDPPacketAssembler3()
{
	UDPPacketAssembler<MessageType> assembler;
	auto maxSize = UDPPacket<MessageType>::GetMaxPayloadSize();

	std::array<int, 100000> origData;
	for (auto i = 0; i < origData.size(); i++)
		origData[i] = i * 2;


	NetMessage<MessageType> msg;
	msg.SetPayload(&origData, sizeof(int) * origData.size());
	msg.SetMessageID(MessageType::Data);

	auto packets = assembler.CreatePackets(msg);

	assert(packets.size() == 274); //((100000 * 4) / 268) + 1

	uint16_t maxBodySize = UDPPacket<MessageType>::GetMaxPayloadSize();

	uint16_t totalSize = sizeof(int) * origData.size();
	for (auto i = 0; i < packets.size(); i++)
	{
		auto h = packets[i].ExtractHeader();
		assert(h.PacketID == 0);
		assert(h.MessageID == MessageType::Data);
		assert(h.PacketMaxSequenceNumbers == 274);
		assert(h.PacketSequenceNumber == i);

		auto pl = packets[i].ExtractPayload();

		//All packets should have the payload full, except for the last one, which will have the remaining bytes
		if (i < packets.size() - 1)
			assert(pl.size() == maxBodySize);
		else
			assert(pl.size() == totalSize);

		totalSize -= pl.size();
	}

	assert(totalSize == 0);

	//Rebuild data
	auto message = assembler.AssemblePayloadFromPackets(packets);
	std::array<int, 100000> gotData;

	std::memcpy(&gotData, message.data(), message.size());

	for (auto i = 0; i < gotData.size(); i++)
		assert(origData[i] == i * 2);
}