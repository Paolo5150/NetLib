#pragma once
#include <iostream>
#include "NetMessage.h"
#include "TSQueue.h"
#include <functional>
#include <windows.h>
#include "Log.h"
template <class T>
class TCPConnection : public std::enable_shared_from_this<TCPConnection<T>>
{
public:

	TCPConnection(asio::io_context& context, asio::ip::tcp::socket socket) :
		m_socket(std::move(socket)),
		m_asioContext(context)

	{
	}
	virtual ~TCPConnection() {
		m_isConnected.store(false);

	}

	void Disconnect()
	{
		if (IsConnected())
		{
			m_isConnected.store(false);
			asio::post(m_asioContext, [this]() {
				m_socket.cancel();
				m_socket.shutdown(asio::ip::tcp::socket::shutdown_both);
				m_socket.close();
				});
		}
	}

	bool IsConnected()
	{
		return m_isConnected.load();
	}

	void Send(const NetMessage<T>& msg)
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
	TSQueue<NetMessage<T>> m_outMEssagesQ;
	NetMessage<T> m_temporaryInMsg;
	uint32_t m_id = 0;
	std::atomic<bool> m_isConnected = false;

	std::function<void(std::shared_ptr<TCPConnection<T>>, std::error_code)> m_onError;

	void ReadHeader()
	{
		asio::async_read(m_socket, asio::buffer(&m_temporaryInMsg.Header, sizeof(NetMessageHeader<T>)),
			[this](std::error_code ec, std::size_t length) {

				if (!ec)
				{
					if (m_temporaryInMsg.Header.Size > 0)
					{
						m_temporaryInMsg.Payload.resize(m_temporaryInMsg.Header.Size - sizeof(NetMessageHeader<T>));
						ReadBody();
					}
					else
						AddToIncomingMessageQueue();
				}
				else
				{
					Log("[TCP Connection]: Error reading header ", ec.message());
					if (m_onError)
						m_onError(this->shared_from_this(), ec);
					m_socket.close();
					m_isConnected.store(false);

				}
			});
	}
private:

	void ReadBody()
	{
		asio::async_read(m_socket, asio::buffer(m_temporaryInMsg.Payload.data(), m_temporaryInMsg.GetPayloadSize()),
			[this](std::error_code ec, std::size_t length)
			{
				if (!ec)
					AddToIncomingMessageQueue();
				else
				{
					Log("[TCP Connection]: Error reading body ", ec.message());
					if (m_onError)
						m_onError(this->shared_from_this(), ec);

					m_isConnected.store(false);
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
		asio::async_write(m_socket, asio::buffer(&m_outMEssagesQ.Front().Header, sizeof(NetMessageHeader<T>)),
			[this](std::error_code ec, std::size_t length) {
				if (!ec)
				{
					if (m_outMEssagesQ.Front().GetPayloadSize() > 0)
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
					Log("[TCP Connection]: Error writing header ", ec.message());

					if (m_onError)
						m_onError(this->shared_from_this(), ec);

					m_isConnected.store(false);
					m_socket.close();
				}
			});
	}

	void WriteBody()
	{
		asio::async_write(m_socket, asio::buffer(m_outMEssagesQ.Front().Payload.data(), m_outMEssagesQ.Front().GetPayloadSize()),
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
					Log("[TCP Connection]: Error writing body ", ec.message());

					if (m_onError)
						m_onError(this->shared_from_this(), ec);

					m_isConnected.store(false);
					m_socket.close();
				}
			});
	}

};