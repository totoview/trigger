#ifndef __MATCHER_H__
#define __MATCHER_H__

#include <hs/hs.h>
#include "common.h"

class Variable;
class Predicate;

class Matcher {
public:
	Matcher(Variable* v) : var(v) {}
	~Matcher();

	String variable() const;
	void add(Predicate* p) { predicates.push_back(p); }
	void init();

	size_t match(Vector<Predicate*>& c);
	void onMatched(unsigned int id, unsigned long long from, unsigned long long to, unsigned int flags);

private:
	Variable* var;
	Vector<Predicate*> predicates;
	hs_database_t* db{nullptr};
	hs_scratch_t* scratch{nullptr};
	size_t matchCnt;
	Vector<Predicate*> *results;
};

#endif
