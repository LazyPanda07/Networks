#pragma once

#include "Http/HttpNetwork.h"

#include <IOSocketStream.h>

#include "NetworksUtility.h"
#include "WebSocket/Frame.h"

namespace web::web_socket
{
	/**
	 * @brief WebSocket network
	 */
	class WsNetwork : public Network
	{
	protected:
		bool isClient;

	protected:
		WsNetwork(bool isClient);

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
		WsNetwork(http::HttpNetwork&& httpNetwork, bool isClient) noexcept;

		bool getIsClient() const;

		int sendData(const utility::ContainerWrapper& data, bool& endOfStream, int flags = 0) override;

		int sendRawData(const char* data, int size, bool& endOfStream, int flags = 0) override;

		int receiveData(utility::ContainerWrapper& data, bool& endOfStream, int flags = 0) override;

		int receiveRawData(char* data, int size, bool& endOfStream, int flags = 0) override;

		virtual ~WsNetwork() = default;
	};
}

namespace streams
{
	IOSocketStream& operator >>(IOSocketStream& stream, std::vector<web::web_socket::Frame>& frame);

	IOSocketStream& operator <<(IOSocketStream& stream, const web::web_socket::Frame& frame);
}
