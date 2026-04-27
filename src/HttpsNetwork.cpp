#include "HttpsNetwork.h"

namespace web
{
	int HttpsNetwork::sendBytesImplementation(const char* data, int count, int flags)
	{
		return SSL_write(ssl, data, count);
	}

	int HttpsNetwork::receiveBytesImplementation(char* data, int count, int flags)
	{
		return SSL_read(ssl, data, count);
	}

	void HttpsNetwork::throwException(int line, std::string_view file) const
	{
		throw exceptions::SslException(line, file);
	}

	HttpsNetwork::HttpsNetwork(HttpsNetwork&& other) noexcept
	{
		(*this) = std::move(other);
	}

	HttpsNetwork& HttpsNetwork::operator =(HttpsNetwork&& other) noexcept
	{
		clientSocket = other.clientSocket;
		buffers = std::move(other.buffers);

		largeBodyHandler = std::move(other.largeBodyHandler);
		largeBodySizeThreshold = other.largeBodySizeThreshold;

		ssl = other.ssl;
		context = other.context;
		isClientSide = other.isClientSide;

		other.clientSocket = INVALID_SOCKET;
		other.ssl = nullptr;
		other.context = nullptr;

		return *this;
	}

	HttpsNetwork::~HttpsNetwork()
	{
		if (isClientSide && context)
		{
			SSL_CTX_free(context);
		}

		if (ssl)
		{
			SSL_free(ssl);
		}
	}
}
