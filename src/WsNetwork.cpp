#include "WsNetwork.h"

#include <array>
#include <bitset>
#include <bit>
#include <random>

class Frame
{
public:
	enum class OpcodeType
	{
		continuation = 0x0,
		text = 0x1,
		binary = 0x2,
		close = 0x8,
		ping = 0x9,
		pong = 0xA
	};

public:
	inline static constexpr size_t baseHeaderSize = 2;
	inline static constexpr size_t maskSize = 4;
	inline static constexpr size_t fullHeaderMaxSize = baseHeaderSize + maskSize + sizeof(uint64_t);

public:
	using BaseHeader = std::array<uint8_t, baseHeaderSize>;
	using Mask = std::array<uint8_t, maskSize>;
	using FullHeader = std::array<uint8_t, fullHeaderMaxSize>;

private:
	BaseHeader baseHeader;
	std::variant<uint16_t, uint64_t, std::nullptr_t> additionalPayloadSize;
	Mask mask;
	std::vector<uint8_t> payload;
	FullHeader fullHeader;
	int actualFullHeaderSize;

public:
	Frame();

	Frame(bool isFinal, OpcodeType opcode, std::string_view payload, const std::optional<Mask>& mask = std::nullopt);

	void encode();

	void decode();

	bool isFinal() const;

	bool hasMask() const;

	OpcodeType getFrameType() const;

	uint64_t getPayloadSize() const;

	const std::vector<uint8_t>& getPayload() const;

	const FullHeader& getFullHeader() const;

	int getActualFullHeaderSize() const;

	~Frame() = default;

	friend class FrameParser;
};

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
	std::vector<Frame> frames;
	size_t totalPayloadSize;
	Frame* currentFrame;
	ParseState buildState;

private:
	void addFrame();

public:
	FrameParser();

	char* receiveBuild(int& bytes);

	size_t get(std::vector<Frame>& frames);

	~FrameParser() = default;
};

namespace web
{
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

	WsNetwork::WsNetwork(HttpNetwork&& httpNetwork, bool isClient) noexcept :
		Network(std::move(httpNetwork.handle)),
		isClient(isClient)
	{

	}

