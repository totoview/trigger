#include <map>
#include "pred.h"
#include "var.h"

const std::map<std::string, Predicate::Type> Predicate::Types = {
	{ "stringMatch", Predicate::Type::STRING_MATCH },
	{ "intLT",       Predicate::Type::INT_LT },
	{ "boolEQ",      Predicate::Type::BOOL_EQ },
};

// STRING_MATCH

PStringMatch::PStringMatch(const std::string& name, Variable* v, const String& p)
: Predicate(name, Type::STRING_MATCH)
, var(v), pattern(p)
{
	assert(v->type == Variable::Type::STRING);
}

bool PStringMatch::eval() const
{
	return false;
}

// INT_LT

PIntLT::PIntLT(const std::string& name, Variable* v, int n)
: Predicate(name, Type::INT_LT)
, var(v), max(n)
{
	assert(v->type == Variable::Type::INT);
}

bool PIntLT::eval() const
{
	return std::get<int>(var->value) < max;
}

// BOOL_EQ

PBoolEQ::PBoolEQ(const std::string& name, Variable* v, bool f)
: Predicate(name, Type::BOOL_EQ)
, var(v), expected(f)
{
	assert(v->type == Variable::Type::BOOL);
}

bool PBoolEQ::eval() const
{
	return std::get<bool>(var->value) == expected;
}
