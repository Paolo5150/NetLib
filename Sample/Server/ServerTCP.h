#pragma once
#include "../Common.h"
#include <NetLib/NetMessage.h>
#include <NetLib/TCPServer.h>
#include <unordered_map>
class ServerTCP : public TCPServer<MessageType>
{
public:
	ServerTCP(uint16_t port);

	bool OnClientConnection(std::shared_ptr <TCPConnection<MessageType>> client, uint32_t assignedID) override;

	void OnClientDisconnection(std::shared_ptr < TCPConnection<MessageType>> client) override;

	bool OnIOError(std::shared_ptr<TCPConnection<MessageType>> client, std::error_code ec) override;

	void OnMessage(std::shared_ptr<TCPConnection<MessageType>> client, const NetMessage<MessageType>& msg);

	void Tick();

	void OnKeyPressed(int k);
private:
	std::unordered_map<uint32_t, std::shared_ptr <TCPConnection<MessageType>>> m_clientMap;
};