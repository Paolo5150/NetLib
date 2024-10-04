#pragma once
#include <asio.hpp>
#include "UDPPacket.h"
#include "UDPPacketAssembler.h"
#include "TSQueue.h"
#include <unordered_map>
#include "Log.h"

template<class T>
class UDPMessager
{
public:
	UDPMessager() :
		m_socket(asio::ip::udp::socket(m_context, asio::ip::udp::v4())),
		m_work_guard(asio::make_work_guard(m_context))

	{
		m_contextThread = std::thread([this]() {m_context.run(); });
	}

	~UDPMessager()
	{
		m_work_guard.reset();
		if (m_socket.is_open())
		{
			m_socket.cancel();
			m_socket.close();
		}
		if (m_contextThread.joinable())
		{
			m_context.stop();
			m_contextThread.join();
		}
	}

	void Send(const NetMessage<T>& msg, const std::string& sendToAddress, uint32_t port)
	{
		try
		{
			asio::post(m_context, [this, msg, sendToAddress, port]() {
				asio::ip::udp::resolver res(m_context);

				m_endpoint = *res.resolve(asio::ip::udp::v4(), sendToAddress, std::to_string(port)).begin();
				bool isWriting = !m_outMessages.Empty();
				m_outMessages.PushBack(msg);

				if (!isWriting)
					WriteSocket();
				});
		}
		catch (std::exception& e)
		{
			Log("[UDP Messager]: Failed to send ", e.what());
		}

	}

	void StartListening(uint32_t port)
	{
		try
		{
			if (!m_socket.is_open())
			{
				m_socket = asio::ip::udp::socket(m_context, asio::ip::udp::v4()); 
			}
			m_socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), port));
			Log("[UDP Messager]: Started listening ", port);

			Receive();
		}
		catch (std::exception& e)
		{
			Log("[UDP Messager]: Error creating UDP Messager: ", e.what());
		}
	}

	/**
	* To be called by inheriting class, in a loop, to process available messages
	*/
	void Update(bool nonBlocking = false, size_t maxMessages = -1 )
	{
		if (!nonBlocking)
		{
			m_inMessages.Wait(); //Unblocked when the queue has something in it, or when calling ForceWake
		}

		size_t messageCount = 0;
		while (messageCount < maxMessages && !m_inMessages.Empty())
		{
			auto msg = m_inMessages.PopFront();
			OnMessage(msg);
			messageCount++;
		}

		//Notify of diconnection
		{
			std::unique_lock<std::mutex> l(m_disonnectListMutex);
			for (auto& s : m_disconnections)
				OnDisconnection(s);

			m_disconnections.clear();
		}
		
		//Notify IO error
		{
			std::unique_lock<std::mutex> l(m_ioErrorMutex);
			for (auto& ec : m_ioErrors)
			{
				if (OnIOError(ec))
				{
					m_socket.close();
				}
			}

			m_ioErrors.clear();
		}

	}

	/**
	* OnMessage: To be overridden by inheriting class.
	* This function is called whenever a complete message is available.
	*/
	virtual void OnMessage(OwnedUDPMessage<T> msg) = 0;
	virtual void OnDisconnection(const std::string& addressPort) = 0;

	/**
	* On Read/Write error
	* Return true to automatically close the socket
	*/
	virtual bool OnIOError(std::error_code ec) = 0;

	/**
	* SetDropMessageThreshold: Set the time threshold for dropping incomplete messages.
	*/
	void SetDropMessageThreshold(uint32_t timeMillis)
	{
		m_dropMessageThresholdMills = timeMillis;
	}

	/**
	* SetDropMessageThreshold: Set the time threshold for disconnecting an inactive endpoint
	*/
	void SetDisconnectEndpointThreshold(uint32_t timeMillis)
	{
		m_disconnectEndpointThresholdMills = timeMillis;
	}

	/**
	* CheckIncompleteMessages: Check if any incomplete messages have exceeded the threshold.
	* Drops them if they have not been completed within the threshold time.
	*/
	void CheckIncompleteMessages()
	{
		//Check for potentially incomplete messages
		//Remove from map if found
		auto now = std::chrono::high_resolution_clock::now();

		for (auto s = m_packetMap.begin(); s != m_packetMap.end(); s++ )
		{
			auto it = s->second.begin();

			for (auto it = s->second.begin(); it != s->second.end(); )
			{
				double timeSinceUpdate = (double)std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.LastUpdated).count();
	
				if (timeSinceUpdate > m_dropMessageThresholdMills)
				{
					Log("[UDP Messager]: Packet ID ", it->second.PacketID, " last updated ", timeSinceUpdate, ". Over threshold, will drop message");
					it = s->second.erase(it);
				}
				else
					it++;
			}
		}
	}

	void CheckInactiveEndpoints()
	{
		auto now = std::chrono::high_resolution_clock::now();

		for (auto it = m_endpointLastUpdate.begin(); it != m_endpointLastUpdate.end(); )
		{
			double timeSinceUpdate = (double)std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count();
			if (timeSinceUpdate > m_disconnectEndpointThresholdMills)
			{
				Log("[UDP Messager]: Endpoint", it->first, " inactive for ", timeSinceUpdate, ", will be removed.");
				{
					std::unique_lock<std::mutex> l(m_disonnectListMutex);
					m_disconnections.push_back(it->first);
				}
				m_packetMap.erase(it->first);
				it = m_endpointLastUpdate.erase(it);
			}
			else
				it++;
		}

		std::unique_lock<std::mutex> l(m_disonnectListMutex);
		if (m_disconnections.size() > 0)
			m_inMessages.ForceWake(); //Force the update, so disconnections callback is invoked
	}

	/**
	* Removes a specific endpoint (and pending packets) from memory.
	* To be called by inheriting class, if necessary (eg. client sends a 'disconnect' message)
	*/
	void DisconnectEndpoint(asio::ip::udp::endpoint& senderPoint)
	{
		std::string senderKey = CreateSenderKey(senderPoint);
		m_packetMap.erase(senderKey);
		m_endpointLastUpdate.erase(senderKey);
	}

	/**
	* Returns true if we received any packets from specified endpoint (which means it's considered connected).
	*/
	bool HasEndpoint(asio::ip::udp::endpoint& senderPoint)
	{
		std::string senderKey = CreateSenderKey(senderPoint);

		auto sender = m_packetMap.find(senderKey);
		return sender != m_packetMap.end();
	}

