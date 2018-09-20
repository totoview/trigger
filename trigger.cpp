#include "trigger.h"

Trigger::Trigger(const std::string& n,
	const Json& spec,
	std::map<String, UPtr<Predicate>>& predicates)
: name(n), be(parseBE(spec, predicates, this))
{
}