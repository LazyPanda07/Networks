#pragma once

#include <functional>

#include "NetworksUtility.h"

#include "Network.h"

namespace web
{
	/// <summary>
	/// Network functions for HTTP
	/// </summary>
	class NETWORKS_API HTTPNetwork : public web::Network
	{
	public:
		static constexpr uint16_t averageHTTPRequestSize = 1500;
		static constexpr uint16_t thresholdSize = 100;
		static constexpr uint64_t defaultLargeBodySize = 128 * 1024 * 1024;
		static constexpr std::string_view contentLengthHeader = "Content-Length";
		static constexpr std::string_view transferEncodingHeader = "Transfer-Encoding";
		static constexpr std::string_view transferEncodingChunked = "Chunked";
		static constexpr std::string_view crlfcrlf = "\r\n\r\n";
		static constexpr std::string_view crlf = "\r\n";

		static inline constexpr std::string_view httpPort = "80";

	private:
		std::function<bool(std::string_view)> largeBodyHandler;
		std::function<void(utility::ContainerWrapper&)> headersHandler;
		uint64_t largeBodySizeThreshold;
		int64_t largeBodyPacketSize;

	public:
		/// @brief Server side constructor
		/// @param clientSocket Socket from WSA accept function
		HTTPNetwork(SOCKET clientSocket, uint64_t largeBodySizeThreshold = 0);

		/// @brief Client side constructor
		/// @param ip Remote address to connect to
		/// @param port Remote port to connect to
		HTTPNetwork(std::string_view ip, std::string_view port = httpPort, DWORD timeout = 30'000, uint64_t largeBodySizeThreshold = 0);

		/**
		 * @brief 
		 * @param largeBodyHandler 
		 * @param largeBodyPacketSize -1 means uses default
		 */
		void setLargeBodyHandler(const std::function<bool(std::string_view)>& largeBodyHandler, const std::function<void(utility::ContainerWrapper&)>& headersHandler, int64_t largeBodyPacketSize = -1);

		int sendData(const utility::ContainerWrapper& data, bool& endOfStream, int flags = 0) override;

		int sendRawData(const char* data, int size, bool& endOfStream, int flags = 0) override;

		int receiveData(utility::ContainerWrapper& data, bool& endOfStream, int flags = 0) override;

		int receiveRawData(char* data, int size, bool& endOfStream, int flags = 0) override;

		virtual ~HTTPNetwork() = default;
	};
}
