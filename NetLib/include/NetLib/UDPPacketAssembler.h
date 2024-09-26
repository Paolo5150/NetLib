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
		auto maxBodySize = UDPPacket<T>::GetMaxBodySize();
		uint16_t numOfPackets = (size / maxBodySize) + (size % maxBodySize != 0 ? 1 : 0);
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
	* Recreates original message from packets.
	* Requires packets to be in correct order
	*/
	std::vector<uint8_t> AssembleMessageFromPackets(const std::vector<UDPPacket<T>>& packets)
	{
		std::vector<uint8_t> message;
		uint32_t actualSize = 0; //Calculate while assembling the message
		//Assume max size
		auto maxSize = packets.size() * UDPPacket<T>::GetMaxBodySize();
		message.resize(maxSize);
		uint8_t* dataStartPoint = message.data();
		for (size_t i = 0; i < packets.size(); i++)
		{
			std::memcpy(dataStartPoint, packets[i].DataBuffer.data() + sizeof(UDPPacketHeader<T>), (packets[i].DataBuffer.size() - sizeof(UDPPacketHeader<T>)));
			dataStartPoint += (packets[i].DataBuffer.size() - sizeof(UDPPacketHeader<T>));
			actualSize += (packets[i].DataBuffer.size() - sizeof(UDPPacketHeader<T>));
		}

		message.resize(actualSize);
		return message;

	}
private:
	uint16_t m_packetIDCounter = 0;

};