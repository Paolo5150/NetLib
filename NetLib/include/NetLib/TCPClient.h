#pragma once
#include "NetMessage.h"
#include "asio.hpp"
#include "TSQueue.h"
#include "TCPClientServerConnection.h"
#include <thread>
#include "Log.h"
template<class T>
class TCPClient : std::enable_shared_from_this<TCPClient<T>>
{
public:
	TCPClient() : m_socket(m_context)
	{
	}

	virtual ~TCPClient()
	{
		Destroy();
	}

	/**
	* Async connection to specified host.
	* Upon success or failure of connection, OnConnectionSuccessful() or OnConnectionFail() are called respectively.
	* If failure, the context is stopped and everything is automatically disconnected, client only need to deal with custom clean up.
	* @param host The address of the server
	* @param port The server port
	*/
	void ConnectAsync(const std::string& host, const uint32_t port)
	{
		try
		{
			asio::ip::tcp::resolver res(m_context);
			auto endpoints = res.resolve(host, std::to_string(port));
			if(!m_connection)
			{
				
				m_connection = std::make_shared<TCPClientServerConnection<T>>(
					m_context,
					asio::ip::tcp::socket(m_context),
					m_inMessages);
			}

			m_connection->ConnectToServerAsync(endpoints, 
				std::bind(&TCPClient::ConnectionCallback, this, std::placeholders::_1), //Connection callbacks
				std::bind(&TCPClient::OnError, this, std::placeholders::_1, std::placeholders::_2) //IOErrors callbacks
				);
			if(!m_contextThread)
				m_contextThread = std::make_unique<std::thread>([this]() {m_context.run(); });

			if (m_context.stopped())
			{
				asio::post([this]() {m_context.restart(); m_context.run(); });
			}

		}
		catch(std::exception& e)
		{
			Log("[Client ERROR]: ", e.what());
			OnConnectionFail();
			Destroy();
		}
	}

	/**
	* Cconnection to specified host.
	* Upon success or failure of connection, OnConnectionSuccessful() or OnConnectionFail() are called respectively.
	* If failure, the context is stopped and everything is automatically disconnected, client only need to deal with custom clean up.
	* @param host The address of the server
	* @param port The server port
	*/
	void Connect(const std::string& host, const uint32_t port)
	{
		try
		{
			asio::ip::tcp::resolver res(m_context);
			auto endpoints = res.resolve(host, std::to_string(port));

			m_connection = std::make_shared<TCPClientServerConnection<T>>(
				m_context,
				asio::ip::tcp::socket(m_context),
				m_inMessages);

			m_connection->ConnectToServer(endpoints);
			if (!m_contextThread)
				m_contextThread = std::make_unique<std::thread>([this]() {m_context.run(); });

		}
		catch (std::exception& e)
		{
			Log("[Client ERROR]: ", e.what());
			OnConnectionFail();
			Destroy();
		}
	}

	void Destroy()
	{
		if (IsConnected())
		{
			m_connection->Disconnect();
			m_connection.reset();
		}
		if(!m_context.stopped())
			m_context.stop();
		if (m_contextThread->joinable())
			m_contextThread->join();
	}

	bool IsConnected()
	{
		if (m_connection)
			return m_connection->IsConnected();

		return false;
	}

	void Send(const NetMessage<T>& msg)
	{
		if (IsConnected())
		{
			m_connection->Send(msg);
		}
	}

	/**
	* Invoked when using the sync connection
	* For async connection, use GetLatestConnectionCallback
	*/
	virtual void OnConnectionSuccessful()
	{}

	/**
	* Invoked when using the sync connection
	* For async connection, use GetLatestConnectionCallback
	*/
	virtual void OnConnectionFail()
	{}

	TSQueue<NetMessage<T>>& GetMessages()
	{
		return m_inMessages;
	}
	/**
	* Returns true if a connection callback is available (success or fail)
	*/
	bool GetLatestConnectionCallback(CallbackToClient& cb)
	{
		if (m_hasConnectionCallback)
		{
			cb = m_callbackInfo;
			m_hasConnectionCallback.store(false);
			return true;
		}

		return false;
	}
protected:
	asio::io_context m_context;
	std::unique_ptr<std::thread> m_contextThread;
	asio::ip::tcp::socket m_socket;
	std::shared_ptr<TCPClientServerConnection<T>> m_connection;

private:
	TSQueue<NetMessage<T>> m_inMessages;

	std::mutex m_callbackMutex;
	CallbackToClient m_callbackInfo;
	std::atomic<bool> m_hasConnectionCallback = false;
	//This is called on the context thread.
	//Save to callback infos, so client can poll these data 
	void ConnectionCallback(CallbackToClient c)
	{
		m_hasConnectionCallback.store(true);
		std::unique_lock<std::mutex> l(m_callbackMutex);
		m_callbackInfo = c;
	}

	//Callback from connection, run on context thread!
	void OnError(std::shared_ptr<TCPConnection<T>> client, std::error_code ec)
	{
		m_hasConnectionCallback.store(true);

		std::unique_lock<std::mutex> l(m_callbackMutex);
		m_callbackInfo.CType = CallbackType::IOError;
		m_callbackInfo.Message = ec.message();
		m_callbackInfo.ErrorCode = ec;

	}
};