#pragma once

#include <vector>

#include <openssl/ssl.h>

#include "WebException.h"
#include "NetworksUtility.h"

namespace web
{
	namespace exceptions
	{
		class NETWORKS_API SslException : public web::exceptions::WebException
		{
		private:
			std::vector<unsigned long> errorCodes;
			int returnCode;
			SSL** ssl;

		private:
			void appendErrorMessage(unsigned long errorCode);

			void getSSLError(int line, std::string_view file);

		public:
			SslException(int line, std::string_view file);

			SslException(int line, std::string_view file, SSL*& ssl, int returnCode);

			SslException(int line, std::string_view file, int returnCode);

			SslException(int line, std::string_view file, int returnCode, int errorCode);

			bool hasSSL() const;

			const std::vector<unsigned long>& getErrorCodes() const;

			int getReturnCode() const;

			~SslException();
		};
	}
}
