// Assignment 3: Symbol Table & Assembly Code
// CPSC 323-02, CSUF Fall 2020
// Gage Dimapindan, Richard Gobea, Armon Rahimi

#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "class.h" 
#include "lexer.h"
#include "Machine.h"

int main(int argc, const char* argv[]) {
	char c;
	if (argv[1] == nullptr) {
		std::cerr << "No Input File Detected\n";
		std::cin >> c;
		return 2;
	}
	if (argv[2] == nullptr) {
		std::cerr << "No Output File Detected\n";
		std::cin >> c;
		return 2;
	}
	std::ifstream source(argv[1]);
	std::ofstream out(argv[2]);
	if (!out.is_open()) {
		std::cout << "Output file failed to open\n";
		std::cin >> c;
		return 2;
	}

	if (!source.is_open()) {
		std::cout << "Input file failed to open\n";
		std::cin >> c;
		return 2;
	}

	Rat20F(out, source);

	print_Symbols(out);
	print_instr(out);

	out.close();
	source.close();
	return 0;
}
