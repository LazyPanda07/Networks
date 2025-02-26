#pragma once

#include <future>

#include "NetworksUtility.h"
#include "ContainerWrapper.h"
#include "HTTPParser.h"

namespace web
{
	class Network;

	class NETWORKS_API LargeBodyHandler
	{
	public:
		enum class WaitBehavior
		{
			wait,
			exit
		};

	protected:
		web::HTTPParser parser;
		std::future<void> runThread;
		std::atomic_bool running;
		size_t chunkSize;
		Network& network;

	private:
		void loadLoop(std::string initialData, size_t bodySize);

	protected:
		virtual bool handleChunk(std::string_view data, size_t bodySize) = 0;

		virtual void onParseHeaders();

		virtual void onFinishHandleChunks();

	public:
		LargeBodyHandler(Network& network);

		void run(const utility::ContainerWrapper& data, size_t bodySize, const std::string& initialData = "");

		bool isRunning() const;

		void setChunkSize(size_t chunkSize);

		virtual WaitBehavior getWaitBehavior() const = 0;

		virtual ~LargeBodyHandler() = default;
	};
}
