#pragma once
#include <vector>
#include <iostream>
#include <asio.hpp>

template<typename T>
struct TCPMessageHeader
{
	T ID{};
	uint32_t Size = 0;
};

template<typename T>
struct TCPMessage
{
	TCPMessageHeader<T> Header;
	std::vector<uint8_t> Body;

	size_t Size() const
	{
		return sizeof(TCPMessageHeader<T>) + Body.size();
	}

	friend std::ostream& operator << (std::ostream& stream, const TCPMessage<T>& msg)
	{
		stream << "ID: " << static_cast<int>(msg.Header.ID) << " Size: " << msg.Header.Size;
		return stream;
	}

	void SetData(void* data, size_t dataSize)
	{
		Body.resize(dataSize);
		std::memcpy(Body.data(), data, dataSize);
		Header.Size = Size();
	}

	template<typename DT>
	friend TCPMessage<T>& operator << (TCPMessage<T>& msg, const DT& data)
	{
		static_assert(std::is_standard_layout<DT>::value, "Data too complex");
		size_t i = msg.Body.size();
		msg.Body.resize(msg.Body.size() + sizeof(DT));
		std::memcpy(msg.Body.data() + i, &data, sizeof(DT));
		msg.Header.Size = msg.Size();
		return msg;
	}

	template<typename DT>
	friend TCPMessage<T>& operator >> (TCPMessage<T>& msg, DT& data)
	{
		static_assert(std::is_standard_layout<DT>::value, "Data too complex");
		size_t i = msg.Body.size() - sizeof(DT);
		std::memcpy(&data, msg.Body.data() + i, sizeof(DT));
		msg.Body.resize(i);
		msg.Header.Size = msg.Size();
		return msg;
	}
};

template<typename T>
struct TCPConnection;

template<typename T>
struct OwnedTCPMessage
{
	std::shared_ptr<TCPConnection<T>> Remote = nullptr;
	TCPMessage<T> TheMessage;

	friend std::ostream& operator <<(std::ostream& os, const OwnedTCPMessage<T>& msg)
	{
		os << msg.TheMessage;
		return os;
	}
};