#include "SSLException.h"

#include <string>
#include <format>

#include <openssl/err.h>

using namespace std;

namespace web
{
	namespace exceptions
	{
		void SSLException::getSSLError(int line, string_view file)
		{
			if (ssl)
			{
				if (data.back() != '\n')
				{
					data += '\n';
				}

				int errorCode = SSL_get_error(*ssl, returnCode);

				errorCodes.push_back(errorCode);

				data += format("SSL error code '{}' in file '{}' on line '{}'", errorCode, file, line);
			}

			while (unsigned long errorCode = ERR_get_error())
			{
				if (data.back() != '\n')
				{
					data += '\n';
				}

				errorCodes.push_back(errorCode);

				data += format("SSL error code '{}'", errorCode);

				if (const char* error = ERR_error_string(errorCode, nullptr))
				{
					data += format(" with description '{}' in file '{}' on line '{}'", error, file, line);
				}
			}
		}

		SSLException::SSLException(int line, string_view file) :
			WebException(line, file),
			ssl(nullptr),
			returnCode((numeric_limits<int>::min)())
		{
			this->getSSLError(line, file);
		}

		SSLException::SSLException(int line, string_view file, SSL*& ssl, int returnCode) :
			WebException(line, file),
			ssl(&ssl),
			returnCode(returnCode)
		{

		}

		const vector<unsigned long>& SSLException::getErrorCodes() const
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
