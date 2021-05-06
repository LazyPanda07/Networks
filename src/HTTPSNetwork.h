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
		HTTPSNetwork(SOCKET clientSocket);

		HTTPSNetwork(const std::string& ip, const std::string& port);

		int sendData(const std::vector<char>& data) override;

		int receiveData(std::vector<char>& data) override;

		virtual ~HTTPSNetwork();
	};
}
