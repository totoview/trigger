#include <iostream>
#include "engine.h"
#include "util.h"

namespace {
	struct Input {
		std::string type;
		std::string name;
	};

	struct Pattern {
	 	std::string pattern;
		bool caseSensitive;
	};

	Input parseInput(const std::string& name, Json& spec) {
		return Input{util::findString(spec, "type"), util::findString(spec, "name")};
	}

	Pattern parsePattern(const std::string& name, Json& spec) {
		Pattern p{util::findString(spec, "pattern").get<std::string>(), true};

		try {
			p.caseSensitive = util::findBool(spec, "caseSensitive").get<bool>();
		} catch (...) {
			// nothing, optional field
		}
		return p;
	}

	bool parseBool(const std::string& name, Json& spec) {
		if (!spec.is_boolean()) {
			throw "Invalid boolean value for " + name;
		}
		return spec.get<bool>();
	}

	int parseInt(const std::string& name, Json& spec) {
		if (!spec.is_number_integer()) {
			throw "Invalid integer value for " + name;
		}
		return spec.get<int>();
	}
}

Engine::Engine(const std::string& s) {
	auto spec = Json::parse(s);

	std::cout << spec.dump(4) << std::endl;

	if (!spec.is_object()) {
		throw "Invalid input data, JSON object expected";
	}

	parseVariables(spec);
	parsePredicates(spec);
	parseTriggers(spec);
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

					if (variables.find(var.name) == variables.end()) {
						throw "Invalid predicate (var '" + var.name + "' not found) for " + e.key();
					} else {
						auto& pv = variables[var.name];
						predicates->emplace(std::make_pair(e.key(), std::make_unique<PBoolEQ>(pv.get(), val)));
						pv->predicates.push_back((*predicates)[e.key()].get());
					}
				}
					break;
				case Predicate::Type::INT_LT:
				{
					auto var = parseInput(e.key(), input[0]);
					auto val = parseInt(e.key(), input[1]);

					if (variables.find(var.name) == variables.end()) {
						throw "Invalid predicate (var '" + var.name + "' not found) for " + e.key();
					} else {
						auto& pv = variables[var.name];
						predicates->emplace(std::make_pair(e.key(), std::make_unique<PIntLT>(pv.get(), val)));
						pv->predicates.push_back((*predicates)[e.key()].get());
					}
				}
					break;
				case Predicate::Type::STRING_MATCH:
				{
					auto var = parseInput(e.key(), input[0]);
					auto pat = parsePattern(e.key(), input[1]);

					if (variables.find(var.name) == variables.end()) {
						throw "Invalid predicate (var '" + var.name + "' not found) for " + e.key();
					} else {
						auto& pv = variables[var.name];

						if (!pv->matcher) {
							pv->matcher = std::make_unique<Matcher>(pv.get());
						}

						predicates->emplace(std::make_pair(e.key(), std::make_unique<PStringMatch>(pv.get(), pat.pattern, pat.caseSensitive)));
						pv->matcher->add((*predicates)[e.key()].get());
					}
				}
					break;
			}
		} catch (...) {
			throw "Invalid predicate spec for " + e.key();
		}
	}
}

void Engine::parseTriggers(const Json& spec)
{
	for (const auto& e : util::findObj(spec, "triggers").items()) {
		auto& spec = e.value();
		if (!spec.is_object()) {
			throw "Invalid trigger spec (JSON object expected) for " + e.key();
		}
		try {
			triggers.emplace(std::make_pair(e.key(), std::make_unique<Trigger>(e.key(), spec, *predicates)));
		} catch (...) {
			throw "Invalid trigger spec for " + e.key();
		}
	}
}

bool Engine::init()
{
	// fixme: normalize BE trees
	return true;
}

Vector<String> Engine::match(Vector<VarValue>& input)
{
	// collect relevant predicates and matchers
	for (const auto& i : input) {
		if (variables.find(i.name) == variables.end()) {
			throw "Variable not found: " + i.name;
		}
		auto& v = variables[i.name];
	}

	// evaluate predicates and matchers

	// evaluate triggers
	Vector<String> matched{};
	return matched;
}
