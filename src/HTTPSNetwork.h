#pragma once

#include <openssl/ssl.h>

#include "HTTPNetwork.h"

namespace web
{
	class NETWORKS_API HTTPSNetwork : public HTTPNetwork
	{
	public:
		static inline constexpr std::string_view httpsPort = "443";

	protected:
		SSL* ssl;
		SSL_CTX* context;
		bool isClientSide;

	protected:
		int sendBytesImplementation(const char* data, int count, int flags = NULL) override;

		int receiveBytesImplementation(char* data, int count, int flags = NULL) override;

	public:
		/// @brief Server side constructor
		/// @param ssl Result from SSL_new
		/// @param context Result from SSL_CTX_new
		/// @exception web::exceptions::SSLException
		HTTPSNetwork(SOCKET clientSocket, SSL* ssl, SSL_CTX* context);

		/// @brief Client side constructor
		/// @param ip Remote address to connect to
		/// @param port Remote port to connect to
		/// @exception web::exceptions::SSLException
		HTTPSNetwork(std::string_view ip, std::string_view port = httpsPort, std::string_view hostName = "");

		virtual ~HTTPSNetwork();
	};
}
