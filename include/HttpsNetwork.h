#pragma once

#include "HttpNetwork.h"

#include <openssl/ssl.h>

#include "Exceptions/SslException.h"

namespace web
{
	class NETWORKS_API HttpsNetwork : public HttpNetwork
	{
	public:
		static inline constexpr std::string_view httpsPort = "443";

	protected:
		SSL* ssl;
		SSL_CTX* context;
		bool isClientSide;

	protected:
		int sendBytesImplementation(const char* data, int count, int flags = 0) override;

		int receiveBytesImplementation(char* data, int count, int flags = 0) override;

		void throwException(int line, std::string_view file) const override;

	public:
		/// @brief Server side constructor
		/// @param ssl Result from SSL_new
		/// @param context Result from SSL_CTX_new
		/// @exception web::exceptions::SSLException
		template<Timeout T = std::chrono::seconds>
		HttpsNetwork(SOCKET clientSocket, SSL* ssl, SSL_CTX* context, T timeout = 30s);

		/// @brief Client side constructor
		/// @param ip Remote address to connect to
		/// @param port Remote port to connect to
		/// @exception web::exceptions::SSLException
		template<Timeout T = std::chrono::seconds>
		HttpsNetwork(std::string_view ip, std::string_view port = httpsPort, T timeout = 30s, std::string_view hostName = "");

		virtual ~HttpsNetwork();
	};
}

namespace web
{
	template<Timeout T>
	HttpsNetwork::HttpsNetwork(SOCKET clientSocket, SSL* ssl, SSL_CTX* context, T timeout) :
		HttpNetwork(clientSocket, timeout),
		ssl(ssl),
		context(context),
		isClientSide(false)
	{

	}

	template<Timeout T>
	HttpsNetwork::HttpsNetwork(std::string_view ip, std::string_view port, T timeout, std::string_view hostName) :
		HttpNetwork(ip, port, timeout),
		isClientSide(true)
	{
		SSL_library_init();
		SSL_load_error_strings();

		context = SSL_CTX_new(TLS_client_method());

		if (!context)
		{
			throw exceptions::SslException(__LINE__, __FILE__);
		}

		ssl = SSL_new(context);

		if (!ssl)
		{
			throw exceptions::SslException(__LINE__, __FILE__);
		}

		if (!SSL_set_fd(ssl, static_cast<int>(this->getClientSocket())))
		{
			throw exceptions::SslException(__LINE__, __FILE__);
		}

		SSL_set_tlsext_host_name(ssl, (hostName.empty() ? ip.data() : hostName.data()));

		while (true)
		{
			int returnCode = SSL_connect(ssl);

			if (returnCode == 1)
			{
				break;
			}
			else if (returnCode == 0)
			{
				throw exceptions::SslException(__LINE__, __FILE__, ssl, returnCode);
			}
			else if (returnCode == -1)
			{
				int errorCode = SSL_get_error(ssl, returnCode);

				if (errorCode == SSL_ERROR_WANT_WRITE || errorCode == SSL_ERROR_WANT_READ)
				{
					continue;
				}

				throw exceptions::SslException(__LINE__, __FILE__, returnCode, errorCode);
			}
		}
	}
}
