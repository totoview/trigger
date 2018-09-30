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

	String name;
	UPtr<BE> be;

	void clearMatched();
	void addMatched(Pred* p);

	UPtr<Pred> matched; // list of matched predicates, sorted
};

#endif
