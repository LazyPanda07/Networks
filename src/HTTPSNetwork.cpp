#include "HTTPSNetwork.h"

#include <algorithm>
#include <charconv>

#include "Exceptions/SSLException.h"

using namespace std;

namespace web
{
	int HTTPSNetwork::receiveDataMethod(char* data, int length)
	{
		return SSL_read(ssl, data, length);
	}

	HTTPSNetwork::HTTPSNetwork(SOCKET clientSocket, SSL* ssl, SSL_CTX* context) :
		HTTPNetwork(clientSocket),
		ssl(ssl),
		context(context),
		isClientSide(false)
	{
		
	}

	HTTPSNetwork::HTTPSNetwork(const string& ip, const string& port, const string& hostName) :
		HTTPNetwork(ip, port),
		isClientSide(true)
	{
		SSL_library_init();
		SSL_load_error_strings();

		context = SSL_CTX_new(TLS_client_method());

		if (!context)
		{
			throw exceptions::SSLException();
		}

		ssl = SSL_new(context);

		if (!ssl)
		{
			throw exceptions::SSLException();
		}

		if (!SSL_set_fd(ssl, static_cast<int>(clientSocket)))
		{
			throw exceptions::SSLException();
		}

		SSL_set_tlsext_host_name(ssl, (hostName.empty() ? ip.data() : hostName.data()));

		if (SSL_connect(ssl) != 1)
		{
			throw exceptions::SSLException();
		}
	}

	int HTTPSNetwork::sendData(const vector<char>& data)
	{
		int lastSend = 0;
		int totalSent = 0;
		int count = static_cast<int>(data.size());

		do
		{
			lastSend = SSL_write(ssl, data.data() + totalSent, count - totalSent);

			if (lastSend == SOCKET_ERROR)
			{
				THROW_WEB_EXCEPTION
			}
			else if (!lastSend)
			{
				break;
			}

			totalSent += lastSend;

		} while (totalSent < count);

		return totalSent;
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
