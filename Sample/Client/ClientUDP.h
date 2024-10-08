#pragma once
#include <NetLib/UDPMessager.h>
#include "../Common.h"

class ClientUDP : public UDPMessager<MessageType>
{
public:
	ClientUDP();

	void OnDisconnection(const std::string& addressPort) override;

	bool OnIOError(std::error_code ec) override;
	void OnMessage(OwnedUDPMessage<MessageType> msg) override;

	void OnKeyPressed(int n);

	void Tick();
private:
	flatbuffers::FlatBufferBuilder m_fbBuilder;

};