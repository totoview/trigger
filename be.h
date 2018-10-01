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

struct PathRef {
	uint8_t last() const { return CLR_MARK((*p)[n-1]); }
	bool isLastMarked() const { return n && IS_MARKED((*p)[n-1]); }

	size_t n;
	std::array<uint8_t, 16>* p;
};

struct Path {
	// set value from a vector
	void set(const std::vector<uint8_t>& data) {
		if (data.size() > p.max_size())
			throw "Path length overflow";

		n = data.size();
		for (auto i = 0; i < n; i++)
			p[i] = data[i];
	}

	// append a new node
	void append(uint8_t v) { p[n++] = v; }

	bool eq(const PathRef& ref) const {
		if (ref.n == n) {
			if (ref.p == &p) return true;
			for (int i = n - 1; i >= 0; --i) {
				if ((*ref.p)[i] != p[i])
					return false;
			}
			return true;
		}
		return false;
	}
	bool startsWith(const PathRef& ref) const {
		if (ref.n <= n) {
			if (ref.p == &p) return true;
			for (int i = ref.n - 1; i >= 0; --i) {
				if ((*ref.p)[i] != p[i])
					return false;
			}
			return true;
		}
		return false;
	}
	PathRef sub(size_t len) { return std::move(PathRef{len, &p}); }

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
