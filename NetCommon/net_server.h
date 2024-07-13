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
			{

			}

			virtual ~server_interface()
			{

			}
			
			bool Start()
			{

			}

			bool Stop()
			{

			}

			//ASYNC  - Instruct asio to wait for connection
			void WaitForClientConnection()
			{

			}

			//Send a message to a  specific client
			void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
			{

			}

			//Send message to all clients
			void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr)
			{

			}

		protected:
			//Called when a client connects, you can veto the connection by returning false
			virtual bool OnClientConnect(std::shared_ptr < connection<connection<T>> client)
			{
				return false;
			}

			// called when a client appears to have disconnected
			virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client)

			

		};
	}
}