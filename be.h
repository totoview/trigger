#ifndef _BE_H__
#define _BE_H__

#include <map>
#include "common.h"

class Predicate;

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
	Pred(Predicate* p) : BE(Type::PRED), pred(p) {}
	Predicate* pred;
};

UPtr<BE> parseBE(const Json& spec, std::map<String, UPtr<Predicate>>& predicates);

#endif
