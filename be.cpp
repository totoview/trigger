#include <cstdio>
#include <iostream>
#include "be.h"
#include "pred.h"
#include "trigger.h"
#include "util.h"

namespace
{
	const std::map<std::string, BE::Type> Types = {
		{ "and",       BE::Type::AND },
		{ "or",        BE::Type::OR },
		{ "predicate", BE::Type::PRED },
	};

	// return total number of PRED parsed
	int parse(Vector<UPtr<BE>>& ops, const Json& spec, std::map<String, UPtr<Predicate>>& predicates, Trigger* trigger)
	{
		int cnt;
		if (spec.find("type") == spec.end())
			throw std::runtime_error{"Invalid trigger spec (type not found)"};

		switch (Types.at(spec["type"])) {
			case BE::Type::AND:
			{
				auto a = util::findArray(spec, "and");
				auto p = std::make_unique<And>();

				for (auto& s : a)
					cnt += parse(p->ops, s, predicates, trigger);

				ops.push_back(std::move(p));
			}
				break;

			case BE::Type::OR:
			{
				auto o = util::findArray(spec, "or");
				auto p = std::make_unique<Or>();

				for (auto& s : o)
					cnt += parse(p->ops, s, predicates, trigger);

				ops.push_back(std::move(p));
			}
				break;

			case BE::Type::PRED:
			{
				auto p = util::findString(spec, "predicate");
				auto predname = p.get<std::string>();

				if (predicates.find(predname) == predicates.end())
					throw std::runtime_error{"Invalid trigger predicate value (no predicate named '" + predname + "')"};

				ops.push_back(std::make_unique<Pred>(predicates[predname].get(), trigger));
				cnt++;
			}
				break;
		}
		return cnt;
	}

	void collectPreds(UPtr<BE>& root, Vector<Pred*>& preds)
	{
		for (auto& be : *root->children()) {
			if (be->type == BE::Type::PRED)
				preds.push_back(static_cast<Pred*>(be.get()));
			else
				collectPreds(be, preds);
		}
	}

	void assignPredIndex(UPtr<BE>& root, Trigger* trigger)
	{
		// collect all the predicates first
		Vector<Pred*> preds;
		collectPreds(root, preds);

		// sort predicates based on (intvStart, intvEnd)
		std::sort(preds.begin(), preds.end(), [](Pred* a, Pred* b) {
			return a->intvStart < b->intvStart || (a->intvStart == b->intvStart && a->intvEnd < b->intvEnd);
		});

		if (preds.size() > MAX_LEAVES)
			throw std::runtime_error{"too many predicates"};

		for (int i = 0; i < preds.size(); i++) {
			preds[i]->index = i;
			trigger->preds[i] = preds[i];
		}
	}

	void assignLeaves(UPtr<BE>& root)
	{
		for (auto& be : *root->children()) {
			if (be->type != BE::Type::PRED)
				assignLeaves(be);
		}
		root->leaves = 0;
		for (auto& be : *root->children())
			root->leaves += be->leaves;
	}

	void assignInterval(UPtr<BE>& root, int leftLeaves, int start, int end)
	{

		if (root->type == BE::Type::PRED) {
			static_cast<Pred*>(root.get())->setInterval(start, end);
			return;
		}

		if (root->type == BE::Type::OR) {
			int left = leftLeaves;
			for (auto& be : *root->children()) {
				assignInterval(be, left, start, end);
				left += be->leaves;
			}
		} else {

			auto& children = *root->children();
			auto cnt = children.size();

			if (cnt == 1) {
				assignInterval(children[0], leftLeaves, start, end);
			} else {
				// first node
				int left = leftLeaves;
				int cur = leftLeaves + children[0]->leaves;
				assignInterval(children[0], left, start, cur++);
				left += children[0]->leaves;

				for (int i = 1; i < cnt-1; i++) {
					int size = children[i]->leaves;
					assignInterval(children[i], left, cur, cur+size-1);
					left += children[i]->leaves;
					cur += size;
				}

				// last node
				assignInterval(children[cnt-1], left, cur, end);
			}
		}
	}

	void print(const UPtr<BE>& be, int index, int indent)
	{
		auto n = indent * 2;
		Vector<UPtr<BE>>* ops = be->children();

		printf("%*s%d.%s leaves=%d [\n", n, "", index, be->type == BE::Type::AND ? "AND" : "OR", be->leaves);

		for (size_t max = ops->size(), i = 0; i < max; i++) {
			UPtr<BE>& be = (*ops)[i];

			if (be->type == BE::Type::PRED) {
				Pred* p = static_cast<Pred*>(be.get());
				printf("%*s%zu.PRED %s index=%d [%d, %d]\n", n+2, "", i, p->pred->name.c_str(), p->index, p->intvStart, p->intvEnd);
			} else
				print(be, i, indent+1);
		}

		printf("%*s]\n", n, "");
	}
}

Pred::Pred(Predicate* p, Trigger* t)
: BE(Type::PRED), pred(p), trigger(t)
{
	leaves = 1;
	if (p)
		p->preds.push_back(this);
}

UPtr<BE> parseBE(const Json& spec, std::map<String, UPtr<Predicate>>& predicates, Trigger* trigger)
{
	Vector<UPtr<BE>> v;
	trigger->totalLeaves = parse(v, spec, predicates, trigger);

	if (v.empty())
		throw std::runtime_error{"Invalid trigger (empty spec)"};

	if (v[0]->type == BE::Type::PRED)
		throw std::runtime_error{"Root must be AND or OR"};

	assignLeaves(v[0]);
	assignInterval(v[0], 0, 1, MAX_INTERVAL_END);
	assignPredIndex(v[0], trigger);

	return std::move(v[0]);
}

void printBE(const UPtr<BE>& be)
{
	print(be, 0, 0);
}
