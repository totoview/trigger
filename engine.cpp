#include <chrono>
#include <iostream>
#include <tuple>
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
			throw std::runtime_error{"Invalid pattern value type for " + name};
		return Pattern{util::findString(spec, "value").get<std::string>()};
	}

	bool parseBool(const std::string& name, Json& spec) {
		if (util::findString(spec, "type").get<std::string>() != "bool")
			throw std::runtime_error{"Invalid boolean value type for " + name};
		return util::findBool(spec, "value").get<bool>();
	}

	int parseInt(const std::string& name, Json& spec) {
		if (util::findString(spec, "type").get<std::string>() != "int")
			throw std::runtime_error{"Invalid int value type for " + name};
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
		throw std::runtime_error{"Invalid input data, JSON object expected"};

	parseVariables(spec);
	parsePredicates(spec);
	parseTriggers(spec);

	// init matchers
	for (auto& v : variables) {
		if (v->matcher)
			v->matcher->init();
	}

	matchedPreds.reserve(512);
	matchedTriggers.reserve(512);
}

Variable* Engine::findVariable(const std::string& name) const
{
	for (const auto& v : variables) {
		if (v->name == name)
			return v.get();
	}
	return nullptr;
}

void Engine::parseVariables(const Json& spec)
{
	for (const auto& e : util::findObj(spec, "vars").items()) {
		auto& spec = e.value();
		try {
			auto type = util::findString(spec, "type");
			variables.push_back(std::make_unique<Variable>(e.key(), Variable::Types.at(type.get<std::string>())));
		} catch (std::exception& err) {
			throw std::runtime_error{"Invalid variable spec for " + e.key() + ": " + err.what()};
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
			auto var = parseInput(e.key(), input[0]);
			auto pv = findVariable(var.name);
			if (!pv)
				throw std::runtime_error{"Invalid predicate (var '" + var.name + "' not found) for " + e.key()};

			auto type = Predicate::Types.at(util::findString(spec, "type").get<std::string>());
			switch (type) {
				default:
					throw std::runtime_error{"Invalid predicate type"};

				case Predicate::Type::BOOL_EQ:
				{
					auto val = parseBool(e.key(), input[1]);
					predicates->emplace(std::make_pair(e.key(), std::make_unique<PBoolEQ>(e.key(), pv, val)));
					pv->predicates.push_back((*predicates)[e.key()].get());
				}
					break;

				case Predicate::Type::INT_LT:
				{
					auto val = parseInt(e.key(), input[1]);
					predicates->emplace(std::make_pair(e.key(), std::make_unique<PIntLT>(e.key(), pv, val)));
					pv->predicates.push_back((*predicates)[e.key()].get());
				}
					break;

				case Predicate::Type::STRING_MATCH:
				{
					auto pat = parsePattern(e.key(), input[1]);
					if (!pv->matcher)
						pv->matcher = std::make_unique<Matcher>(pv);

					predicates->emplace(std::make_pair(e.key(), std::make_unique<PStringMatch>(e.key(), pv, pat.pattern)));
					pv->matcher->add((*predicates)[e.key()].get());
				}
					break;
			}
		} catch (std::exception& err) {
			throw std::runtime_error{"Invalid predicate spec for " + e.key() + ": " + err.what()};
		}
	}
}

void Engine::parseTriggers(const Json& spec)
{
	for (const auto& e : util::findObj(spec, "triggers").items()) {
		auto& spec = e.value();
		if (!spec.is_object())
			throw std::runtime_error{"Invalid trigger spec (JSON object expected) for " + e.key()};

		try {
			triggers.push_back(std::make_unique<Trigger>(e.key(), spec, *predicates));
		} catch (std::exception& err) {
			throw std::runtime_error{"Invalid trigger spec for " + e.key() + ": " + err.what()};
		}
	}
}

std::map<String, int> Engine::getVariableNameIndexes() const
{
	std::map<String, int> m;
	for (int i = 0; i < variables.size(); i++)
		m[variables[i]->name] = i;
	return std::move(m);
}

