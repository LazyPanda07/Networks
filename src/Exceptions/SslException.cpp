#include "Exceptions/SslException.h"

#include <string>
#include <format>

#include <openssl/err.h>

namespace web
{
	namespace exceptions
	{
		SslException::SslException(int line, std::string_view file, int returnCode, int errorCode) :
			WebException(line, file),
			returnCode(returnCode),
			ssl(nullptr)
		{
			this->appendErrorMessage(static_cast<unsigned long>(errorCode));
		}

		void SslException::appendErrorMessage(unsigned long errorCode)
		{
			errorCodes.push_back(errorCode);

			data += std::format("SSL error code '{}'", errorCode);

			if (const char* error = ERR_error_string(errorCode, nullptr))
			{
				data += std::format(" with description '{}' in file '{}' on line '{}'", error, file, line);
			}
		}

		void SslException::getSSLError(int line, std::string_view file)
		{
			if (ssl)
			{
				if (data.back() != '\n')
				{
					data += '\n';
				}

				int errorCode = SSL_get_error(*ssl, returnCode);

				errorCodes.push_back(errorCode);

				data += std::format("SSL error code '{}' in file '{}' on line '{}'", errorCode, file, line);
			}

			while (unsigned long errorCode = ERR_get_error())
			{
				if (data.back() != '\n')
				{
					data += '\n';
				}

				this->appendErrorMessage(errorCode);
			}
		}

		SslException::SslException(int line, std::string_view file) :
			WebException(line, file),
			ssl(nullptr),
			returnCode((std::numeric_limits<int>::min)())
		{
			this->getSSLError(line, file);
		}

		SslException::SslException(int line, std::string_view file, SSL*& ssl, int returnCode) :
			WebException(line, file),
			ssl(&ssl),
			returnCode(returnCode)
		{
			this->getSSLError(line, file);
		}

		SslException::SslException(int line, std::string_view file, int returnCode) :
			WebException(line, file),
			ssl(nullptr),
			returnCode(returnCode)
		{
			this->getSSLError(line, file);
		}

		bool SslException::hasSSL() const
		{
			return static_cast<bool>(ssl);
		}

		const std::vector<unsigned long>& SslException::getErrorCodes() const
		{
			return errorCodes;
		}

		int SslException::getReturnCode() const
		{
			return returnCode;
		}

		SslException::~SslException()
		{
			if (ssl)
			{
				SSL_free(*ssl);

				*ssl = nullptr;
			}
		}
	}
}
