#include "HTTPSNetwork.h"

#include <algorithm>
#include <charconv>

#include "Exceptions/SSLException.h"

using namespace std;

namespace web
{
	HTTPSNetwork::HTTPSNetwork(SOCKET clientSocket, SSL* ssl, SSL_CTX* context) :
		HTTPNetwork(clientSocket),
		ssl(ssl),
		context(context),
		isClientSide(false)
	{
		
	}

	HTTPSNetwork::HTTPSNetwork(const string& ip, const string& port) :
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

		if (!SSL_set_fd(ssl, clientSocket))
		{
			throw exceptions::SSLException();
		}

		if (SSL_connect(ssl) != 1)
		{
			throw exceptions::SSLException();
		}
	}

	int HTTPSNetwork::sendData(const vector<char>& data)
	{
		int lastSend = 0;
		int totalSent = 0;
		int count = data.size();

		do
		{
			lastSend = SSL_write(ssl, data.data() + totalSent, count - totalSent);

			if (lastSend <= 0)
			{
				throw exceptions::SSLException();
			}

			totalSent += lastSend;

		} while (totalSent < count);

		return totalSent;
	}

	int HTTPSNetwork::receiveData(vector<char>& data)
	{
		data.resize(1500);
		int totalSize = 0;
		int lastPacket = 0;
		int bodySize = -1;
		bool isFindEnd = false;

		while (!isFindEnd)
		{
			if (totalSize >= data.size() - 100)
			{
				data.resize(data.size() * 2);
			}

			lastPacket = SSL_read(ssl, data.data() + totalSize, data.size() - totalSize);

			if (lastPacket <= 0)
			{
				throw exceptions::SSLException();
			}

			totalSize += lastPacket;

			string_view findHeader(data.data(), totalSize);
			string_view::const_iterator contentLength = search
			(
				findHeader.begin(), findHeader.end(), contentLengthHeader.begin(), contentLengthHeader.end(),
				[](const char& first, const char& second)
				{
					return tolower(first) == tolower(second);
				}
			);

			if (contentLength == findHeader.end())
			{
				isFindEnd = findHeader.find(crlfcrlf) != string_view::npos;
			}
			else
			{
				size_t endOfHTTP = findHeader.find(crlfcrlf);

				if (bodySize == -1)
				{
					size_t contentLengthHeaderPosition = distance(findHeader.begin(), contentLength) + contentLengthHeader.size() + 2;
					string_view contentLengthValue = findHeader.substr(contentLengthHeaderPosition, findHeader.find("\r\n", distance(findHeader.begin(), contentLength)) - contentLengthHeaderPosition);

					from_chars(contentLengthValue.data(), contentLengthValue.data() + contentLengthValue.size(), bodySize);

					data.resize(data.size() + bodySize);
				}

				if (endOfHTTP != string_view::npos)
				{
					isFindEnd = findHeader.size() == (endOfHTTP + crlfcrlf.size() + bodySize);
				}
			}
		}

		data.resize(totalSize);

		return totalSize;
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
