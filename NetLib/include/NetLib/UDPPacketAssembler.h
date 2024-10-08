#pragma once
#include "NetMessage.h"
#include "UDPPacket.h"

template <class T>
class UDPPacketAssembler
{
public:
	std::vector<UDPPacket<T>> CreatePackets(const NetMessage<T>& msg)
	{
		return CreatePackets(msg.GetMessageID(), (uint8_t*)msg.GetPayload().data(), msg.GetPayloadSize());
	}

	std::vector<UDPPacket<T>> CreatePackets(T id, uint8_t* data, uint32_t size)
	{
		std::vector<UDPPacket<T>> packets;

		uint32_t startSize = size;
		auto maxBodySize = UDPPacket<T>::GetMaxPayloadSize();
		uint32_t numOfPackets = (size / maxBodySize) + (size % maxBodySize != 0 ? 1 : 0);
		//Store the numOfPackets in a larger container (32bit) so it doesn't overflow, but assert that the max number is within limit
		assert(numOfPackets < UINT16_MAX);
		uint8_t* dataStartPointer = data;

		for (uint16_t i = 0; i < numOfPackets; i++)
		{
			UDPPacket<T> packet;

			int div = startSize / maxBodySize;
			uint32_t payloadSize = 0;

			if (div > 0)
			{
				payloadSize = maxBodySize;
				startSize -= maxBodySize;

			}
			else
			{
				payloadSize = startSize;
			}

			packet.SetHeader(id, m_packetIDCounter, i, numOfPackets);
			packet.SetPayload(dataStartPointer, payloadSize);

			//Move data pointer
			dataStartPointer += payloadSize;
			packets.emplace_back(std::move(packet));
		}

		m_packetIDCounter = (m_packetIDCounter + 1) % UINT16_MAX;
		return packets;
	}

	/**
	* Recreates original payload from packets (no header).
	* Requires packets to be in correct order
	*/
	std::vector<uint8_t> AssemblePayloadFromPackets(const std::vector<UDPPacket<T>>& packets)
	{
		std::vector<uint8_t> message;
		uint32_t actualSize = 0; //Calculate while assembling the message
		//Assume max size
		auto maxSize = packets.size() * UDPPacket<T>::GetMaxPayloadSize();
		message.resize(maxSize);
		uint8_t* dataStartPoint = message.data();
		for (size_t i = 0; i < packets.size(); i++)
		{
			std::memcpy(dataStartPoint, packets[i].DataBuffer.data() + sizeof(UDPPacketHeader<T>), (packets[i].DataBuffer.size() - sizeof(UDPPacketHeader<T>)));
			dataStartPoint += (packets[i].DataBuffer.size() - sizeof(UDPPacketHeader<T>));
			actualSize += ((uint32_t)packets[i].DataBuffer.size() - sizeof(UDPPacketHeader<T>));
		}

		message.resize(actualSize);
		return message;

	}

	void AssemblePayloadFromPackets(const std::vector<UDPPacket<T>>& packets, std::vector<uint8_t>& outPayload)
	{
		uint32_t actualSize = 0; //Calculate while assembling the message
		//Assume max size
		auto maxSize = packets.size() * UDPPacket<T>::GetMaxPayloadSize();
		outPayload.resize(maxSize);
		uint8_t* dataStartPoint = outPayload.data();
		for (size_t i = 0; i < packets.size(); i++)
		{
			std::memcpy(dataStartPoint, packets[i].DataBuffer.data() + sizeof(UDPPacketHeader<T>), (packets[i].DataBuffer.size() - sizeof(UDPPacketHeader<T>)));
			dataStartPoint += ((uint32_t)packets[i].DataBuffer.size() - sizeof(UDPPacketHeader<T>));
			actualSize += ((uint32_t)packets[i].DataBuffer.size() - sizeof(UDPPacketHeader<T>));
		}

		outPayload.resize(actualSize);

	}



private:
	uint16_t m_packetIDCounter = 0;

};