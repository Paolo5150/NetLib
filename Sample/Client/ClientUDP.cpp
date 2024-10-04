#include "ClientUDP.h"

ClientUDP::ClientUDP() : UDPMessager()
{
}

void ClientUDP::SendData(MessageType id, uint8_t* data, uint32_t size, const std::string& sendToAddress, uint32_t port)
{

}

void ClientUDP::OnDisconnection(const std::string& addressPort)
{
}

bool ClientUDP::OnIOError(std::error_code ec)
{
	return true;

}
void ClientUDP::OnMessage(OwnedUDPMessage<MessageType> msg) 
{
}

void ClientUDP::OnKeyPressed(int n)
{

}

void ClientUDP::Tick()
{
	Update(true);
}

