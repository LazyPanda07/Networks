#include "LargeBodyHandler.h"

#include "Network.h"

using namespace std;

namespace web
{
	void LargeBodyHandler::loadLoop(string initialData)
	{
		string data(chunkSize, '\0');
		bool endOfStream = false;

		if (initialData.size())
		{
			running = this->handleChunk(initialData);
		}

		while (running)
		{
			int dataSize = network.receiveBytes(data.data(), static_cast<int>(chunkSize), endOfStream, NULL);

			if (endOfStream)
			{
				break;
			}

			currentReceive += dataSize;

			running = this->handleChunk(string_view(data.data(), static_cast<size_t>(dataSize)));
		}

		this->onFinishHandleChunks();

		running = false;
	}

	void LargeBodyHandler::onParseHeaders()
	{

	}

	void LargeBodyHandler::onFinishHandleChunks()
	{

	}

	LargeBodyHandler::LargeBodyHandler(Network& network) :
		contentLength(0),
		chunkSize(0),
		network(network)
	{

	}

	void LargeBodyHandler::run(const utility::ContainerWrapper& data, const string& initialData)
	{
		parser = web::HTTPParser(data.data());
		running = true;
		currentReceive = 0;
		contentLength = stoll(parser.getHeaders().at("Content-Length"));

		this->onParseHeaders();

		runThread = async(launch::async, &LargeBodyHandler::loadLoop, this, initialData);
	}

	bool LargeBodyHandler::isRunning() const
	{
		return running;
	}

	bool LargeBodyHandler::isLast() const
	{
		return currentReceive == contentLength;
	}

	int64_t LargeBodyHandler::getRemainingSize() const
	{
		return contentLength - currentReceive;
	}

	void LargeBodyHandler::setChunkSize(size_t chunkSize)
	{
		this->chunkSize = chunkSize;
	}
}
