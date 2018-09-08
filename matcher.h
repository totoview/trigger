#ifndef __MATCHER_H__
#define __MATCHER_H__

#include "common.h"

class Variable;
class Predicate;

class Matcher {
public:
	Matcher(Variable* v) : var(v) {}
	void add(Predicate* p) { predicates.push_back(p); }

private:
	Variable* var;
	Vector<Predicate*> predicates;
};

#endif
