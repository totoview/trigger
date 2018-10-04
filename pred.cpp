#include <map>
#include "pred.h"
#include "trigger.h"
#include "var.h"

const std::map<std::string, Predicate::Type> Predicate::Types = {
	{ "stringMatch", Predicate::Type::STRING_MATCH },
	{ "intLT",       Predicate::Type::INT_LT },
	{ "boolEQ",      Predicate::Type::BOOL_EQ },
};

void Predicate::init()
{
	for (auto& p : preds) {
		auto it = std::find_if(triggers.begin(), triggers.end(), [&p](TriggerData& td) {
			return td.trigger == p->trigger;
		});
		if (it == triggers.end())
			triggers.emplace_back(p->trigger, MATCHED[p->index]);
		else
			it->matched |= MATCHED[p->index];
	}
}

void Predicate::apply()
{
	for (auto& t : triggers)
		t.trigger->addMatched(t.matched);
}

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
	return var->intValue < max;
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
	return var->boolValue == expected;
}
