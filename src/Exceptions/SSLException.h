#pragma once

#include <vector>

#include "WebException.h"
#include "NetworksUtility.h"

namespace web
{
	namespace exceptions
	{
		class NETWORKS_API SSLException : public web::exceptions::WebException
		{
		private:
			std::vector<int> errorCodes;

		private:
			void getSSLError(int line, std::string_view file);

		public:
			SSLException(int line, std::string_view file);

			const std::vector<int>& getErrorCodes() const;

			~SSLException() = default;
		};
	}
}
