#pragma once

#include <optional>
#include <array>
#include <variant>
#include <cstdint>
#include <vector>

#include "NetworksUtility.h"

class FrameParser;

namespace web::web_socket
{
	/**
	 * @brief Represents a WebSocket frame, provides construction, encoding/decoding, and accessors for header and payload.
	 */
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

	public:
		static Mask generateMask();

	private:
		BaseHeader baseHeader;
		std::variant<uint16_t, uint64_t, std::nullptr_t> additionalPayloadSize;
		Mask mask;
		std::vector<uint8_t> payload;
		FullHeader fullHeader;
		int actualFullHeaderSize;

	public:
		Frame();

		Frame(bool isFinal, OpcodeType opcode, std::string_view payload, const std::optional<Mask>& mask = std::nullopt, bool masked = false);

		void encode();

		void decode();

		bool isFinal() const;

		bool hasMask() const;

		OpcodeType getFrameOpcode() const;

		uint64_t getPayloadSize() const;

		const std::vector<uint8_t>& getPayload() const;

		std::vector<uint8_t> getUnmaskedPayload() const;

		const FullHeader& getFullHeader() const;

		int getActualFullHeaderSize() const;

		Mask getMask() const;

		~Frame() = default;

		friend class ::FrameParser;
	};
}
