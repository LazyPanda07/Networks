#include <string>

#include "HTTPBuilder.h"
#include "HTTPParser.h"
#include "IOSocketStream.h"
#include "HTTPSNetwork.h"

#include "gtest/gtest.h"

std::string token;

TEST(HTTP, GithubAPI)
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
		std::vector<char> testRequest;
		std::vector<char> testResponse;
		// std::string response;

		for (char c : request) 
		{
			testRequest.push_back(c);
		}

		stream << testRequest;

		stream >> testResponse;

		ASSERT_EQ(web::HTTPParser(testResponse).getResponseCode(), web::responseCodes::ok);
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

	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
