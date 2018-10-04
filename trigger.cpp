#include <cstdio>
#include "trigger.h"
#include "pred.h"


uint64_t MATCHED[] = {
	uint64_t(1) << 0,  uint64_t(1) << 1,  uint64_t(1) << 2,  uint64_t(1) << 3,
	uint64_t(1) << 4,  uint64_t(1) << 5,  uint64_t(1) << 6,  uint64_t(1) << 7,
	uint64_t(1) << 8,  uint64_t(1) << 9,  uint64_t(1) << 10, uint64_t(1) << 11,
	uint64_t(1) << 12, uint64_t(1) << 13, uint64_t(1) << 14, uint64_t(1) << 15,
	uint64_t(1) << 16, uint64_t(1) << 17, uint64_t(1) << 18, uint64_t(1) << 19,
	uint64_t(1) << 20, uint64_t(1) << 21, uint64_t(1) << 22, uint64_t(1) << 23,
	uint64_t(1) << 24, uint64_t(1) << 25, uint64_t(1) << 26, uint64_t(1) << 27,
	uint64_t(1) << 28, uint64_t(1) << 29, uint64_t(1) << 30, uint64_t(1) << 31,
	uint64_t(1) << 32, uint64_t(1) << 33, uint64_t(1) << 34, uint64_t(1) << 35,
	uint64_t(1) << 36, uint64_t(1) << 37, uint64_t(1) << 38, uint64_t(1) << 39,
	uint64_t(1) << 40, uint64_t(1) << 41, uint64_t(1) << 42, uint64_t(1) << 43,
	uint64_t(1) << 44, uint64_t(1) << 45, uint64_t(1) << 46, uint64_t(1) << 47,
	uint64_t(1) << 48, uint64_t(1) << 49, uint64_t(1) << 50, uint64_t(1) << 51,
	uint64_t(1) << 52, uint64_t(1) << 53, uint64_t(1) << 54, uint64_t(1) << 55,
	uint64_t(1) << 56, uint64_t(1) << 57, uint64_t(1) << 58, uint64_t(1) << 59,
	uint64_t(1) << 60, uint64_t(1) << 61, uint64_t(1) << 62, uint64_t(1) << 63,
};

uint64_t MASK[] = {
	~(uint64_t(1) << 0),  ~(uint64_t(1) << 1),  ~(uint64_t(1) << 2),  ~(uint64_t(1) << 3),
	~(uint64_t(1) << 4),  ~(uint64_t(1) << 5),  ~(uint64_t(1) << 6),  ~(uint64_t(1) << 7),
	~(uint64_t(1) << 8),  ~(uint64_t(1) << 9),  ~(uint64_t(1) << 10), ~(uint64_t(1) << 11),
	~(uint64_t(1) << 12), ~(uint64_t(1) << 13), ~(uint64_t(1) << 14), ~(uint64_t(1) << 15),
	~(uint64_t(1) << 16), ~(uint64_t(1) << 17), ~(uint64_t(1) << 18), ~(uint64_t(1) << 19),
	~(uint64_t(1) << 20), ~(uint64_t(1) << 21), ~(uint64_t(1) << 22), ~(uint64_t(1) << 23),
	~(uint64_t(1) << 24), ~(uint64_t(1) << 25), ~(uint64_t(1) << 26), ~(uint64_t(1) << 27),
	~(uint64_t(1) << 28), ~(uint64_t(1) << 29), ~(uint64_t(1) << 30), ~(uint64_t(1) << 31),
	~(uint64_t(1) << 32), ~(uint64_t(1) << 33), ~(uint64_t(1) << 34), ~(uint64_t(1) << 35),
	~(uint64_t(1) << 36), ~(uint64_t(1) << 37), ~(uint64_t(1) << 38), ~(uint64_t(1) << 39),
	~(uint64_t(1) << 40), ~(uint64_t(1) << 41), ~(uint64_t(1) << 42), ~(uint64_t(1) << 43),
	~(uint64_t(1) << 44), ~(uint64_t(1) << 45), ~(uint64_t(1) << 46), ~(uint64_t(1) << 47),
	~(uint64_t(1) << 48), ~(uint64_t(1) << 49), ~(uint64_t(1) << 50), ~(uint64_t(1) << 51),
	~(uint64_t(1) << 52), ~(uint64_t(1) << 53), ~(uint64_t(1) << 54), ~(uint64_t(1) << 55),
	~(uint64_t(1) << 56), ~(uint64_t(1) << 57), ~(uint64_t(1) << 58), ~(uint64_t(1) << 59),
	~(uint64_t(1) << 60), ~(uint64_t(1) << 61), ~(uint64_t(1) << 62), ~(uint64_t(1) << 63),
};

Trigger::Trigger(const std::string& n,
	const Json& spec,
	std::map<String, UPtr<Predicate>>& predicates)
: name(n), be(parseBE(spec, predicates, this))
{
}

void Trigger::print() const
{
	printf("[Trigger] id=%lld, name=%s totalLeaves=%d\n", cid, name.c_str(), totalLeaves);

	printf("= BE:\n");
	printBE(be);

	printf("= Matched:\n");
	for (int i = 0; i < MAX_LEAVES; i++) {
		if (matched & MATCHED[i])
			printf("    %2d: %s\n", preds[i]->index, preds[i]->pred->name.c_str());
	}
}

bool Trigger::check()
{
	if (matched == 0) return false;

	uint64_t f = MATCHED[0];
	uint64_t m = matched;
	for (int i = 0; m && i < MAX_LEAVES; i++) {
		if (m & MATCHED[i]) {
			if (f & MATCHED[preds[i]->intvStart-1])
				f |= MATCHED[preds[i]->intvEnd];
			m &= MASK[i];
		}
	}
	return f & MATCHED[63];
}
