#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include "common.h"
#include "comparable.h"
#include "be.h"

struct Pred;

extern uint64_t MATCHED[];

class Trigger : public Comparable<Trigger> {
public:
	Trigger(const std::string& name,
		const Json& spec,
		std::map<String, UPtr<Predicate>>& predicates);
	void print() const;
	bool check(); // evaluation

	String name;
	UPtr<BE> be;
	int totalLeaves{0};

	void clearMatched() { matched = 0; }
	void addMatched(Pred* p) { matched |= p->flagMatched; }
	bool hasMatched() const { return matched != 0; }

	uint64_t matched{0};
	std::array<Pred*, 64> preds;
	std::array<uint64_t, 64> starts;
	std::array<uint64_t, 64> ends;
};

#endif
