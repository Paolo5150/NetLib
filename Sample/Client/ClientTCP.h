#pragma once
#include <iostream>
#include <NetLib/NetMessage.h>
#include <NetLib/TCPClient.h>
#include <NetLib/UDPMessager.h>
#include "../Common.h"

class ClientTCP : public TCPClient<MessageType>
{
public:
	ClientTCP();

	void OnConnectionSuccessful() override;

	void OnConnectionFail() override;

	void OnKeyPressed(int n);

	void Tick();
};