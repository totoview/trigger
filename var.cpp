#include "var.h"

const std::map<std::string, Variable::Type> Variable::Types = {
	{ "string", Variable::Type::STRING },
	{ "int",    Variable::Type::INT },
	{ "bool",   Variable::Type::BOOL },
};
