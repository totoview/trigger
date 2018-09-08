#include <map>
#include "pred.h"
#include "var.h"

const std::map<std::string, Predicate::Type> Predicate::Types = {
	{ "stringMatch", Predicate::Type::STRING_MATCH },
	{ "intLT",       Predicate::Type::INT_LT },
	{ "boolEQ",      Predicate::Type::BOOL_EQ },
};

// STRING_MATCH

PStringMatch::PStringMatch(Variable* v, const String& p, bool cs)
: Predicate(Type::STRING_MATCH)
, var(v), pattern(p), caseSensitive(cs)
{
	assert(v->type == Variable::Type::STRING);
}

bool PStringMatch::eval() const
{
	return false;
}

// INT_LT

PIntLT::PIntLT(Variable* v, int n)
: Predicate(Type::INT_LT)
, var(v), max(n)
{
	assert(v->type == Variable::Type::INT);
}

bool PIntLT::eval() const
{
	return var->intValue < max;
}

// BOOL_EQ

PBoolEQ::PBoolEQ(Variable* v, bool f)
: Predicate(Type::BOOL_EQ)
, var(v), expected(f)
{
	assert(v->type == Variable::Type::BOOL);
}

bool PBoolEQ::eval() const
{
	return var->boolValue == expected;
}
