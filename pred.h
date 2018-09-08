#ifndef __PRED_H__
#define __PRED_H__

#include <map>
#include "common.h"
#include "comparable.h"

class Variable;
class Trigger;

class Predicate : public Comparable {
public:
	enum struct Type {
		STRING_MATCH,
		INT_LT,
		BOOL_EQ,
	};
	static const std::map<std::string, Type> Types;

public:
	virtual bool eval() const = 0;
	virtual ~Predicate() = default;
	Type type() { return ptype; }

	String name;
	Vector<Trigger*> triggers;

protected:
	Predicate(Type t) : ptype(t) {}
	Type ptype;				// predicate type
};


class PStringMatch : public Predicate {
public:
	PStringMatch(Variable* v, const String& p, bool cs);
	bool eval() const override;

private:
	Variable* var;
	String pattern;
	bool caseSensitive;
};

class PIntLT : public Predicate {
public:
	PIntLT(Variable* v, int n);
	bool eval() const override;

private:
	Variable* var;
	int max;
};

class PBoolEQ : public Predicate {
public:
	PBoolEQ(Variable* v, bool f);
	bool eval() const override;

private:
	Variable* var;
	bool expected;
};

#endif
