#pragma once
#include "Message.h"
#include "asio.hpp"
#include "TSQueue.h"
#include "NetConnection.h"
#include <thread>

template<class T>
class Client : std::enable_shared_from_this<Client<T>>
{
public:
	Client() : m_socket(m_context)
	{

	}

	virtual ~Client()
	{
		Disconnect();
	}

	bool Connect(const std::string& host, const uint16_t port)
	{
		try
		{
			asio::ip::tcp::resolver res(m_context);
			auto endpoints = res.resolve(host, std::to_string(port));

			m_connection = std::make_unique<Connection<T>>(
				Connection<T>::Owner::ClientOwner,
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

	void Send(const Message<T>& msg)
	{
		if (IsConnected())
		{
			m_connection->Send(msg);

		}
	}

	TSQueue<OwnedMessage<T>>& GetMessages()
	{
		return m_inMessages;
	}
protected:
	asio::io_context m_context;
	std::thread m_contextThread;
	asio::ip::tcp::socket m_socket;
	std::unique_ptr<Connection<T>> m_connection;

private:
	TSQueue<OwnedMessage<T>> m_inMessages;
};