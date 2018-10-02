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
			throw "Invalid trigger spec (type not found)";

		try {
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
						throw "Invalid trigger predicate value (no predicate named '" + predname + "')";

					ops.push_back(std::make_unique<Pred>(predicates[predname].get(), trigger));
					cnt++;
				}
					break;
			}
			return cnt;
		} catch (std::out_of_range& ex) {
			throw "Invalid trigger spec: " + std::string{ex.what()};
		} catch (std::string& err) {
			throw "Invalid trigger spec: " + err;
		}
	}

	void assignPredIndex(UPtr<BE>& root, int& cur, Trigger* trigger)
	{
		for (auto& be : *root->children()) {
			if (be->type == BE::Type::PRED) {
				trigger->preds[cur] = static_cast<Pred*>(be.get());
				trigger->preds[cur]->index = cur;
				if (cur++ > MAX_LEAVES)
					throw "Too many leaf nodes";
			} else
				assignPredIndex(be, cur, trigger);
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
					int size = (children[i]->type == BE::Type::PRED ? 1 : children[i]->children()->size());
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

		printf("%*s%d.%s #leaves=%d [\n", n, "", index, be->type == BE::Type::AND ? "AND" : "OR", be->leaves);

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
		throw "Invalid trigger (empty spec)";

	if (v[0]->type == BE::Type::PRED)
		throw "Root must be AND or OR";

	int index = 0;
	assignPredIndex(v[0], index, trigger);
	assignLeaves(v[0]);
	assignInterval(v[0], 0, 1, MAX_INTERVAL);

	return std::move(v[0]);
}

void printBE(const UPtr<BE>& be)
{
	print(be, 0, 0);
}
