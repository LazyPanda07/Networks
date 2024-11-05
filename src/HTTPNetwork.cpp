#include "HTTPNetwork.h"

#include <algorithm>
#include <charconv>

using namespace std;

static bool insensetiveSearching(char first, char second);

static bool endsWith(string_view test, string_view suffix);

static string_view getHeaderValue(string_view::const_iterator startHeader, size_t headerSize, string_view http);

namespace web
{
	HTTPNetwork::HTTPNetwork(SOCKET clientSocket) :
		Network(clientSocket)
	{

	}

	HTTPNetwork::HTTPNetwork(string_view ip, string_view port, DWORD timeout) :
		Network(ip, port, timeout)
	{

	}

	int HTTPNetwork::sendData(const utility::ContainerWrapper& data, bool& endOfStream)
	{
		try
		{
			return Network::sendBytes(data.data(), static_cast<int>(data.size()), endOfStream);
		}
		catch (const exceptions::WebException& e)
		{
			this->log(e.what());

			throw;
		}
	}

	int HTTPNetwork::receiveData(utility::ContainerWrapper& data, bool& endOfStream)
	{
		int totalSize = 0;
		int lastPacket = 0;
		int bodySize = -1;
		bool isFindEnd = false;
		bool chunked = false;

		endOfStream = false;

		if (data.size() < averageHTTPRequestSize)
		{
			data.resize(averageHTTPRequestSize);
		}

		while (!isFindEnd)
		{
			if (totalSize >= data.size() - thresholdSize)
			{
				data.resize(data.size() * 2);
			}

			lastPacket = this->receiveBytes(data.data() + totalSize, static_cast<int>(data.size()) - totalSize, endOfStream);

			if (lastPacket == SOCKET_ERROR)
			{
				THROW_WEB_EXCEPTION;
			}
			else if (!lastPacket)
			{
				endOfStream = true;

				break;
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

					if (static_cast<size_t>(totalSize) + static_cast<size_t>(bodySize) > data.size())
					{
						data.resize(static_cast<size_t>(totalSize) + static_cast<size_t>(bodySize));

						http = string_view(data.data(), totalSize);
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

			if (bodySize != -1)
			{
				size_t position = http.find(crlfcrlf);

				if (position != string_view::npos)
				{
					isFindEnd = totalSize - position == bodySize + crlfcrlf.size();
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
