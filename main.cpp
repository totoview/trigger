#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include "engine.h"
#include "util.h"

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: trigger <spec> <input>\n";
		return EXIT_FAILURE;
	}

	try {
		using VarInput = std::tuple<String, VarValue>;

		// parse input data
		Vector<VarInput> input = util::readInput(argv[2]);
		auto it = std::max_element(input.begin(), input.end(), [](VarInput& a, VarInput& b) {
			return std::get<0>(a).size() < std::get<0>(b).size();
		});
		auto namelen = std::get<0>(*it).size();

		// print input
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

		// parse trigger spec
		Engine engine{util::readFile(argv[1])};

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
		auto triggerNames = engine.getTriggerIdNames();
		for (auto t : fired)
			std::cout << triggerNames[t] << '\n';

		std::cout << "====================== BENCHMARK ========================\n";
		std::cout << "Warm up...";
		for (auto i = 0; i < 1'000'000; i++) {
			fired.clear();
			engine.match(input2, fired, false);
		}
		std::cout << "done\n";

		auto total = 10'000'000;

		// auto start = std::chrono::high_resolution_clock::now();
		// for (auto i = 0; i < total; i++) {
		// 	fired.clear();
		// 	engine.match(input2, fired, false);
		// }
		// auto diff = std::chrono::high_resolution_clock::now() - start;
		// auto us = std::chrono::duration<double,std::micro>(diff).count();
		// std::cout << total << " requests processed in " << us << "us (avg=" << us/total << "us or " << total*1e6/us << " req/s)\n";

		engine.bench_match(input, total);

	} catch (std::exception& ex) {
		std::cerr << "Failed to parse input: " << ex.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
