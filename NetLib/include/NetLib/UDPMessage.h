#pragma once
#include <vector>
#include <iostream>
#include <asio.hpp>
#include "UDPHeader.h"

template<typename T>
struct UDPMessageHeader
{
	T MessageID{};
	uint32_t Size = 0;
};

template<typename T>
struct UDPMessage
{
	UDPMessageHeader<T> Header;
	std::vector<uint8_t> Body;

	size_t Size() const
	{
		return sizeof(UDPMessageHeader<T>) + Body.size();
	}

	friend std::ostream& operator << (std::ostream& stream, const UDPMessage<T>& msg)
	{
		stream << "ID: " << static_cast<int>(msg.Header.ID) << " Size: " << msg.Header.Size;
		return stream;
	}

	void SetData(uint8_t* data, size_t dataSize)
	{
		Body.resize(dataSize);
		std::memcpy(Body.data(), data, dataSize);
		Header.Size = Size();
	}

	template<typename DT>
	friend UDPMessage<T>& operator << (UDPMessage<T>& msg, const DT& data)
	{
		static_assert(std::is_standard_layout<DT>::value, "Data too complex");
		size_t i = msg.Body.size();
		msg.Body.resize(msg.Body.size() + sizeof(DT));
		std::memcpy(msg.Body.data() + i, &data, sizeof(DT));
		msg.Header.Size = msg.Size();
		return msg;
	}

	template<typename DT>
	friend UDPMessage<T>& operator >> (UDPMessage<T>& msg, DT& data)
	{
		static_assert(std::is_standard_layout<DT>::value, "Data too complex");
		size_t i = msg.Body.size() - sizeof(DT);
		std::memcpy(&data, msg.Body.data() + i, sizeof(DT));
		msg.Body.resize(i);
		msg.Header.Size = msg.Size();
		return msg;
	}

};

