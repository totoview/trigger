#include <iostream>
#include <set>
#include "engine.h"
#include "util.h"

namespace
{
	struct Input {
		std::string type;
		std::string name;
	};

	struct Pattern {
	 	std::string pattern;
	};

	Input parseInput(const std::string& name, Json& spec) {
		return Input{util::findString(spec, "type"), util::findString(spec, "name")};
	}

	Pattern parsePattern(const std::string& name, Json& spec) {
		if (util::findString(spec, "type").get<std::string>() != "pattern")
			throw "Invalid pattern value type for " + name;
		return Pattern{util::findString(spec, "value").get<std::string>()};
	}

	bool parseBool(const std::string& name, Json& spec) {
		if (util::findString(spec, "type").get<std::string>() != "bool")
			throw "Invalid boolean value type for " + name;
		return util::findBool(spec, "value").get<bool>();
	}

	int parseInt(const std::string& name, Json& spec) {
		if (util::findString(spec, "type").get<std::string>() != "int")
			throw "Invalid int value type for " + name;
		try {
			return util::findUInt(spec, "value").get<int>();
		} catch (...) {
			return util::findInt(spec, "value").get<int>();
		}
	}
}

Engine::Engine(const std::string& s) {
	auto spec = Json::parse(s);

	// std::cout << spec.dump(4) << std::endl;

	if (!spec.is_object())
		throw "Invalid input data, JSON object expected";

	parseVariables(spec);
	parsePredicates(spec);
	parseTriggers(spec);

	// init matchers
	for (auto& v : variables) {
		if (v.second->matcher)
			v.second->matcher->init();
	}
}

void Engine::parseVariables(const Json& spec)
{
	for (const auto& e : util::findObj(spec, "vars").items()) {
		auto& spec = e.value();
		try {
			auto type = util::findString(spec, "type");
			variables.emplace(std::make_pair(e.key(), std::make_unique<Variable>(e.key(), Variable::Types.at(type.get<std::string>()))));
		} catch (...) {
			throw "Invalid variable spec for " + e.key();
		}
	}
}

void Engine::parsePredicates(const Json& spec)
{
	predicates = std::make_unique<PredMap>();

	for (const auto& e : util::findObj(spec, "predicates").items()) {
		auto& spec = e.value();

		try {
			auto input = util::findArray(spec, "input");
			auto type = Predicate::Types.at(util::findString(spec, "type").get<std::string>());

			switch (type) {
				default:
					throw "Invalid predicate type";
				case Predicate::Type::BOOL_EQ:
				{
					auto var = parseInput(e.key(), input[0]);
					auto val = parseBool(e.key(), input[1]);

					if (variables.find(var.name) == variables.end())
						throw "Invalid predicate (var '" + var.name + "' not found) for " + e.key();

					auto& pv = variables[var.name];
					predicates->emplace(std::make_pair(e.key(), std::make_unique<PBoolEQ>(e.key(), pv.get(), val)));
					pv->predicates.push_back((*predicates)[e.key()].get());
				}
					break;
				case Predicate::Type::INT_LT:
				{
					auto var = parseInput(e.key(), input[0]);
					auto val = parseInt(e.key(), input[1]);

					if (variables.find(var.name) == variables.end())
						throw "Invalid predicate (var '" + var.name + "' not found) for " + e.key();

					auto& pv = variables[var.name];
					predicates->emplace(std::make_pair(e.key(), std::make_unique<PIntLT>(e.key(), pv.get(), val)));
					pv->predicates.push_back((*predicates)[e.key()].get());
				}
					break;
				case Predicate::Type::STRING_MATCH:
				{
					auto var = parseInput(e.key(), input[0]);
					auto pat = parsePattern(e.key(), input[1]);

					if (variables.find(var.name) == variables.end())
						throw "Invalid predicate (var '" + var.name + "' not found) for " + e.key();

					auto& pv = variables[var.name];

					if (!pv->matcher)
						pv->matcher = std::make_unique<Matcher>(pv.get());

					predicates->emplace(std::make_pair(e.key(), std::make_unique<PStringMatch>(e.key(), pv.get(), pat.pattern)));
					pv->matcher->add((*predicates)[e.key()].get());
				}
					break;
			}
		} catch (std::string& err) {
			throw "Invalid predicate spec for " + e.key() + ": " + err;
		} catch (...) {
			throw "Invalid predicate spec for " + e.key();
		}
	}
}

void Engine::parseTriggers(const Json& spec)
{
	for (const auto& e : util::findObj(spec, "triggers").items()) {
		auto& spec = e.value();
		if (!spec.is_object())
			throw "Invalid trigger spec (JSON object expected) for " + e.key();

		try {
			triggers.emplace(std::make_pair(e.key(), std::make_unique<Trigger>(e.key(), spec, *predicates)));
		} catch (...) {
			throw "Invalid trigger spec for " + e.key();
		}
	}
}

Vector<String> Engine::match(Vector<VarValue>& input)
{
	std::set<Predicate*> preds{};
	Vector<Matcher*> matchers{};

	// collect relevant predicates and matchers
	for (const auto& i : input) {
		if (variables.find(i.name) == variables.end())
			throw "Variable not found: " + i.name;

		auto& v = variables[i.name];
		switch (v->type) {
			case Variable::Type::BOOL:
				v->boolValue = std::get<bool>(i.value);
				break;
			case Variable::Type::INT:
				v->intValue = std::get<int>(i.value);
				break;
			case Variable::Type::STRING:
				v->stringValue = std::get<String>(i.value);
				break;
		}
		std::copy(v->predicates.begin(), v->predicates.end(), std::inserter(preds, preds.end()));

		if (v->matcher)
			matchers.push_back(v->matcher.get());
	}

	// evaluate predicates and matchers
	Vector<Predicate*> trueps{};

	for (auto& p : preds) {
#ifdef __DEBUG__
		std::cout << "= matched pred: name=" << p->name << " cid=" << p->cid << '\n';
#endif
		if (p->eval())
			trueps.push_back(p);
	}
	for (auto& m : matchers) {
#ifdef __DEBUG__
		std::cout << "= matched trigger: var=" << m->variable() << '\n';
#endif
		m->match(trueps);
	}

#ifdef __DEBUG
	for (const auto& p : trueps)
		std::cout << "= true: name=" << p->name << " cid=" << p->cid << '\n';
#endif

	// evaluate triggers
	std::map<uint64_t, Trigger*> ts; // group matched preds by trigger
	for (const auto& p : trueps) {
		for (const auto& p2 : p->preds) {
			auto& t = p2->trigger;
			if (ts.find(t->cid) == ts.end()) {
				t->clearMatched();
				ts[t->cid] = t;
			}
			t->addMatched(p2);
		}
	}
#ifdef __DEBUG__
	std::cout << "===== group by triggers ======\n";
	for (const auto& [i, t] : ts)
		t->print();
#endif

	Vector<String> matched{};
	for (const auto& [i, t] : ts) {
		if (t->check())
			matched.push_back(t->name);
	}

	return matched;
}