protected:
	struct PacketInfo
	{
		T MessageID;
		uint32_t PacketsCount = 0;
		uint32_t PacketID = 0; //Not really needed, just for testing
		std::vector < UDPPacket<T>> Packets;
		std::vector < bool> PacketsSet;
		std::chrono::high_resolution_clock::time_point LastUpdated;

	};
	uint8_t m_receiveBuffer[MTULimit];
	std::unordered_map<std::string, std::unordered_map<uint16_t, PacketInfo>> m_packetMap;
	std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> m_endpointLastUpdate;

	size_t GetAvailableMessagesCount()
	{
		return m_inMessages.Size();
	}

	bool HasPendingPacketOfId(uint16_t id, asio::ip::udp::endpoint& senderPoint)
	{
		std::string senderKey = CreateSenderKey(senderPoint);
		auto send = m_packetMap.find(senderKey);
		if (send != m_packetMap.end())
		{
			auto it = send->second.find(id);
			return it != send->second.end();
		}
		return false;
	}

	

	void ProcessPacket(asio::ip::udp::endpoint& senderPoint, uint32_t bytesRead)
	{
		std::string senderKey = CreateSenderKey(senderPoint);
		m_endpointLastUpdate[senderKey] = std::chrono::high_resolution_clock::now();

		//Packet info
		UDPPacket<T> packet(m_receiveBuffer, bytesRead);
		auto h = packet.ExtractHeader();


		auto sender = m_packetMap.find(senderKey);
		if (sender == m_packetMap.end())
		{
			m_packetMap[senderKey] = {};
			sender = m_packetMap.find(senderKey);
		}
			
		auto it = sender->second.find(h.PacketID);
		if (it == sender->second.end())
		{
			sender->second[h.PacketID].Packets.resize(h.PacketMaxSequenceNumbers);
			sender->second[h.PacketID].PacketsSet.resize(h.PacketMaxSequenceNumbers, false);
			sender->second[h.PacketID].LastUpdated = std::chrono::high_resolution_clock::now();
			sender->second[h.PacketID].MessageID = h.MessageID;
			sender->second[h.PacketID].PacketID = h.PacketID;
			sender->second[h.PacketID].PacketsCount = 1;
			sender->second[h.PacketID].Packets[h.PacketSequenceNumber] = packet;
			sender->second[h.PacketID].PacketsSet[h.PacketSequenceNumber] = true;

		}
		else
		{
			if (!sender->second[h.PacketID].PacketsSet[h.PacketSequenceNumber])
			{
				sender->second[h.PacketID].PacketsCount++;
				sender->second[h.PacketID].Packets[h.PacketSequenceNumber] = packet;
				sender->second[h.PacketID].PacketsSet[h.PacketSequenceNumber] = true;
				sender->second[h.PacketID].LastUpdated = std::chrono::high_resolution_clock::now();
			}
			else
				Log("[UDP Messager]: Skpping duplicate packets ", h.PacketID, " sequence ", h.PacketSequenceNumber);
		}

		if (sender->second[h.PacketID].PacketsCount == h.PacketMaxSequenceNumbers)
		{
			OwnedUDPMessage<T> msg;
			msg.TheMessage.SetMessageID(sender->second[h.PacketID].MessageID);

			msg.RemoteAddress = senderPoint.address().to_string();
			msg.RemotePort = senderPoint.port();

			//Dump the payload directly in the message
			m_packetAssembler.AssemblePayloadFromPackets(sender->second[h.PacketID].Packets, msg.TheMessage.GetPayload());
			m_inMessages.PushBack(std::move(msg));

			sender->second.erase(h.PacketID);
		}
	}

