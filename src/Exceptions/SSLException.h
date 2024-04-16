#pragma once

#include <stdexcept>

#include "NetworksUtility.h"

namespace web
{
	namespace exceptions
	{
		class NETWORKS_API SSLException : public std::runtime_error
		{
		private:
			static std::string getSSLError(int line, std::string_view file);

		public:
			SSLException(int line, std::string_view file);

			~SSLException() = default;
		};
	}
}
