#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <locale>
#include <regex>
#include "util.h"

using namespace std::string_literals;

namespace
{
	const Json& find(const Json& spec, const std::string& name, nlohmann::json::value_t type) {
		if (!spec.is_object())
			throw std::runtime_error{"Invalid spec (object expected)"};

		const auto& p = spec.find(name);
		if (p == spec.end())
			throw std::runtime_error{"Failed to find " + name};

		const auto& v = p.value();
		if (v.type() != type)
			throw std::runtime_error{"Invalid type for " + name};

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
	std::string readFile(const std::string filename)
	{
		std::ifstream ifs(filename, std::ios::binary);
		if (!ifs)
			throw std::runtime_error("Failed to open file" + filename);
		std::stringstream ss;
		ss << ifs.rdbuf();
		return std::move(ss.str());
	}

	Vector<std::tuple<String, VarValue>> readInput(const std::string filename) {
		Vector<std::tuple<String, VarValue>> input;

		auto pattern {R"(^([^\s]+)\s*(.*)\s*$)"s};
		auto rx = std::regex{pattern};
		size_t namelen{0};

		std::ifstream ifsInput{filename};

		for (std::string line; std::getline(ifsInput, line);) {
			util::trim(line);
			if (!line.empty()) {
				auto match = std::smatch{};
				if (std::regex_search(line, match, rx)) {
					std::string name = match[1];
					std::string value = match[2];
					namelen = std::max(namelen, name.size());

					if (value == "true") {
						input.push_back(std::make_tuple<String, VarValue>(name, true));
					} else if (value == "false") {
						input.push_back(std::make_tuple<String, VarValue>(name, false));
					} else {
						try {
							input.push_back(std::make_tuple<String, VarValue>(name, std::stoi(value)));
						} catch (...) {
							input.push_back(std::make_tuple<String, VarValue>(name, String{value}));
						}
					}
				} else
					std::cerr << "*** [WARN] ignore line: '" << line << "'\n";
			}
		}
		return input;
	}

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
