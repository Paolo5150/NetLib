#pragma once
#include <vector>
#include <iostream>
#include <asio.hpp>

template<typename T>
struct MessageHeader
{
	T ID{};
	uint32_t Size = 0;
};

template<typename T>
struct Message
{
	MessageHeader<T> Header;
	std::vector<uint8_t> Body;

	size_t Size() const
	{
		return sizeof(MessageHeader<T>) + Body.size();
	}

	friend std::ostream& operator << (std::ostream& stream, const Message<T>& msg)
	{
		stream << "ID: " << static_cast<int>(msg.Header.ID) << " Size: " << msg.Header.Size;
		return stream;
	}

	template<typename DT>
	friend Message<T>& operator << (Message<T>& msg, const DT& data)
	{
		static_assert(std::is_standard_layout<DT>::value, "Data too complex");
		size_t i = msg.Body.size();
		msg.Body.resize(msg.Body.size() + sizeof(DT));
		std::memcpy(msg.Body.data() + i, &data, sizeof(DT));
		msg.Header.Size = msg.Size();
		return msg;
	}

	template<typename DT>
	friend Message<T>& operator >> (Message<T>& msg, DT& data)
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
struct Connection;

template<typename T>
struct OwnedMessage
{
	std::shared_ptr<Connection<T>> Remote = nullptr;
	Message<T> TheMessage;

	friend std::ostream& operator <<(std::ostream& os, const OwnedMessage<T>& msg)
	{
		os << msg.TheMessage;
		return os;
	}
};