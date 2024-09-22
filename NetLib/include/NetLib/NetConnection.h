#pragma once
#include <iostream>
#include "Message.h"
#include "TSQueue.h"

template <class T>
class Connection : std::enable_shared_from_this<Connection<T>>
{
public:
	Connection(){}
	virtual ~Connection(){}

	bool ConnectToServer() {}
	bool Disconnect() {}
	bool IsConnected(){}
	bool Send(const Message<T>& msg) {}

protected:
	asio::ip::tcp::socket m_socket;

	asio::io_context& m_asioContext;

	TSQueue<Message<T>> m_outMEssagesQ;
	TSQueue<OwnedMessage<T>>& m_inMessagesQ;

};