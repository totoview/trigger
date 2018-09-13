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
		if (!engine.init()) {
			return EXIT_FAILURE;
		}

		Vector<VarValue> input{
			VarValue{ "age", 5 },
			VarValue{ "single", false },
			VarValue{ "name", String{"Mike abc test"}},
		};

		for (auto t : engine.match(input)) {
			std::cout << t << '\n';
		}

	} catch (std::exception& ex) {
		std::cerr << "Failed to parse input: " << ex.what() << '\n';
		return EXIT_FAILURE;
	} catch (std::string& msg) {
		std::cerr << msg << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
