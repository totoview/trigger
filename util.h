#ifndef __UTIL_H__
#define __UTIL_H__

#include "common.h"
#include "json.h"
#include "var.h"

namespace util
{
	Vector<std::tuple<String, VarValue>> readInput(const std::string filename);

	const Json& findObj(const Json& spec, const std::string&& name);
	const Json& findArray(const Json& spec, const std::string&& name);
	const Json& findString(const Json& spec, const std::string&& name);
	const Json& findBool(const Json& spec, const std::string&& name);
	const Json& findInt(const Json& spec, const std::string&& name);
	const Json& findUInt(const Json& spec, const std::string&& name);
	const Json& findFloat(const Json& spec, const std::string&& name);

	// trim from both ends (in place)
	void trim(std::string &s);
}

#endif
