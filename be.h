#ifndef _BE_H__
#define _BE_H__

#include <map>
#include "common.h"

class Predicate;
class Trigger;

#define MAX_LEAVES    8 * sizeof(uint64_t)
#define MAX_INTERVAL  128

struct BE {
	enum struct Type {
		AND,
		OR,
		PRED
	};
	Type type;
	int leaves;			// number of children leaves
	virtual ~BE() = default;
	virtual Vector<UPtr<BE>>* children() = 0;

protected:
	BE(Type t) : type(t) {}
};

struct And : BE {
	And() : BE(Type::AND) {}
	Vector<UPtr<BE>>* children() override { return &ops; }
	Vector<UPtr<BE>> ops; // operands
};

struct Or : BE {
	Or() : BE(Type::OR) {}
	Vector<UPtr<BE>>* children() override { return &ops; }
	Vector<UPtr<BE>> ops; // operands
};

struct Pred : BE {
	Pred(Predicate* p, Trigger* t);
	Vector<UPtr<BE>>* children() override { return nullptr; }
	void setInterval(int start, int end) { intvStart = start; intvEnd = end; }

	Predicate* pred;
	Trigger* trigger;

	// used by Trigger for evaluation
	int index{-1};  // index of sorted predicates
	int intvStart;  // interval start
	int intvEnd;    // interval end
};

UPtr<BE> parseBE(const Json& spec, std::map<String, UPtr<Predicate>>& predicates, Trigger* trigger);
void printBE(const UPtr<BE>& be);

#endif
