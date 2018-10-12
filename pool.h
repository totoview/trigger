#ifndef __POOL_H__
#define __POOL_H__

#include "common.h"
#include "folly/MPMCQueue.h"

template<typename T, int size>
class Pool
{
public:
	Pool() : pool(size) {
		for (auto i = 0; i < size; i++) {
			objs.emplace_back(std::make_unique<T>());
			put(objs[i].get());
		}
	}

	bool get(T*& p) { return pool.read(p); }
	bool put(T* p) { return pool.write(p); }

private:
	Vector<UPtr<T>> objs;
	folly::MPMCQueue<T*> pool;
};

#endif
