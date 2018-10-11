#ifndef __COMPARABLE_H__
#define __COMPARABLE_H__

#include <atomic>

template <typename T>
struct Comparable {
	auto operator==(const Comparable& rhs) const {
		return cid == rhs.cid;
	}
	auto operator<(const Comparable& rhs) const {
		return cid < rhs.cid;
	}

	uint32_t cid{cid_seed.fetch_add(1)};

private:
	static std::atomic_uint32_t cid_seed;
};

template <typename T>
std::atomic_uint32_t Comparable<T>::cid_seed{0};

#endif
