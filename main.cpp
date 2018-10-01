#include <chrono>
#include <cstdio>
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
		Vector<VarValue> input;

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
						input.push_back(VarValue{ name, true });
					} else if (value == "false") {
						input.push_back(VarValue{ name, false });
					} else {
						try {
							input.push_back(VarValue{ name, std::stoi(value) });
						} catch (...) {
							input.push_back(VarValue{ name, String{value}});
						}
					}
				} else {
					printf("*** [WARN] ignore line: '%s'\n", line.c_str());
				}
			}
		}

		printf("======================== INPUT ==========================\n");
		for (const auto& v : input) {
			printf("%*s: ", int(namelen), v.name.c_str());
			try {
				int n = std::get<int>(v.value);
				printf("%d (int)\n", n);
			} catch (...) {
				try {
					bool f = std::get<bool>(v.value);
					printf("%s (bool)\n", f ? "true" : "false");
				} catch (...) {
					printf("%s (string)\n", std::get<String>(v.value).c_str());
				}
			}
		}

		Engine engine{ssSpec.str()};


		auto fired = engine.match(input, true);

		printf("=================== FIRED TRIGGERS ======================\n");
		auto triggerMap = engine.getTriggerMap();
		for (auto t : fired)
			printf("%s\n", triggerMap[t].c_str());

		printf("====================== BENCHMARK ========================\n");
		engine.bench_match(input, 1000000);

	} catch (std::exception& ex) {
		std::cerr << "Failed to parse input: " << ex.what() << '\n';
		return EXIT_FAILURE;
	} catch (std::string& msg) {
		std::cerr << msg << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
