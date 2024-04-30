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

				data += '\n';
			}
		}

		SSLException::SSLException(int line, string_view file) :
			WebException(line, file),
			returnCode((std::numeric_limits<int>::min)())
		{
			this->getSSLError(line, file);
		}

		SSLException::SSLException(int line, string_view file, int returnCode) :
			WebException(line, file),
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
	}
}
