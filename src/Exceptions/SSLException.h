#pragma once

#include <stdexcept>

namespace web
{
	namespace exceptions
	{
		class SSLException : public std::runtime_error
		{
		private:
			static std::string getSSLError();

		public:
			SSLException();

			~SSLException() = default;
		};
	}
}
