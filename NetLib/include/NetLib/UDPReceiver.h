#include <asio.hpp>

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
			std::cout << "Error creating UDP sender: " << e.what();
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
			uint8_t buf[12];

			m_socket.async_receive_from(asio::buffer(buf, 12), senderPoint,
				[this](std::error_code ec, std::size_t bytesRead) {

					if (!ec)
					{
						std::cout << "[UDP Sender]: Got: " << bytesRead << " from  " << senderPoint << "\n";

					}
					else
					{
						std::cout << "[UDP Sender]: Failed to Got: " << ec.message() << "\n";

					}
					Receive();

				});


		}
		

		//auto sent = m_socket.send_to(asio::buffer(data, size), m_endpoint);
		//std::cout << "Sent " << sent << "\n";
	}

private:

	asio::io_context m_context;
	std::thread m_contextThread;
	asio::ip::udp::socket m_socket;
	asio::ip::udp::endpoint senderPoint;

};