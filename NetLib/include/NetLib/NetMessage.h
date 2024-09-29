#pragma once
#include <vector>
#include <iostream>
#include <asio.hpp>

template<typename T>
class TCPConnection;

template<typename T>
struct NetMessageHeader
{
	T ID{};
	uint32_t Size = 0;
};

template<typename T>
class NetMessage
{
public:
	size_t Size() const
	{
		return sizeof(NetMessageHeader<T>) + Payload.size();
	}

	T GetMessageID() const
	{
		return Header.ID;
	}

	void SetMessageID(T id)
	{
		Header.ID = id;
	}

	size_t GetPayloadSize() const
	{
		return Payload.size();
	}

	const std::vector<uint8_t>& GetPayload() const
	{
		return Payload;
	}

	std::vector<uint8_t>& GetPayload()
	{
		return Payload;
	}

	friend std::ostream& operator << (std::ostream& stream, const NetMessage<T>& msg)
	{
		stream << "ID: " << static_cast<int>(msg.Header.ID) << " Size: " << msg.Header.Size;
		return stream;
	}

	void SetPayload(void* data, size_t dataSize)
	{
		Payload.resize(dataSize);
		std::memcpy(Payload.data(), data, dataSize);
		Header.Size = Size();
	}

	//template<typename DT>
	//friend NetMessage<T>& operator << (NetMessage<T>& msg, const DT& data)
	//{
	//	static_assert(std::is_standard_layout<DT>::value, "Data too complex");
	//	size_t i = msg.Payload.size();
	//	msg.Payload.resize(msg.Payload.size() + sizeof(DT));
	//	std::memcpy(msg.Payload.data() + i, &data, sizeof(DT));
	//	msg.Header.Size = msg.Size();
	//	return msg;
	//}
	//
	//template<typename DT>
	//friend NetMessage<T>& operator >> (NetMessage<T>& msg, DT& data)
	//{
	//	static_assert(std::is_standard_layout<DT>::value, "Data too complex");
	//	size_t i = msg.Payload.size() - sizeof(DT);
	//	std::memcpy(&data, msg.Payload.data() + i, sizeof(DT));
	//	msg.Payload.resize(i);
	//	msg.Header.Size = msg.Size();
	//	return msg;
	//}
private:
	std::vector<uint8_t> Payload;
	friend class TCPConnection<T>;
	NetMessageHeader<T> Header;
};

template<typename T>
struct TCPServerClientConnection;

template<typename T>
struct OwnedTCPMessage
{
	std::shared_ptr<TCPServerClientConnection<T>> Remote = nullptr;
	NetMessage<T> TheMessage;

	friend std::ostream& operator <<(std::ostream& os, const OwnedTCPMessage<T>& msg)
	{
		os << msg.TheMessage;
		return os;
	}
};

template<typename T>
struct OwnedUDPMessage
{
	std::string RemoteAddress;
	uint32_t RemotePort;
	NetMessage<T> TheMessage;

	friend std::ostream& operator <<(std::ostream& os, const OwnedUDPMessage<T>& msg)
	{
		os << msg.TheMessage < ". From " << RemoteAddress << " " << RemotePort;
		return os;
	}
};