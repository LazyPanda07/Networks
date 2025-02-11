#include "HTTPNetwork.h"

#include <algorithm>
#include <charconv>

using namespace std;

static bool insensetiveSearching(char first, char second);

static bool endsWith(string_view test, string_view suffix);

static string_view getHeaderValue(string_view::const_iterator startHeader, size_t headerSize, string_view http);

class ReadOnlyContainerWrapper : public web::utility::ContainerWrapper
{
public:
	ReadOnlyContainerWrapper(char* data, int size);

	~ReadOnlyContainerWrapper() = default;
};

namespace web
{
	HTTPNetwork::HTTPNetwork(SOCKET clientSocket, uint64_t largeBodySizeThreshold) :
		Network(clientSocket),
		largeBodySizeThreshold(largeBodySizeThreshold ? largeBodySizeThreshold : HTTPNetwork::defaultLargeBodySize),
		largeBodyPacketSize(largeBodySizeThreshold ? largeBodySizeThreshold : HTTPNetwork::defaultLargeBodySize / 64)
	{

	}

	HTTPNetwork::HTTPNetwork(string_view ip, string_view port, DWORD timeout, uint64_t largeBodySizeThreshold) :
		Network(ip, port, timeout),
		largeBodySizeThreshold(largeBodySizeThreshold ? largeBodySizeThreshold : HTTPNetwork::defaultLargeBodySize),
		largeBodyPacketSize(largeBodySizeThreshold ? largeBodySizeThreshold : HTTPNetwork::defaultLargeBodySize / 64)
	{

	}

	void HTTPNetwork::setLargeBodyHandler(const std::function<bool(std::string_view)>& largeBodyHandler, const std::function<void(utility::ContainerWrapper&)>& headersHandler, int64_t largeBodyPacketSize)
	{
		this->largeBodyHandler = largeBodyHandler;
		this->headersHandler = headersHandler;

		if (largeBodyPacketSize != -1)
		{
			this->largeBodyPacketSize = largeBodyPacketSize;
		}
	}

	int HTTPNetwork::sendData(const utility::ContainerWrapper& data, bool& endOfStream, int flags)
	{
		return this->sendRawData(data.data(), static_cast<int>(data.size()), endOfStream, flags);
	}

	int HTTPNetwork::sendRawData(const char* data, int size, bool& endOfStream, int flags)
	{
		try
		{
			return Network::sendBytes(data, size, endOfStream, flags);
		}
		catch (const exceptions::WebException& e)
		{
			this->log(e.what());

			throw;
		}
	}

	int HTTPNetwork::receiveData(utility::ContainerWrapper& data, bool& endOfStream, int flags)
	{
		int totalSize = 0;
		int lastPacket = 0;
		int64_t bodySize = -1;
		string largeBodyData;
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

			lastPacket = this->receiveBytes(data.data() + totalSize, static_cast<int>(data.size()) - totalSize, endOfStream, flags);

			if (endOfStream)
			{
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

					if (largeBodyHandler && static_cast<uint64_t>(bodySize) >= largeBodySizeThreshold)
					{
						largeBodyData.resize(largeBodyPacketSize);
					}
					else if (static_cast<size_t>(totalSize) + static_cast<size_t>(bodySize) > data.size())
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
					position += crlfcrlf.size();

					if (largeBodyData.size())
					{
						string bodyData(data.data() + position, totalSize - position);

						memset(data.data() + position, 0, bodyData.size());

						headersHandler(data);
						largeBodyHandler(bodyData);

						bodySize -= bodyData.size();
						totalSize -= static_cast<int>(bodyData.size());

						isFindEnd = true;
					}
					else
					{
						isFindEnd = totalSize - position == bodySize;
					}
				}
			}
			else if (endsWith(http, crlfcrlf))
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

		if (largeBodyData.size())
		{
			while (bodySize)
			{
				lastPacket = this->receiveBytes(largeBodyData.data(), (std::min)(static_cast<int>(largeBodyData.size()), static_cast<int>(bodySize)), endOfStream, flags);

				if (endOfStream)
				{
					break;
				}

				bodySize -= lastPacket;

				largeBodyHandler(string_view(largeBodyData.data(), lastPacket));
			}
		}

		data.resize(totalSize);

		return totalSize;
	}

	int HTTPNetwork::receiveRawData(char* data, int size, bool& endOfStream, int flags)
	{
		ReadOnlyContainerWrapper wrapper(data, size);

		return this->receiveData(wrapper, endOfStream, flags);
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

ReadOnlyContainerWrapper::ReadOnlyContainerWrapper(char* data, int size) :
	ContainerWrapper
	(
		[data]() -> char*
		{
			return data;
		},
		[data]() -> const char*
		{
			return data;
		},
		[size]() -> size_t
		{
			return size;
		},
		[size](size_t newSize)
		{
			if (newSize > size)
			{
				throw runtime_error("Can't resize ReadOnlyContainerWrapper");
			}
		},
		[data, size](size_t index) -> char&
		{
			return data[index];
		}
	)
{

}
