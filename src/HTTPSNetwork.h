#pragma once

#include <openssl/ssl.h>

#include "HTTPNetwork.h"

#ifdef NETWORKS_DLL
#define NETWORKS_API __declspec(dllexport)
#else
#define NETWORKS_API
#endif // NETWORKS_DLL

namespace web
{
	class NETWORKS_API HTTPSNetwork : public HTTPNetwork
	{
	public:
		static inline const std::string httpsPort = "443";

	protected:
		SSL* ssl;
		SSL_CTX* context;
		bool isClientSide;

	protected:
		virtual int receiveDataMethod(char* data, int length) override;

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
		HTTPSNetwork(const std::string& ip, const std::string& port = httpsPort, const std::string& hostName = "");

		/// @brief Send function for streams::BaseIOSocketStream
		/// @return Total number of bytes sended
		/// @exception web::exceptions::SSLException
		int sendData(const std::vector<char>& data) override;

		virtual ~HTTPSNetwork();
	};
}
