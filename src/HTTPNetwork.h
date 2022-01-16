#pragma once

#ifdef NETWORKS_DLL
#define NETWORKS_API __declspec(dllexport)
#else
#define NETWORKS_API
#endif // NETWORKS_DLL

#include "BaseNetwork.h"

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
		static constexpr std::string_view contentLengthHeader = "Content-Length";
		static constexpr std::string_view transferEncodingHeader = "Transfer-Encoding";
		static constexpr std::string_view transferEncodingChunked = "Chunked";
		static constexpr std::string_view crlfcrlf = "\r\n\r\n";
		static constexpr std::string_view crlf = "\r\n";

		static inline const std::string httpPort = "80";

	protected:
		virtual int receiveDataMethod(char* data, int length);

	public:
		/// @brief Server side constructor
		/// @param clientSocket Socket from WSA accept function
		HTTPNetwork(SOCKET clientSocket);

		/// @brief Client side constructor
		/// @param ip Remote address to connect to
		/// @param port Remote port to connect to
		HTTPNetwork(const std::string& ip, const std::string& port = httpPort);

		/// <summary>
		/// Default send function
		/// </summary>
		/// <param name="data">that sends through HTTP</param>
		/// <returns>total send bytes</returns>
		virtual int sendData(const std::vector<char>& data) override;

		/// <summary>
		/// Specific HTTP receive data function
		/// </summary>
		/// <param name="data">output data from HTTP</param>
		/// <returns>total receive bytes</returns>
		/// <exception cref="web::WebException"></exception>
		virtual int receiveData(std::vector<char>& data) override;

		virtual ~HTTPNetwork() = default;
	};
}
