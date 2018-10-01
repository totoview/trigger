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

	const Vector<uint64_t>& match(Vector<VarValue>& input, bool printMatchedPred = false);
	void bench_match(Vector<VarValue>& input, int total);

	std::map<uint64_t, String> getTriggerMap() const;

private:
	void parseVariables(const Json& spec);
	void parsePredicates(const Json& spec);
	void parseTriggers(const Json& spec);

private:
	std::map<String, UPtr<Variable>> variables;
	UPtr<PredMap> predicates;
	Vector<UPtr<Trigger>> triggers;

	// for trigger evaluation
	Vector<Predicate*> matchedPreds{};
	Vector<uint64_t> matchedTriggers{};
};

#endif
