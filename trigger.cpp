#include "trigger.h"
#include "be.h"

Trigger::Trigger(const std::string& n,
	const Json& spec,
	std::map<String, UPtr<Predicate>>& predicates)
: name(n), be(parseBE(spec, predicates, this)), matched(new Pred(nullptr, nullptr))
{
	printBE(be);
}

void Trigger::clearMatched()
{
	matched->prev = matched->next = nullptr;
}

void Trigger::addMatched(Pred* p)
{
	p->next = matched->next;
	p->prev = matched.get();
	matched->next = p;
}
