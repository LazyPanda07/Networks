#pragma once

#include "NetworksUtility.h"
#include "HttpNetwork.h"

namespace web
{
	/**
	 * @brief WebSocket network
	 */
	class NETWORKS_API WsNetwork : public Network
	{
	private:
		bool isClient;

	public:
		/// @brief Server side constructor
		WsNetwork(SOCKET clientSocket);

		/// @brief Client side constructor
		/// @param ip Remote address to connect to
		/// @param port Remote port to connect to
		WsNetwork(std::string_view ip, std::string_view port);

		/**
		 * @brief Upgrade HTTP to WebSocket
		 * @param httpNetwork 
		 */
		WsNetwork(HttpNetwork&& httpNetwork, bool isClient) noexcept;

		int sendData(const utility::ContainerWrapper& data, bool& endOfStream, int flags = 0) override;

		int sendRawData(const char* data, int size, bool& endOfStream, int flags = 0) override;

		int receiveData(utility::ContainerWrapper& data, bool& endOfStream, int flags = 0) override;

		int receiveRawData(char* data, int size, bool& endOfStream, int flags = 0) override;

		virtual ~WsNetwork() = default;
	};
}
