#include "Exceptions/SSLException.h"

#include <string>
#include <format>

#include <openssl/err.h>

namespace web
{
	namespace exceptions
	{
		SSLException::SSLException(int line, std::string_view file, int returnCode, int errorCode) :
			WebException(line, file),
			returnCode(returnCode),
			ssl(nullptr)
		{
			this->appendErrorMessage(static_cast<unsigned long>(errorCode));
		}

		void SSLException::appendErrorMessage(unsigned long errorCode)
		{
			errorCodes.push_back(errorCode);

			data += std::format("SSL error code '{}'", errorCode);

			if (const char* error = ERR_error_string(errorCode, nullptr))
			{
				data += std::format(" with description '{}' in file '{}' on line '{}'", error, file, line);
			}
		}

		void SSLException::getSSLError(int line, std::string_view file)
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

		SSLException::SSLException(int line, std::string_view file) :
			WebException(line, file),
			ssl(nullptr),
			returnCode((std::numeric_limits<int>::min)())
		{
			this->getSSLError(line, file);
		}

		SSLException::SSLException(int line, std::string_view file, SSL*& ssl, int returnCode) :
			WebException(line, file),
			ssl(&ssl),
			returnCode(returnCode)
		{
			this->getSSLError(line, file);
		}

		SSLException::SSLException(int line, std::string_view file, int returnCode) :
			WebException(line, file),
			ssl(nullptr),
			returnCode(returnCode)
		{
			this->getSSLError(line, file);
		}

		bool SSLException::hasSSL() const
		{
			return static_cast<bool>(ssl);
		}

		const std::vector<unsigned long>& SSLException::getErrorCodes() const
		{
			return errorCodes;
		}

		int SSLException::getReturnCode() const
		{
			return returnCode;
		}

		SSLException::~SSLException()
		{
			if (ssl)
			{
				SSL_free(*ssl);

				*ssl = nullptr;
			}
		}
	}
}
