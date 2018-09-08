#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <map>
#include "common.h"

#include "var.h"
#include "pred.h"
#include "trigger.h"

using PredMap = std::map<String, UPtr<Predicate>>;

class Engine {
public:
	explicit Engine(const std::string& jsonSpec);
	bool init();

	Vector<String> match(Vector<VarValue>& input);

private:
	void parseVariables(const Json& spec);
	void parsePredicates(const Json& spec);
	void parseTriggers(const Json& spec);

private:
	std::map<String, UPtr<Variable>> variables;
	UPtr<PredMap> predicates;
	std::map<String, UPtr<Trigger>> triggers;
};

#endif
