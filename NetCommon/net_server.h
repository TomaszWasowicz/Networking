#pragma once

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_connection.h"

namespace olc
{
	namespace net
	{

		template<typename T>
		class server_interface
		{
		public:
			server_interface(uint16_t port)
				: m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
			{

			}

			virtual ~server_interface()
			{
				Stop();
			}

			bool Start()
			{
				try
				{
					WaitForClientConnection();

					//launch the context in its own thread
					m_threadContext = std::thread([this]() { m_asioContext.run(); });
				}
				catch (std::exception& e)
				{
					//Something prohibited the server from listening
					std::cerr << "[SERVER] Exception: " << e.what() << "\n";
					return false;
				}

				std::cout << "[SERVER] Started!\n";
				return true;
			}

			void Stop()
			{
				//request the context to close
				m_asioContext.stop();

				//Tidy up the context thread
				if (m_threadContext.joinable()) m_threadContext.join();

				//Hello, is it me you are looking for ?
				std::cout << "[SERVER] Stopped!\n";
			}

			//ASYNC  - Instruct asio to wait for connection
			void WaitForClientConnection()
			{
				m m_asioAcceptor.async_accept(
					[this](std::error_code ec, asio::ip::tcp::socket socket)
					{
						if (!ec)
						{
							std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";

							std::shared_ptr<connection<T>> newconn =
								std::make_shared<connection<T>>(connection<T>::owner::server,
									m_asioContext, std::move(socket), m_qMessagesIn);

							// give the user server a chance to deny connection
							if (OnClientConnect(newconn))
							{
								//Connection allowed, so add to container of new connections
								m_deqConnections.push_back(std::move(newconn));

								m_deqConnections.back()->ConnectToClient(nIDCounter++);

								std::cout << "[" << m_deqConnections.back()->GetID() << "] Connection Approved\n";
							}
							else
							{
								std::cout << "[-----] Connection Denied\n";
							}
						}
						else
						{
							//error has occured during acceptance
							std::cout << "[SERVER] New Connection Error: " << ec.message() << '\n';
						}

						//prime the asio context with more work - again simply wait for
						// another connection ...
						WaitForClientConnection();
					});
			}

			//Send a message to a  specific client
			void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
			{
				if (client && client->IsConnected())
				{
					client->Send(msg);
				}
				else
				{
					OnClientDisconnect(client);
					client.reset();
					m_deqConnections.erase(
						std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());
				}
			}

			//Send message to all clients
			void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr)
			{
				bool bInvalidClientExists = false;

				for (auto& client : m_deqConnections)
				{
					if (client && client->IsConnected())
					{
						// ... it is!
						if (client != pIgnoreClient)
							client->Send(msg);
					}
					else
					{
						//the client could not be contacted, so assume it has disconnected
						OnClientDisconnect(client);
						client.reset();
						bInvalidClientExists = true;
					}
				}

				if (bInvalidClientExists)
					m_deqConnections.erase(
						std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());

			}

		protected:
			//Called when a client connects, you can veto the connection by returning false
			virtual bool OnClientConnect(std::shared_ptr<connection<T>> client)
			{
				return false;
			}

			// called when a client appears to have disconnected
			virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client)
			{

			}

			// called when a message arrives
			virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg)
			{

			}

		protected:
			//thread safe queue for incoming message packets
			tsqueue<owned_message<T>> m_qMessagesIn;

			//Container of active validated connections
			std::deque<std::shared_ptr<connection<T>>> m_deqConnections;

			//Order of declaration is important - it is also the order of initialisation
			asio::io_context m_asioContext;
			std::thread m_threadContext;

			// these things need an asio context
			asio::ip::tcp::acceptor m_asioAcceptor;

			// client will be identified in the "wider system" via an ID
			uint32_t nIDCounter = 10000;

			

		};
	}
}