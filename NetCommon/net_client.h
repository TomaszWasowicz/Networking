#pragma once
#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"
#include "net_connection.h"

namespace olc
{
	namespace net
	{
		template<typename T>
		class client_interface
		{
			client_interface()
			{
				//initilize the socket with the io context, os it can do stuff
			}

			virtual ~client_interface()
			{
				//if the client is destroyed, always try and disconnect from server
				Disconnect();
			}

		public:
			//connect to the server with hostname/ip-adress and port
			bool Connect(const std::string& host, const uint16_t port)
			{
				try
				{
					//resolve hostname/ip-adress into tangible physical address
					asio::ip::tcp::resolver resolver(m_context);
					asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

					// Create connection
					m_connection = std::make_unique<connection<T>>(
						connection<T>::owner::client,
						m_context,
						asio::ip::tcp::socket(m_context), m_qMessagesIn);

					// tell the connection object to connect to the server
					m_connection->ConnectToServer(endpoints);

					//start context thread
					thrContext = std::thread([this]() { m_context.run(); });
				}
				catch (std::exception& e)
				{
					std::cerr << "Client Exception: " << e.what() << "\n";
					return false;
				}

				return false;
			}

			//Disconnect from server
			void Disconnect()
			{
				//if connection exists and it's connected then ...
				if (IsConnected())
				{
					// ...disconnect from the server gracefully
					m_connection->Disconnect();
				}

				//either way, we're also done with the asio context ...
				m_context.stop();
				// .. and it's thread
				if (thrContext.joinable())
					thrContext.join();

				//destroy the connecttion object
				m_connection.release();
			}

			//check if the client is actually connected to a server
			bool IsConnected()
			{
				if (m_connection)
					return m_connection->IsConnected();
				else
					return false;
			}

			//retrieve queue of messages from the server
			tsqueue<owned_message<T>>& Incoming()
			{
				return m_qMessagesIn;
			}

		protected:
			//asio conext handles the data transfer ...
			asio::io_context m_context;
			// ... but needs a thread of its own to execute its work commands
			std::thread thrContext;
			// this is the hardware socket that is connected to the server
			asio::ip::tcp::socket m_socket;
			// the client has a single instance of a "connection" object, which handles data transfer
			std::unique_ptr<connection<T>> m_connection;


		private:
			//this is the thread safe queue of incoming messages from server
			tsqueue<owned_message<T>> m_qMessagesIn;
		};
	}
}