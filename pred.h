#ifndef __PRED_H__
#define __PRED_H__

#include <map>
#include "common.h"
#include "comparable.h"

class Variable;
class Pred;
class Trigger;

class Predicate : public Comparable<Predicate> {
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
	void init();
	void apply();

	String name;
	Vector<Pred*> preds;

protected:
	Predicate(const std::string& n, Type t) : name(n), ptype(t) {}
	Type ptype;				// predicate type
private:
	struct TriggerData {
		explicit TriggerData(Trigger* p, uint64_t m) : trigger(p), matched(m) {}
		Trigger* trigger;
		uint64_t matched;
	};
	Vector<TriggerData> triggers;
};


class PStringMatch : public Predicate {
public:
	PStringMatch(const std::string& name, Variable* var, const String& pat);
	bool eval() const override;
	const char* patternString() const { return pattern.c_str(); }

private:
	Variable* var;
	String pattern;
};

class PIntLT : public Predicate {
public:
	PIntLT(const std::string& name, Variable* v, int n);
	bool eval() const override;

private:
	Variable* var;
	int max;
};

class PBoolEQ : public Predicate {
public:
	PBoolEQ(const std::string& name, Variable* v, bool f);
	bool eval() const override;

private:
	Variable* var;
	bool expected;
};

#endif
