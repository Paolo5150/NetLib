#pragma once
#include <NetLib/UDPMessager.h>
#include "../Common.h"

class ClientUDP : public UDPMessager<MessageType>
{
public:
	ClientUDP();

	void SendData(MessageType id, uint8_t* data, uint32_t size, const std::string& sendToAddress, uint32_t port);

	void OnDisconnection(const std::string& addressPort) override;

	bool OnIOError(std::error_code ec) override;
	void OnMessage(OwnedUDPMessage<MessageType> msg) override;

	void OnKeyPressed(int n);

	void Tick();


};