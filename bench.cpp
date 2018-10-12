#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <tuple>
#include "common.h"
#include "service.h"
#include "util.h"
#include "var.h"

int main(int argc, char* argv[])
{
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

        Service service{util::readFile(argv[1]), [](ServiceResult* res) {

        }};

        service.start();

        for (;;) {
        }
        
        service.shutdown();
    } catch (const std::exception& ex) {
		std::cerr << "Failed to parse input: " << ex.what() << '\n';
		return EXIT_FAILURE;
    } 
    return EXIT_SUCCESS;
}
