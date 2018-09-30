#include <cstdio>
#include "trigger.h"
#include "pred.h"

Trigger::Trigger(const std::string& n,
	const Json& spec,
	std::map<String, UPtr<Predicate>>& predicates)
: name(n), be(parseBE(spec, predicates, this)), matched(new Pred(nullptr, nullptr))
{
}

void Trigger::print() const
{
	printf("[Trigger] id=%lld, name=%s\n", cid, name.c_str());

	printf("= BE:\n");
	printBE(be);

	printf("= Matched:\n");
	for (auto p = matched->next; p; p = p->next)
		printf("    %2d: %s\n", p->index, p->pred->name.c_str());
}

void Trigger::clearMatched()
{
	matched->prev = matched->next = nullptr;
}

void Trigger::addMatched(Pred* p)
{
	Pred* parent = matched.get();
	while (parent->next && parent->next->index < p->index)
		parent = parent->next;

	p->next = parent->next;
	p->prev = parent;
	parent->next = p;
}

bool Trigger::evalAnd(Path sub) const
{
	if (current->path.eq(sub)) // at a leaf
		return true;

	auto result{false};

	while (current->path.startsWith(sub)) {
		result = result || evalAnd(current->path.sub(sub.n+1));
		current = current->next;
	}

	current = current->prev;
	return result;
}

bool Trigger::evalOr(Path sub) const
{
	if (current->path.eq(sub)) // at a leaf
		return true;

	auto result{true}, lastChild{false};
	uint8_t lastExplored{0};

	while (current->path.startsWith(sub)) {
		auto child = current->path.sub(sub.n+1);
		lastExplored++;
		if (child.last() != lastExplored)
			result = false;
		lastChild = child.isLastMarked();
		result = result && evalOr(child);
		current = current->next;
	}

	current = current->prev;
	return result && lastChild;
}

bool Trigger::check() const
{
	current = matched->next;
	return evalAnd(Path{});
}
