#include "HTTPNetwork.h"

#include <algorithm>
#include <charconv>

#pragma comment (lib, "HTTP.lib")
#pragma comment (lib, "JSON.lib")
#pragma comment (lib, "SocketStreams.lib")

using namespace std;

namespace web
{
	const string HTTPNetwork::contentLengthHeader = "Content-Length";
	const string HTTPNetwork::crlfcrlf = "\r\n\r\n";

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

		while (!isFindEnd)
		{
			if (totalSize >= data.size() - 100)
			{
				data.resize(data.size() * 2);
			}

			lastPacket = recv(Network::clientSocket, data.data() + totalSize, static_cast<int>(data.size()) - totalSize, NULL);

			if (lastPacket == SOCKET_ERROR || !lastPacket)
			{
				throw exceptions::WebException();
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
}