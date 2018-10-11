#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <map>
#include "var.h"
#include "pred.h"
#include "trigger.h"

using PredMap = std::map<String, UPtr<Predicate>>;

class Engine {
public:
	explicit Engine(const std::string& jsonSpec);

	void match(Vector<std::tuple<String, VarValue>>& input, Vector<uint32_t>& output, bool printMatchedPred = false);
	void match(Vector<std::tuple<int, VarValue>>& input, Vector<uint32_t>& output, bool printMatchedPred = false);

	void bench_match(Vector<std::tuple<String, VarValue>>& input, int total);

	std::map<String, int> getVariableNameIndexes() const;
	std::map<uint32_t, String> getTriggerIdNames() const;

private:
	Variable* findVariable(const std::string& name) const;

	void parseVariables(const Json& spec);
	void parsePredicates(const Json& spec);
	void parseTriggers(const Json& spec);

private:
	Vector<UPtr<Variable>> variables;
	UPtr<PredMap> predicates;
	Vector<UPtr<Trigger>> triggers;

	// for trigger evaluation
	Vector<Predicate*> matchedPreds{};
	Vector<uint32_t> matchedTriggers{};
};

#endif
