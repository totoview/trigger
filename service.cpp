#include <iostream>
#include "service.h"


void Service::Worker::start()
{
	if (!thread.empty()) return;

	thread.emplace_back([this]() {
		Request* req;
		Result res;
		bool ok;

		for (;;) {
			{
				std::lock_guard<std::mutex> lock(mux);
				if (stop) break;
			}
			for (int i = 0; i < 100'000; i++) {
				if (requestQueue.read(req)) {
					res.reqId = req->reqId;
					res.triggers.clear();
					engine.match(*req->input, res.triggers);

					if (!freeBlock) {
						ok = requestPool.getFree(freeBlock);
						assert(ok);
					}

					freeBlock->emplace_back(req);
					if (freeBlock->size() == BLOCK_SIZE) {
						ok = requestPool.putData(freeBlock);
						assert(ok);
						freeBlock = nullptr;
					}

					assert(ok);
					callback(&res);
				} else
					break;
			}
		}
		stopped.notify_all();
	});
}

void Service::Worker::shutdown()
{
	if (thread.empty()) return;

	std::unique_lock<std::mutex> lock(mux);
	stop = true;
	stopped.wait(lock);
	thread[0].join();
}

Service::Service(const std::string& spec, Callback cb)
: triggerSpec(spec), callback(cb), nextId(1), requestQueue(MAX_REQS), dataBlock(nullptr)
{
	int n = std::min<int>(std::thread::hardware_concurrency(), MAX_WORKERS);
	for (auto i = 0; i < n; i++)
		workers.emplace_back(std::make_unique<Worker>(i, triggerSpec, requestQueue, requestPool, callback));
}

void Service::start()
{
	std::cout << "Starting " << workers.size() << " workers\n";
	for (auto& w : workers)
		w->start();
}

void Service::shutdown()
{
	for (auto& w : workers)
		w->shutdown();
}

std::uint64_t Service::tryMatch(Input* input)
{
	Request* req;

	if (!dataBlock) {
		if (!requestPool.getData(dataBlock))
			return ERR_REQ_ID;
	}

	req = dataBlock->back();
	dataBlock->pop_back();

	auto id = nextId.fetch_add(1);
	req->reqId = id;
	req->input = input;

	if (!requestQueue.write(req)) {
		dataBlock->emplace_back(req);
		return ERR_REQ_ID;
	}

	if (dataBlock->empty()) {
		requestPool.putFree(dataBlock);
		dataBlock = nullptr;
	}

	return id;
}
