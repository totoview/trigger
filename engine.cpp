#include <iostream>
#include "engine.h"

namespace {
	struct Input {
		std::string type;
		std::string name;
	};

	struct Pattern {
	 	std::string pattern;
		bool caseSensitive;
	};

	const Json& findObj(const Json& spec, const std::string&& name) {
		const auto& p = spec.find(name);
		if (p == spec.end()) {
			throw "failed to find " + name;
		}

		const auto& v = p.value();
		if (!v.is_object()) {
			throw "Invalid " + name + "(object expected)";
		}
		return v;
	}

	Input parseInput(const std::string& name, Json& spec) {
		if (!spec.is_object() || spec.find("type") == spec.end() || spec.find("name") == spec.end()) {
			throw "Invalid 'input' for " + name;
		}
		return Input{spec["type"], spec["name"]};
	}

	Pattern parsePattern(const std::string& name, Json& spec) {
		if (!spec.is_object() || spec.find("pattern") == spec.end()) {
			throw "Invalid 'pattern' for " + name;
		}

		auto pat = spec["pattern"];
		if (!pat.is_string()) {
			throw "Ivnalid pattern (string expected) for " + name;
		}

		Pattern p;
		p.pattern = pat.get<std::string>();

		p.caseSensitive = true;
		if (spec.find("caseSensitive") != spec.end()) {
			auto cs = spec["caseSensitive"];
			if (!cs.is_boolean()) {
				throw "Invalid caseSensitive (boolean expected) for " + name;
			}
			p.caseSensitive = cs.get<bool>();
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
	for (const auto& e : findObj(spec, "vars").items()) {
		auto& spec = e.value();
		if (!spec.is_object() || spec.find("type") == spec.end()) {
			throw "Invalid variable spec for " + e.key();
		}
		try {
			variables.emplace(std::make_pair(e.key(), std::make_unique<Variable>(e.key(), Variable::Types.at(spec["type"]))));
		} catch (const std::out_of_range& ex) {
			throw "Invalid variable spec for " + e.key();
		}
	}
}

void Engine::parsePredicates(const Json& spec)
{
	predicates = std::make_unique<PredMap>();

	for (const auto& e : findObj(spec, "predicates").items()) {
		auto& spec = e.value();
		if (!spec.is_object() || spec.find("type") == spec.end() || spec.find("input") == spec.end()) {
			throw "Invalid predicate spec " + e.key();
		}

		try {
			auto input = spec["input"];
			if (!input.is_array() || input.size() != 2) {
				throw "Invalid predicate input (not an array of length 2) for " + e.key();
			}

			auto type = Predicate::Types.at(spec["type"]);

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
		} catch (const std::out_of_range& ex) {
			throw "Invalid predicate spec for " + e.key();
		}
	}
}

void Engine::parseTriggers(const Json& spec)
{
	for (const auto& e : findObj(spec, "triggers").items()) {
		auto& spec = e.value();
		if (!spec.is_object()) {
			throw "Invalid trigger spec (JSON object expected) for " + e.key();
		}
		try {
			triggers.emplace(std::make_pair(e.key(), std::make_unique<Trigger>(e.key(), spec, *predicates)));
		} catch (const std::out_of_range& ex) {
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
