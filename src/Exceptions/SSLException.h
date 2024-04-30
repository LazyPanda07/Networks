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
			std::vector<unsigned long> errorCodes;
			int returnCode;

		private:
			void getSSLError(int line, std::string_view file);

		public:
			SSLException(int line, std::string_view file);

			SSLException(int line, std::string_view file, int returnCode);

			const std::vector<unsigned long>& getErrorCodes() const;

			int getReturnCode() const;

			~SSLException() = default;
		};
	}
}
