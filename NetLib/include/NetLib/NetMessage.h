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

	T GetMessageID() const
	{
		return Header.ID;
	}

	void SetMessageID(T id)
	{
		Header.ID = id;
	}

	uint32_t GetPayloadSize() const
	{
		return (uint32_t)Payload.size();
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

	void SetPayload(void* data, uint32_t dataSize)
	{
		Payload.resize(dataSize);
		std::memcpy(Payload.data(), data, dataSize);
		Header.Size = (uint32_t)Payload.size() + (uint32_t)sizeof(NetMessageHeader<T>);
	}

private:
	std::vector<uint8_t> Payload;
	friend class TCPConnection<T>;
	NetMessageHeader<T> Header;
};

template<typename T>
class TCPServerClientConnection;

template<typename T>
struct OwnedTCPMessage
{
	std::shared_ptr<TCPConnection<T>> Remote = nullptr;
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