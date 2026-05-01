#include "WebSocket/WsNetwork.h"

#include <cstring>

class FrameParser
{
private:
	enum class ParseState
	{
		baseHeader,
		additionalPayloadSize,
		mask,
		payload,
		finish
	};

private:
	std::vector<web::web_socket::Frame> frames;
	size_t totalPayloadSize;
	web::web_socket::Frame* currentFrame;
	ParseState parseState;

private:
	void addFrame();

public:
	FrameParser();

	char* parse(int& bytes);

	size_t get(std::vector<web::web_socket::Frame>& frames);

	~FrameParser() = default;
};

namespace web::web_socket
{
	WsNetwork::WsNetwork(bool isClient) :
		isClient(isClient)
	{

	}

	WsNetwork::WsNetwork(SOCKET clientSocket) :
		Network(clientSocket),
		isClient(false)
	{

	}

	WsNetwork::WsNetwork(std::string_view ip, std::string_view port) :
		Network(ip, port),
		isClient(true)
	{

	}

	WsNetwork::WsNetwork(http::HttpNetwork&& httpNetwork, bool isClient) noexcept :
		isClient(isClient)
	{
		clientSocket = httpNetwork.clientSocket;
		buffers = std::move(httpNetwork.buffers);

		httpNetwork.clientSocket = INVALID_SOCKET;
	}

	bool WsNetwork::getIsClient() const
	{
		return isClient;
	}

	int WsNetwork::sendData(const web::utility::ContainerWrapper& data, bool& endOfStream, int flags)
	{
		endOfStream = false;

		std::optional<Frame::Mask> mask;

		if (isClient)
		{
			mask = Frame::generateMask();
		}

		Frame frame(true, Frame::OpcodeType::binary, data.data(), mask);
		const std::vector<uint8_t>& payload = frame.getPayload();
		const Frame::FullHeader& fullHeader = frame.getFullHeader();
		std::vector<uint8_t> bytes(frame.getActualFullHeaderSize() + payload.size());

		std::memcpy(bytes.data(), fullHeader.data(), frame.getActualFullHeaderSize());
		std::memcpy(bytes.data() + frame.getActualFullHeaderSize(), payload.data(), payload.size());

		return this->sendBytes(bytes.data(), static_cast<int>(bytes.size()), endOfStream, flags);
	}

	int WsNetwork::receiveData(web::utility::ContainerWrapper& data, bool& endOfStream, int flags)
	{
		FrameParser parser;
		std::vector<Frame> frames;
		int size;
		int totalSize = 0;
		size_t offset = 0;

		endOfStream = false;

		while (char* ptr = parser.parse(size))
		{
			int receiveSize = 0;

			do
			{
				receiveSize += this->receiveBytes(ptr + receiveSize, size - receiveSize, endOfStream, flags);

				if (endOfStream)
				{
					return totalSize + receiveSize;
				}
			}
			while (receiveSize != size);

			totalSize += size;
		}

		data.resize(parser.get(frames));

		for (const Frame& frame : frames)
		{
			const std::vector<uint8_t>& payload = frame.getPayload();

			std::memcpy(data.data() + offset, payload.data(), payload.size());

			offset += payload.size();
		}

		return totalSize;
	}

	int WsNetwork::sendRawData(const char* data, int size, bool& endOfStream, int flags)
	{
		int totalSend = 0;

		do
		{
			int lastPacket = this->sendRawData(data + totalSend, size - totalSend, endOfStream, flags);

			if (endOfStream)
			{
				return totalSend;
			}

			totalSend += lastPacket;
		}
		while (totalSend != size);

		return totalSend;
	}

	int WsNetwork::receiveRawData(char* data, int size, bool& endOfStream, int flags)
	{
		int totalReceive = 0;

		do
		{
			int lastPacket = this->receiveRawData(data + totalReceive, size - totalReceive, endOfStream, flags);

			if (endOfStream)
			{
				return totalReceive;
			}

			totalReceive += lastPacket;
		}
		while (totalReceive != size);

		return totalReceive;
	}
}

