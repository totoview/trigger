#ifndef __POOL_H__
#define __POOL_H__

#include "common.h"
#include "folly/MPMCQueue.h"

template<typename T, int size, int blockSize, int initFreeBlocks>
class Pool
{
public:
	using Block = std::vector<T*>;
public:
	Pool() : dataBlockPool(size/blockSize + initFreeBlocks), freeBlockPool(size/blockSize + initFreeBlocks) {

		static_assert(size % blockSize == 0);

		UPtr<Block> block = std::make_unique<Block>();
		block->reserve(blockSize);

		for (auto i = 0; i < size; i++) {
			objs.emplace_back(std::make_unique<T>());

			block->emplace_back(objs[i].get());
			if (block->size() == blockSize) {
				putData(block.get());
				blocks.emplace_back(std::move(block));
				block = std::make_unique<Block>();
				block->reserve(blockSize);
			}
		}

		for (auto i = 0; i < initFreeBlocks; i++) {
			UPtr<Block> block = std::make_unique<Block>();
			block->reserve(blockSize);
			putFree(block.get());
			blocks.emplace_back(std::move(block));
		}
	}

	bool getData(Block*& p) { return dataBlockPool.read(p); }
	bool putData(Block* p) { return dataBlockPool.write(p); }

	bool getFree(Block*& p) { return freeBlockPool.read(p); }
	bool putFree(Block* p) { return freeBlockPool.write(p); }

private:
	Vector<UPtr<T>> objs;
	Vector<UPtr<Block>> blocks;
	folly::MPMCQueue<Block*> dataBlockPool;
	folly::MPMCQueue<Block*> freeBlockPool;
};

#endif
