#ifndef _BE_H__
#define _BE_H__

#include <map>
#include "common.h"

class Predicate;
class Trigger;

#define MAX_INDEX    127
#define MARK(n)      ((n)| uint8_t(128))
#define IS_MARKED(n) ((n) & 128)
#define CLR_MARK(n)  ((n) & 127)

struct Path {
	// set value from a vector
	void set(const std::vector<uint8_t>& p);
	// append a new node
	void append(uint8_t v) { p[n++] = v; }
	// check equality
	bool eq(const Path& rhs) const;
	// check if rhs is a prefix
	bool startsWith(const Path& rhs) const;
	// get the last node (without mark bit)
	uint8_t last() const { return CLR_MARK(p[n-1]); }
	// check if the last node is marked
	bool isLastMarked() const { return n && IS_MARKED(p[n-1]); }
	// get a subpath of length len
	Path sub(size_t len) const { Path path; path.n = n; path.p = p; return std::move(path); }

	size_t n{0};
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