std::map<uint32_t, String> Engine::getTriggerIdNames() const
{
	std::map<uint32_t, String> m;
	for (const auto& t : triggers)
		m[t->cid] = t->name;
	return std::move(m);
}

void Engine::match(Vector<std::tuple<String, VarValue>>& input, Vector<uint32_t>& output, bool printMatchedPred)
{
	auto varNameIndexes = getVariableNameIndexes();
	Vector<std::tuple<int, VarValue>> input2;

	for (auto& [name, value] : input) {
		if (varNameIndexes.find(name) == varNameIndexes.end())
			throw std::runtime_error{"Variable not found: " + name.toStdString()};
		input2.push_back(std::make_tuple(varNameIndexes[name], value));
	}

	return match(input2, output, printMatchedPred);
}

void Engine::match(Vector<std::tuple<int, VarValue>>& input, Vector<uint32_t>& output, bool printMatchedPred)
{
	matchedPreds.clear();
	matchedTriggers.clear();

	for (const auto& [i, value] : input) {
		auto& v = variables[i];
		v->value = value;

		for (auto& p : v->predicates) {
#ifdef __DEBUG__
			std::cout << "= check pred: name=" << p->name << " cid=" << p->cid << '\n';
#endif
			if (p->eval())
				matchedPreds.push_back(p);
		}

		if (v->matcher) {
#ifdef __DEBUG__
			std::cout << "= check matcher: var=" << v->matcher->variable() << '\n';
#endif
			v->matcher->match(matchedPreds);
		}
	}

	if (printMatchedPred) {
		std::cout << "================= MATCHED PREDICATES ====================\n";
		for (const auto& p : matchedPreds)
			std::cout << p->name << '\n';
	}

	// evaluate triggers
	for (auto& t : triggers)
		t->clearMatched();

	for (const auto& p : matchedPreds) {
		for (const auto& p2 : p->preds) {
			p2->trigger->addMatched(p2);
		}
	}

#ifdef __DEBUG__
	std::cout << "===== group by triggers ======\n";
	for (const auto& t : triggers) {
		if (t->hasMatched())
			t->print();
	}
#endif

	for (auto& t : triggers) {
		if (t->check())
			output.push_back(t->cid);
	}
}

void Engine::bench_match(Vector<std::tuple<String, VarValue>>& input, int total)
{
	Vector<Predicate*> preds{};
	Vector<Matcher*> matchers{};

	// collect relevant predicates and matchers
	for (auto& [name, value] : input) {
		if (auto v = findVariable(name.toStdString()); v) {
			v->value = value;
			std::copy(v->predicates.begin(), v->predicates.end(), std::inserter(preds, preds.end()));

			if (v->matcher)
				matchers.push_back(v->matcher.get());
		} else
			throw std::runtime_error{"Variable not found: " + name.toStdString()};
	}

	auto cnt = 0;
	auto start = std::chrono::high_resolution_clock::now();

	for (auto i = 0; i < total; i++) {
		matchedPreds.clear();

		// evaluate predicates and matchers
		for (auto& p : preds) {
			if (p->eval())
				matchedPreds.push_back(p);
		}

		for (auto& m : matchers)
			m->match(matchedPreds);

		// evaluate triggers
		for (auto& t : triggers)
			t->clearMatched();

		for (const auto& p : matchedPreds) {
			for (const auto& p2 : p->preds)
				p2->trigger->addMatched(p2);
		}

		matchedTriggers.clear();
		for (auto& t : triggers) {
			if (t->check())
				matchedTriggers.push_back(t->cid);
		}
		if (!matchedTriggers.empty()) cnt++;
	}

	auto diff = std::chrono::high_resolution_clock::now() - start;
	auto us = std::chrono::duration<double,std::micro>(diff).count();
	std::cout << total << " matches completed in " << us << "us (avg=" << us/total << "us or " << total*1e6/us << " matches/s)\n";
}
