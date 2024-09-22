#include <iostream>
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include "NetLib/Common.h"

void main()
{
	asio::error_code ec;
	asio::io_context context;
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("127.0.0.1", ec), 80);

	asio::ip::tcp::socket socket(context);

	socket.connect(endpoint, ec);

	if (!ec)
	{
		std::cout << "connected!" << std::endl;
	}
	else
	{
		std::cout << "Failed: " << ec.message() << std::endl;
	}

	if (socket.is_open())
	{

	}

	return;
}