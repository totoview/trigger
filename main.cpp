#include <chrono>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include "var.h"
#include "engine.h"
#include "util.h"

using namespace std::string_literals;

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: trigger <spec> <input>\n";
		return EXIT_FAILURE;
	}

	try {
		// parse trigger spec
		std::ifstream ifsSpec{argv[1], std::ios::binary};

		if (!ifsSpec) {
			std::cerr << "Failed to open input\n";
			return EXIT_FAILURE;
		}

		std::stringstream ssSpec;
		ssSpec << ifsSpec.rdbuf();

		// parse input data
		Vector<std::tuple<String, VarValue>> input;

		auto pattern {R"(^([^\s]+)\s*(.*)\s*$)"s};
		auto rx = std::regex{pattern};
		size_t namelen{0};

		std::ifstream ifsInput{argv[2]};

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
					std::cout << "*** [WARN] ignore line: '" << line << "'\n";
			}
		}

		std::cout << "======================== INPUT ==========================\n";
		for (const auto& [name, value] : input) {
			std::cout << std::setw(namelen) << name << ": ";
			try {
				int n = std::get<int>(value);
				std::cout << n << " (int)\n";
			} catch (...) {
				try {
					bool f = std::get<bool>(value);
					std::cout << (f ? "true" : "false") << " (bool)\n";
				} catch (...) {
					std::cout << std::get<String>(value) << " (string)\n";
				}
			}
		}

		Engine engine{ssSpec.str()};

		// convert name-based input to index-based input
		auto varNameIndexes = engine.getVariableNameIndexes();
		Vector<std::tuple<int, VarValue>> input2;

		for (auto& [name, value] : input) {
			if (varNameIndexes.find(name) == varNameIndexes.end())
				throw std::runtime_error{"Variable not found: " + name.toStdString()};
			input2.push_back(std::make_tuple(varNameIndexes[name], value));
		}


		Vector<uint32_t> fired;
		engine.match(input2, fired, true);

		std::cout << "=================== FIRED TRIGGERS ======================\n";
		auto triggerMap = engine.getTriggerIdNames();
		for (auto t : fired)
			std::cout << triggerMap[t] << '\n';

		auto total = 5000000;

		// std::cout << "====================== BENCHMARK ========================\n";
		// auto start = std::chrono::high_resolution_clock::now();
		// for (auto i = 0; i < total; i++) {
		// 	fired.clear();
		// 	engine.match(input2, fired, false);
		// }
		// auto diff = std::chrono::high_resolution_clock::now() - start;
		// auto us = std::chrono::duration<double,std::micro>(diff).count();
		// std::cout << total << " matches completed in " << us << "us (avg=" << us/total << "us or " << total*1e6/us << " matches/s)\n";

		std::cout << "====================== BENCHMARK ========================\n";
		engine.bench_match(input, total);

	} catch (std::exception& ex) {
		std::cerr << "Failed to parse input: " << ex.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
