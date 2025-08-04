#include "HTTPSNetwork.h"

#include <algorithm>
#include <charconv>

#include "Exceptions/SSLException.h"

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

	HTTPSNetwork::HTTPSNetwork(SOCKET clientSocket, SSL* ssl, SSL_CTX* context) :
		HTTPNetwork(clientSocket),
		ssl(ssl),
		context(context),
		isClientSide(false)
	{

	}

	HTTPSNetwork::HTTPSNetwork(string_view ip, string_view port, DWORD timeout, string_view hostName) :
		HTTPNetwork(ip, port, timeout),
		isClientSide(true)
	{
		SSL_library_init();
		SSL_load_error_strings();

		context = SSL_CTX_new(TLS_client_method());

		if (!context)
		{
			throw exceptions::SSLException(__LINE__, __FILE__);
		}

		ssl = SSL_new(context);

		if (!ssl)
		{
			throw exceptions::SSLException(__LINE__, __FILE__);
		}

		if (!SSL_set_fd(ssl, static_cast<int>(*clientSocket)))
		{
			throw exceptions::SSLException(__LINE__, __FILE__);
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
				throw exceptions::SSLException(__LINE__, __FILE__, ssl, returnCode);
			}
			else if (returnCode == -1)
			{
				int errorCode = SSL_get_error(ssl, returnCode);

				if (errorCode == SSL_ERROR_WANT_WRITE || errorCode == SSL_ERROR_WANT_READ)
				{
					continue;
				}

				throw exceptions::SSLException(__LINE__, __FILE__, returnCode, errorCode);
			}
		}
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
