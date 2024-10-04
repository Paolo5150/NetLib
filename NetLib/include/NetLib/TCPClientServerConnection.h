#pragma once
#include <iostream>
#include "NetMessage.h"
#include "TCPConnection.h"
#include "TSQueue.h"
#include <functional>
#include <optional>
enum class CallbackType
{
	ConnectionSuccess,
	ConnectionFail,
	IOError
};

struct CallbackToClient
{
	CallbackType CType;
	std::string Message;
	std::optional<std::error_code> ErrorCode;
};

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
		std::cout << "Nuked client con\n";
	}

	void ConnectToServerAsync(const asio::ip::tcp::resolver::results_type& endpoints, 
		const std::function<void(CallbackToClient)>& callback,
		const std::function<void(std::shared_ptr<TCPConnection<T>>, std::error_code)>& onError)
	{
		if (m_isConnected) return;

		asio::async_connect(m_socket, endpoints, [this, callback, onError](std::error_code ec, asio::ip::tcp::endpoint endpoint) {

			if (!ec)
			{
				m_onError = onError;

				m_isConnected.store(true);

				callback({ CallbackType::ConnectionSuccess, ""});

				ReadHeader();
			}
			else
			{
				Log("[Client] Failed to connect to server: ", ec.message());

				m_isConnected.store(false);
				m_socket.close();
				callback({ CallbackType::ConnectionFail, ec.message() });
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
		ReadHeader();
	}


};