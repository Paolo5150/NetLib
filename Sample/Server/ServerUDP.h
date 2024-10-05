#pragma once
#include <NetLib/UDPMessager.h>
#include "../Common.h"

class ServerUDP : public UDPMessager<MessageType>
{
public:
	ServerUDP();

	void OnDisconnection(const std::string& addressPort) override;

	bool OnIOError(std::error_code ec) override;

	void OnMessage(OwnedUDPMessage<MessageType> msg) override;

	void Tick();
};

