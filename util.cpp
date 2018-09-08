#include "util.h"

namespace
{
	const Json& find(const Json& spec, const std::string& name, nlohmann::json::value_t type) {
		const auto& p = spec.find(name);
		if (p == spec.end()) {
			throw "failed to find " + name;
		}

		const auto& v = p.value();
		if (v.type() != type) {
			throw "Invalid type for " + name;
		}
		return v;
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
}