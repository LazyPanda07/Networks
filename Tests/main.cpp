#include <string>

#include "HTTPBuilder.h"
#include "HTTPParser.h"
#include "IOSocketStream.h"
#include "HTTPSNetwork.h"

#include "gtest/gtest.h"

std::string token;

extern void runServer();

TEST(HTTPS, GithubAPI)
{
	try
	{
		streams::IOSocketStream stream(std::make_unique<web::HTTPSNetwork>("api.github.com", "443"));
		std::string request = web::HTTPBuilder()
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

		ASSERT_EQ(web::HTTPParser(response).getResponseCode(), web::responseCodes::ok);
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
		streams::IOSocketStream stream(std::make_unique<web::HTTPSNetwork>("api.github.com", "443"));
		web::HTTPBuilder request = web::HTTPBuilder()
			.getRequest()
			.parameters("repos/LazyPanda07/Networks/branches")
			.headers
			(
				"Host", "api.github.com",
				"Accept", "application/vnd.github+json",
				"Authorization", "Bearer " + token,
				"User-Agent", "NetworkTests"
			);
		web::HTTPParser response;

		stream << request;

		stream >> response;

		ASSERT_EQ(response.getResponseCode(), web::responseCodes::ok);
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
		streams::IOSocketStream stream(std::make_unique<web::HTTPSNetwork>("api.github.com", "443"));
		std::string request = web::HTTPBuilder()
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
		web::HTTPParser response;

		stream << request;

		stream >> response;

		ASSERT_EQ(response.getResponseCode(), web::responseCodes::ok);
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
		streams::IOSocketStream stream(std::make_unique<web::HTTPNetwork>("127.0.0.1", "8080"));
		web::HTTPBuilder request = web::HTTPBuilder()
			.getRequest()
			.parameters("repos/LazyPanda07/Networks/branches")
			.headers
			(
				"Host", "api.github.com",
				"Accept", "application/vnd.github+json",
				"Authorization", "Bearer " + token,
				"User-Agent", "NetworkTests"
			);
		web::HTTPParser response;

		stream << request;

		stream >> response;

		ASSERT_EQ(response.getResponseCode(), web::responseCodes::ok);
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
		streams::IOSocketStream stream(std::make_unique<web::HTTPNetwork>("127.0.0.1", "8080"));
		std::string request = web::HTTPBuilder()
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
		web::HTTPParser response;

		stream << request;

		stream >> response;

		ASSERT_EQ(response.getResponseCode(), web::responseCodes::ok);
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

	std::thread(runServer).detach();

	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
