#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include "common.h"
#include "comparable.h"
#include "be.h"

struct Pred;

class Trigger : public Comparable<Trigger> {
public:
	Trigger(const std::string& name,
		const Json& spec,
		std::map<String, UPtr<Predicate>>& predicates);
	void print() const;
	bool check() const; // evaluation

	String name;
	UPtr<BE> be;

	void clearMatched();
	void addMatched(Pred* p);
	bool hasMatched() const { return matched->next != nullptr; }


private:
	bool evalAnd(Path p) const;
	bool evalOr(Path p) const;

	UPtr<Pred> matched;			// list of matched predicates, sorted
	mutable Pred* current;		// the current predicate under evaluation
};

#endif
