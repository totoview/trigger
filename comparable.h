#ifndef __COMPARABLE_H__
#define __COMPARABLE_H__

#include <atomic>

struct Comparable {
	auto operator==(const Comparable& rhs) const {
		return cid == rhs.cid;
	}
	auto operator<(const Comparable& rhs) const {
		return cid < rhs.cid;
	}

	uint64_t cid{cid_seed.fetch_add(1)};

private:
	static std::atomic_uint64_t cid_seed;
};

#endif
