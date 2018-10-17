#ifndef __VARS_H__
#define __VARS_H__

#include <map>
#include <variant>
#include "common.h"
#include "comparable.h"
#include "matcher.h"

class Predicate;

using VarValue = std::variant<bool, int, String>;

class Variable : public Comparable<Variable> {
public:
	enum struct Type {
		INT,
		BOOL,
		STRING,
	};
	static const std::map<std::string, Type> Types;

public:
	Variable(const std::string& n, Type t) : name(n), type(t) {}

	String name;
	Type type;
	VarValue value;

	// string matcher for string value
	UPtr<Matcher> matcher;

	// all other predicates
	Vector<Predicate*> predicates;
};

#endif

