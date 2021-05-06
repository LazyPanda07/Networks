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
	protected:
		SSL* ssl;
		SSL_CTX* context;

	public:
		/// @brief Server side constructor
		HTTPSNetwork(SOCKET clientSocket);

		/// @brief Client side constructor
		/// @param ip Server address
		/// @param port Server listen socket port
		HTTPSNetwork(const std::string& ip, const std::string& port);

		/// @brief Send function for streams::BaseIOSocketStream
		/// @return Total number of bytes sended
		/// @exception web::exceptions::SSLException
		int sendData(const std::vector<char>& data) override;

		/// @brief Specific HTTP receive data function
		/// @return Total number of bytes received
		/// @exception web::exceptions::SSLException
		int receiveData(std::vector<char>& data) override;

		virtual ~HTTPSNetwork();
	};
}
