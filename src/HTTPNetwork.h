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
		/// <summary>
		/// Server side constructor
		/// </summary>
		/// <param name="clientSocket">socket from WSA accept function</param>
		HTTPNetwork(SOCKET clientSocket);

		/// <summary>
		/// Client side constructor
		/// </summary>
		/// <param name="ip">server address</param>
		/// <param name="port">server listen socket port</param>
		HTTPNetwork(const std::string& ip, const std::string& port);

		/// <summary>
		/// Default send function
		/// </summary>
		/// <param name="data">that sends through HTTP</param>
		/// <returns>total send bytes</returns>
		int sendData(const std::vector<char>& data) override;

		/// <summary>
		/// Specific HTTP receive data function
		/// </summary>
		/// <param name="data">output data from HTTP</param>
		/// <returns>total receive bytes</returns>
		/// <exception cref="web::WebException"></exception>
		int receiveData(std::vector<char>& data) override;

		virtual ~HTTPNetwork() = default;
	};
}
