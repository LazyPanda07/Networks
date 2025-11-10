#include <string>
#include <chrono>

#include <gtest/gtest.h>

#include "HttpBuilder.h"
#include "HttpParser.h"
#include "IOSocketStream.h"
#include "HttpsNetwork.h"

std::string token;

extern void runServer(bool& isRunning);

TEST(HTTPS, GithubAPI)
{
	try
	{
		streams::IOSocketStream stream = streams::IOSocketStream::createStream<web::HttpsNetwork>("api.github.com", "443");
		std::string request = web::HttpBuilder()
			.getRequest()
			.parameters("repos/LazyPanda07/Networks/branches")
			.headers
			(
				"Host", "api.github.com",
				"Accept", "application/vnd.github+json",
				"Authorization", "Bearer " + token,
				"User-Agent", "NetworkTests"
			)
			.build();
		std::string response;

		stream << request;

		stream >> response;

		ASSERT_EQ(web::HttpParser(response).getResponseCode(), web::ResponseCodes::ok);
	}
	catch (const web::exceptions::WebException& e)
	{
		std::cout << e.what() << ' ' << e.getFile() << ' ' << e.getLine() << ' ' << e.getErrorCode() << std::endl;

		exit(1);
	}
}

TEST(HTTPS, GithubAPIBuilder)
{
	try
	{
		streams::IOSocketStream stream = streams::IOSocketStream::createStream<web::HttpsNetwork>("api.github.com", "443");
		web::HttpBuilder request = web::HttpBuilder()
			.getRequest()
			.parameters("repos/LazyPanda07/Networks/branches")
			.headers
			(
				"Host", "api.github.com",
				"Accept", "application/vnd.github+json",
				"Authorization", "Bearer " + token,
				"User-Agent", "NetworkTests"
			);
		web::HttpParser response;

		stream << request;

		stream >> response;

		ASSERT_EQ(response.getResponseCode(), web::ResponseCodes::ok);
	}
	catch (const web::exceptions::WebException& e)
	{
		std::cout << e.what() << ' ' << e.getFile() << ' ' << e.getLine() << ' ' << e.getErrorCode() << std::endl;

		exit(1);
	}
}

TEST(HTTPS, GithubAPIParser)
{
	try
	{
		streams::IOSocketStream stream = streams::IOSocketStream::createStream<web::HttpsNetwork>("api.github.com", "443");
		std::string request = web::HttpBuilder()
			.getRequest()
			.parameters("repos/LazyPanda07/Networks/branches")
			.headers
			(
				"Host", "api.github.com",
				"Accept", "application/vnd.github+json",
				"Authorization", "Bearer " + token,
				"User-Agent", "NetworkTests"
			)
			.build();
		web::HttpParser response;

		stream << request;

		stream >> response;

		ASSERT_EQ(response.getResponseCode(), web::ResponseCodes::ok);
	}
	catch (const web::exceptions::WebException& e)
	{
		std::cout << e.what() << ' ' << e.getFile() << ' ' << e.getLine() << ' ' << e.getErrorCode() << std::endl;

		exit(1);
	}
}

TEST(HTTP, GithubAPIBuilder)
{
	try
	{
		streams::IOSocketStream stream = streams::IOSocketStream::createStream<web::HttpNetwork>("127.0.0.1", "8080");
		web::HttpBuilder request = web::HttpBuilder()
			.getRequest()
			.parameters("repos/LazyPanda07/Networks/branches")
			.headers
			(
				"Host", "api.github.com",
				"Accept", "application/vnd.github+json",
				"Authorization", "Bearer " + token,
				"User-Agent", "NetworkTests"
			);
		web::HttpParser response;

		stream << request;
		stream >> response;

		ASSERT_EQ(response.getResponseCode(), web::ResponseCodes::ok);
	}
	catch (const web::exceptions::WebException& e)
	{
		std::cout << e.what() << ' ' << e.getFile() << ' ' << e.getLine() << ' ' << e.getErrorCode() << std::endl;

		exit(1);
	}
}

TEST(HTTP, GithubAPIParser)
{
	try
	{
		streams::IOSocketStream stream = streams::IOSocketStream::createStream<web::HttpNetwork>("127.0.0.1", "8080");
		std::string request = web::HttpBuilder()
			.getRequest()
			.parameters("repos/LazyPanda07/Networks/branches")
			.headers
			(
				"Host", "api.github.com",
				"Accept", "application/vnd.github+json",
				"Authorization", "Bearer " + token,
				"User-Agent", "NetworkTests"
			)
			.build();
		web::HttpParser response;

		stream << request;

		stream >> response;

		ASSERT_EQ(response.getResponseCode(), web::ResponseCodes::ok);
	}
	catch (const web::exceptions::WebException& e)
	{
		std::cout << e.what() << ' ' << e.getFile() << ' ' << e.getLine() << ' ' << e.getErrorCode() << std::endl;

		exit(1);
	}
}

int main(int argc, char** argv)
{
	token = argv[1];

	bool isRunning = false;

	std::thread(runServer, std::ref(isRunning)).detach();

	testing::InitGoogleTest(&argc, argv);

	while (!isRunning)
	{
		std::cout << "Wait server..." << std::endl;

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return RUN_ALL_TESTS();
}
