#include <asio.hpp>
#include "UDPPacket.h"
#include "UDPPacketAssembler.h"
#include "TSQueue.h"

template<class T>
class UDPSender
{
public:
	UDPPacket<T> m_packet;

	UDPSender(const std::string& sendToAddress, uint16_t port) :
		m_socket(asio::ip::udp::socket(m_context, asio::ip::udp::v4())),
		m_work_guard(asio::make_work_guard(m_context))

	{
		try
		{
			asio::ip::udp::resolver res(m_context);
			m_endpoint = *res.resolve(asio::ip::udp::v4(), sendToAddress, std::to_string(port)).begin();
			m_contextThread = std::thread([this]() {
				std::cout << "Start context\n";
				m_context.run(); 
				std::cout << "Conext stopped in thread\n";

				});
		}
		catch (std::exception& e)
		{
			std::cout << "Error creating UDP sender: " << e.what();
		}
		
	}

	~UDPSender()
	{
		Destroy();
	}

	void Send(const NetMessage<T>& msg)
	{
		std::cout << "Stopped " << m_context.stopped() << std::endl;
		try
		{
			asio::post(m_context, [this, msg]() {

				bool isWriting = !m_outMessages.Empty();
				m_outMessages.PushBack(msg);

				if (!isWriting)
					WriteSocket();
				});
		}
		catch (std::exception& e)
		{
			std::cout << "Somethow failed to send " << e.what();
		}
		

	}

private:
	void Destroy()
	{
		std::cout << "Destroy udp sender " << "\n";

		asio::post(m_context, [this]() {
			m_socket.close();
		
			});
		if (m_contextThread.joinable())
			m_contextThread.join();
	}

	void WriteSocket()
	{
		if (m_outMessages.Empty()) return;

		auto& msg = m_outMessages.PopFront();
		m_packets = m_packetAssembler.CreatePackets(msg);
		auto packetCounts = m_packets.size();
		std::cout << "Writing " << packetCounts << " packets" << "\n";

		for (auto& p : m_packets)
		{
			m_socket.async_send_to(asio::buffer(p.DataBuffer, p.DataBuffer.size()), m_endpoint,
				[this, packetCounts](std::error_code ec, std::size_t bytes_sent) mutable {

					if (!ec)
					{
						std::cout << "[UDP Sender]: Sent: " << bytes_sent << "\n";
						packetCounts--;
						if (packetCounts == 0)
						{
							WriteSocket();
						}
					}
					else
					{
						std::cout << "[UDP Sender]: Failed to send: " << ec.message() << "\n";
						Destroy();
					}
				});
		}

		
	}
	asio::io_context m_context;
	std::thread m_contextThread;
	asio::ip::udp::socket m_socket;
	asio::ip::udp::endpoint m_endpoint;
	UDPPacketAssembler<T> m_packetAssembler;
	uint16_t m_packetIDCounter = 0;
	TSQueue<NetMessage<T>> m_outMessages;
	asio::executor_work_guard<asio::io_context::executor_type> m_work_guard;

	std::vector<UDPPacket<T>> m_packets; //Local cache 


};