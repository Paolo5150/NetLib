#pragma once
#include "TCPMessage.h"
#include "TSQueue.h"
#include "TCPConnection.h"

template<class T>
class TCPServer
{
public:
	TCPServer(uint16_t port) :
		m_asioAcceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
	{

	}

	virtual ~TCPServer()
	{
		Stop();
	}

	bool Start()
	{
		try
		{
			WaitForConnection();
			m_contextThread = std::thread([this]() {m_context.run(); });

		}
		catch (std::exception& e)
		{
			std::cout << "[Server] ERROR: " << e.what() << "\n";
			return false;
		}

		std::cout << "[Server] Started!\n";
		return true;
	}

	void Stop()
	{
		m_context.stop();
		if (m_contextThread.joinable()) m_contextThread.join();
		std::cout << "[Server] Stopped\n";
	}

	void WaitForConnection()
	{
		std::cout << "[Server] Waiting for connection:...\n";

		m_asioAcceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
			if (!ec)
			{
				std::cout << "[Server] New Connection: " << socket.remote_endpoint() << std::endl;
				std::shared_ptr<TCPConnection<T>> neWcon = std::make_shared<TCPConnection<T>>(TCPConnection<T>::Owner::ServerOwner, m_context, std::move(socket), m_inMessages);
				
				if (OnClientConnection(neWcon, m_clientIDCounter))
				{
					m_connections.push_back(std::move(neWcon));
					m_connections.back()->ConnectToClient(m_clientIDCounter);
					m_connections.back()->SetOnErrorCallback([this](int errorCode, std::shared_ptr < TCPConnection<T>> client) {

						std::cout << "[Server] Received error: " << errorCode << "\n";

						switch (errorCode)
						{
							case 10054: //Client disconnected
								OnClientDisconnection(client);
								m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), client), m_connections.end());
								break;

						default:
							break;
						}
						});
					m_clientIDCounter++;
					std::cout << "[Server] Connection approved by server\n";
				
				}
				else
				{
					std::cout << "[Server] Connection refused by server\n";
				
				}
			}
			else
			{
				std::cout << "[Server] New Connection Error: " << ec.message() << std::endl;
			}
			
			WaitForConnection();
			});
	}

	void MessageClient(std::shared_ptr<TCPConnection<T>> client, const TCPMessage<T>& msg)
	{
		if (client && client->IsConnected())
		{
			client->Send(msg);
		}
		else
		{
			OnClientDisconnection(client);
			client.reset();
			m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), client), m_connections.end());
		}
	}

	void MessageAllClients(const TCPMessage<T>& msg, std::shared_ptr<TCPConnection<T>> ignoreClient = nullptr)
	{
		bool foundInvalid = false;
		for (auto& client : m_connections)
		{
			if (client && client->IsConnected())
			{
				if(client != ignoreClient)
					client->Send(msg);
			}
			else
			{
				OnClientDisconnection(client);
				client.reset();
				foundInvalid = true;
				m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), client), m_connections.end());
			}
		}

		if (foundInvalid)
		{
			m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), nullptr), m_connections.end());
		}
	}

	void Update(size_t maxMessages = -1)
	{
		size_t messageCount = 0;
		while (messageCount < maxMessages && !m_inMessages.Empty())
		{
			auto msg = m_inMessages.PopFront();
			OnMessage(msg.Remote, msg.TheMessage);
			messageCount++;
		}
	}

protected:
	virtual bool OnClientConnection(std::shared_ptr <TCPConnection<T>> client, uint32_t assignedID)
	{
		return false;
	}

	virtual void OnClientDisconnection(std::shared_ptr <TCPConnection<T>> client)
	{
	}

	virtual void OnMessage(std::shared_ptr<TCPConnection<T>> client, const TCPMessage<T>& msg)
	{

	}

	std::deque<std::shared_ptr<TCPConnection<T>>> m_connections;
	TSQueue < OwnedTCPMessage<T>> m_inMessages;
	asio::io_context m_context;
	std::thread m_contextThread;
	asio::ip::tcp::acceptor m_asioAcceptor;
	uint32_t m_clientIDCounter = 10000;

};