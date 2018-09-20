#include <cstdio>
#include <iostream>
#include "be.h"
#include "pred.h"
#include "util.h"

namespace {
	const std::map<std::string, BE::Type> Types = {
		{ "and",       BE::Type::AND },
		{ "or",        BE::Type::OR },
		{ "predicate", BE::Type::PRED },
	};

	void parse(Vector<UPtr<BE>>& ops, const Json& spec, std::map<String, UPtr<Predicate>>& predicates, Trigger* trigger)
	{
		if (spec.find("type") == spec.end())
			throw "Invalid trigger spec (type not found)";

		try {
			switch (Types.at(spec["type"])) {
				case BE::Type::AND:
				{
					auto a = util::findArray(spec, "and");
					auto p = std::make_unique<And>();

					for (auto& s : a)
						parse(p->ops, s, predicates, trigger);

					ops.push_back(std::move(p));
				}
					break;

				case BE::Type::OR:
				{
					auto o = util::findArray(spec, "or");
					auto p = std::make_unique<Or>();

					for (auto& s : o)
						parse(p->ops, s, predicates, trigger);

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
				}
					break;
			}
		} catch (std::out_of_range& ex) {
			throw "Invalid trigger spec: " + std::string{ex.what()};
		} catch (std::string& err) {
			throw "Invalid trigger spec: " + err;
		}
	}

	// interleave And/Or
	void interleave(UPtr<BE>& p, BE::Type parentType)
	{
		if (p->type == BE::Type::PRED || p->type != parentType)
			return;

		if (parentType == BE::Type::AND) {
			Or* po = new Or();
			po->ops.push_back(std::move(p));
			p.reset(po);
		} else {
			And* pa = new And();
			pa->ops.push_back(std::move(p));
			p.reset(pa);
		}
	}

	// normalize BE for evaluation
	void normalize(UPtr<BE>& root, std::vector<uint8_t> path)
	{
		Vector<UPtr<BE>>* ops;

		if (root->type == BE::Type::AND) {
			ops = &(static_cast<And*>(root.get())->ops);
		} else {
			ops = &(static_cast<Or*>(root.get())->ops);
		}

		auto max = ops->size();
		if (max > 127)
			throw "BE index overflow";

		for (auto i = 0; i < max; i++) {

			UPtr<BE>& be = (*ops)[i];

			uint8_t n = static_cast<uint8_t>(i);
			if (i == max-1 && root->type == BE::Type::AND) {
				n |= uint8_t(128);
			}

			auto newpath = path;
			newpath.push_back(n);

			if (be->type == BE::Type::PRED) {
				static_cast<Pred*>(be.get())->path.set(newpath);
			} else {
				interleave(be, root->type);
				normalize(be, newpath);
			}
		}
	}

	void print(const UPtr<BE>& be, int index, int indent)
	{
		auto n = indent * 2;

		Vector<UPtr<BE>>* ops;

		if (be->type == BE::Type::AND) {
			ops = &(static_cast<And*>(be.get())->ops);
			printf("%*s%d.AND [\n", n, "", index);
		} else {
			ops = &(static_cast<Or*>(be.get())->ops);
			printf("%*s%d.OR [\n", n, "", index);
		}

		for (auto i = 0; i < ops->size(); i++) {
			UPtr<BE>& be = (*ops)[i];
			if (be->type == BE::Type::PRED) {
				Pred* p = static_cast<Pred*>(be.get());
				printf("%*s%d.PRED %s ( ", n+2, "", i, p->pred->name.c_str());
				for (auto j = 0; j < p->path.n; j++) {
					if (p->path.p[j] & 128) {
						printf("%d* ", p->path.p[j] & 127);
					} else {
						printf("%d ", p->path.p[j]);
					}
				}
				printf(")\n");
			} else {
				print(be, i, indent+1);
			}
		}

		printf("%*s]\n", n, "");
	}
}

void Path::set(const std::vector<uint8_t>& data)
{
	if (data.size() > p.max_size())
		throw "Path length overflow";

	n = data.size();
	for (auto i = 0; i < n; i++)
		p[i] = data[i];
}

Pred::Pred(Predicate* p, Trigger* t)
: BE(Type::PRED), pred(p), trigger(t)
{
	p->preds.push_back(this);
}

UPtr<BE> parseBE(const Json& spec, std::map<String, UPtr<Predicate>>& predicates, Trigger* trigger)
{
	Vector<UPtr<BE>> v;
	parse(v, spec, predicates, trigger);
	if (v.empty()) {
		throw "Invalid trigger (empty spec)";
	}

	// ensure the root is And
	interleave(v[0], BE::Type::OR);

	std::vector<uint8_t> path{};
	normalize(v[0], path);

	return std::move(v[0]);
}

void printBE(const UPtr<BE>& be)
{
	print(be, 0, 0);
}
