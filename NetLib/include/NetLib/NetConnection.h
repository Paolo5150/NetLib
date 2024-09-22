#pragma once
#include <iostream>
#include "Message.h"
#include "TSQueue.h"

template <class T>
class Connection : public std::enable_shared_from_this<Connection<T>>
{
public:
	enum class Owner
	{
		ServerOwner,
		ClientOwner
	};

	Connection(Owner parent, asio::io_context& context, asio::ip::tcp::socket socket, TSQueue<OwnedMessage<T>>& in) :
		m_inMessagesQ(in),
		m_socket(std::move(socket)),
		m_asioContext(context),
		m_owner(parent)

	{
	}
	virtual ~Connection(){}

	void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints) 
	{
		if (m_owner == Owner::ClientOwner)
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

	}
	void ConnectToClient(uint32_t id)
	{
		if (m_owner == Owner::ServerOwner)
		{
			if (m_socket.is_open())
			{
				m_id = id;
				ReadHeader();
			}
		}

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
	void Send(const Message<T>& msg) 
	{
		asio::post(m_asioContext, [this, msg]() {
			bool isWriting = !m_outMEssagesQ.Empty();

			m_outMEssagesQ.PushBack(msg);
			if (!isWriting)
			{
				WriteHeader();

			}
			});
	}
	uint32_t GetID() { return m_id; }

protected:
	asio::ip::tcp::socket m_socket;

	asio::io_context& m_asioContext;
	Owner m_owner;
	TSQueue<Message<T>> m_outMEssagesQ;
	TSQueue<OwnedMessage<T>>& m_inMessagesQ;
	Message<T> m_temporaryInMsg;
	uint32_t m_id = 0;

private:
	void ReadHeader()
	{
		asio::async_read(m_socket, asio::buffer(&m_temporaryInMsg.Header, sizeof(MessageHeader<T>)), 
			[this](std::error_code ec, std::size_t length) {

				if (!ec)
				{
					if (m_temporaryInMsg.Header.Size > 0)
					{
						//std::cout << "body size should be " << (m_temporaryInMsg.Header.Size - sizeof(MessageHeader<T>)) << "\n";

						m_temporaryInMsg.Body.resize(m_temporaryInMsg.Header.Size - sizeof(MessageHeader<T>));

						ReadBody();
					}
					else
					{
						AddToIncomingMessageQueue();
					}
				}
				else
				{
					std::cout << "Read header failed! ID: " << m_id << "\n";
					m_socket.close();
				}
			});
	}

	void ReadBody()
	{
		asio::async_read(m_socket, asio::buffer(m_temporaryInMsg.Body.data(), m_temporaryInMsg.Body.size()),
			[this](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					AddToIncomingMessageQueue();
				}
				else
				{
					// As above!
					std::cout << "[" << m_id << "] Read Body Fail.\n";
					m_socket.close();
				}
			});
	}

	void AddToIncomingMessageQueue()
	{
		if (m_owner == Owner::ServerOwner)
		{
			OwnedMessage<T> om;
			om.Remote = this->shared_from_this();
			om.TheMessage = m_temporaryInMsg;
			
			m_inMessagesQ.PushBack(om);
		}
		else
		{
			m_inMessagesQ.PushBack({ nullptr, m_temporaryInMsg });
		}

		ReadHeader();
	}

	void WriteHeader()
	{
		asio::async_write(m_socket, asio::buffer(&m_outMEssagesQ.Front().Header, sizeof(MessageHeader<T>)),
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