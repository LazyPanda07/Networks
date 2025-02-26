#include "LargeBodyHandler.h"

#include "Network.h"

using namespace std;

namespace web
{
	void LargeBodyHandler::loadLoop(string initialData, size_t bodySize)
	{
		string data(chunkSize, '\0');
		bool endOfStream = false;

		if (initialData.size())
		{
			running = this->handleChunk(initialData, bodySize);
		}

		while (running && bodySize)
		{
			int dataSize = network.receiveBytes(data.data(), static_cast<int>(chunkSize), endOfStream, NULL);

			if (endOfStream)
			{
				break;
			}

			bodySize -= dataSize;

			running = this->handleChunk(string_view(data.data(), static_cast<size_t>(dataSize)), bodySize);
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
		chunkSize(0),
		network(network)
	{

	}

	void LargeBodyHandler::run(const utility::ContainerWrapper& data, size_t bodySize, const string& initialData)
	{
		parser = web::HTTPParser(data.data());
		running = true;

		this->onParseHeaders();

		runThread = async(launch::async, &LargeBodyHandler::loadLoop, this, initialData, bodySize);
	}

	bool LargeBodyHandler::isRunning() const
	{
		return running;
	}

	void LargeBodyHandler::setChunkSize(size_t chunkSize)
	{
		this->chunkSize = chunkSize;
	}
}
