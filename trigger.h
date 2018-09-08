#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include "common.h"
#include "comparable.h"
#include "be.h"

class Trigger : public Comparable {
public:
	Trigger(const std::string& name,
		const Json& spec,
		std::map<String, UPtr<Predicate>>& predicates);

	String name;
	UPtr<BE> be;
};

#endif
