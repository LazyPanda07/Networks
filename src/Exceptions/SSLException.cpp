#include "SSLException.h"

#include <string>
#include <format>

#include <openssl/err.h>

using namespace std;

namespace web
{
	namespace exceptions
	{
		string SSLException::getSSLError(int line, string_view file)
		{
			string result;
			int errorCode;

			while (errorCode = ERR_get_error())
			{
				result += format("SSL error code '{}'", errorCode);

				if (const char* error = ERR_error_string(errorCode, nullptr))
				{
					result += format(" with description '{}' in file '{}' on line '{}'", error, file, line);
				}

				result += '\n';
			}

			return result;
		}

		SSLException::SSLException(int line, string_view file) :
			WebException(line, file)
		{
			if (!errorCode)
			{
				data = SSLException::getSSLError(line, file);
			}
		}
	}
}
