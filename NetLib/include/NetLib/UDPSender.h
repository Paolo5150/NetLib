#include <asio.hpp>
#include "UDPPacket.h"
#include "UDPPacketAssembler.h"
template<class T>
class UDPSender
{
public:
	UDPPacket<T> m_packet;

	UDPSender(const std::string& sendToAddress, uint16_t port) :
		m_socket(asio::ip::udp::socket(m_context, asio::ip::udp::v4()))

	{
		try
		{
			asio::ip::udp::resolver res(m_context);
			m_endpoint = *res.resolve(asio::ip::udp::v4(), sendToAddress, std::to_string(port)).begin();
			m_contextThread = std::thread([this]() {m_context.run(); });
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

	void Send(T id, const std::vector<uint8_t>& data)
	{
		Send(data.data(), data.size());
	}


	void Send(T id, uint8_t* data, uint32_t size)
	{
		auto packets = m_packetAssembler.CreatePackets(id, data, size);

		for (auto& p : packets)
		{
			m_socket.async_send_to(asio::buffer(p.DataBuffer, p.DataBuffer.size()), m_endpoint,
				[this](std::error_code ec, std::size_t bytes_sent) {

					if (!ec)
					{
						std::cout << "[UDP Sender]: Sent: " << bytes_sent << "\n";
					}
					else
					{
						std::cout << "[UDP Sender]: Failed to send: " << ec.message() << "\n";
						Destroy();
					}
				});
		}

		//auto sent = m_socket.send_to(asio::buffer(data, size), m_endpoint);
		//std::cout << "Sent " << sent << "\n";
	}

private:
	void Destroy()
	{
		asio::post(m_context, [this]() {
			m_socket.close();
		
			});
		if (m_contextThread.joinable())
			m_contextThread.join();
	}
	asio::io_context m_context;
	std::thread m_contextThread;
	asio::ip::udp::socket m_socket;
	asio::ip::udp::endpoint m_endpoint;
	UDPPacketAssembler<T> m_packetAssembler;
	uint16_t m_packetIDCounter = 0;

};