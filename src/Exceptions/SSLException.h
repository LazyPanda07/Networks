#pragma once

#include <stdexcept>

#ifdef NETWORKS_DLL
#define NETWORKS_API __declspec(dllexport)
#else
#define NETWORKS_API
#endif // NETWORKS_DLL

namespace web
{
	namespace exceptions
	{
		class NETWORKS_API SSLException : public std::runtime_error
		{
		private:
			static std::string getSSLError();

		public:
			SSLException();

			~SSLException() = default;
		};
	}
}
