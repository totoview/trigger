#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <thread>
#include "folly/MPMCQueue.h"
#include "common.h"
#include "engine.h"
#include "pool.h"
#include "var.h"

class Service
{
public:
	struct Result
	{
		std::uint64_t reqId;
		Vector<std::uint32_t> triggers;
	};

	using Callback = std::function<void(Result*)>;
	using Input = Vector<std::tuple<int, VarValue>>;

	static const std::uint64_t ERR_REQ_ID{0};

public:
	explicit Service(const std::string& spec, Callback cb);

	void start();
	void shutdown();

	std::uint64_t tryMatch(Input* req);

	std::map<String, int> getVariableNameIndexes() const { return workers[0]->engine.getVariableNameIndexes(); }
	std::map<uint32_t, String> getTriggerIdNames() const { return workers[0]->engine.getTriggerIdNames(); }

private:
	static const int MAX_WORKERS{8};
	static const int BLOCK_SIZE{8192*4};
	static const int MAX_REQS{BLOCK_SIZE * MAX_WORKERS};

	struct Request {
		std::uint64_t reqId;
		Input* input;
	};

	using RequestQueue = folly::MPMCQueue<Request*>;
	using RequestPool = Pool<Request, MAX_REQS, BLOCK_SIZE, 32>;
	using RequestBlock = RequestPool::Block;

	struct Worker
	{
		explicit Worker(int wid, const std::string& spec, RequestQueue& q, RequestPool& p, Callback& cb)
		: id(wid), engine(spec), requestQueue(q), requestPool(p), callback(cb), stop(false)
		{}

		void start();
		void shutdown();

		int id;
		Engine engine;
		RequestQueue& requestQueue;
		RequestPool& requestPool;

		Callback& callback;
		Vector<std::thread> thread;

		bool stop;
		std::mutex mux;
		std::condition_variable stopped;
	};


	std::string triggerSpec;
	Vector<UPtr<Worker>> workers;
	Callback callback;

	std::atomic_uint64_t nextId;
	RequestBlock* dataBlock;
	RequestQueue requestQueue;
	RequestPool requestPool;
};

#endif
