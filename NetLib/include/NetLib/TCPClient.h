#pragma once
#include "TCPMessage.h"
#include "asio.hpp"
#include "TSQueue.h"
#include "TCPClientServerConnection.h"
#include <thread>

template<class T>
class TCPClient : std::enable_shared_from_this<TCPClient<T>>
{
public:
	TCPClient() : m_socket(m_context)
	{

	}

	virtual ~TCPClient()
	{
		Disconnect();
	}

	/**
	* Async connection to specified host.
	* Upon success or failure of connection, OnConnectionSuccessful() or OnConnectionFail() are called respectively.
	* If failure, the context is stopped and everything is automatically disconnected, client only need to deal with custom clean up.
	* @param host The address of the server
	* @param port The server port
	*/
	void Connect(const std::string& host, const uint16_t port)
	{
		try
		{
			asio::ip::tcp::resolver res(m_context);
			auto endpoints = res.resolve(host, std::to_string(port));

			m_connection = std::make_unique<TCPClientServerConnection<T>>(
				m_context,
				asio::ip::tcp::socket(m_context),
				m_inMessages);

			m_connection->ConnectToServerAsync(endpoints, 	
				[this]() {	OnConnectionSuccessful();},
				[this]() {OnConnectionFail();}
				);
			m_contextThread = std::thread([this]() {m_context.run(); });

		}
		catch(std::exception& e)
		{
			std::cout << "[Client ERROR]: " << e.what() << "\n";
			Disconnect();
			OnConnectionFail();
		}
	}

	void Disconnect()
	{
		if (IsConnected())
		{
			m_connection->Disconnect();
		}

		m_context.stop();
		if (m_contextThread.joinable())
			m_contextThread.join();
	}

	bool IsConnected()
	{
		if (m_connection)
			return m_connection->IsConnected();

		return false;
	}

	void Send(const TCPMessage<T>& msg)
	{
		if (IsConnected())
		{
			m_connection->Send(msg);
		}
	}

	virtual void OnConnectionSuccessful()
	{}

	virtual void OnConnectionFail()
	{}

	TSQueue<TCPMessage<T>>& GetMessages()
	{
		return m_inMessages;
	}
protected:
	asio::io_context m_context;
	std::thread m_contextThread;
	asio::ip::tcp::socket m_socket;
	std::unique_ptr<TCPClientServerConnection<T>> m_connection;

private:
	TSQueue<TCPMessage<T>> m_inMessages;
};