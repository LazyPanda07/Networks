#pragma once

#include "WebException.h"
#include "NetworksUtility.h"

namespace web
{
	namespace exceptions
	{
		class NETWORKS_API SSLException : public web::exceptions::WebException
		{
		private:
			static std::string getSSLError(int line, std::string_view file);

		public:
			SSLException(int line, std::string_view file);

			~SSLException() = default;
		};
	}
}
