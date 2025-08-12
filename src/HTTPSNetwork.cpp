#include "HTTPSNetwork.h"

#include <algorithm>
#include <charconv>

using namespace std;

namespace web
{
	int HTTPSNetwork::sendBytesImplementation(const char* data, int count, int flags)
	{
		return SSL_write(ssl, data, count);
	}

	int HTTPSNetwork::receiveBytesImplementation(char* data, int count, int flags)
	{
		return SSL_read(ssl, data, count);
	}

	void HTTPSNetwork::throwException(int line, string_view file) const
	{
		throw exceptions::SSLException(line, file);
	}

	HTTPSNetwork::~HTTPSNetwork()
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
