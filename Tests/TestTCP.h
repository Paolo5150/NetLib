#pragma once
#include <iostream>
#include <NetLib/NetMessage.h>
#include <NetLib/TCPClient.h>
#include <NetLib/TCPServer.h>
#include "Common.h"


class MockTCPClient : public TCPClient<MessageType>
{
public:
	MockTCPClient()
	{
	}

	void OnConnectionSuccessful() override
	{
		ConnectionSuccessulCallback();
	}


	void OnConnectionFail() override
	{
		ConnectionFailCallback();
	}

	void Update()
	{
		CallbackToClient cc;
		if (GetLatestCallback(cc))
		{
			if (cc.CType == CallbackType::ConnectionFail)
			{
				if(ConnectionFailCallback)
					ConnectionFailCallback();
			}
			else if (cc.CType == CallbackType::ConnectionSuccess)
			{
				if(ConnectionSuccessulCallback)
					ConnectionSuccessulCallback();

			}
			else if (cc.CType == CallbackType::IOError)
			{
				if (IOErrorCallback)
					IOErrorCallback(cc.ErrorCode);
				Destroy();
			}
		}
	}

	std::function<void()> ConnectionSuccessulCallback;
	std::function<void()> ConnectionFailCallback;
	std::function<void(std::optional<std::error_code>)> IOErrorCallback;
};

class MockTCPServer : public TCPServer<MessageType>
{
public:
	MockTCPServer(uint32_t port) : TCPServer(port)
	{

	}

	bool OnClientConnection(std::weak_ptr <TCPConnection<MessageType>> client, uint32_t assignedID) override
	{
		return ClientConnectionCallback(client);
	}

	void OnClientDisconnection(std::weak_ptr < TCPConnection<MessageType>> client) override
	{
		ClientDisconnectionCallback(client);
	}

	bool OnIOError(std::weak_ptr<TCPConnection<MessageType>> client, std::error_code ec) override
	{
		return true;
	}

	void OnMessage(std::weak_ptr<TCPConnection<MessageType>> client, const NetMessage<MessageType>& msg)
	{}


	std::function<bool(std::weak_ptr < TCPConnection<MessageType>>)> ClientConnectionCallback;
	std::function<void(std::weak_ptr < TCPConnection<MessageType>>)> ClientDisconnectionCallback;
};

void SleepMills(uint64_t mills)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(mills));
}

void TestTCP()
{
	bool quitTime = false;
	MockTCPClient client;
	MockTCPServer server(60000);

	server.Start();

	//Refuse connection
	server.ClientConnectionCallback = [](std::weak_ptr < TCPConnection<MessageType>> client) -> bool {
		return false;
		};

	client.ConnectionSuccessulCallback = []() {
		//Triggered when connection
		std::cout << "ConnectionSuccessulCallback invoked, asserting true\n";
		assert(true);
		};

	client.IOErrorCallback = [](std::optional<std::error_code> optionalEC) {
		std::cout << "IO error " << optionalEC.value().value();
		//We should get a End of file error, because the server refused the connection
		assert(optionalEC.value().value() == 2);

		};

	SleepMills(100);
	client.ConnectAsync("127.0.0.1", 60000);
	SleepMills(100);
	server.Update(true);
	client.Update();
	SleepMills(100);


}