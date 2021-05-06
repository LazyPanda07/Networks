#include "SSLException.h"

#include <string>

#include <openssl/err.h>

using namespace std;

namespace web
{
	namespace exceptions
	{
		string SSLException::getSSLError()
		{
			string result;
			int errorCode;

			while (errorCode = ERR_get_error())
			{
				const char* error = ERR_error_string(errorCode, nullptr);

				if (error)
				{
					result += error;
				}
				else
				{
					result += "SSL error code: " + to_string(errorCode);
				}

				result += '\n';
			}

			return result;
		}

		SSLException::SSLException() :
			runtime_error(getSSLError())
		{

		}
	}
}
