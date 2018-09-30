#ifndef _BE_H__
#define _BE_H__

#include <map>
#include "common.h"

class Predicate;
class Trigger;

struct Path {
	void set(const std::vector<uint8_t>& p);
	size_t n;
	std::array<uint8_t, 16> p;
};

struct BE {
	enum struct Type {
		AND,
		OR,
		PRED
	};
	Type type;
	virtual ~BE() = default;

protected:
	BE(Type t) : type(t) {}
};

struct And : BE {
	And() : BE(Type::AND) {}
	Vector<UPtr<BE>> ops; // operands
};

struct Or : BE {
	Or() : BE(Type::OR) {}
	Vector<UPtr<BE>> ops; // operands
};

struct Pred : BE {
	Pred(Predicate* p, Trigger* t);

	Predicate* pred;
	Trigger* trigger;
	Path path;

	// used by Trigger for evaluation
	int index{-1}; // index of sorted predicates
	Pred* prev{nullptr};
	Pred* next{nullptr};
};

UPtr<BE> parseBE(const Json& spec, std::map<String, UPtr<Predicate>>& predicates, Trigger* trigger);
void printBE(const UPtr<BE>& be);

#endif
