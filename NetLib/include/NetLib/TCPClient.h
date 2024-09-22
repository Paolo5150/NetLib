#pragma once
#include "TCPMessage.h"
#include "asio.hpp"
#include "TSQueue.h"
#include "TCPConnection.h"
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

	bool Connect(const std::string& host, const uint16_t port)
	{
		try
		{
			asio::ip::tcp::resolver res(m_context);
			auto endpoints = res.resolve(host, std::to_string(port));

			m_connection = std::make_unique<TCPConnection<T>>(
				TCPConnection<T>::Owner::ClientOwner,
				m_context,
				asio::ip::tcp::socket(m_context),
				m_inMessages);

			m_connection->ConnectToServer(endpoints);
			

			m_contextThread = std::thread([this]() {m_context.run(); });
			std::cout << "[Client]: Connected "  << "\n";

			return true;
		}
		catch(std::exception& e)
		{
			std::cout << "[Client ERROR]: " << e.what() << "\n";
			return false;
		}
		return false;
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

	TSQueue<OwnedTCPMessage<T>>& GetMessages()
	{
		return m_inMessages;
	}
protected:
	asio::io_context m_context;
	std::thread m_contextThread;
	asio::ip::tcp::socket m_socket;
	std::unique_ptr<TCPConnection<T>> m_connection;

private:
	TSQueue<OwnedTCPMessage<T>> m_inMessages;
};