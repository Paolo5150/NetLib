#pragma once
#include <iostream>
#include "NetMessage.h"
#include "TSQueue.h"
#include "TCPConnection.h"
#include <functional>

template <class T>
class TCPServerClientConnection : public TCPConnection<T>
{
public:

	TCPServerClientConnection(asio::io_context& context, asio::ip::tcp::socket socket, TSQueue<OwnedTCPMessage<T>>& in)
		: TCPConnection(context, std::move(socket)),
		m_inMessagesQ(in)

	{
	}

	virtual ~TCPServerClientConnection() 
	{
	}

	void ConnectToClient(uint32_t id, const std::function<void(std::shared_ptr<TCPConnection<T>>, std::error_code)>& onError)
	{
		if (m_socket.is_open())
		{
			m_id = id;
			m_onError = onError;
			ReadHeader();
		}
	}

protected:
	TSQueue<OwnedTCPMessage<T>>& m_inMessagesQ;

private:

	void AddToIncomingMessageQueue() override
	{
		OwnedTCPMessage<T> om;
		om.Remote = this->shared_from_this();
		om.TheMessage = m_temporaryInMsg;
		m_inMessagesQ.PushBack(om);
		ReadHeader();
	}

};