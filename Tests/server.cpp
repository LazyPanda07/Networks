#include <iostream>

#include <BaseTCPServer.h>

#include "IOSocketStream.h"
#include "HttpBuilder.h"
#include "HttpParser.h"
#include "HttpNetwork.h"

void check(const web::HeadersMap& headers, const std::string& header, const std::string& value);

class TestServer : public web::BaseTCPServer
{
private:
	void clientConnection(const std::string& ip, SOCKET clientSocket, sockaddr address, std::function<void()>& cleanup) override
	{
		streams::IOSocketStream stream = streams::IOSocketStream::createStream<web::HttpNetwork>(clientSocket);

		web::HttpParser parser;
		web::HttpBuilder builder;

		stream >> parser;

		try
		{
			const web::HeadersMap& headers = parser.getHeaders();

			check(headers, "Host", "api.github.com");
			check(headers, "Accept", "application/vnd.github+json");
			check(headers, "User-Agent", "NetworkTests");

			builder.responseCode(web::ResponseCodes::ok);
		}
		catch (const std::exception&)
		{
			builder.responseCode(web::ResponseCodes::internalServerError);
		}

		stream << builder;
	}

public:
	TestServer() :
		BaseTCPServer("8080", "127.0.0.1")
	{

	}
};

void runServer(bool& isRunning)
{
	TestServer server;

	server.start(true, [&isRunning]() { std::cout << "Server is running" << std::endl; isRunning = true; });
}

void check(const web::HeadersMap& headers, const std::string& header, const std::string& value)
{
	if (headers.at(header) != value)
	{
		throw std::runtime_error("Error");
	}
}
