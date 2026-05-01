#include "WebSocket/WssNetwork.h"

#include "Exceptions/SslException.h"

namespace web::web_socket
{
	int WssNetwork::sendBytesImplementation(const char* data, int count, int flags)
	{
		return SSL_write(ssl, data, count);
	}

	int WssNetwork::receiveBytesImplementation(char* data, int count, int flags)
	{
		return SSL_read(ssl, data, count);
	}

	void WssNetwork::throwException(int line, std::string_view file) const
	{
		throw exceptions::SslException(line, file);
	}

	WssNetwork::WssNetwork(SOCKET clientSocket, SSL* ssl, SSL_CTX* context) :
		WsNetwork(clientSocket),
		ssl(ssl),
		context(context)
	{
		isClient = false;
	}

	WssNetwork::WssNetwork(std::string_view ip, std::string_view port, std::string_view hostName) :
		WsNetwork(ip, port)
	{
		isClient = true;

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
			SSL_CTX_free(context);

			throw exceptions::SslException(__LINE__, __FILE__);
		}

		if (!SSL_set_fd(ssl, static_cast<int>(this->getClientSocket())))
		{
			SSL_CTX_free(context);

			SSL_free(ssl);

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
				exceptions::SslException exception(__LINE__, __FILE__, ssl, returnCode);

				SSL_CTX_free(context);

				throw exception;
			}
			else if (returnCode == -1)
			{
				int errorCode = SSL_get_error(ssl, returnCode);

				if (errorCode == SSL_ERROR_WANT_WRITE || errorCode == SSL_ERROR_WANT_READ)
				{
					continue;
				}

				exceptions::SslException exception(__LINE__, __FILE__, returnCode, errorCode);

				SSL_CTX_free(context);

				throw exception;
			}
		}
	}

	WssNetwork::WssNetwork(http::HttpsNetwork&& httpsNetwork, bool isClient) :
		WsNetwork(isClient)
	{
		clientSocket = httpsNetwork.clientSocket;
		buffers = std::move(httpsNetwork.buffers);

		ssl = httpsNetwork.ssl;
		context = httpsNetwork.context;

		httpsNetwork.clientSocket = INVALID_SOCKET;

		httpsNetwork.ssl = nullptr;
		httpsNetwork.context = nullptr;
	}

	WssNetwork::~WssNetwork()
	{
		if (isClient && context)
		{
			SSL_CTX_free(context);
		}

		if (ssl)
		{
			SSL_free(ssl);
		}
	}
}
