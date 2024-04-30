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

	HTTPSNetwork::HTTPSNetwork(string_view ip, string_view port, string_view hostName) :
		HTTPNetwork(ip, port),
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

		if (!SSL_set_fd(ssl, static_cast<int>(clientSocket)))
		{
			throw exceptions::SSLException(__LINE__, __FILE__);
		}

		SSL_set_tlsext_host_name(ssl, (hostName.empty() ? ip.data() : hostName.data()));

		if (int errorCode = SSL_connect(ssl); errorCode != 1)
		{
			throw exceptions::SSLException(__LINE__, __FILE__, errorCode);
		}
	}

	HTTPSNetwork::~HTTPSNetwork()
	{
		if (isClientSide)
		{
			SSL_CTX_free(context);
		}

		SSL_free(ssl);
	}
}
