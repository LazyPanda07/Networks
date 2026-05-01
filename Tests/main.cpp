#include <string>
#include <chrono>
#include <format>
#include <filesystem>

#include <gtest/gtest.h>
#include <openssl/ssl.h>

#include <HttpBuilder.h>
#include <HttpParser.h>
#include <IOSocketStream.h>
#include <Http/HttpsNetwork.h>
#include <WebSocket/WsNetwork.h>
#include <Base64.h>

std::string token;

extern void runServer(bool& isRunning);

TEST(HTTPS, GithubAPI)
{
	try
	{
		streams::IOSocketStream stream = streams::IOSocketStream::createStream<web::http::HttpsNetwork>("api.github.com", "443");
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
		streams::IOSocketStream stream = streams::IOSocketStream::createStream<web::http::HttpsNetwork>("api.github.com", "443");
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
		streams::IOSocketStream stream = streams::IOSocketStream::createStream<web::http::HttpsNetwork>("api.github.com", "443");
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
		streams::IOSocketStream stream = streams::IOSocketStream::createStream<web::http::HttpNetwork>("127.0.0.1", "8080");
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
		streams::IOSocketStream stream = streams::IOSocketStream::createStream<web::http::HttpNetwork>("127.0.0.1", "8080");
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

static std::unique_ptr<streams::IOSocketStream> webSocketStream;

TEST(WebSocket, UpgradeConnection)
{
	auto getRandomBytes = []() -> std::string
		{
			std::string result(16, '\0');

			for (size_t i = 0; i < result.size(); i++)
			{
				result[i] = static_cast<uint8_t>(rand() % 128);
			}

			return result;
		};

	std::string randomBytes = utility::conversion::encodeBase64(getRandomBytes());
	webSocketStream = std::make_unique<streams::IOSocketStream>(streams::IOSocketStream::createStream<web::http::HttpNetwork>("127.0.0.1", "5050", std::chrono::seconds(0)));
	std::string request = web::HttpBuilder()
		.getRequest()
		.headers
		(
			"Host", "127.0.0.1",
			"Upgrade", "websocket",
			"Connection", "Upgrade",
			"Sec-WebSocket-Key", randomBytes,
			"Sec-WebSocket-Version", 13
		)
		.build();
	std::string response;

	(*webSocketStream) << request;

	(*webSocketStream) >> response;

	web::HttpParser parser(response);

	ASSERT_EQ(parser.getResponseCode(), web::ResponseCodes::switchingProtocols);

	{
		constexpr std::string_view guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

		std::string temp = std::format("{}{}", randomBytes, guid);
		std::vector<uint8_t> output(SHA_DIGEST_LENGTH);

		SHA1(reinterpret_cast<const uint8_t*>(temp.data()), temp.size(), output.data());

		ASSERT_EQ(utility::conversion::encodeBase64(output), parser.getHeaders().at("Sec-WebSocket-Accept"));
	}

	webSocketStream = std::make_unique<streams::IOSocketStream>(streams::IOSocketStream::createStream<web::web_socket::WsNetwork>(std::move(webSocketStream->getNetwork<web::http::HttpNetwork>()), true));
}

TEST(WebSocket, Echo)
{
	auto generateRandomText = [](size_t size) -> std::string
		{
			std::string result;

			result.reserve(size);

			for (size_t i = 0; i < size; i++)
			{
				result += rand() % 26 + 65;
			}

			return result;
		};

	std::vector<size_t> sizes =
	{
		50,
		40'000,
		120'000
	};

	for (size_t size : sizes)
	{
		std::string randomText(generateRandomText(size));
		std::string response;

		(*webSocketStream) << randomText;

		(*webSocketStream) >> response;

		ASSERT_EQ(randomText, response);
	}

	web::web_socket::Frame sendFrame(true, web::web_socket::Frame::OpcodeType::text, "Hello, World!", web::web_socket::Frame::generateMask());
	std::vector<web::web_socket::Frame> echoFrames;

	(*webSocketStream) << sendFrame;

	(*webSocketStream) >> echoFrames;

	ASSERT_EQ(echoFrames.size(), 1);

	web::web_socket::Frame& echoFrame = echoFrames.front();

	ASSERT_EQ(sendFrame.getUnmaskedPayload(), echoFrame.getPayload());
	ASSERT_EQ(sendFrame.getFrameOpcode(), echoFrame.getFrameOpcode());
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

	while (!std::filesystem::exists("web_socket_server_ready.txt"))
	{
		std::cout << "Wait WebSocket server..." << std::endl;

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return RUN_ALL_TESTS();
}
