#include <algorithm>
#include <cctype>
#include <locale>
#include "util.h"

namespace
{
	const Json& find(const Json& spec, const std::string& name, nlohmann::json::value_t type) {
		if (!spec.is_object())
			throw "Invalid spec (object expected)";

		const auto& p = spec.find(name);
		if (p == spec.end())
			throw "Failed to find " + name;

		const auto& v = p.value();
		if (v.type() != type)
			throw "Invalid type for " + name;

		return v;
	}

	void ltrim(std::string &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::isspace(ch);
		}));
	}

	void rtrim(std::string &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}
}

namespace util
{
	const Json& findObj(const Json& spec, const std::string&& name) {
		return ::find(spec, name, nlohmann::detail::value_t::object);
	}

	const Json& findArray(const Json& spec, const std::string&& name) {
		return ::find(spec, name, nlohmann::detail::value_t::array);
	}

	const Json& findString(const Json& spec, const std::string&& name) {
		return ::find(spec, name, nlohmann::detail::value_t::string);
	}

	const Json& findBool(const Json& spec, const std::string&& name) {
		return ::find(spec, name, nlohmann::detail::value_t::boolean);
	}

	const Json& findInt(const Json& spec, const std::string&& name) {
		return ::find(spec, name, nlohmann::detail::value_t::number_integer);
	}

	const Json& findUInt(const Json& spec, const std::string&& name) {
		return ::find(spec, name, nlohmann::detail::value_t::number_unsigned);
	}

	const Json& findFloat(const Json& spec, const std::string&& name) {
		return ::find(spec, name, nlohmann::detail::value_t::number_float);
	}

	void trim(std::string &s) {
		ltrim(s);
		rtrim(s);
	}
}