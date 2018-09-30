#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "var.h"
#include "engine.h"

int main(int argc, char* argv[]) {
	try {
		std::ifstream ifs{"test.json", std::ios::binary};

		if (!ifs) {
			std::cerr << "Failed to open input\n";
			return EXIT_FAILURE;
		}

		std::stringstream ss;
		ss << ifs.rdbuf();

		Engine engine{ss.str()};

		Vector<VarValue> input{
			VarValue{ "age", 5 },
			VarValue{ "single", false },
			VarValue{ "name", String{"Mike abc test"}},
		};

		auto cnt = 0;
		auto start = std::chrono::high_resolution_clock::now();
		int total = 1000000;
		for (auto i = 0; i < total; i++) {
			if (!engine.match(input).empty())
				cnt++;
		}
		auto diff = std::chrono::high_resolution_clock::now() - start;

		std::cout << "cnt=" << cnt << '\n';
		auto us = std::chrono::duration<double,std::micro>(diff).count();
		std::cout << total << " matches completed in " << us << "us (avg=" << us/total << "us or " << total*1e6/us << " matches/s)\n";

		// for (auto t : engine.match(input)) {
		// 	std::cout << t << '\n';
		// }

	} catch (std::exception& ex) {
		std::cerr << "Failed to parse input: " << ex.what() << '\n';
		return EXIT_FAILURE;
	} catch (std::string& msg) {
		std::cerr << msg << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
