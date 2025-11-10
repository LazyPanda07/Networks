#include "HttpsNetwork.h"

#include <algorithm>

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

	HttpsNetwork::~HttpsNetwork()
	{
		if (isClientSide)
		{
			SSL_CTX_free(context);
		}

		if (ssl)
		{
			SSL_free(ssl);
		}
	}
}
