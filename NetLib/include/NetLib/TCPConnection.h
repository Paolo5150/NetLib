#pragma once
#include <iostream>
#include "TCPMessage.h"
#include "TSQueue.h"
#include <functional>

template <class T>
class TCPConnection
{
public:

	TCPConnection(asio::io_context& context, asio::ip::tcp::socket socket) :
		m_socket(std::move(socket)),
		m_asioContext(context)

	{
	}
	virtual ~TCPConnection() {
		m_socket.close();
	}

	void Disconnect()
	{
		if (IsConnected())
		{
			asio::post(m_asioContext, [this]() {
				m_socket.close();
				});
		}
	}

	bool IsConnected()
	{
		return m_socket.is_open();
	}

	void Send(const TCPMessage<T>& msg)
	{
		asio::post(m_asioContext, [this, msg]() {
			bool isWriting = !m_outMEssagesQ.Empty();

			m_outMEssagesQ.PushBack(msg);
			if (!isWriting)
				WriteHeader();
			});
	}

	uint32_t GetID() { return m_id; }

	std::string GetEndpointInfo()
	{
		std::stringstream ss;
		ss << m_socket.remote_endpoint();
		return ss.str();
	}

protected:
	asio::ip::tcp::socket m_socket;
	asio::io_context& m_asioContext;
	TSQueue<TCPMessage<T>> m_outMEssagesQ;
	TCPMessage<T> m_temporaryInMsg;
	uint32_t m_id = 0;

	void ReadHeader()
	{
		asio::async_read(m_socket, asio::buffer(&m_temporaryInMsg.Header, sizeof(TCPMessageHeader<T>)),
			[this](std::error_code ec, std::size_t length) {

				if (!ec)
				{
					if (m_temporaryInMsg.Header.Size > 0)
					{
						m_temporaryInMsg.Body.resize(m_temporaryInMsg.Header.Size - sizeof(TCPMessageHeader<T>));
						ReadBody();
					}
					else
						AddToIncomingMessageQueue();
				}
				else
				{
					std::cout << "Read header failed! ID: " << m_id << " " << ec.value() << " " << ec.message() << "\n";
					m_socket.close();
				}
			});
	}
private:

	void ReadBody()
	{
		asio::async_read(m_socket, asio::buffer(m_temporaryInMsg.Body.data(), m_temporaryInMsg.Body.size()),
			[this](std::error_code ec, std::size_t length)
			{
				if (!ec)
					AddToIncomingMessageQueue();
				else
				{
					std::cout << "[" << m_id << "] Read Body Fail.\n";
					m_socket.close();
				}
					
			});
	}
	/**
	* Server and client connections have different message queue types, so each implement its own version of this method
	* The method must call ReadHeader to initiate a subsequent reading, after the message has been moved to the incoming queue
	*/
	virtual void AddToIncomingMessageQueue() = 0;

	void WriteHeader()
	{
		asio::async_write(m_socket, asio::buffer(&m_outMEssagesQ.Front().Header, sizeof(TCPMessageHeader<T>)),
			[this](std::error_code ec, std::size_t length) {
				if (!ec)
				{
					if (m_outMEssagesQ.Front().Body.size() > 0)
					{
						WriteBody();
					}
					else
					{
						m_outMEssagesQ.PopFront();
						if (!m_outMEssagesQ.Empty())
							WriteHeader();						
					}
				}
				else
				{
					// As above!
					std::cout << "Write header failed! ID: " << m_id << "\n";
					m_socket.close();
				}
			});
	}

	void WriteBody()
	{
		asio::async_write(m_socket, asio::buffer(m_outMEssagesQ.Front().Body.data(), m_outMEssagesQ.Front().Body.size()),
			[this](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					m_outMEssagesQ.PopFront();

					// If the queue still has messages in it, then issue the task to 
					// send the next messages' header.
					if (!m_outMEssagesQ.Empty())
					{
						WriteHeader();
					}
				}
				else
				{
					// Sending failed, see WriteHeader() equivalent for description :P
					std::cout << "Write body failed! ID: " << m_id << "\n";
					m_socket.close();
				}
			});
	}

};