#include <asio.hpp>
#include "UDPPacket.h"
#include "UDPPacketAssembler.h"
#include <unordered_map>
template<class T>
class UDPReceiver
{
public:
	UDPReceiver(uint16_t port) :
		m_socket(asio::ip::udp::socket(m_context, asio::ip::udp::v4()))

	{
		try
		{
			m_socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), port));
			m_contextThread = std::thread([this]() {m_context.run(); });
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
			m_contextThread.join();
	}

	void Receive()
	{
		if (m_socket.is_open())
		{
			m_socket.async_receive_from(asio::buffer(m_receiveBuffer, MTULimit), senderPoint,
				[this](std::error_code ec, std::size_t bytesRead) {

					if (!ec)
					{
						if (bytesRead > 0)
						{
							std::cout << "[UDP Receiver]: Got: " << bytesRead << " from  " << senderPoint << "\n";
							//Packet info
							UDPPacket<T> packet(m_receiveBuffer, bytesRead);
							auto h = packet.ExtractHeader();
							
							auto it = m_packetMap.find(h.PacketID);
							if (it == m_packetMap.end())
							{
								m_packetMap[h.PacketID].resize(h.PacketMaxSequenceNumbers);
								m_packetMap[h.PacketID][h.PacketSequenceNumber] = packet;

								m_packetCount[h.PacketID] = 1;
							}
							else
							{
								m_packetCount[h.PacketID]++;
								m_packetMap[h.PacketID][h.PacketSequenceNumber] = packet;
							}

							if (m_packetCount[h.PacketID] == h.PacketMaxSequenceNumbers)
							{
								std::cout << "Received all packets for ID " << h.PacketID << "\n";
								auto msg = m_packetAssembler.AssembleMessageFromPackets(m_packetMap[h.PacketID]);
								//I know it's a string, let's see
								std::string s;
								s.resize(msg.size());
								std::memcpy((void*)s.data(), msg.data(), msg.size());

								std::cout << "Message is: " << s << "\n";
							}
						
						}
						Receive();
					}
					else
					{
						std::cout << "[UDP Receiver]: Failed to Got: " << ec.message() << "\n";

					}

				});
		}
		
		//Sync version
		//auto sent = m_socket.send_to(asio::buffer(data, size), m_endpoint);
		//std::cout << "Sent " << sent << "\n";
	}

private:

	asio::io_context m_context;
	std::thread m_contextThread;
	asio::ip::udp::socket m_socket;
	asio::ip::udp::endpoint senderPoint;
	uint8_t m_receiveBuffer[MTULimit];
	std::unordered_map<uint16_t, std::vector<UDPPacket<T>>> m_packetMap;
	std::unordered_map<uint16_t, uint32_t> m_packetCount;
	UDPPacketAssembler<T> m_packetAssembler;



};