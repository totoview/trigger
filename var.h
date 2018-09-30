#ifndef __VARS_H__
#define __VARS_H__

#include <map>
#include <variant>
#include "common.h"
#include "comparable.h"
#include "matcher.h"

class Predicate;

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

	String stringValue;
	long intValue;
	bool boolValue;

	// string matcher for string value
	UPtr<Matcher> matcher;

	// all other predicates
	Vector<Predicate*> predicates;
};

struct VarValue {
	VarValue(String n, bool v): name(n), value(v) {}
	VarValue(String n, int v): name(n), value(v) {}
	VarValue(String n, const String& v): name(n), value(v) {}

	String name;
	std::variant<bool, int, String> value;
};

#endif