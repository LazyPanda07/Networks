#include "WebSocket/Frame.h"

#include <bitset>
#include <bit>
#include <random>
#include <cstring>
#include <ctime>

namespace web::web_socket
{
	Frame::Mask Frame::generateMask()
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

	Frame::Frame() :
		baseHeader({}),
		additionalPayloadSize(nullptr),
		mask({}),
		fullHeader({}),
		actualFullHeaderSize(0)
	{

	}

	Frame::Frame(bool isFinal, OpcodeType opcode, std::string_view payload, const std::optional<Mask>& mask, bool masked) :
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

				actualFullHeaderSize += sizeof(uint16_t);
			}
			else
			{
				byteData = 127;

				additionalPayloadSize.emplace<uint64_t>(static_cast<uint64_t>(payload.size()));

				actualFullHeaderSize += sizeof(uint64_t);
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

		if (!masked)
		{
			this->encode();
		}
		
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

	Frame::OpcodeType Frame::getFrameOpcode() const
	{
		return static_cast<OpcodeType>(baseHeader[0] & 15);
	}

	uint64_t Frame::getPayloadSize() const
	{
		switch (additionalPayloadSize.index())
		{
		case 0:
			return static_cast<uint64_t>(std::byteswap(std::get<uint16_t>(additionalPayloadSize)));

		case 1:
			return std::byteswap(std::get<uint64_t>(additionalPayloadSize));

		default:
			return static_cast<uint64_t>(baseHeader[1] & 127);
		}
	}

	const std::vector<uint8_t>& Frame::getPayload() const
	{
		return payload;
	}

	std::vector<uint8_t> Frame::getUnmaskedPayload() const
	{
		std::vector<uint8_t> result(payload);

		for (size_t i = 0; i < payload.size(); i++)
		{
			result[i] ^= mask[i % mask.size()];
		}

		return result;
	}

	const Frame::FullHeader& Frame::getFullHeader() const
	{
		return fullHeader;
	}

	int Frame::getActualFullHeaderSize() const
	{
		return actualFullHeaderSize;
	}

	Frame::Mask Frame::getMask() const
	{
		return mask;
	}
}