private:
	
	asio::io_context m_context;  // The IO context to handle asynchronous IO.
	std::thread m_contextThread;  // The thread that runs the IO context.
	asio::ip::udp::socket m_socket;  // UDP socket for communication.
	asio::ip::udp::endpoint m_senderPoint;  // Endpoint that stores the sender's information.
	UDPPacketAssembler<T> m_packetAssembler;  // Assembler to reconstruct messages from packets.
	uint32_t m_dropMessageThresholdMills = 500;  // Time threshold to drop incomplete messages (in milliseconds).
	uint32_t m_disconnectEndpointThresholdMills = 30000;  // Time threshold to remove an inactive endpoint from the map (disconnection.

	std::mutex m_disonnectListMutex;
	std::vector<std::string> m_disconnections;

	std::mutex m_ioErrorMutex;
	std::vector<std::error_code> m_ioErrors;

	UDPPacket<T> m_outPacket;
	std::vector<UDPPacket<T>> m_packets; //Local cache 

	TSQueue<OwnedUDPMessage<T>> m_inMessages;  // Thread-safe queue to store incoming messages.
	TSQueue<NetMessage<T>> m_outMessages;
	asio::ip::udp::endpoint m_endpoint;
	asio::executor_work_guard<asio::io_context::executor_type> m_work_guard;

	std::string CreateSenderKey(asio::ip::udp::endpoint endPoint)
	{
		return endPoint.address().to_string() + ":" + std::to_string(endPoint.port());

	}
	/**
	* Receive: Start asynchronous receiving of UDP packets.
	* It recursively calls itself to continue receiving packets indefinitely.
	*/
	void Receive()
	{
		if (m_socket.is_open())
		{
			m_socket.async_receive_from(asio::buffer(m_receiveBuffer, MTULimit), m_senderPoint,
				[this](std::error_code ec, std::size_t bytesRead) {

					//Update that this endpoint received an update
					//there's one in ProcessPacket too, maybe not necessary, only really used for the test
					//Won't hurt to have it anyway
					std::string senderKey = CreateSenderKey(m_senderPoint);
					m_endpointLastUpdate[senderKey] = std::chrono::high_resolution_clock::now();

					CheckInactiveEndpoints();
					CheckIncompleteMessages();

					if (!ec)
					{
						if (bytesRead > 0)
							ProcessPacket(m_senderPoint, (uint32_t)bytesRead);

						Receive();
					}
					else if (ec == asio::error::operation_aborted)
						Log("[UDP Messager]: Aborted gracefully");
					else
					{
						Log("[UDP Messager]:  Failed to read ", ec.message());
						std::unique_lock<std::mutex> l(m_ioErrorMutex);
						m_ioErrors.push_back(ec);
					}

				});
		}
	}

	void WriteSocket()
	{
		if (m_outMessages.Empty()) return;

		auto& msg = m_outMessages.PopFront();
		m_packets = m_packetAssembler.CreatePackets(msg);
		auto packetCounts = m_packets.size();

		for (auto& p : m_packets)
		{
			m_socket.async_send_to(asio::buffer(p.DataBuffer, p.DataBuffer.size()), m_endpoint,
				[this, packetCounts](std::error_code ec, std::size_t bytes_sent) mutable {

					if (!ec)
					{
						packetCounts--;
						if (packetCounts == 0)
						{
							WriteSocket();
						}
					}
					else
					{
						Log("[UDP Messager]:  Failed to send ", ec.message());
						std::unique_lock<std::mutex> l(m_ioErrorMutex);
						m_ioErrors.push_back(ec);
					}
				});
		}
	}

};