namespace streams
{
	IOSocketStream& operator >>(IOSocketStream& stream, std::vector<web::web_socket::Frame>& frame)
	{
		if (!dynamic_cast<const web::web_socket::WsNetwork*>(&stream.getNetwork()))
		{
			throw std::runtime_error("Frames can be sended only with WsNetwork or WssNetwork");
		}

		FrameParser parser;
		int bytes = 0;

		while (char* ptr = parser.parse(bytes))
		{
			std::streamsize lastPacket = stream.read(ptr, static_cast<std::streamsize>(bytes)).gcount();
		}

		parser.get(frame);

		return stream;
	}

	IOSocketStream& operator <<(IOSocketStream& stream, const web::web_socket::Frame& frame)
	{
		const web::web_socket::WsNetwork* network = dynamic_cast<const web::web_socket::WsNetwork*>(&stream.getNetwork());

		if (!network)
		{
			throw std::runtime_error("Frames can be received only with WsNetwork or WssNetwork");
		}

		std::optional<web::web_socket::Frame::Mask> mask;

		if (network->getIsClient())
		{
			mask = web::web_socket::Frame::generateMask();
		}

		const std::vector<uint8_t>& payload = frame.getPayload();
		std::string_view tempPayload(reinterpret_cast<const char*>(payload.data()), payload.size());
		web::web_socket::Frame resultFrame(frame.isFinal(), frame.getFrameOpcode(), tempPayload, mask);
		const web::web_socket::Frame::FullHeader& fullHeader = resultFrame.getFullHeader();
		std::vector<uint8_t> bytes(resultFrame.getActualFullHeaderSize() + payload.size());

		std::memcpy(bytes.data(), fullHeader.data(), resultFrame.getActualFullHeaderSize());
		std::memcpy(bytes.data() + resultFrame.getActualFullHeaderSize(), payload.data(), payload.size());

		stream.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());

		return stream;
	}
}

void FrameParser::addFrame()
{
	currentFrame = &frames.emplace_back();
}

FrameParser::FrameParser() :
	totalPayloadSize(0),
	parseState(ParseState::baseHeader)
{
	this->addFrame();
}

char* FrameParser::parse(int& bytes)
{
	switch (parseState)
	{
	case FrameParser::ParseState::baseHeader:
		parseState = ParseState::additionalPayloadSize;

		bytes = static_cast<int>(currentFrame->baseHeader.size());

		return reinterpret_cast<char*>(currentFrame->baseHeader.data());

	case FrameParser::ParseState::additionalPayloadSize:
		switch (currentFrame->getPayloadSize())
		{
		case 126:
			bytes = sizeof(uint16_t);

			return reinterpret_cast<char*>(&currentFrame->additionalPayloadSize.emplace<uint16_t>());

		case 127:
			bytes = sizeof(uint64_t);

			return reinterpret_cast<char*>(&currentFrame->additionalPayloadSize.emplace<uint64_t>());

		default:
			if (currentFrame->hasMask())
			{
				parseState = ParseState::mask;
			}
			else
			{
				parseState = ParseState::payload;
			}

			return this->parse(bytes);
		}

	case FrameParser::ParseState::mask:
		bytes = static_cast<int>(currentFrame->mask.size());

		parseState = ParseState::payload;

		return reinterpret_cast<char*>(currentFrame->mask.data());

	case FrameParser::ParseState::payload:
		bytes = static_cast<int>(currentFrame->getPayloadSize());

		currentFrame->payload.resize(bytes);
		parseState = ParseState::finish;

		return reinterpret_cast<char*>(currentFrame->payload.data());

	case FrameParser::ParseState::finish:
		currentFrame->decode();

		totalPayloadSize += currentFrame->getPayloadSize();

		if (currentFrame->isFinal())
		{
			return nullptr;
		}

		this->addFrame();

		parseState = ParseState::baseHeader;

		return this->parse(bytes);

	default:
		return nullptr;
	}
}

size_t FrameParser::get(std::vector<web::web_socket::Frame>& frames)
{
	frames = std::move(this->frames);

	return totalPayloadSize;
}
