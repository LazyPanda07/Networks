#include "HttpNetwork.h"

#include <algorithm>
#include <charconv>
#include <cstring>

static bool insensetiveSearching(char first, char second);

static bool endsWith(std::string_view test, std::string_view suffix);

static std::string_view getHeaderValue(std::string_view::const_iterator startHeader, size_t headerSize, std::string_view http);

class ReadOnlyContainerWrapper : public web::utility::ContainerWrapper
{
public:
	ReadOnlyContainerWrapper(char* data, int size);

	~ReadOnlyContainerWrapper() = default;
};

namespace web
{
	void HttpNetwork::setLargeBodySizeThreshold(size_t largeBodySizeThreshold)
	{
		this->largeBodySizeThreshold = largeBodySizeThreshold;
	}

	int HttpNetwork::sendData(const utility::ContainerWrapper& data, bool& endOfStream, int flags)
	{
		return this->sendRawData(data.data(), static_cast<int>(data.size()), endOfStream, flags);
	}

	int HttpNetwork::sendRawData(const char* data, int size, bool& endOfStream, int flags)
	{
		return Network::sendBytes(data, size, endOfStream, flags);
	}

	int HttpNetwork::receiveData(utility::ContainerWrapper& data, bool& endOfStream, int flags)
	{
		int totalSize = 0;
		int lastPacket = 0;
		int64_t bodySize = -1;
		bool isFindEnd = false;
		bool chunked = false;
		bool largeBodyDetected = false;

		endOfStream = false;

		if (data.size() < averageHTTPRequestSize)
		{
			data.resize(averageHTTPRequestSize);
		}

		if (largeBodyHandler)
		{
			LargeBodyHandler::WaitBehavior behavior = largeBodyHandler->getWaitBehavior();

			switch (behavior)
			{
			case web::LargeBodyHandler::WaitBehavior::wait:
				while (largeBodyHandler->isRunning())
				{
					std::this_thread::sleep_for(1s);
				}

				break;

			case web::LargeBodyHandler::WaitBehavior::exit:
				if (largeBodyHandler->isRunning())
				{
					return 0;
				}

				break;

			default:
				return 0;
			}
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
			std::string_view http(data.data(), totalSize);

			if (bodySize == -1 && !chunked)
			{
				std::string_view::const_iterator contentLength = std::search
				(
					http.begin(), http.end(),
					contentLengthHeader.begin(), contentLengthHeader.end(),
					insensetiveSearching
				);

				if (contentLength != http.end())
				{
					std::string_view contentLengthValue = getHeaderValue(contentLength, contentLengthHeader.size(), http);

					std::from_chars(contentLengthValue.data(), contentLengthValue.data() + contentLengthValue.size(), bodySize);

					if (largeBodyHandler && bodySize >= static_cast<int64_t>(largeBodySizeThreshold))
					{
						largeBodyDetected = true;
					}
					else if (static_cast<size_t>(totalSize) + static_cast<size_t>(bodySize) > data.size())
					{
						data.resize(static_cast<size_t>(totalSize) + static_cast<size_t>(bodySize));

						http = std::string_view(data.data(), totalSize);
					}
				}
				else
				{
					std::string_view::const_iterator transferEncoding = std::search
					(
						http.begin(), http.end(),
						transferEncodingHeader.begin(), transferEncodingHeader.end(),
						insensetiveSearching
					);

					if (transferEncoding != http.end())
					{
						std::string_view transferEncodingValue = getHeaderValue(transferEncoding, transferEncodingHeader.size(), http);

						chunked = std::equal
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
				size_t position = http.find(web::HttpParser::crlfcrlf);

				if (position != std::string_view::npos)
				{
					position += web::HttpParser::crlfcrlf.size();

					if (largeBodyDetected)
					{
						std::string bodyData(data.data() + position, totalSize - position);

						std::memset(data.data() + position, 0, bodyData.size());

						largeBodyHandler->run(data, bodyData);

						bodySize -= bodyData.size();
						totalSize -= static_cast<int>(bodyData.size());

						isFindEnd = true;
						largeBodyDetected = false;
					}
					else
					{
						isFindEnd = totalSize - position == bodySize;
					}
				}
			}
			else if (endsWith(http, web::HttpParser::crlfcrlf))
			{
				if (chunked)
				{
					using namespace std::string_literals;

					isFindEnd = endsWith(http, "0"s.append(web::HttpParser::crlfcrlf));
				}
				else if (bodySize != -1)
				{
					isFindEnd = (bodySize > 0 && http.find(web::HttpParser::crlfcrlf) != http.rfind(web::HttpParser::crlfcrlf)) ||
						(!bodySize && http.find(web::HttpParser::crlfcrlf) != std::string_view::npos);
				}
				else
				{
					isFindEnd = http.find(web::HttpParser::crlfcrlf) != std::string_view::npos;
				}
			}
		}

		if (largeBodyDetected)
		{
			largeBodyHandler->run(data);
		}

		data.resize(totalSize);

		return totalSize;
	}

	int HttpNetwork::receiveRawData(char* data, int size, bool& endOfStream, int flags)
	{
		ReadOnlyContainerWrapper wrapper(data, size);

		return this->receiveData(wrapper, endOfStream, flags);
	}
}

bool insensetiveSearching(char first, char second)
{
	return tolower(first) == tolower(second);
}

bool endsWith(std::string_view test, std::string_view suffix)
{
	return test.size() >= suffix.size() &&
		test.compare(test.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string_view getHeaderValue(std::string_view::const_iterator startHeader, size_t headerSize, std::string_view http)
{
	size_t headerValuePosition = std::distance(http.begin(), startHeader) + headerSize + web::constants::crlf.size();
	size_t valueSize = http.find(web::constants::crlf, std::distance(http.begin(), startHeader)) - headerValuePosition;

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
				throw std::runtime_error("Can't resize ReadOnlyContainerWrapper");
			}
		},
		[data, size](size_t index) -> char&
		{
			return data[index];
		}
	)
{

}
