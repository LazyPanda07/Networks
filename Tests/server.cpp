#include <iostream>

#include "IOSocketStream.h"
#include "HTTPBuilder.h"
#include "HTTPParser.h"

#include "BaseTCPServer.h"

void check(const web::HeadersMap& headers, const std::string& header, const std::string& value);

class TestServer : public web::BaseTCPServer
{
private:
	void clientConnection(const std::string& ip, SOCKET clientSocket, sockaddr address, std::function<void()>& cleanup) override
	{
		streams::IOSocketStream stream(clientSocket);

		while (true)
		{
			web::HTTPParser parser;
			web::HTTPBuilder builder;

			if (stream.eof() || stream.bad())
			{
				break;
			}

			stream >> parser;

			if (stream.eof() || stream.bad())
			{
				break;
			}

			try
			{
				const auto& headers = parser.getHeaders();

				check(headers, "Host", "api.github.com");
				check(headers, "Accept", "application/vnd.github+json");
				check(headers, "User-Agent", "NetworkTests");

				builder.responseCode(web::responseCodes::ok);
			}
			catch (const std::exception&)
			{
				builder.responseCode(web::responseCodes::internalServerError);
			}

			stream << builder;
		}
	}

public:
	TestServer() :
		BaseTCPServer("8080", "127.0.0.1")
	{

	}
};

void runServer()
{
	TestServer server;

	server.start(true, []() { std::cout << "Server is running" << std::endl; });
}

void check(const web::HeadersMap& headers, const std::string& header, const std::string& value)
{
	if (headers.at(header) != value)
	{
		throw std::runtime_error("Error");
	}
}
