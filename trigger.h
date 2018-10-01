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
	bool check(); // evaluation

	String name;
	UPtr<BE> be;

	void clearMatched();
	void addMatched(Pred* p);
	bool hasMatched() const { return matched->next != nullptr; }


private:
	bool evalAnd(PathRef& p);
	bool evalOr(PathRef& p);

	UPtr<Pred> matched;			// list of matched predicates, sorted
	mutable Pred* current;		// the current predicate under evaluation
};

#endif
