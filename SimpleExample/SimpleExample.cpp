#include <iostream>
#include <chrono>

#ifndef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

std::vector<char> vBuffer(20 * 1024);

void GrabSomeData(asio::ip::tcp::socket& socket)
{
	socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
		[&](std::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				std::cout << "\n\nRead " << length << " bytes\n\n";

				for (int i = 0; i < length; i++)
					std::cout << vBuffer[i];

				GrabSomeData(socket);
			}
		}
	);
}

int main()
{
	asio::error_code ec;


	// create a 'context' - platform specific interface
	asio::io_context context;

	// give some fake tasks to asio so the context doesnt finish
	asio::io_context::work idleWork(context);

	//start the context
	std::thread thrContext = std::thread([&]() {context.run(); });

	//get adress to connect to "example.com"
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);

	// create a socket. context delivers the implementation
	asio::ip::tcp::socket socket(context);

	//socket tries to connect
	socket.connect(endpoint, ec);

	if (!ec)
	{
		std::cout << "Connected!" << std::endl;
	}
	else
	{
		std::cout << "Failed to connect to adress:\n" << ec.message() << std::endl;
	}

	if (socket.is_open())
	{

		GrabSomeData(socket);

		//writing the http request

		std::string sRequest =
			"GET /index.html HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Connecttion: close\r\n\r\n";

		socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);


		// program does smth else, while asio handles data transfer in the background
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(20000ms);

		context.stop();
		if (thrContext.joinable()) thrContext.join();



		//manual read


		//wait to read all data
		//socket.wait(socket.wait_read);


		//size_t bytes = socket.available();
		//std::cout << "Bytes Avaible: " << bytes << std::endl;

		//if (bytes > 0)
		//{
		//	std::vector<char> vBuffer(bytes);
		//	socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);
		//
		//	for (auto c : vBuffer)
		//		std::cout << c;
		//
		//}

	}

	system("pause");
	return 0;
}