	int WsNetwork::sendData(const web::utility::ContainerWrapper& data, bool& endOfStream, int flags)
	{
		auto generateMask = []() -> Frame::Mask
			{
				thread_local std::mt19937 random(static_cast<uint32_t>(std::time(nullptr)));

				uint32_t randomBytes = random();
				uint8_t* ptr = reinterpret_cast<uint8_t*>(&randomBytes);

				return
				{
					ptr[0],
					ptr[1],
					ptr[2],
					ptr[3]
				};
			};

		endOfStream = false;

		std::optional<Frame::Mask> mask;

		if (isClient)
		{
			mask = generateMask();
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
		FrameParser builder;
		std::vector<Frame> frames;
		int size;
		int totalSize = 0;
		size_t offset = 0;

		endOfStream = false;

		while (char* ptr = builder.receiveBuild(size))
		{
			totalSize += this->receiveBytes(ptr, size, endOfStream, flags);

			if (endOfStream)
			{
				return totalSize;
			}
		}

		data.resize(builder.get(frames));

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
		throw std::runtime_error("sendRawData not supported");

		return 0;
	}

	int WsNetwork::receiveRawData(char* data, int size, bool& endOfStream, int flags)
	{
		throw std::runtime_error("receiveRawData not supported");

		return 0;
	}
}

Frame::Frame() :
	baseHeader({}),
	additionalPayloadSize(nullptr),
	mask({}),
	fullHeader({}),
	actualFullHeaderSize(0)
{

}

Frame::Frame(bool isFinal, OpcodeType opcode, std::string_view payload, const std::optional<Mask>& mask) :
	baseHeader({}),
	additionalPayloadSize(nullptr),
	mask(mask ? *mask : Mask()),
	payload(payload.begin(), payload.end()),
	fullHeader({}),
	actualFullHeaderSize(sizeof(BaseHeader))
{
	std::bitset<8> byteData(static_cast<int32_t>(opcode));

	byteData[7] = isFinal;

	baseHeader[0] = static_cast<uint8_t>(byteData.to_ulong());

	byteData.reset();

	if (payload.size() > 125)
	{
		if (payload.size() <= (std::numeric_limits<uint16_t>::max)())
		{
			byteData = 126;

			additionalPayloadSize.emplace<uint16_t>(static_cast<uint16_t>(payload.size()));

			actualFullHeaderSize = sizeof(uint16_t);
		}
		else
		{
			byteData = 127;

			additionalPayloadSize.emplace<uint64_t>(static_cast<uint64_t>(payload.size()));

			actualFullHeaderSize = sizeof(uint64_t);
		}
	}
	else
	{
		byteData = payload.size();
	}

	if (mask)
	{
		actualFullHeaderSize += sizeof(Mask);
	}

	byteData[7] = mask.has_value();

	baseHeader[1] = static_cast<uint8_t>(byteData.to_ulong());

	this->encode();

	std::memcpy(fullHeader.data(), baseHeader.data(), baseHeader.size());

	size_t offset = baseHeader.size();

	if (std::holds_alternative<uint16_t>(additionalPayloadSize))
	{
		uint16_t value = std::byteswap(std::get<uint16_t>(additionalPayloadSize));

		std::memcpy(fullHeader.data() + offset, &value, sizeof(value));

		offset += sizeof(value);
	}
	else if (std::holds_alternative<uint64_t>(additionalPayloadSize))
	{
		uint64_t value = std::byteswap(std::get<uint64_t>(additionalPayloadSize));

		std::memcpy(fullHeader.data() + offset, &value, sizeof(value));

		offset += sizeof(value);
	}

	if (mask)
	{
		std::memcpy(fullHeader.data() + offset, mask->data(), mask->size());
	}
}

void Frame::encode()
{
	if (!this->hasMask())
	{
		return;
	}

	for (size_t i = 0; i < payload.size(); i++)
	{
		payload[i] ^= mask[i % mask.size()];
	}
}

void Frame::decode()
{
	if (!this->hasMask())
	{
		return;
	}

	for (size_t i = 0; i < payload.size(); i++)
	{
		payload[i] ^= mask[i % mask.size()];
	}
}

bool Frame::isFinal() const
{
	return static_cast<bool>(baseHeader[0] & 128);
}

bool Frame::hasMask() const
{
	return static_cast<bool>(baseHeader[1] & 128);
}

Frame::OpcodeType Frame::getFrameType() const
{
	return static_cast<OpcodeType>(baseHeader[0] & 15);
}

uint64_t Frame::getPayloadSize() const
{
	switch (additionalPayloadSize.index())
	{
	case 0:
		return std::byteswap(std::get<uint16_t>(additionalPayloadSize));

	case 1:
		return std::byteswap(std::get<uint16_t>(additionalPayloadSize));

	default:
		return static_cast<uint64_t>(baseHeader[1] | 127);
	}
}

const std::vector<uint8_t>& Frame::getPayload() const
{
	return payload;
}

const Frame::FullHeader& Frame::getFullHeader() const
{
	return fullHeader;
}

int Frame::getActualFullHeaderSize() const
{
	return actualFullHeaderSize;
}

void FrameParser::addFrame()
{
	currentFrame = &frames.emplace_back();
}

FrameParser::FrameParser() :
	totalPayloadSize(0),
	buildState(ParseState::baseHeader)
{
	this->addFrame();
}

char* FrameParser::receiveBuild(int& bytes)
{
	switch (buildState)
	{
	case FrameParser::ParseState::baseHeader:
		buildState = ParseState::additionalPayloadSize;

		bytes = static_cast<int>(currentFrame->baseHeader.size());

		return reinterpret_cast<char*>(currentFrame->baseHeader.data());

	case FrameParser::ParseState::additionalPayloadSize:
		switch (currentFrame->getPayloadSize())
		{
		case 126:
			bytes = sizeof(uint16_t);

			return reinterpret_cast<char*>(currentFrame->additionalPayloadSize.emplace<uint16_t>());

		case 127:
			bytes = sizeof(uint64_t);

			return reinterpret_cast<char*>(currentFrame->additionalPayloadSize.emplace<uint64_t>());

		default:
			if (currentFrame->hasMask())
			{
				buildState = ParseState::mask;
			}
			else
			{
				buildState = ParseState::payload;
			}

			return this->receiveBuild(bytes);
		}

	case FrameParser::ParseState::mask:
		bytes = static_cast<int>(currentFrame->mask.size());

		buildState = ParseState::payload;

		return reinterpret_cast<char*>(currentFrame->mask.data());

	case FrameParser::ParseState::payload:
		bytes = static_cast<int>(currentFrame->payload.size());

		currentFrame->payload.resize(currentFrame->getPayloadSize());
		buildState = ParseState::finish;

		return reinterpret_cast<char*>(currentFrame->payload.data());

	case FrameParser::ParseState::finish:
		currentFrame->decode();

		totalPayloadSize += currentFrame->getPayloadSize();

		if (currentFrame->isFinal())
		{
			return nullptr;
		}

		this->addFrame();

		buildState = ParseState::baseHeader;

		return this->receiveBuild(bytes);

	default:
		return nullptr;
	}
}

size_t FrameParser::get(std::vector<Frame>& frames)
{
	frames = std::move(this->frames);

	return totalPayloadSize;
}
