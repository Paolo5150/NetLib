#include <asio.hpp>
#include "UDPPacket.h"
#include "UDPPacketAssembler.h"
#include "TSQueue.h"
#include <unordered_map>
template<class T>
class UDPReceiver
{
public:
	UDPReceiver(uint16_t port, bool automaticallyStartReceiving = true) :
		m_socket(asio::ip::udp::socket(m_context, asio::ip::udp::v4()))

	{
		try
		{
			m_socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), port));
			m_contextThread = std::thread([this]() {m_context.run(); });
			if(automaticallyStartReceiving)
				Receive();
		}
		catch (std::exception& e)
		{
			std::cout << "Error creating UDP Receiver: " << e.what();

		}
	}

	~UDPReceiver()
	{
		if (m_socket.is_open())
			m_socket.close();
		if (m_contextThread.joinable())
		{
			m_context.stop();
			m_contextThread.join();
		}
	}

	/**
	* To be called by inheriting class, in a loop, to process available messages
	*/
	void Update(size_t maxMessages = -1)
	{
		m_inMessages.Wait(); //Unblocked when the queue has something in it

		size_t messageCount = 0;
		while (messageCount < maxMessages && !m_inMessages.Empty())
		{
			auto msg = m_inMessages.PopFront();
			OnMessage(msg);
			messageCount++;
		}
	}

	/**
	* OnMessage: To be overridden by inheriting class.
	* This function is called whenever a complete message is available.
	*/
	virtual void OnMessage(OwnedUDPMessage<T> msg) = 0;

	/**
	* SetDropMessageThreshold: Set the time threshold for dropping incomplete messages.
	*/
	void SetDropMessageThreshold(uint32_t timeMillis)
	{
		m_dropMessageThresholdMills = timeMillis;
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

		for (auto it = m_packetMap.begin(); it != m_packetMap.end(); )
		{
			auto timeSinceUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.LastUpdated).count();
			if (timeSinceUpdate > m_dropMessageThresholdMills)
			{
				std::cout << "Packet ID " << it->second.PacketID << " last updated " << timeSinceUpdate << ". Over threshold, will drop message\n";
				it = m_packetMap.erase(it);
			}
			else
				it++;
		}
	}

	/**
	* Receive: Start asynchronous receiving of UDP packets.
	* It recursively calls itself to continue receiving packets indefinitely.
	*/
	void Receive()
	{
		if (m_socket.is_open())
		{
			m_socket.async_receive_from(asio::buffer(m_receiveBuffer, MTULimit), senderPoint,
				[this](std::error_code ec, std::size_t bytesRead) {

					CheckIncompleteMessages();

					if (!ec)
					{

						if (bytesRead > 0)
							ProcessPacket(senderPoint, bytesRead);

						Receive();
					}
					else
						std::cout << "[UDP Receiver]: Failed to Got: " << ec.message() << "\n";

				});
		}
	}

protected:
	struct PacketInfo
	{
		T MessageID;
		uint32_t PacketsCount = 0;
		uint32_t PacketID = 0; //Not really needed, just for testing
		std::vector < UDPPacket<T>> Packets;
		std::chrono::high_resolution_clock::time_point LastUpdated;

	};
	uint8_t m_receiveBuffer[MTULimit];
	std::unordered_map<uint16_t, PacketInfo> m_packetMap;

	size_t GetAvailableMessagesCount()
	{
		return m_inMessages.Size();
	}

	bool HasPendingPacketOfId(uint16_t id)
	{
		auto it = m_packetMap.find(id);
		return it != m_packetMap.end();
	}

	void ProcessPacket(asio::ip::udp::endpoint& senderPoint, size_t& bytesRead)
	{
		//Packet info
		UDPPacket<T> packet(m_receiveBuffer, bytesRead);
		auto h = packet.ExtractHeader();
		auto it = m_packetMap.find(h.PacketID);
		if (it == m_packetMap.end())
		{
			//std::cout << "Received first packet of id " << h.PacketID << "\n";
			m_packetMap[h.PacketID].Packets.resize(h.PacketMaxSequenceNumbers);
			m_packetMap[h.PacketID].MessageID = h.MessageID;
			m_packetMap[h.PacketID].PacketID = h.PacketID;
			m_packetMap[h.PacketID].PacketsCount = 1;
		}
		else
			m_packetMap[h.PacketID].PacketsCount++;

		m_packetMap[h.PacketID].Packets[h.PacketSequenceNumber] = packet;
		m_packetMap[h.PacketID].LastUpdated = std::chrono::high_resolution_clock::now();

		if (m_packetMap[h.PacketID].PacketsCount == h.PacketMaxSequenceNumbers)
		{
			OwnedUDPMessage<T> msg;
			msg.TheMessage.SetMessageID(m_packetMap[h.PacketID].MessageID);

			msg.RemoteAddress = senderPoint.address().to_string();
			msg.RemotePort = senderPoint.port();

			//Dump the payload directly in the message
			m_packetAssembler.AssemblePayloadFromPackets(m_packetMap[h.PacketID].Packets, msg.TheMessage.GetPayload());
			m_inMessages.PushBack(std::move(msg));

			m_packetMap.erase(h.PacketID);
		}
	}

private:
	
	asio::io_context m_context;  // The IO context to handle asynchronous IO.
	std::thread m_contextThread;  // The thread that runs the IO context.
	asio::ip::udp::socket m_socket;  // UDP socket for communication.
	asio::ip::udp::endpoint senderPoint;  // Endpoint that stores the sender's information.
	UDPPacketAssembler<T> m_packetAssembler;  // Assembler to reconstruct messages from packets.
	TSQueue<OwnedUDPMessage<T>> m_inMessages;  // Thread-safe queue to store incoming messages.
	uint32_t m_dropMessageThresholdMills = 100;  // Time threshold to drop incomplete messages (in milliseconds).
};