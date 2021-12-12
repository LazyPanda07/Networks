#include "HTTPNetwork.h"

#include <algorithm>
#include <charconv>

#pragma comment (lib, "HTTP.lib")
#pragma comment (lib, "JSON.lib")
#pragma comment (lib, "SocketStreams.lib")

using namespace std;

static bool insensetiveSearching(char first, char second);

static bool endsWith(string_view test, string_view suffix);

static string_view getHeaderValue(string_view::const_iterator startHeader, size_t headerSize, string_view http);

namespace web
{
	int HTTPNetwork::receiveDataMethod(char* data, int len)
	{
		return recv(clientSocket, data, len, NULL);
	}

	HTTPNetwork::HTTPNetwork(SOCKET clientSocket) :
		Network(clientSocket)
	{

	}

	HTTPNetwork::HTTPNetwork(const string& ip, const string& port) :
		Network(ip, port)
	{

	}

	int HTTPNetwork::sendData(const vector<char>& data)
	{
		try
		{
			return Network::sendBytes(data.data(), static_cast<int>(data.size()));
		}
		catch (const exceptions::WebException&)
		{
			return -1;
		}
	}

	int HTTPNetwork::receiveData(vector<char>& data)
	{
		data.resize(averageHTTPRequestSize);
		int totalSize = 0;
		int lastPacket = 0;
		int bodySize = -1;
		bool isFindEnd = false;
		bool chunked = false;

		while (!isFindEnd)
		{
			if (totalSize >= data.size() - thresholdSize)
			{
				data.resize(data.size() * 2);
			}

			lastPacket = this->receiveDataMethod(data.data() + totalSize, static_cast<int>(data.size()) - totalSize);

			if (lastPacket == SOCKET_ERROR || !lastPacket)
			{
				throw exceptions::WebException();
			}

			totalSize += lastPacket;
			string_view http(data.data(), totalSize);

			if (bodySize == -1 && !chunked)
			{
				string_view::const_iterator contentLength = search
				(
					http.begin(), http.end(),
					contentLengthHeader.begin(), contentLengthHeader.end(),
					insensetiveSearching
				);

				if (contentLength != http.end())
				{
					string_view contentLengthValue = getHeaderValue(contentLength, contentLengthHeader.size(), http);

					from_chars(contentLengthValue.data(), contentLengthValue.data() + contentLengthValue.size(), bodySize);

					if (static_cast<size_t>(totalSize + bodySize) > data.size())
					{
						data.resize(static_cast<size_t>(totalSize + bodySize));
					}
				}
				else
				{
					string_view::const_iterator transferEncoding = search
					(
						http.begin(), http.end(),
						transferEncodingHeader.begin(), transferEncodingHeader.end(),
						insensetiveSearching
					);

					if (transferEncoding != http.end())
					{
						string_view transferEncodingValue = getHeaderValue(transferEncoding, transferEncodingHeader.size(), http);

						chunked = equal
						(
							transferEncodingValue.begin(), transferEncodingValue.end(),
							transferEncodingChunked.begin(), transferEncodingChunked.end(),
							insensetiveSearching
						);
					}
				}
			}

			if (endsWith(http, crlfcrlf))
			{
				if (chunked)
				{
					isFindEnd = endsWith(http, "0"s.append(crlfcrlf));
				}
				else if (bodySize != -1)
				{
					isFindEnd = (bodySize > 0 && http.find(crlfcrlf) != http.rfind(crlfcrlf)) ||
						(!bodySize && http.find(crlfcrlf) != string_view::npos);
				}
				else
				{
					isFindEnd = http.find(crlfcrlf) != string_view::npos;
				}
			}
		}

		data.resize(totalSize);

		return totalSize;
	}
}

bool insensetiveSearching(char first, char second)
{
	return tolower(first) == tolower(second);
}

bool endsWith(string_view test, string_view suffix)
{
	return test.size() >= suffix.size() &&
		test.compare(test.size() - suffix.size(), suffix.size(), suffix) == 0;
}

string_view getHeaderValue(string_view::const_iterator startHeader, size_t headerSize, string_view http)
{
	size_t headerValuePosition = distance(http.begin(), startHeader) + headerSize + web::HTTPNetwork::crlf.size();
	size_t valueSize = http.find(web::HTTPNetwork::crlf, distance(http.begin(), startHeader)) - headerValuePosition;

	return http.substr(headerValuePosition, valueSize);
}
