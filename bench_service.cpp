#include <cstdlib>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <tuple>
#include "common.h"
#include "service.h"
#include "util.h"
#include "var.h"

using namespace std::chrono_literals;

int main(int argc, char* argv[])
{
	if (argc != 3) {
		std::cerr << "Usage: bench <spec> <input>\n";
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

		std::atomic_uint32_t cnt{0};
		Service service{util::readFile(argv[1]), [&cnt](Service::Result* res) {
			cnt++;
		}};

		// convert name-based input to index-based input
		auto varNameIndexes = service.getVariableNameIndexes();
		Vector<std::tuple<int, VarValue>> input2;

		for (auto& [name, value] : input) {
			if (varNameIndexes.find(name) == varNameIndexes.end())
				throw std::runtime_error{"Variable not found: " + name.toStdString()};
			input2.push_back(std::make_tuple(varNameIndexes[name], value));
		}

		std::cout << "====================== BENCHMARK ========================\n";
		service.start();

		auto start = std::chrono::high_resolution_clock::now();
		int total = 20'000'000;

		for (auto i = 0; i < total; i++) {
			while (service.tryMatch(&input2) == Service::ERR_REQ_ID)
				std::this_thread::sleep_for(1us);
		}
		while (cnt.load() < total) {
			std::this_thread::sleep_for(1ms);
		}

		auto diff = std::chrono::high_resolution_clock::now() - start;
		auto us = std::chrono::duration<double,std::micro>(diff).count();
		std::cout << total << " requests processed in " << us << "us (avg=" << us/total << "us or " << total*1e6/us << " req/s)\n";

		service.shutdown();
	} catch (const std::exception& ex) {
		std::cerr << "Failed to parse input: " << ex.what() << '\n';
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
