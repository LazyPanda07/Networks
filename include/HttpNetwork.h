#pragma once

#include <functional>
#include <format>

#include "NetworksUtility.h"
#include "Network.h"
#include "LargeBodyHandler.h"

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
		static constexpr size_t defaultLargeBodySize = 128 * 1024 * 1024;
		static constexpr std::string_view contentLengthHeader = "Content-Length";
		static constexpr std::string_view transferEncodingHeader = "Transfer-Encoding";
		static constexpr std::string_view transferEncodingChunked = "Chunked";

		static inline constexpr std::string_view httpPort = "80";

	private:
		std::unique_ptr<LargeBodyHandler> largeBodyHandler;
		size_t largeBodySizeThreshold;

	public:
		/// @brief Server side constructor
		template<Timeout T = std::chrono::seconds>
		HTTPNetwork(SOCKET clientSocket, T timeout = 30s, size_t largeBodySizeThreshold = 0);

		/// @brief Client side constructor
		/// @param ip Remote address to connect to
		/// @param port Remote port to connect to
		template<Timeout T = std::chrono::seconds>
		HTTPNetwork(std::string_view ip, std::string_view port = httpPort, T timeout = 30s, size_t largeBodySizeThreshold = 0);

		void setLargeBodySizeThreshold(size_t largeBodySizeThreshold);

		template<std::derived_from<LargeBodyHandler> T, typename... Args>
		void setLargeBodyHandler(size_t largeBodyPacketSize, Args&&... args);

		template<std::derived_from<LargeBodyHandler> T = LargeBodyHandler>
		T& getLargeBodyHandler();

		template<std::derived_from<LargeBodyHandler> T = LargeBodyHandler>
		const T& getLargeBodyHandler() const;

		int sendData(const utility::ContainerWrapper& data, bool& endOfStream, int flags = 0) override;

		int sendRawData(const char* data, int size, bool& endOfStream, int flags = 0) override;

		int receiveData(utility::ContainerWrapper& data, bool& endOfStream, int flags = 0) override;

		int receiveRawData(char* data, int size, bool& endOfStream, int flags = 0) override;

		virtual ~HTTPNetwork() = default;
	};
}

namespace web
{
	template<Timeout T>
	HTTPNetwork::HTTPNetwork(SOCKET clientSocket, T timeout, size_t largeBodySizeThreshold) :
		Network(clientSocket, timeout),
		largeBodySizeThreshold(largeBodySizeThreshold ? largeBodySizeThreshold : HTTPNetwork::defaultLargeBodySize)
	{

	}

	template<Timeout T>
	HTTPNetwork::HTTPNetwork(std::string_view ip, std::string_view port, T timeout, size_t largeBodySizeThreshold) :
		Network(ip, port, timeout),
		largeBodySizeThreshold(largeBodySizeThreshold ? largeBodySizeThreshold : HTTPNetwork::defaultLargeBodySize)
	{

	}

	template<std::derived_from<LargeBodyHandler> T, typename... Args>
	void HTTPNetwork::setLargeBodyHandler(size_t largeBodyPacketSize, Args&&... args)
	{
		largeBodyHandler = std::make_unique<T>(std::forward<Args>(args)...);

		largeBodyHandler->setChunkSize(largeBodyPacketSize);
	}

	template<std::derived_from<LargeBodyHandler> T>
	T& HTTPNetwork::getLargeBodyHandler()
	{
		if (largeBodyHandler)
		{
			return static_cast<T&>(*largeBodyHandler);
		}

		throw std::runtime_error("largeBodyHandler is nullptr");

		return static_cast<T&>(*largeBodyHandler);
	}

	template<std::derived_from<LargeBodyHandler> T>
	const T& HTTPNetwork::getLargeBodyHandler() const
	{
		if (largeBodyHandler)
		{
			return static_cast<const T&>(*largeBodyHandler);
		}

		throw std::runtime_error("largeBodyHandler is nullptr");

		return static_cast<const T&>(*largeBodyHandler);
	}
}
