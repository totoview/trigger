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

	void parse(Vector<UPtr<BE>>& ops, const Json& spec, std::map<String, UPtr<Predicate>>& predicates)
	{
		if (spec.find("type") == spec.end()) {
			throw "Invalid trigger spec (type not found)";
		}
		try {
			switch (Types.at(spec["type"])) {
				case BE::Type::AND:
				{
					auto a = util::findArray(spec, "and");
					auto p = std::make_unique<And>();
					for (auto& s : a) {
						parse(p->ops, s, predicates);
					}
					ops.push_back(std::move(p));
				}
					break;

				case BE::Type::OR:
				{
					auto o = util::findArray(spec, "or");
					auto p = std::make_unique<Or>();
					for (auto& s : o) {
						parse(p->ops, s, predicates);
					}
					ops.push_back(std::move(p));
				}
					break;

				case BE::Type::PRED:
				{
					auto p = util::findString(spec, "predicate");
					auto predname = p.get<std::string>();
					if (predicates.find(predname) == predicates.end()) {
						throw "Invalid trigger predicate value (no predicate named '" + predname + "')";
					}

					ops.push_back(std::make_unique<Pred>(predicates[predname].get()));
				}
					break;
			}
		} catch (std::out_of_range& ex) {
			throw "Invalid trigger spec: " + std::string{ex.what()};
		} catch (std::string& err) {
			throw "Invalid trigger spec: " + err;
		}
	}
}

UPtr<BE> parseBE(const Json& spec, std::map<String, UPtr<Predicate>>& predicates)
{
	Vector<UPtr<BE>> root;
	parse(root, spec, predicates);
	if (root.size() != 1) {
		throw "Invalid trigger (empty spec)";
	}
	return std::move(root[0]);
}
