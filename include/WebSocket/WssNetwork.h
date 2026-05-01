#pragma once

#include "WsNetwork.h"

#include "Http/HttpsNetwork.h"

namespace web::web_socket
{
	class WssNetwork : public WsNetwork
	{
	private:
		SSL* ssl;
		SSL_CTX* context;

	private:
		int sendBytesImplementation(const char* data, int count, int flags = 0) override;

		int receiveBytesImplementation(char* data, int count, int flags = 0) override;

		void throwException(int line, std::string_view file) const override;

	public:
		/// @brief Server side constructor
		WssNetwork(SOCKET clientSocket, SSL* ssl, SSL_CTX* context);

		/// @brief Client side constructor
		/// @param ip Remote address to connect to
		/// @param port Remote port to connect to
		WssNetwork(std::string_view ip, std::string_view port, std::string_view hostName = "");

		/**
		 * @brief Upgrade HTTPs to WebSocket
		 * @param httpNetwork
		 */
		WssNetwork(http::HttpsNetwork&& httpsNetwork, bool isClient);

		~WssNetwork();
	};
}
