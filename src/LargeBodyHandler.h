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
		std::atomic_int64_t currentReceive;
		int64_t contentLength;
		size_t chunkSize;
		Network& network;

	private:
		void loadLoop(std::string initialData);

	protected:
		virtual bool handleChunk(std::string_view data) = 0;

		virtual void onParseHeaders();

		virtual void onFinishHandleChunks();

	public:
		LargeBodyHandler(Network& network);

		void run(const utility::ContainerWrapper& data, const std::string& initialData = "");

		bool isRunning() const;

		bool isLast() const;

		int64_t getRemainingSize() const;

		void setChunkSize(size_t chunkSize);

		virtual WaitBehavior getWaitBehavior() const = 0;

		virtual ~LargeBodyHandler() = default;
	};
}
