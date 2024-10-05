#pragma once
#include "NetMessage.h"
#include "TSQueue.h"
#include "TCPServerClientConnection.h"
#include <functional>
#include "Log.h"

template<class T>
struct ErrorInfo
{
	std::shared_ptr<TCPConnection<T>> Client;
	std::error_code ErrorCode;
};

template<class T>
class TCPServer
{
public:
	TCPServer(uint32_t port) :
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
			Log("[TCPServer] ERROR: ", e.what());

			return false;
		}

		Log("[TCPServer] Started!");
		return true;
	}

	void Stop()
	{
		m_context.stop();
		if (m_contextThread.joinable()) m_contextThread.join();
		Log("[TCPServer] Stopped!");
	}

	void MessageClient(std::weak_ptr<TCPConnection<T>> cl, const NetMessage<T>& msg)
	{
		if (auto client = cl.lock())
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
		
	}

	void MessageAllClients(const NetMessage<T>& msg, std::weak_ptr<TCPConnection<T>> ignoreClient = nullptr)
	{
		bool foundInvalid = false;
		for (auto& client : m_connections)
		{
			if (client && client->IsConnected())
			{
				if (auto c = ignoreClient.lock())
				{
					if (client != c)
						client->Send(msg);
				}
				
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

	void Update(bool nonblocking = false, size_t maxMessages = -1)
	{
		if (!nonblocking)
		{
			m_inMessages.Wait(); //Unblocked when the queue has something in it
		}

		size_t messageCount = 0;
		while (messageCount < maxMessages && !m_inMessages.Empty())
		{
			auto msg = m_inMessages.PopFront();
			OnMessage(msg.Remote, msg.TheMessage);
			messageCount++;
		}

		std::unique_lock<std::mutex> l(m_errorCallbackMutex);
		for (auto& error : m_errors)
			if (OnIOError(error.Client, error.ErrorCode))
			{
				OnClientDisconnection(error.Client);
				m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), error.Client), m_connections.end());
				error.Client.reset();
			}
			

		m_errors.clear();

	}

protected:
	/**
	* Invoked when a new client is connected.
	* Must return true/false to determine whether the connection is to be accepted. Default is false.
	* @param client The pointer to the client connection
	* @param assigned ID The artificial ID assigned to the client, should the connection be accepted
	*/
	virtual bool OnClientConnection(std::weak_ptr <TCPConnection<T>> client, uint32_t assignedID)
	{
		return false;
	}

	/**
	* Invoked when a new client is disconnected.
	* @param client The pointer to the client connection
	* @param assigned ID The artificial ID assigned to the client, should the connection be accepted
	*/
	virtual void OnClientDisconnection(std::weak_ptr <TCPConnection<T>> client)
	{
	}

	/**
	* Invoked when a new message is received.
	* @param client The client sending the message
	* @param msg The message
	*/
	virtual void OnMessage(std::weak_ptr<TCPConnection<T>> client, const NetMessage<T>& msg)
	{

	}

	/**
	* Invoked when a read/write message occur.
	* Return true to disconnect the client throwing the error
	*/
	virtual bool OnIOError(std::weak_ptr<TCPConnection<T>> client, std::error_code ec)
	{
		return true;
	}
private:

	std::deque<std::shared_ptr<TCPServerClientConnection<T>>> m_connections;
	TSQueue <OwnedTCPMessage<T>> m_inMessages;
	asio::io_context m_context;
	std::thread m_contextThread;
	asio::ip::tcp::acceptor m_asioAcceptor;
	uint32_t m_clientIDCounter = 0;

	std::mutex m_errorCallbackMutex;
	std::vector <ErrorInfo<T>> m_errors;

	//Callback from connection, run on context thread!
	void OnError(std::shared_ptr<TCPConnection<T>> client, std::error_code ec)
	{
		{
			std::unique_lock<std::mutex> l(m_errorCallbackMutex);
			m_errors.push_back({ client, ec });
		}
		
		m_inMessages.ForceWake(); //Force update cycle
	}

	void WaitForConnection()
	{
		Log("[TCPServer] Waiting for connection:...");
		m_asioAcceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
			if (!ec)
			{
				Log("[TCPServer] New Connection: ", socket.remote_endpoint());

				std::shared_ptr<TCPServerClientConnection<T>> neWcon = std::make_shared<TCPServerClientConnection<T>>(m_context, std::move(socket), m_inMessages);

				if (OnClientConnection(neWcon, m_clientIDCounter))
				{
					m_connections.push_back(std::move(neWcon));
					m_connections.back()->ConnectToClient(m_clientIDCounter, std::bind(&TCPServer::OnError, this, std::placeholders::_1, std::placeholders::_2));
					m_clientIDCounter++;
					Log("[TCPServer] Connection approved by server");
				}
				else
				{
					neWcon.reset();
					Log("[TCPServer] Connection refused by server");
				}
			}
			else
			{
				Log("[TCPServer] New Connection Error: ", ec.message());
			}

			WaitForConnection();
			});
	}

};