#include "HTTPSNetwork.h"

#include <algorithm>
#include <charconv>

#include "Exceptions/SSLException.h"

using namespace std;

namespace web
{
	HTTPSNetwork::HTTPSNetwork(SOCKET clientSocket) :
		HTTPNetwork(clientSocket),
		ssl(nullptr),
		context(nullptr)
	{
		SSL_library_init();
		SSL_load_error_strings();


	}

	HTTPSNetwork::HTTPSNetwork(const string& ip, const string& port) :
		HTTPNetwork(ip, port)
	{
		SSL_library_init();
		SSL_load_error_strings();

		context = SSL_CTX_new(TLS_client_method());

		ssl = SSL_new(context);

		SSL_set_fd(ssl, clientSocket);

		SSL_connect(ssl);
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
		SSL_CTX_free(context);
	}
}
