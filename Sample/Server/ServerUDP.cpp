#include "ServerUDP.h"

ServerUDP::ServerUDP() : UDPMessager()
{

}

void ServerUDP::OnDisconnection(const std::string& addressPort)
{
}

bool ServerUDP::OnIOError(std::error_code ec)
{
	return true;
}

void ServerUDP::OnMessage(OwnedUDPMessage<MessageType> msg) 
{

}

void ServerUDP::Tick()
{
	Update(true);
}
