#pragma once
#include <vector>
#include <iostream>
#include <asio.hpp>

static constexpr uint32_t MTULimit = 1500;

template<typename T>
struct UDPPacketHeader
{
	T MessageID{};
	uint16_t PacketID = 0;
	uint8_t PacketSequenceNumber = 0;
	uint8_t PacketMaxSequenceNumbers = 0;
};

template<typename T>
struct UDPPacket
{
	std::vector<uint8_t> DataBuffer; //Header + Payload
	uint32_t PacketSize = 0;
	bool m_headerSet = false;

	UDPPacket(const std::vector<uint8_t>& fullPacketData)
	{
		DataBuffer = fullPacketData;
		auto h = ExtractHeader();
		PacketSize = DataBuffer.size();
	}

	UDPPacket()
	{
		DataBuffer.resize(MTULimit);
	}

	static size_t GetMaxBodySize()
	{
		return MTULimit - sizeof(UDPPacketHeader<T>);
	}

	size_t GetPayloadSize()
	{
		return DataBuffer.size() - sizeof(UDPPacketHeader<T>);
	}
	
	void SetHeader( T id, uint16_t packetID,  uint8_t packetSequence, uint8_t packetMaxSequence)
	{
		UDPPacketHeader<T> header;
		header.MessageID = id;
		header.PacketID = packetID;
		header.PacketSequenceNumber = packetSequence;
		header.PacketMaxSequenceNumbers = packetMaxSequence;
		std::memcpy(DataBuffer.data(), &header, sizeof(UDPPacketHeader<T>));
		m_headerSet = true;
	}

	UDPPacketHeader<T> ExtractHeader()
	{
		UDPPacketHeader<T> header;
		std::memcpy(&header, DataBuffer.data(), sizeof(UDPPacketHeader<T>));
		return header;
	}

	std::vector<uint8_t> ExtractPayload()
	{
		std::vector<uint8_t> pl;
		auto s = PacketSize - sizeof(UDPPacketHeader<T>);
		pl.resize(s);
		std::memcpy(pl.data(), DataBuffer.data() + sizeof(UDPPacketHeader<T>),s);
		return pl;
	}

	void SetPayload(uint8_t* data, size_t dataSize)
	{ 
		if (!m_headerSet)
			throw std::runtime_error("Header must be set first");

		assert(dataSize <= MTULimit - sizeof(UDPPacketHeader<T>), "Packet body size over the limit");

		PacketSize = dataSize + sizeof(UDPPacketHeader<T>);

		DataBuffer.resize(PacketSize);

		std::memcpy(DataBuffer.data() + sizeof(UDPPacketHeader<T>), data, dataSize);
	}

};

