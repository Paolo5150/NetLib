#pragma once
#include <vector>
#include <iostream>
#include <asio.hpp>

static constexpr uint32_t MTULimit = 1500;
static constexpr uint32_t UDPOverhead = 20 + 8; //20 bytes  ip header, 8 bytes udp header

template<typename T>
struct UDPPacketHeader
{
	/**
	* User defined ID
	*/
	T MessageID{};
	/**
	* Packet ID: all packets belonging to the same message have the same PacketID. Assigned by the UPDPacketAssembler
	*/
	uint16_t PacketID = 0; 
	/**
	* The packet index
	*/
	uint16_t PacketSequenceNumber = 0;
	/**
	* The maximum number of packets for the full message
	*/
	uint16_t PacketMaxSequenceNumbers = 0;
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
		PacketSize = (uint32_t)DataBuffer.size();
	}

	UDPPacket(uint8_t* fullPacketData, uint32_t size)
	{
		DataBuffer.resize(size);
		std::memcpy(DataBuffer.data(), fullPacketData, size);
		auto h = ExtractHeader();
		PacketSize = (uint32_t)DataBuffer.size();
	}

	UDPPacket()
	{
		DataBuffer.resize(MTULimit);
	}

	static uint32_t GetMaxPayloadSize()
	{
		return MTULimit - sizeof(UDPPacketHeader<T>) - UDPOverhead;
	}

	static uint32_t GetTotalHeaderSizeIncludingOverheads()
	{
		return sizeof(UDPPacketHeader<T>) + UDPOverhead;
	}

	uint32_t GetPayloadSize()
	{
		return DataBuffer.size() - sizeof(UDPPacketHeader<T>);
	}
	
	void SetHeader( T id, uint16_t packetID, uint16_t packetSequence, uint16_t packetMaxSequence)
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

	void SetPayload(uint8_t* data, uint32_t dataSize)
	{ 
		if (!m_headerSet)
			throw std::runtime_error("Header must be set first");

		if(dataSize > GetMaxPayloadSize())
			throw std::runtime_error("Packet body size exceeds the limit");


		assert(dataSize <= MTULimit - GetTotalHeaderSizeIncludingOverheads() && "Packet body size over the limit");

		PacketSize = dataSize + +sizeof(UDPPacketHeader<T>);

		DataBuffer.resize(PacketSize);

		std::memcpy(DataBuffer.data() + sizeof(UDPPacketHeader<T>), data, dataSize);
	}

};

