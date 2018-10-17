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
					ok = requestPool.put(req);
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
: triggerSpec(spec), callback(cb), nextId(1), requestQueue(MAX_REQS)
{
	int n = std::min<int>(std::thread::hardware_concurrency(), 8);
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
	if (!requestPool.get(req))
		return ERR_REQ_ID;

	auto id = nextId.fetch_add(1);
	req->reqId = id;
	req->input = input;

	if (!requestQueue.write(req)) {
		requestPool.put(req);
		return ERR_REQ_ID;
	}
	return id;
}
