#pragma once
#include <iostream>
#include "NetMessage.h"
#include "TCPConnection.h"
#include "TSQueue.h"
#include <functional>

template <class T>
class TCPClientServerConnection : public TCPConnection<T>
{
public:


	TCPClientServerConnection(asio::io_context& context, asio::ip::tcp::socket socket, TSQueue<NetMessage<T>>& in) :
		TCPConnection( context, std::move(socket)),
		m_inMessagesQ(in) 

	{
	}
	virtual ~TCPClientServerConnection() {
	}

	void ConnectToServerAsync(const asio::ip::tcp::resolver::results_type& endpoints)
	{
		asio::async_connect(m_socket, endpoints, [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {

			if (!ec)
			{

				ReadHeader();
			}
			else
			{
				std::cout << "[Client] Failed to connect to server: " << ec.message() << std::endl;

			}
			});
	}

	void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
	{
		//Calling function will catch exception
		asio::connect(m_socket, endpoints);		
	}

protected:
	TSQueue<NetMessage<T>>& m_inMessagesQ;

private:
	void AddToIncomingMessageQueue() override
	{
		m_inMessagesQ.PushBack(m_temporaryInMsg);
		std::cout << "Got message " << m_inMessagesQ.Size() << std::endl;
		ReadHeader();
	}

};